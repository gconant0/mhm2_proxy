/*
 HipMer v 2.0, Copyright (c) 2020, The Regents of the University of California,
 through Lawrence Berkeley National Laboratory (subject to receipt of any required
 approvals from the U.S. Dept. of Energy).  All rights reserved."

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 (1) Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 (2) Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 (3) Neither the name of the University of California, Lawrence Berkeley National
 Laboratory, U.S. Dept. of Energy nor the names of its contributors may be used to
 endorse or promote products derived from this software without specific prior
 written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 DAMAGE.

 You are under no obligation whatsoever to provide any bug fixes, patches, or upgrades
 to the features, functionality or performance of the source code ("Enhancements") to
 anyone; however, if you choose to make your Enhancements available either publicly,
 or directly to Lawrence Berkeley National Laboratory, without imposing a separate
 written license agreement for such Enhancements, then you hereby grant the following
 license: a  non-exclusive, royalty-free perpetual license to install, use, modify,
 prepare derivative works, incorporate into other computer software, distribute, and
 sublicense such enhancements or derivative works thereof, in binary and source code
 form.
*/

#include "contigs.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <string>
#include <upcxx/upcxx.hpp>
#include <vector>

#include "upcxx_utils/log.hpp"
#include "upcxx_utils/ofstream.hpp"

#include "utils.hpp"
#include "zstr.hpp"

using std::endl;
using std::max;
using std::memory_order_relaxed;
using std::pair;
using std::string;
using std::to_string;
using std::vector;

using upcxx::barrier;
using upcxx::dist_object;
using upcxx::op_fast_add;
using upcxx::op_fast_max;
using upcxx::promise;
using upcxx::rank_me;
using upcxx::rank_n;
using upcxx::reduce_one;
using upcxx::rpc;
using upcxx::rpc_ff;

using namespace upcxx_utils;

void Contigs::clear() {
  contigs.clear();
  vector<Contig>().swap(contigs);
}

void Contigs::set_capacity(int64_t sz) { contigs.reserve(sz); }

void Contigs::add_contig(Contig contig) { contigs.push_back(contig); }

size_t Contigs::size() { return contigs.size(); }

void Contigs::print_stats(unsigned min_ctg_len) {
  
  int64_t tot_len = 0, max_len = 0;
  double tot_depth = 0;
  vector<pair<unsigned, uint64_t>> length_sums({{1, 0}, {5, 0}, {10, 0}, {25, 0}, {50, 0}});
  int64_t num_ctgs = 0;
  int64_t num_ns = 0;
  vector<unsigned> lens;
  lens.reserve(contigs.size());
  for (auto ctg : contigs) {
    auto len = ctg.seq.length();
    if (len < min_ctg_len) continue;
    num_ctgs++;
    tot_len += len;
    tot_depth += ctg.depth;
    max_len = max(max_len, static_cast<int64_t>(len));
    for (auto &length_sum : length_sums) {
      if (len >= length_sum.first * 1000) length_sum.second += len;
    }
    num_ns += count(ctg.seq.begin(), ctg.seq.end(), 'N');
    lens.push_back(len);
  }
  /*
  // Compute local N50 and then take the median across all of them. This gives a very good approx of the exact N50 and is much
  // cheaper to compute
  // Actually, the approx is only decent if there are not too many ranks
  sort(lens.rbegin(), lens.rend());
  int64_t sum_lens = 0;
  int64_t n50 = 0;
  for (auto l : lens) {
    sum_lens += l;
    if (sum_lens >= tot_len / 2) {
      n50 = l;
      break;
    }
  }
  barrier();
  dist_object<int64_t> n50_val(n50);
  int64_t median_n50 = 0;
  if (!rank_me()) {
    vector<int64_t> n50_vals;
    n50_vals.push_back(n50);
    for (int i = 1; i < rank_n(); i++) {
      n50_vals.push_back(n50_val.fetch(i).wait());
      //SOUT(i, " n50 fetched ", n50_vals.back(), "\n");
    }
    sort(n50_vals.begin(), n50_vals.end());
    median_n50 = n50_vals[rank_n() / 2];
  }
  // barrier to ensure the other ranks dist_objects don't go out of scope before rank 0 is done
  barrier();
  */
  int64_t all_num_ctgs = reduce_one(num_ctgs, op_fast_add, 0).wait();
  int64_t all_tot_len = reduce_one(tot_len, op_fast_add, 0).wait();
  int64_t all_max_len = reduce_one(max_len, op_fast_max, 0).wait();
  double all_tot_depth = reduce_one(tot_depth, op_fast_add, 0).wait();
  int64_t all_num_ns = reduce_one(num_ns, op_fast_add, 0).wait();
  // int64_t all_n50s = reduce_one(n50, op_fast_add, 0).wait();

  SLOG("Assembly statistics (contig lengths >= ", min_ctg_len, ")\n");
  SLOG("    Number of contigs:       ", all_num_ctgs, "\n");
  SLOG("    Total assembled length:  ", all_tot_len, "\n");
  SLOG("    Average contig depth:    ", all_tot_depth / all_num_ctgs, "\n");
  SLOG("    Number of Ns/100kbp:     ", (double)all_num_ns * 100000.0 / all_tot_len, " (", all_num_ns, ")", KNORM, "\n");
  // SLOG("    Approx N50 (average):    ", all_n50s / rank_n(), " (rank 0 only ", n50, ")\n");
  // SLOG("    Approx N50:              ", median_n50, "\n");
  SLOG("    Max. contig length:      ", all_max_len, "\n");
  SLOG("    Contig lengths:\n");
  for (auto &length_sum : length_sums) {
    SLOG("        > ", std::left, std::setw(19),
         to_string(length_sum.first) + "kbp:", perc_str(reduce_one(length_sum.second, op_fast_add, 0).wait(), all_tot_len), "\n");
  }
}

void Contigs::dump_contigs(const string &fname, unsigned min_ctg_len) {
  
  dist_ofstream of(fname);
  for (auto it = contigs.begin(); it != contigs.end(); ++it) {
    auto ctg = it;
    if (ctg->seq.length() < min_ctg_len) continue;
    of << ">Contig" << to_string(ctg->id) << " " << to_string(ctg->depth) << "\n";
    string rc_uutig = revcomp(ctg->seq);
    string seq = (rc_uutig < ctg->seq ? rc_uutig : ctg->seq);
    // for (int64_t i = 0; i < ctg->seq.length(); i += 50) fasta += ctg->seq.substr(i, 50) + "\n";
    of << ctg->seq << "\n";
  }
  of.close();  // sync and output stats

}


