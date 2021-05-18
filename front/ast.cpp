/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *                 ast.cpp:  Classes For AST                 *
 *************************************************************/

#define DEBUG_AST_CPP_DR_
#define DEBUG_LTREE_CPP_DR_

#include "ast.hpp"
#include "error.hpp"
#include "parser.tab.hpp"

#ifdef DEBUG_AST_CPP_DR_
#include <iostream>
#endif

ASTptr mainptr = nullptr;

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

static int Dobinaryop(ASTptr lop, ASTptr rop, int op) {
	int a = lop->Cval(), b = rop->Cval();
	switch(op){
		case '+': return a + b;
		case '-': return a - b;
		case '*': return a * b;
		case '/': {
			if(!b){
				errptr = rop;
				Reporterror(Error::Devidebyzero);
				errptr = nullptr;
			}
			return a / b;
		}
		case '%':{
			if(!b){
				errptr = rop;
				Reporterror(Error::Devidebyzero);
				errptr = nullptr;
			}
			return a % b;
		}
		case '<':
			return a < b;
		case '>':
			return a > b;
		case LE:
			return a <= b;
		case GE:
			return a >= b;
		case EQ:
			return a == b;
		case NE:
			return a != b;
		case AND:
			return a && b; // Note that evaluating const exp will not defy short circuit rule.
		case OR:
			return a || b; // Note that evaluating const exp will not defy short circuit rule.
#ifdef DEBUG_AST_CPP_DR_
		default:
			Reportexception(Exception::Unexpectedop, Tokentostring(op));
			return 0;
#endif
	}
}

