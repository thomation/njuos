#include <stdio.h>
#include <string.h>

enum CommandType {
  FUNC,
  EXPR,
};
int parse_line(char * line) {
  if(strlen(line) > 3 && line[0] == 'i' && line[1] == 'n' && line[2] == 't') {
    return FUNC;
  }
  return EXPR;
}
void handle_func(char  * line) {
  printf("Func:%s\n", line);
}
void handle_expr(char * line) {
  printf("Expr:%s\n", line);
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
