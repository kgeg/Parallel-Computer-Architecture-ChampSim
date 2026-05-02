#include "stride_ghb.h"

void stride_ghb::prefetcher_cycle_operate() {}  // not used in this design

uint32_t stride_ghb::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch,
                                           champsim::address evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

uint32_t stride_ghb::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit,
                                              bool useful_prefetch, access_type type, uint32_t metadata_in)
{
  if (cache_hit)
    return metadata_in;

  champsim::block_number cl_addr{addr};

  // --- Update GHB and index table (same logic) ---
  int prev_link = -1;
  if (auto it = index_table.find(ip); it != index_table.end()) {
    prev_link = it->second;
  }

  ghb[ghb_head].miss_address = cl_addr;
  ghb[ghb_head].ip = ip;  // Store the IP alongside the miss
  ghb[ghb_head].link_pointer = prev_link;
  index_table[ip] = ghb_head;

  int current = ghb_head;
  ghb_head = (ghb_head + 1) % GHB_SIZE;

  // --- Look for a stable stride with wrap-around protection ---
  int prev = ghb[current].link_pointer;
  
  // Verify prev is valid AND hasn't been overwritten by a different IP
  if (prev != -1 && ghb[prev].ip == ip) {
    auto stride1 = champsim::offset(ghb[prev].miss_address, ghb[current].miss_address);

    int prev_prev = ghb[prev].link_pointer;
    
    // Verify prev_prev is valid AND hasn't been overwritten
    if (prev_prev != -1 && ghb[prev_prev].ip == ip) {
      auto stride2 = champsim::offset(ghb[prev_prev].miss_address, ghb[prev].miss_address);

      if (stride1 == stride2 && stride1 != 0) {
        int degree = 2;  // prefetch two lines ahead
        for (int i = 1; i <= degree; ++i) {
          champsim::block_number pf_block = ghb[current].miss_address + i * stride1;
          champsim::address pf_addr{pf_block};

          // Use the same prefetch_line call as the example
          prefetch_line(pf_addr, true, metadata_in);
        }
      }
    }
  }

  return metadata_in;
}