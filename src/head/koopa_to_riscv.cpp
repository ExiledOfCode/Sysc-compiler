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

    // 在函数入口分配栈空间
    reset_state(); // 重置栈状态
    for (size_t i = 0; i < func->bbs.len; ++i) {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb =
            (koopa_raw_basic_block_t)func->bbs.buffer[i];
        // 先遍历所有指令以确定栈大小
        for (size_t j = 0; j < bb->insts.len; ++j) {
            koopa_raw_value_t value = (koopa_raw_value_t)bb->insts.buffer[j];
            if (value->kind.tag == KOOPA_RVT_BINARY ||
                value->kind.tag == KOOPA_RVT_RETURN ||
                value->kind.tag == KOOPA_RVT_ALLOC ||
                value->kind.tag == KOOPA_RVT_LOAD ||
                value->kind.tag == KOOPA_RVT_STORE ||
                value->kind.tag == KOOPA_RVT_BRANCH ||
                value->kind.tag == KOOPA_RVT_JUMP) {
                allocate_stack(value);
            }
        }
    }
    int total_stack_size = (stack_offset + 15) & ~15; // 16 字节对齐
    if (total_stack_size > 0) {
        out << "  addi sp, sp, -" << total_stack_size << "\n";
        out << "  sw ra, " << (total_stack_size - 4)
            << "(sp)\n"; // 保存返回地址
    }

    // 生成基本块代码
    for (size_t i = 0; i < func->bbs.len; ++i) {
        koopa_raw_basic_block_t bb =
            (koopa_raw_basic_block_t)func->bbs.buffer[i];
        generate_riscv(bb, out);
    }
}

// 访问基本块
void generate_riscv(const koopa_raw_basic_block_t &bb, std::ostream &out) {
    if (bb->name && strlen(bb->name) > 0) {
        out << (bb->name + 1) << ":\n"; // 跳过 '%' 前缀
    }
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
    case KOOPA_RVT_ALLOC:
        // 因为我们已经在进入函数的时候申请了栈帧，所以现在不需要再处理这个指令了
        break;
    case KOOPA_RVT_STORE:
        generate_riscv(value->kind.data.store, out);
        break;
    case KOOPA_RVT_LOAD:
        generate_riscv(value->kind.data.load, value, out);
        break;
    case KOOPA_RVT_JUMP:
        generate_riscv(value->kind.data.jump, out);
        break;
    case KOOPA_RVT_BRANCH: // 新增分支指令支持
        generate_riscv(value->kind.data.branch, out);
        break;
    default:
        assert(false); // 未处理的指令类型
    }
}
// 访问 jump 指令
void generate_riscv(const koopa_raw_jump_t &jump, std::ostream &out) {
    const koopa_raw_basic_block_t target = jump.target;
    out << "  j " << (target->name + 1)
        << "\n"; // 跳过 '%' 前缀，直接跳转到目标标签
}
// 访问 br 分支命令
void generate_riscv(const koopa_raw_branch_t &branch, std::ostream &out) {
    const koopa_raw_value_t cond = branch.cond;               // 条件值
    const koopa_raw_basic_block_t true_bb = branch.true_bb;   // then 块
    const koopa_raw_basic_block_t false_bb = branch.false_bb; // else 块

    // 加载条件值到 t0
    if (cond->kind.tag == KOOPA_RVT_INTEGER) {
        out << "  li t0, " << cond->kind.data.integer.value << "\n";
    } else {
        int cond_offset = get_stack_offset(cond);
        out << "  lw t0, " << cond_offset << "(sp)\n";
    }

    // 生成条件分支：如果 t0 != 0，则跳转到 true_bb，否则跳转到 false_bb
    out << "  bnez t0, " << (true_bb->name + 1)
        << "\n"; // 如果条件为真，跳转到 then 块
    out << "  j " << (false_bb->name + 1) << "\n"; // 否则跳转到 else 块
}
// 访问 load 指令
void generate_riscv(const koopa_raw_load_t &load,
                    const koopa_raw_value_t &value, std::ostream &out) {
    const koopa_raw_value_t &src_value = load.src;
    assert(src_value->kind.tag == KOOPA_RVT_ALLOC); // 目前只支持从 alloc 加载

    // 获取源地址的栈偏移量
    int src_offset = get_stack_offset(src_value);

    // 从栈加载值到 t0
    out << "  lw t0, " << src_offset << "(sp)\n";

    // 获取 load 结果的栈偏移量
    int result_offset = get_stack_offset(value);

    // 将结果存储到栈上
    out << "  sw t0, " << result_offset << "(sp)\n";
}

