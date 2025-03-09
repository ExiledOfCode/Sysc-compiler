%code requires {
  #include <memory>
  #include <string>
  #include <vector>
  #include "head/ast.hpp"  // 包含AST定义
  #include "head/exp.hpp"  // 包含表达式定义
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

// 定义常量以提高代码可读性
#define IS_EXP 0    // PrimaryExp是表达式
#define IS_LVAL 1   // PrimaryExp是左值
#define IS_NUMBER 2 // PrimaryExp是数字
#define IS_DECL true // BlockItem是声明
#define IS_STMT false // BlockItem是语句
%}

// 定义解析参数，传递AST的根节点
%parse-param {std::unique_ptr<BaseAST>& ast}

// 定义联合类型，用于存储不同类型的值
%union {
  std::string *str_val;  // 字符串指针，用于标识符 (IDENT)
  int int_val;           // 整数值，用于整数常量 (INT_CONST)
  BaseAST *ast_val;      // AST节点指针，用于语法树节点
  std::vector<std::unique_ptr<BaseAST>> *vec_ast_val; // 用于多个ConstDef或BlockItemList
}

// 定义终结符 (token)
%token INT RETURN CONST        // 关键字：int, return, const
%token AND OR                  // 逻辑运算符：&& 和 ||
%token EQ NE LT GT LE GE       // 比较运算符：==, !=, <, >, <=, >=
%token <str_val> IDENT         // 标识符
%token <int_val> INT_CONST     // 整数常量

// 定义非终结符的类型
%type <ast_val> CompUnit FuncDef FuncType Block Stmt Decl ConstDecl BType ConstDef ConstInitVal
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp Number LVal ConstExp BlockItem
%type <ast_val> AddExp MulExp RelExp EqExp LAndExp LOrExp
%type <vec_ast_val> BlockItemList ConstDefList

// 定义操作符的优先级和结合性（从低到高）
%left OR                 // 逻辑或，最低优先级，左结合
%left AND                // 逻辑与，左结合
%left EQ NE              // 相等性比较，左结合
%left LT GT LE GE        // 关系运算符，左结合
%left '+' '-'            // 加减法，左结合
%left '*' '/' '%'        // 乘除模，左结合
%right UNARY_OP          // 一元操作符，右结合，优先级最高

%%

// 编译单元：整个程序的入口
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>(unique_ptr<BaseAST>($1));
    ast = move(comp_unit);
  }
  ;

// 函数定义
FuncDef
  : FuncType IDENT '(' ')' Block {
    $$ = new FuncDefAST(unique_ptr<BaseAST>($1), *$2, unique_ptr<BaseAST>($5));
    delete $2;
  }
  ;

// 函数类型
FuncType
  : INT {
    $$ = new FuncTypeAST("int");
  }
  ;

// 函数体
Block
  : '{' BlockItemList '}' {
    $$ = new BlockAST(std::move(*$2));
    delete $2;
  }
  ;

// 块项列表
BlockItemList
  : /* empty */ {
    $$ = new vector<unique_ptr<BaseAST>>();
  }
  | BlockItemList BlockItem {
    $1->push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  ;

// 块项
BlockItem
  : Decl {
    $$ = new BlockItemAST(unique_ptr<BaseAST>($1), IS_DECL);
  }
  | Stmt {
    $$ = new BlockItemAST(unique_ptr<BaseAST>($1), IS_STMT);
  }
  ;

// 声明
Decl
  : ConstDecl {
    $$ = new DeclAST(unique_ptr<BaseAST>($1));
  }
  ;

// 常量声明
ConstDecl
  : CONST BType ConstDefList ';' {
    $$ = new ConstDeclAST(unique_ptr<BaseAST>($2), std::move(*$3));
    delete $3;
  }
  ;

// 基本类型
BType
  : INT {
    $$ = new BTypeAST("int");
  }
  ;

// 常量定义列表
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

// 常量定义
ConstDef
  : IDENT '=' ConstInitVal {
    $$ = new ConstDefAST(*$1, unique_ptr<BaseAST>($3));
    delete $1;
  }
  ;

// 常量初始值
ConstInitVal
  : ConstExp {
    $$ = new ConstInitValAST(unique_ptr<BaseAST>($1));
  }
  ;

// 语句
Stmt
  : RETURN Exp ';' {
    $$ = new StmtAST(unique_ptr<BaseAST>($2));
  }
  ;

// 表达式
Exp
  : LOrExp {
    $$ = new ExpAST(unique_ptr<BaseAST>($1));
  }
  ;

// 逻辑或表达式
LOrExp
  : LAndExp {
    $$ = new LOrExpAST(unique_ptr<BaseAST>($1));
  }
  | LOrExp OR LAndExp {
    $$ = new LOrExpAST(unique_ptr<BaseAST>($1), unique_ptr<BaseAST>($3));
  }
  ;

// 逻辑与表达式
LAndExp
  : EqExp {
    $$ = new LAndExpAST(unique_ptr<BaseAST>($1));
  }
  | LAndExp AND EqExp {
    $$ = new LAndExpAST(unique_ptr<BaseAST>($1), unique_ptr<BaseAST>($3));
  }
  ;

// 相等性表达式
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

// 关系表达式
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

// 加法表达式
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

// 乘法表达式
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

// 初级表达式
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

// 左值
LVal
  : IDENT {
    $$ = new LValAST(*$1);
    delete $1;
  }
  ;

// 一元表达式
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

// 一元运算符
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

// 数字
Number
  : INT_CONST {
    $$ = new NumberAST($1);
  }
  ;

// 常量表达式
ConstExp
  : Exp {
    $$ = new ConstExpAST(unique_ptr<BaseAST>($1));
  }
  ;

%%

// 错误处理函数
void yyerror(std::unique_ptr<BaseAST>& ast, const char *s) {
  cerr << "错误: " << s << endl;
}