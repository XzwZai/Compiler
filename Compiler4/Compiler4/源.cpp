#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <map>
#include <set>
#include <stack>
#include <vector>
#include <sstream>
#include <cstdlib>
#pragma warning (disable : 4996)
#define Word_Max 100
#define Tab_Max 100
#define Func_Max 100
#define Num_Max 2147483647 //2^31-1
#define judge(x) \
if(sy == x) {\
getsymbol();\
}
using namespace std;

void expression();
void statement();

enum mipsop { madd, maddiu, msub, msubiu, mmult, mdiv, mlw, msw, mjr, mjal, mj, mbeq, mbne, mblt, mble, mbgt, mbge, msyscall, msll, mmflo, mnop, mgenlab };
string mipsopstr[] = { "addu","addiu","sub","subiu","mult","div","lw","sw","jr","jal","j","beq","bne","blt","ble","bgt","bge","syscall","sll","mflo", "nop" };
enum opsymbol {
	addop, subop, multop, divop, jal, j, became, beqop, bneop, bgtop, bgeop, bltop, bleop, readop, writeop,
	loadop, storeop, funcbeginop, returnop, pushop, getreturnop, genlab
};
string opsymbolstr[] = {
	"addop", "subop", "multop", "divop", "jal", "j", "became", "beqop", "bneop", "bgtop", "bgeop", "bltop", "bleop", "readop", "writeop",
	"loadop", "storeop", "funcbeginop", "returnop", "pushop", "getreturnop", "genlab"
};
enum object { constty, globalvarty, varty, functionty };
enum type { intty, charty, voidty, noty, stringty };
enum symbol {
	constsy, intsy, charsy, voidsy, mainsy, ifsy, elsesy, dosy, whilesy, forsy, scanfsy, printfsy, returnsy,
	ident, intcon, charcon, stringcon,
	plusy, minusy, multsy, divsy,
	eql, neq, leq, lss, geq, grt, becames,
	lparent, rparent, lbrack, rbrack, lbrace, rbrace,
	comma, semicolon, quote,
	endsy
};
string symbols[] = { "constsy","intsy","charsy","voidsy","mainsy","ifsy","elsesy","dosy","whilesy","forsy","scanfsy","printfsy","returnsy",
"ident","intcon","charcon","stringcon",
"plusy","minusy","multsy","divsy",
"eql","neq","leq","lss","geq","grt","becames",
"lparent","rparent","lbrack","rbrack","lbrace","rbrace",
"comma","semicolon","quote" };
struct record
{
	string name;
	int link;
	object obj;
	type typ;
	int mult;
	int ref;
	int lv;
	int adr;
	int value;
};

struct opnum
{
	int obj;						//1:ident;2:linshi;3:intcon,charcon;4:label;5:string
	type typ;						//1:int;2:char;3:string
	int value;						//ident:loc int tab;linshi:xv hao;3:value;4:xv hao;5:loc int consttab
};

struct code
{
	opsymbol op;
	opnum *num[3];
	int finaladr;
};

struct funcrecord
{
	type functype;
	int last;
	int lastpar;
	int psize;
	int vsize;
	int codestart;
	int codeend;
	int tempsize;
	int globalregsize;
	opnum *label;
	vector<code> funmidcodes;
};

struct constrecord
{
	int type;
	int size;
	int value;
	string svalue;
	int adr;
};

struct finalopnum
{
	int type;						//1:lijishu;2:jicunqi
	int value;
};

struct finalcode
{
	int adr;
	mipsop op;
	finalopnum num[3];
};

int tempindex = 0;
int partadr = 0;
int staticadr = 0x10010000;
int errortag = 1;
int endtag = 0;
symbol sy;
FILE *in;
FILE *midcodeout;
FILE *codeout;
int codeadr = 0x00400000;
char str[Word_Max];
string line = "";
int hnumber;
char ch;
char charactor;
string word;
vector<funcrecord> ftab;
vector<record> tab;
vector<constrecord> consttab;
vector<code> midcodes;
vector<finalcode> finalcodes;
type curfunctype;
int curfunc;
int curlv;
int curchindex = 0;
int curchbg = 0;
int curlineindex = 0;
int labelcount = 0;
int errorcount = 0;
map<string, symbol> reservermap;
map<char, symbol> specialmap;
map<symbol, opsymbol> sy2op;
set<symbol> emptyset;
set<symbol> endset;
set<symbol> beginset;
set<symbol> dividesyset;
set<symbol> statebgset;
set<symbol> expbgset;
set<symbol> compareset;
set<mipsop> needlabelset;
stack<opnum*> numst;
stack<symbol> opst;
vector<string> errorinfos;
void initmap()
{
	record r;
	tab.insert(tab.end(), r);

	reservermap.insert(pair<string, symbol>("const", constsy));
	reservermap.insert(pair<string, symbol>("int", intsy));
	reservermap.insert(pair<string, symbol>("char", charsy));
	reservermap.insert(pair<string, symbol>("void", voidsy));
	reservermap.insert(pair<string, symbol>("main", mainsy));
	reservermap.insert(pair<string, symbol>("if", ifsy));
	reservermap.insert(pair<string, symbol>("else", elsesy));
	reservermap.insert(pair<string, symbol>("do", dosy));
	reservermap.insert(pair<string, symbol>("while", whilesy));
	reservermap.insert(pair<string, symbol>("for", forsy));
	reservermap.insert(pair<string, symbol>("scanf", scanfsy));
	reservermap.insert(pair<string, symbol>("printf", printfsy));
	reservermap.insert(pair<string, symbol>("return", returnsy));

	specialmap.insert(pair<char, symbol>('+', plusy));
	specialmap.insert(pair<char, symbol>('-', minusy));
	specialmap.insert(pair<char, symbol>('*', multsy));
	specialmap.insert(pair<char, symbol>('/', divsy));
	specialmap.insert(pair<char, symbol>('{', lbrace));
	specialmap.insert(pair<char, symbol>('}', rbrace));
	specialmap.insert(pair<char, symbol>('(', lparent));
	specialmap.insert(pair<char, symbol>(')', rparent));
	specialmap.insert(pair<char, symbol>('[', lbrack));
	specialmap.insert(pair<char, symbol>(']', rbrack));
	specialmap.insert(pair<char, symbol>(';', semicolon));
	specialmap.insert(pair<char, symbol>(',', comma));
	sy2op.insert(pair<symbol, opsymbol>(plusy, addop));
	sy2op.insert(pair<symbol, opsymbol>(minusy, subop));
	sy2op.insert(pair<symbol, opsymbol>(multsy, multop));
	sy2op.insert(pair<symbol, opsymbol>(divsy, divop));
	sy2op.insert(pair<symbol, opsymbol>(eql, beqop));
	sy2op.insert(pair<symbol, opsymbol>(neq, bneop));
	sy2op.insert(pair<symbol, opsymbol>(grt, bgtop));
	sy2op.insert(pair<symbol, opsymbol>(geq, bgeop));
	sy2op.insert(pair<symbol, opsymbol>(lss, bltop));
	sy2op.insert(pair<symbol, opsymbol>(leq, bleop));

}

void initset()
{
	endset.insert(semicolon);
	endset.insert(comma);

	beginset.insert(constsy);
	beginset.insert(ifsy);
	beginset.insert(dosy);

	dividesyset.insert(semicolon);
	dividesyset.insert(rbrace);
	dividesyset.insert(lbrace);

	statebgset.insert(dosy);
	statebgset.insert(ifsy);
	statebgset.insert(forsy);
	statebgset.insert(lbrace);
	statebgset.insert(ident);
	statebgset.insert(scanfsy);
	statebgset.insert(printfsy);
	statebgset.insert(returnsy);
	statebgset.insert(semicolon);

	expbgset.insert(plusy);
	expbgset.insert(minusy);
	expbgset.insert(ident);
	expbgset.insert(intcon);
	expbgset.insert(charcon);
	expbgset.insert(lparent);

	compareset.insert(eql);
	compareset.insert(neq);
	compareset.insert(leq);
	compareset.insert(lss);
	compareset.insert(geq);
	compareset.insert(grt);

	needlabelset.insert(mjal);
	needlabelset.insert(mj);
	needlabelset.insert(mbeq);
	needlabelset.insert(mbne);
	needlabelset.insert(mbgt);
	needlabelset.insert(mblt);
	needlabelset.insert(mble);
	needlabelset.insert(mbgt);
	needlabelset.insert(mbge);
}

