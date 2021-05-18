/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *              baseast.cpp: Base Class for AST              *
 *************************************************************/

#include "baseast.hpp"
#include "error.hpp"


// BaseAST

void BaseAST::Eval() {Reportexception(Exception::Unexpectedbasecall);}

bool BaseAST::Isconst() {Reportexception(Exception::Unexpectedbasecall); return false;}
int BaseAST::Cval() {Reportexception(Exception::Unexpectedbasecall); return 0;}
bool BaseAST::Isanumber() {Reportexception(Exception::Unexpectedbasecall); return false;}
void BaseAST::Generator() {Reportexception(Exception::Unexpectedbasecall);}
void BaseAST::Dump2file() {Reportexception(Exception::Unexpectedbasecall);}