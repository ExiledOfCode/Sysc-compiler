#pragma once
#include "ast.hpp"
#include <iostream>

// Exp ::= UnaryExp;
// Exp ::= AddExp;
class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> add_exp;

    ExpAST(std::unique_ptr<BaseAST> add_exp_ptr)
        : add_exp(std::move(add_exp_ptr)) {
    }

    int Dump() const override {
        // 直接委托给 AddExp
        return add_exp->Dump();
    }
};

// MulExp ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
class MulExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> unary_exp;       // 用于 UnaryExp
    std::unique_ptr<BaseAST> mul_exp;         // 用于 MulExp（递归）
    std::unique_ptr<BaseAST> unary_exp_right; // 右侧的 UnaryExp
    std::string op;                           // 操作符：*、/ 或 %
    bool is_unary; // 区分是单一 UnaryExp 还是二元操作

    // 构造函数 1：单一 UnaryExp
    MulExpAST(std::unique_ptr<BaseAST> unary_exp_ptr)
        : unary_exp(std::move(unary_exp_ptr)), mul_exp(nullptr),
          unary_exp_right(nullptr), is_unary(true) {
    }

    // 构造函数 2：MulExp op UnaryExp
    MulExpAST(std::unique_ptr<BaseAST> mul_exp_ptr, std::string operation,
              std::unique_ptr<BaseAST> unary_exp_right_ptr)
        : unary_exp(nullptr), mul_exp(std::move(mul_exp_ptr)),
          unary_exp_right(std::move(unary_exp_right_ptr)),
          op(std::move(operation)), is_unary(false) {
    }

    int Dump() const override {
        if (is_unary) {
            // 如果只是 UnaryExp，直接返回其结果
            return unary_exp->Dump();
        } else {
            // 二元操作：计算左右操作数
            int left_id = mul_exp->Dump();
            int right_id = unary_exp_right->Dump();
            int nowId = TemValId;

            // 根据操作符生成 Koopa IR
            if (op == "*") {
                std::cout << "%" << TemValId++ << " = mul %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == "/") {
                std::cout << "%" << TemValId++ << " = div %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == "%") {
                std::cout << "%" << TemValId++ << " = mod %" << left_id << ", %"
                          << right_id << "\n";
            }
            return nowId;
        }
    }
};

// AddExp ::= MulExp | AddExp ("+" | "-") MulExp;
class AddExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> mul_exp;       // 用于 MulExp
    std::unique_ptr<BaseAST> add_exp;       // 用于 AddExp（递归）
    std::unique_ptr<BaseAST> mul_exp_right; // 右侧的 MulExp
    std::string op;                         // 操作符：+ 或 -
    bool is_mul;                            // 区分是单一 MulExp 还是二元操作

    // 构造函数 1：单一 MulExp
    AddExpAST(std::unique_ptr<BaseAST> mul_exp_ptr)
        : mul_exp(std::move(mul_exp_ptr)), add_exp(nullptr),
          mul_exp_right(nullptr), is_mul(true) {
    }

    // 构造函数 2：AddExp op MulExp
    AddExpAST(std::unique_ptr<BaseAST> add_exp_ptr, std::string operation,
              std::unique_ptr<BaseAST> mul_exp_right_ptr)
        : mul_exp(nullptr), add_exp(std::move(add_exp_ptr)),
          mul_exp_right(std::move(mul_exp_right_ptr)), op(std::move(operation)),
          is_mul(false) {
    }

    int Dump() const override {
        if (is_mul) {
            // 如果只是 MulExp，直接返回其结果
            return mul_exp->Dump();
        } else {
            // 二元操作：计算左右操作数
            int left_id = add_exp->Dump();
            int right_id = mul_exp_right->Dump();
            int nowId = TemValId;

            // 根据操作符生成 Koopa IR
            if (op == "+") {
                std::cout << "%" << TemValId++ << " = add %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == "-") {
                std::cout << "%" << TemValId++ << " = sub %" << left_id << ", %"
                          << right_id << "\n";
            }
            return nowId;
        }
    }
};

// RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> add_exp;       // 用于单一 AddExp
    std::unique_ptr<BaseAST> rel_exp;       // 用于 RelExp（递归）
    std::unique_ptr<BaseAST> add_exp_right; // 右侧的 AddExp
    std::string op;                         // 操作符：<、>、<= 或 >=
    bool is_add;                            // 区分是单一 AddExp 还是比较操作

    // 构造函数 1：单一 AddExp
    RelExpAST(std::unique_ptr<BaseAST> add_exp_ptr)
        : add_exp(std::move(add_exp_ptr)), rel_exp(nullptr),
          add_exp_right(nullptr), is_add(true) {
    }

    // 构造函数 2：RelExp op AddExp
    RelExpAST(std::unique_ptr<BaseAST> rel_exp_ptr, std::string operation,
              std::unique_ptr<BaseAST> add_exp_right_ptr)
        : add_exp(nullptr), rel_exp(std::move(rel_exp_ptr)),
          add_exp_right(std::move(add_exp_right_ptr)), op(std::move(operation)),
          is_add(false) {
    }

    int Dump() const override {
        if (is_add) {
            return add_exp->Dump();
        } else {
            int left_id = rel_exp->Dump();
            int right_id = add_exp_right->Dump();
            int nowId = TemValId;

            if (op == "<") {
                std::cout << "%" << TemValId++ << " = lt %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == ">") {
                std::cout << "%" << TemValId++ << " = gt %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == "<=") {
                std::cout << "%" << TemValId++ << " = le %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == ">=") {
                std::cout << "%" << TemValId++ << " = ge %" << left_id << ", %"
                          << right_id << "\n";
            }
            return nowId;
        }
    }
};

// EqExp ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> rel_exp;       // 用于单一 RelExp
    std::unique_ptr<BaseAST> eq_exp;        // 用于 EqExp（递归）
    std::unique_ptr<BaseAST> rel_exp_right; // 右侧的 RelExp
    std::string op;                         // 操作符：== 或 !=
    bool is_rel;                            // 区分是单一 RelExp 还是相等性操作

    // 构造函数 1：单一 RelExp
    EqExpAST(std::unique_ptr<BaseAST> rel_exp_ptr)
        : rel_exp(std::move(rel_exp_ptr)), eq_exp(nullptr),
          rel_exp_right(nullptr), is_rel(true) {
    }

    // 构造函数 2：EqExp op RelExp
    EqExpAST(std::unique_ptr<BaseAST> eq_exp_ptr, std::string operation,
             std::unique_ptr<BaseAST> rel_exp_right_ptr)
        : rel_exp(nullptr), eq_exp(std::move(eq_exp_ptr)),
          rel_exp_right(std::move(rel_exp_right_ptr)), op(std::move(operation)),
          is_rel(false) {
    }

    int Dump() const override {
        if (is_rel) {
            return rel_exp->Dump();
        } else {
            int left_id = eq_exp->Dump();
            int right_id = rel_exp_right->Dump();
            int nowId = TemValId;

            if (op == "==") {
                std::cout << "%" << TemValId++ << " = eq %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == "!=") {
                std::cout << "%" << TemValId++ << " = ne %" << left_id << ", %"
                          << right_id << "\n";
            }
            return nowId;
        }
    }
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> eq_exp;       // 用于单一 EqExp
    std::unique_ptr<BaseAST> land_exp;     // 用于 LAndExp（递归）
    std::unique_ptr<BaseAST> eq_exp_right; // 右侧的 EqExp
    bool is_eq;                            // 区分是单一 EqExp 还是逻辑与操作

    // 构造函数 1：单一 EqExp
    LAndExpAST(std::unique_ptr<BaseAST> eq_exp_ptr)
        : eq_exp(std::move(eq_exp_ptr)), land_exp(nullptr),
          eq_exp_right(nullptr), is_eq(true) {
    }

    // 构造函数 2：LAndExp && EqExp
    LAndExpAST(std::unique_ptr<BaseAST> land_exp_ptr,
               std::unique_ptr<BaseAST> eq_exp_right_ptr)
        : eq_exp(nullptr), land_exp(std::move(land_exp_ptr)),
          eq_exp_right(std::move(eq_exp_right_ptr)), is_eq(false) {
    }

    int Dump() const override {
        if (is_eq) {
            return eq_exp->Dump();
        } else {
            int left_id = land_exp->Dump();
            int right_id = eq_exp_right->Dump();
            int nowId = TemValId;
            // Koopa IR 使用 ne 0 来确保布尔值
            std::cout << "%" << TemValId++ << " = ne 0, %" << left_id << "\n";
            int temp_id = TemValId;
            std::cout << "%" << TemValId++ << " = ne 0, %" << right_id << "\n";
            std::cout << "%" << TemValId++ << " = and %" << (temp_id - 1)
                      << ", %" << temp_id << "\n";
            return nowId;
        }
    }
};

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> land_exp;       // 用于单一 LAndExp
    std::unique_ptr<BaseAST> lor_exp;        // 用于 LOrExp（递归）
    std::unique_ptr<BaseAST> land_exp_right; // 右侧的 LAndExp
    bool is_land; // 区分是单一 LAndExp 还是逻辑或操作

    // 构造函数 1：单一 LAndExp
    LOrExpAST(std::unique_ptr<BaseAST> land_exp_ptr)
        : land_exp(std::move(land_exp_ptr)), lor_exp(nullptr),
          land_exp_right(nullptr), is_land(true) {
    }

    // 构造函数 2：LOrExp || LAndExp
    LOrExpAST(std::unique_ptr<BaseAST> lor_exp_ptr,
              std::unique_ptr<BaseAST> land_exp_right_ptr)
        : land_exp(nullptr), lor_exp(std::move(lor_exp_ptr)),
          land_exp_right(std::move(land_exp_right_ptr)), is_land(false) {
    }

    int Dump() const override {
        if (is_land) {
            return land_exp->Dump();
        } else {
            int left_id = lor_exp->Dump();
            int right_id = land_exp_right->Dump();
            int nowId = TemValId;
            // Koopa IR 使用 ne 0 来确保布尔值
            std::cout << "%" << TemValId++ << " = ne 0, %" << left_id << "\n";
            int temp_id = TemValId;
            std::cout << "%" << TemValId++ << " = ne 0, %" << right_id << "\n";
            std::cout << "%" << TemValId++ << " = or %" << (temp_id - 1)
                      << ", %" << temp_id << "\n";
            return nowId;
        }
    }
};
// Number ::= INT_CONST;
class NumberAST : public BaseAST {
public:
    int number;
    NumberAST(int num) : number(num) {
    }

