#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  char **exec_argv = malloc(sizeof(char*) * (argc + 1));
  exec_argv[0] = "strace";
  for(int i = 1; i < argc; i ++) {
    exec_argv[i] = malloc(sizeof(char) * (strlen(argv[i]) + 1));
    strcpy(exec_argv[i], argv[i]);
  }
  exec_argv[argc] = NULL;
  // for(int i = 0; exec_argv[i]; i++) {
  //   printf("%s\n", exec_argv[i]);
  // }
  char *exec_envp[] = { "PATH=/bin", NULL, };
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  pid_t pid = fork();
  if(pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  } else if(pid == 0) {
    close(pipefd[0]); // close read
    dup2(pipefd[1], STDERR_FILENO);
    execve("strace",          exec_argv, exec_envp);
    execve("/bin/strace",     exec_argv, exec_envp);
    execve("/usr/bin/strace", exec_argv, exec_envp);
  } else {
    close(pipefd[1]); // close write
    char buf;
    while(read(pipefd[0], &buf, 1) > 0) {
      // write(STDOUT_FILENO, &buf, 1);
    }
    wait(NULL);
    printf("finished\n");
  }
}
