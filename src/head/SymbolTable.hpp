#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
extern int TemValId; // 声明全局变量，不初始化
class SymbolTable {
public:
    struct Symbol {
        std::string name; // 修改后的名称，如 @ident_层 或 @ident_层_const
        bool is_const;    // 是否为常量
        Symbol(std::string n, bool c) : name(std::move(n)), is_const(c) {
        }
    };
    struct LoopContext {
        int cond_block_id; // 条件块 ID
        int end_block_id;  // 结束块 ID
    };

private:
    std::vector<std::unordered_map<std::string, Symbol>> table; // 符号表栈
    int current_level;                                          // 当前层级
    std::vector<LoopContext> loop_stack;                        // 循环上下文栈
public:
    SymbolTable() : current_level(-1) {
        // 初始化全局层
        table.emplace_back();
        current_level = 0;
    }

    // 进入新作用域
    void enterScope() {
        table.emplace_back();
        current_level++;
    }

    // 退出当前作用域
    void exitScope() {
        if (current_level > 0) {
            table.pop_back();
            current_level--;
        }
    }

    // 添加变量
    void addVariable(const std::string &ident, bool is_const) {
        std::string modified_name =
            "@" + ident + "_" + std::to_string(TemValId);
        if (is_const)
            modified_name += "_const";

        if (table[current_level].find(ident) != table[current_level].end()) {
            std::cerr << "Error: Variable '" << ident << "' conflicts at level "
                      << TemValId << "\n";
            assert(false && "Error: Variable name conflicts with existing "
                            "constant or variable");
        }

        table[current_level].emplace(ident, Symbol(modified_name, is_const));
    }

    // 查找变量，返回修改后的名称
    std::string findVariable(const std::string &ident) const {
        for (int i = current_level; i >= 0; --i) {
            auto it = table[i].find(ident);
            if (it != table[i].end()) {
                return it->second.name;
            }
        }
        assert(false && "Error: Variable not found");
        return "";
    }

    // 检查变量是否存在
    bool variableExists(const std::string &ident) const {
        for (int i = current_level; i >= 0; --i) {
            if (table[i].find(ident) != table[i].end()) {
                return true;
            }
        }
        return false;
    }

    void enterLoop(int cond_id, int end_id) {
        loop_stack.push_back({cond_id, end_id});
    }

    void exitLoop() {
        if (!loop_stack.empty()) {
            loop_stack.pop_back();
        }
    }

    LoopContext getCurrentLoop() const {
        if (loop_stack.empty()) {
            std::cerr << "Error: break/continue outside of loop\n";
            assert(false && "No loop context available");
        }
        return loop_stack.back();
    }

    bool inLoop() const {
        return !loop_stack.empty();
    }
};