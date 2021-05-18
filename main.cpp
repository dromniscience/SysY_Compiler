/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *                     main.cpp:  main()                     *
 *************************************************************/

#include "front/ast.hpp"
#include "front/error.hpp"
#include "front/geneey.hpp"
#include "front/parser.tab.hpp"
#include "front/optimee.hpp"
#include "front/eerep.hpp"

#include "back/gentg.hpp"
#include "back/optimtg.hpp"


#include <iostream>
#include <fstream>
#include <cstring>

#define EEMODE 0
#define TGMODE 1
#define RVMODE 2
extern FILE *yyin, *yyout;

static inline void Helper(const char *p){
	std::cerr << "usage: " << p << " -S [-e <INPUT> | -t <INPUT>] -o <OUTPUT>" << std::endl;
}

int main(int argc, char *argv[]){
	using std::string;
	
	if(argc < 2){
		Helper(argv[0]);
		exit(-1);
	}

	// parsing command line parameters
	int opt, mode;
	char *infile = nullptr, *outfile = nullptr;
	for(int i = 1;i < argc;++i){
		if(!strcmp(argv[i], "-S")){
			if(i == argc - 1) break;
			if(argv[i + 1][0] != '-') {
				infile = argv[i + 1];
				mode = RVMODE;
			}
		}
		else if(!strcmp(argv[i], "-t")){
			if(i == argc - 1) break;
			if(argv[i + 1][0] != '-') {
				infile = argv[i + 1];
				mode = TGMODE;
			}
		}
		else if(!strcmp(argv[i], "-e")){
			if(i == argc - 1) break;
			if(argv[i + 1][0] != '-') {
				infile = argv[i + 1];
				mode = EEMODE;
			}
		}
		else if(!strcmp(argv[i], "-o")){
			if(i == argc - 1) break;
			if(argv[i + 1][0] != '-')
				outfile = argv[i + 1];
		}
		else if(!strcmp(argv[i], "-h")){
			Helper(argv[0]);
			exit(-1);
		}
	}

	if(!infile)
		Reporterror(Error::Noinputfile);

	FILE *file;
	file = fopen(infile, "r");
	if(!file){ /* open failed */
		Reporterror(Error::Filenotfound, string(infile));
	}
	yyin = file;
	Filepreread(infile);

	// output file
	if(!outfile) Reporterror(Error::Nooutputfile);
		
	file = fopen(outfile, "w");
	if(!file){ /* open failed */
		Reporterror(Error::Filenotfound, string(outfile));
	}
	yyout = file;
	std::ofstream fout(outfile);
	std::streambuf *oldbuf = std::cout.rdbuf(fout.rdbuf());
	
	// parser
	ASTptr root;
	yyparse(&root);
	
	if(!mainptr)
		Reporterror(Error::Nomain);

	Errornummessage();

	// Debugsymtab(glbst);
	// root->Debug(0);

	// eeyore generator
	Treatmain(root);
	TraverseAST(root);

	// Naive Opt (EE-level)
	NaiveEEOpt();
	// Eliminate redundant Basic Block (Eeyore-level)
	RedundantBBEE();

	if(mode == EEMODE)
		DumpEE2file();
	else {
		TranslateEE2TG();

		// Naive Opt (TG-level)
		EliminateSTLDTG();
		if(mode == TGMODE)
			DumpTG2file();
		else
			DumpRV2file();
	}

	// release resources
	std::cout.rdbuf(oldbuf);
	fout.close();
}