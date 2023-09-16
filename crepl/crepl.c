#include <stdio.h>
#include <string.h>

enum CommandType {
  FUNC,
  EXPR,
};
static char code[1024];
const char * MAIN_PATH = "/tmp/crepl_main.c";
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
void handle_func(char  * line) {
  printf("Func:%s\n", line);
}
void handle_expr(char * line) {
  printf("Expr:%s\n", line);
  char * code_temple = "#include<stdio.h>\n int main(){int ret = %s; printf(\"%%d\\n\", ret);return 0;}"; 
  sprintf(code, code_temple, line);
  FILE * p = create_code_file(MAIN_PATH);
  fprintf(p, "%s\n", code);
  fclose(p);
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
