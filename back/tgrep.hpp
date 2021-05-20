/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *        tgrep.hpp:  Syntax Representation of Tigger        *
 *************************************************************/

#pragma once
#ifndef SYSY_TGREP_HPP_DR_
#define SYSY_TRREP_HPP_DR_

#include <vector>
#include <string>
#include <list>
#include <unordered_map>
#define RISCV_A(x) (20 + (x))
#define RISCV_T(x) (13 + (x))
#define RISCV_S(x) (1 + (x))

extern std::vector<std::string> regfile;
extern std::vector<std::string> args;

// Decl:	VAR  [NUM]
// Header:	VAR  NUM,NUM
// End:		VAR
// Binary:	REG,REG,REG  OP
// Unary:	OP  REG,REG
// Copy:	REG,REG
// Ass:		REG  NUM
// LArr:	REG,REG  NUM
// RArr:	REG,REG  NUM
// Cond:	REG,REG  OP  LABEL
// Uncond:	LABEL
// Label:	LABEL
// Call:	VAR
// Ret:
// ST:		REG  NUM
// LDH:		VAR  REG
// LDS:		REG  NUM
// LDAH:	VAR  REG
// LDAS:	REG  NUM

enum class TGRecord{ Decl, Header, End, \
Binary, Unary, Copy, Ass, LArr, RArr, Cond, Uncond, Label, Call, Ret, ST, LDH, LDS, LDAH, LDAS}; 

extern bool ee_global;
extern int ee_offset;
extern int ee_glbcnt;

class TGLine{
public:
	TGLine(TGRecord t): type(t) {}
	TGLine(TGRecord t, std::string &&p): type(t){
		if(t == TGRecord::Call || t == TGRecord::End)
			var = p;
		else if(t == TGRecord::Uncond || t == TGRecord::Label)
			label = p;
		else if(t == TGRecord::Decl){
			var = p;
			num[0] = 0;
		}
	}
	TGLine(TGRecord t, int _reg, int _num): type(t){
		if(t == TGRecord::LDAS || t == TGRecord::LDS \
			|| t == TGRecord::ST || t == TGRecord::Ass){
			reg[0] = _reg;
			num[0] = _num;
		}
		else if(t == TGRecord::Copy){
			reg[0] = _reg;
			reg[1] = _num;
		}
	}
	TGLine(TGRecord t, std::string &&p, int ind): type(t) {
		if(t == TGRecord::Decl) {var = p; num[0] = ind;}
		else if(t == TGRecord::LDH || t == TGRecord::LDAH) {
			var = p;
			reg[0] = ind;
		}
	}
	TGLine(TGRecord t, std::string &&p, int a, int b):type(t) {
		if(t == TGRecord::Header){
			var = p;
			num[0] = a;
			num[1] = b;
		}
		else if(t == TGRecord::Unary){
			op = p;
			reg[0] = a;
			reg[1] = b;
		}
	}
	TGLine(TGRecord t, int a, int b, int c): type(t){
		if(t == TGRecord::LArr || t == TGRecord::RArr){
			reg[0] = a;
			reg[1] = b;
			num[0] = c;
		}
	}
	TGLine(TGRecord t, int a, int b, int c, std::string &&p):type(t) {
		if(t == TGRecord::Binary){
			reg[0] = a;
			reg[1] = b;
			reg[2] = c;
			op = p;
		}
	}
	TGLine(TGRecord t, int a, int b, std::string &&p, std::string &&l):type(t) {
		if(t == TGRecord::Cond){
			reg[0] = a;
			reg[1] = b;
			op = p;
			label = l;
		}
	}

	// Dump as Tigger
	void TGDump();
	// Dump as RISC-V
	void RVDump();

	TGRecord type;
	std::string var;
	int reg[3];
	int num[2];
	std::string label;
	std::string op;
};

class EESymbol{
public:
	EESymbol(): global(ee_global), offset(ee_offset) {}
	EESymbol(std::string &&t): name(t), global(ee_global), offset(ee_offset) {}
	void Update() {global = ee_global; offset = ee_offset;}

	std::string name;
	bool global;
	bool array;
	int offset; // Offset from base position of heap or procedural stack. It must be a multiple of 4.
};

extern std::list<TGLine> tglines;
extern std::unordered_map<std::string, EESymbol> ee_symtab;

void InsertEEEntry(const std::string &p, bool isarray);
EESymbol &LookupEESymtab(const std::string &p);
void TGDump(const TGLine &p);
void RVDump(const TGLine &p);
#endif