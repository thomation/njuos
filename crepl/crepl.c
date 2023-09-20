#define _GNU_SOURCE
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
char * LIB_SRC_PATH_TEMP = "/tmp/crepl_lib_%s.c";
char * LIB_TARGET_PATH = "/tmp/libcrepl_lab.so";

#define FUNC_COUNT 32
static char * funcs[FUNC_COUNT];
static int func_count = 0;
int parse_line(char * line) {
  if(strlen(line) > 3 && line[0] == 'i' && line[1] == 'n' && line[2] == 't') {
    return FUNC;
  }
  return EXPR;
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
FILE * create_code_file(const char * path) {
  FILE *p = fopen(path, "w+");
  return p;
}
void create_src(char * src_path, char * line, char * code_template) {
  sprintf(code, code_template, line);
  FILE * p = create_code_file(src_path);
  fprintf(p, "%s\n", code);
  fclose(p);
}
int compile_src(char ** compile_cmd) {
  int wstatus = 0;
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  int pid = fork();
  if(pid == 0) {
    close(pipefd[0]); // close read
    dup2(pipefd[1], STDERR_FILENO);
    execvp(compile_cmd[0], compile_cmd);
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
char * generate_func_file_name(char * func_name) {
  char * file_name = malloc(64);
  sprintf(file_name, LIB_SRC_PATH_TEMP, func_name);
  return file_name;
}
void handle_func(char  * line) {
  printf("Func:%s\n", line);
  char * name = parse_func_name(line);
  printf("Func name:%s\n", name);
  for(int i = 0; i < func_count; i ++) {
    if(strcmp(funcs[i], name) == 0) {
      printf("Dup name:%s\n", name);
      free(name);
      return;
    }
  }
  funcs[func_count ++] = name;
  char * code_template = "%s\n";
  char * file_name = generate_func_file_name(name);
  create_src(file_name, line, code_template);
  char * main_argv[FUNC_COUNT + 10] = {"gcc"};
  for(int i = 0; i < func_count; i ++) {
    main_argv[i + 1] = generate_func_file_name(funcs[i]);
  }
  char * others[10] = {"-fPIC", "-shared", "-o", LIB_TARGET_PATH, NULL};
  for(int i = 0; i < 10; i ++) {
    main_argv[i + func_count + 1] = others[i];
  }
  for(int i = 0; main_argv[i] != NULL; i ++)
    printf("main_argv %d:%s\n", i, main_argv[i]);
  compile_src(main_argv);
  // TODO: free file name
}
void run() {
  int pid = fork();
  char * main_argv[2] = {MAIN_TARGET_PATH, NULL};
  if(pid == 0) {
    char * env[2] = {"LD_LIBRARY_PATH=/tmp", NULL};
    execvpe(main_argv[0], main_argv, env);
  } else {
    int wstatus;
    wait(&wstatus);
  }
}
void handle_expr(char * line) {
  printf("Expr:%s\n", line);
  char * code_temple = "#include<stdio.h>\n int main(){int ret = %s; printf(\"%%d\\n\", ret);return 0;}"; 
  create_src(MAIN_SRC_PATH, line, code_temple);
  char * main_argv[16] = {"gcc", MAIN_SRC_PATH, "-L/tmp", "-lcrepl_lab", "-o", MAIN_TARGET_PATH, NULL};
  int status = compile_src(main_argv);
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
