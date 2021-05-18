/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *           lex.l:  Auto-generated Lexer for SysY           *
 *************************************************************/

%{
// #define DEBUG_LEXER_L_DR_
#define UPDATE_LINEPOS_ {tokenpos += yyleng; tokenwidth = yyleng;}

#include <sstream>
#include "global.hpp"
#include "token.hpp"
#include "baseast.hpp"
#include "error.hpp"
#include "parser.tab.hpp"

static int Convert2int(const char *);
static int Countlines();


#ifdef DEBUG_LEXER_L_DR_
static void Testandprint(const char *);
YYSTYPE yylval;
#endif


%}

%option noyywrap

/*** Regular Definitions ***/
nondigit	[_a-zA-Z]
decdigit	[0-9]
hexdigit	[0-9a-fA-F]
octdigit	[0-7]
space		[ \t\r]+
singleton	[\+\-\!\*\/\%\>\<]
delimiter	[\,\;\[\]\=\{\}\(\)]
newline		[\n]

/* Identifier */
ident		{nondigit}({nondigit}|{decdigit})*

/* Integer Constant */
decimal		[1-9]{decdigit}*
hexadecimal	0[xX]{hexdigit}+
octal		0{octdigit}*
integer		({decimal}|{hexadecimal}|{octal})

/* Line Comment & Block Comment */
lcomment	\/\/.*
bcomment	\/\*([^\*]|(\*)*[^\*/])*(\*)*\*\/

%%

{newline}	{lineno += 1; tokenpos = 1; }
{space}		{UPDATE_LINEPOS_ }
{singleton} {
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(*yytext);
			return *yytext;
			}
{delimiter} {
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(*yytext);
			return *yytext;
			}
"const"		{
			UPDATE_LINEPOS_ 
			yylval.tkptr = new Token(CONST);
			return CONST;
			}
"int"		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(INT);
			return INT;
			}
"void"		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(VOID);
			return VOID;
			}
"if"		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(IF);
			return IF;
			}
"else"		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(ELSE);
			return ELSE;
			}
"while"		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(WHILE);
			return WHILE;
			}
"break"		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(BREAK);
			return BREAK;
			}
"continue"	{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(CONTINUE);
			return CONTINUE;
			}
"return"	{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(RETURN);
			return RETURN;
			}
"<="		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(LE);
			return LE;
			}
">="		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(GE);
			return GE;
			}
"=="		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(EQ);
			return EQ;
			}
"!="		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(NE);
			return NE;
			}
"&&"		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(AND);
			return AND;
			}
"||"		{
			UPDATE_LINEPOS_
			yylval.tkptr = new Token(OR);
			return OR;
			}
{ident}		{
			UPDATE_LINEPOS_ 
			if(yyleng > MAXIDENTLENGTH){
				errptr = new BaseAST(lineno, tokenpos - tokenwidth, lineno, tokenpos);
				Reportwarning(Warning::Identtoolong);
				delete errptr;
				errptr = nullptr;
			}
			yylval.tkptr = new Token(yytext, IDENT);
			return IDENT;
			}
{integer}	{
			UPDATE_LINEPOS_ 
			yylval.tkptr = new Token(Convert2int(yytext), INT_CONST);
			return INT_CONST;
			}
{lcomment}	{UPDATE_LINEPOS_ }
{bcomment}	{
			int lines = Countlines();
			lineno += lines;
			tokenpos = lines? 1 + yyleng: tokenpos + yyleng;
			}
.			{
			UPDATE_LINEPOS_
			errptr = new BaseAST(lineno, tokenpos - tokenwidth, lineno, tokenpos);
			Reporterror(Error::Unknowntoken);
			delete errptr;
			errptr = nullptr;
			}

%%

/*** Auxiliary Functions ***/

// Count the lines within a block comment
int Countlines(){
	int lines = 0;
	yyleng = 0;
	while(*yytext){
		if(*yytext == '\n') {lines += 1; yyleng = 0;}
		else {yyleng += 1;}
		yytext++;
	}
	return lines;
}

// Convert a string to int. Base 8, 10, 16 are supported.
int Convert2int(const char *yytext){
	using std::stringstream;
	using std::hex;
	using std::oct;
	
	long result;
	stringstream sst;
	if(*yytext != '0') sst << yytext;
	else if(yyleng == 1) return 0;
	else if(yytext[1] == 'x' || yytext[1] == 'X') sst << hex << yytext;
	else sst << oct << yytext;
	
	sst >> result;
	if(result & (1L << 31)) {
		errptr = new BaseAST(lineno, tokenpos - tokenwidth, lineno, tokenpos);
		Reportwarning(Warning::Intoverflow);
		delete errptr;
		errptr = nullptr;
	}
	return result & 0x7FFFFFFF;
}

#ifdef DEBUG_LEXER_L_DR_
void Testandprint(const char *ptr){
	using std::strcmp;
	using std::fprintf;
	if(yyleng != yylval.tkptr->end - yylval.tkptr->bgn || tokenpos != yylval.tkptr->end) fprintf(yyout, RED("ERROR!\n"));
	if(!strcmp(ptr, "integer")) fprintf(yyout, "%d:%d: %s, %s = %d\n", lineno, tokenpos - tokenwidth, ptr, yytext, yylval.tkptr->val);
	else if(!strcmp(ptr, "identifier")) fprintf(yyout, "%d:%d: %s, %s\n", lineno, tokenpos - tokenwidth, ptr, yylval.tkptr->name);
	else fprintf(yyout, "%d:%d: %s, %s\n", lineno, tokenpos - tokenwidth, ptr, yytext);
}
#endif

#ifdef DEBUG_LEXER_L_DR_
// Debugging.
// Print the result of lexer verbatim.
void Debug(){
	int token;
	while(token = yylex()){
		switch(token){
			case IDENT:	Testandprint("identifier"); break;
			case INT_CONST: Testandprint("integer"); break;
			case CONST:
			case INT:
			case VOID:
			case IF:
			case ELSE:
			case WHILE:
			case BREAK:
			case CONTINUE:
			case RETURN: Testandprint("keyword"); break;
			case '+':
			case '-':
			case '*':
			case '/':
			case '!':
			case '%':
			case '>':
			case '<':
			case GE:
			case LE:
			case EQ:
			case NE:
			case AND:
			case OR: Testandprint("op"); break;
			case ';':
			case ',':
			case '[':
			case ']':
			case '=':
			case '{':
			case '}':
			case '(':
			case ')': Testandprint("delimiter"); break;
			default: Testandprint("error\n");
		}
		if(yylval.tkptr) delete yylval.tkptr;
	}
}
#endif

#ifdef DEBUG_LEXER_L_DR_
int main(int argc, char *argv[]){
	using std::string;
	if(argc == 1) Reporterror(Error::Noinputfile);
	
	FILE *file;
	file = fopen(argv[1], "r");
	if(!file){ /* open failed */
		Reporterror(Error::Filenotfound, string(argv[1]));
	}

	yyin = file;
	Filepreread(argv[1]);

	if(argc > 2){
		FILE *file;
		file = fopen(argv[2], "w");
		if(!file){ /* open failed */
			Reporterror(Error::Filenotfound, string(argv[2]));
		}
		yyout = file;
	}
	
	Debug();
}
#endif