/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *        tgrep.hpp:  Syntax Representation of Tigger        *
 *************************************************************/

#include <vector>
#include <string>
#include <iostream>
#include "tgrep.hpp"
#include "../front/global.hpp"

std::vector<std::string> regfile = {"x0", "s0", "s1", "s2", "s3", "s4", \
"s5", "s6", "s7", "s8", "s9", "s10", "s11", "t0", "t1", "t2", "t3", "t4", \
 "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
std::vector<std::string> args = {"p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7"};

std::list<TGLine> tglines;
std::unordered_map<std::string, EESymbol> ee_symtab;

bool ee_global = true;
int ee_offset = 0;
int ee_glbcnt = 0;

void TGDump(const TGLine &p){
	using std::cout;
	using std::endl;
	using std::string;

	switch(p.type){
		case TGRecord::Decl:
			if(p.num[0])
				cout << p.var << " = malloc " << p.num[0] << endl;
			else
				cout << p.var << " = 0" << endl;
			return;
		case TGRecord::Header:
			cout << p.var << " [" << p.num[0] << "] [" << p.num[1] << "]" << endl;
			return;
		case TGRecord::End:
			cout << "end " << p.var << endl;
			return;
		case TGRecord::Binary:
			cout << regfile[p.reg[0]] << " = " << regfile[p.reg[1]] \
				<< " " << p.op << " " << regfile[p.reg[2]] << endl;
			return;
		case TGRecord::Unary:
			cout << regfile[p.reg[0]] << " = " << p.op << " " << regfile[p.reg[1]] << endl;
			return;
		case TGRecord::Copy:
			cout << regfile[p.reg[0]] << " = " << regfile[p.reg[1]] << endl;
			return;
		case TGRecord::LArr:
			cout << regfile[p.reg[0]] << "[" << p.num[0] << "] = " << regfile[p.reg[1]] << endl;
			return;
		case TGRecord::RArr:
			cout << regfile[p.reg[0]] << " = " << regfile[p.reg[1]] << "[" << p.num[0] << "]" << endl;
			return;
		case TGRecord::Cond:
			cout << "if " << regfile[p.reg[0]] << " " << p.op << " " << regfile[p.reg[1]] \
				<< " goto " << p.label << endl;
			return;
		case TGRecord::Uncond:
			cout << "goto " << p.label << endl;
			return;
		case TGRecord::Label:
			cout << p.label << ":" << endl;
			return;
		case TGRecord::Ass:
			cout << regfile[p.reg[0]] << " = " << p.num[0] << endl;
			return;
		case TGRecord::Call:
			cout << "call " << p.var << endl;
			return;
		case TGRecord::Ret:
			cout << "return" << endl;
			return;
		case TGRecord::ST:
			cout << "store " << regfile[p.reg[0]] << " " << p.num[0] << endl;
			return;
		case TGRecord::LDH:
			cout << "load " << p.var << " " << regfile[p.reg[0]] << endl;
			return;
		case TGRecord::LDS:
			cout << "load " << p.num[0] << " " << regfile[p.reg[0]] << endl;
			return;
		case TGRecord::LDAH:
			cout << "loadaddr " << p.var << " " << regfile[p.reg[0]] << endl;
			return;
		default:
			cout << "loadaddr " << p.num[0] << " " << regfile[p.reg[0]] << endl;
			return;
		
	}
}

static void Op2RV(const TGLine &p){
	using std::cout;
	using std::endl;
	using std::string;

	string pref, sec;
	if(p.type == TGRecord::Binary) goto Arithmetic;
	if(p.type == TGRecord::Cond) goto Boolean;
	if(p.type == TGRecord::Unary) goto Unary;
Arithmetic:
	if(p.op == "+") pref = std::move("  add   ");
	else if(p.op == "-") pref = "  sub   ";
	else if(p.op == "*") pref = "  mul   ";
	else if(p.op == "/") pref = "  div   ";
	else if(p.op == "%") pref = "  rem   ";
	else if(p.op == "<") pref = "  slt   ";
	else if(p.op == ">") pref = "  sgt   ";
	else if(p.op == "<=") {pref = "  sgt  "; sec = "  seqz  ";}
	else if(p.op == ">=") {pref = "  slt   "; sec = "  seqz  ";}
	else if(p.op == "!=") {pref = "  xor   "; sec = "  snez  ";}
	else if(p.op == "==") {pref = "  xor   "; sec = "  seqz  ";}
	cout << pref << regfile[p.reg[0]] << ", " << regfile[p.reg[1]] << ", " << regfile[p.reg[2]] << endl;
	if(!sec.empty())
		cout << sec << regfile[p.reg[0]] << ", " << regfile[p.reg[0]] << endl;
	return;
Boolean:
	if(p.op == "<") pref = "  blt   ";
	else if(p.op == ">") pref = "  bgt   ";
	else if(p.op == "<=") pref = "  ble   ";
	else if(p.op == ">=") pref = "  bge   ";
	else if(p.op == "!=") pref = "  bne   ";
	else if(p.op == "==") pref = "  beq   ";
	cout << pref << regfile[p.reg[0]] << ", " << regfile[p.reg[1]] << ", ." << p.label << endl;
	return;
Unary:
	if(p.op == "-") pref = "  neg   ";
	else if(p.op == "!") pref = "  seqz  ";
	cout << pref << regfile[p.reg[0]] << ", " << regfile[p.reg[1]] << endl;
	return;
}

void RVDump(const TGLine &p){
	using std::cout;
	using std::endl;
	using std::string;

	static int STK = 0;

	switch(p.type){
		case TGRecord::Decl:
			if(p.num[0])
				cout << "  .comm     " << p.var << ", " << p.num[0] << ", 4" << endl;
			else{
				cout << "  .global   " << p.var << endl;
				cout << "  .section  " << ".sdata" << endl;
				cout << "  .align    " << "2" << endl;
				cout << "  .type     " << p.var << ", @object" << endl;
				cout << "  .size     " << p.var << ", 4" << endl;
				cout << p.var << ":" << endl;
				cout << "  .word     " << p.num[0] << endl;
			}
			cout << endl;
			return;
		case TGRecord::Header:
			STK = ((p.num[1] >> 2) + 2) << 4;
			cout << "  .text" << endl;
			cout << "  .align    2" << endl;
			cout << "  .global   " << p.var.substr(2) << endl;
			cout << "  .type     " << p.var.substr(2) << ", @function" << endl;
			cout << p.var.substr(2) << ":" << endl;
			cout << "  sw    ra, -4(sp)" << endl;
			// cout << "  sw    s0, -8(sp)" << endl;
			// cout << "  sw    s1, -12(sp)" << endl;
			// cout << "  sw    s2, -16(sp)" << endl;
			// cout << "  sw    s3, -20(sp)" << endl;
			// cout << "  addi  s3, sp, -20" << endl;
			if( STK <= 2048)
				cout << "  addi  sp, sp, -" << STK << endl;
			else {
				cout << "  li    t0, " << -STK << endl;
				cout << "  add   sp, sp, t0" << endl;
			}
			// cout << "  mv    s0, a0" << endl;
			// cout << "  mv    s1, a1" << endl;
			// cout << "  mv    s2, a2" << endl;
			// cout << "  mv    a0, sp" << endl;
			// cout << "  mv    a1, x0" << endl;
			// cout << "  mv    a2, x0" << endl;
			// cout << "  call  memset" << endl;
			// cout << "  mv    a0, s0" << endl;
			// cout << "  mv    a1, s1" << endl;
			// cout << "  mv    a2, s2" << endl;
			// cout << "  lw    s0, 12(s3)" << endl;
			// cout << "  lw    s1, 8(s3)" << endl;
			// cout << "  lw    s2, 4(s3)" << endl;
			// cout << "  lw    s3, 0(s3)" << endl;
			return;
		case TGRecord::End:
			cout << "  .size     " << p.var.substr(2) << ", .-" << p.var.substr(2) << endl;
			cout << endl;
			return;
		case TGRecord::Binary:
		case TGRecord::Unary:
		case TGRecord::Cond:
			Op2RV(p);
			return;
		case TGRecord::Copy:
			cout << "  mv    " << regfile[p.reg[0]] << ", " << regfile[p.reg[1]] << endl;
			return;
		case TGRecord::LArr:
			if(p.num[0] < 2048)
			cout << "  sw    " << regfile[p.reg[1]] << ", " \
				<< p.num[0] << "(" << regfile[p.reg[0]] << ")" << endl;
			else{
				cout << "  li    t0, " << p.num[0] << endl;
				cout << "  add   t0, t0, " << regfile[p.reg[0]] << endl;
				cout << "  sw    " << regfile[p.reg[1]] << ", 0(t0)" << endl;
			}
			return;
		case TGRecord::RArr:
			if(p.num[0] < 2048)
			cout << "  lw    " << regfile[p.reg[0]] << ", " \
				<< p.num[0] << "(" << regfile[p.reg[1]] << ")" << endl;
			else{
				cout << "  li    t0, " << p.num[0] << endl;
				cout << "  add   t0, t0, " << regfile[p.reg[1]] << endl;
				cout << "  lw    " << regfile[p.reg[0]] << ", 0(t0)" << endl;
			}
			return;
		case TGRecord::Uncond:
			cout << "  j  ." << p.label << endl;
			return;
		case TGRecord::Label:
			cout << "." << p.label << ":" << endl;
			return;
		case TGRecord::Ass:
			cout << "  li    " << regfile[p.reg[0]] << ", " << p.num[0] << endl;
			return;
		case TGRecord::Call:
			cout << "  call  " << p.var.substr(2) << endl;
			return;
		case TGRecord::Ret:
			if(STK < 2048)
				cout << "  addi  sp, sp, " << STK << endl;
			else{
				cout << "  li    t0, " << STK << endl;
				cout << "  add   sp, sp, t0" << endl;
			}
			cout << "  lw    ra, -4(sp)" << endl;
			cout << "  ret" << endl;
			return;
		case TGRecord::ST:
			if(p.num[0] < 512)
				cout << "  sw    " << regfile[p.reg[0]] << ", " << (p.num[0] * 4) << "(sp)" << endl;
			else{
				cout << "  li    t0, " << (p.num[0] * 4) << endl;
				cout << "  add   t0, t0, sp" << endl;
				cout << "  sw    " << regfile[p.reg[0]] << ", 0(t0)" << endl;
			}
			return;
		case TGRecord::LDH:
			cout << "  lui   " << regfile[p.reg[0]] << ", %hi(" << p.var << ")" << endl;
			cout << "  lw    " << regfile[p.reg[0]] << ", %lo(" << p.var << ")(" << regfile[p.reg[0]] << ")" << endl;
			return;
		case TGRecord::LDS:
			if(p.num[0] < 512)
				cout << "  lw    " << regfile[p.reg[0]] << ", " << (p.num[0] * 4) << "(sp)" << endl;
			else{
				cout << "  li    t0, " << (p.num[0] * 4) << endl;
				cout << "  add   t0, t0, sp" << endl;
				cout << "  lw    " << regfile[p.reg[0]] << ", 0(t0)" << endl;
			}
			return;
		case TGRecord::LDAH:
			cout << "  la    " << regfile[p.reg[0]] << ", " << p.var << endl;
			return;
		default:
			if(p.num[0] < 512)
				cout << "  addi  " << regfile[p.reg[0]] << ", sp, " << (p.num[0] * 4) << endl;
			else{
				cout << "  li    t0, " << (p.num[0] * 4) << endl;
				cout << "  add   " << regfile[p.reg[0]] << ", sp, t0" << endl;
			}
			return;
	}
}

void InsertEEEntry(const std::string &p, bool isarray){
	if(ee_symtab.count(p))
		ee_symtab[p].Update();
	else{
		if(ee_global)
			ee_symtab.emplace(p, "v" + Encodemessage(ee_glbcnt));
		else
			ee_symtab[p];
		ee_symtab[p].array = isarray;
	}
}

EESymbol &LookupEESymtab(const std::string &p){
	return ee_symtab[p];
}