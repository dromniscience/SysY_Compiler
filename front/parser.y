/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *    parser.y:  Auto-generated Bottom-up Parser for SysY    *
 *************************************************************/


/*    I extend the SysY grammar in the following sense:
 *  1. Support ';' after declarations. No effect.
 */


/*    The grammar production provided below is left with 1 shift/reduce 
 *  reduction intentionally, which always groups "else" to the nearest
 *  "then" if we always choose shift rather than reduce. This greatly
 *  alleviate the burden on us to assign SDD to it. Below we provide 
 *  a set of productions which is LR and is equivalent to the non-
 *  terminal Stmt in the grammar we implement.
 *
 * Stmt		:	OpenStmt
 * 			|	CloseStmt
 * 			;
 * OpenStmt	:	IF '(' Cond ')' Stmt
 * 			|	IF '(' Cond ')' CloseStmt ELSE OpenStmt
 * 			|	WHILE '(' Cond ')' OpenStmt
 * 			;
 * CloseStmt	:	IF '(' Cond ')' CloseStmt ELSE CloseStmt
 * 				|	WHILE '(' Cond ')' CloseStmt
 * 				|	SimpleStmt
 * 				;
 * SimpleStmt	:	';'
 * 				|	LVal '=' Exp ';'
 * 				|	Exp ';'
 * 				|	Block
 * 				|	BREAK ';'
 * 				|	CONTINUE ';'
 * 				|	RETURN ';'
 * 				|	RETURN Exp ';'
 * 				;
 */


%{
#include "ast.hpp"
#include "error.hpp"
#include "geneey.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// These are intricacies when putting lex and yacc together.
// Just neglect these minor points, or nasty here.
extern FILE *yyin, *yyout;
int yylex(void);
void yyerror(ASTptr *, const char *);

%}

%union{
	TKptr tkptr;
	ASTptr astptr;
}

%define parse.error verbose
%define parse.lac full
%parse-param {ASTptr *root}

%token <tkptr> IDENT "identifier"
%token <tkptr> INT_CONST "constant"
%token <tkptr> CONST "const" INT "int" VOID "void" IF "if" ELSE "else" WHILE "while" BREAK "break" CONTINUE "continue" RETURN "return"
%token <tkptr> LE "<=" GE ">=" EQ "==" NE "!=" AND "&&" OR "||"
%token <tkptr> '+' '-' '!' '*' '/' '%' '<' '>' ',' ';' '[' ']' '=' '{' '}' '(' ')'

%type <tkptr> UnaryOp
%type <astptr> CompUnit ConstDef VarDef FuncFParam FuncDef Block Stmt Exp Cond LVal PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp ConstExp
%type <astptr> Decl ConstDefArr ConstDecl VarDefArr VarDecl FuncFParams BlockItemArr ExpArr FuncRParams ConstArr
%type <astptr> ConstInitValArr ConstInitVal InitValArr InitVal FuncDefHead BlockStart FuncDefproto WhileHead CompHead
%%
CompUnit	:	CompUnit Decl
	{
		/* CompUnit: BlockAST */
		BlockAST *p = dynamic_cast<BlockAST *>($1);
		p->Insertclauses(std::move(dynamic_cast<Vecptr>($2)->astlist));
		p->Setrange($1, $2);
		$$ = p;

		delete $2;
	}
			|	CompUnit FuncDef
	{
		/* CompUnit: BlockAST */
		dynamic_cast<BlockAST *>($1)->Insertclause($2);
		$1->Setrange($1, $2);
		$$ = $1;
	}
			|	CompUnit ';'
	{
		/* CompUnit: BlockAST */
		$$ = $1;
		delete $2;
	}
			|	CompHead Decl
	{
		/* CompUnit: BlockAST */
		BlockAST *p = new BlockAST;
		p->Insertclauses(std::move(dynamic_cast<Vecptr>($2)->astlist));
		p->Setrange($2, $2);
		*root = p;
		$$ = p;

		delete $2;
	}
			|	CompHead FuncDef
	{
		/* CompUnit: BlockAST */
		BlockAST *p = new BlockAST;
		p->Insertclause($2);
		p->Setrange($2, $2);
		*root = p;

		$$ = p;
	}
			|	CompHead ';'
	{
		/* CompUnit: BlockAST */
		$$ = new BlockAST;
		$$->Setrange($2, $2);
		*root = $$;

		delete $2;
	}
			;
