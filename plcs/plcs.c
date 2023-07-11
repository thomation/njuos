#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"
#include "thread-sync.h"

#define MAXN 10000
#define MAX_T 16
int T, N, M;
char A[MAXN + 1], B[MAXN + 1];
int dp[MAXN][MAXN];
int result;
volatile int current_round = -1;
int max_round;
mutex_t finished_lock[MAX_T] = {MUTEX_INIT()};
cond_t finished_cond[MAX_T] = {COND_INIT()};
int finished[MAX_T] = {0};
mutex_t round_lock = MUTEX_INIT();
cond_t round_cond = COND_INIT();


#define DP(x, y) (((x) >= 0 && (y) >= 0) ? dp[x][y] : 0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MAX3(x, y, z) MAX(MAX(x, y), z)

void Tworker(int id) {
  int my_round = -1;
  while(current_round < max_round) {
    mutex_lock(&round_lock);
    if(my_round == current_round)
      cond_wait(&round_cond, &round_lock);
    mutex_unlock(&round_lock);
    my_round = current_round;
    // find the tasks for me of current round
    int task_count = my_round < N ? my_round + 1 : 2 * N - 1 - my_round;
    int task_count_per_thread =  task_count / T;
    if(task_count_per_thread * T < task_count)
      task_count_per_thread ++;
    // handle tasks for me
    for(int t = 0; t < task_count_per_thread; t ++) {
      int task = (id - 1) * task_count_per_thread + t;
      if(task < task_count) {
        // printf("id:%d, compute round %d, task %d of %d\n", id, my_round, task, task_count);
        int i, j;
        if(my_round < N) {
          i = my_round - task;
          j = task;
        } else {
          i = N - 1 - task;
          j = my_round + 1 - N + task;
        }
        int skip_a = DP(i - 1, j);
        int skip_b = DP(i, j - 1);
        int take_both = DP(i - 1, j - 1) + (A[i] == B[j]);
        dp[i][j] = MAX3(skip_a, skip_b, take_both);
      }
    }
    mutex_lock(&finished_lock[id - 1]);
    finished[id - 1] = 1;
    cond_signal(&finished_cond[id - 1]);
    // printf("id %d, finished count:%d, my:%d, current:%d\n", id, finished_count, my_round, current_round);
    mutex_unlock(&finished_lock[id - 1]);
  }
}

void raw_worker() {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      // Always try to make DP code more readable
      int skip_a = DP(i - 1, j);
      int skip_b = DP(i, j - 1);
      int take_both = DP(i - 1, j - 1) + (A[i] == B[j]);
      dp[i][j] = MAX3(skip_a, skip_b, take_both);
    }
  }

  result = dp[N - 1][M - 1];
}
int main(int argc, char *argv[]) {
  if(argc != 4) {
    printf("Usage: plcs threads count a b, argc = %d\n", argc);
    exit(1);
  }
  strcpy(A, argv[2]);
  strcpy(B, argv[3]);
  N = strlen(A);
  M = strlen(B);
  assert(M == N);
  T = atoi(argv[1]);
  assert(T < MAX_T);
  if(T == 0) {
    printf("Single Thread Version\n");
    raw_worker();
  } else {
    printf("Multiple Threads Version\n");
    max_round = N * 2 - 1;
    for (int i = 0; i < T; i++) {
      create(Tworker);
    }
    mutex_lock(&round_lock);
    current_round = 0;
    cond_broadcast(&round_cond);
    mutex_unlock(&round_lock);
    while(current_round < max_round) {
        for(int i = 0; i < T; i ++) {
          mutex_lock(&finished_lock[i]);
          while(!finished[i])
            cond_wait(&finished_cond[i], &finished_lock[i]);
          finished[i] = 0;
          mutex_unlock(&finished_lock[i]);
        }
      mutex_lock(&round_lock);
      current_round ++;
      cond_broadcast(&round_cond);
      mutex_unlock(&round_lock);
    }

    join();  // Wait for all workers
    result = dp[N - 1][M - 1];
  }
  printf("%d\n", result);
}
