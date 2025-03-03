#include <iostream>
#include <memory>
#include <string>

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;
    // 添加纯虚函数 print，要求所有派生类实现
    virtual void print(int indent = 0) const = 0;

protected:
    // 辅助函数，用于打印缩进
    void printIndent(int indent) const {
        for (int i = 0; i < indent; ++i) {
            std::cout << "  "; // 每个缩进级别用两个空格
        }
    }
};

// FuncType ::= "int"
class FuncTypeAST : public BaseAST {
public:
    std::string type; // 这里固定为 "int"
    FuncTypeAST() : type("int") {
    }

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "FuncTypeAST: " << type << std::endl;
    }
};

// Number ::= INT_CONST
class NumberAST : public BaseAST {
public:
    int value; // 整数常量值
    explicit NumberAST(int val) : value(val) {
    }

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "NumberAST: " << value << std::endl;
    }
};

// Stmt ::= "return" Number ";"
class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> number;
    explicit StmtAST(std::unique_ptr<BaseAST> num) : number(std::move(num)) {
    }

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "StmtAST: return" << std::endl;
        number->print(indent + 1);
    }
};

// Block ::= "{" Stmt "}"
class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;
    explicit BlockAST(std::unique_ptr<BaseAST> s) : stmt(std::move(s)) {
    }

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "BlockAST: {" << std::endl;
        stmt->print(indent + 1);
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};

// FuncDef ::= FuncType IDENT "(" ")" Block
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    FuncDefAST(std::unique_ptr<BaseAST> ft, std::string id,
               std::unique_ptr<BaseAST> b)
        : func_type(std::move(ft)), ident(std::move(id)), block(std::move(b)) {
    }

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "FuncDefAST: " << ident << std::endl;
        func_type->print(indent + 1);
        printIndent(indent + 1);
        std::cout << "Parameters: ()" << std::endl;
        block->print(indent + 1);
    }
};

// CompUnit ::= FuncDef
class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def; // 编译单元中的函数定义
    explicit CompUnitAST(std::unique_ptr<BaseAST> fd)
        : func_def(std::move(fd)) {
    }

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "CompUnitAST:" << std::endl;
        func_def->print(indent + 1);
    }
};