CompHead	:
	{
		/* Add lib functions */
		// getint
		TKptr p = new Token("getint", IDENT);
		SYMptr q = Newfunsymentry(false, p, nullptr);
		Insertsymentry(q);
		delete p;
		
		// getch
		p = new Token("getch", IDENT);
		q = Newfunsymentry(false, p, nullptr);
		Insertsymentry(q);
		delete p;
		
		// getarray
		p = new Token("getarray", IDENT);
		q = Newfunsymentry(false, p, nullptr);
		Insertsymentry(q);

		Pushsymtab();
		TKptr r = new Token("a", IDENT);
		SYMptr s = Newvarsymentry(false, r, nullptr);
		Insertsymentry(s);
		dynamic_cast<VarSYM *>(s)->dim_pro.push_back(1);
		dynamic_cast<VarSYM *>(s)->dim_pro.push_back(0);
		dynamic_cast<FunSYM *>(q)->Insertfparams(s);
		Popsymtab();
		delete r;
		delete p;

		// putint
		p = new Token("putint", IDENT);
		q = Newfunsymentry(true, p, nullptr);
		Insertsymentry(q);

		Pushsymtab();
		r = new Token("a", IDENT);
		s = Newvarsymentry(false, r, nullptr);
		Insertsymentry(s);
		dynamic_cast<VarSYM *>(s)->dim_pro.push_back(1);
		dynamic_cast<FunSYM *>(q)->Insertfparams(s);
		Popsymtab();
		delete r;
		delete p;

		// putch
		p = new Token("putch", IDENT);
		q = Newfunsymentry(true, p, nullptr);
		Insertsymentry(q);

		Pushsymtab();
		r = new Token("a", IDENT);
		s = Newvarsymentry(false, r, nullptr);
		Insertsymentry(s);
		dynamic_cast<VarSYM *>(s)->dim_pro.push_back(1);
		dynamic_cast<FunSYM *>(q)->Insertfparams(s);
		Popsymtab();
		delete r;
		delete p;

		// putarray
		p = new Token("putarray", IDENT);
		q = Newfunsymentry(true, p, nullptr);
		Insertsymentry(q);

		Pushsymtab();
		r = new Token("a", IDENT);
		s = Newvarsymentry(false, r, nullptr);
		Insertsymentry(s);
		dynamic_cast<VarSYM *>(s)->dim_pro.push_back(1);
		dynamic_cast<FunSYM *>(q)->Insertfparams(s);
		delete r;
		r = new Token("b", IDENT);
		s = Newvarsymentry(false, r, nullptr);
		Insertsymentry(s);
		dynamic_cast<VarSYM *>(s)->dim_pro.push_back(1);
		dynamic_cast<VarSYM *>(s)->dim_pro.push_back(0);
		dynamic_cast<FunSYM *>(q)->Insertfparams(s);
		delete r;
		Popsymtab();
		delete p;

		// starttime
		p = new Token("starttime", IDENT);
		q = Newfunsymentry(true, p, nullptr);
		Insertsymentry(q);
		delete p;

		// stoptime
		p = new Token("stoptime", IDENT);
		q = Newfunsymentry(true, p, nullptr);
		Insertsymentry(q);
		delete p;
	}
			;
Decl		:	ConstDecl
			|	VarDecl
			;
ConstDefArr	:	ConstDefArr ',' ConstDef
	{
		/* ConstDefArr: VecAST */
		Vecptr p = dynamic_cast<Vecptr>($1);
		p->Insertastlist($3);
		p->Setrange($1, $3);
		$$ = p;

		delete $2;
	}
			|	ConstDef
	{
		/* ConstDefArr: VecAST */
		Vecptr p = new VecAST;
		p->Insertastlist($1);
		p->Setrange($1, $1);
		$$ = p;
	}
			;
