%{
#include <stdio.h>
#include "calc.tab.h"  /* Bison 生成的头文件 */
%}
%option noyywrap
%option yylineno

%%
[0-9]+      { yylval = atoi(yytext); return NUMBER; }  /* 将数字转为整数 */
"+"         { return ADD; }
"^"         { return POW; }
"-"         { return SUB; }
"*"         { return MUL; }
"/"         { return DIV; }
"("         { return LPAREN; }
")"         { return RPAREN; }
\n          { printf("返回 EOL\n"); return EOL; }
[ \t]       { /* 忽略空白 */ }
.           { printf("无效字符: %c\n", yytext[0]); return -1; }
%%

int yyerror(char *s) {
    fprintf(stderr, "错误: %s 在行 %d\n", s, yylineno);
    return 0;
}