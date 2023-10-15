#include <os.h>
#include <syscall.h>

#include "initcode.inc"
static void * vme_alloc(int size) {
    return pmm->alloc(size);
}
static void vme_free(void * content) {
    pmm->free(content);
}
static void uproc_init() {
  printf("uproc_init \n");
  vme_init(vme_alloc, vme_free);
}
int kputc(task_t *task, char ch) {
  putch(ch); // safe for qemu even if not lock-protected
  return 0;
}

MODULE_DEF(uproc) = {
  .init = uproc_init,
  .kputc = kputc,
};