// BaseAST
void BaseAST::Debug(int depth){
	std::string prefix(depth * 2, ' ');
	switch(type){
		case ASTType::Block: {
			BlockAST *p = dynamic_cast<BlockAST *>(this);
			std::cout << prefix << "block " << p->bno << std::endl;
			int m = 0;
			for(auto i: p->clauses) {
				std::cout << prefix << "stt " << m << std::endl;
				i->Debug(depth + 1);
				m += 1;
			}
			std::cout << prefix << "end block" << std::endl;
			return;
		}
		case ASTType::Break: {
			BreakAST *p = dynamic_cast<BreakAST *>(this);
			std::cout << prefix << "break: " << p->back->bgnlno << "," << p->back->bgnpos << std::endl;
			return;
		}
		case ASTType::Binary: {
			BinaryAST *p = dynamic_cast<BinaryAST *>(this);
			if(p->Isconst()) std::cout << prefix << p->Cval() << std::endl;
			else {
				std::cout << prefix << Tokentostring(p->op) << std::endl;
				std::cout << prefix << "oprand 1:" << std::endl;
				/*if(p->lexp)*/ p->lexp->Debug(depth + 1);
				std::cout << prefix << "oprand 2:" << std::endl;
				/*if(p->rexp)*/ p->rexp->Debug(depth + 1);
				std::cout << prefix << "end of " << Tokentostring(p->op) << std::endl;
			}
			return;
		}
		case ASTType::Unary: {
			UnaryAST *p = dynamic_cast<UnaryAST *>(this);
			if(p->Isconst()) std::cout << prefix << p->Cval() << std::endl;
			else {
				std::cout << prefix << (char)(p->op >> 16) << (char)(p->op >> 8) << (char)(p->op) << std::endl;
				/*if(p->exp)*/ p->exp->Debug(depth + 1);
				std::cout << prefix << "end of " << (char)(p->op >> 16) << (char)(p->op >> 8) << (char)(p->op) << std::endl;
			}
			return;
		}
		case ASTType::Funcall: {
			FunCallAST *p = dynamic_cast<FunCallAST *>(this);
			FunSYM *q = dynamic_cast<FunSYM *>(p->sym);
			std::cout << prefix << "Funcall: " << *(q->strptr) << std::endl;
			int m = 0;
			for(auto i: p->rparams) {
				std::cout << prefix << "param " << m << std::endl;
				i->Debug(depth + 1);
				m += 1;
			}
			std::cout << prefix << "end params" << std::endl;
			return;
		}
		case ASTType::Lval: {
			LvalAST *p = dynamic_cast<LvalAST *>(this);
			VarSYM *q = dynamic_cast<VarSYM *>(p->sym);
			if(p->isconst) {
				std::cout << prefix << p->Cval() << std::endl;
				return;
			}
			std::cout << prefix << "Varref: " << *(q->strptr) << std::endl;
			int m = 0;
			for(auto i: p->indices) {
				std::cout << prefix << "index " << m << std::endl;
				i->Debug(depth + 1);
				m += 1;
			}
			std::cout << prefix << "end indices" << std::endl;
			return;
		}
		case ASTType::Decl: {
			DeclAST *p = dynamic_cast<DeclAST *>(this);
			VarSYM *q = dynamic_cast<VarSYM *>(p->sym);
			std::cout << prefix << addr << " ";
			if(q->isconst) std::cout << "const ";
			std::cout << Encodemessage(q->dim_pro) << ": " << *(q->strptr) << std::endl;
			for(auto i: p->init) {
				std::cout << prefix << "init " << i.first << std::endl;
				i.second->Debug(depth + 1);
			}
			return;
		}
		case ASTType::Fundef: {
			FunDefAST *p = dynamic_cast<FunDefAST *>(this);
			FunSYM *q = dynamic_cast<FunSYM *>(p->sym);
			std::cout << prefix;
			if(q->isvoid) std::cout << "void ";
			else std::cout << "int ";
			std::cout << p->addr << "(";
			int m = 0;
			for(auto j: p->fparams){
				if(m) std::cout << ", ";
				VarSYM *r = dynamic_cast<VarSYM *>(dynamic_cast<DeclAST *>(j)->sym);
				std::cout << Encodemessage(r->dim_pro) << " " << r->astptr->addr << " " << *(r->strptr);
				m += 1;
			}
			std::cout << ")" << std::endl;
			p->body->Debug(depth + 1);
			std::cout << prefix << "end func" << std::endl;
			return;
		}
		case ASTType::Assign: {
			AssignAST *p = dynamic_cast<AssignAST *>(this);
			std::cout << prefix << "assign:" << std::endl;
			p->lval->Debug(depth + 1);
			std::cout << prefix << "value" << std::endl;
			p->exp->Debug(depth + 1);
			std::cout << prefix << "end assign" << std::endl;
			return;
		}
		case ASTType::If: {
			IfAST *p = dynamic_cast<IfAST *>(this);
			std::cout << prefix << "if" << std::endl;
			p->cond->Debug(depth + 1);
			std::cout << prefix << "then" << std::endl;
			p->then->Debug(depth + 1);
			if(p->els) {
				std::cout << prefix << "else" << std::endl;
				p->els->Debug(depth + 1);
				std::cout << prefix << "end if" << std::endl;
			}
			return;
		}
		case ASTType::While: {
			WhileAST *p = dynamic_cast<WhileAST *>(this);
			std::cout << prefix << "while" << std::endl;
			p->cond->Debug(depth + 1);
			std::cout << prefix << "do" << std::endl;
			p->body->Debug(depth + 1);
			std::cout << prefix << "end while" << std::endl;
			return;
		}
		case ASTType::Continue: {
			ContAST *p = dynamic_cast<ContAST *>(this);
			std::cout << prefix << "continue: " << p->back->bgnlno << "," << p->back->bgnpos << std::endl;
			return;
		}
		case ASTType::Return: {
			RetAST *p = dynamic_cast<RetAST *>(this);
			std::cout << prefix << "ret: " << *(dynamic_cast<FunDefAST *>(p->back)->sym->strptr) << std::endl;
			return;
		}
		default:
			std::cout << prefix << RED("error") << std::endl;
	}
}

// BinaryAST
// used to represent + - * / < > <= >= == != && ||

// INT_CONST
BinaryAST::BinaryAST(TKptr p): BaseAST(p->lno, p->bgn, p->lno, p->end, ASTType::Binary), \
	isconst(true), cval(p->val), lexp(nullptr), rexp(nullptr), op(0), isevaled(false)  {
#ifdef DEBUG_AST_CPP_DR_
	if(p->type != INT_CONST) Reportexception(Exception::Unexpectedop, Tokentostring(p->type));
#endif
	};

