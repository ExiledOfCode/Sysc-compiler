%code requires {
  #include <memory>
  #include <string>
  #include <vector>
  #include "head/ast.hpp"  // 包含 AST 定义
  #include "head/exp.hpp"  // 包含表达式定义
  #include "head/stmt.hpp"
}

%{
#include <iostream>
#include <memory>
#include <string>
#include "head/ast.hpp"
#include "head/exp.hpp"
#include "head/stmt.hpp"

int yylex();
void yyerror(std::unique_ptr<BaseAST>& ast, const char *s);

using namespace std;

#define IS_EXP 0    // PrimaryExp 是表达式
#define IS_LVAL 1   // PrimaryExp 是左值
#define IS_NUMBER 2 // PrimaryExp 是数字
#define IS_DECL true // BlockItem 是声明
#define IS_STMT false // BlockItem 是语句
%}

%parse-param {std::unique_ptr<BaseAST>& ast}

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  std::vector<std::unique_ptr<BaseAST>> *vec_ast_val;
}

%token INT RETURN CONST
%token AND OR
%token EQ NE LT GT LE GE
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> CompUnit FuncDef FuncType Block Stmt Decl ConstDecl VarDecl BType 
%type <ast_val> ConstDef ConstInitVal VarDef InitVal Exp PrimaryExp UnaryExp UnaryOp 
%type <ast_val> Number LVal ConstExp BlockItem AddExp MulExp RelExp EqExp LAndExp LOrExp
%type <vec_ast_val> BlockItemList ConstDefList VarDefList

%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left '+' '-'
%left '*' '/' '%'
%right UNARY_OP

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>(unique_ptr<BaseAST>($1));
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    $$ = new FuncDefAST(unique_ptr<BaseAST>($1), *$2, unique_ptr<BaseAST>($5));
    delete $2;
  }
  ;

FuncType
  : INT {
    $$ = new FuncTypeAST("int");
  }
  ;

Block
  : '{' BlockItemList '}' {
    $$ = new BlockAST(std::move(*$2));
    delete $2;
  }
  ;

