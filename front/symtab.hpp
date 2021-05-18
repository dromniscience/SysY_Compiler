/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *            symbol.hpp:  Class For Symbol Table            *
 *************************************************************/

#pragma once
#ifndef SYSY_SYMTAB_HPP_DR_
#define SYSY_SYMTAB_HPP_DR_

#include "baseast.hpp"
#include "strtab.hpp"
#include "token.hpp"

#include <string>
#include <vector>
#include <unordered_set>

class BaseSYM;
class EqualSYM;
class HashSYM;
class SYMTab;
using SYMptr = BaseSYM *;
using STptr = SYMTab *;

extern std::hash<std::string> hashstr;
extern STptr const glbst; // global symbol table
// extern STptr crrst; // current symbol table

enum class SYMType {Var, Fun};

/*** Classes for SYM ***/

class BaseSYM {
public:
	BaseSYM(STRptr p, SYMType type_, ASTptr astptr_, int bno_, int lno_, int bgn_, int end_): \
	strptr(p), type(type_), astptr(astptr_), bno(bno_), lno(lno_), bgn(bgn_), end(end_), \
	fngprnt(hashstr(*p)) {};
	virtual ~BaseSYM() {};

	STRptr strptr; // pointer into the string table
	SYMType type; // type of SYM
	ASTptr astptr; // the link back to AST
	int bno; // The block within which the symbol is defined
	int lno, bgn, end;
	const size_t fngprnt; // Fingerprint of the token name. Compute it once for all.
};

// SYM for variables
class VarSYM: public BaseSYM {
public:
	VarSYM(bool isconst_, TKptr p, ASTptr astptr_);
	~VarSYM() {};

	void Setarraytype(std::vector<int> &&);

	const bool isconst;
	std::vector<int> dim_pro; // e.g. [2][3][5] -> <1, 5, 15, 30>, [][2][3] -> <1, 3, 6, 0>
};

// SYM for functions
class FunSYM: public BaseSYM {
public:
	FunSYM(bool isvoid_, TKptr p, ASTptr astptr_);
	~FunSYM() {};

	void Insertfparams(SYMptr);

	const bool isvoid;
	std::vector<SYMptr> fparams; // formal parameters, whose scopes are exactly within the body
};

// Just functional Class
class EqualSYM{
public:
	bool operator () (SYMptr p, SYMptr q) const {
		return *(p->strptr) == *(q->strptr);
	}
};

// Just functional class
class HashSYM{
public:
	size_t operator () (SYMptr p) const {return p->fngprnt;}
};

class SYMTab{
public:
	SYMTab(int bno_, STptr fth = nullptr): father(fth), bno(bno_) {}
	~SYMTab(); // Recursively destory all the child nodes and the entries associated with it.

	std::unordered_set<SYMptr, HashSYM, EqualSYM> symtab;
	std::vector<STptr> child;
	STptr father;
	int bno;
};


/*** Symtab management functions ***/

// Creates a new SYM record, and check its uniqueness in the current scope.
SYMptr Newfunsymentry(bool isvoid, TKptr, ASTptr);
SYMptr Newvarsymentry(bool isconst, TKptr, ASTptr);
// Enter a new scope.
// Remember that while the function should be put in the upper layer
// (the global scope actually in sysY), its formal params should be put in the layer underneath.
// Warning: This function automatically increments blockno.
void Pushsymtab();
// Quit the scope. So the reference later on should not be bound to entries in the current SymTab.
// Warning: This function automatically decrements blockno.
void Popsymtab();
// Insert a new SYM record into the current symbol table.
// If there is collision, the old one remains.
void Insertsymentry(SYMptr);
// Invoked when resolving a reference by its name. (actually the index into the strtab)
SYMptr Lookupstring(STRptr p);
// Invoked when a new identifier is defined.
SYMptr Checkifdeclared(SYMptr p);
// Visualize the SymTab tree.
void Debugsymtab(STptr);

#endif