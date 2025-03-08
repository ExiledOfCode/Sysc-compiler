#include "koopa_to_riscv.hpp"

using namespace std;

// 全局变量定义
int stack_offset = 0;
unordered_map<koopa_raw_value_t, int> value_to_offset;

// 重置全局状态
void reset_state() {
    stack_offset = 0;
    value_to_offset.clear();
}

// 为值分配栈空间并返回偏移量
int allocate_stack(koopa_raw_value_t value) {
    int offset = stack_offset;
    value_to_offset[value] = offset;
    stack_offset += 4; // 每个值占 4 字节 (i32)
    return offset;
}

// 获取值的栈偏移量
int get_stack_offset(koopa_raw_value_t value) {
    auto it = value_to_offset.find(value);
    assert(it != value_to_offset.end()); // 确保值已分配
    return it->second;
}

// 访问 raw program
void generate_riscv(const koopa_raw_program_t &program, std::ostream &out) {
    reset_state();
    out << "  .text\n";
    for (size_t i = 0; i < program.funcs.len; ++i) {
        assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func =
            (koopa_raw_function_t)program.funcs.buffer[i];
        generate_riscv(func, out);
    }
}

// 访问函数
void generate_riscv(const koopa_raw_function_t &func, std::ostream &out) {
    out << "  .globl " << (func->name + 1) << "\n"; // 跳过 '@' 前缀
    out << (func->name + 1) << ":\n";

    // 遍历基本块，先生成所有指令，确定栈大小
    for (size_t i = 0; i < func->bbs.len; ++i) {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb =
            (koopa_raw_basic_block_t)func->bbs.buffer[i];
        generate_riscv(bb, out);
    }

    // 在函数末尾分配和释放栈空间（这里简化处理，实际应在入口分配）
}

// 访问基本块
void generate_riscv(const koopa_raw_basic_block_t &bb, std::ostream &out) {
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
    if (ret_value) {
        if (ret_value->kind.tag == KOOPA_RVT_INTEGER) {
            out << "  li a0, " << ret_value->kind.data.integer.value << "\n";
        } else {
            int offset = get_stack_offset(ret_value);
            out << "  lw a0, " << offset << "(sp)\n"; // 从栈加载返回值
        }
    }
    int total_stack_size = stack_offset + 4;                  // 包括返回地址
    out << "  addi sp, sp, -" << total_stack_size << "\n";    // 分配栈空间
    out << "  sw ra, " << (total_stack_size - 4) << "(sp)\n"; // 保存返回地址
    out << "  lw ra, " << (total_stack_size - 4) << "(sp)\n"; // 恢复返回地址
    out << "  addi sp, sp, " << total_stack_size << "\n";     // 释放栈空间
    out << "  ret\n";
}

// 访问 integer
void generate_riscv(const koopa_raw_integer_t &integer, std::ostream &out) {
    out << "  li a0, " << integer.value << "\n";
}

// 访问 binary 指令
void generate_riscv(const koopa_raw_binary_t &binary,
                    const koopa_raw_value_t &value, std::ostream &out) {
    koopa_raw_value_t lhs = binary.lhs;
    koopa_raw_value_t rhs = binary.rhs;

    // 为结果分配栈空间
    int result_offset = allocate_stack(value);

    // 加载左操作数到 t0
    if (lhs->kind.tag == KOOPA_RVT_INTEGER) {
        out << "  li t0, " << lhs->kind.data.integer.value << "\n";
    } else {
        int lhs_offset = get_stack_offset(lhs);
        out << "  lw t0, " << lhs_offset << "(sp)\n";
    }

    // 加载右操作数到 t1
    if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
        out << "  li t1, " << rhs->kind.data.integer.value << "\n";
    } else {
        int rhs_offset = get_stack_offset(rhs);
        out << "  lw t1, " << rhs_offset << "(sp)\n";
    }

    // 根据操作符生成相应的 RISC-V 指令
    switch (binary.op) {
    case KOOPA_RBO_SUB:
        out << "  sub t2, t0, t1\n";
        break;
    case KOOPA_RBO_EQ:            // 用于一元运算符 "!"
        out << "  seqz t2, t1\n"; // t2 = (t1 == 0) ? 1 : 0
        break;
    case KOOPA_RBO_ADD:
        out << "  add t2, t0, t1\n";
        break;
    case KOOPA_RBO_MUL:
        out << "  mul t2, t0, t1\n";
        break;
    case KOOPA_RBO_DIV:
        out << "  div t2, t0, t1\n";
        break;
    case KOOPA_RBO_MOD:
        out << "  rem t2, t0, t1\n";
        break;
    default:
        assert(false); // 未处理的操作符
    }

    // 将结果存入栈
    out << "  sw t2, " << result_offset << "(sp)\n";
}