#include "koopa_to_riscv.hpp"

using namespace std;

// 访问 raw program
void generate_riscv(const koopa_raw_program_t &program, std::ostream &out) {
    // 输出代码段声明
    out << "  .text\n";
    // 遍历全局函数
    for (size_t i = 0; i < program.funcs.len; ++i) {
        assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func =
            (koopa_raw_function_t)program.funcs.buffer[i];
        generate_riscv(func, out);
    }
}

// 访问函数
void generate_riscv(const koopa_raw_function_t &func, std::ostream &out) {
    // 输出全局符号和函数入口标签
    out << "  .globl " << (func->name + 1) << "\n"; // 跳过 '@' 前缀
    out << (func->name + 1) << ":\n";               // 函数名标签，例如 "main:"
    // 遍历基本块
    for (size_t i = 0; i < func->bbs.len; ++i) {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb =
            (koopa_raw_basic_block_t)func->bbs.buffer[i];
        generate_riscv(bb, out);
    }
}

// 访问基本块
void generate_riscv(const koopa_raw_basic_block_t &bb, std::ostream &out) {
    // 遍历指令
    for (size_t i = 0; i < bb->insts.len; ++i) {
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t value = (koopa_raw_value_t)bb->insts.buffer[i];
        generate_riscv(value, out);
    }
}

// 访问指令
void generate_riscv(const koopa_raw_value_t &value, std::ostream &out) {
    switch (value->kind.tag) {
    case KOOPA_RVT_RETURN:
        generate_riscv(value->kind.data.ret, out);
        break;
    default:
        assert(false); // 暂时只支持 return 指令
    }
}

// 访问 return 指令
void generate_riscv(const koopa_raw_return_t &ret, std::ostream &out) {
    koopa_raw_value_t ret_value = ret.value;
    assert(ret_value->kind.tag == KOOPA_RVT_INTEGER); // 假设返回值是整数
    generate_riscv(ret_value->kind.data.integer, out);
    out << "  ret\n"; // 输出返回指令
}

// 访问 integer
void generate_riscv(const koopa_raw_integer_t &integer, std::ostream &out) {
    out << "  li a0, " << integer.value << "\n"; // 将整数加载到 a0 寄存器
}
