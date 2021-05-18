/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *                 ast.hpp:  Classes For AST                 *
 *************************************************************/

#pragma once
#ifndef SYSY_AST_HPP_DR_
#define SYSY_AST_HPP_DR_

#include "token.hpp"
#include "baseast.hpp"
#include "symtab.hpp"
#include "flow.hpp"
#include <vector>
#include <map>

using LTTable = std::map<int, ASTptr>;

class ListTree;
class VecAST;
class FunDefAST;
class LvalAST;
using LTptr = ListTree *;
using Vecptr = VecAST *;

extern ASTptr mainptr;

class BinaryAST: public BaseAST{
public:
	BinaryAST(TKptr p);
	BinaryAST(ASTptr lch, ASTptr rch, TKptr op);
	~BinaryAST();

	void Eval();
	bool Isconst() {return isconst;}
	int Cval() {return cval;}
	// Side Effect: On error, it modifies errptr
	bool Isanumber();

	ASTptr Lexp() {return lexp;}
	ASTptr Rexp() {return rexp;}
	int Op() {return op;}
	
	void Generator();

	bool isconst; // Does the tree stand for a const expression?
	bool isevaled; // Has the node been evaled?
	int op;
	int cval; // Evaluation
	ASTptr lexp, rexp;
};

class UnaryAST: public BaseAST{
public:
	UnaryAST(ASTptr ch, TKptr op);
	~UnaryAST();
	void Coalesce(TKptr p);

	void Eval();
	bool Isconst() {return isconst;}
	bool Isanumber();
	int Cval() {return cval;}

	ASTptr Exp() {return exp;}
	int Op() {return op;}

	void Generator();

	bool isconst; // Does the tree stand for a const expression?
	bool isevaled; // Has the node been evaled?
	int op; // Any sequence of + - and ! can be reduced to one that has no more than 3 unary ops
	int cval; // Evaluation
	ASTptr exp;
};

class FunCallAST: public BaseAST{
public:
	FunCallAST(TKptr);
	~FunCallAST(); // Destroy all the real params

	// Check whether the param list match
	// Side Effect: On error, it modifies errptr
	bool Checkparamlistsize();
	void Verifyfuncall();

	bool Isanumber();
	bool Isconst() {return false;}
	void Eval() {return;}

	void Generator();

	SYMptr sym;
	std::vector<ASTptr> rparams;
};

class LvalAST: public BaseAST{
public:
	LvalAST(TKptr p, ASTptr q);
	~LvalAST(); // Destroy only the entries in ind

	bool Checkiftoomanyindices();
	bool Checknegindex();
	bool Checkbigindex();

	bool Isanumber();
	void Eval();
	bool Isconst() {return isconst;}
	int Cval() {return cval;}

	void Generator();

	SYMptr sym;
	std::vector<ASTptr> indices;
	bool isconst;
	bool isevaled; // Has the node been evaled?
	int cval;

	/* eeyore: left value */
	bool isleft;
};

class BlockAST: public BaseAST{
public:
	BlockAST(): BaseAST(0, 0, 0, 0, ASTType::Block), bno(blockno) {}
	~BlockAST() {for(auto i: clauses) if(i) delete i; clauses.clear();}
	void Insertclause(ASTptr p) {if(p) clauses.push_back(p);}
	void Insertclauses(std::vector<ASTptr> &&p) {for(auto i: p) clauses.push_back(i);}

	void Generator();

	const int bno;
	std::vector<ASTptr> clauses;
};

class DeclAST: public BaseAST{
public:
	DeclAST():BaseAST(0, 0, 0, 0, ASTType::Decl), sym(nullptr) {}
	~DeclAST(); // Destroy only the entries in the initialization table

	ASTptr Getinitvalue(int index);

	void Generator();

	SYMptr sym;
	LTTable init;
};

class FunDefAST: public BaseAST{
public:
	FunDefAST():BaseAST(0, 0, 0, 0, ASTType::Fundef), sym(nullptr), body(nullptr) {}
	~FunDefAST(); // Destroy AST nodes for fparams and function body

	void Generator();
	void Dump2file();

	SYMptr sym;
	std::vector<ASTptr> fparams;
	ASTptr body;
};

class AssignAST: public BaseAST{
public:
	AssignAST(): BaseAST(0, 0, 0, 0, ASTType::Assign), lval(nullptr), exp(nullptr) {}
	~AssignAST() {if(lval) {delete lval; lval = nullptr;} if(exp) {delete exp; exp = nullptr;}}

	void Generator();

	ASTptr lval, exp;
};