    int Dump() const override {
        int nowId = TemValId;
        std::cout << "%" << TemValId++ << " = add 0, " << number << "\n";
        return nowId;
    }
};

// PrimaryExp ::= "(" Exp ")" | Number;
class PrimaryExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;    // 用于 "(" Exp ")"
    std::unique_ptr<BaseAST> number; // 用于 Number
    bool is_number;                  // 区分是 Number 还是括号表达式

    PrimaryExpAST(std::unique_ptr<BaseAST> exp_ptr,
                  std::unique_ptr<BaseAST> num_ptr, bool isNumber)
        : exp(std::move(exp_ptr)), number(std::move(num_ptr)),
          is_number(isNumber) {
    }

    int Dump() const override {
        if (is_number) {
            return number->Dump(); // NumberAST 会生成新变量
        } else {
            return exp->Dump(); // 括号中的 Exp
        }
    }
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp;
class UnaryExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> primary_exp; // 用于 PrimaryExp
    std::string unary_op;                 // 用于 UnaryOp
    std::unique_ptr<BaseAST> unary_exp;   // 用于 UnaryExp
    bool is_primary;                      // 区分两者

    UnaryExpAST(std::unique_ptr<BaseAST> primary_exp_ptr)
        : primary_exp(std::move(primary_exp_ptr)), unary_exp(nullptr),
          is_primary(true) {
    }

    UnaryExpAST(std::string op, std::unique_ptr<BaseAST> unary_exp_ptr)
        : primary_exp(nullptr), unary_op(std::move(op)),
          unary_exp(std::move(unary_exp_ptr)), is_primary(false) {
    }

    int Dump() const override {
        if (is_primary) {
            return primary_exp->Dump();
        } else {
            int operand_id = unary_exp->Dump(); // 计算操作数
            int nowId = TemValId;
            if (unary_op == "+") {
                // 正号不改变值，直接返回操作数 ID
                return operand_id;
            } else if (unary_op == "-") {
                // 取负: %n = sub 0, operand
                std::cout << "%" << TemValId++ << " = sub 0, %" << operand_id
                          << "\n";
            } else if (unary_op == "!") {
                // 逻辑非: %n = eq 0 , operand
                std::cout << "%" << TemValId++ << " = eq 0, %" << operand_id
                          << "\n";
            }
            return nowId;
        }
    }
};

// UnaryOp ::= "+" | "-" | "!";
class UnaryOpAST : public BaseAST {
public:
    std::string op;

    UnaryOpAST(std::string operation) : op(std::move(operation)) {
    }

    int Dump() const override {
        // UnaryOpAST 本身不生成 IR，仅提供操作符给 UnaryExpAST
        return 0;
    }
};
