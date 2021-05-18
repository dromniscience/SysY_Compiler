/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *                error.cpp: Class For Errors                *
 *************************************************************/

#define REPORT_ERROR_DR_
#define REPORT_WARNING_DR_
#define REPORT_EXCEPTION_DR_

#include "global.hpp"
#include "error.hpp"
#include <iostream>

using std::vector;
using std::string;
using std::cerr;
using std::endl;
using Handler = void (*)(const string &, ASTptr);

ASTptr errptr = nullptr;
static int errnum = 0;
static int warnnum = 0;

static void Errorline();

// All these functions should not modify errptr.
// Each function should at least guarantee that on return the errptr is the same as it is invoked.

static void Unknowntoken(const string &, ASTptr);
static void Emptyfile(const string &, ASTptr);
static void Noinputfile(const string &, ASTptr);
static void Filenotfound(const string &, ASTptr);
static void Invalidsyntax(const string &, ASTptr); // must leave
static void Devidebyzero(const string &, ASTptr); // must leave
static void Notconstexp(const string &, ASTptr); // must leave
static void Arrayinitmismatch(const string &, ASTptr); // .
static void Arraynegindex(const string &, ASTptr);
static void Identdeclared(const string &, ASTptr); // must leave
static void Identnotdeclared(const string &, ASTptr); // must leave
static void Toomanyindices(const string &, ASTptr); // must leave
static void Unexpectedfuncall(const string &, ASTptr); // must leave
static void Varnotcallable(const string &, ASTptr); // must leave
static void Rparamsnomismatch(const string &, ASTptr); // must leave
static void Nowhiletoescape(const string &, ASTptr);
static void Returnavalue(const string &, ASTptr);
static void Assignarray(const string &, ASTptr);
static void Arraybigindex(const string &, ASTptr); // must leave
static void Notanumber(const string &, ASTptr); // must leave
static void Assignconstant(const string &, ASTptr); // must leave
static void Notapointer(const string &, ASTptr);
static void Pointerdimmismatch(const string &, ASTptr);
static void Voidmain(const string &, ASTptr);
static void Parammain(const string &, ASTptr);
static void Nomain(const string &, ASTptr);
static void Constarrtoptr(const string &, ASTptr);
static void Nooutputfile(const string &, ASTptr); // must leave
static void Noreturnvalue(const string &, ASTptr);

static void Intoverflow(const string &, ASTptr);
static void Identtoolong(const string &, ASTptr);

static void Binarynullchild(const string &, ASTptr);
static void Unexpectedop(const string &, ASTptr);
static void Unexpectedbasecall(const string &, ASTptr);
static void Arraytypeexp(const string &, ASTptr);
static void Scopingexp(const string &, ASTptr); // must leave
static void Unexpectednonconstexp(const string &, ASTptr); // must leave
static void Unexpectedgencall(const string &, ASTptr); // must leave

static Handler ErrorHandler[(int)Error::_Final] {Unknowntoken, Emptyfile, Noinputfile, Filenotfound, Invalidsyntax, Devidebyzero, \
Notconstexp, Arrayinitmismatch, Arraynegindex, Identdeclared, Identnotdeclared, Toomanyindices, \
Unexpectedfuncall, Varnotcallable, Rparamsnomismatch, Nowhiletoescape, Returnavalue, Assignarray, Arraybigindex, \
Notanumber, Assignconstant, Notapointer, Pointerdimmismatch, Voidmain, Parammain, Nomain, Constarrtoptr, Nooutputfile, \
Noreturnvalue};
static Handler WarningHandler[(int)Warning::_Final] {Intoverflow, Identtoolong};
static Handler ExceptionHandler[(int)Exception::_Final] {Binarynullchild, Unexpectedop, Unexpectedbasecall, Arraytypeexp, Scopingexp, \
Unexpectednonconstexp, Unexpectedgencall};


/*** Handlers Implementation ***/