ConstDecl	:	CONST INT ConstDefArr ';'
	{
		/* ConDecl: VecAST */
		$3->Setrange($1, $4);
		$$ = $3;
		delete $1;
		delete $2;
		delete $4;
	}
			;
ConstArr	:	ConstArr '[' ConstExp ']'
	{
		/* ConstArr: VecAST */

		/* Maintain positioning info */
		if(!$1->bgnlno)
			$1->Setrange($2, $4);
		dynamic_cast<Vecptr>($1)->Insertastlist($3);
		$1->Setrange($1, $4); 
		$$ = $1;

		delete $2;
		delete $4;
	}
			|
	{
		/* ConstArr: VecAST */
		$$ = new VecAST();
	}
			;
ConstDef	:	IDENT ConstArr '=' ConstInitVal
	{
		/* ConstDef: DeclAST */
		/* Type check */
		std::vector<int> tmp = dynamic_cast<Vecptr>($2)->Getrawarraytype();
		tmp = Arraydimprocess(tmp);

		// dynamic_cast<LTptr>($4)->Debug(0);

		if(!dynamic_cast<LTptr>($4)->Checkform(tmp)){
			// Side effect of Checkform(...)
			Reporterror(Error::Arrayinitmismatch, Encodemessage(tmp));
			errptr = nullptr;
		}
		LTTable t = dynamic_cast<LTptr>($4)->Flatten(tmp); // Move constructor

		/* new DeclAST */
		DeclAST *p = new DeclAST;
		if(tmp.size() == 1) p->Setrange($1, $1);
		else p->Setrange($1, $2);

		/* new SYM */
		SYMptr newsym = Newvarsymentry(true, $1, p);
		Insertsymentry(newsym);
		dynamic_cast<VarSYM *>(newsym)->Setarraytype(std::move(tmp));
		
		/* Update DeclSYM */
		p->sym = newsym;
		p->init = std::move(t);
		$$ = p;

		/* eeyore: endono */
		p->Setaddr(Setendoaddr());
		Increaseendono();

		delete $1;
		delete $2;
		delete $3;
		delete $4;
	}
			;
ConstInitValArr	:	ConstInitValArr ',' ConstInitVal
	{
		/* ConstInitValArr: ListTree */
		dynamic_cast<LTptr>($1)->Insertsublist($3);
		/* Maintain positioning info */
		$1->Setrange($1, $3);

		$$ = $1;
		delete $2;
	}
				|	ConstInitVal
	{
		/* ConstInitValArr: ListTree */
		LTptr tmp = new ListTree;
		tmp->Setrange($1, $1);
		tmp->Insertsublist($1);
		$$ = tmp;
	}
				;
ConstInitVal	:	ConstExp
	{
		/* ConstInitVal: ListTree */
		$$ = new ListTree($1);
	}
				|	'{' '}'
	{
		/* ConstInitVal: ListTree */
		$$ = new ListTree($1, $2);
		
		delete $1;
		delete $2;
	}
				|	'{' ConstInitValArr '}'
	{
		/* ConstInitVal: ListTree */
		$$ = $2;
		$$->Setrange($1, $3);

		delete $1;
		delete $3;
	}
				;
VarDefArr	:	VarDefArr ',' VarDef
	{
		/* VarDefArr: VecAST */
		Vecptr p = dynamic_cast<Vecptr>($1);
		p->Insertastlist($3);
		p->Setrange($1, $3);
		$$ = p;

		delete $2;
	}
			|	VarDef
	{

		/* VarDefArr: VecAST */
		Vecptr p = new VecAST;
		p->Insertastlist($1);
		p->Setrange($1, $1);
		$$ = p;
		
	}
			;
VarDecl		:	INT VarDefArr ';'
	{
		/* VarDecl: VecAST */
		$2->Setrange($1, $3);
		$$ = $2;

		delete $1;
		delete $3;
	}
			;
