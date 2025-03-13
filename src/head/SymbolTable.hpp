#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

extern int
    TemValId; // 声明全局变量，用于生成临时变量 ID，必须在某个 .cpp 文件中定义

class SymbolTable {
public:
    // 符号结构体：存储变量的修改后名称和常量属性
    struct Symbol {
        std::string name; // 修改后的名称，如 @ident_层 或 @ident_层_const
        bool is_const;    // 是否为常量
        Symbol(std::string n, bool c) : name(std::move(n)), is_const(c) {
        }
    };

    // 循环上下文结构体：存储循环的条件块和结束块 ID
    struct LoopContext {
        int cond_block_id; // 条件块 ID
        int end_block_id;  // 结束块 ID
    };

    // 函数结构体：存储函数名称、返回类型和参数类型列表
    struct Function {
        std::string name;                     // 函数名
        std::string return_type;              // 返回类型 ("int" 或 "void")
        std::vector<std::string> param_types; // 参数类型列表
        Function(std::string n, std::string rt, std::vector<std::string> pt)
            : name(std::move(n)), return_type(std::move(rt)),
              param_types(std::move(pt)) {
        }
    };

private:
    std::vector<std::unordered_map<std::string, Symbol>> table; // 变量符号表栈
    std::unordered_map<std::string, Function> functions;        // 函数符号表
    int current_level;                   // 当前作用域层级
    std::vector<LoopContext> loop_stack; // 循环上下文栈

public:
    // 构造函数：初始化符号表
    SymbolTable() : current_level(-1) {
        table.emplace_back(); // 添加一个空的作用域作为全局层
        current_level = 0;    // 设置当前层级为 0
    }

    // 进入新作用域
    void enterScope() {
        table.emplace_back(); // 在栈顶添加一个新的空符号表
        current_level++;      // 层级加 1
    }

    // 退出当前作用域
    void exitScope() {
        if (current_level > 0) { // 确保不会退出全局作用域
            table.pop_back();    // 移除栈顶符号表
            current_level--;     // 层级减 1
        } else {
            std::cerr << "错误: 尝试退出全局作用域\n";
        }
    }

    // 添加变量到符号表
    void addVariable(const std::string &ident, bool is_const) {
        // 生成修改后的变量名，例如 @x_0 或 @x_0_const
        std::string modified_name =
            "@" + ident + "_" + std::to_string(TemValId);
        if (is_const) {
            modified_name += "_const"; // 常量变量名后缀
        }

        // 检查当前作用域是否已存在同名变量
        if (table[current_level].find(ident) != table[current_level].end()) {
            std::cerr << "错误: 变量 '" << ident << "' 在层级 " << TemValId
                      << " 已存在\n";
        }

        // 添加变量到当前作用域的符号表
        table[current_level].emplace(ident, Symbol(modified_name, is_const));
    }

    // 查找变量，返回修改后的名称
    std::string findVariable(const std::string &ident) const {
        // 从当前作用域向全局作用域逐层查找
        for (int i = current_level; i >= 0; --i) {
            auto it = table[i].find(ident);
            if (it != table[i].end()) {
                return it->second.name;
            }
        }
        std::cerr << "错误: 变量 '" << ident << "' 未找到\n";
        return ""; // 理论上不会到达这里，因 assert 会终止程序
    }

    // 检查变量是否存在
    bool variableExists(const std::string &ident) const {
        // 从当前作用域向全局作用域逐层检查
        for (int i = current_level; i >= 0; --i) {
            if (table[i].find(ident) != table[i].end()) {
                return true;
            }
        }
        return false;
    }

    // 添加函数到符号表
    void addFunction(const std::string &name, const std::string &return_type,
                     const std::vector<std::string> &param_types) {
        // 检查函数是否已存在
        if (functions.find(name) != functions.end()) {
            std::cerr << "错误: 函数 '" << name << "' 已定义\n";
        }

        // 添加函数到函数表
        functions.emplace(name, Function(name, return_type, param_types));
    }

    // 查找函数，返回函数信息
    Function findFunction(const std::string &name) const {
        auto it = functions.find(name);
        if (it == functions.end()) {
            std::cerr << "错误: 函数 '" << name << "' 未定义\n";
        }
        return it->second;
    }

    // 检查函数是否存在
    bool functionExists(const std::string &name) const {
        return functions.find(name) != functions.end();
    }

    // 进入循环作用域
    void enterLoop(int cond_id, int end_id) {
        loop_stack.push_back({cond_id, end_id}); // 添加循环上下文
    }

    // 退出循环作用域
    void exitLoop() {
        if (!loop_stack.empty()) {
            loop_stack.pop_back(); // 移除栈顶循环上下文
        } else {
            std::cerr << "错误: 尝试退出不存在的循环\n";
        }
    }

    // 获取当前循环上下文
    LoopContext getCurrentLoop() const {
        if (loop_stack.empty()) {
            std::cerr << "错误: break/continue 在循环外使用\n";
        }
        return loop_stack.back(); // 返回栈顶循环上下文
    }

    // 检查是否在循环中
    bool inLoop() const {
        return !loop_stack.empty(); // 如果循环栈不为空，则在循环中
    }
};