%{
#include <math.c>
#include <stdio.h>
extern int yylex();
extern int yyerror(char *s);
%}
/*
flex calc.l
bison -d calc.y
gcc lex.yy.c calc.tab.c -o calc
./calc

*/
/* 定义 Token */
%token NUMBER ADD SUB MUL DIV LPAREN RPAREN EOL

/* 指定运算符优先级和结合性 */
%left ADD SUB   /* 加减优先级较低，左结合 */
%left MUL DIV   /* 乘除优先级较高，左结合 */
%token POW
%left POW  /* 最高优先级 */

%%
/* 语法规则 */
program:
    |program expression EOL     { printf("结果: %d\n", $2); }
    ;

expression:
    expression ADD expression   { $$ = $1 + $3; }
    | expression SUB expression { $$ = $1 - $3; }
    | expression MUL expression { $$ = $1 * $3; }
    | expression POW expression { $$ = (int)pow($1, $3); }  /* 需要 #include <math.h> 和 -lm */
    | expression DIV expression { if ($3 == 0) { yyerror("除以零"); $$ = 0; } else { $$ = $1 / $3; } }
    | LPAREN expression RPAREN  { $$ = $2; }
    | NUMBER                    { $$ = $1; }
    ;
%%

int main() {
    yyparse();  /* Bison 自动生成的语法分析函数 */
    return 0;
}