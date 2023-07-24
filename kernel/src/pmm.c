#include <common.h>

typedef struct {
  int size;
  int magic;
} alloc_header_t;

typedef struct _free_node_t {
  int size;
  struct _free_node_t * next;
} free_node_t;

static free_node_t * free_head;
static void *kalloc(size_t size) {
  printf("kalloc: %d\n", size);
  return NULL;
}

static void kfree(void *ptr) {
  alloc_header_t * header = (alloc_header_t *)ptr - 1;
  // assert(header->magic = 1234567);
  int size = header->size;
  printf("kfree: %p, size:%d\n", ptr, size);
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
