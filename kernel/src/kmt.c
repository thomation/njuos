#include <os.h>
static void kmt_init() {
    printf("kmt init\n");
}
MODULE_DEF(kmt) = {
  .init  = kmt_init,
};
