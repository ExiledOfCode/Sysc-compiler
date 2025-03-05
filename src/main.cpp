#include "head/ast.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
using namespace std;
/*
cmake --build build --parallel 4   # 增量构建
./build/compiler                   # 运行程序
*/

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
void test(unique_ptr<BaseAST> &ast, const char *output, int flag) {
    // 将 Dump 的输出重定向到文件
    // 打开输出文件
    if (!flag) {
        ast->Dump();
        return;
    }

    std::ofstream out_file(output);
    if (!out_file.is_open()) {
        cerr << "Error: Cannot open output file " << output << endl;
        return;
    }

    std::streambuf *coutbuf = cout.rdbuf(); // 保存原始 cout 缓冲区
    std::cout.rdbuf(out_file.rdbuf());      // 重定向 cout 到文件
    ast->Dump();                            // 输出 Koopa IR
    std::cout.rdbuf(coutbuf);               // 恢复 cout
    out_file.close();                       // 关闭文件
}
int main(int argc, const char *argv[]) {
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);

    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
    assert(yyin);

    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);

    test(ast, output, 1);

    return 0;
}
