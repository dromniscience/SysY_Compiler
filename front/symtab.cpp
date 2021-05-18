/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *            symbol.cpp:  Class For Symbol Table            *
 *************************************************************/

#define DEBUG_SYMTAB_CPP_DR_

#include "symtab.hpp"
#include "error.hpp"
#include "global.hpp"

#ifdef DEBUG_SYMTAB_CPP_DR_
#include <iostream>
#endif

STptr const glbst = new SYMTab(blockno);
static STptr crrst = glbst;

std::hash<std::string> hashstr;

// SYMTab
SYMTab::~SYMTab(){
	for(auto p: symtab) delete p;
	for(auto p: child) delete p;
}

// VarSYM
VarSYM::VarSYM(bool isconst_, TKptr p, ASTptr astptr_): \
BaseSYM(p->strptr, SYMType::Var, astptr_, blockno, p->lno, p->bgn, p->end), isconst(isconst_) {};

void VarSYM::Setarraytype(std::vector<int> &&a) {dim_pro = a;}

// FunSYM
FunSYM::FunSYM(bool isvoid_, TKptr p, ASTptr astptr_): \
BaseSYM(p->strptr, SYMType::Fun, astptr_, blockno, p->lno, p->bgn, p->end), isvoid(isvoid_) {};

void FunSYM::Insertfparams(SYMptr p){
	fparams.push_back(p);
}

// Symtab managements

void Pushsymtab(){
	blockno += 1;
	STptr p = new SYMTab(blockno, crrst);
	crrst->child.push_back(p);
	crrst = p;
}

void Popsymtab(){
	blockno -= 1;
	crrst = crrst->father;
#ifdef DEBUG_SYMTAB_CPP_DR_
	if(!crrst)
		Reportexception(Exception::Scopingexp);
#endif
}

void Insertsymentry(SYMptr p){
	if(p) crrst->symtab.insert(p);
}

SYMptr Lookupstring(STRptr p){
	SYMptr tmp = new BaseSYM(p, SYMType::Var, nullptr, 0, 0, 0, 0);
	for(auto q = crrst; q; q = q->father){
		auto k = q->symtab.find(tmp);
		if(k != q->symtab.end()) {
			delete tmp;
			return *k;
		}
	}
	delete tmp;
	return nullptr;
}

SYMptr Checkifdeclared(SYMptr p){
	auto k = crrst->symtab.find(p);
	if(k != crrst->symtab.end()) return *k;
	return nullptr;
}

SYMptr Newfunsymentry(bool isvoid, TKptr p, ASTptr q){
	SYMptr newsym = new FunSYM(isvoid, p, q);
	
	SYMptr prev = Checkifdeclared(newsym);
	if(prev) {
		ASTptr t = new BaseAST(prev->lno, prev->bgn, prev->lno, prev->end);
		errptr = new BaseAST(p->lno, p->bgn, p->lno, p->end);
		Reporterror(Error::Identdeclared, *(p->strptr), t);
		delete errptr; errptr = nullptr;
		delete t;

		delete newsym;
		return nullptr;
	}
	
	return newsym;
}

SYMptr Newvarsymentry(bool isconst, TKptr p, ASTptr q){
	SYMptr newsym = new VarSYM(isconst, p, q);
	
	SYMptr prev = Checkifdeclared(newsym);
	if(prev) {
		ASTptr t = new BaseAST(prev->lno, prev->bgn, prev->lno, prev->end);
		errptr = new BaseAST(p->lno, p->bgn, p->lno, p->end);
		Reporterror(Error::Identdeclared, *(p->strptr), t);
		delete errptr; errptr = nullptr;
		delete t;

		delete newsym;
		return nullptr;
	}
	
	return newsym;
}

void Debugsymtab(STptr p){
#ifdef DEBUG_SYMTAB_CPP_DR_
	std::string tmp(p->bno * 2, ' ');
	for(auto i: p->symtab){
		std::cout << tmp;
		if(i->type == SYMType::Var){
			VarSYM *p = dynamic_cast<VarSYM *>(i);
			if(p->isconst) std::cout << "const ";
			std::cout << Encodemessage(p->dim_pro) << ": " << *(i->strptr);
		}
		else{
			FunSYM *p = dynamic_cast<FunSYM *>(i);
			if(p->isvoid) std::cout << "void ";
			else std::cout << "int ";
			std::cout << *(i->strptr) << "(";
			int m = 0;
			for(auto j: p->fparams){
				if(m) std::cout << ", ";
				std::cout << Encodemessage(dynamic_cast<VarSYM *>(j)->dim_pro);
				m += 1;
			}
			std::cout << ")";
		}
		std::cout << "  in block " << i->bno << " at line " << i->lno << std::endl;
	}
	for(auto i: p->child)
		Debugsymtab(i);
#endif
}