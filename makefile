# This is Ding Rui's mini SysY Compiler
F=front
B=back

OBJS = \
  $F/parser.tab.o \
  $F/lex.yy.o \
  $F/global.o \
  $F/baseast.o \
  $F/error.o \
  $F/ast.o \
  $F/strtab.o \
  $F/symtab.o \
  $F/flow.o \
  $F/geneey.o \
  $F/eerep.o \
  $F/optimee.o \
  $B/gentg.o \
  $B/tgrep.o \
  $B/optimtg.o \
  main.o

Header = \
  $F/*.hpp \
  $B/*.hpp

compiler: $(OBJS)
	g++ -std=c++11 -o compiler $(OBJS) -ll -ly -I.

$F/lex.yy.cpp: $F/lex.l
	lex -o $@ $<

$F/parser.tab.cpp: $F/parser.y
	yacc -d -o $@ $< -Wno-yacc

$F/%.o: $F/%.cpp $(Header)
	g++ -std=c++11 -c -o $@ $< -O2 -Wno-deprecated-register -I./front

$B/%.o: $B/%.cpp $(Header)
	g++ -std=c++11 -c -o $@ $< -O2 -I./back

main.o: main.cpp $(Header)
	g++ -std=c++11 -c -o $@ $< -O2 -I.

clean:
	rm -f $(OBJS)

cleanall:
	rm -f $(OBJS) $F/parser.tab.hpp $F/parser.tab.cpp $F/lex.yy.cpp compiler