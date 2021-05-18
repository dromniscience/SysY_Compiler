/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *             flow.hpp:  Class For Flow Control             *
 *************************************************************/

#pragma once
#ifndef SYSY_FLOW_HPP_DR_
#define SYSY_FLOW_HPP_DR_

#include "baseast.hpp"
#include <list>

extern std::list<ASTptr> whilestk;
extern ASTptr curfundef;

inline void Pushwhilestk(ASTptr p) {whilestk.push_back(p); }
inline void Popwhilestk() {whilestk.pop_back();}
inline ASTptr Topwhilestk() {return whilestk.back();}
inline bool Isemptywhilestk() {return whilestk.empty();}


#endif