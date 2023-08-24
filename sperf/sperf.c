#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>

#define RESERVE_ARGC 1
#define NAME_LEN 32
#define TIME_LEN 32
#define NODE_COUNT 128

typedef struct strace_node {
  char name[NAME_LEN];
  float time;
} strace_node_t;

static strace_node_t nodes[NODE_COUNT];
static int lines_count;
static int find_index_in_line(char * line, char c) {
  size_t n = strlen(line);
  for(int i = 0; i < n; i ++) {
    if(line[i] == c)
      return i;
  }
  return -1;
}
static strace_node_t * find_node_with_name(char * name) {
  for(int i = 0; i < lines_count; i ++) {
    if(strcmp(nodes[i].name, name) == 0) {
      return &nodes[i];
    }
  }
  return NULL;
}
static void add_node(char * name, float time) {
  assert(lines_count < NODE_COUNT);
  strace_node_t * node = &nodes[lines_count ++];
  strcpy(node->name, name);
  node->time = time;
}
static void parse_line(char * line) {
  // printf("%s\n", line);
  int name_end = find_index_in_line(line, '(');
  if(name_end < 0) {
    return;
  }
  char name[NAME_LEN];
  assert(name_end <= NAME_LEN);
  for(int i = 0; i < name_end; i ++)
    name[i] = line[i];
  name[name_end] = '\0';
  // printf("%s\n", name);
  int time_start = find_index_in_line(line, '<');
  int time_end = find_index_in_line(line, '>');
  float time = 0.0;
  if(time_start >= 0 && time_end >= 0) {
    time_start ++;
    char time_str[TIME_LEN];
    int time_index = 0;
    for(int i = time_start; i < time_end; i ++) {
      time_str[time_index ++] = line[i];
    }
    time_str[time_index] = '\0';
    sscanf(time_str, "%f", &time);
  }
  // printf("%f\n", time);
  strace_node_t * node = find_node_with_name(name);
  if(node) {
    node->time += time;
  } else {
    add_node(name, time);
  }
}
static void print_result() {
  for(int i = 0; i < lines_count; i ++) {
    strace_node_t * node = &nodes[i];
    printf("%s:%f\n", node->name, node->time);
  }
}
int main(int argc, char *argv[]) {
  char **exec_argv = malloc(sizeof(char*) * (argc + RESERVE_ARGC + 1));
  exec_argv[0] = "strace";
  exec_argv[1] = "-T";
  for(int i = 1; i < argc; i ++) {
    int exec_index = i + RESERVE_ARGC;
    exec_argv[exec_index] = malloc(sizeof(char) * (strlen(argv[i]) + 1));
    strcpy(exec_argv[exec_index], argv[i]);
  }
  exec_argv[argc + RESERVE_ARGC] = NULL;
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
    char line[512];
    int i = 0;
    while(read(pipefd[0], &buf, 1) > 0) {
      // write(STDOUT_FILENO, &buf, 1);
      if(buf != '\n') {
        line[i ++ ] = buf;
      } else {
        line[i] = '\0';
        parse_line(line);
        i = 0;
      }
    }
    wait(NULL);
    printf("finished\n");
    print_result();
  }
}
