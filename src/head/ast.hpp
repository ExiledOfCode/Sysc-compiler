#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>

extern int TemValId; // 声明全局变量，不初始化

class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual int Dump() const = 0; // 纯虚函数，输出Koopa IR并返回临时变量ID
};

// CompUnit ::= FuncDef;
class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;
    CompUnitAST(std::unique_ptr<BaseAST> func_def_ptr)
        : func_def(std::move(func_def_ptr)) {
    }
    int Dump() const override {
        func_def->Dump();
        return 0;
    }
};

// FuncDef ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    FuncDefAST(std::unique_ptr<BaseAST> func_type_ptr, std::string id,
               std::unique_ptr<BaseAST> block_ptr)
        : func_type(std::move(func_type_ptr)), ident(std::move(id)),
          block(std::move(block_ptr)) {
    }
    int Dump() const override {
        std::cout << "fun ";
        std::cout << "@" << ident << "() : ";
        func_type->Dump();
        std::cout << "{\n";
        block->Dump();
        std::cout << "}\n";
        return 0;
    }
};

// FuncType ::= "int";
class FuncTypeAST : public BaseAST {
public:
    std::string type;
    FuncTypeAST(std::string type_name) : type(std::move(type_name)) {
    }
    int Dump() const override {
        if (type == "int")
            std::cout << "i32 ";
        return 0;
    }
};

// Block ::= "{" {BlockItem} "}";
class BlockAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST>> block_items;
    BlockAST(std::vector<std::unique_ptr<BaseAST>> items)
        : block_items(std::move(items)) {
    }
    int Dump() const override {
        std::cout << "%entry:\n";
        for (const auto &item : block_items) {
            item->Dump();
        }
        return 0;
    }
};

// BlockItem ::= Decl | Stmt;
class BlockItemAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> item;
    bool is_decl; // true表示Decl，false表示Stmt
    BlockItemAST(std::unique_ptr<BaseAST> item_ptr, bool is_decl_flag)
        : item(std::move(item_ptr)), is_decl(is_decl_flag) {
    }
    int Dump() const override {
        return item->Dump();
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
        return 0;
    }
};

// Decl ::= ConstDecl;
class DeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> const_decl;
    DeclAST(std::unique_ptr<BaseAST> const_decl_ptr)
        : const_decl(std::move(const_decl_ptr)) {
    }
    int Dump() const override {
        const_decl->Dump();
        return 0;
    }
};

// ConstDecl ::= "const" BType ConstDef {"," ConstDef} ";";
class ConstDeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> btype;
    std::vector<std::unique_ptr<BaseAST>> const_defs;
    ConstDeclAST(std::unique_ptr<BaseAST> btype_ptr,
                 std::vector<std::unique_ptr<BaseAST>> defs)
        : btype(std::move(btype_ptr)), const_defs(std::move(defs)) {
    }
    int Dump() const override {
        for (const auto &def : const_defs) {
            def->Dump();
        }
        return 0;
    }
};

// BType ::= "int";
class BTypeAST : public BaseAST {
public:
    std::string type;
    BTypeAST(std::string type_name) : type(std::move(type_name)) {
    }
    int Dump() const override {
        if (type == "int")
            std::cout << "i32 ";
        return 0;
    }
};

// ConstDef ::= IDENT "=" ConstInitVal;
class ConstDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;
    ConstDefAST(std::string id, std::unique_ptr<BaseAST> init_val)
        : ident(std::move(id)), const_init_val(std::move(init_val)) {
    }
    int Dump() const override {
        int val_id = const_init_val->Dump();
        std::cout << "@" << ident << " = alloc i32\n";
        std::cout << "store %" << val_id << ", @" << ident << "\n";
        return 0;
    }
};

// ConstInitVal ::= ConstExp;
class ConstInitValAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> const_exp;
    ConstInitValAST(std::unique_ptr<BaseAST> exp) : const_exp(std::move(exp)) {
    }
    int Dump() const override {
        return const_exp->Dump();
    }
};

// ConstExp ::= Exp;
class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    ConstExpAST(std::unique_ptr<BaseAST> exp_ptr) : exp(std::move(exp_ptr)) {
    }
    int Dump() const override {
        return exp->Dump();
    }
};