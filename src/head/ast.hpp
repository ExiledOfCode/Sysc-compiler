#pragma once
#include "ast.hpp"
#include <iostream>
#include <memory>
#include <string>

class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
};

class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;

    // 构造函数
    CompUnitAST(std::unique_ptr<BaseAST> func_def_ptr)
        : func_def(std::move(func_def_ptr)) {
    }

    void Dump() const override {
        func_def->Dump();
    }
};

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

    void Dump() const override {
        std::cout << "fun ";
        std::cout << "@" << ident << "() : ";
        func_type->Dump();
        std::cout << "{";
        std::cout << std::endl;
        block->Dump();
        std::cout << "}\n";
    }
};

class FuncTypeAST : public BaseAST {
public:
    std::string type;

    // 构造函数
    FuncTypeAST(std::string type_name) : type(std::move(type_name)) {
    }

    void Dump() const override {
        if (this->type == "int")
            std::cout << "i32 ";
    }
};

class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;

    // 构造函数
    BlockAST(std::unique_ptr<BaseAST> stmt_ptr) : stmt(std::move(stmt_ptr)) {
    }

    void Dump() const override {
        std::cout << "\%entry:\n";
        stmt->Dump();
    }
};

class StmtAST : public BaseAST {
public:
    int number;

    // 构造函数
    StmtAST(int num) : number(num) {
    }
    void Dump() const override {
        std::cout << "  ret " << number << std::endl;
    }
};