BlockItemList
  : /* empty */ {
    $$ = new vector<unique_ptr<BaseAST>>();
  }
  | BlockItemList BlockItem {
    $1->push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  ;

BlockItem
  : Decl {
    $$ = new BlockItemAST(unique_ptr<BaseAST>($1), IS_DECL);
  }
  | Stmt {
    $$ = new BlockItemAST(unique_ptr<BaseAST>($1), IS_STMT);
  }
  ;

Decl
  : ConstDecl {
    $$ = new DeclAST(unique_ptr<BaseAST>($1), true);
  }
  | VarDecl {
    $$ = new DeclAST(unique_ptr<BaseAST>($1), false);
  }
  ;

ConstDecl
  : CONST BType ConstDefList ';' {
    $$ = new ConstDeclAST(unique_ptr<BaseAST>($2), std::move(*$3));
    delete $3;
  }
  ;

VarDecl
  : BType VarDefList ';' {
    $$ = new VarDeclAST(unique_ptr<BaseAST>($1), std::move(*$2));
    delete $2;
  }
  ;

BType
  : INT {
    $$ = new BTypeAST("int");
  }
  ;

ConstDefList
  : ConstDef {
    $$ = new vector<unique_ptr<BaseAST>>();
    $$->push_back(unique_ptr<BaseAST>($1));
  }
  | ConstDefList ',' ConstDef {
    $1->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

VarDefList
  : VarDef {
    $$ = new vector<unique_ptr<BaseAST>>();
    $$->push_back(unique_ptr<BaseAST>($1));
  }
  | VarDefList ',' VarDef {
    $1->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    $$ = new ConstDefAST(*$1, unique_ptr<BaseAST>($3));
    delete $1;
  }
  ;

VarDef
  : IDENT {
    $$ = new VarDefAST(*$1);
    delete $1;
  }
  | IDENT '=' InitVal {
    $$ = new VarDefAST(*$1, unique_ptr<BaseAST>($3));
    delete $1;
  }
  ;

ConstInitVal
  : ConstExp {
    $$ = new ConstInitValAST(unique_ptr<BaseAST>($1));
  }
  ;

InitVal
  : Exp {
    $$ = new InitValAST(unique_ptr<BaseAST>($1));
  }
  ;
  
Stmt
  : LVal '=' Exp ';' {
      $$ = new StmtAST(StmtAST::StmtKind::ASSIGN,
                       std::unique_ptr<BaseAST>($1),  // lval
                       std::unique_ptr<BaseAST>($3),  // exp
                       nullptr);                      // block
    }
  | Block {
      $$ = new StmtAST(StmtAST::StmtKind::BLOCK,
                       nullptr,                       // lval
                       nullptr,                       // exp
                       std::unique_ptr<BaseAST>($1)); // block
    }
  | RETURN Exp ';' {
      $$ = new StmtAST(StmtAST::StmtKind::RETURN_EXP,
                       nullptr,                       // lval
                       std::unique_ptr<BaseAST>($2),  // exp
                       nullptr);                      // block
    }
  | RETURN ';' {
      $$ = new StmtAST(StmtAST::StmtKind::RETURN_EMPTY,
                       nullptr,                       // lval
                       nullptr,                       // exp
                       nullptr);                      // block
    }
  | ';' {
      $$ = new StmtAST(StmtAST::StmtKind::EMPTY,
                       nullptr,                       // lval
                       nullptr,                       // exp
                       nullptr);                      // block
    }
  ;

Exp
  : LOrExp {
    $$ = new ExpAST(unique_ptr<BaseAST>($1));
  }
  ;

LOrExp
  : LAndExp {
    $$ = new LOrExpAST(unique_ptr<BaseAST>($1));
  }
  | LOrExp OR LAndExp {
    $$ = new LOrExpAST(unique_ptr<BaseAST>($1), unique_ptr<BaseAST>($3));
  }
  ;

LAndExp
  : EqExp {
    $$ = new LAndExpAST(unique_ptr<BaseAST>($1));
  }
  | LAndExp AND EqExp {
    $$ = new LAndExpAST(unique_ptr<BaseAST>($1), unique_ptr<BaseAST>($3));
  }
  ;

EqExp
  : RelExp {
    $$ = new EqExpAST(unique_ptr<BaseAST>($1));
  }
  | EqExp EQ RelExp {
    $$ = new EqExpAST(unique_ptr<BaseAST>($1), "==", unique_ptr<BaseAST>($3));
  }
  | EqExp NE RelExp {
    $$ = new EqExpAST(unique_ptr<BaseAST>($1), "!=", unique_ptr<BaseAST>($3));
  }
  ;

RelExp
  : AddExp {
    $$ = new RelExpAST(unique_ptr<BaseAST>($1));
  }
  | RelExp LT AddExp {
    $$ = new RelExpAST(unique_ptr<BaseAST>($1), "<", unique_ptr<BaseAST>($3));
  }
  | RelExp GT AddExp {
    $$ = new RelExpAST(unique_ptr<BaseAST>($1), ">", unique_ptr<BaseAST>($3));
  }
  | RelExp LE AddExp {
    $$ = new RelExpAST(unique_ptr<BaseAST>($1), "<=", unique_ptr<BaseAST>($3));
  }
  | RelExp GE AddExp {
    $$ = new RelExpAST(unique_ptr<BaseAST>($1), ">=", unique_ptr<BaseAST>($3));
  }
  ;

AddExp
  : MulExp {
    $$ = new AddExpAST(unique_ptr<BaseAST>($1));
  }
  | AddExp '+' MulExp {
    $$ = new AddExpAST(unique_ptr<BaseAST>($1), "+", unique_ptr<BaseAST>($3));
  }
  | AddExp '-' MulExp {
    $$ = new AddExpAST(unique_ptr<BaseAST>($1), "-", unique_ptr<BaseAST>($3));
  }
  ;

MulExp
  : UnaryExp {
    $$ = new MulExpAST(unique_ptr<BaseAST>($1));
  }
  | MulExp '*' UnaryExp {
    $$ = new MulExpAST(unique_ptr<BaseAST>($1), "*", unique_ptr<BaseAST>($3));
  }
  | MulExp '/' UnaryExp {
    $$ = new MulExpAST(unique_ptr<BaseAST>($1), "/", unique_ptr<BaseAST>($3));
  }
  | MulExp '%' UnaryExp {
    $$ = new MulExpAST(unique_ptr<BaseAST>($1), "%", unique_ptr<BaseAST>($3));
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    $$ = new PrimaryExpAST(unique_ptr<BaseAST>($2), nullptr, nullptr, PrimaryExpAST::EXP);
  }
  | LVal {
    $$ = new PrimaryExpAST(nullptr, unique_ptr<BaseAST>($1), nullptr, PrimaryExpAST::LVAL);
  }
  | Number {
    $$ = new PrimaryExpAST(nullptr, nullptr, unique_ptr<BaseAST>($1), PrimaryExpAST::NUMBER);
  }
  ;

LVal
  : IDENT {
    $$ = new LValAST(*$1);
    delete $1;
  }
  ;

UnaryExp
  : PrimaryExp {
    $$ = new UnaryExpAST(unique_ptr<BaseAST>($1));
  }
  | UnaryOp UnaryExp %prec UNARY_OP {
    UnaryOpAST* unary_op = dynamic_cast<UnaryOpAST*>($1);
    if (unary_op) {
      $$ = new UnaryExpAST(unary_op->op, unique_ptr<BaseAST>($2));
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
    $$ = new NumberAST($1);
  }
  ;

ConstExp
  : Exp {
    $$ = new ConstExpAST(unique_ptr<BaseAST>($1));
  }
  ;

%%

void yyerror(std::unique_ptr<BaseAST>& ast, const char *s) {
  cerr << "错误: " << s << endl;
}