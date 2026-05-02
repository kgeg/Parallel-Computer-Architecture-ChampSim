#ifndef STRIDE_GHB_H
#define STRIDE_GHB_H

#include <cstdint>
#include <map>
#include <vector>

#include "address.h"
#include "champsim.h"
#include "modules.h"

class stride_ghb : public champsim::modules::prefetcher
{
public:
  using champsim::modules::prefetcher::prefetcher;  // inherit constructor

  // mandatory interfaces
  uint32_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit,
                                    bool useful_prefetch, access_type type, uint32_t metadata_in);
  uint32_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch,
                                 champsim::address evicted_addr, uint32_t metadata_in);
  void prefetcher_cycle_operate();

private:
  static constexpr std::size_t GHB_SIZE = 512;

  struct GHB_Entry {
    champsim::block_number miss_address;
    champsim::address ip;  // Store IP to verify ownership on wrap-around
    int link_pointer = -1;
  };

  std::vector<GHB_Entry> ghb{GHB_SIZE};
  int ghb_head = 0;
  std::map<champsim::address, int> index_table;  // IP -> GHB index
};

#endif