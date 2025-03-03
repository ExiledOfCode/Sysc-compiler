%option noyywrap
/*
快速执行：

flex simple.l
gcc lex.yy.c -o simple
./simple < text.txt

*/

%{
#include <stdio.h>
#define NUMBER 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
%}

%%

[0-9]+      { return NUMBER; }
"+"         { return ADD; }
"-"         { return SUB; }
"*"         { return MUL; }
"/"         { return DIV; }
[ \t\n]     { /* 忽略 */ }
.           { printf("无效字符: %c\n", yytext[0]); return -1; }
%%

int main() {
    int token;
    while((token=yylex())!=0){
        switch(token){
            case NUMBER: printf("%s ",yytext);break;
            case ADD:    printf("%s ",yytext);break;
            case SUB:    printf("%s ",yytext);break;
            case MUL:    printf("%s ",yytext);break;
            case DIV:    printf("%s ",yytext);break;
            default: printf("error\n");
        }
    }
    printf("\n");
    return 0;
}