// 访问 store 指令
void generate_riscv(const koopa_raw_store_t &store, std::ostream &out) {
    const koopa_raw_value_t &src_value = store.value; // 源值（如 %2）
    const koopa_raw_value_t &dest_value = store.dest; // 目标地址（如 @x）

    // 加载源值到 t0
    if (src_value->kind.tag == KOOPA_RVT_INTEGER) {
        int32_t int_val = src_value->kind.data.integer.value;
        out << "  li t0, " << int_val << "\n";
    } else {
        int src_offset = get_stack_offset(src_value);
        out << "  lw t0, " << src_offset << "(sp)\n"; // 从栈加载源值
    }

    // 获取目标地址的栈偏移量
    assert(dest_value->kind.tag ==
           KOOPA_RVT_ALLOC); // 目前只支持 alloc 类型的目标
    int dest_offset = get_stack_offset(dest_value);

    // 存储值到目标地址
    out << "  sw t0, " << dest_offset << "(sp)\n";
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
    int total_stack_size = (stack_offset + 15) & ~15; // 16 字节对齐
    if (total_stack_size > 0) {
        out << "  lw ra, " << (total_stack_size - 4)
            << "(sp)\n";                                      // 恢复返回地址
        out << "  addi sp, sp, " << total_stack_size << "\n"; // 释放栈空间
    }
    out << "  ret\n";
}

// 访问 integer
void generate_riscv(const koopa_raw_integer_t &integer, std::ostream &out) {
    out << "  li a0, " << integer.value << "\n";
}

// 加载操作数到寄存器
void load_operand(const koopa_raw_value_t &operand, const char *reg,
                  std::ostream &out) {
    if (operand->kind.tag == KOOPA_RVT_INTEGER) {
        out << "  li " << reg << ", " << operand->kind.data.integer.value
            << "\n";
    } else {
        int offset = get_stack_offset(operand);
        out << "  lw " << reg << ", " << offset << "(sp)\n";
    }
}

// 访问 binary 指令
void generate_riscv(const koopa_raw_binary_t &binary,
                    const koopa_raw_value_t &value, std::ostream &out) {
    koopa_raw_value_t lhs = binary.lhs;
    koopa_raw_value_t rhs = binary.rhs;

    // 为结果分配栈空间
    int result_offset = get_stack_offset(value); // 应该已经在函数入口分配

    // 加载左操作数到 t0
    load_operand(lhs, "t0", out);

    // 加载右操作数到 t1
    load_operand(rhs, "t1", out);

    // 根据操作符生成 RISC-V 指令
    switch (binary.op) {
    case KOOPA_RBO_ADD:
        out << "  add t2, t0, t1\n";
        break;
    case KOOPA_RBO_SUB:
        out << "  sub t2, t0, t1\n";
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
    case KOOPA_RBO_AND:
        out << "  and t2, t0, t1\n";
        break;
    case KOOPA_RBO_OR:
        out << "  or t2, t0, t1\n";
        break;
    case KOOPA_RBO_EQ:
        out << "  sub t2, t0, t1\n"; // t2 = t0 - t1
        out << "  seqz t2, t2\n";    // t2 = (t2 == 0) ? 1 : 0
        break;
    case KOOPA_RBO_NOT_EQ:
        out << "  sub t2, t0, t1\n"; // t2 = t0 - t1
        out << "  snez t2, t2\n";    // t2 = (t2 != 0) ? 1 : 0
        break;
    case KOOPA_RBO_GT:
        out << "  sgt t2, t0, t1\n"; // t2 = (t0 > t1) ? 1 : 0
        break;
    case KOOPA_RBO_LT:
        out << "  slt t2, t0, t1\n"; // t2 = (t0 < t1) ? 1 : 0
        break;
    case KOOPA_RBO_GE:
        out << "  slt t2, t0, t1\n"; // t2 = (t0 < t1) ? 1 : 0
        out << "  xori t2, t2, 1\n"; // t2 = !t2 (t0 >= t1)
        break;
    case KOOPA_RBO_LE:
        out << "  sgt t2, t0, t1\n"; // t2 = (t0 > t1) ? 1 : 0
        out << "  xori t2, t2, 1\n"; // t2 = !t2 (t0 <= t1)
        break;
    default:
        assert(false); // 未处理的操作符
    }

    // 将结果存入栈
    out << "  sw t2, " << result_offset << "(sp)\n";
}