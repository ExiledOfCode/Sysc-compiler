#pragma once
#include "ast.hpp"
#include <iostream>
#include <memory>
#include <string>

extern int TemValId; // 声明全局变量，不初始化

class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual int Dump() const = 0; // 修改为 int 返回值
};

// CompUnit ::= FuncDef;
class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;

    // 构造函数
    CompUnitAST(std::unique_ptr<BaseAST> func_def_ptr)
        : func_def(std::move(func_def_ptr)) {
    }

    int Dump() const override { // 修改为 int 返回值
        func_def->Dump();
        return 0; // 添加默认返回值
    }
};

// FuncDef ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    // 构造函数
    FuncDefAST(std::unique_ptr<BaseAST> func_type_ptr, std::string id,
               std::unique_ptr<BaseAST> block_ptr)
        : func_type(std::move(func_type_ptr)), ident(std::move(id)),
          block(std::move(block_ptr)) {
    }

    int Dump() const override { // 修改为 int 返回值
        std::cout << "fun ";
        std::cout << "@" << ident << "() : ";
        func_type->Dump();
        std::cout << "{";
        std::cout << std::endl;
        block->Dump();
        std::cout << "}\n";
        return 0; // 添加默认返回值
    }
};

// FuncType ::= "int";
class FuncTypeAST : public BaseAST {
public:
    std::string type;

    // 构造函数
    FuncTypeAST(std::string type_name) : type(std::move(type_name)) {
    }

    int Dump() const override { // 修改为 int 返回值
        if (this->type == "int")
            std::cout << "i32 ";
        return 0; // 添加默认返回值
    }
};

// Block ::= "{" Stmt "}";
class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;

    // 构造函数
    BlockAST(std::unique_ptr<BaseAST> stmt_ptr) : stmt(std::move(stmt_ptr)) {
    }

    int Dump() const override { // 修改为 int 返回值
        std::cout << "\%entry:\n";
        stmt->Dump();
        return 0; // 添加默认返回值
    }
};

// Stmt ::= "return" Exp ";";
class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    StmtAST(std::unique_ptr<BaseAST> exp_ptr) : exp(std::move(exp_ptr)) {
    }

    int Dump() const override {
        int exp_id = exp->Dump();
        std::cout << "ret %" << exp_id << "\n";
        return 0; // Stmt 的返回值无意义
    }
};
