#pragma once
#include "ast.hpp"
#include <iostream>
#include <string>

class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> add_exp;
    ExpAST(std::unique_ptr<BaseAST> add_exp_ptr)
        : add_exp(std::move(add_exp_ptr)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        return add_exp->Dump();
    }
};

class MulExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> unary_exp;
    std::unique_ptr<BaseAST> mul_exp;
    std::unique_ptr<BaseAST> unary_exp_right;
    std::string op;
    bool is_unary;
    MulExpAST(std::unique_ptr<BaseAST> unary_exp_ptr)
        : unary_exp(std::move(unary_exp_ptr)), mul_exp(nullptr),
          unary_exp_right(nullptr), is_unary(true) {
    }
    MulExpAST(std::unique_ptr<BaseAST> mul_exp_ptr, std::string operation,
              std::unique_ptr<BaseAST> unary_exp_right_ptr)
        : unary_exp(nullptr), mul_exp(std::move(mul_exp_ptr)),
          unary_exp_right(std::move(unary_exp_right_ptr)),
          op(std::move(operation)), is_unary(false) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        if (is_unary) {
            return unary_exp->Dump();
        } else {
            int left_id = mul_exp->Dump();
            int right_id = unary_exp_right->Dump();
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
            return TemValId - 1;
        }
    }
};

class AddExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> mul_exp;
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> mul_exp_right;
    std::string op;
    bool is_mul;
    AddExpAST(std::unique_ptr<BaseAST> mul_exp_ptr)
        : mul_exp(std::move(mul_exp_ptr)), add_exp(nullptr),
          mul_exp_right(nullptr), is_mul(true) {
    }
    AddExpAST(std::unique_ptr<BaseAST> add_exp_ptr, std::string operation,
              std::unique_ptr<BaseAST> mul_exp_right_ptr)
        : mul_exp(nullptr), add_exp(std::move(add_exp_ptr)),
          mul_exp_right(std::move(mul_exp_right_ptr)), op(std::move(operation)),
          is_mul(false) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        if (is_mul) {
            return mul_exp->Dump();
        } else {
            int left_id = add_exp->Dump();
            int right_id = mul_exp_right->Dump();
            if (op == "+") {
                std::cout << "%" << TemValId++ << " = add %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == "-") {
                std::cout << "%" << TemValId++ << " = sub %" << left_id << ", %"
                          << right_id << "\n";
            }
            return TemValId - 1;
        }
    }
};

class RelExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> rel_exp;
    std::unique_ptr<BaseAST> add_exp_right;
    std::string op;
    bool is_add;
    RelExpAST(std::unique_ptr<BaseAST> add_exp_ptr)
        : add_exp(std::move(add_exp_ptr)), rel_exp(nullptr),
          add_exp_right(nullptr), is_add(true) {
    }
    RelExpAST(std::unique_ptr<BaseAST> rel_exp_ptr, std::string operation,
              std::unique_ptr<BaseAST> add_exp_right_ptr)
        : add_exp(nullptr), rel_exp(std::move(rel_exp_ptr)),
          add_exp_right(std::move(add_exp_right_ptr)), op(std::move(operation)),
          is_add(false) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        if (is_add) {
            return add_exp->Dump();
        } else {
            int left_id = rel_exp->Dump();
            int right_id = add_exp_right->Dump();
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
            return TemValId - 1;
        }
    }
};

class EqExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> rel_exp;
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> rel_exp_right;
    std::string op;
    bool is_rel;
    EqExpAST(std::unique_ptr<BaseAST> rel_exp_ptr)
        : rel_exp(std::move(rel_exp_ptr)), eq_exp(nullptr),
          rel_exp_right(nullptr), is_rel(true) {
    }
    EqExpAST(std::unique_ptr<BaseAST> eq_exp_ptr, std::string operation,
             std::unique_ptr<BaseAST> rel_exp_right_ptr)
        : rel_exp(nullptr), eq_exp(std::move(eq_exp_ptr)),
          rel_exp_right(std::move(rel_exp_right_ptr)), op(std::move(operation)),
          is_rel(false) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        if (is_rel) {
            return rel_exp->Dump();
        } else {
            int left_id = eq_exp->Dump();
            int right_id = rel_exp_right->Dump();
            if (op == "==") {
                std::cout << "%" << TemValId++ << " = eq %" << left_id << ", %"
                          << right_id << "\n";
            } else if (op == "!=") {
                std::cout << "%" << TemValId++ << " = ne %" << left_id << ", %"
                          << right_id << "\n";
            }
            return TemValId - 1;
        }
    }
};

class LAndExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> land_exp;
    std::unique_ptr<BaseAST> eq_exp_right;
    bool is_eq;
    LAndExpAST(std::unique_ptr<BaseAST> eq_exp_ptr)
        : eq_exp(std::move(eq_exp_ptr)), land_exp(nullptr),
          eq_exp_right(nullptr), is_eq(true) {
    }
    LAndExpAST(std::unique_ptr<BaseAST> land_exp_ptr,
               std::unique_ptr<BaseAST> eq_exp_right_ptr)
        : eq_exp(nullptr), land_exp(std::move(land_exp_ptr)),
          eq_exp_right(std::move(eq_exp_right_ptr)), is_eq(false) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        if (is_eq) {
            return eq_exp->Dump();
        } else {
            int left_id = land_exp->Dump();
            int right_id = eq_exp_right->Dump();
            std::cout << "%" << TemValId++ << " = ne 0, %" << left_id << "\n";
            int temp_id = TemValId;
            std::cout << "%" << TemValId++ << " = ne 0, %" << right_id << "\n";
            std::cout << "%" << TemValId++ << " = and %" << (temp_id - 1)
                      << ", %" << temp_id << "\n";
            return TemValId - 1;
        }
    }
};

class LOrExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> land_exp;
    std::unique_ptr<BaseAST> lor_exp;
    std::unique_ptr<BaseAST> land_exp_right;
    bool is_land;
    LOrExpAST(std::unique_ptr<BaseAST> land_exp_ptr)
        : land_exp(std::move(land_exp_ptr)), lor_exp(nullptr),
          land_exp_right(nullptr), is_land(true) {
    }
    LOrExpAST(std::unique_ptr<BaseAST> lor_exp_ptr,
              std::unique_ptr<BaseAST> land_exp_right_ptr)
        : land_exp(nullptr), lor_exp(std::move(lor_exp_ptr)),
          land_exp_right(std::move(land_exp_right_ptr)), is_land(false) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        if (is_land) {
            return land_exp->Dump();
        } else {
            int left_id = lor_exp->Dump();
            int right_id = land_exp_right->Dump();
            std::cout << "%" << TemValId++ << " = ne 0, %" << left_id << "\n";
            int temp_id = TemValId;
            std::cout << "%" << TemValId++ << " = ne 0, %" << right_id << "\n";
            std::cout << "%" << TemValId++ << " = or %" << (temp_id - 1)
                      << ", %" << temp_id << "\n";
            return TemValId - 1;
        }
    }
};

class NumberAST : public BaseAST {
public:
    int number;
    NumberAST(int num) : number(num) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        int nowId = TemValId;
        std::cout << "%" << TemValId++ << " = add 0, " << number << "\n";
        return nowId;
    }
};

class PrimaryExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> number;
    enum PrimaryType { EXP, LVAL, NUMBER }; // 命名枚举类型
    PrimaryType type;                       // 使用命名的枚举类型

    PrimaryExpAST(std::unique_ptr<BaseAST> exp_ptr,
                  std::unique_ptr<BaseAST> lval_ptr,
                  std::unique_ptr<BaseAST> num_ptr,
                  PrimaryType t) // 参数改为 PrimaryType 类型
        : exp(std::move(exp_ptr)), lval(std::move(lval_ptr)),
          number(std::move(num_ptr)), type(t) {
    }

    int Dump() const override {
        if (has_returned)
            return 0;
        switch (type) {
        case EXP:
            return exp->Dump();
        case LVAL:
            return lval->Dump();
        case NUMBER:
            return number->Dump();
        default:
            return 0; // 不会发生
        }
    }
};

class UnaryExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> primary_exp;
    std::string unary_op;
    std::unique_ptr<BaseAST> unary_exp;
    bool is_primary;
    UnaryExpAST(std::unique_ptr<BaseAST> primary_exp_ptr)
        : primary_exp(std::move(primary_exp_ptr)), unary_exp(nullptr),
          is_primary(true) {
    }
    UnaryExpAST(std::string op, std::unique_ptr<BaseAST> unary_exp_ptr)
        : primary_exp(nullptr), unary_op(std::move(op)),
          unary_exp(std::move(unary_exp_ptr)), is_primary(false) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        if (is_primary) {
            return primary_exp->Dump();
        } else {
            int operand_id = unary_exp->Dump();
            int nowId = TemValId;
            if (unary_op == "+") {
                return operand_id;
            } else if (unary_op == "-") {
                std::cout << "%" << TemValId++ << " = sub 0, %" << operand_id
                          << "\n";
            } else if (unary_op == "!") {
                std::cout << "%" << TemValId++ << " = eq 0, %" << operand_id
                          << "\n";
            }
            return nowId;
        }
    }
};

class UnaryOpAST : public BaseAST {
public:
    std::string op;
    UnaryOpAST(std::string operation) : op(std::move(operation)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        return 0;
    }
};