%{
#include "simple.tab.h"
%}

%%
[0-9]+    { yylval = atoi(yytext); return NUMBER; }
"+"       { return PLUS; }                          // 新增加号
\n        { return EOL; }
[ \t]     { /* 忽略空白 */ }
.         { printf("未知字符: %s\n", yytext); }
%%

int yywrap() { return 1; }