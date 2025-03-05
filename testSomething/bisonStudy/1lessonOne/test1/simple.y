%{
#include <stdio.h>
int yylex();                  // Flex 提供的词法分析函数
void yyerror(const char *s) { printf("错误: %s\n", s); }
%}

%token NUMBER EOL  // 定义 Token 类型，和 Flex 对应

%%

program: /* 空 */ | program line;     // 程序由一行组成，这是一个递归定义，表示program可以由program line组成，也就是program后面可以追加line
;

line: NUMBER EOL { printf("你输入的数字是: %d\n", $1); }  // 打印数字
;

%%

int main() {
    yyparse();  // 开始解析
    return 0;
}