void error(int kind, string word)
{
	string oneerror = "";
	oneerror += "error: ";
	switch (kind)
	{
	case 1:
		oneerror += "the size of array must be bigger than 0\n";
		break;
	case 2:
		oneerror += "the type of main function must be void\n";
		break;
	case 3:
		oneerror += "unsighed number can not begin with 0\n";
		break;
	case 4:
		oneerror += "illegal charactor " + word + "\n";
		break;
	case 5:
		oneerror += "multiple definition of " + word + "\n";
		break;
	case 6:
		oneerror += "too much parameters\n";
		break;
	case 7:
		oneerror += "too little parameters\n";
		break;
	case 8:
		oneerror += "undefined ident " + word + "\n";
		break;
	case 9:
		oneerror += "type should be int or char\n";
		break;
	case 10:
		oneerror += "constant " + word + " can not be changed\n";
		break;
	case 11:
		oneerror += "number is too big\n";
		break;
	case 12:
		oneerror += "expect " + word + "\n";
		break;
	case 13:
		oneerror += "illegal charractor type identifier\n";
		break;
	case 14:
		oneerror += "void function can not be factor\n";
		break;
	case 15:
		oneerror += "type of identifier " + word + " should be var\n";
		break;
	case 16:
		oneerror += "identifier " + word + " should not be an array\n";
		break;
	case 17:
		oneerror += "program not completed\n";
		break;
	case 18:
		oneerror += "voidty function should not return value\n";
		break;
	case 19:
		oneerror += "intty or charty function should return value\n";
		break;
	default:
		break;
	}
	cout << oneerror.c_str();
	errorinfos.push_back(oneerror);
	errorcount++;
}

void newcode(opsymbol op, opnum *num1, opnum *num2, opnum *num3) {
	code co;
	co.num[0] = num1;
	co.num[1] = num2;
	co.num[2] = num3;
	co.op = op;
	midcodes.insert(midcodes.end(), co);
	ftab[curfunc].funmidcodes.push_back(co);
	cout << "\t" << curfunc << opsymbolstr[op] << endl;
}

opnum *newlabel()
{
	opnum *num = new opnum;
	num->obj = 4;
	num->value = labelcount++;
	return num;
}

opnum *newfac(type t, int v) {
	opnum *num = new opnum;
	num->obj = 3;
	num->typ = t;
	num->value = v;
	return num;
}

opnum *gettemp(type t)
{
	opnum *num = new opnum;
	num->obj = 2;
	num->typ = t;
	num->value = tempindex;
	tempindex++;
	return num;
}

opnum *numstpush(int o, type t, int a) {
	opnum *num = new opnum;
	num->obj = o;
	num->typ = t;
	num->value = a;
	numst.push(num);
	return num;
}

opnum *numstpop() {
	if (numst.size() == 0) {
		return newfac(intty, 0);
	}
	opnum *num = numst.top();
	numst.pop();
	return num;
}

void popandcode() {
	opnum *num2 = numstpop();
	opnum *num1 = numstpop();
	symbol opsy = opst.top();
	opst.pop();
	if (num1->obj == 3 && num2->obj == 3) {
		int result = 0;
		switch (opsy)
		{
		case plusy:
			result = num1->value + num2->value;
			break;
		case minusy:
			result = num1->value - num2->value;
			break;
		case multsy:
			result = num1->value * num2->value;
			break;
		case divsy:
			result = num1->value / num1->value;
			break;
		default:
			break;
		}
		opnum *num3 = newfac(intty, result);
		numst.push(num3);
		return;
	}
	opnum *num3 = gettemp(intty);
	numst.push(num3);
	opsymbol op;
	op = sy2op.find(opsy)->second;
	if (num1->obj == 3 && op == addop) {
		newcode(op, num2, num1, num3);
	}
	else {
		newcode(op, num1, num2, num3);
	}
}

void opstpush(symbol ne) {
	if (opst.size() == 0) {
		opst.push(ne);
		return;
	}
	symbol ol = opst.top();
	if (ne == lparent) {
		opst.push(ne);
	}
	else if (ne == rparent) {
		while (opst.top() != lparent) {
			popandcode();
		}
		opst.pop();
	}
	else {
		if (ol == lparent) {
			opst.push(ne);
		}
		else if (ne == plusy || ne == minusy) {
			while (opst.size() > 0 && opst.top() != lparent) {
				popandcode();
			}
			opst.push(ne);
		}
		else if (ne == multsy || ne == divsy) {
			if (ol == multsy || ol == divsy) {
				popandcode();
			}
			opst.push(ne);
		}
		else {
			opst.push(ne);
		}
	}
}

void grammarerror(set<symbol> s)
{
	string strexp = "error: expect ";
	string linenum;
	string chnum;
	stringstream ss;
	ss << curlineindex;
	ss >> linenum;
	ss.clear();
	ss << curchbg;
	ss >> chnum;
	ss.clear();
	set<symbol>::iterator it;
	symbol syexp;
	for (it = s.begin(); it != s.end(); it++) {
		syexp = *it;
		strexp += symbols[int(syexp)] + ",";
	}
	strexp += "before line " + linenum + " charactor " + chnum + " " + "\'" + line[curchbg - 1] + "\'\n";
	cout << strexp;
	errortag = 0;
	errorcount++;
}

void nextch()
{
	char c;
	if (line == "" || ch == '\n' || curchindex >= line.size()) {
		if (line != "") {
			cout << curlineindex << '\t' + line;
		}
		curlineindex++;
		line = "";
		curchindex = 0;
		while (1) {
			if (fscanf(in, "%c", &c) != EOF) {
				line += c;
				if (c == '\n') {
					break;
				}
			}
			else {
				line += '\0';
				break;
			}
		}
	}
	ch = line[curchindex++];
}

void getsymbol()
{
	int k = 0, over = 0, zero = 0, numl = 0;
	word = "";
	while (ch == '\n' || ch == '\t' || ch == ' ' || ch == '\0') {
		if (ch == '\0') {
			if (endtag == 0) {
				endtag++;
			}
			else {
				sy = endsy;
				return;
			}
		}
		nextch();
	}
	curchbg = curchindex;
	if ((ch <= 'z' && ch >= 'a') || (ch <= 'Z' && ch >= 'A') || (ch == '_')) {
		word += ch;
		nextch();
		while ((ch <= 'z' && ch >= 'a') || (ch <= 'Z' && ch >= 'A') || (ch == '_') || (ch <= '9' && ch >= '0')) {
			word += ch;
			nextch();
		}
		map<string, symbol>::iterator it;
		it = reservermap.find(word);
		if (it == reservermap.end()) {
			sy = ident;
		}
		else {
			sy = it->second;
		}
	}
	else if (ch <= '9' && ch >= '0') {
		hnumber = 0;
		numl = 0;
		hnumber = hnumber * 10 + ch - '0';
		numl++;
		if (ch == '0') {
			zero = 1;
		}
		nextch();
		while (ch <= '9' && ch >= '0') {
			if (hnumber < Num_Max / 10 || (hnumber == Num_Max && ch <= Num_Max % 10 + '0')) {
				hnumber = hnumber * 10 + ch - '0';
				numl++;
			}
			else {
				over = 1;
			}
			nextch();
		}
		if (over) {
			error(11, "");
		}
		if (zero && (numl > 1)) {
			error(3, "");
		}
		sy = intcon;
	}
	else if (ch == '<') {
		k = 0;
		word += ch;
		nextch();
		if (ch == '=') {
			word += ch;
			sy = leq;
			nextch();
		}
		else {
			sy = lss;
		}
	}
	else if (ch == '>') {
		k = 0;
		word += ch;
		nextch();
		if (ch == '=') {
			word += ch;
			sy = geq;
			nextch();
		}
		else {
			sy = grt;
		}
	}
	else if (ch == '!') {
		k = 0;
		word += ch;
		nextch();
		if (ch == '=') {
			word += ch;
			sy = neq;
			nextch();
		}
		else {
			error(12, "=");
		}
	}
	else if (ch == '=') {
		k = 0;
		word += ch;
		nextch();
		if (ch == '=') {
			word += ch;
			sy = eql;
			nextch();
		}
		else {
			sy = becames;
		}
	}
	else if (ch == '\'') {
		sy = charcon;
		nextch();
		int n = 0;
		while ((ch == '_') || (ch == '+') || (ch == '-') || (ch == '*') || (ch == '/') || (ch <= '9' && ch >= '0') || (ch <= 'z' && ch >= 'a') || (ch <= 'Z' && ch >= 'A')) {
			n++;
			charactor = ch;
			nextch();
		}
		if (ch == '\'') {
			if (n == 0) {
				charactor = '\0';
				error(13, "");
			}
			else if (n > 1) {
				error(13, "");
			}
			nextch();
		}
		else {
			if (n == 0) {
				charactor = '\0';
			}
			error(13, "");
		}

	}
	else if (ch == '"') {
		sy = stringcon;
		nextch();
		k = 0;
		while ((ch == 32) || (ch == 33) || (ch <= 126 && ch >= 35)) {
			word += ch;
			nextch();
		}
		if (ch != '"') {
			error(12, "\"");
		}
		else {
			nextch();
		}

	}
	else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '{' || ch == '}' || ch == '(' || ch == ')'
		|| ch == '[' || ch == ']' || ch == ';' || ch == ',') {
		map<char, symbol>::iterator it = specialmap.find(ch);
		sy = it->second;
		k = 0;
		word += ch;
		nextch();
	}
	else {
		error(4, "");
		nextch();
	}
	errortag = 1;
}

