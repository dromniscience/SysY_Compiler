/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *              gentg.cpp: Generate eeyore file              *
 *************************************************************/

#define DEBUG_GENTG_CPP_DR_

#include <list>
#include <string>
#include <iostream>
#include <unordered_map>
#include "ast.hpp"
#include "error.hpp"
#include "geneey.hpp"
#include "eerep.hpp"
#include "parser.tab.hpp"

using std::list;
using std::string;
using std::cout;
using std::endl;

static list<EELine> funclines;
static list<string> temporaries;
static list<string> labels;
static list<ASTptr> endovars;
static list<string> glbvars;
static list<EELine> maininit;

int paramno = 0;
int endono = 0;
int temporaryno = 0;
int labelno = 0;

bool endpoint = false;

void DumpEE2file(){
	for(auto &i: eelines)
		i.EEDump();
}

static inline list<string>::iterator Newtemp(){
	temporaries.push_front(Settemporaryaddr());
	Increasetemporaryno();
	return temporaries.begin();
}

static inline list<string>::iterator Newlabel(){
	labels.push_front(Setlabeladdr());
	Increaselabelno();
	return labels.begin();
}

static std::string Tokentostring(int type){
	switch(type){
		case CONST: return "const";
		case INT: return "int";
		case VOID: return "void";
		case IF: return "if";
		case ELSE: return "else";
		case WHILE: return "while";
		case BREAK: return "break";
		case CONTINUE: return "continue";
		case RETURN: return "return";
		case LE: return "<=";
		case GE: return ">=";
		case EQ: return "==";
		case NE: return "!=";
		case AND: return "&&";
		case OR: return "||";
		case IDENT: return "identifier";
		case INT_CONST: return "constant";
		default:
			char tmp[2]; tmp[0] = type; tmp[1] = 0;
			return tmp;
	}
}

void BinaryAST::Generator(){
	// const value
	if(isconst){
		addr = Encodemessage(cval);
		if(endpoint){
			if(cval) funclines.emplace_back(EERecord::Uncond, std::move(truebranch));
			else funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
		}
		return;
	}

	addr = *Newtemp();
	// an intermediate node
#ifdef DEBUG_GENEEY_CPP_DR_
	if(!lexp || !rexp) Reportexception(Exception::Binarynullchild);
#endif

	bool tmpsave = endpoint;
	endpoint = false;
	switch (op)
	{
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
			lexp->Generator();
			rexp->Generator();
			funclines.emplace_back(EERecord::Binary, \
				std::string(addr), std::move(lexp->addr), std::move(rexp->addr), Tokentostring(op));
			endpoint = tmpsave;
			if(endpoint){
				funclines.emplace_back(EERecord::Cond, \
					std::string(addr), std::string("0"), std::string("!="), std::move(truebranch));
				funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
			}
			return;
		case LE:
		case GE:
		case EQ:
		case NE:
		case '<':
		case '>':
			lexp->Generator();
			rexp->Generator();
			/* make use of logic op, save tmp label in label */
			funclines.emplace_back(EERecord::Binary, \
				std::string(addr), std::move(lexp->addr), std::move(rexp->addr), Tokentostring(op));
			funclines.back().label = std::move(*Newlabel());
			funclines.back().num = std::move(*Newlabel());
			/*
			auto tt = Newlabel(), next = Newlabel();
			funclines.emplace_back(EERecord::Cond, std::move(lexp->addr), std::move(rexp->addr), Tokentostring(op), std::string(*tt));
			funclines.emplace_back(EERecord::Copy, std::string(addr), std::string("0"));
			funclines.emplace_back(EERecord::Uncond, std::string(*next));
			funclines.emplace_back(EERecord::Label, std::move(*tt));
			funclines.emplace_back(EERecord::Copy, std::string(addr), std::string("1"));
			funclines.emplace_back(EERecord::Label, std::move(*next));
			*/
			endpoint = tmpsave;
			if(endpoint){
				funclines.emplace_back(EERecord::Cond, \
					std::string(addr), std::string("0"), std::string("!="), std::move(truebranch));
				funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
			}
			return;
	}

	endpoint = tmpsave;
	if(op == OR && endpoint){
		lexp->truebranch = truebranch;
		lexp->falsebranch = *Newlabel();
		std::string tmpfalse = lexp->falsebranch;
		rexp->truebranch = truebranch;
		rexp->falsebranch = falsebranch;
		lexp->Generator();
		funclines.emplace_back(EERecord::Label, std::move(tmpfalse));
		rexp->Generator();
		return;
	}
	if(op == AND && endpoint){
		lexp->truebranch = *Newlabel();
		std::string tmptrue = lexp->truebranch;
		lexp->falsebranch = falsebranch;
		rexp->truebranch = truebranch;
		rexp->falsebranch = falsebranch;
		lexp->Generator();
		funclines.emplace_back(EERecord::Label, std::move(tmptrue));
		rexp->Generator();
		return;
	}
	Reportexception(Exception::Unexpectedop);
}

