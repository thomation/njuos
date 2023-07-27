#include<common.h>

void init_lock(lock_t *lock) {
    lock->flag = 0;
}
void lock(lock_t *lock) {
    while(atomic_xchg(&lock->flag, 1) == 1)
        // spin wait
        ;
}
void unlock(lock_t *lock) {
    lock->flag = 0;
}