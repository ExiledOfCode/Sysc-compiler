#pragma once
#include "ast.hpp"
#include "exp.hpp"

class LValAST : public BaseAST {
public:
    std::string ident;
    LValAST(std::string id) : ident(std::move(id)) {
    }
    int Dump() const override {
        int nowId = TemValId;
        std::cout << "%" << TemValId++ << " = load @" << ident << "\n";
        return nowId;
    }
};

class StmtAST : public BaseAST {
public:
    // 使用枚举类型定义语句的种类
    enum class StmtKind {
        ASSIGN,       // 赋值语句 (LVal = Exp)
        RETURN_EXP,   // 带表达式的返回语句 (return Exp)
        RETURN_EMPTY, // 空返回语句 (return;)
        BLOCK,        // 块语句 (Block)
        EMPTY         // 空语句 (;)
    };

    StmtKind kind;                  // 语句类型
    std::unique_ptr<BaseAST> exp;   // 用于返回表达式或赋值语句的右值
    std::unique_ptr<BaseAST> lval;  // 用于赋值语句的左值
    std::unique_ptr<BaseAST> block; // 用于块语句

    // 单一构造函数，带默认参数，允许不使用的指针为 nullptr
    StmtAST(StmtKind k, std::unique_ptr<BaseAST> lval_ptr = nullptr,
            std::unique_ptr<BaseAST> exp_ptr = nullptr,
            std::unique_ptr<BaseAST> block_ptr = nullptr)
        : kind(k), exp(std::move(exp_ptr)), lval(std::move(lval_ptr)),
          block(std::move(block_ptr)) {
    }

    // 重写 Dump 函数，根据 kind 输出相应的内容
    int Dump() const override {
        switch (kind) {
        case StmtKind::ASSIGN: {
            int exp_id = exp->Dump();
            std::cout << "store %" << exp_id << ", @"
                      << dynamic_cast<LValAST *>(lval.get())->ident << "\n";
            return 0;
        }
        case StmtKind::RETURN_EXP: {
            int exp_id = exp->Dump();
            std::cout << "ret %" << exp_id << "\n";
            return 0;
        }
        case StmtKind::RETURN_EMPTY: {
            std::cout << "ret\n"; // 空返回语句
            return 0;
        }
        case StmtKind::BLOCK: {
            block->Dump(); // 调用块的 Dump
            return 0;
        }
        case StmtKind::EMPTY: {
            // 空语句无需输出
            return 0;
        }
        default:
            std::cerr << "Unknown StmtKind\n";
            return -1;
        }
    }
};