VarDef		:	IDENT ConstArr
	{
		/* VarDef: DeclAST */
		/* new DeclAST */
		DeclAST *p = new DeclAST;
		p->Setrange($1, $2);

		/* array type */
		std::vector<int> arrtype = dynamic_cast<Vecptr>($2)->Getrawarraytype();
		if(arrtype.empty()) p->Setrange($1, $1); // If ConstArr is empty, update positioning info
		arrtype = Arraydimprocess(arrtype);
	
		/* new SYM */
		SYMptr newsym = Newvarsymentry(false, $1, p);
		Insertsymentry(newsym);
		dynamic_cast<VarSYM *>(newsym)->Setarraytype(std::move(arrtype));

		/* Update info */
		p->sym = newsym;
		$$ = p;

		/* eeyore: endono */
		p->Setaddr(Setendoaddr());
		Increaseendono();

		delete $1;
		delete $2;
	}
			|	IDENT ConstArr '=' InitVal
	{
		/* VarDef: DeclAST */
		/* type check */
		std::vector<int> arrtype = dynamic_cast<Vecptr>($2)->Getrawarraytype();
		arrtype = Arraydimprocess(arrtype);

		if(!dynamic_cast<LTptr>($4)->Checkform(arrtype)){
			// Side effect of Checkform(...)
			Reporterror(Error::Arrayinitmismatch, Encodemessage(arrtype));
			errptr = nullptr;
		}
		LTTable t = dynamic_cast<LTptr>($4)->Flatten(arrtype);

		/* new DeclAST */
		DeclAST *p = new DeclAST;
		if(arrtype.size() == 1) p->Setrange($1, $1);
		else p->Setrange($1, $2);

		/* new SYM */
		SYMptr newsym = Newvarsymentry(false, $1, p);
		Insertsymentry(newsym);
		dynamic_cast<VarSYM *>(newsym)->Setarraytype(std::move(arrtype));
		
		/* Update DeclAST */
		p->sym = newsym;
		p->init = std::move(t);
		$$ = p;

		/* eeyore: endono */
		p->Setaddr(Setendoaddr());
		Increaseendono();

		delete $1;
		delete $2;
		delete $3;
		delete $4;
	}
			;
InitValArr	:	InitValArr ',' InitVal
	{
		/* InitValArr: ListTree */
		dynamic_cast<LTptr>($1)->Insertsublist($3);
		$1->Setrange($1, $3);
		$$ = $1;

		delete $2;
	}
			|	InitVal
	{
		/* InitValArr: ListTree */
		LTptr p = new ListTree;
		p->Setrange($1, $1);
		p->Insertsublist($1); 
		$$ = p;
	}
			;
InitVal		:	Exp
	{
		/* InitVal: ListTree */
		$$ = new ListTree($1);
		Verifyanumber($1);
	}
			|	'{' '}'
	{
		/* InitVal: ListTree */
		$$ = new ListTree($1, $2);
		delete $1;
		delete $2;
	}
			|	'{' InitValArr '}'
	{
		/* InitVal: ListTree */
		$$ = $2;
		$$->Setrange($1, $3);
		delete $1;
		delete $3;
	}
			;

FuncFParams	:	FuncFParams ',' FuncFParam
	{
		/* FuncFParams: VecAST */
		dynamic_cast<Vecptr>($1)->Insertastlist($3);
		$1->Setrange($1, $3);
		$$ = $1;

		delete $2;
	}
			|	FuncFParam
	{
		/* FuncFParams: VecAST */
		Vecptr p = new VecAST;
		p->Setrange($1, $1);
		p->Insertastlist($1);
		$$ = p;
	}
			;