set<symbol> createset(set<symbol> se, int count, ...)
{
	set<symbol> result;
	set<symbol>::iterator it;
	symbol syinsert;
	for (it = se.begin(); it != se.end(); it++) {
		syinsert = *it;
		result.insert(syinsert);
	}
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; i++) {
		syinsert = (symbol)va_arg(args, int);
		result.insert(syinsert);
	}
	return result;
}

set<symbol> createset(int count, ...)
{
	set<symbol> result;
	symbol syinsert;
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; i++) {
		syinsert = (symbol)va_arg(args, int);
		result.insert(syinsert);
	}
	return result;
}

void skip(set<symbol> end)
{
	while (end.find(sy) == end.end()) {
		getsymbol();
		errortag = 0;
	}
}

void test(set<symbol> best, set<symbol> end)
{
	if (sy == endsy) {
		error(17, "");
		return;
	}
	if (best.find(sy) == best.end()) {
		if (errortag) {
			grammarerror(best);
		}
		while (best.find(sy) == best.end() && end.find(sy) == end.end()) {
			getsymbol();
			if (sy == endsy) {
				break;
			}
			errortag = 0;
		}
	}
}

int enterstatic(int type, int mult, int value, string svalue) {
	constrecord cr;
	cr.type = type;
	if (type == 1 || type == 2) {
		staticadr = staticadr % 4 == 0 ? staticadr : (staticadr / 4 + 1) * 4;
		cr.adr = staticadr;
		cr.value = value;
		staticadr += mult * 4;
		cr.size = mult * 4;
	}
	else if (type == 3) {
		cr.adr = staticadr;
		cr.value = staticadr;
		cr.svalue = svalue;
		int length = svalue.length();
		staticadr += length + 1;
		cr.size = length + 1;
	}
	consttab.insert(consttab.end(), cr);
	return consttab.size() - 1;
}

int enter(string name, object obj, type ty, int lv)
{
	int i = ftab[curfunc].last;
	int it = i;
	record r;
	while (it != 0) {
		if (tab[it].name != name) {
			it = tab[it].link;
		}
		else {
			error(5, name);
			return 0;
		}
	}

	r.name = name;
	r.obj = obj;
	r.typ = ty;
	r.lv = lv;
	r.link = i;
	r.mult = 0;
	if (obj == constty || obj == globalvarty) {
		staticadr = staticadr % 4 == 0 ? staticadr : (staticadr / 4 + 1) * 4;
		r.adr = staticadr;
	}
	ftab[curfunc].last = tab.size();
	tab.insert(tab.end(), r);
	return tab.size() - 1;
}

void enterfunc(type t)
{
	funcrecord fr;
	fr.last = 0;
	fr.lastpar = 0;
	fr.functype = t;
	ftab.push_back(fr);
}

int loc(string s)
{
	int ti = ftab[curfunc].last;
	while (ti != 0) {
		if (tab[ti].name == s) {
			return ti;
		}
		ti = tab[ti].link;
	}
	ti = ftab[0].last;
	while (ti != 0) {
		if (tab[ti].name == s) {
			return ti;
		}
		ti = tab[ti].link;
	}
	return ti;
}

int integer()
{
	int sign = 1;
	test(createset(3, plusy, minusy, intcon), dividesyset);
	if (sy == plusy || sy == minusy) {
		sign = sy == plusy ? 1 : -1;
		getsymbol();
		test(createset(1, intcon), createset(dividesyset, 2, semicolon, comma));
		if (sy == intcon) {
			getsymbol();
			if (hnumber == 0) {
				cout << "error: \"+0\" or \"-0\" error" << endl;
				errorcount++;
			}
			return hnumber*sign;
		}
	}
	else if (sy == intcon) {
		getsymbol();
		return hnumber;
	}
	return 0;
}

void constdec()
{
	int tabi;
	set<symbol> tset;
	type t = noty;
	while (sy == constsy) {
		tset = createset(emptyset, 4, plusy, minusy, intcon, charcon);
		getsymbol();
		test(createset(emptyset, 2, intsy, charsy), createset(tset, 4, ident, becames, comma, semicolon));
		if (sy == intsy || sy == charsy) {
			t = sy == intsy ? intty : charty;
			if (sy == intsy) {
				tset.erase(charcon);
			}
			else {
				tset.erase(plusy);
				tset.erase(minusy);
				tset.erase(intcon);
			}
			getsymbol();
		}
		while (1) {
			tabi = 0;
			test(createset(emptyset, 1, ident), createset(tset, 3, becames, comma, semicolon));
			if (sy == ident) {
				tabi = enter(word, constty, t, curlv);
				getsymbol();
			}
			test(createset(emptyset, 1, becames), createset(tset, 2, comma, semicolon));
			judge(becames);
			test(tset, createset(emptyset, 2, comma, semicolon));
			if (tset.find(sy) != tset.end()) {
				if (sy == charcon) {
					if (tabi != 0) {
						enterstatic(2, 1, charactor, "");
					}
					getsymbol();
				}
				else {
					int getint = integer();
					if (tabi != 0) {
						enterstatic(1, 1, getint, "");
					}
				}
			}
			test(createset(emptyset, 2, comma, semicolon), createset(emptyset, 1, constsy));
			if (sy != comma) {
				break;
			}
			else {
				getsymbol();
			}
		}
		test(createset(emptyset, 1, semicolon), createset(emptyset, 1, constsy));
		judge(semicolon);
		test(createset(statebgset, 4, constsy, intsy, charsy, voidsy), emptyset);
	}
}

int parameterlist()
{
	int parnum = 0;
	type t = intty;
	while (1)
	{
		test(createset(3, intsy, charsy, rparent), dividesyset);
		if (sy == intsy || sy == charsy) {
			t = sy == intsy ? intty : charty;
			getsymbol();
		}
		else {
			break;
		}
		test(createset(emptyset, 1, ident), createset(dividesyset, 2, comma, rparent));
		if (sy == ident) {
			enter(word, varty, t, curlv);
			partadr -= 4;
			tab[tab.size() - 1].adr = partadr;
			parnum++;
			getsymbol();
		}
		test(createset(emptyset, 2, rparent, comma), dividesyset);
		if (sy == comma) {
			getsymbol();
		}
		else {
			break;
		}
	}
	if (parnum != 0) {
		ftab[curfunc].lastpar = tab.size() - 1;
	}
	getsymbol();
	return parnum;
}

