#pragma once
#include "SymbolTable.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

extern SymbolTable symTab;
extern bool has_returned;
class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual int Dump() const = 0; // 纯虚函数，输出 Koopa IR 并返回临时变量 ID
};

// CompUnit ::= FuncDefs;
class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_defs; // 函数定义列表
    CompUnitAST(std::unique_ptr<BaseAST> func_defs_ptr)
        : func_defs(std::move(func_defs_ptr)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        func_defs->Dump();
        return 0;
    }
};

// FuncDefs ::= {FuncDef};
class FuncDefsAST : public BaseAST {
public:
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> func_defs;
    FuncDefsAST(std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> defs)
        : func_defs(std::move(defs)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        for (const auto &func : *func_defs) {
            if (has_returned)
                break;
            symTab.enterScope(); // 进入块作用域
            func->Dump();
            has_returned = 0;
            symTab.exitScope(); // 进入块作用域
        }
        return 0;
    }
};

// FuncFParam ::= BType IDENT;
class FuncFParamAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> btype;
    std::string ident;
    FuncFParamAST(std::unique_ptr<BaseAST> btype_ptr, std::string id)
        : btype(std::move(btype_ptr)), ident(std::move(id)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        std::cout << symTab.findVariable(ident) << ": ";
        btype->Dump();
        return 0;
    }
};
// FuncFParams ::= FuncFParam {"," FuncFParam};
class FuncFParamsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST>> params;
    FuncFParamsAST(std::vector<std::unique_ptr<BaseAST>> p)
        : params(std::move(p)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        for (size_t i = 0; i < params.size(); ++i) {
            FuncFParamAST *param_ast =
                dynamic_cast<FuncFParamAST *>(params[i].get());
            if (param_ast) {
                // 添加参数到符号表（非常量）
                symTab.addVariable(param_ast->ident, false, true);
                // 输出参数定义
                param_ast->Dump();
                if (i < params.size() - 1)
                    std::cout << ", ";
            }
        }
        return 0;
    }
};
// FuncType ::= "void" | "int";
class FuncTypeAST : public BaseAST {
public:
    std::string type;
    FuncTypeAST(std::string type_name) : type(std::move(type_name)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        if (type == "int")
            std::cout << ": i32 ";
        else if (type == "void")
            std::cout << "";
        return 0;
    }
};

// FuncRParams ::= Exp {"," Exp};
class FuncRParamsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST>> params;
    FuncRParamsAST(std::vector<std::unique_ptr<BaseAST>> p)
        : params(std::move(p)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        for (const auto &param : params) {
            param->Dump();
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
        if (has_returned)
            return 0;
        if (type == "int")
            std::cout << "i32 ";
        return 0;
    }
};

// FuncDef ::= FuncType IDENT "(" [FuncFParams] ")" Block;
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> func_params; // 可选参数列表
    std::unique_ptr<BaseAST> block;
    FuncDefAST(std::unique_ptr<BaseAST> func_type_ptr, std::string id,
               std::unique_ptr<BaseAST> params_ptr,
               std::unique_ptr<BaseAST> block_ptr)
        : func_type(std::move(func_type_ptr)), ident(std::move(id)),
          func_params(std::move(params_ptr)), block(std::move(block_ptr)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;

        // 获取返回类型
        FuncTypeAST *func_type_ast =
            dynamic_cast<FuncTypeAST *>(func_type.get());
        if (!func_type_ast) {
            std::cerr << "错误: 函数类型无效\n";
            assert(false);
        }
        std::string return_type = func_type_ast->type;

        // 获取参数类型列表
        std::vector<std::string> param_types;
        if (func_params) {
            FuncFParamsAST *params_ast =
                dynamic_cast<FuncFParamsAST *>(func_params.get());
            if (params_ast) {
                for (const auto &param : params_ast->params) {
                    FuncFParamAST *param_ast =
                        dynamic_cast<FuncFParamAST *>(param.get());
                    if (param_ast && param_ast->btype) {
                        BTypeAST *btype_ast =
                            dynamic_cast<BTypeAST *>(param_ast->btype.get());
                        if (btype_ast) {
                            param_types.push_back(btype_ast->type);
                        }
                    }
                }
            }
        }

        // 将函数添加到符号表
        symTab.addFunction(ident, return_type, param_types);

        // 输出函数定义
        std::cout << "fun @" << ident << "(";
        if (func_params) {
            func_params->Dump(); // 输出参数并将其加入符号表
        }
        std::cout << ") ";
        func_type->Dump();
        std::cout << "{\n%entry:\n";

        // 重置返回标志并输出函数体
        has_returned = 0;
        block->Dump();
        if (!has_returned) {
            std::cout << "ret" << std::endl;
        }
        std::cout << "}\n";
        return 0;
    }
};

class BlockAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST>> block_items;
    BlockAST(std::vector<std::unique_ptr<BaseAST>> items)
        : block_items(std::move(items)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        symTab.enterScope(); // 进入块作用域
        for (const auto &item : block_items) {
            if (has_returned)
                break;
            item->Dump();
        }
        symTab.exitScope(); // 退出块作用域
        return 0;
    }
};

// BlockItem ::= Decl | Stmt;
class BlockItemAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> item;
    bool is_decl; // true 表示 Decl，false 表示 Stmt
    BlockItemAST(std::unique_ptr<BaseAST> item_ptr, bool is_decl_flag)
        : item(std::move(item_ptr)), is_decl(is_decl_flag) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        return item->Dump();
    }
};

// Decl ::= ConstDecl | VarDecl; // 更新：支持变量声明
class DeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> decl; // 可以是 ConstDecl 或 VarDecl
    bool is_const;                 // true 表示 ConstDecl，false 表示 VarDecl
    DeclAST(std::unique_ptr<BaseAST> decl_ptr, bool is_const_flag)
        : decl(std::move(decl_ptr)), is_const(is_const_flag) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        decl->Dump();
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
        if (has_returned)
            return 0;
        for (const auto &def : const_defs) {
            def->Dump();
        }
        return 0;
    }
};

// VarDecl ::= BType VarDef {"," VarDef} ";"; // 新增：变量声明
class VarDeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> btype;
    std::vector<std::unique_ptr<BaseAST>> var_defs;
    VarDeclAST(std::unique_ptr<BaseAST> btype_ptr,
               std::vector<std::unique_ptr<BaseAST>> defs)
        : btype(std::move(btype_ptr)), var_defs(std::move(defs)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        for (const auto &def : var_defs) {
            def->Dump();
        }
        return 0;
    }
};

class ConstDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;
    ConstDefAST(std::string id, std::unique_ptr<BaseAST> init_val)
        : ident(std::move(id)), const_init_val(std::move(init_val)) {
        // 不再调用 symTab.addVariable
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        // 在 Dump 时添加常量到符号表
        symTab.addVariable(ident, true, false); // 添加常量
        int val_id = const_init_val->Dump();
        std::string modified_name = symTab.findVariable(ident);
        std::cout << modified_name << " = alloc i32\n";
        std::cout << "store %" << val_id << ", " << modified_name << "\n";
        return 0;
    }
};

class VarDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> init_val;
    VarDefAST(std::string id, std::unique_ptr<BaseAST> init_val_ptr = nullptr)
        : ident(std::move(id)), init_val(std::move(init_val_ptr)) {
        // 不再调用 symTab.addVariable
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        // 在 Dump 时添加变量到符号表
        symTab.addVariable(ident, false, false); // 添加非常量变量
        std::string modified_name = symTab.findVariable(ident);
        std::cout << modified_name << " = alloc i32\n";
        if (init_val) {
            int val_id = init_val->Dump();
            std::cout << "store %" << val_id << ", " << modified_name << "\n";
        }
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
        if (has_returned)
            return 0;
        return const_exp->Dump();
    }
};

// InitVal ::= Exp; // 新增：变量初始值
class InitValAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    InitValAST(std::unique_ptr<BaseAST> exp_ptr) : exp(std::move(exp_ptr)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        return exp->Dump();
    }
};

// ConstExp ::= Exp;
class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    ConstExpAST(std::unique_ptr<BaseAST> exp_ptr) : exp(std::move(exp_ptr)) {
    }
    int Dump() const override {
        if (has_returned)
            return 0;
        return exp->Dump();
    }
};