// Print Error Lines
void Errorline(){
	cerr << filecontent[errptr->bgnlno - 1] << endl;
	cerr << string(errptr->bgnpos - 1, ' ') << GREEN("^");
	if(errptr->bgnlno != errptr->endlno && errptr->endpos > 1){
		cerr << endl << filecontent[errptr->endlno - 1] << endl;
		cerr << string(errptr->endpos - 2, ' ') << GREEN("~");
	}
	else if(errptr->endpos - errptr->bgnpos > 1)
		for(int i = 0;i < errptr->endpos - errptr->bgnpos - 1;++i) cerr << GREEN("~");
	cerr << endl << endl;
}

void Reporterror(Error e, const string &p, ASTptr q){
#ifdef REPORT_ERROR_DR_
	errnum += 1;
	ErrorHandler[(int)e](p, q);
#endif
}

void Reportwarning(Warning w, const string &p, ASTptr q){
#ifdef REPORT_WARNING_DR_
	warnnum += 1;
	WarningHandler[(int)w](p, q);
#endif
}

void Reportexception(Exception e, const string &p, ASTptr q){
#ifdef REPORT_EXCEPTION_DR_
	ExceptionHandler[(int)e](p, q);
#endif
}

/*** User Handlers ***/

// Error Part

void Unknowntoken(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " unrecognized token" << endl;
	if(errptr) Errorline();
	//std::exit(-1);
}

void Emptyfile(const string &p, ASTptr q){
	cerr << RED("fatal error: ") << p << ": empty file" << endl;
	cerr << "compilation terminated." << endl;
	std::exit(-1);
}

void Noinputfile(const string &p, ASTptr q){
	cerr << RED("fatal error: ") << "no input file" << endl;
	cerr << "compilation terminated." << endl;
	std::exit(-1);
}

void Filenotfound(const string &p, ASTptr q){
	cerr << RED("fatal error: ") << "file '" << p << "' not found" << std::endl;
	cerr << "compilation terminated" << endl;
	std::exit(-1);
}