void callfunc(int loc)
{
	int it = loc;
	int parnum = 0;
	test(createset(emptyset, 1, lparent), createset(dividesyset, 1, rparent));
	judge(lparent);
	vector<opnum*> params;
	while (1) {
		if (sy == rparent) {
			break;
		}
		it++;
		expression();
		parnum++;
		opnum *num1 = numstpop();
		if (it > ftab[tab[loc].ref].lastpar) {
			error(6, "");
		}
		else {
			if (num1->typ == intcon && tab[it].typ == charty) {
				cout << "warnning: int to char" << endl;
			}
			params.push_back(num1);
		}
		test(createset(emptyset, 2, comma, rparent), dividesyset);
		if (sy == comma) {
			getsymbol();
			continue;
		}
		else {
			break;
		}
	}
	judge(rparent);
	if (it < ftab[tab[loc].ref].lastpar) {
		error(7, "");
	}
	for (int i = 0; i < params.size(); i++) {
		opnum *par = params[i];
		newcode(pushop, newfac(intty, i + 1), NULL, par);
	}
	newcode(jal, NULL, NULL, ftab[tab[loc].ref].label);
}

void factor()
{
	int loca = 0;
	test(createset(6, plusy, minusy, ident, intcon, charcon, lparent), dividesyset);
	if (sy == plusy || sy == minusy || sy == intcon)
	{
		int getint = integer();
		numstpush(3, intty, getint);
	}
	else if (sy == ident)
	{
		loca = loc(word);
		if (loca == 0) {
			error(8, word);
			getsymbol();
		}
		else {
			if (tab[loca].obj == functionty) {
				if (tab[loca].typ == voidty) {
					error(14, "");
				}
				getsymbol();
				callfunc(loca);
				opnum *returnvalue;
				if (tab[loca].typ == charty || tab[loca].typ == intty) {
					returnvalue = gettemp(tab[loca].typ);
				}
				else {
					returnvalue = gettemp(intty);
				}
				newcode(getreturnop, NULL, NULL, returnvalue);
				numst.push(returnvalue);
			}
			else {
				if (tab[loca].mult != 0) {
					getsymbol();
					test(createset(1, lbrack), createset(dividesyset, 2, semicolon, rbrack));
					judge(lbrack);
					expression();
					opnum *num1 = numstpop();
					opnum *num2 = new opnum;
					num2->obj = 1;
					num2->typ = tab[loca].typ;
					num2->value = loca;
					opnum *num3;
					if (tab[loca].typ == charty || tab[loca].typ == intty) {
						num3 = gettemp(tab[loca].typ);
					}
					else {
						num3 = gettemp(intty);
					}
					newcode(loadop, num2, num1, num3);
					numst.push(num3);
					test(createset(1, rbrack), createset(dividesyset, 1, semicolon));
					judge(rbrack);
				}
				else {
					numstpush(1, tab[loca].typ, loca);
					getsymbol();
				}
			}
		}
	}
	else if (sy == charcon) {
		numstpush(3, charty, charactor);
		getsymbol();
	}
	else if (sy == lparent)
	{
		getsymbol();
		expression();
		test(createset(1, rparent), dividesyset);
		if (sy == rparent) {
			getsymbol();
		}
	}
}

void term()
{
	factor();
	while (sy == multsy || sy == divsy)
	{
		opstpush(sy);
		getsymbol();
		factor();
	}
}

void expression()
{
	opstpush(lparent);
	if (sy == plusy || sy == minusy) {
		if (sy == minusy) {
			numstpush(3, intty, 0);
			opstpush(sy);
		}
		getsymbol();
	}
	term();
	while (sy == minusy || sy == plusy) {
		opstpush(sy);
		getsymbol();
		term();
	}
	opstpush(rparent);
}

void ifstatement()
{
	test(createset(1, lparent), createset(dividesyset, 12, plusy, minusy, ident, intcon, charcon, rparent, eql, neq, leq, lss, geq, grt));
	judge(lparent);
	expression();
	opnum *num1 = numstpop();
	opnum *num2;
	opnum *label1;
	if (sy == eql || sy == neq || sy == leq || sy == lss || sy == geq || sy == grt) {
		opsymbol op = beqop;
		switch (sy)
		{
		case eql:
			op = bneop;
			break;
		case neq:
			op = beqop;
			break;
		case lss:
			op = bgeop;
			break;
		case leq:
			op = bgtop;
			break;
		case geq:
			op = bltop;
			break;
		case grt:
			op = bleop;
			break;
		default:
			break;
		}
		getsymbol();
		expression();
		num2 = numstpop();
		label1 = newlabel();
		newcode(op, num1, num2, label1);
	}
	else {
		num2 = newfac(intty, 0);
		label1 = newlabel();
		newcode(beqop, num1, num2, label1);
	}
	test(createset(1, rparent), dividesyset);
	judge(rparent);
	statement();
	opnum *endif = newlabel();
	newcode(j, NULL, NULL, endif);
	if (sy == elsesy) {
		newcode(genlab, NULL, NULL, label1);
		getsymbol();
		statement();
		if (midcodes.back().num[2] == label1) {
			midcodes.pop_back();
			midcodes.pop_back();
			newcode(genlab, NULL, NULL, label1);
		}
		if (ftab[curfunc].funmidcodes.back().num[2] == label1) {
			ftab[curfunc].funmidcodes.pop_back();
			ftab[curfunc].funmidcodes.pop_back();
			newcode(genlab, NULL, NULL, label1);
		}
		else {
			newcode(genlab, NULL, NULL, endif);
		}
	}
	else {
		midcodes.pop_back();
		ftab[curfunc].funmidcodes.pop_back();
		newcode(genlab, NULL, NULL, label1);
	}
}

void dowhilestatement()
{
	int codeloc = 0;
	codeloc = midcodes.size();
	opnum *label = newlabel();
	newcode(genlab, NULL, NULL, label);
	statement();
	test(createset(1, whilesy), createset(dividesyset, 12, plusy, minusy, ident, intcon, charcon, lparent, eql, neq, leq, lss, geq, grt));
	judge(whilesy);
	judge(lparent);
	expression();
	opnum *num1 = numstpop();
	opnum *num2;
	opsymbol op;
	if (sy == eql || sy == neq || sy == leq || sy == lss || sy == geq || sy == grt) {
		op = sy2op.find(sy)->second;
		getsymbol();
		expression();
		num2 = numstpop();
		newcode(op, num1, num2, label);
	}
	else {
		op = bneop;
		num2 = newfac(intty, 0);
		newcode(op, num1, num2, label);
	}
	test(createset(1, rparent), dividesyset);
	judge(rparent);
}

