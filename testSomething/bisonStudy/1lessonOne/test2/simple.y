%{
#include <stdio.h>
int yylex();
void yyerror(const char *s) { printf("错误: %s\n", s); }
%}

%token NUMBER PLUS EOL

%%

program: line
;

line: NUMBER EOL           { printf("你输入的数字是: %d\n", $1); }
    | NUMBER PLUS NUMBER EOL { printf("结果: %d\n", $1 + $3); }  // 新增加法
;

%%

int main() {
    yyparse();
    return 0;
}