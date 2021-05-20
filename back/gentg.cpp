/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *        gentg.cpp:  Syntax Representation of Tigger        *
 *************************************************************/

#include "../front/eerep.hpp"
#include "tgrep.hpp"
#include "../front/global.hpp"

using EEptr = std::list<EELine>::iterator;

inline bool IsEEVar(const std::string &p){
	return p[0] == 't' || p[0] == 'T' || p[0] == 'p';
}

// overwrite reg
static void EmitNum2Reg(int no, int reg){
	tglines.emplace_back(TGRecord::Ass, reg, no);
}

// overwrite reg
static void EmitRvar2Reg(const std::string &p, int reg){
	EESymbol &tmp = LookupEESymtab(p);
	// array
	if(tmp.array){
		if(tmp.global)
			tglines.emplace_back(TGRecord::LDAH, std::string(tmp.name), reg);
		else
			tglines.emplace_back(TGRecord::LDAS, reg, tmp.offset >> 2);
	}
	else{
		if(tmp.global)
			tglines.emplace_back(TGRecord::LDH, std::string(tmp.name), reg);
		else
			tglines.emplace_back(TGRecord::LDS, reg, tmp.offset >> 2);
	}
}

// overwrite reg
static void EmitRval(const std::string &p, int reg){
	if(!IsEEVar(p)){
		int tmp;
		sscanf(p.c_str(), "%d", &tmp);
		EmitNum2Reg(tmp, reg);
	}
	else{
		EmitRvar2Reg(p, reg);
	}
}

// overwrite t0 & reg
static void EmitSingleAss(const std::string &p, int reg){
	EESymbol &tmp = LookupEESymtab(p);
	if(tmp.global){
		tglines.emplace_back(TGRecord::LDAH, std::string(tmp.name), RISCV_T(0));
		tglines.emplace_back(TGRecord::LArr, RISCV_T(0), reg, 0);
	}
	else
		tglines.emplace_back(TGRecord::ST, reg, tmp.offset >> 2);
}

// overwrite t0 & dest
static void EmitArr2Reg(const std::string &p, int reg, int dest){
	EESymbol &tmp = LookupEESymtab(p);
	// An array
	if(tmp.array){
		if(tmp.global)
			tglines.emplace_back(TGRecord::LDAH, std::string(tmp.name), RISCV_T(0));
		else
			tglines.emplace_back(TGRecord::LDAS, RISCV_T(0), tmp.offset >> 2);
	}
	else{
		if(tmp.global)
			tglines.emplace_back(TGRecord::LDH, std::string(tmp.name), RISCV_T(0));
		else
			tglines.emplace_back(TGRecord::LDS, RISCV_T(0), tmp.offset >> 2);
	}
	tglines.emplace_back(TGRecord::Binary, RISCV_T(0), RISCV_T(0), reg, std::string("+"));
	tglines.emplace_back(TGRecord::RArr, dest, RISCV_T(0), 0);
}

// overwrite t0
static void EmitArrAss(const std::string &p, int reg, int src){
	EESymbol &tmp = LookupEESymtab(p);
	// An array
	if(tmp.array){
		if(tmp.global)
			tglines.emplace_back(TGRecord::LDAH, std::string(tmp.name), RISCV_T(0));
		else
			tglines.emplace_back(TGRecord::LDAS, RISCV_T(0), tmp.offset >> 2);
	}
	else
		tglines.emplace_back(TGRecord::LDS, RISCV_T(0), tmp.offset >> 2);
	tglines.emplace_back(TGRecord::Binary, RISCV_T(0), RISCV_T(0), reg, std::string("+"));
	tglines.emplace_back(TGRecord::LArr, RISCV_T(0), src, 0);
}