void forstatement()
{
	int loca = 0;
	opnum *label1, *label2, *label3, *label4;
	opsymbol op;
	test(createset(1, lparent), createset(dividesyset, 1, rparent));
	judge(lparent);
	test(createset(1, ident), createset(dividesyset, 1, rparent));
	opnum *num1 = new opnum;
	opnum *num2;
	opnum *num3;
	if (sy == ident) {
		loca = loc(word);
		if (loca == 0) {
			error(8, word);
		}
		else {
			if (tab[loca].obj != varty && tab[loca].obj != globalvarty) {
				error(15, word);
			}
			else if (tab[loca].typ != intty && tab[loca].typ != charty) {
				error(9, "");
			}
			else {
				num1->obj = 1;
				num1->typ = tab[loca].typ;
				num1->value = loca;
			}
		}
		getsymbol();
	}
	test(createset(1, becames), createset(dividesyset, 1, rparent));
	judge(becames);
	expression();
	num2 = numstpop();
	if (num2->obj == 2 && midcodes.back().num[2] == num2) {
		delete(num2);
		midcodes.back().num[2] = num1;
	}
	else {
		newcode(became, num2, NULL, num1);
	}
	test(createset(1, semicolon), createset(dividesyset, 1, rparent));
	judge(semicolon);
	label1 = newlabel();
	newcode(genlab, NULL, NULL, label1);
	expression();
	num1 = numstpop();
	if (sy == eql || sy == neq || sy == leq || sy == lss || sy == geq || sy == grt) {
		op = sy2op.find(sy)->second;
		getsymbol();
		expression();
		num2 = numstpop();
		label2 = newlabel();
		newcode(op, num1, num2, label2);
	}
	else {
		op = bneop;
		num2 = newfac(intty, 0);
		label2 = newlabel();
		newcode(op, num1, num2, label2);
	}
	label3 = newlabel();
	label4 = newlabel();
	newcode(j, num1, num1, label4);
	newcode(genlab, NULL, NULL, label3);
	test(createset(1, semicolon), createset(dividesyset, 1, rparent));
	judge(semicolon);
	test(createset(1, ident), createset(dividesyset, 1, rparent));
	num3 = new opnum;
	if (sy == ident) {
		loca = loc(word);
		if (loca == 0) {
			error(8, word);
		}
		else {
			if (tab[loca].obj != varty && tab[loca].obj != globalvarty) {
				error(15, word);
			}
			else if (tab[loca].typ != intty && tab[loca].typ != charty) {
				error(9, word);
			}
			else {
				num3->obj = 1;
				num3->typ = tab[loca].typ;
				num3->value = loca;
			}
		}
		getsymbol();
	}
	test(createset(1, becames), createset(dividesyset, 1, rparent));
	judge(becames);
	test(createset(1, ident), createset(dividesyset, 1, rparent));
	num1 = new opnum;
	if (sy == ident) {
		loca = loc(word);
		if (loca == 0) {
			error(8, word);
		}
		else {
			if (tab[loca].obj != varty && tab[loca].obj != globalvarty) {
				error(15, word);
			}
			else if (tab[loca].typ != intty && tab[loca].typ != charty) {
				error(9, word);
			}
			else {
				num1->obj = 1;
				num1->typ = tab[loca].typ;
				num1->value = loca;
			}
		}
		getsymbol();
	}
	op = addop;
	if (sy == plusy || sy == minusy) {
		op = sy2op.find(sy)->second;
		getsymbol();
	}
	test(createset(1, intcon), createset(dividesyset, 1, rparent));
	if (sy == intcon) {
		num2 = newfac(intty, hnumber);
		getsymbol();
	}
	else {
		num2 = newfac(intty, 1);
	}
	newcode(op, num1, num2, num3);
	newcode(j, NULL, NULL, label1);
	test(createset(1, rparent), dividesyset);
	judge(rparent);
	test(statebgset, emptyset);
	newcode(genlab, NULL, NULL, label2);
	statement();
	newcode(j, NULL, NULL, label3);
	newcode(genlab, NULL, NULL, label4);
}

void instatement()
{
	int loca = 0;
	test(createset(1, lparent), createset(dividesyset, 3, comma, ident, rparent));
	judge(lparent);
	while (1)
	{
		opnum *num1;
		if (sy == ident) {
			loca = loc(word);
			if (loca == 0) {
				error(8, word);
			}
			else {
				if (tab[loca].obj != varty && tab[loca].obj != globalvarty) {
					error(15, word);
				}
				else if (tab[loca].typ != intty && tab[loca].typ != charty) {
					error(9, word);
				}
				else if (tab[loca].mult > 0) {
					error(16, "");
				}
				else {
					num1 = new opnum;
					num1->obj = 1;
					num1->typ = tab[loca].typ;
					num1->value = loca;
					newcode(readop, NULL, NULL, num1);
				}
			}
			getsymbol();
		}
		if (sy == comma)
		{
			getsymbol();
		}
		//judge(comma)
		else {
			break;
		}
	}
	test(createset(1, rparent), dividesyset);
	judge(rparent);
	test(createset(1, semicolon), dividesyset);
	judge(semicolon);
}

void outstatement()
{
	int stringloc = 0;
	test(createset(1, lparent), createset(dividesyset, 2, stringcon, rparent));
	judge(lparent);
	opnum *num1;
	opnum *lf;
	if (sy == stringcon) {
		stringloc = enterstatic(3, 1, 0, word);
		num1 = new opnum;
		num1->obj = 1;
		num1->typ = stringty;
		num1->value = stringloc;
		newcode(writeop, NULL, NULL, num1);
		getsymbol();
		if (sy == comma) {
			getsymbol();
			expression();
			num1 = numstpop();
			newcode(writeop, NULL, NULL, num1);
		}
	}
	else {
		expression();
		num1 = numstpop();
		newcode(writeop, NULL, NULL, num1);
	}
	lf = new opnum;
	lf->obj = 3;
	lf->typ = charty;
	lf->value = '\n';
	newcode(writeop, NULL, NULL, lf);
	test(createset(1, rparent), dividesyset);
	judge(rparent);
	test(createset(1, semicolon), dividesyset);
	judge(semicolon);
}

void returnstatement()
{
	test(createset(2, lparent, semicolon), createset(dividesyset, 1, rparent));
	if (sy == lparent) {
		if (ftab[curfunc].functype == voidty) {
			error(18,"");
		}
		getsymbol();
		expression();
		opnum *num1 = numstpop();
		newcode(returnop, NULL, NULL, num1);
		test(createset(1, rparent), dividesyset);
		judge(rparent);
		test(createset(1, semicolon), dividesyset);
		judge(semicolon);
	}
	else if (sy == semicolon) {
		if (ftab[curfunc].functype != voidty) {
			error(19,"");
		}
		newcode(returnop, NULL, NULL, newfac(intty, 0));
		getsymbol();
	}
}

void statement()
{
	int loca = 0;
	if (statebgset.find(sy) != statebgset.end()) {
		switch (sy)
		{
		case ifsy:
			getsymbol();
			ifstatement();
			break;
		case dosy:
			getsymbol();
			dowhilestatement();
			break;
		case forsy:
			getsymbol();
			forstatement();
			break;
		case lbrace:
			getsymbol();
			while (statebgset.find(sy) != statebgset.end()) {
				statement();
			}
			if (sy == rbrace) {
				getsymbol();
			}
			break;
		case ident:
			loca = loc(word);
			if (loca == 0) {
				error(8, word);
				getsymbol();
			}
			else {
				if (tab[loca].obj == functionty) {
					getsymbol();
					callfunc(loca);
					test(createset(1, semicolon), dividesyset);
					judge(semicolon);
				}
				else if (tab[loca].obj == varty || tab[loca].obj == globalvarty) {
					opnum *num1 = new opnum;
					opnum *num2;
					opnum *num3;
					getsymbol();
					if (tab[loca].mult > 0) {
						test(createset(1, lbrack), createset(dividesyset, 1, rbrack));
						judge(lbrack);
						expression();
						num1->obj = 1;
						num1->typ = tab[loca].typ;
						num1->value = loca;
						num2 = numstpop();
						test(createset(1, rbrack), dividesyset);
						judge(rbrack);
					}
					else {
						num1->obj = 1;
						num1->typ = tab[loca].typ;
						num1->value = loca;
						num2 = NULL;
					}
					test(createset(1, becames), dividesyset);
					judge(becames);
					expression();
					num3 = numstpop();
					if (num3->typ == intty && tab[loca].typ == charty) {
						cout << "warning: int to char" << endl;
					}
					if (tab[loca].mult > 0) {
						newcode(storeop, num3, num2, num1);
					}
					else {
						if (num3->obj == 2 && midcodes.back().num[2] == num3) {
							delete(num3);
							midcodes.back().num[2] = num1;
							ftab[curfunc].funmidcodes.back().num[2] = num1;
						}
						else {
							newcode(became, num3, num2, num1);
						}
					}
					test(createset(1, semicolon), dividesyset);
					judge(semicolon);
				}
				else {
					error(10, "");
					skip(dividesyset);
				}
			}
			break;
		case scanfsy:
			getsymbol();
			instatement();
			break;
		case printfsy:
			getsymbol();
			outstatement();
			break;
		case returnsy:
			getsymbol();
			returnstatement();
			break;
		case semicolon:
			getsymbol();
			break;
		default:
			break;
		}
	}
}

