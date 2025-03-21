#pragma once
#include "ast.hpp"
#include "koopa.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
// 访问 raw program
void generate_riscv(const koopa_raw_program_t &program, std::ostream &out);

// 访问函数
void generate_riscv(const koopa_raw_function_t &func, std::ostream &out);

// 访问基本块
void generate_riscv(const koopa_raw_basic_block_t &bb, std::ostream &out);

// 访问指令
void generate_riscv(const koopa_raw_value_t &value, std::ostream &out);

// 访问 store 指令
void generate_riscv(const koopa_raw_store_t &ret, std::ostream &out);

// 访问 return 指令
void generate_riscv(const koopa_raw_return_t &ret, std::ostream &out);

// 访问 integer
void generate_riscv(const koopa_raw_integer_t &integer, std::ostream &out);

// 访问 binary 指令（包括一元运算符生成的 sub 和 eq）
void generate_riscv(const koopa_raw_binary_t &binary,
                    const koopa_raw_value_t &value, std::ostream &out);

// 访问 load 指令
void generate_riscv(const koopa_raw_load_t &load,
                    const koopa_raw_value_t &value, std::ostream &out);

// 访问 jump 指令
void generate_riscv(const koopa_raw_jump_t &jump, std::ostream &out);

// 访问 br 分支指令
void generate_riscv(const koopa_raw_branch_t &jump, std::ostream &out);