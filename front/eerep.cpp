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

void EELine::EEDump(){
	using std::cout;
	using std::endl;
	using std::string;

	switch(type){
		case EERecord::Decl:
			cout << "var ";
			if(!num.empty())
				cout << num << " ";
			cout << sym[0] << endl;
			return;
		case EERecord::Header:
			cout << sym[0] << " [" << num << "]" << endl;
			return;
		case EERecord::End:
			cout << "end " << sym[0] << endl;
			return;
		case EERecord::Binary:
			if(label.empty()) // Arithmetic Op
			cout << sym[0] << " = " << sym[1] \
				<< " " << op << " " << sym[2] << endl;
			else{ // Logic Op
			cout << "if " << sym[1] << " " << op << " " << sym[2] \
				<< " goto " << label << endl;
			cout << sym[0] << " = 0" << endl;
			cout << "goto " << num << endl;
			cout << label << ":" << endl;
			cout << sym[0] << " = 1" << endl;
			cout << num << ":" << endl;
			}
			return;
		case EERecord::Unary:
			cout << sym[0] << " = " << op << " " << sym[1] << endl;
			return;
		case EERecord::Copy:
			cout << sym[0] << " = " << sym[1] << endl;
			return;
		case EERecord::LArr:
			cout << sym[0] << "[" << sym[1] << "] = " << sym[2] << endl;
			return;
		case EERecord::RArr:
			cout << sym[0] << " = " << sym[1] << "[" << sym[2] << "]" << endl;
			return;
		case EERecord::Cond:
			cout << "if " << sym[0] << " " << op << " " << sym[1] \
				<< " goto " << label << endl;
			return;
		case EERecord::Uncond:
			cout << "goto " << label << endl;
			return;
		case EERecord::Label:
			cout << label << ":" << endl;
			return;
		case EERecord::Param:
			cout << "param " << sym[0] << endl;
			return;
		case EERecord::Asscall:
			cout << sym[0] << " = call " << sym[1] << endl;
			return;
		case EERecord::Voidcall:
			cout << "call " << sym[0] << endl;
			return;
		case EERecord::Voidret:
			cout << "return" << endl;
			return;
		default:
			cout << "return " << sym[0] << endl;
			return;
	}
}