void function(int ismain)
{
	tempindex = 0;
	partadr = -8;
	type t;
	int funcloc = tab.size() - 1;
	int tabi;
	int parnum = 0;
	opnum *label = newlabel();
	curlv++;
	ftab[tab[funcloc].ref].codestart = midcodes.size();
	tab[funcloc].adr = midcodes.size();
	if (!ismain) {
		ftab[tab[funcloc].ref].label = label;
		newcode(genlab, NULL, NULL, label);
		parnum = parameterlist();
	}
	else {
		ftab[tab[funcloc].ref].codestart--;
		test(createset(1, lparent), dividesyset);
		judge(lparent);
		test(createset(1, rparent), dividesyset);
		judge(rparent);
	}
	test(createset(1, lbrace), createset(statebgset, 2, rbrace, semicolon));
	judge(lbrace);
	if (sy == constsy) {
		constdec();
	}
	while (sy == intsy || sy == charsy) {
		t = sy == intsy ? intty : charty;
		getsymbol();
		while (1) {
			tabi = 0;
			test(createset(1, ident), createset(dividesyset, 1, comma));
			if (sy == ident) {
				tabi = enter(word, varty, t, curlv);
				getsymbol();
			}
			if (sy == lbrack) {
				getsymbol();
				test(createset(1, intcon), createset(dividesyset, 2, comma, rbrack));
				if (sy == intcon) {
					if (hnumber == 0) {
						error(1, "");
						hnumber = 1;
					}
					if (tabi != 0) {
						partadr -= 4 * hnumber;
						tab[tabi].adr = partadr;
						tab[tabi].mult = hnumber;
					}
					getsymbol();
				}
				test(createset(1, rbrack), createset(dividesyset, 1, comma));
				judge(rbrack);
			}
			else {
				if (tabi != 0) {
					partadr -= 4;
					tab[tabi].adr = partadr;
				}
			}
			test(createset(emptyset, 2, comma, semicolon), dividesyset);
			//judge(comma)
			if (sy == comma)
			{
				getsymbol();
			}
			else if (sy == semicolon) {
				getsymbol();
				break;
			}
		}
	}
	for (int it = tab.size() - 1; it > funcloc; it--) {
		if (tab[it].obj == varty) {
			tab[it].adr -= partadr;
		}
	}
	ftab[tab[funcloc].ref].psize = parnum * 4;
	ftab[tab[funcloc].ref].vsize = -partadr;
	test(createset(statebgset, 1, rbrace), emptyset);
	newcode(funcbeginop, NULL, NULL, NULL);
	if (!ismain) {
		test(createset(statebgset, 1, rbrace), createset(3, intsy, charsy, voidsy));
	}
	else {
		test(createset(statebgset, 1, rbrace), emptyset);
	}
	while (sy != rbrace) {
		if (sy == endsy) {
			error(17, "");
			break;
		}
		else if (sy == intsy || sy == charsy || sy == voidsy) {
			break;
		}
		statement();
		if (sy == endsy) {
			error(17, "");
			break;
		}
		if (!ismain) {
			test(createset(statebgset, 1, rbrace), createset(3, intsy, charsy, voidsy));
		}
		else {
			test(createset(statebgset, 1, rbrace), emptyset);
		}
	}
	if (sy == rbrace) {
		getsymbol();
	}
	if (ismain) {
	}
	else {
		newcode(returnop, NULL, NULL, newfac(intty, 0));
	}
	ftab[tab[funcloc].ref].codeend = midcodes.size() - 1;
	curlv--;
}

void program()
{
	partadr = -4;
	curfunc = 0;
	enterfunc(voidty);
	int tabi;
	type t;
	opnum *mainlabel = newlabel();
	newcode(j, NULL, NULL, mainlabel);
	if (sy == constsy) {
		constdec();
	}
	test(createset(3, charsy, intsy, voidsy), emptyset);
	while (sy == charsy || sy == intsy) {
		t = sy == charsy ? charty : intty;
		getsymbol();
		test(createset(1, ident), createset(dividesyset, 1, comma));
		if (sy == ident) {
			tabi = tab.size();
			enter(word, globalvarty, t, 0);
			getsymbol();
		}
		if (sy == lparent) {
			getsymbol();
			tab[tabi].ref = ftab.size();
			tab[tabi].obj = functionty;
			curfunc = ftab.size();
			enterfunc(t);
			function(0);
			curfunc = 0;
			break;
		}
		else if (sy == lbrack || sy == comma || sy == semicolon) {
			if (sy == lbrack) {
				getsymbol();
				test(createset(1, intcon), createset(dividesyset, 1, rbrack));
				if (sy == intcon) {
					if (hnumber == 0) {
						error(1, "");
						hnumber = 1;
					}
					tab[tabi].mult = hnumber;
					if (t == charty) {
						enterstatic(2, hnumber, 0, "");
					}
					else {
						enterstatic(1, hnumber, 0, "");
					}
					getsymbol();
				}
				test(createset(1, rbrack), dividesyset);
				judge(rbrack);
			}
			else {
				if (t == charty) {
					enterstatic(2, 1, 0, "");
				}
				else {
					enterstatic(1, 1, 0, "");
				}
			}
			while (1) {
				test(createset(2, comma, semicolon), createset(dividesyset, 3, intsy, charsy, voidsy));
				if(sy == comma) {
					getsymbol();
				}
				else
				{
					break;
				}
				test(createset(1, ident), dividesyset);
				if (sy == ident) {
					tabi = tab.size();
					enter(word, globalvarty, t, 0);
					getsymbol();
				}
				if (sy == lbrack) {
					getsymbol();
					test(createset(1, intcon), createset(dividesyset, 1, rbrack));
					if (sy == intcon) {
						if (hnumber == 0) {
							error(1, "");
							hnumber = 1;
						}
						tab[tabi].mult = hnumber;
						if (t == charty) {
							enterstatic(2, hnumber, 0, "");
						}
						else {
							enterstatic(1, hnumber, 0, "");
						}
						getsymbol();
					}
					judge(rbrack);
				}
				else {
					if (t == charty) {
						enterstatic(2, 1, 0, "");
					}
					else {
						enterstatic(1, 1, 0, "");
					}
				}
			}
			judge(semicolon);
		}
	}
	test(createset(3, intsy, charsy, voidsy), emptyset);
	while (sy == intsy || sy == charsy || sy == voidsy) {
		if (sy == intsy) {
			t = intty;
		}
		else if (sy == charsy) {
			t = charty;
		}
		else {
			t = voidty;
		}
		getsymbol();
		test(createset(2, ident, mainsy), dividesyset);
		if (sy == ident) {
			tabi = tab.size();
			if (enter(word, varty, t, 0) != 0) {
				getsymbol();
				test(createset(1, lparent), createset(dividesyset, 1, rparent));
				judge(lparent);
				tab[tabi].ref = ftab.size();
				tab[tabi].obj = functionty;
				curfunc = ftab.size();
				enterfunc(t);
				function(0);
				curfunc = 0;
			}
			else {
				skip(createset(3, intsy, charsy, voidsy));
			}
		}
		else if (sy == mainsy) {
			if (t != voidty) {
				error(2, "");
			}
			tabi = tab.size();
			enter(word, varty, t, 0);
			getsymbol();
			tab[tabi].ref = ftab.size();
			tab[tabi].obj = functionty;
			curfunc = ftab.size();
			enterfunc(voidty);
			newcode(genlab, NULL, NULL, mainlabel);
			function(1);
			curfunc = 0;
			break;
		}
	}
}



void counttmp()
{
	for (int i = 1; i < ftab.size(); i++) {
		set<opnum*> tmpset;
		int tmpadr = 0;
		int cbg = ftab[i].codestart;
		int ced = ftab[i].codeend;
		for (int ci = cbg; ci < ced; ci++) {
			code mc = midcodes[ci];
			for (int ni = 0; ni < 3; ni++) {
				if (mc.num[ni] != NULL && mc.num[ni]->obj == 2) {
					tmpset.insert(mc.num[ni]);
				}
			}
		}
		set<opnum*>::iterator si;
		for (si = tmpset.begin(); si != tmpset.end(); si++) {
			opnum *num = *si;
			num->value = tmpadr;
			tmpadr += 4;
		}
		ftab[i].tempsize = tmpset.size() * 4;
		ftab[i].vsize += ftab[i].tempsize;
	}
}

