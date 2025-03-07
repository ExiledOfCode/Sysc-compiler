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
    case KOOPA_RVT_INTEGER:
        generate_riscv(value->kind.data.integer, out);
        break;
    case KOOPA_RVT_BINARY:
        generate_riscv(value->kind.data.binary, value, out);
        break;
    default:
        assert(false); // 未处理的指令类型
    }
}

// 访问 return 指令
void generate_riscv(const koopa_raw_return_t &ret, std::ostream &out) {
    koopa_raw_value_t ret_value = ret.value;
    if (ret_value->kind.tag == KOOPA_RVT_INTEGER) {
        generate_riscv(ret_value->kind.data.integer, out);
    } else {
        // 对于非立即数的返回值，已经由前面的指令计算好，结果在 a0 中
        // 这里无需额外操作
    }
    out << "  ret\n"; // 输出返回指令
}

// 访问 integer
void generate_riscv(const koopa_raw_integer_t &integer, std::ostream &out) {
    out << "  li a0, " << integer.value << "\n"; // 将整数加载到 a0 寄存器
}

// 访问 binary 指令（包括一元运算符生成的 sub 和 eq）
void generate_riscv(const koopa_raw_binary_t &binary,
                    const koopa_raw_value_t &value, std::ostream &out) {
    koopa_raw_value_t lhs = binary.lhs;
    koopa_raw_value_t rhs = binary.rhs;

    switch (binary.op) {
    case KOOPA_RBO_SUB:
        // 检查是否为一元运算符 "-": sub 0, rhs
        if (lhs->kind.tag == KOOPA_RVT_INTEGER &&
            lhs->kind.data.integer.value == 0) {
            // 一元运算符 "-rhs"
            if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
                // 常量折叠
                int32_t result = -rhs->kind.data.integer.value;
                out << "  li a0, " << result << "\n";
            } else {
                // 非常量，使用 neg 指令
                generate_riscv(rhs, out); // 结果在 a0
                out << "  neg a0, a0\n";  // a0 = -a0
            }
        } else {
            // 普通二元减法
            if (lhs->kind.tag == KOOPA_RVT_INTEGER) {
                out << "  li t0, " << lhs->kind.data.integer.value << "\n";
            } else {
                generate_riscv(lhs, out);
                out << "  mv t0, a0\n";
            }
            if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
                out << "  li t1, " << rhs->kind.data.integer.value << "\n";
            } else {
                generate_riscv(rhs, out);
                out << "  mv t1, a0\n";
            }
            out << "  sub a0, t0, t1\n";
        }
        break;

    case KOOPA_RBO_EQ: // 用于一元运算符 "!"
        // 只需加载 lhs，rhs 固定为 0
        if (lhs->kind.tag == KOOPA_RVT_INTEGER) {
            int32_t val = lhs->kind.data.integer.value;
            out << "  li a0, " << (val == 0 ? 1 : 0) << "\n";
        } else {
            generate_riscv(lhs, out);
            out << "  seqz a0, a0\n";
        }
        break;

    case KOOPA_RBO_ADD:
        if (lhs->kind.tag == KOOPA_RVT_INTEGER &&
            lhs->kind.data.integer.value == 0) {
            // 优化 add 0, rhs
            if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
                out << "  li a0, " << rhs->kind.data.integer.value << "\n";
            } else {
                generate_riscv(rhs, out); // 结果已在 a0，无需额外操作
            }
        } else {
            // 普通二元加法
            if (lhs->kind.tag == KOOPA_RVT_INTEGER) {
                out << "  li t0, " << lhs->kind.data.integer.value << "\n";
            } else {
                generate_riscv(lhs, out);
                out << "  mv t0, a0\n";
            }
            if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
                out << "  li t1, " << rhs->kind.data.integer.value << "\n";
            } else {
                generate_riscv(rhs, out);
                out << "  mv t1, a0\n";
            }
            out << "  add a0, t0, t1\n";
        }
        break;

    default:
        assert(false); // 未处理的操作符
    }
}