// Binary expression
BinaryAST::BinaryAST(ASTptr lch, ASTptr rch, TKptr op): BaseAST(lch->bgnlno, lch->bgnpos, rch->endlno, rch->endpos, ASTType::Binary), \
	isconst(false), cval(0), lexp(lch), rexp(rch), op(op->type), isevaled(false) { 
		if(!lch || !rch) {
			errptr = this;
			Reportexception(Exception::Binarynullchild);
			errptr = nullptr;
		}
	};

BinaryAST::~BinaryAST() {if(lexp) {delete lexp; lexp = nullptr;} if(rexp) {delete rexp; rexp = nullptr;}}

void BinaryAST::Eval(){
	if(isevaled || isconst) return;

#ifdef DEBUG_AST_CPP_DR_
	if(!lexp || !rexp) {
		errptr = this;
		Reportexception(Exception::Binarynullchild);
		errptr = nullptr;
	}
#endif
	lexp->Eval();
	rexp->Eval();

	if(lexp->Isconst() && rexp->Isconst()) {
		isconst = true;
		cval = Dobinaryop(lexp, rexp, op);
		delete lexp; lexp = nullptr;
		delete rexp; rexp = nullptr;
	}

	isevaled = true;
	return;
}

bool BinaryAST::Isanumber(){
	if(isconst) return true;
	if(lexp && !lexp->Isanumber()) return false;
	if(rexp && !rexp->Isanumber()) return false;
	return true;
}

// UnaryAST
// used to represent + - !

UnaryAST::UnaryAST(ASTptr ch, TKptr op): BaseAST(op->lno, op->bgn, ch->endlno, ch->endpos, ASTType::Unary), \
	isconst(false), cval(0), exp(ch), op(op->type), isevaled(false) {}

UnaryAST::~UnaryAST() {if(exp) {delete exp; exp = nullptr;}}

void UnaryAST::Coalesce(TKptr p) {
	switch(op){
		case '+': op = p->type;
				  return;
		case '-': if(p->type == '+') return;
				  if(p->type == '-') {op = '+'; return;}
				  op = '!';
				  return;
		case '!': op += (p->type << 8);
				  return;
		case ('+' << 8) + '!': op = (p->type << 8) + '!';
							   return;
		case ('-' << 8) + '!': if(p->type == '+') return;
							   if(p->type == '-') {op = ('+' << 8) + '!'; return;}
							   op = ('!' << 8) + '!';
							   return;
		case ('!' << 8) + '!': if(p->type == '!') {op = '!'; return;}
							   op += (p->type << 16);
							   return;
		case ('+' << 16) + ('!' << 8) + '!': if(p->type == '!') {op = '!'; return;}
											 op = (p->type << 16) + ('!' << 8) + '!';
											 return;
		case ('-' << 16) + ('!' << 8) + '!': if(p->type == '!') {op = '!'; return;}
											 if(p->type == '+') return;
											 p->type = ('+' << 16) + ('!' << 8) + '!';
	}
}

void UnaryAST::Eval(){
	if(isevaled || isconst) return;

#ifdef DEBUG_AST_CPP_DR_
	if(!exp) {
		errptr = this;
		Reportexception(Exception::Binarynullchild);
		errptr = nullptr;
	}
#endif
	exp->Eval();

	if(exp->Isconst()){
		isconst = true;
		cval = exp->Cval();
		
		delete exp;
		exp = nullptr;
		
		for(int i = 0;i < 3;++i){
			int tmp = (op >> (i << 3)) & 0xFF;
			if(tmp == '-') cval = -cval;
			if(tmp == '!') cval = !cval;
		}
		op = 0;
	}

	isevaled = true;
	return;
}

bool UnaryAST::Isanumber(){
	if(isconst) return true;
	if(exp && !exp->Isanumber()) return false;
	return true;
}

// FunCallAST

FunCallAST::FunCallAST(TKptr p):BaseAST(p->lno, p->bgn, p->lno, p->end, ASTType::Funcall){
	SYMptr ptr = Lookupstring(p->strptr);
	/* Declared? */
	if(!ptr){
		errptr = this;
		Reporterror(Error::Identnotdeclared, *(p->strptr));
		errptr = nullptr;
	}
	/* Erroreously call a varaible? */
	if(ptr->type == SYMType::Var){
		errptr = this;
		ASTptr n = new BaseAST(ptr->lno, ptr->bgn, ptr->lno, ptr->end);
		Reporterror(Error::Varnotcallable, *(p->strptr), n);
		delete n;
		errptr = nullptr;
	}
	sym = ptr;
}

