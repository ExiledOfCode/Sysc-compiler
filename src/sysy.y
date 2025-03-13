%code requires {
  #include <memory>
  #include <string>
  #include <vector>
  #include "head/SymbolTable.hpp"
  #include "head/ast.hpp"  // 包含 AST 定义
  #include "head/exp.hpp"  // 包含表达式定义
  #include "head/stmt.hpp"
}

%{
#include <iostream>
#include <memory>
#include <string>
#include "head/SymbolTable.hpp"
#include "head/ast.hpp"
#include "head/exp.hpp"
#include "head/stmt.hpp"

int yylex();
void yyerror(std::unique_ptr<BaseAST>& ast, const char *s);

// 全局调试开关
bool flag = 0;  // 默认关闭调试输出，可在外部设置为 true 开启

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

%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token AND OR
%token EQ NE LT GT LE GE
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> CompUnit FuncDefs FuncDef FuncType Block Decl ConstDecl VarDecl BType
%type <ast_val> ConstDef ConstInitVal VarDef InitVal Exp PrimaryExp UnaryExp UnaryOp
%type <ast_val> Number LVal ConstExp BlockItem AddExp MulExp RelExp EqExp LAndExp LOrExp
%type <ast_val> Stmt OpenStmt ClosedStmt FuncFParam
%type <vec_ast_val> BlockItemList ConstDefList VarDefList  FuncFParams FuncRParams
%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left '+' '-'
%left '*' '/' '%'
%right UNARY_OP

%%

// CompUnit ::= FuncDefs;
CompUnit
  : FuncDefs {
    if (flag) cerr << "解析 CompUnit: 函数定义列表" << endl;
    auto comp_unit = make_unique<CompUnitAST>(unique_ptr<BaseAST>($1));
    ast = move(comp_unit);
  }
  ;

// FuncDefs ::= {FuncDef};
FuncDefs
  : /* empty */ {
    if (flag) cerr << "解析 FuncDefs: 空函数列表" << endl;
    $$ = new FuncDefsAST(make_unique<vector<unique_ptr<BaseAST>>>());
  }
  | FuncDefs FuncDef {
    if (flag) cerr << "解析 FuncDefs: 添加函数定义，总数 " << dynamic_cast<FuncDefsAST*>($1)->func_defs->size() + 1 << endl;
    dynamic_cast<FuncDefsAST*>($1)->func_defs->push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  ;

// FuncDef ::= FuncType IDENT "(" [FuncFParams] ")" Block;
FuncDef
  : FuncType IDENT '(' ')' Block {
    if (flag) cerr << "解析 FuncDef: " << *$2 << " 无参数" << endl;
    $$ = new FuncDefAST(unique_ptr<BaseAST>($1), *$2, nullptr, unique_ptr<BaseAST>($5));
    delete $2;
  }
  | FuncType IDENT '(' FuncFParams ')' Block {
    if (flag) cerr << "解析 FuncDef: " << *$2 << " 带有参数" << endl;
    $$ = new FuncDefAST(unique_ptr<BaseAST>($1), *$2, unique_ptr<BaseAST>(new FuncFParamsAST(std::move(*$4))), unique_ptr<BaseAST>($6));
    delete $2;
  }
  ;

// FuncType ::= "void" | "int";
FuncType
  : INT {
    if (flag) cerr << "解析 FuncType: int" << endl;
    $$ = new FuncTypeAST("int");
  }
  | VOID {
    if (flag) cerr << "解析 FuncType: void" << endl;
    $$ = new FuncTypeAST("void");
  }
  ;

// FuncFParams ::= FuncFParam {"," FuncFParam};
FuncFParams
  : FuncFParam {
    if (flag) cerr << "解析 FuncFParams: 单一参数" << endl;
    $$ = new vector<unique_ptr<BaseAST>>();
    $$->push_back(unique_ptr<BaseAST>($1));
  }
  | FuncFParams ',' FuncFParam {
    if (flag) cerr << "解析 FuncFParams: 添加参数，总数 " << $1->size() + 1 << endl;
    $1->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

// FuncFParam ::= BType IDENT;
FuncFParam
  : BType IDENT {
    if (flag) cerr << "解析 FuncFParam: " << *$2 << endl;
    $$ = new FuncFParamAST(unique_ptr<BaseAST>($1), *$2);
    delete $2;
  }
  ;

Block
  : '{' BlockItemList '}' {
    if (flag) cerr << "Parsed Block: Block with " << $2->size() << " items" << endl;
    $$ = new BlockAST(std::move(*$2));
  }
  ;

BlockItemList
  : /* empty */ {
    if (flag) cerr << "Parsed BlockItemList: empty" << endl;
    $$ = new vector<unique_ptr<BaseAST>>();
  }
  | BlockItemList BlockItem {
    if (flag) cerr << "Parsed BlockItemList: added item, total " << $1->size() + 1 << " items" << endl;
    $1->push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  }
  ;

BlockItem
  : Decl {
    if (flag) cerr << "Parsed BlockItem: Declaration" << endl;
    $$ = new BlockItemAST(unique_ptr<BaseAST>($1), IS_DECL);
  }
  | Stmt {
    if (flag) cerr << "Parsed BlockItem: Statement" << endl;
    $$ = new BlockItemAST(unique_ptr<BaseAST>($1), IS_STMT);
  }
  ;

Decl
  : ConstDecl {
    if (flag) cerr << "Parsed Decl: Const Declaration" << endl;
    $$ = new DeclAST(unique_ptr<BaseAST>($1), true);
  }
  | VarDecl {
    if (flag) cerr << "Parsed Decl: Variable Declaration" << endl;
    $$ = new DeclAST(unique_ptr<BaseAST>($1), false);
  }
  ;

ConstDecl
  : CONST BType ConstDefList ';' {
    if (flag) cerr << "Parsed ConstDecl: const " << dynamic_cast<BTypeAST*>($2)->type << " with " << $3->size() << " definitions" << endl;
    $$ = new ConstDeclAST(unique_ptr<BaseAST>($2), std::move(*$3));
  }
  ;

VarDecl
  : BType VarDefList ';' {
    if (flag) cerr << "Parsed VarDecl: " << dynamic_cast<BTypeAST*>($1)->type << " with " << $2->size() << " definitions" << endl;
    $$ = new VarDeclAST(unique_ptr<BaseAST>($1), std::move(*$2));
  }
  ;

BType
  : INT {
    if (flag) cerr << "Parsed BType: int" << endl;
    $$ = new BTypeAST("int");
  }
  ;

ConstDefList
  : ConstDef {
    if (flag) cerr << "Parsed ConstDefList: single definition" << endl;
    $$ = new vector<unique_ptr<BaseAST>>();
    $$->push_back(unique_ptr<BaseAST>($1));
  }
  | ConstDefList ',' ConstDef {
    if (flag) cerr << "Parsed ConstDefList: added definition, total " << $1->size() + 1 << " definitions" << endl;
    $1->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

VarDefList
  : VarDef {
    if (flag) cerr << "Parsed VarDefList: single definition" << endl;
    $$ = new vector<unique_ptr<BaseAST>>();
    $$->push_back(unique_ptr<BaseAST>($1));
  }
  | VarDefList ',' VarDef {
    if (flag) cerr << "Parsed VarDefList: added definition, total " << $1->size() + 1 << " definitions" << endl;
    $1->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    if (flag) cerr << "Parsed ConstDef: " << *$1 << " = ConstInitVal" << endl;
    $$ = new ConstDefAST(*$1, unique_ptr<BaseAST>($3));
  }
  ;

VarDef
  : IDENT {
    if (flag) cerr << "Parsed VarDef: " << *$1 << endl;
    $$ = new VarDefAST(*$1);
  }
  | IDENT '=' InitVal {
    if (flag) cerr << "Parsed VarDef: " << *$1 << " = InitVal" << endl;
    $$ = new VarDefAST(*$1, unique_ptr<BaseAST>($3));
  }
  ;

ConstInitVal
  : ConstExp {
    if (flag) cerr << "Parsed ConstInitVal: ConstExp" << endl;
    $$ = new ConstInitValAST(unique_ptr<BaseAST>($1));
  }
  ;

InitVal
  : Exp {
    if (flag) cerr << "Parsed InitVal: Exp" << endl;
    $$ = new InitValAST(unique_ptr<BaseAST>($1));
  }
  ;

Stmt
  : OpenStmt
  | ClosedStmt
  | BREAK ';' {
    if (flag) cerr << "Parsed Stmt: Break" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::BREAK,
                     nullptr,                    // lval
                     nullptr,                    // exp
                     nullptr,                    // then_stmt
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  | CONTINUE ';' {
    if (flag) cerr << "Parsed Stmt: Continue" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::CONTINUE,
                     nullptr,                    // lval
                     nullptr,                    // exp
                     nullptr,                    // then_stmt
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt {
    if (flag) cerr << "Parsed Stmt: If without else" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::IF,
                     nullptr,                    // lval
                     std::unique_ptr<BaseAST>($3), // exp
                     std::unique_ptr<BaseAST>($5), // then_stmt
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  | WHILE '(' Exp ')' Stmt {  // 新增 while 语句解析
    if (flag) cerr << "Parsed Stmt: While" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::WHILE,  // 添加 WHILE 类型
                     nullptr,                    // lval
                     std::unique_ptr<BaseAST>($3), // exp (条件)
                     std::unique_ptr<BaseAST>($5), // then_stmt (循环体)
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  ;

ClosedStmt
  : IF '(' Exp ')' ClosedStmt ELSE Stmt {
    if (flag) cerr << "Parsed Stmt: If with else" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::IF_ELSE,
                     nullptr,                    // lval
                     std::unique_ptr<BaseAST>($3), // exp
                     std::unique_ptr<BaseAST>($5), // then_stmt
                     std::unique_ptr<BaseAST>($7), // else_stmt
                     nullptr);                   // block
  }
  | LVal '=' Exp ';' {
    if (flag) cerr << "Parsed Stmt: Assignment" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::ASSIGN,
                     std::unique_ptr<BaseAST>($1), // lval
                     std::unique_ptr<BaseAST>($3), // exp
                     nullptr,                    // then_stmt
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  | Block {
    if (flag) cerr << "Parsed Stmt: Block" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::BLOCK,
                     nullptr,                    // lval
                     nullptr,                    // exp
                     nullptr,                    // then_stmt
                     nullptr,                    // else_stmt
                     std::unique_ptr<BaseAST>($1)); // block
  }
  | RETURN Exp ';' {
    if (flag) cerr << "Parsed Stmt: Return with Expression" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::RETURN_EXP,
                     nullptr,                    // lval
                     std::unique_ptr<BaseAST>($2), // exp
                     nullptr,                    // then_stmt
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  | RETURN ';' {
    if (flag) cerr << "Parsed Stmt: Return Empty" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::RETURN_EMPTY,
                     nullptr,                    // lval
                     nullptr,                    // exp
                     nullptr,                    // then_stmt
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  | Exp ';' {
    if (flag) cerr << "Parsed Stmt: Simple Exp" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::SIMPLE_EXP,
                     nullptr,                    // lval
                     std::unique_ptr<BaseAST>($1), // exp
                     nullptr,                    // then_stmt
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  | ';' {
    if (flag) cerr << "Parsed Stmt: Empty" << endl;
    $$ = new StmtAST(StmtAST::StmtKind::EMPTY,
                     nullptr,                    // lval
                     nullptr,                    // exp
                     nullptr,                    // then_stmt
                     nullptr,                    // else_stmt
                     nullptr);                   // block
  }
  ;

Exp
  : LOrExp {
    if (flag) cerr << "Parsed Exp: LOrExp" << endl;
    $$ = new ExpAST(unique_ptr<BaseAST>($1));
  }
  ;

LOrExp
  : LAndExp {
    if (flag) cerr << "Parsed LOrExp: Single LAndExp" << endl;
    $$ = new LOrExpAST(unique_ptr<BaseAST>($1));
  }
  | LOrExp OR LAndExp {
    if (flag) cerr << "Parsed LOrExp: || operation" << endl;
    $$ = new LOrExpAST(unique_ptr<BaseAST>($1), unique_ptr<BaseAST>($3));
  }
  ;

LAndExp
  : EqExp {
    if (flag) cerr << "Parsed LAndExp: Single EqExp" << endl;
    $$ = new LAndExpAST(unique_ptr<BaseAST>($1));
  }
  | LAndExp AND EqExp {
    if (flag) cerr << "Parsed LAndExp: && operation" << endl;
    $$ = new LAndExpAST(unique_ptr<BaseAST>($1), unique_ptr<BaseAST>($3));
  }
  ;

EqExp
  : RelExp {
    if (flag) cerr << "Parsed EqExp: Single RelExp" << endl;
    $$ = new EqExpAST(unique_ptr<BaseAST>($1));
  }
  | EqExp EQ RelExp {
    if (flag) cerr << "Parsed EqExp: == operation" << endl;
    $$ = new EqExpAST(unique_ptr<BaseAST>($1), "==", unique_ptr<BaseAST>($3));
  }
  | EqExp NE RelExp {
    if (flag) cerr << "Parsed EqExp: != operation" << endl;
    $$ = new EqExpAST(unique_ptr<BaseAST>($1), "!=", unique_ptr<BaseAST>($3));
  }
  ;

RelExp
  : AddExp {
    if (flag) cerr << "Parsed RelExp: Single AddExp" << endl;
    $$ = new RelExpAST(unique_ptr<BaseAST>($1));
  }
  | RelExp LT AddExp {
    if (flag) cerr << "Parsed RelExp: < operation" << endl;
    $$ = new RelExpAST(unique_ptr<BaseAST>($1), "<", unique_ptr<BaseAST>($3));
  }
  | RelExp GT AddExp {
    if (flag) cerr << "Parsed RelExp: > operation" << endl;
    $$ = new RelExpAST(unique_ptr<BaseAST>($1), ">", unique_ptr<BaseAST>($3));
  }
  | RelExp LE AddExp {
    if (flag) cerr << "Parsed RelExp: <= operation" << endl;
    $$ = new RelExpAST(unique_ptr<BaseAST>($1), "<=", unique_ptr<BaseAST>($3));
  }
  | RelExp GE AddExp {
    if (flag) cerr << "Parsed RelExp: >= operation" << endl;
    $$ = new RelExpAST(unique_ptr<BaseAST>($1), ">=", unique_ptr<BaseAST>($3));
  }
  ;

AddExp
  : MulExp {
    if (flag) cerr << "Parsed AddExp: Single MulExp" << endl;
    $$ = new AddExpAST(unique_ptr<BaseAST>($1));
  }
  | AddExp '+' MulExp {
    if (flag) cerr << "Parsed AddExp: + operation" << endl;
    $$ = new AddExpAST(unique_ptr<BaseAST>($1), "+", unique_ptr<BaseAST>($3));
  }
  | AddExp '-' MulExp {
    if (flag) cerr << "Parsed AddExp: - operation" << endl;
    $$ = new AddExpAST(unique_ptr<BaseAST>($1), "-", unique_ptr<BaseAST>($3));
  }
  ;

MulExp
  : UnaryExp {
    if (flag) cerr << "Parsed MulExp: Single UnaryExp" << endl;
    $$ = new MulExpAST(unique_ptr<BaseAST>($1));
  }
  | MulExp '*' UnaryExp {
    if (flag) cerr << "Parsed MulExp: * operation" << endl;
    $$ = new MulExpAST(unique_ptr<BaseAST>($1), "*", unique_ptr<BaseAST>($3));
  }
  | MulExp '/' UnaryExp {
    if (flag) cerr << "Parsed MulExp: / operation" << endl;
    $$ = new MulExpAST(unique_ptr<BaseAST>($1), "/", unique_ptr<BaseAST>($3));
  }
  | MulExp '%' UnaryExp {
    if (flag) cerr << "Parsed MulExp: % operation" << endl;
    $$ = new MulExpAST(unique_ptr<BaseAST>($1), "%", unique_ptr<BaseAST>($3));
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    if (flag) cerr << "Parsed PrimaryExp: (Exp)" << endl;
    $$ = new PrimaryExpAST(unique_ptr<BaseAST>($2), nullptr, nullptr, PrimaryExpAST::EXP);
  }
  | LVal {
    if (flag) cerr << "Parsed PrimaryExp: LVal" << endl;
    $$ = new PrimaryExpAST(nullptr, unique_ptr<BaseAST>($1), nullptr, PrimaryExpAST::LVAL);
  }
  | Number {
    if (flag) cerr << "Parsed PrimaryExp: Number" << endl;
    $$ = new PrimaryExpAST(nullptr, nullptr, unique_ptr<BaseAST>($1), PrimaryExpAST::NUMBER);
  }
  ;

LVal
  : IDENT {
    if (flag) cerr << "Parsed LVal: " << *$1 << endl;
    $$ = new LValAST(*$1);
    delete $1;
  }
  ;

// UnaryExp ::= ... | IDENT "(" [FuncRParams] ")" | ...;
UnaryExp
  : PrimaryExp {
    if (flag) cerr << "解析 UnaryExp: PrimaryExp" << endl;
    $$ = new UnaryExpAST(UnaryExpAST::StmtKind::PRIMARY, nullptr, nullptr, unique_ptr<BaseAST>($1));
  }
  | UnaryOp UnaryExp %prec UNARY_OP {
    UnaryOpAST* unary_op = dynamic_cast<UnaryOpAST*>($1);
    if (flag) cerr << "解析 UnaryExp: 一元运算符 " << unary_op->op << endl;
    $$ = new UnaryExpAST(UnaryExpAST::StmtKind::UNARY_OP, unique_ptr<BaseAST>($1), unique_ptr<BaseAST>($2), nullptr);
  }
  | IDENT '(' ')' {
    if (flag) cerr << "解析 UnaryExp: 函数调用 " << *$1 << " 无参数" << endl;
    $$ = new UnaryExpAST(UnaryExpAST::StmtKind::CALL, nullptr, nullptr, nullptr, *$1, nullptr);
    delete $1;
  }
  | IDENT '(' FuncRParams ')' {
    if (flag) cerr << "解析 UnaryExp: 函数调用 " << *$1 << " 有参数" << endl;
    $$ = new UnaryExpAST(UnaryExpAST::StmtKind::CALL, nullptr, nullptr, nullptr, *$1, unique_ptr<BaseAST>(new FuncRParamsAST(std::move(*$3))));
    delete $1;
  }
  ;

// FuncRParams ::= Exp {"," Exp};
FuncRParams
  : Exp {
    if (flag) cerr << "解析 FuncRParams: 单一实参" << endl;
    $$ = new vector<unique_ptr<BaseAST>>();
    $$->push_back(unique_ptr<BaseAST>($1));
  }
  | FuncRParams ',' Exp {
    if (flag) cerr << "解析 FuncRParams: 添加实参，总数 " << $1->size() + 1 << endl;
    $1->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  }
  ;

UnaryOp
  : '+' {
    if (flag) cerr << "Parsed UnaryOp: +" << endl;
    $$ = new UnaryOpAST("+");
  }
  | '-' {
    if (flag) cerr << "Parsed UnaryOp: -" << endl;
    $$ = new UnaryOpAST("-");
  }
  | '!' {
    if (flag) cerr << "Parsed UnaryOp: !" << endl;
    $$ = new UnaryOpAST("!");
  }
  ;

Number
  : INT_CONST {
    if (flag) cerr << "Parsed Number: " << $1 << endl;
    $$ = new NumberAST($1);
  }
  ;

ConstExp
  : Exp {
    if (flag) cerr << "Parsed ConstExp: Exp" << endl;
    $$ = new ConstExpAST(unique_ptr<BaseAST>($1));
  }
  ;

%%

void yyerror(std::unique_ptr<BaseAST>& ast, const char *s) {
  cerr << "错误: " << s << endl;
}