FuncFParam	:	INT IDENT
	{
		/* FuncFParam: DeclAST */
		/* new DeclAST */
		DeclAST *p = new DeclAST;
		p->Setrange($1, $2);

		/* new SYM */
		SYMptr newsym = Newvarsymentry(false, $2, p);
		Insertsymentry(newsym);
		dynamic_cast<VarSYM *>(newsym)->Setarraytype(std::vector<int> (1,1));

		/* Update DeclAST */
		p->sym = newsym;
		$$ = p;

		/* eeyore: paramno */
		p->Setaddr(Setparamaddr());
		Increaseparamno();

		delete $1;
		delete $2;
	}
			|	INT IDENT '[' ']' ConstArr
	{
		/* FuncFParam: DeclAST */
		/* new DeclAST */
		DeclAST *p = new DeclAST;
		p->Setrange($1, $5);

		/* new SYM */
		SYMptr newsym = Newvarsymentry(false, $2, p);
		Insertsymentry(newsym);

		std::vector<int> arrtype = dynamic_cast<Vecptr>($5)->Getrawarraytype();
		if(arrtype.empty()) p->Setrange($1, $4);
		/* If ConstArr is empty, update positioning info */
		arrtype = Arraydimprocess(arrtype);
		arrtype.push_back(0); // pointer
		dynamic_cast<VarSYM *>(newsym)->Setarraytype(std::move(arrtype));

		p->sym = newsym;
		$$ = p;

		/* eeyore: paramno */
		p->Setaddr(Setparamaddr());
		Increaseparamno();

		delete $1;
		delete $2;
		delete $3;
		delete $4;
		delete $5;
	}
			;
FuncDef		:	FuncDefproto '{' BlockItemArr '}'
	{
		/* FuncDef: FunDefAST */
		FunDefAST *p = dynamic_cast<FunDefAST *>($1);
		p->body = $3;
		//$1->Setrange($1, $4);
		$$ = $1;

		/* Check main */
		if(*(p->sym->strptr) == "main"){
			if(dynamic_cast<FunSYM *>(p->sym)->isvoid){
				errptr = $1;
				Reporterror(Error::Voidmain);
				errptr = nullptr;
			}
			if(dynamic_cast<FunSYM *>(p->sym)->fparams.size()){
				errptr = $1;
				Reporterror(Error::Parammain);
				errptr = nullptr;
			}
			mainptr = p;
		}

		/* End of the scope */
		Popsymtab();

		/* eeyore: reset paramno */
		Resetparamno();

		delete $2;
		delete $4;
	}
			;
FuncDefproto	:	FuncDefHead ')'
	{
		/* FuncDefproto: FunDefAST */
		//$1->Setrange($1, $2);
		$$ = $1;

		delete $2;
	}
				|	FuncDefHead FuncFParams ')'
	{
		/* FuncDefproto: FunDefAST */
		//$1->Setrange($1, $3);
		FunDefAST *p = dynamic_cast<FunDefAST *>($1);
		p->fparams = dynamic_cast<Vecptr>($2)->Getfparams(); // Move assignment

		/* bind SYMs of fparams to FunDefAST */
		for(auto i: p->fparams)
			dynamic_cast<FunSYM *>(p->sym)->fparams.push_back(dynamic_cast<DeclAST *>(i)->sym);
		$$ = $1;
		
		delete $2;
		delete $3;
	}
				;
FuncDefHead	:	INT IDENT '('
	{
		/* FuncDefHead: FunDefAST */
		/* new FunDefAST */
		FunDefAST *p = new FunDefAST;
		p->Setrange($2, $2);

		/* new SYM */
		SYMptr newsym = Newfunsymentry(false, $2, p);
		Insertsymentry(newsym);

		/* Update FunDefAST */
		p->sym = newsym;
		$$ = p;

		/* New scope for its formal parameters */
		Pushsymtab();
		curfundef = p;

		/* eeyore: reset paramno */
		Resetparamno();
		/* eeyore: set funcaddr */
		p->Setaddr(Setfuncaddr(*(p->sym->strptr)));

		delete $1;
		delete $2;
		delete $3;
	}
			|	VOID IDENT '('
	{
		/* FuncDefHead: FunDefAST */
		/* new FunDefAST */
		FunDefAST *p = new FunDefAST;
		p->Setrange($2, $2);

		/* new SYM */
		SYMptr newsym = Newfunsymentry(true, $2, p);
		Insertsymentry(newsym);

		/* Update FunDefAST */
		p->sym = newsym;
		$$ = p;

		/* New scope for its formal parameters */
		Pushsymtab();
		curfundef = p;

		/* eeyore: reset paramno */
		Resetparamno();
		/* eeyore: set func addr */
		p->Setaddr(Setfuncaddr(*(p->sym->strptr)));

		delete $1;
		delete $2;
		delete $3;
	}
			;

