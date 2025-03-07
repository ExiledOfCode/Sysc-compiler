%code requires {
  #include <memory>
  #include <string>
  #include "head/ast.hpp"  // 假设 CompUnitAST, FuncDefAST 等定义在此文件中
  #include "head/exp.hpp"  // ExpAST, UnaryExpAST 等定义在此文件中
}

%{
#include <iostream>
#include <memory>
#include <string>
#include "head/ast.hpp"
#include "head/exp.hpp"
int yylex();
void yyerror(std::unique_ptr<BaseAST>& ast, const char *s);
using namespace std;

// 定义常量提高可读性
#define IS_NUMBER true
#define IS_EXP false
%}

%parse-param {std::unique_ptr<BaseAST>& ast}

%union {
  std::string *str_val;  // 字符串指针，用于 IDENT
  int int_val;           // 整数值，用于 INT_CONST
  BaseAST *ast_val;      // AST 节点指针
}

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> FuncDef FuncType Block Stmt
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp Number

// 定义一元操作符的优先级和结合性
%right '+' '-' '!'  // 右结合性，符合一元操作符的语义

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>(unique_ptr<BaseAST>($1));
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    $$ = new FuncDefAST(
        unique_ptr<BaseAST>($1),  // FuncType 的所有权转移
        *$2,                      // IDENT 解引用
        unique_ptr<BaseAST>($5)   // Block 的所有权转移
    );
    delete $2;  // 释放 IDENT 的内存
  }
  ;

FuncType
  : INT {
    $$ = new FuncTypeAST("int");
  }
  ;

Block
  : '{' Stmt '}' {
    $$ = new BlockAST(unique_ptr<BaseAST>($2));  // Stmt 的所有权转移
  }
  ;

Stmt
  : RETURN Exp ';' {
    $$ = new StmtAST(unique_ptr<BaseAST>($2));   // Exp 的所有权转移
  }
  ;

Exp
  : UnaryExp {
    $$ = new ExpAST(unique_ptr<BaseAST>($1));    // 创建 ExpAST
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    $$ = new PrimaryExpAST(unique_ptr<BaseAST>($2), nullptr, IS_EXP);  // 括号表达式
  }
  | Number {
    $$ = new PrimaryExpAST(nullptr, unique_ptr<BaseAST>($1), IS_NUMBER);  // Number
  }
  ;

UnaryExp
  : PrimaryExp {
    $$ = new UnaryExpAST(unique_ptr<BaseAST>($1));    // 创建 UnaryExpAST (PrimaryExp)
  }
  | UnaryOp UnaryExp {
    UnaryOpAST* unary_op = dynamic_cast<UnaryOpAST*>($1);
    if (unary_op) {
      $$ = new UnaryExpAST(
          unary_op->op,              // 获取 op 的值
          unique_ptr<BaseAST>($2)    // UnaryExp 的所有权转移
      );
      delete $1;  // 释放 UnaryOpAST 的内存
    } else {
      yyerror(ast, "Invalid UnaryOpAST cast");
      $$ = nullptr;
    }
  }
  ;

UnaryOp
  : '+' {
    $$ = new UnaryOpAST("+");
  }
  | '-' {
    $$ = new UnaryOpAST("-");
  }
  | '!' {
    $$ = new UnaryOpAST("!");
  }
  ;

Number
  : INT_CONST {
    $$ = new NumberAST($1);  // 创建 NumberAST
  }
  ;

%%

void yyerror(std::unique_ptr<BaseAST>& ast, const char *s) {
  cerr << "错误: " << s << endl;
}