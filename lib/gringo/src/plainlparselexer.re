#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>

#include "plainlparselexer.h"
#include "lparseconverter.h"
#include "lparseconverter_impl.h"

#define YYCTYPE         char
#define YYCURSOR        cursor
#define YYLIMIT         limit
#define YYMARKER        marker
#define YYFILL(n)       {fill(n);}

#define EOI -1
#define ERR 0

using namespace NS_GRINGO;

PlainLparseLexer::PlainLparseLexer() : GrinGoLexer(0)
{
}
	
int PlainLparseLexer::lex(std::string *&lval)
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
		IDENTIFIER      = [a-z_] [a-zA-Z0-9_]*;
		STRING          = "\"" [^"\n]* "\"";
		VARIABLE        = [A-Z] [a-zA-Z0-9_]*;
		COMMENT         = "%" [^\n]* "\n";
		SHOW            = "#"? 'show';
		HIDE            = "#"? 'hide';
		MINIMIZE        = "#"? 'minimize';
		MAXIMIZE        = "#"? 'maximize';
		COMPUTE         = "#"? 'compute';

		SHOW            { return LPARSECONVERTER_SHOW; }
		HIDE            { return LPARSECONVERTER_HIDE; }
		COMMENT         { if(eof == cursor) return LPARSECONVERTER_EOI; step(); goto begin; }
		WS              { goto begin; }
		NL              { if(eof == cursor) return LPARSECONVERTER_EOI; step(); goto begin; }
		IF              { return LPARSECONVERTER_IF; }
		NOT             { return LPARSECONVERTER_NOT; }
		MINIMIZE        { return LPARSECONVERTER_MINIMIZE; }
		MAXIMIZE        { return LPARSECONVERTER_MAXIMIZE; }
		COMPUTE         { return LPARSECONVERTER_COMPUTE; }
		DIGIT1 DIGIT*   { lval = new std::string(start, cursor); return LPARSECONVERTER_NUMBER; }
		"0"             { lval = new std::string(start, cursor); return LPARSECONVERTER_NUMBER; }
		"sum"           { return LPARSECONVERTER_SUM; }
		"max"           { return LPARSECONVERTER_MIN; }
		"min"           { return LPARSECONVERTER_MAX; }
		"count"         { return LPARSECONVERTER_COUNT; }
		STRING          { lval = new std::string(start, cursor); return LPARSECONVERTER_STRING; }
		IDENTIFIER      { lval = new std::string(start, cursor); return LPARSECONVERTER_IDENTIFIER; }
		VARIABLE        { lval = new std::string(start, cursor); return LPARSECONVERTER_VARIABLE; }
		"|"             { return LPARSECONVERTER_DISJUNCTION; }
		"("             { return LPARSECONVERTER_LPARA; }
		")"             { return LPARSECONVERTER_RPARA; }
		"{"             { return LPARSECONVERTER_LBRAC; }
		"}"             { return LPARSECONVERTER_RBRAC; }
		"["             { return LPARSECONVERTER_LSBRAC; }
		"]"             { return LPARSECONVERTER_RSBRAC; }
		"."             { return LPARSECONVERTER_DOT; }
		"/"             { return LPARSECONVERTER_SLASH; }
		","             { return LPARSECONVERTER_COMMA; }
		"-"             { return LPARSECONVERTER_MINUS; }
		ANY             { return LPARSECONVERTER_ERROR; }
	*/
}

PlainLparseLexer::~PlainLparseLexer()
{
}

