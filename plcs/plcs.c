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

#define DP(x, y) (((x) >= 0 && (y) >= 0) ? dp[x][y] : 0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MAX3(x, y, z) MAX(MAX(x, y), z)

void Tworker(int id) {
  if (id != 1) {
    // This is a serial implementation
    // Only one worker needs to be activated
    return;
  }

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
  T = atoi(argv[1]);
  if(T == 1) {
    printf("Single Thread Version\n");
    raw_worker();
  } else {
    printf("Multiple Threads Version\n");
    // Add preprocessing code here

    for (int i = 0; i < T; i++) {
      create(Tworker);
    }
    join();  // Wait for all workers
  }
  printf("%d\n", result);
}
