/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *             geneey.hpp:  Generate eeyore file             *
 *************************************************************/

#pragma once
#ifndef SYSY_GENEEY_HPP_DR_
#define SYSY_GENEEY_HPP_DR_

#include <string>
#include "global.hpp"
#include "baseast.hpp"

extern int paramno;
extern int endono;
extern int temporaryno;
extern int labelno;

inline std::string Setparamaddr(){
	return "p" + Encodemessage(paramno);
}

inline std::string Setendoaddr(){
	return "T" + Encodemessage(endono);
}

inline std::string Settemporaryaddr(){
	return "t" + Encodemessage(temporaryno);
}

inline std::string Setlabeladdr(){
	return "l" + Encodemessage(labelno);
}

inline std::string Setfuncaddr(const std::string &p){
	return "f_" + p;
}

inline void Resetparamno(){
	paramno = 0;
}

inline void Increaseparamno(){
	paramno += 1;
}

inline void Increaseendono(){
	endono += 1;
}

inline void Increasetemporaryno(){
	temporaryno += 1;
}

inline void Increaselabelno(){
	labelno += 1;
}

void Treatmain(ASTptr);

void TraverseAST(ASTptr);

void DumpEE2file();
#endif