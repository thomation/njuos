#include "common.h"
#include <inttypes.h>

void mem_read(uintptr_t block_num, uint8_t *buf);
void mem_write(uintptr_t block_num, const uint8_t *buf);

struct cache {
  bool valid;
  uintptr_t tag;
  uint8_t block[BLOCK_SIZE];
};
struct cache_group {
  struct cache * caches;
};
static uint64_t cycle_cnt = 0;
static struct cache_group * cache_groups;
static int block_count_per_group;
static int group_count;

void cycle_increase(int n) { cycle_cnt += n; }

static struct cache * handle_miss(struct cache_group * group, uintptr_t block_num, uintptr_t tag) {
  int replace = -1;
  for(int i = 0; i < block_count_per_group; i ++) {
    struct cache *c = &group->caches[i];
    if(!c->valid) {
      replace = i;
      break;
    }
  }
  if(replace < 0) {
    replace = rand() % block_count_per_group;
  }
  struct cache * c = &group->caches[replace];
  c->tag = tag;
  mem_read(block_num, c->block);
  return c;
}

static struct cache * find_cache(uintptr_t block_num) {
  uint32_t group_index = block_num % group_count;
  uintptr_t tag = block_num / group_count;  
  // printf("find cache: block_num:%lu, group_index:%u, tag:%lu\n", block_num, group_index, tag);
  struct cache_group * group = &cache_groups[group_index];
  for(int i = 0; i < block_count_per_group; i ++) {
    struct cache *c = &group->caches[i];
    if(c->valid && c->tag == tag) {
      return c;
    }
  }
  return handle_miss(group, block_num, tag);
}
uint32_t cache_read(uintptr_t addr) {
  // printf("cache read: old addr:%lu\n", addr);
  addr &= ~0x3;
  // printf("cache read: addr:%lu\n", addr);
  uintptr_t block_num = addr >> BLOCK_WIDTH;
  struct cache * c = find_cache(block_num);
  int block_offset = addr & mask_with_len(BLOCK_WIDTH);
  // for(int i = 0; i < BLOCK_SIZE; i ++) {
  //   printf("%x", c->block[i]);
  // }
  // printf("\n");
  uint32_t * p = (void *)c->block + block_offset;
  // printf("cache read: block_offset:%d, ret:%x\n", block_offset, *p);
  return *p;
}

void cache_write(uintptr_t addr, uint32_t data, uint32_t wmask) {
  // printf("cache write: old addr:%lu\n", addr);
  addr &= ~0x3;
  // printf("cache write: addr:%lu, data:%x, wmask:%x \n", addr, data, wmask);
  uintptr_t block_num = addr >> BLOCK_WIDTH;
  struct cache * c = find_cache(block_num);
  int block_offset = addr & mask_with_len(BLOCK_WIDTH);
  // printf("cache write: block_offset: %d\n", block_offset);
  uint32_t *p = (void *)c->block + block_offset;
  *p = (*p & ~wmask) | (data & wmask);
  // for(int i = 0; i < BLOCK_SIZE; i ++) {
  //   printf("%x", c->block[i]);
  // }
  // printf("\n");
  // printf("cache write: %x\n", *p);
  mem_write(block_num, c->block);
}

void init_cache(int total_size_width, int associativity_width) {
  block_count_per_group = 1 << associativity_width;
  group_count = (1 << total_size_width) / BLOCK_SIZE / block_count_per_group;
  printf("init_cache: block_count_per_group:%d, group_count:%d\n", block_count_per_group, group_count);
  cache_groups = malloc(sizeof(struct cache_group) * group_count);
  for(int g = 0; g < group_count; g ++) {
    cache_groups[g].caches = malloc(sizeof(struct cache) * block_count_per_group);
    for(int i = 0; i < block_count_per_group; i ++) {
      cache_groups[g].caches[i].valid = false;
    }
  }
}

void display_statistic(void) {
}