void EE2TG(EEptr p){
	static int info = 0;
	static int callarg = 0;
	switch(p->type){
		case EERecord::Decl:
			InsertEEEntry(p->sym[0], !p->num.empty());
			// A scalar
			if(p->num.empty()) {
				if(ee_global)
					tglines.emplace_back(TGRecord::Decl, "v" + Encodemessage(ee_glbcnt));
				ee_offset += 4;
			}
			// An array
			else{
				int tmp;
				sscanf(p->num.c_str(), "%d", &tmp);
				if(ee_global)
					tglines.emplace_back(TGRecord::Decl, "v" + Encodemessage(ee_glbcnt),  tmp);
				ee_offset += tmp;
			}
			if(ee_global) ee_glbcnt += 1;
			// End of Decl in a function
			if(!ee_global && (++p)->type != EERecord::Decl){
				tglines.back().num[1] = ee_offset >> 2;
				// Store Params
				for(int i = 0;i < info;++i){
					ee_offset = i << 2;
					InsertEEEntry(args[i], false);
					tglines.emplace_back(TGRecord::ST, RISCV_A(i), i);
				}
			}
			return;
		case EERecord::Header:
		{
			sscanf(p->num.c_str(), "%d", &info);
			ee_global = false;
			ee_offset = info << 2;
			tglines.emplace_back(TGRecord::Header, std::move(p->sym[0]), info, 0);
			
			++p;
			// End of Decl in a function
			if(p->type != EERecord::Decl){
				tglines.back().num[1] = ee_offset >> 2;
				// Store Params
				for(int i = 0;i < info;++i){
					ee_offset = i << 2;
					InsertEEEntry(args[i], false);
					tglines.emplace_back(TGRecord::ST, RISCV_A(i), i);
				}
			}
			return;
		}
		case EERecord::End:
		{
			ee_global = true;
			ee_offset = 0;
			info = 0;
			tglines.emplace_back(TGRecord::End, std::move(p->sym[0]));
			return;
		}
		case EERecord::Label:
			tglines.emplace_back(TGRecord::Label, std::move(p->label));
			return;
		case EERecord::Voidret:
			tglines.emplace_back(TGRecord::Ret);
			return;
		case EERecord::Uncond:
			tglines.emplace_back(TGRecord::Uncond, std::move(p->label));
			return;
		case EERecord::Binary:
		{
			int dest = RISCV_T(1);
			int lop = RISCV_T(1);
			int rop = RISCV_T(2);
			EmitRval(p->sym[1], lop);
			EmitRval(p->sym[2], rop);
			tglines.emplace_back(TGRecord::Binary, dest, lop, rop, std::move(p->op));
			EmitSingleAss(p->sym[0], dest);
			return;
		}
		case EERecord::Unary:
		{
			int op = RISCV_T(1);
			int dest = RISCV_T(1);
			EmitRval(p->sym[1], op);
			tglines.emplace_back(TGRecord::Unary, std::move(p->op), dest, op);
			EmitSingleAss(p->sym[0], dest);
			return;
		}
		case EERecord::Copy:
		{
			int val = RISCV_T(1);
			EmitRval(p->sym[1], val);
			EmitSingleAss(p->sym[0], val);
			return;
		}
		case EERecord::LArr:
		{
			int ind = RISCV_T(1);
			int val = RISCV_T(2);
			EmitRval(p->sym[1], ind);
			EmitRval(p->sym[2], val);
			EmitArrAss(p->sym[0], ind, val);
			return;
		}
		case EERecord::RArr:
		{
			int ind = RISCV_T(1);
			int val = RISCV_T(2);
			EmitRval(p->sym[2], ind);
			EmitArr2Reg(p->sym[1], ind, val);
			EmitSingleAss(p->sym[0], val);
			return;
		}
		case EERecord::Cond:
		{
			int lop = RISCV_T(1);
			int rop = RISCV_T(2);
			EmitRval(p->sym[0], lop);
			EmitRval(p->sym[1], rop);
			tglines.emplace_back(TGRecord::Cond, lop, rop, std::move(p->op), std::move(p->label));
			return;
		}
		case EERecord::Voidcall:
			callarg = 0;
			tglines.emplace_back(TGRecord::Call, std::move(p->sym[0]));
			return;
		case EERecord::Param:
			EmitRval(p->sym[0], RISCV_A(callarg));
			callarg += 1;
			return;
		case EERecord::Asscall:
			callarg = 0;
			tglines.emplace_back(TGRecord::Call, std::move(p->sym[1]));
			EmitSingleAss(p->sym[0], RISCV_A(0));
			return;
		case EERecord::Ret:
			EmitRval(p->sym[0], RISCV_A(0));
			tglines.emplace_back(TGRecord::Ret);
			return;
		default: return;
	}
}

void TranslateEE2TG(){
	for(auto i = eelines.begin(); i != eelines.end(); ++i)
		EE2TG(i);
}

void DumpTG2file(){
	for(auto &i: tglines)
		i.TGDump();
}

void DumpRV2file(){
	for(auto &i: tglines)
		i.RVDump();
}