void newfinalcode(mipsop op, int t1, int num1, int t2, int num2, int t3, int num3)
{
	finalcode fc;
	finalopnum fn1;
	finalopnum fn2;
	finalopnum fn3;
	fn1.type = t1;
	fn2.type = t2;
	fn3.type = t3;
	fn1.value = num1;
	fn2.value = num2;
	fn3.value = num3;
	fc.num[0] = fn1;
	fc.num[1] = fn2;
	fc.num[2] = fn3;
	fc.adr = codeadr;
	fc.op = op;
	codeadr += 4;
	finalcodes.insert(finalcodes.end(), fc);
}

void loadopnum(int r, int obj, int value, int off)
{
	record rec;
	switch (obj)
	{
	case 1:
		rec = tab[value];
		if (rec.obj == globalvarty || rec.obj == constty) {
			newfinalcode(mlw, 2, r, 1, rec.adr, 2, 0);
		}
		else if (rec.obj == varty) {
			newfinalcode(mlw, 2, r, 1, rec.adr + off, 2, 29);
		}
		break;
	case 2:
		newfinalcode(mlw, 2, r, 1, value, 2, 29);
		break;
	case 3:
		newfinalcode(maddiu, 2, r, 2, 0, 1, value);
		break;
	default:
		break;
	}
}

void swnum(int r, int obj, int value, int off)
{
	record rec;
	switch (obj)
	{
	case 1:
		rec = tab[value];
		if (rec.obj == globalvarty || rec.obj == constty) {
			newfinalcode(msw, 2, r, 1, rec.adr, 2, 0);
		}
		else if (rec.obj == varty) {
			newfinalcode(msw, 2, r, 1, rec.adr + off, 2, 29);
		}
		break;
	case 2:
		newfinalcode(msw, 2, r, 1, value, 2, 29);
		break;
	default:
		break;
	}
}

void getfinalcodes()
{
	//newfinalcode(mj, 0, 0, 0, 0, 1, midcodes[0].num[2]->value);
	newfinalcode(mj, 0, 0, 0, 0, 1, ftab[0].funmidcodes[0].num[2]->value);
	for (int i = 1; i < ftab.size(); i++) {
		funcrecord fr = ftab[i];
		int parnum = 0;
		int cbg = fr.codestart;
		int ced = fr.codeend;
		int tmpsize = fr.tempsize;
		for (int ci = 0; ci < fr.funmidcodes.size(); ci++)
		{
			code mc = fr.funmidcodes[ci];
		/*for (int ci = cbg; ci <= ced; ci++) {
			code mc = midcodes[ci];*/
			opsymbol op = mc.op;
			opnum *onum;
			midcodes[ci].finaladr = codeadr;
			record rec;
			int vnum;
			switch (op)
			{
			case addop:
			case subop:
			case divop:
			{
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				int type1 = 2, value1 = 3;
				if (mc.num[1]->obj == 3) {
					type1 = 1;
					value1 = mc.num[1]->value;
				}
				else {
					loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				}
				if (op == addop) {
					newfinalcode(madd, 2, 2, 2, 2, type1, value1);
				}
				else if (op == subop) {
					newfinalcode(msub, 2, 2, 2, 2, type1, value1);
				}
				else if (op == divop) {
					newfinalcode(mdiv, 2, 2, 2, 2, type1, value1);
				}
				swnum(2, mc.num[2]->obj, mc.num[2]->value, tmpsize);
				break;
			}
			case multop:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(mmult, 2, 2, 2, 3, 0, 0);
				newfinalcode(mmflo, 2, 2, 0, 0, 0, 0);
				swnum(2, mc.num[2]->obj, mc.num[2]->value, tmpsize);
				break;
			case jal:
				newfinalcode(mjal, 0, 0, 0, 0, 1, mc.num[2]->value);
				break;
			case j:
				newfinalcode(mj, 0, 0, 0, 0, 1, mc.num[2]->value);
				break;
			case became:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				swnum(2, mc.num[2]->obj, mc.num[2]->value, tmpsize);
				break;
			case beqop:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(mbeq, 2, 2, 2, 3, 1, mc.num[2]->value);
				break;
			case bneop:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(mbne, 2, 2, 2, 3, 1, mc.num[2]->value);
				break;
			case bgtop:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(mbgt, 2, 2, 2, 3, 1, mc.num[2]->value);
				break;
			case bgeop:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(mbge, 2, 2, 2, 3, 1, mc.num[2]->value);
				break;
			case bltop:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(mblt, 2, 2, 2, 3, 1, mc.num[2]->value);
				break;
			case bleop:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(mble, 2, 2, 2, 3, 1, mc.num[2]->value);
				break;
			case readop:
				onum = mc.num[2];
				if (onum->typ == intty) {
					newfinalcode(maddiu, 2, 2, 2, 0, 1, 5);
					newfinalcode(msyscall, 0, 0, 0, 0, 0, 0);
					swnum(2, onum->obj, mc.num[2]->value, tmpsize);
				}
				else if (onum->typ == charty) {
					newfinalcode(maddiu, 2, 2, 2, 0, 1, 12);
					newfinalcode(msyscall, 0, 0, 0, 0, 0, 0);
					swnum(2, onum->obj, mc.num[2]->value, tmpsize);
				}
				break;
			case writeop:
				onum = mc.num[2];
				if (onum->typ == intty) {
					newfinalcode(maddiu, 2, 2, 2, 0, 1, 1);
					loadopnum(4, onum->obj, onum->value, tmpsize);
				}
				else if (onum->typ == charty) {
					newfinalcode(maddiu, 2, 2, 2, 0, 1, 11);
					loadopnum(4, onum->obj, onum->value, tmpsize);
				}
				else if (onum->typ == stringty) {
					newfinalcode(maddiu, 2, 2, 2, 0, 1, 4);
					newfinalcode(maddiu, 2, 4, 2, 0, 1, consttab[onum->value].adr);
				}
				newfinalcode(msyscall, 0, 0, 0, 0, 0, 0);
				break;
			case loadop:
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(msll, 2, 3, 2, 3, 1, 2);
				if (mc.num[0]->obj == 1) {
					rec = tab[mc.num[0]->value];
					if (rec.obj == globalvarty || rec.obj == constty) {
						newfinalcode(maddiu, 2, 3, 2, 3, 1, rec.adr);
					}
					else if (rec.obj == varty) {
						newfinalcode(maddiu, 2, 3, 2, 3, 1, rec.adr + tmpsize);
						newfinalcode(madd, 2, 3, 2, 3, 2, 29);
					}
				}
				else if (mc.num[0]->obj == 2) {
					newfinalcode(maddiu, 2, 3, 2, 3, 1, mc.num[0]->value);
					newfinalcode(madd, 2, 3, 2, 3, 2, 29);
				}
				newfinalcode(mlw, 2, 2, 1, 0, 2, 3);
				swnum(2, mc.num[2]->obj, mc.num[2]->value, tmpsize);
				break;
			case storeop:
				loadopnum(2, mc.num[0]->obj, mc.num[0]->value, tmpsize);
				loadopnum(3, mc.num[1]->obj, mc.num[1]->value, tmpsize);
				newfinalcode(msll, 2, 3, 2, 3, 1, 2);
				if (mc.num[2]->obj == 1) {
					rec = tab[mc.num[2]->value];
					if (rec.obj == globalvarty || rec.obj == constty) {
						newfinalcode(maddiu, 2, 3, 2, 3, 1, rec.adr);
					}
					else if (rec.obj == varty) {
						newfinalcode(maddiu, 2, 3, 2, 3, 1, rec.adr + tmpsize);
						newfinalcode(madd, 2, 3, 2, 3, 2, 29);
					}
				}
				else if (mc.num[2]->obj == 2) {
					newfinalcode(maddiu, 2, 3, 2, 3, 1, mc.num[2]->value);
					newfinalcode(madd, 2, 3, 2, 3, 2, 29);
				}
				newfinalcode(msw, 2, 2, 1, 0, 2, 3);
				break;
			case funcbeginop:
				newfinalcode(maddiu, 2, 29, 2, 29, 1, -fr.vsize);
				newfinalcode(msw, 2, 31, 1, fr.vsize - 4, 2, 29);
				break;
			case returnop:
				loadopnum(2, mc.num[2]->obj, mc.num[2]->value, tmpsize);
				newfinalcode(mlw, 2, 31, 1, fr.vsize - 4, 2, 29);
				newfinalcode(maddiu, 2, 29, 2, 29, 1, fr.vsize);
				newfinalcode(mjr, 0, 0, 0, 0, 2, 31);
				break;
			case pushop:
				parnum = mc.num[0]->value;
				loadopnum(2, mc.num[2]->obj, mc.num[2]->value, tmpsize);
				newfinalcode(msw, 2, 2, 1, -parnum * 4 - 8, 2, 29);
				break;
			case getreturnop:
				swnum(2, mc.num[2]->obj, mc.num[2]->value, tmpsize);
				break;
			case genlab:
				newfinalcode(mgenlab, 0, 0, 0, 0, 1, mc.num[2]->value);
				break;
			default:
				break;
			}
		}

	}
}