BlockItemArr	:	BlockItemArr Decl
	{
		/* BlockItemArr: BlockAST */
		if(!$1->bgnlno)
			$1->Setrange($2, $2);
		else
			$1->Setrange($1, $2);
		BlockAST *p = dynamic_cast<BlockAST *>($1);
		p->Insertclauses(std::move(dynamic_cast<Vecptr>($2)->astlist));
		$$ = $1;

		delete $2;
	}
				|	BlockItemArr Stmt
	{
		/* BlockItemArr: BlockAST */
		if($2){
			if(!$1->bgnlno)
				$1->Setrange($2, $2);
			else
				$1->Setrange($1, $2);
		}
		BlockAST *p = dynamic_cast<BlockAST *>($1);
		p->Insertclause($2);
		$$ = $1;
	}
				|
	{
		/* BlockItemArr: BlockAST */
		$$ = new BlockAST;
	}
				;
Block		:	'{' BlockStart BlockItemArr '}'
	{
		/* BlockItemArr: BlockAST */
		Popsymtab();

		$3->Setrange($1, $4);
		$$ = $3;

		delete $1;
		delete $4;
	}
			;
BlockStart	:
	{
		/* BlockStart: null */
		Pushsymtab();
		$$ = nullptr;
	}
			;
Stmt		:	';'
	{
		/* Stmt: polymorphism */
		$$ = nullptr;
	}
			|	LVal '=' Exp ';'
	{
		/* Stmt: AssignAST */
		AssignAST *p = new AssignAST;
		p->Setrange($1, $3);

		/* Check that constant cannot be assigned */
		LvalAST *q = dynamic_cast<LvalAST *>($1);
		if(dynamic_cast<VarSYM *>(q->sym)->isconst){
			errptr = $1;
			Reporterror(Error::Assignconstant, *(q->sym->strptr), q->sym->astptr);
			errptr = nullptr;
		}

		/* Check that Lval is a single element */
		if(!q->Isanumber()){
			// Side Effect: Isanumber() will modify errptr
			Reporterror(Error::Assignarray, std::string(), q->sym->astptr);
			errptr = nullptr;
		}

		/* Check that exp is a single element */
		Verifyanumber($3);

		p->lval = $1;
		p->exp = $3;
		$$ = p;

		delete $2;
		delete $4;
	}
			|	Exp ';'
	{
		/* Stmt: polymorphism */
		if($1->type != ASTType::Funcall)
			Verifyanumber($1);
		$$ = $1;

		delete $2;
	}
			|	Block
	{
		/* Stmt: BlockAST */
		$$ = $1;
	}
			|	IF '(' Cond ')' Stmt
	{
		/* Stmt: IfAST */
		IfAST *p = new IfAST;
		if($5)
			p->Setrange($1, $5);
		else
			p->Setrange($1, $4);
		p->cond = $3;
		p->then = $5;
		$$ = p;

		delete $1;
		delete $2;
		delete $4;
	}
			|	IF '(' Cond ')' Stmt ELSE Stmt
	{
		/* Stmt: IfAST */
		IfAST *p = new IfAST;
		if($7)
			p->Setrange($1, $7);
		else
			p->Setrange($1, $6);
		p->cond = $3;
		p->then = $5;
		p->els = $7;
		$$ = p;

		delete $1;
		delete $2;
		delete $4;
		delete $6;
	}
			|	WhileHead '(' Cond ')' Stmt
	{
		/* Stmt: WhileAST */
		WhileAST *p = dynamic_cast<WhileAST *>($1);
		p->cond = $3;
		p->body = $5;
		if($5)
			p->Setrange($1, $5);
		else
			p->Setrange($1, $4);
		$$ = p;

		/* Pop whilestk */
		Popwhilestk();

		delete $2;
		delete $4;
	}
			|	BREAK ';'
	{
		/* Stmt: BreakAST */
		if(Isemptywhilestk()){
			errptr = new BaseAST($1->lno, $1->bgn, $1->lno, $1->end);
			Reporterror(Error::Nowhiletoescape, "break");
			delete errptr;
			errptr = nullptr;
			$$ = nullptr;
		}
		else{
			BreakAST *p = new BreakAST;
			p->Setrange($1, $1);
			$$ = p;
		}

		delete $1;
		delete $2;
	}
			|	CONTINUE ';'
	{
		/* Stmt: ContAST */

		/* No while to continue */
		if(Isemptywhilestk()){
			errptr = new BaseAST($1->lno, $1->bgn, $1->lno, $1->end);
			Reporterror(Error::Nowhiletoescape, "continue");
			delete errptr;
			errptr = nullptr;
			$$ = nullptr;
		}
		else{
			ContAST *p = new ContAST;
			p->Setrange($1, $1);
			$$ = p;
		}

		delete $1;
		delete $2;
	}
			|	RETURN ';'
	{
		/* Stmt: RetAST */
		RetAST *p = new RetAST;
		p->Setrange($1, $2);

		/* Check ret value */
		FunSYM *q = dynamic_cast<FunSYM *>(dynamic_cast<FunDefAST *>(p->back)->sym);
		if(!q->isvoid){
			errptr = p;
			ASTptr t = new BaseAST(q->lno, q->bgn, q->lno, q->end);
			Reporterror(Error::Noreturnvalue, *(q->strptr), t);
			delete t;
			errptr = nullptr;
			/* return 0 by default */
			TKptr r = new Token(0, INT_CONST);
			BinaryAST *s = new BinaryAST(r);
			delete r;
			p->exp = s;
		}

		$$ = p;

		delete $1;
		delete $2;
	}
			|	RETURN Exp ';'
	{
		/* Stmt: RetAST */
		RetAST *p = new RetAST;
		p->Setrange($1, $3);

		/* Check ret value */
		FunSYM *q = dynamic_cast<FunSYM *>(dynamic_cast<FunDefAST *>(p->back)->sym);
		if(q->isvoid){
			errptr = p;
			ASTptr t = new BaseAST(q->lno, q->bgn, q->lno, q->end);
			Reporterror(Error::Returnavalue, *(q->strptr), t);
			delete t;
			errptr = nullptr;
			/* Neglect Exp, since compilation will terminate eventually */
			delete $2;
		}
		else p->exp = $2;
		$$ = p;

		delete $1;
		delete $3;
	}
			;
