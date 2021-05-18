/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *        eerep.hpp:  Syntax Representation of Eeyore        *
 *************************************************************/

#pragma once
#ifndef SYSY_EEREP_HPP_DR_
#define SYSY_EEREP_HPP_DR_
#include <string>
#include <list>

// Decl:		SYM [NUM]
// Header:		SYM  NUM
// End:			SYM
// Binary:		SYM,RV1,RV2 OP
// Unary:		SYM,RV1 OP
// Copy:		SYM,RV1
// LArr:		SYM,RV1,RV2
// RArr:		SYM,SYM,RV1
// Cond:		RV1,RV2 OP LABEL
// UnCond:		LABEL
// Label:		LABEL
// Param:		RV
// Voidcall:	SYM
// Asscall:		SYM,SYM
// Voidret:		
// Ret:			RV

enum class EERecord{Decl, Header, End, \
Binary, Unary, Copy, LArr, RArr, Cond, Uncond, Label, Param, \
Voidcall, Asscall, Voidret, Ret};

class EELine{
public:
	EELine(EERecord t):type(t) {}
	EELine(EERecord t, std::string &&p): type(t) {
		if(t == EERecord::Ret || t == EERecord::Param \
			|| t == EERecord::Voidcall || t == EERecord::End) sym[0] = p;
		else if(t == EERecord::Uncond || t == EERecord::Label) label = p;
		else if(t == EERecord::Decl) sym[0] = p;
	}
	EELine(EERecord t, std::string &&p, std::string &&q): type(t) {
		if(t == EERecord::Decl || t == EERecord::Header) {sym[0] = p; num = q;}
		else if (t == EERecord::Copy) {sym[0] = p; sym[1] = q;}
		else if (t == EERecord::Asscall) {sym[0] = p; sym[1] = q;}
	}
	EELine(EERecord t, std::string &&p, std::string &&q, std::string &&r):type(t) {
		if(t == EERecord::Unary) {sym[0] = p; sym[1] = q; op = r;}
		else if(t == EERecord::LArr || t == EERecord::RArr) {sym[0] = p; sym[1] = q; sym[2] = r;}
	}
	EELine(EERecord t, std::string &&p, std::string &&q, std::string &&r, std::string &&s):type(t) {
		if(t == EERecord::Binary) {sym[0] = p; sym[1] = q; sym[2] = r; op = s;}
		else if(t == EERecord::Cond) {sym[0] = p; sym[1] = q; op = r; label = s;}
	}

	EERecord type;
	std::string sym[3];
	std::string op;
	std::string label;
	std::string num;

	int opt_bno;
};

void EEDump(const EELine &);

extern std::list<EELine> eelines;


#endif
