%{
#include <stdio.h>
%}
%option noyywrap
%option yylineno
%option case-insensitive
%x COMMENT

KEYWORD if|else|while|for|int|return

%%

"//"            { BEGIN(COMMENT); printf("注释开始: "); }
<COMMENT>.|\n   { printf("%c", yytext[0]); if (yytext[0] == '\n') BEGIN(INITIAL); }
{KEYWORD}       { printf("关键字: %s\n", yytext); }
[0-9]+          { printf("数字: %s\n", yytext); }
[a-zA-Z_][a-zA-Z0-9_]*  { printf("标识符: %s\n", yytext); }
[ \t\n]         { /* 忽略空白 */ }
.               { printf("符号: %c\n", yytext[0]); }

%%

int main() {
    yylex();
    return 0;
}