WhileHead	:	WHILE
	{
		/* WhileHead: WhileAST */
		WhileAST *p = new WhileAST;
		p->Setrange($1, $1);
		$$ = p;

		/* Update whilestk */
		Pushwhilestk(p);

		delete $1;
	}
			;

Exp			:	AddExp
	{
		/* Exp: polymorphism (BinaryAST, UnaryAST, FunCallAST, LvalAST) */
		$1->Eval();
		$$ = $1;
	}
			;
Cond		:	LOrExp
	{
		/* Cond: polymorphism */
		$1->Eval();
		$$ = $1;
		Verifyanumber($1);
	}
			;
ExpArr		:	ExpArr '[' Exp ']'
	{
		/* ExpArr: VecAST */
		if(!$1->bgnlno)
			$1->Setrange($2, $4);
		else $1->Setrange($1, $4);

		dynamic_cast<Vecptr>($1)->Insertastlist($3);
		$$ = $1;

		delete $2;
		delete $4;
	}
			|
	{
		/* ExpArr: VecAST */
		$$ = new VecAST;
	}
			;
LVal		:	IDENT ExpArr
	{
		/* LVal: LvalAST */
		$$ = new LvalAST($1, $2);
		if(!$2->bgnlno)
			$$->Setrange($1, $1);

		delete $1;
		delete $2;
	}
			;
