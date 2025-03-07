#pragma once
#include "ast.hpp"
#include <iostream>

// Exp ::= UnaryExp;
class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> unary_exp;

    ExpAST(std::unique_ptr<BaseAST> unary_exp_ptr)
        : unary_exp(std::move(unary_exp_ptr)) {
    }

    int Dump() const override {
        return unary_exp->Dump();
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
                std::cout << "%" << TemValId++ << " = eq %" << operand_id
                          << ", 0" << "\n";
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