void UnaryAST::Generator() {
	// const value
	if(isconst){
		addr = Encodemessage(cval);
		if(endpoint){
			if(cval) funclines.emplace_back(EERecord::Uncond, std::move(truebranch));
			else funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
		}
		return;
	}

	// non-const value
#ifdef DEBUG_GENEEY_CPP_DR_
	if(!exp) Reportexception(Exception::Binarynullchild);
#endif
	bool tmpsave = endpoint;
	endpoint = false;
	exp->Generator();
	addr = exp->addr;
	
	for(int i = 0;i < 3;++i){
		int tmp = (op >> (i << 3)) & 0xFF;
		if(tmp == '-') {
			auto p = Newtemp();
			funclines.emplace_back(EERecord::Unary, std::string(*p), std::string(addr), std::string("-"));
			addr = *p;
		}
		if(tmp == '!') {
			auto p = Newtemp();
			funclines.emplace_back(EERecord::Unary, std::string(*p), std::string(addr), std::string("!"));
			addr = *p;
		}
	}

	endpoint = tmpsave;
	if(endpoint){
		funclines.emplace_back(EERecord::Cond, \
			std::string(addr), std::string("0"), std::string("!="), std::move(truebranch));
		funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
	}
}

void FunCallAST::Generator() {
	/* special treatment for starttime & stoptime */
	if(*(sym->strptr) == "starttime"){
		funclines.emplace_back(EERecord::Param, Encodemessage(bgnlno));
		funclines.emplace_back(EERecord::Voidcall, std::string("f__sysy_starttime"));
		return;
	}
	if(*(sym->strptr) == "stoptime"){
		funclines.emplace_back(EERecord::Param, Encodemessage(bgnlno));
		funclines.emplace_back(EERecord::Voidcall, std::string("f__sysy_stoptime"));
		return;
	}

	bool tmpsave = endpoint;
	endpoint = false;

	for(auto i: rparams)
		i->Generator();
	for(auto i: rparams)
		funclines.emplace_back(EERecord::Param, std::move(i->addr));
	// void function
	if(dynamic_cast<FunSYM *>(sym)->isvoid)
		funclines.emplace_back(EERecord::Voidcall, Setfuncaddr(*(sym->strptr)));
	// non-void function
	else{
		addr = *Newtemp();
		funclines.emplace_back(EERecord::Asscall, std::string(addr), Setfuncaddr(*(sym->strptr)));
	}

	endpoint = tmpsave;
	if(endpoint){
		funclines.emplace_back(EERecord::Cond, \
			std::string(addr), std::string("0"), std::string("!="), std::move(truebranch));
		funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
	}
}

