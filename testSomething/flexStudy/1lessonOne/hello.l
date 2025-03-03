%option noyywrap


/* 定义部分 */
%{
#include <stdio.h>
%}

/* 规则部分 */
%%
"hello"     { printf("你好！\n"); }  /* 匹配 "hello" 并输出中文 */
.           { /* 忽略其他字符 */ }  /* . 表示任意单个字符 */
%%

/* C 代码部分 */
int main() {
    yylex();  /* Flex 自动生成的词法分析函数 */
    return 0;
}