FunCallAST::~FunCallAST(){
	for(auto i: rparams) delete i;
}

bool FunCallAST::Checkparamlistsize(){
	const int flen = dynamic_cast<FunSYM *>(sym)->fparams.size();
	const int rlen = rparams.size();

	if(rlen != flen) errptr = this;
	return rlen == flen;
}

bool FunCallAST::Isanumber(){
	if(dynamic_cast<FunSYM *>(sym)->isvoid){
		if(!errptr) errptr = this;
		return false;
	}
	return true;
}

void FunCallAST::Verifyfuncall(){
	const int flen = dynamic_cast<FunSYM *>(sym)->fparams.size();
	const int rlen = rparams.size();
	if(rlen != flen){
		errptr = this;
		Reporterror(Error::Rparamsnomismatch, *(sym->strptr), sym->astptr);
		errptr = nullptr;
		return;
	}
	// Empty parameter list!
	if(!rlen) return;

	const int size = rparams.size();
	for(int i = 0;i < size; ++i){
		SYMptr fp = dynamic_cast<FunSYM *>(sym)->fparams[i];
		ASTptr rp = rparams[i];
		// rparam should be a scalar
		if(dynamic_cast<VarSYM *>(fp)->dim_pro.size() == 1) {Verifyanumber(rp); continue;}
		// Pointer
		if(rp->type != ASTType::Lval) {
			errptr = rp;
			Reporterror(Error::Notapointer, std::string(), fp->astptr);
			errptr = nullptr;
			continue;
		}
		else{
			// rparam should be a pointer
			LvalAST *t = dynamic_cast<LvalAST *>(rp);
			VarSYM *pt = dynamic_cast<VarSYM *>(t->sym);
			std::vector<int> &rvec = pt->dim_pro;
			std::vector<ASTptr> &r = t->indices;
			std::vector<int> &fvec = dynamic_cast<VarSYM *>(fp)->dim_pro;
			// Pointer cannot be pointed to a const array
			if(pt->isconst){
				errptr = rp;
				Reporterror(Error::Constarrtoptr, *(fp->strptr), pt->astptr);
				errptr = nullptr;
				continue;
			}
			// the dimension of array should match
			if(r.size() + fvec.size() != rvec.size()){
				errptr = rp;
				Reporterror(Error::Pointerdimmismatch, std::string(), fp->astptr);
				errptr = nullptr;
				continue;
			}
			// each index should be a scalar
			for(auto j: r) Verifyanumber(j);
			int size = fvec.size();
			// dimension-wise matching
			for(int k = 0;k < size - 1;++k)
				if(fvec[k] != rvec[k]){
					errptr = rp;
					Reporterror(Error::Pointerdimmismatch, std::string(), fp->astptr);
					errptr = nullptr;
					break;
				}
		}
	}
}

// LvalAST

LvalAST::LvalAST(TKptr p, ASTptr q): BaseAST(p->lno, p->bgn, q->endlno, q->endpos, ASTType::Lval), \
isconst(false), cval(0), isleft(false), isevaled(false) {
	SYMptr ptr = Lookupstring(p->strptr);
	/* Declared? */
	if(!ptr){
		errptr = this;
		Reporterror(Error::Identnotdeclared, *(p->strptr));
		errptr = nullptr;
	}
	/* Erroreously call a function? */
	if(ptr->type == SYMType::Fun){
		errptr = this;
		ASTptr n = new BaseAST(ptr->lno, ptr->bgn, ptr->lno, ptr->end);
		Reporterror(Error::Unexpectedfuncall, *(p->strptr), n);
		delete n;
		errptr = nullptr;
	}

	sym = ptr;
	indices = dynamic_cast<Vecptr>(q)->Getindices(); // Move constructor

	/* Too many indices given? */
	if(Checkiftoomanyindices()){
		errptr = q;
		ASTptr n = new BaseAST(ptr->lno, ptr->bgn, ptr->lno, ptr->end);
		Reporterror(Error::Toomanyindices, std::string(), n);
		delete n;
		errptr = nullptr;
	}

	/* Valid Range? */
	if(Checknegindex()){
		// Side Effect
		Reporterror(Error::Arraynegindex, Encodemessage(errptr->Cval()));
		errptr = nullptr;
	}
	if(Checkbigindex()){
		// Side Effect
		Reporterror(Error::Arraybigindex, Encodemessage(errptr->Cval()));
		errptr = nullptr;
	}
}

