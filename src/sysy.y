%code requires {
  #include <memory>
  #include <string>
  #include "head/ast.hpp"  // 假设 CompUnitAST, FuncDefAST 等定义在此文件中
  #include "head/exp.hpp"  // ExpAST, UnaryExpAST, AddExpAST 等定义在此文件中
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
#define IS_UNARY true
#define IS_MUL true
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

%type <ast_val> CompUnit FuncDef FuncType Block Stmt
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp Number
%type <ast_val> AddExp MulExp

// 定义操作符的优先级和结合性
%left '+' '-'          // 加减法，左结合
%left '*' '/' '%'      // 乘除模，左结合，优先级高于加减
%right UNARY_OP        // 一元操作符，右结合，优先级最高

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
    delete $2;  // 释放 IDENT 的内存,因为$2已经深拷贝了，所以可以释放
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
  : AddExp {
    $$ = new ExpAST(unique_ptr<BaseAST>($1));    // 创建 ExpAST，包装 AddExp
  }
  ;

AddExp
  : MulExp {
    $$ = new AddExpAST(unique_ptr<BaseAST>($1));  // 单一 MulExp
  }
  | AddExp '+' MulExp {
    $$ = new AddExpAST(
        unique_ptr<BaseAST>($1),       // 左侧 AddExp
        "+",                           // 操作符
        unique_ptr<BaseAST>($3)        // 右侧 MulExp
    );
  }
  | AddExp '-' MulExp {
    $$ = new AddExpAST(
        unique_ptr<BaseAST>($1),       // 左侧 AddExp
        "-",                           // 操作符
        unique_ptr<BaseAST>($3)        // 右侧 MulExp
    );
  }
  ;

MulExp
  : UnaryExp {
    $$ = new MulExpAST(unique_ptr<BaseAST>($1));  // 单一 UnaryExp
  }
  | MulExp '*' UnaryExp {
    $$ = new MulExpAST(
        unique_ptr<BaseAST>($1),       // 左侧 MulExp
        "*",                           // 操作符
        unique_ptr<BaseAST>($3)        // 右侧 UnaryExp
    );
  }
  | MulExp '/' UnaryExp {
    $$ = new MulExpAST(
        unique_ptr<BaseAST>($1),       // 左侧 MulExp
        "/",                           // 操作符
        unique_ptr<BaseAST>($3)        // 右侧 UnaryExp
    );
  }
  | MulExp '%' UnaryExp {
    $$ = new MulExpAST(
        unique_ptr<BaseAST>($1),       // 左侧 MulExp
        "%",                           // 操作符
        unique_ptr<BaseAST>($3)        // 右侧 UnaryExp
    );
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
  | UnaryOp UnaryExp %prec UNARY_OP {
    UnaryOpAST* unary_op = dynamic_cast<UnaryOpAST*>($1);
    if (unary_op) {
      $$ = new UnaryExpAST(
          unary_op->op,              // 获取 op 的值
          unique_ptr<BaseAST>($2)    // UnaryExp 的所有权转移
      );
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