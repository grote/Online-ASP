#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>

#include "lparselexer.h"
#include "lparseparser.h"
#include "lparseparser_impl.h"

#define YYCTYPE         char
#define YYCURSOR        cursor
#define YYLIMIT         limit
#define YYMARKER        marker
#define YYFILL(n)       {fill(n);}

#define EOI -1
#define ERR 0

using namespace NS_GRINGO;

LparseLexer::LparseLexer(std::istream *in) : GrinGoLexer(in)
{
}
	
int LparseLexer::lex(std::string *&lval)
{
	lval = 0;
begin:
	start = cursor;
	/*!re2c
		ANY             = [\000-\377];
		WS              = [\t\r ]*;
		NL              = "\n";
		IF              = ":-";
		NOT             = 'not';
		DIGIT           = [0-9];
		DIGIT1          = [1-9];
		IDENTIFIER      = "-"? [a-z_] [a-zA-Z0-9_]*;
		STRING          = "\"" [^"\n]* "\"";
		VARIABLE        = [A-Z] [a-zA-Z0-9_]*;
		COMMENT         = "%" [^\n]* "\n";
		SHOW            = "#"? 'show';
		HIDE            = "#"? 'hide';
		CONST           = "#"? 'const';
		DOMAIN          = "#"? 'domain';
		MINIMIZE        = "#"? 'minimize';
		MAXIMIZE        = "#"? 'maximize';
		COMPUTE         = "#"? 'compute';

		CONST           { return LPARSEPARSER_CONST; }
		SHOW            { return LPARSEPARSER_SHOW; }
		HIDE            { return LPARSEPARSER_HIDE; }
		COMMENT         { if(eof == cursor) return LPARSEPARSER_EOI; step(); goto begin; }
		WS              { goto begin; }
		NL              { if(eof == cursor) return LPARSEPARSER_EOI; step(); goto begin; }
		IF              { return LPARSEPARSER_IF; }
		NOT             { return LPARSEPARSER_NOT; }
		DOMAIN          { return LPARSEPARSER_DOMAIN; }
		MINIMIZE        { return LPARSEPARSER_MINIMIZE; }
		MAXIMIZE        { return LPARSEPARSER_MAXIMIZE; }
		COMPUTE         { return LPARSEPARSER_COMPUTE; }
		DIGIT1 DIGIT*   { lval = new std::string(start, cursor); return LPARSEPARSER_NUMBER; }
		"0"             { lval = new std::string(start, cursor); return LPARSEPARSER_NUMBER; }
		"sum"           { return LPARSEPARSER_SUM; }
		"abs"           { return LPARSEPARSER_ABS; }
		"times"         { return LPARSEPARSER_TIMES; }
		"max"           { return LPARSEPARSER_MIN; }
		"min"           { return LPARSEPARSER_MAX; }
		"count"         { return LPARSEPARSER_COUNT; }
		STRING          { lval = new std::string(start, cursor); return LPARSEPARSER_STRING; }
		IDENTIFIER      { lval = new std::string(start, cursor); return LPARSEPARSER_IDENTIFIER; }
		VARIABLE        { lval = new std::string(start, cursor); return LPARSEPARSER_VARIABLE; }
		"("             { return LPARSEPARSER_LPARA; }
		")"             { return LPARSEPARSER_RPARA; }
		"{"             { return LPARSEPARSER_LBRAC; }
		"}"             { return LPARSEPARSER_RBRAC; }
		"["             { return LPARSEPARSER_LSBRAC; }
		"]"             { return LPARSEPARSER_RSBRAC; }
		"."             { return LPARSEPARSER_DOT; }
		".."            { return LPARSEPARSER_DOTS; }
		";"             { return LPARSEPARSER_SEMI; }
		":"             { return LPARSEPARSER_DDOT; }
		","             { return LPARSEPARSER_COMMA; }
		"+"             { return LPARSEPARSER_PLUS; }
		"-"             { return LPARSEPARSER_MINUS; }
		"*"             { return LPARSEPARSER_TIMES; }
		"/"             { return LPARSEPARSER_DIVIDE; }
		"="             { return LPARSEPARSER_ASSIGN; }
		"=="            { return LPARSEPARSER_EQ; }
		"!="            { return LPARSEPARSER_NE; }
		">"             { return LPARSEPARSER_GT; }
		"<"             { return LPARSEPARSER_LT; }
		">="            { return LPARSEPARSER_GE; }
		"<="            { return LPARSEPARSER_LE; }
		ANY             { return LPARSEPARSER_ERROR; }
	*/
}

LparseLexer::~LparseLexer()
{
}