void printmips()
{
	fprintf(codeout, ".data\n");
	for (int i = 0; i < consttab.size(); i++) {
		int length = 1;
		constrecord cr = consttab[i];
		if (cr.type == 1 || cr.type == 2) {
			if (cr.size > 4) {
				length = cr.size / 4;
				fprintf(codeout, ".word %d : %d\n", 0, length);
			}
			else {
				fprintf(codeout, ".word %d\n", cr.value);
			}
		}
		else if (cr.type == 3) {
			string s = ".asciiz \"" + cr.svalue + "\"\n";
			for (int i = 0; i < s.size(); i++) {
				if (s[i] == '\\') {
					fprintf(codeout, "%c", s[i]);
				}
				fprintf(codeout, "%c", s[i]);
			}
		}
	}
	fprintf(codeout, ".text\n");
	string numstr;
	stringstream ss;
	for (int i = 0; i < finalcodes.size(); i++) {
		finalcode fc = finalcodes[i];
		string codestr = "";
		mipsop op = fc.op;
		if (op == mgenlab) {
			ss.clear();
			ss << fc.num[2].value;
			ss >> codestr;
			codestr = "\nlabel" + codestr + ":\n";
			fprintf(codeout, codestr.c_str());
		}
		else if (op == mlw || op == msw) {
			ss.clear();
			codestr += mipsopstr[op] + " ";
			ss << fc.num[0].value;
			ss >> numstr;
			ss.clear();
			codestr += "$" + numstr + ", ";
			ss << fc.num[1].value;
			ss >> numstr;
			ss.clear();
			codestr += numstr + "(";
			ss << fc.num[2].value;
			ss >> numstr;
			ss.clear();
			codestr += "$" + numstr + ")\n";
			fprintf(codeout, codestr.c_str());
		}
		else if (needlabelset.find(op) != needlabelset.end()) {
			codestr += mipsopstr[op] + " ";
			for (int m = 0; m < 2; m++) {
				ss.clear();
				if (fc.num[m].type == 1) {
					ss << fc.num[m].value;
					ss >> numstr;
					codestr += numstr + ",";
				}
				else if (fc.num[m].type == 2) {
					ss << fc.num[m].value;
					ss >> numstr;
					codestr += "$" + numstr + ",";
				}
				else {
					continue;
				}
			}
			ss.clear();
			ss << fc.num[2].value;
			ss >> numstr;
			codestr += " label" + numstr + "\n";
			fprintf(codeout, codestr.c_str());
			fprintf(codeout, "nop\n");
		}
		else {
			codestr += mipsopstr[op] + " ";
			for (int m = 0; m < 3; m++) {
				ss.clear();
				if (fc.num[m].type == 1) {
					ss << fc.num[m].value;
					ss >> numstr;
					codestr += numstr;
				}
				else if (fc.num[m].type == 2) {
					ss << fc.num[m].value;
					ss >> numstr;
					codestr += "$" + numstr;
				}
				else {
					continue;
				}
				if (m != 2) {
					codestr += ", ";
				}
			}
			codestr += "\n";
			fprintf(codeout, codestr.c_str());
			if (op == mjr) {
				fprintf(codeout, "nop\n");
			}
		}
	}
}

void printmidcode()
{
	string onecode;
	code mco;
	stringstream ss;
	string numstr;
	for (int i = 0; i < midcodes.size(); i++)
	{
		mco = midcodes[i];
		onecode = "";
		if (mco.op == genlab) {
			onecode += "\n";
		}
		onecode += opsymbolstr[mco.op] + " ";
		for (int j = 0; j < 3; j++)
		{
			opnum* pnum = mco.num[j];
			if (pnum != NULL)
			{
				switch (pnum->obj)
				{
				case 1:
					if (pnum->typ == stringty) {
						onecode += consttab[pnum->value].svalue;
					}
					else {
						onecode += tab[pnum->value].name;
					}
					break;
				case 2:
					ss.clear();
					ss << pnum->value;
					ss >> numstr;
					onecode += "t" + numstr;
					break;
				case 3:
					ss.clear();
					ss << pnum->value;
					ss >> numstr;
					onecode += numstr;
					break;
				case 4:
					ss.clear();
					ss << pnum->value;
					ss >> numstr;
					onecode += "label" + numstr;
					break;
				default:
					break;
				}
				onecode += j == 2 ? "" : ", ";
			}
			onecode += j == 2 ? "\n" : "";
		}
		fprintf(midcodeout, onecode.c_str());
	}
	for (int i = 0; i < ftab.size(); i++) {
		for (int j = 0; j < ftab[i].funmidcodes.size(); j++)
		{
			mco = ftab[i].funmidcodes[j];
			onecode = "";
			if (mco.op == genlab) {
				onecode += "\n";
			}
			onecode += opsymbolstr[mco.op] + " ";
			for (int j = 0; j < 3; j++)
			{
				opnum* pnum = mco.num[j];
				if (pnum != NULL)
				{
					switch (pnum->obj)
					{
					case 1:
						if (pnum->typ == stringty) {
							onecode += consttab[pnum->value].svalue;
						}
						else {
							onecode += tab[pnum->value].name;
						}
						break;
					case 2:
						ss.clear();
						ss << pnum->value;
						ss >> numstr;
						onecode += "t" + numstr;
						break;
					case 3:
						ss.clear();
						ss << pnum->value;
						ss >> numstr;
						onecode += numstr;
						break;
					case 4:
						ss.clear();
						ss << pnum->value;
						ss >> numstr;
						onecode += "label" + numstr;
						break;
					default:
						break;
					}
					onecode += j == 2 ? "" : ", ";
				}
				onecode += j == 2 ? "\n" : "";
			}
			cout << onecode;
		}
	}
}

int main()
{
	//char path[100];
	//char mipscode[100];
	char path[100] = "F:\\t.txt";
	char midcodepath[100] = "f:\\midcode.txt";
	char mipscode[100] = "c:\\users\\woi\\desktop\\mars\\mips2.asm";
	/*cout << "Src..." << endl;
	cin >> path;
	cout << "Code File..." << endl;
	cin >> mipscode;*/
	initmap();
	initset();
	if ((in = fopen(path, "r")) == NULL) {
		cout << "Wrong src path" << endl;
		return 0;
	}
	if ((midcodeout = fopen(midcodepath, "w")) == NULL) {
		cout << "Wrong midcode path" << endl;
		return 0;
	}
	if ((codeout = fopen(mipscode, "w")) == NULL) {
		cout << "Wrong result path" << endl;
		return 0;
	}
	getsymbol();
	program();
	cout << errorcount << " errors" << endl;
	if (errorcount == 0) {
		printmidcode();
		counttmp();
		getfinalcodes();
		printmips();
	}
	fclose(in);
	fclose(codeout);
}
