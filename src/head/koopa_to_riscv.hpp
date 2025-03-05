#pragma once
#include "ast.hpp"
#include "koopa.h"
#include <cassert>
#include <iostream>
#include <ostream>
#include <string>
// 访问 Koopa IR 并生成 RISC-V 汇编
void generate_riscv(const koopa_raw_program_t &program, std::ostream &out);
void generate_riscv(const koopa_raw_function_t &func, std::ostream &out);
void generate_riscv(const koopa_raw_basic_block_t &bb, std::ostream &out);
void generate_riscv(const koopa_raw_value_t &value, std::ostream &out);
void generate_riscv(const koopa_raw_return_t &ret, std::ostream &out);
void generate_riscv(const koopa_raw_integer_t &integer, std::ostream &out);
