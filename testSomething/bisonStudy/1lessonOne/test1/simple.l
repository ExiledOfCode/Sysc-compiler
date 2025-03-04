%{
#include "simple.tab.h"  // Bison 自动生成的头文件
%}

%%

[0-9]+    { yylval = atoi(yytext); return NUMBER; }  // 返回数字
\n        { return EOL; }                            // 返回换行
[ \t]     { /* 忽略空白 */ }                        // 忽略空格
.         { printf("未知字符: %s\n", yytext); }     // 其他字符报错

%%

int yywrap() { return 1; }