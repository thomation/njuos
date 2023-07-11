#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"
#include "thread-sync.h"

#define MAXN 10000
int T, N, M;
char A[MAXN + 1], B[MAXN + 1];
int dp[MAXN][MAXN];
int result;
volatile int current_round = -1;
int max_round;
volatile int finished_count = 0;
mutex_t lock = MUTEX_INIT();
volatile int compute_count = 0;

#define DP(x, y) (((x) >= 0 && (y) >= 0) ? dp[x][y] : 0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MAX3(x, y, z) MAX(MAX(x, y), z)

void Tworker(int id) {
  int my_round = -1;
  while(current_round < max_round) {
    if(my_round == current_round)
      continue;
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
        // mutex_lock(&lock);
        // compute_count ++;
        // mutex_unlock(&lock);
      }
    }
    mutex_lock(&lock);
    finished_count ++;
    // printf("finished count:%d, my:%d, current:%d\n", finished_count, my_round, current_round);
    mutex_unlock(&lock);
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
      compute_count ++;
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
  if(T == 1) {
    printf("Single Thread Version\n");
    raw_worker();
    printf("Comput count: %d\n", compute_count);
  } else {
    printf("Multiple Threads Version\n");
    max_round = N * 2 - 1;
    for (int i = 0; i < T; i++) {
      create(Tworker);
    }
    mutex_lock(&lock);
    current_round = 0;
    finished_count = 0;
    mutex_unlock(&lock);
    while(current_round < max_round) {
      while(finished_count < T) {
        // printf("%d\n", finished_count);
      }
      mutex_lock(&lock);
      current_round ++;
      finished_count = 0;
      mutex_unlock(&lock);
    }

    join();  // Wait for all workers
    printf("Compute count:%d\n", compute_count);
  }
  printf("%d\n", result);
}
