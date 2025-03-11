#pragma once
#include "ast.hpp"
#include "exp.hpp"
extern int block_counter;

class LValAST : public BaseAST {
public:
    std::string ident;
    LValAST(std::string id) : ident(std::move(id)) {
    }
    int Dump() const override {
        int nowId = TemValId;
        if (!symTab.variableExists(ident)) {
            std::cerr << "Error: Undefined variable '" << ident << "'\n";
            assert(false && "Undefined variable");
        }
        std::string modified_name = symTab.findVariable(ident);
        std::cout << "%" << TemValId++ << " = load " << modified_name << "\n";
        return nowId;
    }
};

class StmtAST : public BaseAST {
public:
    enum class StmtKind {
        ASSIGN,
        RETURN_EXP,
        RETURN_EMPTY,
        BLOCK,
        EMPTY,
        SIMPLE_EXP,
        IF,
        IF_ELSE
    };

    StmtKind kind;

    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> then_stmt;
    std::unique_ptr<BaseAST> else_stmt;
    std::unique_ptr<BaseAST> block;

    StmtAST(StmtKind k, std::unique_ptr<BaseAST> lval_ptr = nullptr,
            std::unique_ptr<BaseAST> exp_ptr = nullptr,
            std::unique_ptr<BaseAST> then_ptr = nullptr,
            std::unique_ptr<BaseAST> else_ptr = nullptr,
            std::unique_ptr<BaseAST> block_ptr = nullptr)
        : kind(k), lval(std::move(lval_ptr)), exp(std::move(exp_ptr)),
          then_stmt(std::move(then_ptr)), else_stmt(std::move(else_ptr)),
          block(std::move(block_ptr)) {
    }

    int Dump() const override {
        switch (kind) {
        case StmtKind::ASSIGN: {
            int exp_id = exp->Dump();
            LValAST *lval_ptr = dynamic_cast<LValAST *>(lval.get());
            if (!lval_ptr) {
                std::cerr << "Error: lval is not an LValAST\n";
                assert(false);
            }
            std::string modified_name = symTab.findVariable(lval_ptr->ident);
            std::cout << "store %" << exp_id << ", " << modified_name << "\n";
            return 0;
        }
        case StmtKind::RETURN_EXP: {
            int exp_id = exp->Dump();
            std::cout << "ret %" << exp_id << "\n";
            return 0;
        }
        case StmtKind::RETURN_EMPTY: {
            std::cout << "ret\n";
            return 0;
        }
        case StmtKind::BLOCK: {
            block->Dump();
            return 0;
        }
        case StmtKind::SIMPLE_EXP: {
            exp->Dump();
            return 0;
        }
        case StmtKind::EMPTY: {
            return 0;
        }
        case StmtKind::IF: {
            if (!exp || !then_stmt) {
                std::cerr << "Error: Null exp or then_stmt in IF\n";
                assert(false);
            }
            return dumpIf();
        }
        case StmtKind::IF_ELSE: {
            if (!exp || !then_stmt || !else_stmt) {
                std::cerr
                    << "Error: Null exp, then_stmt, or else_stmt in IF_ELSE\n";
                assert(false);
            }
            return dumpIfElse();
        }
        default:
            std::cerr << "Unknown StmtKind\n";
            return -1;
        }
    }

private:
    int dumpIf() const {
        int cond_id = exp->Dump();
        int then_block_id = block_counter++;
        int end_block_id = block_counter++;

        std::cout << "br %" << cond_id << ", %then_" << then_block_id
                  << ", %end_" << end_block_id << "\n";

        std::cout << "\n%then_" << then_block_id << ":\n";
        then_stmt->Dump();
        std::cout << "jump %end_" << end_block_id << "\n";

        std::cout << "\n%end_" << end_block_id << ":\n";
        return 0;
    }

    int dumpIfElse() const {
        int cond_id = exp->Dump();
        int then_block_id = block_counter++;
        int else_block_id = block_counter++;
        int end_block_id = block_counter++;

        std::cout << "br %" << cond_id << ", %then_" << then_block_id
                  << ", %else_" << else_block_id << "\n";

        std::cout << "\n%then_" << then_block_id << ":\n";
        then_stmt->Dump();
        std::cout << "jump %end_" << end_block_id << "\n";

        std::cout << "\n%else_" << else_block_id << ":\n";
        else_stmt->Dump();
        std::cout << "jump %end_" << end_block_id << "\n";

        std::cout << "\n%end_" << end_block_id << ":\n";
        return 0;
    }
};