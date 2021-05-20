/*************************************************************
 *              Compiler: Principles & Practice              *
 *             Ding Rui, dromniscience@gmail.com             *
 *                       2021,  Spring                       *
 *                                                           *
 *                       SysY Compiler                       *
 *                                                           *
 *          optimee.cpp:  Eeyore-level Optimization          *
 *************************************************************/

#include "eerep.hpp"
#include <vector>
#include <unordered_map>
#define BGBB 0
#define EDBB -1

using EEptr = std::list<EELine>::iterator;

class BBEE{
public:
	BBEE(int no, EEptr p, EEptr q): bno(no), bgn(p), end(q), visited(false) {
		nbno[0] = nbno[1] = EDBB;
	}

	int bno; // block no [into bblist]
	EEptr bgn, end;
	int nbno[2]; // child block no
	bool visited; // used in function VisitBBEE
};

// List of basic block (EE-level)
static std::vector<BBEE> bblist;
// Mapped a temporary to its declaration (EE-level)
static std::unordered_map<std::string, EEptr> eerec;
// Mapped a constant temporary to its value (EE-level)
static std::unordered_map<std::string, std::string> eeval;


static void VisitBBEE(BBEE &p){
	if(p.visited) return;
	p.visited = true;
	if(p.nbno[0] != EDBB) VisitBBEE(bblist[p.nbno[0]]);
	if(p.nbno[1] != EDBB) VisitBBEE(bblist[p.nbno[1]]);
}

// Common subexpression (EE-level)
static void CommonsubexpBBEE(BBEE &p){
	// std::unordered_map<
}


// Build up basic block (EE-level)
static EEptr InFuncBBEE(EEptr p){
	// assert(p->type == EERecord::Header);

	// Record Label
	std::unordered_map<std::string, int> tmp_label;

	bblist.clear();
	p->opt_bno = BGBB;
	bblist.emplace_back(BGBB, p, ++p);
	int bno = BGBB + 1;

	auto q = p;
	while(true){
		// End of a basic block
		if(q->type == EERecord::Ret || q->type == EERecord::Voidret\
		|| q->type == EERecord::Uncond || q->type == EERecord::Cond){
			q->opt_bno = bno;
			bblist.emplace_back(bno, p, ++q);
			bno += 1;
			p = q;
		}
		else if(q->type == EERecord::Label){
			if(p == q) {q->opt_bno = bno; ++q; tmp_label.emplace(p->label, bno); continue;}
			// End of a basic block
			bblist.emplace_back(bno, p, q);
			bno += 1;
			p = q;
		}
		else if(p->type == EERecord::End){
			if(p == q) break;
			bblist.emplace_back(bno, p, q);
			p = q;
			break;
		}
		else {q->opt_bno = bno; ++q;}
	}

	// Mark Exit
	int size = bblist.size();
	for(int i = 0;i < size;++i){
		if(!i) bblist[i].nbno[0] = 1;
		else {
			auto t = bblist[i].end;
			--t;
			// Ends with return
			if(t->type == EERecord::Ret || t->type == EERecord::Voidret) continue;
			// Ends with unconditional jump
			if(t->type == EERecord::Uncond)
				bblist[i].nbno[0] = tmp_label[t->label];
			// End with conditional jump
			else if(t->type == EERecord::Cond){
				if(i != size - 1) bblist[i].nbno[0] = bblist[i].bno + 1;
				bblist[i].nbno[1] = tmp_label[t->label];
			}
			// The only reason here why a basic block ends is due to a following label or function endmark
			else if(bblist[i].end->type == EERecord::Label){
				bblist[i].nbno[0] = bblist[i].bno + 1;
			}
		}
	}
	
	// Scan
	VisitBBEE(bblist[0]);

	// Eliminate Inaccessible BB
	int last = 0;
	for(int i = 1; i < size; ++i)
		if(!bblist[i].visited){
			bblist[last].end = bblist[i].end;
			eelines.erase(bblist[i].bgn, bblist[i].end);
		}
		else last = i;
	
	// End of function
	return p;
}

// Remove dead BB (EE-level)
void RedundantBBEE(){
	EEptr i = eelines.begin();
	while(i->type != EERecord::Header) i++;

	while(true){
		i = InFuncBBEE(i);
		i++;
		if(i == eelines.end()) break;
	}
}


static inline bool IsEEVar(std::string &p){
	return p[0] == 't' || p[0] == 'T' || p[0] == 'p';
}

