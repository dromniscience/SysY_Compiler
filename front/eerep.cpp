/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *        eerep.cpp:  Syntax Representation of Eeyore        *
 *************************************************************/

#include "eerep.hpp"
#include <iostream>
#include <list>

std::list<EELine> eelines;

void EEDump(const EELine &p){
	using std::cout;
	using std::endl;
	using std::string;

	switch(p.type){
		case EERecord::Decl:
			cout << "var ";
			if(!p.num.empty())
				cout << p.num << " ";
			cout << p.sym[0] << endl;
			return;
		case EERecord::Header:
			cout << p.sym[0] << " [" << p.num << "]" << endl;
			return;
		case EERecord::End:
			cout << "end " << p.sym[0] << endl;
			return;
		case EERecord::Binary:
			if(p.label.empty()) // Arithmetic Op
			cout << p.sym[0] << " = " << p.sym[1] \
				<< " " << p.op << " " << p.sym[2] << endl;
			else{ // Logic Op
			cout << "if " << p.sym[1] << " " << p.op << " " << p.sym[2] \
				<< " goto " << p.label << endl;
			cout << p.sym[0] << " = 0" << endl;
			cout << "goto " << p.num << endl;
			cout << p.label << ":" << endl;
			cout << p.sym[0] << " = 1" << endl;
			cout << p.num << ":" << endl;
			}
			return;
		case EERecord::Unary:
			cout << p.sym[0] << " = " << p.op << " " << p.sym[1] << endl;
			return;
		case EERecord::Copy:
			cout << p.sym[0] << " = " << p.sym[1] << endl;
			return;
		case EERecord::LArr:
			cout << p.sym[0] << "[" << p.sym[1] << "] = " << p.sym[2] << endl;
			return;
		case EERecord::RArr:
			cout << p.sym[0] << " = " << p.sym[1] << "[" << p.sym[2] << "]" << endl;
			return;
		case EERecord::Cond:
			cout << "if " << p.sym[0] << " " << p.op << " " << p.sym[1] \
				<< " goto " << p.label << endl;
			return;
		case EERecord::Uncond:
			cout << "goto " << p.label << endl;
			return;
		case EERecord::Label:
			cout << p.label << ":" << endl;
			return;
		case EERecord::Param:
			cout << "param " << p.sym[0] << endl;
			return;
		case EERecord::Asscall:
			cout << p.sym[0] << " = call " << p.sym[1] << endl;
			return;
		case EERecord::Voidcall:
			cout << "call " << p.sym[0] << endl;
			return;
		case EERecord::Voidret:
			cout << "return" << endl;
			return;
		default:
			cout << "return " << p.sym[0] << endl;
			return;
	}
}