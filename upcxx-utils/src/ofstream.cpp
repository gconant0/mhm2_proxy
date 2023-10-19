#include "upcxx_utils/ofstream.hpp"

#include <fcntl.h>
#include <unistd.h>

#include "upcxx_utils/binary_search.hpp"
#include "upcxx_utils/log.hpp"
#include "upcxx_utils/reduce_prefix.hpp"

using upcxx::dist_object;
using upcxx::rank_me;
using upcxx::rank_n;

namespace upcxx_utils {

dist_ofstream_handle::OffsetSizeBuffer::OffsetSizeBuffer()
    : offset(0)
    , size(0)
    , sh_obw(nullptr) {}

dist_ofstream_handle::OffsetSizeBuffer::OffsetSizeBuffer(uint64_t offset, uint64_t size, ShOptimizedBlockWrite sh_obw)
    : offset(offset)
    , size(size)
    , sh_obw(sh_obw) {}

// compare for binary search.  Returns -1, 0 or 1 assuming a prefix reduction of osb.offset for the rank that contains the given pos

int dist_ofstream_handle::OffsetSizeBuffer::operator()(const dist_object<OffsetSizeBuffer> &dist_osb, const uint64_t &pos) const {
  const OffsetSizeBuffer &osb = *dist_osb;
  if (pos < osb.offset) {
    return -1;
  } else if (pos > osb.offset + osb.size) {
    return 1;
  } else {
    // it depends on edge cases...
    // osb.offset <= pos <= osb.offset + osb.size
    DBG_VERBOSE("pos=", pos, " osb=", (string)osb, "\n");
    assert(pos >= osb.offset);
    assert(pos <= osb.offset + osb.size);
    if (osb.size == 0) {
      assert(pos == osb.offset);
      return 1;  // next rank will have same offset, let the next non-zero sized rank handle
    }
    if (osb.size > 0 && pos == osb.offset + osb.size) return 1;  // next non-zero-sized rank will have pos as offset
    DBG_VERBOSE("Found pos=", pos, " within my range: offset=", osb.offset, " size=", osb.size, " last=", osb.offset + osb.size,
                "\n");
    return 0;
  }
}

string dist_ofstream_handle::OffsetSizeBuffer::to_string() const {
  ostringstream os;
  _logger_recurse(os, "OffsetSizeBuffer(offset=", offset, " size=", size, " sh_obw=", sh_obw, ")");
  return os.str();
}

dist_ofstream_handle::OffsetSizeBuffer::operator string() const { return to_string(); }

std::map<upcxx::team_id, shared_ptr<dist_ofstream_handle::AD> > &dist_ofstream_handle::dist_ofstream_handle_state::get_ad_map() {
  // hack to avoid most AD construction & destruction
  static std::map<upcxx::team_id, shared_ptr<AD> > ad_map;
  return ad_map;
}

void dist_ofstream_handle::dist_ofstream_handle_state::clear_ad_map() {
  // hack - memory lead to avoid most AD construction & destruction
  barrier();
  auto &ad_map = get_ad_map();
  for (auto &ad_kv : ad_map) {
    ad_kv.second->destroy(upcxx::entry_barrier::none);
  }
  ad_map.clear();
  barrier();
}

dist_ofstream_handle::AD &dist_ofstream_handle::dist_ofstream_handle_state::get_ad(const upcxx::team &team) {
  // hack introduces a memory leak so atomic domains are never destroyed
  auto &ad_map = get_ad_map();
  auto it = ad_map.find(world().id());  // try world first which should be sufficient
  if (it == ad_map.end()) {
    // no world, so lets allocate a new one for this team
    it = ad_map.find(team.id());
  }
  if (it == ad_map.end()) {
    barrier(team);
    it = ad_map.insert(it, {team.id(), make_shared<AD>(ad_ops(), team)});
  }
  assert(it != ad_map.end());
  return *(it->second);
}

dist_ofstream_handle::dist_ofstream_handle_state::dist_ofstream_handle_state(const string _fname, const upcxx::team &_myteam)
    : myteam(_myteam)
    , ad(get_ad(myteam))
    , global_offset(nullptr)
    , count_async(0)
    , count_collective(0)
    , count_bytes(0)
    , wrote_bytes(0)
    , opening_ops(make_future())
    , pending_io_ops(make_future())
    , pending_net_ops(make_future())
    , msm_metrics(7)
    , fname(_fname)
    , last_known_tellp(0)
    , close_barrier(myteam)
    , fd(-1) {}
dist_ofstream_handle::dist_ofstream_handle_state::~dist_ofstream_handle_state() {
  DBG_VERBOSE("Destroying state ", this, " ", fname, " fd=", fd, " global_offset=", global_offset,
              " opening_ops=", opening_ops.ready(), " io_ops=", pending_io_ops.ready(), " net_ops=", pending_net_ops.ready(), "\n");
}

dist_ofstream_handle::dist_ofstream_handle(const string _fname, const upcxx::team &_myteam, bool append)
    : sh_state(make_shared<dist_ofstream_handle_state>(_fname, _myteam))
    , fd(sh_state->fd)
    , fname(sh_state->fname)
    , myteam(sh_state->myteam)
    , ad(sh_state->ad)
    , global_offset(sh_state->global_offset)
    , count_async(sh_state->count_async)
    , count_collective(sh_state->count_collective)
    , count_bytes(sh_state->count_bytes)
    , wrote_bytes(sh_state->wrote_bytes)
    , opening_ops(sh_state->opening_ops)
    , pending_net_ops(sh_state->pending_net_ops)
    , pending_io_ops(sh_state->pending_io_ops)
    , is_closed(false) {
  DBG_VERBOSE("fname=", fname, " append=", append, "\n");

  uint64_t pos = 0;

  if (myteam.rank_me() == 0) {
    open_file_sync(sh_state, append);
    if (append) {
      
      assert(sh_state->fd >= 0);
      pos = lseek(sh_state->fd, 0, SEEK_END);
      LOG("Found pos = ", pos, " in appended file ", sh_state->fname, "\n");
      
      sh_state->last_known_tellp = pos;
    }
  }
  if (myteam.rank_me() == 0) {
    sh_state->global_offset = upcxx::new_<uint64_t>(pos);
  }

  
  auto broadcast_global_offset_lambda = [sh_state = this->sh_state](global_ptr<uint64_t> global_offset) {
    
    sh_state->global_offset = global_offset;
    DBG_VERBOSE("dist_ofstream finished construction net\n");
  };

  // this is effectively a barrier for other ranks on rank0 opening the file
  auto fut_bcast = upcxx::broadcast(sh_state->global_offset, 0, sh_state->myteam).then(broadcast_global_offset_lambda);

  // no additional io operations are pending
  pending_net_ops = when_all(pending_net_ops, fut_bcast);
  opening_ops = when_all(opening_ops, pending_net_ops);
}

dist_ofstream_handle::~dist_ofstream_handle() {
  DBG_VERBOSE("fname=", fname, ", fd=", fd, " is_closed=", is_closed, "\n");
  if (!is_closed) {
    // close_async was never called, so close and wait now
    close_file().wait();
  }
}

future<> dist_ofstream_handle::get_pending_ops(ShState sh_state) {
  return when_all(sh_state->pending_net_ops, sh_state->pending_io_ops, sh_state->opening_ops);
}
future<> dist_ofstream_handle::get_pending_ops() const { return get_pending_ops(sh_state); }

uint64_t dist_ofstream_handle::write_block(ShState sh_state, const char *src, uint64_t len, uint64_t file_offset) {
  if (len > 0) {
    assert(!sh_state->global_offset.is_null());
    if (!sh_state->is_open()) open_file_sync(sh_state);
    
    assert(sh_state->fd >= 0);
    uint64_t wrote_bytes = 0;
    int attempts = 0;
    while (wrote_bytes < len) {
      int64_t bytes = pwrite(sh_state->fd, src + wrote_bytes, len - wrote_bytes, file_offset + wrote_bytes);
      if (bytes < 0) {
        DIE("Error writing ", len, " bytes to ", sh_state->fname, " at offset ", file_offset, "!", strerror(errno), "\n");
      }
      if (bytes != len) {
        DBG("Could not write all ", len, " bytes. Wrote ", wrote_bytes, " + ", bytes, " at offset ", file_offset, "! ",
            strerror(errno), "\n");
      }
      if (bytes == 0 && ++attempts > 100) {
        DIE("Could not write ", len, " bytes (wrote", wrote_bytes, ") at offset ", file_offset, " after ", attempts, " making 0 ",
            strerror(errno), "\n");
      }
      wrote_bytes += bytes;
    }
    LOG("Wrote ", len, " at ", file_offset, " to ", sh_state->fname, "\n");

  }
  sh_state->wrote_bytes += len;
  return sh_state->last_known_tellp = file_offset + len;
}

void dist_ofstream_handle::read_all(ShSS sh_ss, char *buf, uint64_t len) {
  DBG_VERBOSE("Reading len=", len, " from ss tellg=", sh_ss->tellg(), " tellp=", sh_ss->tellp(), "\n");
  for (uint64_t read_start = 0; read_start < len;) {
    auto read_len = len - read_start;
    // DBG_VERBOSE("Reading len=", len, " from ss tellg=", sh_ss->tellg(), " tellp=", sh_ss->tellp(), " read_start=", read_start, "
    // read_len=", read_len, "\n");
    sh_ss->read(buf + read_start, read_len);
    auto read_bytes = sh_ss->gcount();
    if (read_bytes != read_len) {
      if (sh_ss->fail())
        DIE("stringstream is not failed after read_len=", read_len, " read_bytes=", read_bytes, " len=", len, "\n");
    }
    read_start += read_bytes;
  }
}

uint64_t dist_ofstream_handle::write_block(ShState sh_state, ShSS sh_ss, uint64_t file_offset) {
  uint64_t len = sh_ss->tellp() - sh_ss->tellg();
  DBG_VERBOSE("Writing len=", len, " sh_ss tellp=", sh_ss->tellp(), " tellg=", sh_ss->tellg(), " at file_offset=", file_offset,
              "\n");
  const uint64_t max_buf = std::min(len, (uint64_t)16 * 1024 * 1024);  // 16 MB
  uint64_t ret = 0;
  char *buf = new char[max_buf];
  while (len) {
    uint64_t buf_len = std::min(len, max_buf);
    read_all(sh_ss, buf, buf_len);
    ret = write_block(sh_state, buf, buf_len, file_offset);
    if (sh_ss->gcount() != buf_len) DIE("incomplete read from stringstream!");
    file_offset += buf_len;
    len -= buf_len;
  }
  delete[] buf;
  return ret;
}

void dist_ofstream_handle::open_file_sync(ShState sh_state, bool append) {
  if (sh_state->is_open()) return;
  
  int flags = O_WRONLY;  // TODO add O_DIRECT when caching is ready
  // rank 0 can create the file, others must see it already
  string tmpfname = sh_state->fname + ".tmp";
  if (sh_state->myteam.rank_me() == 0) {
    flags |= O_CREAT;
    if (append && file_exists(sh_state->fname)) {
      auto ret = rename(sh_state->fname.c_str(), tmpfname.c_str());
      if (ret < 0) {
        SDIE("Could not append file ", sh_state->fname, " as the rename failed\n");
      } else {
        LOG("Appending existing file ", sh_state->fname, ", now ", tmpfname, "\n");
      }
    } else {
      append = false;  // cannot append a non-existant file
    }
    if (!append) {
      flags |= O_EXCL;
      unlink(tmpfname.c_str());  // remove if it already exists
    }
  }
  int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  sh_state->fd = open(tmpfname.c_str(), flags, mode);
  if (sh_state->fd < 0) DIE("Could not open ", tmpfname, "!", strerror(errno), "\n");
  
  
  DBG("Opened ", tmpfname, "\n");
}

future<> dist_ofstream_handle::open_file(ShState sh_state, bool append) {
  if (sh_state->is_open()) return sh_state->opening_ops;
  // do not affect pending ops as this call may be a *dependency* of pending_io_ops
  // depends on opening_ops, such as broadcast / barrier from rank 0 opening and possibly renaming first
  auto fut_open = sh_state->opening_ops.then([sh_state, append]() {
    assert(!sh_state->global_offset.is_null());
    open_file_sync(sh_state, append);
    assert(sh_state->is_open());
  });
  // tracking opening_op
  sh_state->opening_ops = when_all(sh_state->opening_ops, fut_open);
  // opening is not also an pending_io_op, as a lazy open may be a dependency on one
  return fut_open;
}
future<> dist_ofstream_handle::open_file(bool append) { return open_file(sh_state, append); }

future<bool> dist_ofstream_handle::is_open_async(ShState sh_state) {
  return when_all(sh_state->pending_io_ops, sh_state->opening_ops).then([sh_state]() { return sh_state->is_open(); });
}
bool dist_ofstream_handle::is_open(ShState sh_state) { return is_open_async(sh_state).wait(); }

future<bool> dist_ofstream_handle::is_open_async() const { return is_open_async(sh_state); }
bool dist_ofstream_handle::is_open() const { return is_open_async().wait(); }

string dist_ofstream_handle::get_file_name() const { return fname; }

void dist_ofstream_handle::tear_down(ShState sh_state, bool did_open) {
  assert(sh_state->fd == -1);
  assert(sh_state->pending_net_ops.ready());

  // barrier (from fut_all_closing) is necessary for rename operation
  // -- all lazy opens must have happened before
  // AtomicDomain is NOT destroyed because of hack and static get_ad methods
  ////// sh_state->ad.destroy(upcxx::entry_barrier::none);
  if (sh_state->global_offset && sh_state->global_offset.where() == rank_me()) {
    upcxx::delete_(sh_state->global_offset);
  }
  sh_state->global_offset = nullptr;
  DBG("Finished teardown\n");

  // barrier (from fut_all_closing) is necessary to prevent renaming before a lazy open has completed
  if (sh_state->myteam.rank_me() == 0) {
    // rename tmp back
    string tmpfname = sh_state->fname + ".tmp";
    
    auto ret_un = unlink(sh_state->fname.c_str());  // ignore any error here
    DBG_VERBOSE("Unlinked ", sh_state->fname.c_str(), " (errors okay) ret=", ret_un, " ",
                string(ret_un == 0 ? "" : strerror(errno)), "\n");
    auto ret = rename(tmpfname.c_str(), sh_state->fname.c_str());
    if (ret < 0) {
      DIE("Could not rename ", tmpfname, " to ", sh_state->fname, "! ", strerror(errno), "\n");
    }
    LOG("Renamed back to ", sh_state->fname, "\n");
  }

 
  auto &msms = sh_state->msm_metrics;
  assert(msms.size() == 3);
  
  auto &msm_wrote = msms[0];
  msm_wrote.reset(sh_state->wrote_bytes);
  auto &msm_had_io = msms[1];
  msm_had_io.reset(did_open);
  auto &msm_open = msms[2];
}

double dist_ofstream_handle::close_file_sync(ShState sh_state) {
  DBG_VERBOSE("Actually closing the fd: ", sh_state->fd, "\n");
  if (sh_state->fd < 0) return 0.0;
  
  
  close(sh_state->fd);
  sh_state->fd = -1;
  LOG("Closed ", sh_state->fname, "\n");
  return 0.0;
}

future<> dist_ofstream_handle::close_file() {
  DBG_VERBOSE("fname=", fname, " is_closed=", is_closed, "\n");

  // do not call close a second time!
  if (is_closed) return get_pending_ops();
  is_closed = true;

  // start a barrier that ensures synchronous ordering of events
  // and that all ranks tear down after all have called this close_file method
  sh_state->close_barrier.fulfill();
  pending_net_ops = when_all(pending_net_ops, sh_state->close_barrier.get_future());
  auto fut_all_closing = pending_net_ops;

  // The close operation may execute after dist_ofstream_handle goes out of scope, so lambda-capture/copy member data

  auto close_and_teardown_lambda = [sh_state = this->sh_state, fut_all_closing]() {
    DBG_VERBOSE("\n");

    double file_op_duration = 0.0;
    int did_open = sh_state->fd >= 0 ? 1 : 0;
    if (did_open) {
      file_op_duration = close_file_sync(sh_state);
    }
    

    auto teardown_lamba_after_close = [sh_state, did_open] {
      tear_down(sh_state, did_open);
    };

    auto fut_stats_and_teardown = fut_all_closing.then(teardown_lamba_after_close);

    return fut_stats_and_teardown;
  };

  auto fut_closed = pending_io_ops.then(close_and_teardown_lambda);

  return pending_io_ops = when_all(fut_closed, fut_all_closing)  // fut_all_closing include pending_net_ops
                              .then([sh_state = this->sh_state]() { DBG_VERBOSE("Finished closing\n"); });
}

future<> dist_ofstream_handle::report_timings(ShState sh_state) {
  // this method initiates a collective, and must be run after all network_ops and io_ops have completed.
  get_pending_ops(sh_state).wait();

  double micro_s = 1000000.0;
  auto &msms = sh_state->msm_metrics;

  auto report_timings_lambda = [sh_state, micro_s]() {
    auto &msms = sh_state->msm_metrics;
    auto &msm_bytes = msms[0];
    auto &msm_wrote = msms[1];
    auto &msm_had_io = msms[2];
 

    LOG("Writing times: ", sh_state->fname,  "s bytes ", get_size_str(msm_bytes.my), " wrote ",
        get_size_str(msm_wrote.my), "\n");

    if (!sh_state->myteam.rank_me()) {
      intrank_t writers = msm_had_io.sum;
      assert(msm_bytes.sum == 0 || writers > 0);
      assert(writers <= sh_state->myteam.rank_n());

      SLOG_VERBOSE("Wrote file ", sh_state->fname, writers,
                   " writers of ",
                   get_size_str(msm_wrote.sum), "-", get_size_str(msm_wrote.max), " writes\n");
      
    }
    LOG("Closed upcxx_utils::ofstream ", sh_state->fname, " with ", sh_state->count_bytes, " flushed bytes (",
        sh_state->wrote_bytes, " written) and ", sh_state->count_async, " async and ", sh_state->count_collective,
        " collective operations\n");
  };
  auto fut = min_sum_max_reduce_one(msms.data(), msms.data(), msms.size(), 0, sh_state->myteam).then(report_timings_lambda);
  sh_state->pending_net_ops = when_all(sh_state->pending_net_ops, fut);
  return fut;
}
future<> dist_ofstream_handle::report_timings() const { return report_timings(this->sh_state); }

future<uint64_t> dist_ofstream_handle::append_batch_async(ShSS sh_ss) {
  auto ss_size = sh_ss->tellp();
  LOG("append_batch_async(fname=", fname, " sh_ss bytes=", ss_size, ")\n");

  if (ss_size == 0) return pending_io_ops.then([]() { return (uint64_t)0; });
  count_bytes += ss_size;
  future<uint64_t> fut_pos = pending_io_ops.then([sh_ss, ss_size, sh_state = this->sh_state]() {
    DBG_VERBOSE("append_batch_asyc size=", ss_size, "\n");
    sh_state->count_async++;
    auto fut_open = open_file(sh_state);
    return when_all(sh_state->pending_net_ops, fut_open)
        .then([sh_state, sh_ss, ss_size]() {
          
          return sh_state->ad.fetch_add(sh_state->global_offset, ss_size, std::memory_order_relaxed);
        })
        .then([sh_ss, sh_state](uint64_t write_offset) {
          
          return write_block(sh_state, sh_ss, write_offset);
        });
  });
  pending_io_ops = fut_pos.then([](uint64_t ignored) { DBG_VERBOSE("append_batch_async done. pos=", ignored, "\n"); });
  return fut_pos;
}

dist_ofstream_handle::OffsetPrefixes dist_ofstream_handle::getOffsetPrefixes(ShState sh_state, uint64_t my_size) {
  DBG_VERBOSE("my_size=", my_size, "\n");
  // this method initiates collectives and blocks. wait on other pending collectives to complete first

  // first start prefix reduction
  auto fut_prefix = reduce_prefix(my_size, upcxx::op_fast_add, sh_state->myteam, true);

  // wait on other net_ops (like global_offset broadcast)
  sh_state->pending_net_ops.wait();

  
  uint64_t prefix = fut_prefix.wait();

  // open the file and get the base_offset
  uint64_t global_start_size[2];
  auto &global_start = global_start_size[0];
  auto &global_size = global_start_size[1];
  if (sh_state->myteam.rank_me() == 0) {
    // add the total append size to the global_offset
    // rank0 has the global size presently
    global_size = prefix;
    // rank 0 is special, replace offset with just its size, not the total size
    prefix = my_size;

    global_start = sh_state->ad.fetch_add(sh_state->global_offset, global_size, std::memory_order_relaxed).wait();

    if (global_size) {
     
      // truncate the file once the global_offset has been atomically changed

      
      open_file_sync(sh_state);
      auto ret = ftruncate(sh_state->fd, global_start + global_size);
      if (ret != 0) {
        DIE("Could not ftruncate ", sh_state->fname, "! ", strerror(errno), "\n");
      }
      LOG("truncated ", sh_state->fname, " to (", global_start, " + ", global_size, ") bytes\n");
      
    } else {
      // special case of 0 byte file write.  Rank 0 still needs to open it to later make an empty file
      DBG_VERBOSE("Opening what will be an empty file: ", sh_state->fname, "\n");
      open_file(sh_state);  // future already captured by opening_ops
    }
    DBG_VERBOSE("global_start=", global_start, " global_size=", global_size, "\n");
  }
  // this broadcast is effectively a barrier for the file to be truncated by rank 0
  upcxx::broadcast(global_start_size, 2, 0, sh_state->myteam).wait();
  assert(prefix >= my_size);

  OffsetPrefix global{global_start, global_size}, my{prefix - my_size, my_size};
  OffsetPrefixes offpre{global, my};

  // no additional network, io or opening operations are pending
  assert(sh_state->pending_net_ops.ready());

  DBG_VERBOSE("global.start=", offpre.global.start, " global.size=", offpre.global.size, " my.start=", offpre.my.start,
              " my.size=", offpre.my.size, "\n");
  return offpre;
}

dist_ofstream_handle::ShDistOffsetSizeBuffer dist_ofstream_handle::write_blocked_batch_collective_start(
    ShState sh_state, OffsetPrefixes offset_prefixes, uint64_t block_size, ShSS sh_ss) {
  auto ss_size = sh_ss->tellp() - sh_ss->tellg();

  DBG("Optimizing write (of ", ss_size, ") for block_size=", block_size, " total_write=", offset_prefixes.global.size, "\n");
  // optimize the writing using fewer ranks in block_sized chunks at block boundaries
  // creates a dist_object and may perform a binary search across it
  // may send up to block_size bytes to another rank before data is fully written to files

  auto my_prefix_offset = offset_prefixes.global.start + offset_prefixes.my.start;
  auto &my_size = offset_prefixes.my.size;
  auto &base_offset = offset_prefixes.global.start;
  auto &total_write_size = offset_prefixes.global.size;

  ShOptimizedBlockWrite sh_obw = make_shared<OptimizedBlockWrite>(sh_ss, my_prefix_offset, block_size, 0, 0, sh_state->myteam);
  OptimizedBlockWrite &obw = *sh_obw;
  assert(obw.my_size == my_size);
  assert(obw.my_size == ss_size);

  uint64_t last_byte = obw.last_byte();      // my_prefix_offset + my_size;
  uint64_t start_block = obw.start_block();  // my_prefix_offset / block_size;
  uint64_t end_block = obw.end_block();      // last_byte / block_size;

  uint64_t start_block_offset = obw.start_block_offset();  // my_prefix_offset % block_size;
  uint64_t end_block_offset = obw.end_block_offset();      // last_byte % block_size;

  // is_first is (not necessarily rank0) covers the entire first block when appending.
  bool is_first = my_prefix_offset == base_offset;

  bool will_send = my_size > 0 && !is_first && start_block_offset != 0;
  uint64_t &send_first_bytes = obw.send_first_bytes;
  send_first_bytes = 0;
  if (will_send) {
    if (start_block_offset + my_size <= block_size) {
      // send entire part
      send_first_bytes = my_size;
    } else {
      send_first_bytes = block_size - start_block_offset;
    }
    DBG_VERBOSE("Will send ", send_first_bytes, " bytes\n");
  }

  // will not receive if ending at a block boundary -- a higher rank will instead
  bool will_receive = my_size - send_first_bytes > 0 && end_block_offset != 0;
  uint64_t end_block_remainder = block_size - end_block_offset;
  if (end_block == (base_offset + total_write_size) / block_size) {
    // participant of the end_block == very last block

    DBG_VERBOSE("Writing in very last block: ", end_block, " at end pos=", base_offset + total_write_size, "\n");
    uint64_t last_block_size = (base_offset + total_write_size) % block_size;
    assert(last_block_size >= end_block_offset);
    end_block_remainder = last_block_size - end_block_offset;

    if (end_block_remainder == 0) {
      will_receive = false;
    }
  }

  if (will_receive) {
    assert(my_size > 0);
    assert(end_block_offset != 0);
    assert(end_block_remainder > 0);
    assert((my_prefix_offset + my_size) < (base_offset + total_write_size));
    DBG_VERBOSE("Will receive ", end_block_remainder, " bytes\n");
  }
  assert(will_send == (obw.send_first_bytes > 0));

  DBG_VERBOSE("my_prefix_offset=", my_prefix_offset, " my_size=", my_size, " start_block=", start_block, " end_block=", end_block,
              " start_block_offset=", start_block_offset, " end_block_offset=", end_block_offset,
              " end_block_remainder=", end_block_remainder, " base_offset=", base_offset, " total_write_size=", total_write_size,
              "\n");

  uint64_t receive_bytes = will_receive ? end_block_remainder : 0;
  if (will_receive) {
    assert(receive_bytes > 0);
    obw.receive_bytes(receive_bytes);
    DBG_VERBOSE("Will receive ", receive_bytes, " for remainder of my last block into buffer after ", end_block_offset, "\n");
  }
  obw.receive_prom.fulfill_anonymous(1);  // 1 extra from construction

  // create a distributed object to share buffer communications
  // scope is this function and dependent future chains and incoming rpcs / promises
  // find the rank with the start of my start block

  uint64_t my_range_start = is_first ? my_prefix_offset - start_block_offset : my_prefix_offset;
  uint64_t my_range_size = is_first && my_size > 0 ? start_block_offset + my_size : my_size;
  auto sh_dist_offset = make_shared<DistOffsetSizeBuffer>(sh_state->myteam, my_range_start, my_range_size, sh_obw);

  DBG_VERBOSE("constructed dist_obj my_range_start=", my_range_start, " my_range_size=", my_range_size, "\n");

  return sh_dist_offset;
}

future<> dist_ofstream_handle::write_blocked_batch_collective_finish(ShState sh_state, ShDistOffsetSizeBuffer sh_dist_offset) {
  future<> fut_receive = make_future();
  future<> fut_send = make_future();
  future<> fut_wrote_my = make_future();
  future<> fut_binary_search = make_future();

  auto &[my_range_start, my_range_size, sh_obw] = *(*sh_dist_offset);
  const auto &team = sh_dist_offset->team();
  OptimizedBlockWrite &obw = *sh_obw;

  auto receive_bytes = obw.receive_buf.size();

  if (receive_bytes > 0) {
    fut_receive = obw.receive_prom.get_future();
  }

  auto &sh_ss = obw.sh_ss;
  auto &send_first_bytes = obw.send_first_bytes;
  auto &my_size = obw.my_size;
  auto &block_size = obw.block_size;

  future<> write_fut = sh_state->pending_io_ops;

  if (obw.send_first_bytes > 0) {
    // find rank to send first bytes
    auto start_block = obw.start_block();
    DBG_VERBOSE("Sending first bytes send_first_bytes=", send_first_bytes,
                " to another rank at start_block_offset=", obw.start_block_offset(), " in block# ", start_block,
                " which starts at ", start_block * block_size, "\n");

    
    future<intrank_t> fut_find_rank = binary_search_rpc(*sh_dist_offset, start_block * block_size);
    fut_binary_search = fut_find_rank.then([sh_dist_offset, &obw, start_block, sh_state](intrank_t ignored) {
      if (ignored >= sh_dist_offset->team().rank_n()) {
        WARN("Did not find rank for start_block# ", start_block, " at ", start_block * obw.block_size, "\n");
      }
      DBG_VERBOSE("Finished binary search\n");
      
    });

    // read and send the first bytes
    assert(send_first_bytes <= my_size);
    char *send_buf = new char[send_first_bytes];
    read_all(sh_ss, send_buf, send_first_bytes);
    if (sh_ss->gcount() != send_first_bytes) DIE("incomplete read from stringstream!");

    fut_send = fut_find_rank.then([sh_dist_offset, send_first_bytes, &obw, send_buf](intrank_t dest) {
      if (dest >= sh_dist_offset->team().rank_n()) DIE("Can not send to an unknown destination from nowhere\n");
      DBG_VERBOSE("Sending send_first_bytes=", send_first_bytes, " bytes to ", dest,
                  " at start_block_offset=", obw.start_block_offset(), "\n");

      rpc_ff(sh_dist_offset->team(), dest,
             [](DistOffsetSizeBuffer &dist_osb, intrank_t fromrank, uint64_t start_block_offset, upcxx::view<Byte> data) {
               DBG_VERBOSE("Received ", data.size(), " bytes from ", fromrank, "\n");
               OffsetSizeBuffer &osb = *dist_osb;
               assert(osb.sh_obw);
               OptimizedBlockWrite &obw = *osb.sh_obw;
               assert(start_block_offset >= obw.end_block_offset() && "sender start offset >= receiver end offset");
               auto offset = start_block_offset - obw.end_block_offset();
               auto &buf = obw.receive_buf;
               assert(buf.size() >= offset + data.size());
               auto dest = buf.data() + offset;
               std::copy(data.begin(), data.end(), dest);
               obw.receive_prom.fulfill_anonymous(data.size());
             },
             *sh_dist_offset, sh_dist_offset->team().rank_me(), obw.start_block_offset(),
             make_view(send_buf, send_buf + send_first_bytes, send_first_bytes));
      delete[] send_buf;
    });
  }

  if (my_size > send_first_bytes) {
    DBG_VERBOSE("writing my (my_size=", my_size, " - send_first_bytes=", send_first_bytes, " = ", my_size - send_first_bytes,
                " bytes\n");
    // write my bytes to my block(s)
    uint64_t write_size = my_size - send_first_bytes;
    uint64_t write_offset = obw.offset + send_first_bytes;
    assert(sh_ss->tellp() - sh_ss->tellg() == write_size);

    // write after send has read the send_first_bytes bytes
    fut_wrote_my = when_all(write_fut, open_file(sh_state))
                       .then([sh_ss, write_size, send_first_bytes, write_offset, sh_state, sh_dist_offset, &obw]() {
                         DBG_VERBOSE("Writing my range write_size=", write_size, " at my_prefix_offset=", obw.offset,
                                     " + send_first_bytes=", send_first_bytes, " == write_offset=", write_offset, "\n");
                         assert(sh_ss->tellp() - sh_ss->tellg() == write_size);
                         write_block(sh_state, sh_ss, write_offset);
                         assert(sh_ss == obw.sh_ss);
                         obw.sh_ss.reset();
                       });
  }

  // write received bytes when ready
  future<> fut_wrote_received = make_future();
  if (obw.will_receive()) {
    assert(obw.receive_bytes() > 0);
    uint64_t file_offset = obw.end_block() * obw.block_size + obw.end_block_offset();
    DBG_VERBOSE("Will receive ", obw.receive_bytes(), " bytes and write them at file_offset=", file_offset, "\n");
    future<> receive_ready_fut = when_all(write_fut, fut_wrote_my, fut_receive);  // keep write ordering
    fut_wrote_received = receive_ready_fut.then([sh_state, &obw, file_offset, sh_dist_offset]() {
      DBG_VERBOSE("Received all bytes, writing and freeing buffer at ", file_offset, " for ", obw.receive_bytes(), " len\n");
      write_block(sh_state, obw.receive_buf.data(), obw.receive_buf.size(), file_offset);
      // free the buffer
      obw.clear_receive_buf();
    });
  } else {
    assert(obw.receive_buf.empty());
  }

  // preserve lifetime of dist_objects OptimizedBlockWrite and sh_state
  // before the distributed objects are destroyed
  // write io operations and other network operations can still remain pending

  auto sh_obw_copy = sh_obw;
  sh_state->pending_net_ops =
      when_all(sh_state->pending_net_ops, fut_binary_search, fut_send, fut_receive).then([sh_dist_offset, sh_obw = sh_obw_copy]() {
        sh_obw->all_done_barrier.fulfill();
        return sh_obw->all_done_barrier.get_future().then([sh_dist_offset, sh_obw]() {});
      });

  sh_state->pending_io_ops =
      when_all(sh_state->pending_io_ops, fut_wrote_my, fut_wrote_received).then([sh_dist_offset, sh_state]() {
        DBG_VERBOSE("Completed write in write_blocked_batch_collective\n");
      });

  discharge();  // ensure communications have at least started by this rank
  return sh_state->pending_io_ops;
}

future<uint64_t> dist_ofstream_handle::append_batch_collective(ShSS sh_ss, uint64_t block_size) {
  uint64_t ss_size = sh_ss->tellp() - sh_ss->tellg();
  LOG("append_batch_collective(fname=", fname, " sh_ss bytes=", ss_size, ", block_size=", block_size, ")\n");
  count_bytes += ss_size;
  count_collective++;

  OffsetPrefixes offset_prefixes = getOffsetPrefixes(sh_state, ss_size);

  future<> fut_write = pending_io_ops;
  if (block_size > 0) {
    DBG_VERBOSE("Block optimized write of ", block_size, "\n");
    auto sh_dist_offset_size = write_blocked_batch_collective_start(sh_state, offset_prefixes, block_size, sh_ss);
    fut_write = write_blocked_batch_collective_finish(sh_state, sh_dist_offset_size);

  } else {
    DBG_VERBOSE("No block optimization\n");
    // once all offsets are calculated, simply fseek and write this data
    if (ss_size) {
      fut_write = fut_write.then([sh_state = this->sh_state, sh_ss, ss_size, offset_prefixes]() {
        auto file_offset = offset_prefixes.global.start + offset_prefixes.my.start;
        assert(ss_size > 0);
        DBG_VERBOSE("append_batch_collective writing directly\n");
        auto pos = write_block(sh_state, sh_ss, file_offset);
        assert(pos == file_offset + ss_size);
      });
    }
  }

  auto fut_pos = fut_write.then([sh_state = this->sh_state, ss_size, offset_prefixes]() {
    DBG_VERBOSE("append_batch_collective of ", ss_size, " done\n");
    uint64_t pos = offset_prefixes.global.start + offset_prefixes.global.size;
    sh_state->last_known_tellp = pos;
    return pos;
  });

  pending_io_ops = fut_pos.then([sh_state = this->sh_state](uint64_t ignored) {});
  return fut_pos;
}

uint64_t dist_ofstream_handle::get_last_known_tellp() const { return sh_state->last_known_tellp; }

//
// dist_ofstream class
//

// FIXME hack to destroy AD when done globally
vector<future<> > dist_ofstream::all_files;
void dist_ofstream::sync_all_files() {
  while (!all_files.empty()) {
    auto fut = all_files.back();
    all_files.pop_back();
    fut.wait();
  }
  upcxx::barrier();
  dist_ofstream_handle::dist_ofstream_handle_state::clear_ad_map();
}

future<> dist_ofstream::close_async() {
  DBG_VERBOSE("\n");

  // do not close twice
  if (is_closed) return close_fut;
  is_closed = true;

  close_fut = flush_batch(false).then([sh_ofsh = this->sh_ofsh]() {
    // keep sh_ofsh in scope until actually closed
    DBG_VERBOSE("calling ofsh->close_file()\n");
    return (*sh_ofsh)->close_file();
  });
  all_files.push_back(close_fut);
  discharge();  // ensure communications have at least started by this rank
  return close_fut;
}

future<> dist_ofstream::flush_batch(bool async) {
  DBG_VERBOSE("flush_batch async=", async, "\n");
  bytes_written += ss.tellp();

  // create a new stringstream, and swap out it for dist_ofstream's member
  auto sh_ss = make_shared<stringstream>();
  ss.swap(*sh_ss);
  assert(ss.tellp() == 0);
  assert(ss.tellg() == 0);

  future<uint64_t> fut_pos;
  if (async) {
    fut_pos = (*sh_ofsh)->append_batch_async(sh_ss);
  } else {
    fut_pos = (*sh_ofsh)->append_batch_collective(sh_ss, block_size);
  }
  return fut_pos.then([](uint64_t ignored) {});
}

dist_ofstream::dist_ofstream(const upcxx::team &myteam, const string ofname, bool append, uint64_t block_size)
    : ss(*((std::stringstream *)this))  // convenience ref to this stringstream
    , sh_ofsh(make_shared<DistOFSHandle>(myteam, ofname, myteam, append))
    , ofsh(*sh_ofsh)
    , block_size(block_size)
    , bytes_written(0)
    , close_fut()
    , is_closed(false) {
  LOG("dist_ofstream(ofname=", ofname, " append=", append, ")\n");
}

dist_ofstream::dist_ofstream(const string ofname, bool append, uint64_t block_size)
    : dist_ofstream(upcxx::world(), ofname, append, block_size) {}

// collective

dist_ofstream::~dist_ofstream() {
  DBG_VERBOSE("Destroying ", ofsh->get_file_name(), "\n");
  if (!is_closed) close();
  assert(is_closed);
  stringstream().swap(ss);
  DBG_VERBOSE("close_fut=", close_fut.ready(), "\n");
}

void dist_ofstream::close() {
  DBG("Closing ", ofsh->get_file_name(), " and waiting\n");
  assert(upcxx::master_persona().active_with_caller());
  auto fut = close_async();
  fut.wait();
  report_timings().wait();
  assert(ofsh->get_pending_ops().ready());
  assert(!ofsh->is_open());
}

future<> dist_ofstream::report_timings() {
  assert(is_closed);
  close_fut.wait();
  return ofsh->report_timings();
}

// returns the total buffered size

uint64_t dist_ofstream::size() { return bytes_written + ss.tellp(); }

string dist_ofstream::str() const { return ss.str(); }

uint64_t dist_ofstream::get_last_known_tellp() const { return ofsh->get_last_known_tellp(); }

future<> dist_ofstream::flush_async() {
  DBG_VERBOSE("\n");
  if (is_closed) DIE("flush_async called on closed dist_ofstream\n");
  return flush_batch(true);
}

future<> dist_ofstream::flush_collective() {
  DBG_VERBOSE("\n");
  if (is_closed) DIE("flush_collective called on closed dist_ofstream\n");
  return flush_batch(false);
}

dist_ofstream &dist_ofstream::flush() {
  DBG_VERBOSE("\n");
  if (is_closed) DIE("flush called on closed dist_ofstream\n");
  flush_async().wait();
  return *this;
}

};  // namespace upcxx_utils
