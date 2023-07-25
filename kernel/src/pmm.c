#include <common.h>
#define ALLOC_MAGIC_NUM 1234567
#define REAL_ALLOC_SIZE(size) (size + sizeof(alloc_header_t))
#define REAL_FREE_SIZE(size) (size + sizeof(free_node_t))
typedef struct {
  size_t size;
  int magic;
} alloc_header_t;

typedef struct _free_node_t {
  size_t size;
  struct _free_node_t * next;
} free_node_t;

static free_node_t * free_head;
static void print_free_list() {
  for(free_node_t * p = free_head; p; p = p->next) {
    printf("free_node %p, size:%d\n", p, p->size);
  }
}
static free_node_t * find_free_node(size_t size) {
  for(free_node_t *p = free_head; p; p = p->next) {
    if(REAL_FREE_SIZE(p->size) >= REAL_ALLOC_SIZE(size))
      return p;
  }
  return NULL;
}
static void remove_free_node(free_node_t * free, size_t size) {
  assert(free);
  free_node_t * next;
  if(free->size <= REAL_ALLOC_SIZE(size)) {
    next = free->next;
  } else {
    next = (free_node_t *)((uintptr_t)free + REAL_ALLOC_SIZE(size));
    next->size = free->size - REAL_ALLOC_SIZE(size);
  }
  printf("remove_free_node: next %p\n", next);
  if(free_head == free) {
    free_head = next;
  } else {
    free_node_t * pre = free_head;
    while(pre->next != free) {
      pre = pre->next;
    }
    assert(pre);
    pre->next = next;
  }
}
static void *kalloc(size_t size) {
  printf("kalloc: %d\n", size);
  free_node_t * free = find_free_node(size);
  if(!free) {
    printf("kalloc: no enough space for size %u\n", size);
    return NULL;
  }
  remove_free_node(free, size);
  alloc_header_t * header = (alloc_header_t *) free;
  header->size = size;
  header->magic = ALLOC_MAGIC_NUM;
  printf("kalloc: header %p\n", header);
  print_free_list();
  return header + 1;
}

static void kfree(void *ptr) {
  alloc_header_t * header = (alloc_header_t *)ptr - 1;
  assert(header->magic = ALLOC_MAGIC_NUM);
  int size = header->size;
  printf("kfree: %p, size:%d\n", ptr, size);
  print_free_list();
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("pmm_init %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  free_head = (free_node_t *)heap.start;
  free_head->size = pmsize - sizeof(free_node_t);
  free_head->next = NULL;
  printf("pmm_init free_head: %p, free_size:%d\n", free_head, free_head->size);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