LvalAST::~LvalAST() {for(auto i: indices) delete i;}

bool LvalAST::Checkiftoomanyindices(){
	const int a = indices.size();
	const int b = dynamic_cast<VarSYM *>(sym)->dim_pro.size();
	return a + 1 > b;
}

bool LvalAST::Checknegindex(){
	VarSYM *r = dynamic_cast<VarSYM *>(sym);
	const int size = indices.size();
	for(int i = 0; i < size;++i){
		if(!indices[i]->Isconst()) continue;
		if(indices[i]->Cval() < 0){
			errptr = indices[i];
			return true;
		}
	}
	return false;
}

bool LvalAST::Checkbigindex(){
	VarSYM *r = dynamic_cast<VarSYM *>(sym);
	const int size = indices.size(), arrsize = r->dim_pro.size();
	for(int i = 0; i < size;++i){
		if(!indices[i]->Isconst()) continue;
		int tmp = r->dim_pro[arrsize - 1 - i] / r->dim_pro[arrsize - 2 - i];
		if(indices[i]->Cval() >= tmp && tmp){
			errptr = indices[i];
			return true;
		}
	}
	return false;
}

void LvalAST::Eval(){
	if(isevaled || isconst) return;
	isevaled = true;
	
	if(!dynamic_cast<VarSYM *>(sym)->isconst) return;
	for(auto i: indices){
		if(!i->Isconst()) return;
	}

	if(!Isanumber()) {errptr = nullptr; return;}
	
	isconst = true;
	std::vector<int> index;
	for(auto i: indices){
		index.push_back(i->Cval());
		// Is the index valid?
	}

	for(auto i: indices) delete i;
	indices.clear();

	int no = Arraylinearno(dynamic_cast<VarSYM *>(sym)->dim_pro, index);
	ASTptr tmp = dynamic_cast<DeclAST *>(sym->astptr)->Getinitvalue(no);
	if(tmp) cval = tmp->Cval();
	else cval = 0;
}

bool LvalAST::Isanumber(){
	if(isconst) return true;
	const int len = indices.size(), flen = dynamic_cast<VarSYM *>(sym)->dim_pro.size();
	if(len == flen - 1) {
		for(auto i: indices) if(!i->Isanumber()) return false;
		return true;
	}
	if(!errptr) errptr = this;
	return false;
}

// DeclAST
// Record a declaration, either within a statement or a formal parameter table

DeclAST::~DeclAST() {for(auto i:init) delete i.second;}

ASTptr DeclAST::Getinitvalue(int index) {
	auto k = init.find(index);
	if(k == init.end()) return nullptr;
	else return k->second;
}

// FunDefAST
// Record a function definition

FunDefAST::~FunDefAST() {
	for(auto i:fparams) delete i;
	delete body;
}

// VecAST
// Store a vector of ASTptr

void VecAST::Insertastlist(ASTptr p) {
	astlist.push_back(p);
}

std::vector<int> VecAST::Getrawarraytype(){
	std::vector<int> tmp;
	for(auto p: astlist){
#ifdef DEBUG_LTREE_CPP_DR_
		if(!p->Isconst()) {
			errptr = this;
			Reportexception(Exception::Unexpectednonconstexp);
			errptr = nullptr;
		}
#endif
		if(p->Cval() <= 0) {
			errptr = p;
			Reporterror(Error::Arraynegindex, Encodemessage(p->Cval()));
			errptr = nullptr;
		}
		tmp.push_back(p->Cval());
	}

	for(auto p: astlist) delete p;
	astlist.clear();
	return tmp;
}

std::vector<ASTptr> VecAST::Getfparams(){
	return std::move(astlist);
}

std::vector<ASTptr> VecAST::Getrparams(){
	return std::move(astlist);
}

