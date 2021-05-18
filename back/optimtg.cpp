/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *          optimtg.cpp:  Tigger-level Optimization          *
 *************************************************************/

#include "tgrep.hpp"

void EliminateSTLDTG(){
	for(auto i = tglines.begin(); i != tglines.end(); ++i){
		if(i->type != TGRecord::ST) continue;
		auto p = i; p++;
		if(p->type != TGRecord::LDS || i->num[0] != p->num[0]) continue;
		if(i->reg[0] == p->reg[0]) tglines.erase(p);
		else{
			p->type = TGRecord::Copy;
			p->reg[1] = i->reg[0];
		}
	}
}