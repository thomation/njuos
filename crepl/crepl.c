#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

enum CommandType {
  FUNC,
  EXPR,
};
static char code[1024];
char * MAIN_SRC_PATH = "/tmp/crepl_main.c";
char * MAIN_TARGET_PATH = "/tmp/crepl_main.out";
int parse_line(char * line) {
  if(strlen(line) > 3 && line[0] == 'i' && line[1] == 'n' && line[2] == 't') {
    return FUNC;
  }
  return EXPR;
}
FILE * create_code_file(const char * path) {
  FILE *p = fopen(path, "w+");
  return p;
}
char * parse_func_name(char * line) {
  char * name = malloc(strlen(line));
  char * s = &line[3];
  while(*s == ' ')
    s ++;
  // printf("s=%c\n", *s);
  char * e = s;
  while(*e != ' ' && *e != '(')
    e ++;
  // printf("e=%c\n", *e);
  for(int i = 0; s + i < e; i ++) 
    name[i] = *(s + i);
  name[e - s] = '\0';
  return name;
}
void handle_func(char  * line) {
  printf("Func:%s\n", line);
  char * name = parse_func_name(line);
  printf("Func name:%s\n", name);
  free(name);
}
void create_src(char * line) {
  char * code_temple = "#include<stdio.h>\n int main(){int ret = %s; printf(\"%%d\\n\", ret);return 0;}"; 
  sprintf(code, code_temple, line);
  FILE * p = create_code_file(MAIN_SRC_PATH);
  fprintf(p, "%s\n", code);
  fclose(p);
}
int compile_src() {
  int wstatus = 0;
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  int pid = fork();
  char * main_argv[16] = {"gcc", MAIN_SRC_PATH, "-o", MAIN_TARGET_PATH, NULL};
  if(pid == 0) {
    close(pipefd[0]); // close read
    dup2(pipefd[1], STDERR_FILENO);
    execvp(main_argv[0], main_argv);
  } else {
    close(pipefd[1]); // close write
    wait(&wstatus);
    if(wstatus != 0) {
      char buf;
      while(read(pipefd[0], &buf, 1) > 0)
        write(STDOUT_FILENO, &buf, 1);
    }
  }
  return wstatus;
}
void run() {
  int pid = fork();
  char * main_argv[2] = {MAIN_TARGET_PATH, NULL};
  if(pid == 0) {
    execvp(main_argv[0], main_argv);
  } else {
    int wstatus;
    wait(&wstatus);
  }
}
void handle_expr(char * line) {
  printf("Expr:%s\n", line);
  create_src(line);
  int status = compile_src();
  if(status == 0)
    run();
}
int main(int argc, char *argv[]) {
  static char line[4096];
  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    if(parse_line(line) == FUNC) {
      handle_func(line);
    } else {
      handle_expr(line);
    }
  }
}
