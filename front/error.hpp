/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *                error.hpp: Class For Errors                *
 *************************************************************/

#pragma once
#ifndef SYSY_ERROR_HPP_DR_
#define SYSY_ERROR_HPP_DR_

#include "baseast.hpp"
#include <string>

extern ASTptr errptr;

// Error Type
enum class Error {Unknowntoken, Emptyfile, Noinputfile, Filenotfound, Invalidsyntax, Devidebyzero, \
Notconstexp, Arrayinitmismatch, Arraynegindex, Identdeclared, Identnotdeclared, Toomanyindices, \
Unexpectedfuncall, Varnotcallable, Rparamsnomismatch, Nowhiletoescape, Returnavalue, Assignarray, Arraybigindex, \
Notanumber, Assignconstant, Notapointer, Pointerdimmismatch, Voidmain, Parammain, Nomain, Constarrtoptr, Nooutputfile, \
Noreturnvalue, _Final};
enum class Warning {Intoverflow, Identtoolong, _Final};
enum class Exception {Binarynullchild, Unexpectedop, Unexpectedbasecall, Arraytypeexp, Scopingexp, \
Unexpectednonconstexp, Unexpectedgencall, _Final};

// Error Handler
void Reporterror(Error, const std::string & = std::string(), ASTptr = nullptr);
void Reportwarning(Warning, const std::string & = std::string(), ASTptr = nullptr);
void Reportexception(Exception, const std::string & = std::string(), ASTptr = nullptr);

void Errornummessage();

#endif