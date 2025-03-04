%{
#include <stdio.h>
int yylex();
void yyerror(const char *s) { printf("错误: %s\n", s); }
%}

%token NUMBER PLUS EOL

%%

program: line
;

line: exp EOL { printf("结果: %d\n", $1); }
;

exp: NUMBER         { $$ = $1; }           // 单个数字
   | exp PLUS NUMBER { $$ = $1 + $3; }     // 表达式加数字
;

%%

int main() {
    yyparse();
    return 0;
}