class IfAST: public BaseAST{
public:
	IfAST():BaseAST(0, 0, 0, 0, ASTType::If), cond(nullptr), then(nullptr), els(nullptr) {}
	~IfAST() {if(cond) {delete cond; cond = nullptr;} if(then) {delete then; then = nullptr;} if(els) {delete els; els = nullptr;}}

	void Generator();

	ASTptr cond, then, els;
};

class WhileAST: public BaseAST{
public:
	WhileAST():BaseAST(0, 0, 0, 0, ASTType::While), cond(nullptr), body(nullptr) {}
	~WhileAST() {if(cond) {delete cond; cond = nullptr;} if(body) {delete body; body = nullptr;}}

	void Generator();

	ASTptr cond, body;
};

class BreakAST: public BaseAST{
public:
	BreakAST():BaseAST(0, 0, 0, 0, ASTType::Break), back(Topwhilestk()) {}
	~BreakAST() {}

	void Generator();
	
	ASTptr back;
};

class ContAST: public BaseAST{
public:
	ContAST():BaseAST(0, 0, 0, 0, ASTType::Continue), back(Topwhilestk()) {}
	~ContAST() {}

	void Generator();

	ASTptr back;
};

class RetAST: public BaseAST{
public:
	RetAST():BaseAST(0, 0, 0, 0, ASTType::Return), back(curfundef), exp(nullptr) {}
	~RetAST() {if(exp) delete exp; exp = nullptr;}

	void Generator();

	ASTptr back;
	ASTptr exp;
};


// This is a tool class, whose job is to collect an array of AST.
// What's more, its positioning info should be maintained by hand.
class VecAST: public BaseAST{
public:
	VecAST():BaseAST(0, 0, 0, 0, ASTType::Vec) {}
	~VecAST() {} // Do not release the AST nodes that it contains.

	void Insertastlist(ASTptr p);
	std::vector<int> Getrawarraytype(); // automatically release the AST Tree below
	std::vector<ASTptr> Getfparams(); // Never release the AST node below
	std::vector<ASTptr> Getrparams(); // Never release the ASt node below
	std::vector<ASTptr> Getindices(); // Never release the AST node below

	std::vector<ASTptr> &Astlist() {return astlist;}

	std::vector<ASTptr> astlist;
};

// This is a tool class, whose job is to build up the list tree for list initializor.
// The positioning need to be maintained by hand sometimes.
class ListTree: public BaseAST {
public:
	//
	ListTree():BaseAST(0, 0, 0, 0, ASTType::LT), \
	astptr(nullptr) {} 
	// '{' '}', create a null list
	ListTree(TKptr lbr, TKptr rbr):BaseAST(lbr->lno, lbr->bgn, rbr->lno, rbr->end, ASTType::LT), \
	astptr(nullptr) {}
	// Create a leaf node.
	// Do not confuse it with one that initialize the object with the first element of sublist_
	ListTree(ASTptr astptr):BaseAST(astptr->bgnlno, astptr->bgnpos, astptr->endlno, astptr->endpos, ASTType::LT), \
	astptr(astptr) {}	
	// Only release ListTree nodes, since AST nodes must be flattened into a map.
	~ListTree() {
		if(!_Isleaf_()) {
			for(auto p: sublist) delete p;
			sublist.clear();
		}
	}

	// Insert sub-list
	void Insertsublist(ASTptr p);
	// Check whether the list initialization confroms to predeclared array form
	// Side effect: modify errptr on error
	// dim has to processed from raw input, e.g. invoked with  <1, 5, 15, 30> rather than [2][3][5].
	bool Checkform(const std::vector<int> &dim_pro);
	// Flatten the list tree
	// dim has to processed from raw input, e.g. invoked with  <1, 5, 15, 30> rather than [2][3][5].
	// Warning: Invoking Flatten even if Checkform(...) returns false is prone to memory leak
	LTTable Flatten(const std::vector<int> &dim_pro);
	// Visualize ListTree
	void Debug(int dep);

private:
	// dim has to processed from raw input, e.g. invoked with  <1, 5, 15, 30> rather than [2][3][5].
	// It always pops element before pushes it in. Thus the reference is always valid.
	bool _Checkform_(std::vector<int> &dim);
	// Flatten the list tree to one dimension
	// dim has to processed from raw input, e.g. invoked with  <1, 5, 15, 30> rather than [2][3][5].
	void _Flatten_(std::vector<int> &dim, LTTable &res, int ind, int upper);
	
	bool _Isnulllist_() {return !astptr && sublist.empty();}
	bool _Isleaf_() {return astptr;}

public:
	ASTptr astptr; // Pointer to AST. An expression is stored there, which is hopefully a scalar.
	std::vector<ASTptr> sublist; // Pointer to a sub-list.
};


void Verifyanumber(ASTptr p);

#endif