%code requires {
  #include <memory>
  #include <string>
  #include "head/ast.hpp"
}

%{
#include <iostream>
#include <memory>
#include <string>
#include "head/ast.hpp"
int yylex();
void yyerror(std::unique_ptr<BaseAST>& ast, const char *s);
using namespace std;
%}

%parse-param {std::unique_ptr<BaseAST>& ast}

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> FuncDef FuncType Block Stmt
%type <int_val> Number

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>(unique_ptr<BaseAST>($1));
    ast = move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
FuncDef
  : FuncType IDENT '(' ')' Block {
    $$ = new FuncDefAST(
        unique_ptr<BaseAST>($1),           // func_type
        *unique_ptr<string>($2),           // ident
        unique_ptr<BaseAST>($5)            // block
    );
  }
  ;

// FuncType ::= "int";
FuncType
  : INT {
    $$ = new FuncTypeAST("int");           // 直接构造 "int" 类型
  }
  ;

Block
  : '{' Stmt '}' {
    $$ = new BlockAST(unique_ptr<BaseAST>($2));
  }
  ;

Stmt
  : RETURN Number ';' {
    $$ = new StmtAST($2);                  // Number 是 int 类型
  }
  ;

Number
  : INT_CONST {
    $$ = $1;                               // 直接传递整数值
  }
  ;

%%

void yyerror(std::unique_ptr<BaseAST>& ast, const char *s) {
  cerr << "error: " << s << endl;
}