void LvalAST::Generator() {
	// const value, cannot be on the left side
	if(isconst){
		addr = Encodemessage(cval);
		if(endpoint){
			if(cval) funclines.emplace_back(EERecord::Uncond, std::move(truebranch));
			else funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
		}
		return;
	}

	bool tmpsave = endpoint;
	endpoint = false;

	// non-const
	auto symp = dynamic_cast<VarSYM *>(sym);
	const int fsize = symp->dim_pro.size();
	const int rsize = indices.size();
	// refered to a plain varaible, regardless of the fact whether it is a left value
	if(fsize == 1 || rsize == 0){
		addr = symp->astptr->addr;
		endpoint = tmpsave;
		if(endpoint){
			funclines.emplace_back(EERecord::Cond, std::string(addr), std::string("0"), std::string("!="), std::move(truebranch));
			funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
		}
		return;
	}

	// refered to an array type or pointer
	list<string>::iterator p1, p2, p3;
	p1 = Newtemp();
	indices[0]->Generator();
	funclines.emplace_back(EERecord::Binary, \
		std::string(*p1), std::move(indices[0]->addr), Encodemessage(symp->dim_pro[fsize - 2]), std::string("*"));
	for(int i = 1;i < rsize;++i){
		p2 = Newtemp();
		p3 = Newtemp();
		indices[i]->Generator();
		funclines.emplace_back(EERecord::Binary, \
			std::string(*p2), std::move(indices[i]->addr), Encodemessage(symp->dim_pro[fsize - 2 - i]), std::string("*"));
		funclines.emplace_back(EERecord::Binary, std::string(*p3), std::move(*p1), std::move(*p2), std::string("+"));
		p1 = p3;
	}

	p2 = Newtemp();
	funclines.emplace_back(EERecord::Binary, \
		std::string(*p2), std::move(*p1), std::string("4"), std::string("*"));

	if(!isleft){
		addr = *Newtemp();
		// a scalar
		if(fsize == rsize + 1){
			funclines.emplace_back(EERecord::RArr, \
				std::string(addr), std::string(symp->astptr->addr), std::move(*p2));
		}
		// a pointer
		else
			funclines.emplace_back(EERecord::Binary, \
				std::string(addr), std::string(symp->astptr->addr), std::move(*p2), std::string("+"));
		
		endpoint = tmpsave;
		if(endpoint){
			funclines.emplace_back(EERecord::Cond, \
				std::string(addr), std::string("0"), std::string("!="), std::move(truebranch));
			funclines.emplace_back(EERecord::Uncond, std::move(falsebranch));
		}
		
		return;
	}
	else{
		endpoint = tmpsave;
		addr = sym->astptr->addr + "[" + *p2 + "]";
		return;
	}
}

// BlockAST

void BlockAST::Generator(){
	for(auto i:clauses)
		i->Generator();
}

// DeclAST

void DeclAST::Generator(){
	endovars.emplace_back(this);
	auto symp = dynamic_cast<VarSYM *>(sym);

	// a scalar
	if(symp->dim_pro.size() == 1){
		if(!init.empty()) {
			/* initialize */
			init[0]->Generator();
			funclines.emplace_back(EERecord::Copy, std::string(addr),  std::move(init[0]->addr));
		}
		return;
	}

	// an array
	for(auto i:init){
		/* initialize */
		i.second->Generator();
		funclines.emplace_back(EERecord::LArr, std::string(addr), Encodemessage(i.first * 4), std::move(i.second->addr));
	}
	return;
}

// FunDefAST

void FunDefAST::Convert2EE(){
	eelines.splice(eelines.end(), funclines);
}

void FunDefAST::Generator(){
	/* clear all the containers */
	funclines.clear();
	temporaries.clear();
	labels.clear();
	endovars.clear();

	/* append function prologue */
	funclines.emplace_back(EERecord::Header, std::string(addr), Encodemessage(fparams.size()));

	/* special treatment for main() */
	if(*(sym->strptr) == "main")
		funclines.splice(funclines.end(), maininit);
	
	/* generate function body */
	body->Generator();
	/* append endos and temps */
	auto q = ++funclines.begin();
	for(auto i: endovars){
		DeclAST *tmpp = dynamic_cast<DeclAST *>(i);
		VarSYM *symp = dynamic_cast<VarSYM *>(tmpp->sym);
		if(symp->dim_pro.size() == 1){ // a scalar
			funclines.emplace(q, EERecord::Decl, std::string(tmpp->addr));
		}
		else{ // an array
			funclines.emplace(q, EERecord::Decl, \
				std::string(tmpp->addr), Encodemessage(symp->dim_pro.back() * 4));
		}
	}
	for(auto &i: temporaries)
		funclines.emplace(q, EERecord::Decl, std::move(i));

	/* append function epilogue */
	/* guaranteed return */
	if(dynamic_cast<FunSYM *>(sym)->isvoid)
		funclines.emplace_back(EERecord::Voidret);
	else
		funclines.emplace_back(EERecord::Ret, std::string("0"));
	funclines.emplace_back(EERecord::End, std::string(addr));

	/* Dump to file */
	Convert2EE();
	
	/* clear all the containers */
	funclines.clear();
	temporaries.clear();
	labels.clear();
	endovars.clear();
}

