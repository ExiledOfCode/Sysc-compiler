#pragma once
#include "ast.hpp"
#include "exp.hpp"

// Stmt ::= "return" Exp ";" | LVal "=" Exp ";"; // 更新：支持赋值语句
class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;  // 用于 return 语句的表达式
    std::unique_ptr<BaseAST> lval; // 用于赋值语句的左值
    bool is_return;                // true 表示 return，false 表示赋值
    StmtAST(std::unique_ptr<BaseAST> exp_ptr)
        : exp(std::move(exp_ptr)), lval(nullptr), is_return(true) {
    }
    StmtAST(std::unique_ptr<BaseAST> lval_ptr, std::unique_ptr<BaseAST> exp_ptr)
        : exp(std::move(exp_ptr)), lval(std::move(lval_ptr)), is_return(false) {
    }
    int Dump() const override {
        if (is_return) {
            int exp_id = exp->Dump();
            std::cout << "ret %" << exp_id << "\n";
            return 0;
        } else {
            int exp_id = exp->Dump();
            std::cout << "store %" << exp_id << ", @"
                      << dynamic_cast<LValAST *>(lval.get())->ident << "\n";
            return 0;
        }
    }
};