std::vector<ASTptr> VecAST::Getindices(){
	return std::move(astlist);
}

// ListTree
// Dedicated data structure to represent list initialization

void ListTree::Insertsublist(ASTptr p){
	sublist.push_back(p);
}

bool ListTree::Checkform(const std::vector<int> &dim_pro){
#ifdef DEBUG_LTREE_CPP_DR_
	if(!dim_pro.size() || dim_pro[0] != 1) Reportexception(Exception::Arraytypeexp);
#endif
	std::vector<int> savedim = dim_pro; // Copy Constructor
	return _Checkform_(savedim);
}

bool ListTree::_Checkform_(std::vector<int> &dim_pro){
	goto _Check;
_Fail:
	if(!errptr) errptr = this;
	return false;
_Check:
	// End points
	if(dim_pro.size() == 1) {
		if(_Isleaf_() || _Isnulllist_()) return true;
		else goto _Fail;
	}

	// {} could generate everything
	if(_Isnulllist_()) return true;
	// On a leaf node, error
	if(_Isleaf_()) goto _Fail;

	const int tot = dim_pro.back();
	int tmp = 0, acc = 0;

	dim_pro.pop_back();
	const int no = dim_pro.back();

	for(auto p: sublist){
		if(!dynamic_cast<LTptr>(p)->_Isleaf_()) {
			// Within a pair of brace, 
			// the number of exps must be a multiple of the rest dimension
			if(acc % no) goto _Fail;
			if(!dynamic_cast<LTptr>(p)->_Checkform_(dim_pro)) goto _Fail;
			tmp += 1;
		}
		else acc += 1;
	}

	// Allow padding 0 by default
	tmp += ((acc + no - 1) / no); // ceiling
	if(tmp > tot / no) goto _Fail;
	dim_pro.push_back(tot);
	return true;
}

// Flatten the list tree
LTTable ListTree::Flatten(const std::vector<int> &dim_pro){
#ifdef DEBUG_LTREE_CPP_DR_
	if(!dim_pro.size() || dim_pro[0] != 1) Reportexception(Exception::Arraytypeexp);
#endif
	LTTable ret; // The reference to map will always remain valid however insertions and deletions are carried out
	std::vector<int> savedim = dim_pro; // Copy constructor
	_Flatten_(savedim, ret, 0, savedim.back());
	return ret;
}

// Flatten the list tree to one dimension
void ListTree::_Flatten_(std::vector<int> &dim, LTTable &res, int ind, int upper){
	if(_Isnulllist_()) return;
	if(dim.size() == 1) {
		if(ind >= upper) return; // Too many entries!
		if(astptr) res[ind] = astptr;
		return;
	}

	const int tot = dim.back();
	dim.pop_back();
	const int no = dim.back();

	for(auto p: sublist){
		if(!dynamic_cast<LTptr>(p)->_Isleaf_()){
			dynamic_cast<LTptr>(p)->_Flatten_(dim, res, ind, upper);
			ind += no;
			if(ind >= upper) return; // Too many entries!
		}
		else {
			if(ind >= upper) return; // Too many entries!
			res[ind] = dynamic_cast<LTptr>(p)->astptr;
			ind += 1;
		}
	}
	dim.push_back(tot);
	return;
}

void ListTree::Debug(int dep){
#ifdef DEBUG_LTREE_CPP_DR_
	if(_Isleaf_()) std::cout << std::string(dep, ' ') << astptr->Cval() << std::endl;
	else if(_Isnulllist_()) std::cout << std::string(dep, ' ') << "{}" << std::endl;
	else for(auto p: sublist) dynamic_cast<LTptr>(p)->Debug(dep + 1);
#endif
}

void Verifyanumber(ASTptr p){
	if(!p->Isanumber()){
		// Side Effect: Isanumber() will modify errptr
		if(errptr->type == ASTType::Funcall){
			FunCallAST *r = dynamic_cast<FunCallAST *>(errptr);
			Reporterror(Error::Notanumber, *(r->sym->strptr), r->sym->astptr);
		}
		else{
			LvalAST *r = dynamic_cast<LvalAST *>(errptr);
			Reporterror(Error::Notanumber, *(r->sym->strptr), r->sym->astptr);
		}
		errptr = nullptr;
	}
}