PrimaryExp	:	'(' Exp ')'
	{
		/* PrimaryExp: polymorphism */
		$$ = $2;
		$$->Setrange($1, $3);

		delete $1;
		delete $3;
	}
			|	LVal
			|	INT_CONST
	{
		/* PrimaryExp: polymorphism */
		$$ = new BinaryAST($1);
		delete $1;
	}
			;
FuncRParams	:	FuncRParams ',' Exp
	{
		/* FuncRParams: VecAST */
		dynamic_cast<VecAST *>($1)->Insertastlist($3);
		$1->Setrange($1, $3);
		
		delete $2;
	}
			|	Exp
	{
		/* FuncRParams: VecAST */
		VecAST *p = new VecAST;
		p->Setrange($1, $1);
		p->Insertastlist($1);
		$$ = p;
	}
			;
UnaryExp	:	PrimaryExp
			|	IDENT '(' ')'
	{
		/* UnaryExp: polymorphism */
		
		FunCallAST *ptr = new FunCallAST($1);

		ptr->Setrange($1, $3);
		$$ = ptr;

		/* Do the params match? */
		ptr->Verifyfuncall();

		delete $1;
		delete $2;
		delete $3;
	}
			|	IDENT '(' FuncRParams ')'
	{
		/* UnaryExp: polymorphism */

		FunCallAST *ptr = new FunCallAST($1);

		ptr->rparams = dynamic_cast<Vecptr>($3)->Getrparams(); // Move Constructor
		ptr->Setrange($1, $4);
		$$ = ptr;

		/* Check the params list */
		ptr->Verifyfuncall();
		
		delete $1;
		delete $2;
		delete $3;
		delete $4;
	}
			|	UnaryOp UnaryExp
	{
		/* UnaryExp: polymorphism */

		/* Only when UnaryExp is a unaryexp should coalesce be called */
		if($2->type == ASTType::Unary) {
			dynamic_cast<UnaryAST *>($2)->Coalesce($1);
			$2->Setrange($1, $2);
			$$ = $2;
		}
		/* Only when UnaryExp is not a unaryexp should constructor be called */
		else $$ = new UnaryAST($2, $1);
		
		delete $1;
	}
			;
UnaryOp		:	'+'
			|	'-'
			|	'!'
			;
MulExp		:	UnaryExp
			|	MulExp '*' UnaryExp
	{
		/* MulExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			|	MulExp '/' UnaryExp
	{
		/* MulExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			|	MulExp '%' UnaryExp
	{
		/* MulExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			;
AddExp		:	MulExp
			|	AddExp '+' MulExp
	{
		/* AddExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			|	AddExp '-' MulExp
	{
		/* AddExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			;
RelExp		:	AddExp
			|	RelExp '<' AddExp
	{
		/* RelExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			|	RelExp '>' AddExp
	{
		/* RelExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			|	RelExp LE AddExp
	{
		/* RelExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			|	RelExp GE AddExp
	{
		/* RelExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			;
EqExp		:	RelExp
			|	EqExp EQ RelExp
	{
		/* EqExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			|	EqExp NE RelExp
	{
		/* EqExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			;
LAndExp		:	EqExp
			|	LAndExp AND EqExp
	{
		/* LAndExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			;
LOrExp		:	LAndExp
			|	LOrExp OR LAndExp
	{
		/* LOrExp: polymorphism */
		$$ = new BinaryAST($1, $3, $2);
		delete $2;
	}
			;
ConstExp	:	AddExp
	{
		/* ConstExp: polymorphism */

		$1->Eval();
		if(!$1->Isconst()) {
			errptr = $1;
			Reporterror(Error::Notconstexp);
			errptr = nullptr;
		}
		$$ = $1;
	}
			;

%%

/*** Supporting Routines ***/
void yyerror(ASTptr *root, const char *str){
	std::string tmp = str;
	//std::transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);
	errptr = new BaseAST(lineno, tokenpos - tokenwidth, lineno, tokenpos);
	Reporterror(Error::Invalidsyntax, tmp);
	if(errptr) {delete errptr; errptr = nullptr;}
}