// must leave
void Invalidsyntax(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << p << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

// must leave
void Devidebyzero(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << "devide by zero" << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

// must leave
void Notconstexp(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << "constant expression is expected, but got a non-const one" << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

// .
void Arrayinitmismatch(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " type mismatch, expected type: " << p << endl;
	if(errptr) Errorline();
	// std::exit(-1);
}

void Arraynegindex(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " non-positive index: " << p << endl;
	if(errptr) Errorline();
}

// must leave
void Identdeclared(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " redeclaration";
	if(p.size()) cerr << " of '" << p << "'";
	cerr << endl;
	if(errptr) Errorline();
	if(q){
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " Previous declaration was here at line " << \
		errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	std::exit(-1);
}

// must leave
void Identnotdeclared(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " undefined symbol '" << p << "'" << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

// must leave
void Toomanyindices(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " Too many indices here:" << endl;
	if(errptr) Errorline();
	if(q){
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " Declaration was here at line " << \
		errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	std::exit(-1);
}

// must leave
void Unexpectedfuncall(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " Variable is perferred, but a function is called" << endl;
	if(errptr) Errorline();
	if(q){
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " Definition of '" << p << "' was here at line " << \
		errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	std::exit(-1);
}

// must leave
void Varnotcallable(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << "Variable is not callable" << endl;
	if(errptr) Errorline();
	if(q){
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " Definition of '" << p << "' was here at line " << \
		errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	std::exit(-1);
}

// must leave
void Rparamsnomismatch(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " no matching function for call to '" << p << "'" << endl;
	if(errptr) Errorline();
	if(q){
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " candidate function not viable at line " << \
		errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	std::exit(-1);
}

void Nowhiletoescape(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " no while to " << p << endl;
	if(errptr) Errorline();
}

void Returnavalue(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " void function '" << p << "' should not return a value" << endl;
	if(errptr) Errorline();
	if(q){
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " declaration was here at line " << \
		errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
}

void Assignarray(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " array type is not assignable" << endl;
	if(errptr) Errorline();
	if(q){
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " declaration was here at line " << \
		errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
}

// must leave
void Arraybigindex(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error: ") << " Too big the index '" << p << "' is" << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

// .
void Notanumber(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " non-scalar value is provided where a scalar expression is wanted" << endl;
	if(errptr) Errorline();
	if(q) {
		ASTptr tmp = errptr;
		errptr = q;
		if(q->type == ASTType::Fundef)
			cerr << GREY("note:") << " void function '" << p << "' declared at line ";
		else
			cerr << GREY("note:") << " '" << p << "' declared at line ";
		cerr << errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	// std::exit(-1);
}

// .
void Assignconstant(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " constant '" << p << "' is not assignable" << endl;
	if(errptr) Errorline();
	if(q) {
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " it was declared here at line " \
		<< errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	// std::exit(-1);
}

// must leave
void Notapointer(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " only pointers can be assigned to a pointer" << endl;
	if(errptr) Errorline();
	if(q) {
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("not:") << " previous declaration was here at line " \
		<< errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	std::exit(-1);
}

// must leave
void Pointerdimmismatch(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " pointer type mismatch" << endl;
	if(errptr) Errorline();
	if(q) {
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " previous declaration was here at line " \
		<< errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
	std::exit(-1);
}

// .
void Voidmain(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " function 'main' should be non-void" << endl;
	if(errptr) Errorline();
}

// .
void Parammain(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " function 'main' should have no arguments" << endl;
	if(errptr) Errorline();
}

// .
void Nomain(const string &p, ASTptr q){
	cerr << RED("error:") << " No main function found in the file" << endl;
}

// .
void Constarrtoptr(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " non-const pointer '" << p << "' is pointing to a const array" << endl;
	if(errptr) Errorline();
	if(q) {
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " previous declaration was here at line " \
		<< errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
}

// must leave
void Nooutputfile(const string &, ASTptr q){
	cerr << RED("fatal error:") << " no output file " << endl;
}

void Noreturnvalue(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << RED("error:") << " non-void function '" << p << "' should return a value" << endl;
	if(errptr) Errorline();
	if(q) {
		ASTptr tmp = errptr;
		errptr = q;
		cerr << GREY("note:") << " declaration was here at line " << \
		errptr->bgnlno << ", column " << errptr->bgnpos << ":" << endl;
		Errorline();
		errptr = tmp;
	}
}

// Warning Part

void Intoverflow(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << PINK("warning:") << " integer overflow" << endl;
	if(errptr) Errorline();
}

void Identtoolong(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << PINK("warning:") << " identidier too long" << endl;
	if(errptr) Errorline();
}

// Exception Part

void Binarynullchild(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << YELLOW("exception:") << " initialize binary AST with null child" << endl;
	if(errptr) Errorline();
}

void Unexpectedop(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << YELLOW("exception:") << " unexpected op when evaluation: " << p << endl;
	if(errptr) Errorline();
}

void Unexpectedbasecall(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << YELLOW("exception:") << " unexpected base call: " << p << endl;
	if(errptr) Errorline();
}

void Arraytypeexp(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << YELLOW("exception:") << " invalid array type" << endl;
	cerr << "got type: " << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

// must leave
void Scopingexp(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << YELLOW("exception:") << " scoping error" << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

// must leave
void Unexpectednonconstexp(const string &p, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << YELLOW("exception:") << " constant expression expected. Compiler failed to examine it in advance." << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

// must leave
void Unexpectedgencall(const string &, ASTptr q){
	if(errptr) cerr << errptr->bgnlno << ":" << errptr->bgnpos << ": ";
	cerr << YELLOW("exception:") << " Errorneously call generator with incompatible AST type." << endl;
	if(errptr) Errorline();
	std::exit(-1);
}

void Errornummessage(){
	if(errnum){
		cerr << "SysY compiler: " << errnum << " errors";
		if(warnnum) cerr << " and " << warnnum << " warnings";
		cerr << " generated." << endl;
		cerr << "compilation terminated." << endl;
		std::exit(-1);
	}
}