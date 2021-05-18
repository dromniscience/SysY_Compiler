/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *              baseast.hpp: Base Class for AST              *
 *************************************************************/

#pragma once
#ifndef SYSY_BASEAST_HPP_DR_
#define SYSY_BASEAST_HPP_DR_

#include "token.hpp"

class BaseAST;
using ASTptr = BaseAST *;

enum class ASTType {Base, Binary, Unary, Funcall, Lval, Block, Decl, Fundef, Assign, \
If, While, Break, Continue, Return, LT, Vec};

class BaseAST{
public:
	BaseAST(int bgnlno_, int bgnpos_, int endlno_, int endpos_, ASTType def = ASTType::Base): \
	bgnlno(bgnlno_), bgnpos(bgnpos_), endlno(endlno_), endpos(endpos_), type(def) {}

	void Setrange(int bgnlno_, int bgnpos_, int endlno_, int endpos_){
		bgnlno = bgnlno_;
		bgnpos = bgnpos_;
		endlno = endlno_;
		endpos = endpos_;
	}

	void Setrange(TKptr p, ASTptr q){
		bgnlno = p->lno;
		bgnpos = p->bgn;
		endlno = q->endlno;
		endpos = q->endpos;
	}

	void Setrange(TKptr p, TKptr q){
		bgnlno = p->lno;
		bgnpos = p->bgn;
		endlno = q->lno;
		endpos = q->end;
	}

	void Setrange(ASTptr p, TKptr q){
		bgnlno = p->bgnlno;
		bgnpos = p->bgnpos;
		endlno = q->lno;
		endpos = q->end;
	}

	void Setrange(ASTptr p, ASTptr q){
		bgnlno = p->bgnlno;
		bgnpos = p->bgnpos;
		endlno = q->endlno;
		endpos = q->endpos;
	}

	void Setaddr(std::string &&name){
		addr = name;
	}

	virtual ~BaseAST() {};
	virtual void Eval();

	virtual bool Isconst();
	virtual int Cval();
	virtual bool Isanumber();
	
	void Debug(int depth);

	virtual void Generator();
	virtual void Dump2file();

	ASTType type;
	int bgnlno, endlno;
	int bgnpos, endpos;
	std::string addr; // addr in eeyore
	
	/* eeyore: flow of control */
	std::string truebranch;
	std::string falsebranch;

	/* eeyore: flow of control */
	std::string next;
	std::string begin;
};

#endif