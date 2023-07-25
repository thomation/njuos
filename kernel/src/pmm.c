#include <common.h>
#define ALLOC_MAGIC_NUM 1234567
#define ALLOC_SIZE_WITH_HEADER(size) (size + sizeof(alloc_header_t))
#define ALLOC_PAYLOAD_SIZE(size) (size - sizeof(alloc_header_t))
#define FREE_SIZE_WITH_HEADER(size) (size + sizeof(free_node_t))
#define FREE_PAYLOAD_SIZE(size) (size - sizeof(free_node_t))

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
  printf("free_node>>>>>>>>>>>>>>>>>>>>>>>>\n");
  int i = 0;
  for(free_node_t * p = free_head; p; p = p->next) {
    printf("(%p,%d)->", p, p->size);
    if(++i % 10 == 0)
      printf("\n");
  }
  printf("\nfree_node<<<<<<<<<<<<<<<<<<<<<<<<\n");
}
static free_node_t * find_free_node(size_t size) {
  for(free_node_t *p = free_head; p; p = p->next) {
    if(FREE_SIZE_WITH_HEADER(p->size) >= ALLOC_SIZE_WITH_HEADER(size))
      return p;
  }
  return NULL;
}
static size_t remove_free_node(free_node_t * free, size_t size) {
  assert(free);
  size_t realsize = size;
  free_node_t * next;
  // Cannot split free node because the free header needs extra space
  if(free->size <= ALLOC_SIZE_WITH_HEADER(size)) {
    next = free->next;
    realsize = ALLOC_PAYLOAD_SIZE(FREE_SIZE_WITH_HEADER(free->size));
  } else {
    next = (free_node_t *)((uintptr_t)free + ALLOC_SIZE_WITH_HEADER(size));
    next->size = free->size - ALLOC_SIZE_WITH_HEADER(size);
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
  return realsize;
}
static void *kalloc(size_t size) {
  printf("kalloc: %d\n", size);
  free_node_t * free = find_free_node(size);
  if(!free) {
    printf("kalloc: no enough space for size %u\n", size);
    return NULL;
  }
  size_t realsize = remove_free_node(free, size);
  alloc_header_t * header = (alloc_header_t *) free;
  header->size = realsize;
  header->magic = ALLOC_MAGIC_NUM;
  printf("kalloc: header %p\n", header);
  print_free_list();
  return header + 1;
}
static void add_free_node(alloc_header_t * alloc) {
  size_t size = alloc->size;
  free_node_t * to_free = (free_node_t *) alloc;
  to_free->next = free_head;
  to_free->size = FREE_PAYLOAD_SIZE(ALLOC_SIZE_WITH_HEADER(size));
  free_head = to_free;
}
static void kfree(void *ptr) {
  alloc_header_t * header = (alloc_header_t *)ptr - 1;
  assert(header->magic = ALLOC_MAGIC_NUM);
  printf("kfree: %p, size:%d\n", ptr, header->size);
  add_free_node(header);
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
