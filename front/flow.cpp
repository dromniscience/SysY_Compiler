/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *             flow.cpp:  Class For Flow Control             *
 *************************************************************/

#include "baseast.hpp"

std::list<ASTptr> whilestk;
ASTptr curfundef;