static inline std::string CalculateEE(std::string &p, std::string &q, std::string &op){
	int tmp1, tmp2;
	char a[100];
	sscanf(p.c_str(), "%d", &tmp1);
	sscanf(q.c_str(), "%d", &tmp2);
	if(op == "+") tmp1 += tmp2;
	else if(op == "-") tmp1 -= tmp2;
	else if(op == "*") tmp1 *= tmp2;
	else if(op == "/") tmp1 /= tmp2;
	else if(op == "%") tmp1 %= tmp2;
	else if(op == "<") tmp1 = tmp1 < tmp2;
	else if(op == ">") tmp1 = tmp1 > tmp2;
	else if(op == "<=") tmp1 = tmp1 <= tmp2;
	else if(op == ">=") tmp1 = tmp1 >= tmp2;
	else if(op == "==") tmp1 = tmp1 == tmp2;
	else if(op == "!=") tmp1 = tmp1 != tmp2;
	sprintf(a, "%d", tmp1);
	return a;
}

// Label absorption
// 1-stride boolean exp absorption
// 1-stride assignment absorption
// Arithmetic eliminations
void NaiveEEOpt(){
	for(auto i = eelines.begin(); i != eelines.end();++i){
		if(i->type == EERecord::Decl){
			eerec.emplace(i->sym[0], i);
			continue;
		}

		// This first line may be of use in a Cond EERecord
		if(eeval.count(i->sym[0])) i->sym[0] = eeval[i->sym[0]];
		if(eeval.count(i->sym[1])) i->sym[1] = eeval[i->sym[1]];
		if(eeval.count(i->sym[2])) i->sym[2] = eeval[i->sym[2]];

		// Const boolean exp evaluation
		if(i->type == EERecord::Cond){
			if(!IsEEVar(i->sym[0]) && !IsEEVar(i->sym[1])){
				if(CalculateEE(i->sym[0], i->sym[1], i->op) == "1")
					i->type = EERecord::Uncond;
				else {
					auto p = i--;
					eelines.erase(p);
				}
				continue;
			}
		}
		
		if(i->type == EERecord::Binary){
			// The op must be logical
			if(i->op[0] == '<' || i->op[0] == '>' || i->op.back() == '='){
				auto p = i; p++;
				// 1-stride boolean exp absorption
				if(p->type == EERecord::Cond && i->sym[0][0] == 't' && \
				p->sym[0] == i->sym[0] && p->sym[1] == "0" && p->op == "!="){
					p->sym[0] = i->sym[1];
					p->sym[1] = i->sym[2];
					p->op = i->op;
					eelines.erase(eerec[i->sym[0]]);
					eelines.erase(i);
					i = --p;
					continue;
				}
			}
		}

		if(i->type == EERecord::Binary || i->type == EERecord::Unary \
		|| i->type == EERecord::Asscall || i->type == EERecord::RArr){
			EEptr q = i; ++q;
			// 1-stride assignment absorption
			if(i->sym[0][0] == 't' && q->type == EERecord::Copy \
			&& q->sym[1] == i->sym[0]){
				i->sym[0] = q->sym[0];
				eelines.erase(eerec[q->sym[1]]);
				eelines.erase(q);
			}
		}
		// Arithmetic Elimination
		if(i->type == EERecord::Binary){
			// Compiling time calculation (many have already be performed at the front end)
			if(!IsEEVar(i->sym[1]) && !IsEEVar(i->sym[2])){
				i->type = EERecord::Copy;
				i->sym[1] = CalculateEE(i->sym[1], i->sym[2], i->op);
			}
			// Arithmetic identities
			else if(i->sym[2] == "0"){
				if(i->op == "+" || i->op == "-")
					i->type = EERecord::Copy;
				else if(i->op == "*"){
					i->type = EERecord::Copy;
					i->sym[1] = "0";
				}
			}
			else if(i->sym[2] == "1"){
				if(i->op == "/" || i->op == "*")
					i->type = EERecord::Copy;
				if(i->op == "%"){
					i->type = EERecord::Copy;
					i->sym[1] = "0";
				}
			}
		}
		if(i->type == EERecord::Copy){
			// source code is ganruateed to be SSA with respect to tempos
			// substitute tempos whose value is compiling-time constant
			if(i->sym[0][0] == 't' && !IsEEVar(i->sym[1])){
				eeval[i->sym[0]] = i->sym[1];
				eelines.erase(eerec[i->sym[0]]);
				auto p = i--;
				eelines.erase(p);
				continue;
			}
		}
	}
}