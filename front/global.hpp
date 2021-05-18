/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *              global.hpp: Global Declarations              *
 *************************************************************/

#pragma once
#ifndef SYSY_GLOBAL_HPP_DR_
#define SYSY_GLOBAL_HPP_DR_
#include <vector>
#include <string>

/*** Global Macros ***/

#define RED(message) ("\033[1;31m" message "\033[0m")
#define PINK(message) ("\033[1;35m" message "\033[0m")
#define GREEN(message) ("\033[1;32m" message "\033[0m")
#define YELLOW(message) ("\033[1;33m" message "\033[0m")
#define GREY(message)	("\033[1;90m" message "\033[0m")

#define MAXIDENTLENGTH 31

/*** Global Variables ***/

// Position Information
extern int lineno, tokenpos, tokenwidth;

// Block #. 0 for the global scope
extern int blockno;

extern std::vector<std::string> filecontent;

/*** Global Functions ***/

// File Preread in (to facilitate error reporting)
void Filepreread(const std::string &);

// Array Dimension
std::vector<int> Arraydimprocess(const std::vector<int> &);

// Given a partial list of indices, return the start # of the array
int Arraylinearno(const std::vector<int> &, const std::vector<int> &);

// Encode error message
std::string Encodemessage(const std::vector<int> &p);
std::string Encodemessage(int p);

#endif