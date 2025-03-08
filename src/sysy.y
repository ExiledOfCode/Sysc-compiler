%code requires {
  #include <memory>
  #include <string>
  #include "head/ast.hpp"  // 包含 CompUnitAST, FuncDefAST 等定义的头文件
  #include "head/exp.hpp"  // 包含 ExpAST, UnaryExpAST, AddExpAST 等定义的头文件
}

%{
#include <iostream>
#include <memory>
#include <string>
#include "head/ast.hpp"
#include "head/exp.hpp"

// 声明外部词法分析函数
int yylex();
void yyerror(std::unique_ptr<BaseAST>& ast, const char *s);

using namespace std;

// 定义常量以提高代码可读性
#define IS_NUMBER true    // 表示 PrimaryExp 是数字
#define IS_EXP false      // 表示 PrimaryExp 是表达式
#define IS_UNARY true     // 表示 MulExp 是单一的 UnaryExp
#define IS_MUL true       // 表示 AddExp 是单一的 MulExp
%}

// 定义解析参数，传递 AST 的根节点
%parse-param {std::unique_ptr<BaseAST>& ast}

// 定义联合类型，用于存储不同类型的值
%union {
  std::string *str_val;  // 字符串指针，用于标识符 (IDENT)
  int int_val;           // 整数值，用于整数常量 (INT_CONST)
  BaseAST *ast_val;      // AST 节点指针，用于语法树节点
}

// 定义终结符 (token)
%token INT RETURN        // 关键字：int 和 return
%token AND OR            // 逻辑运算符：&& 和 ||
%token EQ NE LT GT LE GE // 比较运算符：==, !=, <, >, <=, >=
%token <str_val> IDENT   // 标识符
%token <int_val> INT_CONST  // 整数常量

// 定义非终结符的类型
%type <ast_val> CompUnit FuncDef FuncType Block Stmt
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp Number
%type <ast_val> AddExp MulExp RelExp EqExp LAndExp LOrExp

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
  : '{' Stmt '}' {
    $$ = new BlockAST(unique_ptr<BaseAST>($2));
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
    $$ = new PrimaryExpAST(unique_ptr<BaseAST>($2), nullptr, IS_EXP);
  }
  | Number {
    $$ = new PrimaryExpAST(nullptr, unique_ptr<BaseAST>($1), IS_NUMBER);
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

%%

// 错误处理函数
void yyerror(std::unique_ptr<BaseAST>& ast, const char *s) {
  cerr << "错误: " << s << endl;
}