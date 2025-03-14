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
        if (has_returned)
            return 0;
        int nowId = TemValId;
        if (!symTab.variableExists(ident)) {
            std::cerr << "Error: Undefined variable '" << ident << "'\n";
            assert(false && "Undefined variable");
        }
        std::string modified_name = symTab.findVariable(ident);
        if (modified_name[0] == '@') // 是一个局部变量（局部变量都是地址）
            std::cout << "%" << TemValId++ << " = load " << modified_name
                      << "\n";
        else // 是一个临时变量（临时变量都是数）
            std::cout << "%" << TemValId++ << " = add 0, " << modified_name
                      << "\n";
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
        IF_ELSE,
        WHILE,
        BREAK,
        CONTINUE
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
        if (has_returned)
            return 0;
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
            has_returned = 1;
            return 0;
        }
        case StmtKind::RETURN_EMPTY: {
            std::cout << "ret\n";
            has_returned = 1;
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
        case StmtKind::WHILE: { // 新增 WHILE 处理
            if (!exp || !then_stmt) {
                std::cerr << "Error: Null exp or then_stmt in WHILE\n";
                assert(false);
            }
            return dumpWhile();
        }
        case StmtKind::BREAK: {
            return dumpBreak();
        }
        case StmtKind::CONTINUE: {
            return dumpContinue();
        }
        default:
            std::cerr << "Unknown StmtKind\n";
            return -1;
        }
    }

private:
    int dumpIf() const {
        if (has_returned)
            return 0;
        int cond_id = exp->Dump();
        int then_block_id = block_counter;
        int end_block_id = block_counter++;

        std::cout << "br %" << cond_id << ", %then_" << then_block_id
                  << ", %end_" << end_block_id << "\n";

        std::cout << "\n%then_" << then_block_id << ":\n";
        then_stmt->Dump();
        if (!has_returned)
            std::cout << "jump %end_" << end_block_id << "\n";

        has_returned = 0;
        std::cout << "\n%end_" << end_block_id << ":\n";
        return 0;
    }

    int dumpIfElse() const {
        if (has_returned)
            return 0;
        int cond_id = exp->Dump();
        int then_block_id = block_counter;
        int else_block_id = block_counter;
        int end_block_id = block_counter++;

        std::cout << "br %" << cond_id << ", %then_" << then_block_id
                  << ", %else_" << else_block_id << "\n";

        has_returned = 0;
        std::cout << "\n%then_" << then_block_id << ":\n";
        then_stmt->Dump();
        int has1 = has_returned;
        if (!has1)
            std::cout << "jump %end_" << end_block_id << "\n";

        has_returned = 0;
        std::cout << "\n%else_" << else_block_id << ":\n";
        else_stmt->Dump();
        int has2 = has_returned;
        if (!has2)
            std::cout << "jump %end_" << end_block_id << "\n";

        if (has1 && has2)
            return 0;
        has_returned = 0;
        std::cout << "\n%end_" << end_block_id << ":\n";
        return 0;
    }

    int dumpWhile() const {
        if (has_returned)
            return 0;
        int cond_block_id = block_counter;
        int body_block_id = block_counter;
        int end_block_id = block_counter++;

        // 进入循环上下文
        symTab.enterLoop(cond_block_id, end_block_id);

        std::cout << "jump %cond_" << cond_block_id << "\n";

        std::cout << "\n%cond_" << cond_block_id << ":\n";
        int cond_id = exp->Dump();
        std::cout << "br %" << cond_id << ", %body_" << body_block_id
                  << ", %end_" << end_block_id << "\n";

        std::cout << "\n%body_" << body_block_id << ":\n";
        then_stmt->Dump();
        if (!has_returned)
            std::cout << "jump %cond_" << cond_block_id << "\n";
        // 退出循环上下文
        symTab.exitLoop();

        has_returned = 0;
        std::cout << "\n%end_" << end_block_id << ":\n";
        return 0;
    }

    int dumpBreak() const {
        if (!symTab.inLoop()) {
            std::cerr << "Error: break statement outside of loop\n";
            assert(false);
        }
        auto loop = symTab.getCurrentLoop();
        if (has_returned)
            return 0;
        std::cout << "jump %end_" << loop.end_block_id << "\n";
        // break 在循环中和return相似，也是必须退出这个域后才能输出后续语句
        has_returned = 1;
        return 0;
    }

    int dumpContinue() const {
        if (!symTab.inLoop()) {
            std::cerr << "Error: continue statement outside of loop\n";
            assert(false);
        }
        auto loop = symTab.getCurrentLoop();
        if (has_returned)
            return 0;
        std::cout << "jump %cond_" << loop.cond_block_id << "\n";
        // continue 在循环中和return相似，也是必须退出这个域后才能输出后续语句
        has_returned = 1;
        return 0;
    }
};