void AssignAST::Generator() {
	dynamic_cast<LvalAST *>(lval)->isleft = true;
	lval->Generator();
	exp->Generator();
	if(lval->addr.back() == ']'){
		int t = lval->addr.find("["), len = lval->addr.size();
		funclines.emplace_back(EERecord::LArr, \
			lval->addr.substr(0, t), lval->addr.substr(t + 1, len - t - 2), std::move(exp->addr));
	}
	else
	funclines.emplace_back(EERecord::Copy, std::move(lval->addr), std::move(exp->addr));
}

void IfAST::Generator() {
	std::string tmptrue = *Newlabel();
	cond->truebranch = tmptrue;
	std::string tmpfalse = *Newlabel();
	cond->falsebranch = tmpfalse;
	next = *Newlabel();

	endpoint = true;
	cond->Generator();
	endpoint = false;
	
	funclines.emplace_back(EERecord::Label, std::move(tmptrue));
	if(then) then->Generator();
	funclines.emplace_back(EERecord::Uncond, std::string(next));
	
	funclines.emplace_back(EERecord::Label, std::move(tmpfalse));
	if(els) els->Generator();
	funclines.emplace_back(EERecord::Label, std::move(next));
}

void WhileAST::Generator() {
	begin = *Newlabel();
	cond->truebranch = *Newlabel();
	std::string tmptrue = cond->truebranch;
	cond->falsebranch = *Newlabel();
	next = cond->falsebranch;
	funclines.emplace_back(EERecord::Label, std::string(begin));
	
	endpoint = true;
	cond->Generator();
	endpoint = false;

	funclines.emplace_back(EERecord::Label, std::move(tmptrue));
	if(body) body->Generator();
	funclines.emplace_back(EERecord::Uncond, std::move(begin));
	funclines.emplace_back(EERecord::Label, std::move(next));
}

void BreakAST::Generator() {
	funclines.emplace_back(EERecord::Uncond, std::string(back->next));
}

void ContAST::Generator() {
	funclines.emplace_back(EERecord::Uncond, std::string(back->begin));
}

void RetAST::Generator() {
	if(exp){
		exp->Generator();
		funclines.emplace_back(EERecord::Ret, std::move(exp->addr));
	}
	else funclines.emplace_back(EERecord::Voidret);
}

void Treatmain(ASTptr root){
	for(auto i:dynamic_cast<BlockAST *>(root)->clauses){
		if(i->type != ASTType::Decl) continue;
		auto p = dynamic_cast<DeclAST *>(i);
		auto symp = dynamic_cast<VarSYM *>(p->sym);
		// a scalar
		if(symp->dim_pro.size() == 1){
			glbvars.emplace_back(p->addr);
			if(!p->init.empty()) {
				/* initialize */
				p->init[0]->Generator();
				maininit.splice(maininit.end(), funclines);
				maininit.emplace_back(EERecord::Copy, std::string(p->addr), std::move(p->init[0]->addr));
			}
			continue;
		}
		glbvars.emplace_back(Encodemessage(symp->dim_pro.back() * 4) + " " + p->addr);
		// an array
		for(auto i:p->init){
			/* initialize */
			i.second->Generator();
			maininit.splice(maininit.end(), funclines);
			maininit.emplace_back(EERecord::LArr, std::string(p->addr), Encodemessage(i.first * 4), std::move(i.second->addr));
		}
	}
	glbvars.splice(glbvars.end(), temporaries);
	for(auto &i: glbvars){
		if ('0' <= i[0] && i[0] <= '9'){
			int t = i.find(" ");
			eelines.emplace_back(EERecord::Decl, i.substr(t + 1), i.substr(0, t));
		}
		else
			eelines.emplace_back(EERecord::Decl, std::move(i));
	}
	glbvars.clear();
}

void TraverseAST(ASTptr root){
	for(auto i: dynamic_cast<BlockAST *>(root)->clauses){
		if(i->type == ASTType::Fundef) i->Generator();
	}
}