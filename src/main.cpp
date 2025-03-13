#include "head/ast.hpp"
#include "head/exp.hpp"
#include "head/koopa.h"
#include "head/koopa_to_riscv.hpp"
#include "head/stmt.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
using namespace std;
/*
cmake --build build --parallel 4   # 增量构建
./build/compiler                   # 运行程序

*/

/*
    使用全局变量控制输出流，如果出线了return，则不输出作用域内的后续语句
    每个%开头的标识符都是一个作用域块
*/
bool has_returned = 0; // 全局控制输出流标记
int TemValId = 0;      // 全局变量编号（临时，局部，全局）
int block_counter = 0;
SymbolTable symTab;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
void getIR(std::unique_ptr<BaseAST> &ast, const char *output_file) {
    ofstream out_file(output_file);
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out_file.rdbuf());
    ast->Dump();
    std::cout.rdbuf(coutbuf);
}
void getRiscv(std::unique_ptr<BaseAST> &ast, const char *output_file) {
    // 第一步：生成 Koopa IR 到字符串
    std::ostringstream oss;
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(oss.rdbuf());
    ast->Dump();
    std::cout.rdbuf(coutbuf);
    std::string koopa_ir_str = oss.str();

    // 第二步：转换为内存形式的 Koopa IR
    koopa_program_t program;
    koopa_error_code_t ret =
        koopa_parse_from_string(koopa_ir_str.c_str(), &program);
    if (ret != KOOPA_EC_SUCCESS) {
        std::cerr << "Error: Failed to parse Koopa IR" << std::endl;
        return;
    }

    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    // 第三步：生成 RISC-V 汇编并输出到文件
    std::ofstream out_file(output_file);
    if (!out_file.is_open()) {
        std::cerr << "Error: Cannot open output file " << output_file
                  << std::endl;
        koopa_delete_raw_program_builder(builder);
        return;
    }
    generate_riscv(raw, out_file);
    out_file.close();

    // 第四步：清理内存
    koopa_delete_raw_program_builder(builder);
}

int main(int argc, const char *argv[]) {
    // 检查命令行参数
    assert(argc == 5);

    const char *input_file = argv[2];
    const char *output_file = argv[4];

    // 打开输入文件
    yyin = fopen(input_file, "r");
    if (!yyin) {
        std::cerr << "Error: Cannot open input file " << input_file
                  << std::endl;
        return 1;
    }

    // 解析 SysY 源文件生成 AST
    std::unique_ptr<BaseAST> ast;
    int parse_ret = yyparse(ast);
    fclose(yyin);
    if (parse_ret != 0) {
        std::cerr << "Error: Parsing failed" << std::endl;
        return 1;
    }

    if (strcmp(argv[1], "-koopa") == 0) {
        getIR(ast, output_file);
    } else if (strcmp(argv[1], "-riscv") == 0) {
        getRiscv(ast, output_file);
    } else {
        std::cerr << "Error: 不正确的指令" << std::endl;
    }

    return 0;
}
