#ifndef __lock_h__
#define __lock_h__
typedef struct _lock_t {
  int flag;
} lock_t;
void init_lock(lock_t *lock); 
void lock(lock_t *lock); 
void unlock(lock_t *lock);
#endif