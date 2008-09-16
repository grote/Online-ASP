// Copyright (c) 2008, Roland Kaminski
//
// This file is part of GrinGo.
//
// GrinGo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrinGo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrinGo.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>

#include "lparselexer.h"
#include "lparseparser.h"
#include "gringoexception.h"
#include "lparseparser_impl.h"

#define YYCTYPE         char
#define YYCURSOR        cursor
#define YYLIMIT         limit
#define YYMARKER        marker
#define YYFILL(n)       {fill(n);}

#define EOI -1
#define ERR 0

using namespace NS_GRINGO;

LparseLexer::LparseLexer() : GrinGoLexer(0)
{
}

int LparseLexer::lex(std::string *&lval)
{
	int nested = 0;
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
		BEGIN_COMMENT   = "%*";
		END_COMMENT     = "*%";
		COMMENT         = "%";
		SHOW            = "#"? 'show';
		HIDE            = "#"? 'hide';
		CONST           = "#"? 'const';
		DOMAIN          = "#"? 'domain';
		MINIMIZE        = "#"? 'minimize';
		MAXIMIZE        = "#"? 'maximize';
		COMPUTE         = "#"? 'compute';
		LAMBDA          = "#cumulative";
		BASE            = "#base";
		DELTA           = "#volatile";
		
		BEGIN_COMMENT   { nested++; goto block_comment; }
		COMMENT         { goto comment; }
		CONST           { return LPARSEPARSER_CONST; }
		SHOW            { return LPARSEPARSER_SHOW; }
		HIDE            { return LPARSEPARSER_HIDE; }
		WS              { goto begin; }
		NL              { if(eof == cursor) return LPARSEPARSER_EOI; step(); goto begin; }
		IF              { return LPARSEPARSER_IF; }
		NOT             { return LPARSEPARSER_NOT; }
		DOMAIN          { return LPARSEPARSER_DOMAIN; }
		MINIMIZE        { return LPARSEPARSER_MINIMIZE; }
		MAXIMIZE        { return LPARSEPARSER_MAXIMIZE; }
		COMPUTE         { return LPARSEPARSER_COMPUTE; }
		LAMBDA          { return LPARSEPARSER_LAMBDA; }
		BASE            { return LPARSEPARSER_BASE; }
		DELTA           { return LPARSEPARSER_DELTA; }
		DIGIT1 DIGIT*   { lval = new std::string(start, cursor); return LPARSEPARSER_NUMBER; }
		"0"             { lval = new std::string(start, cursor); return LPARSEPARSER_NUMBER; }
		"sum"           { return LPARSEPARSER_SUM; }
		"abs"           { return LPARSEPARSER_ABS; }
		"min"           { return LPARSEPARSER_MIN; }
		"max"           { return LPARSEPARSER_MAX; }
		"count"         { return LPARSEPARSER_COUNT; }
		"div"           { return LPARSEPARSER_DIVIDE; }
		"mod"           { return LPARSEPARSER_MOD; }
		STRING          { lval = new std::string(start, cursor); return LPARSEPARSER_STRING; }
		IDENTIFIER      { lval = new std::string(start, cursor); return LPARSEPARSER_IDENTIFIER; }
		VARIABLE        { lval = new std::string(start, cursor); return LPARSEPARSER_VARIABLE; }
		"|"             { return LPARSEPARSER_DISJUNCTION; }
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
		"~"             { return LPARSEPARSER_TILDE; }
		"?"             { return LPARSEPARSER_OR; }
		"&"             { return LPARSEPARSER_AND; }
		"^"             { return LPARSEPARSER_XOR; }
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
comment:
	/*!re2c
		NL              { if(eof == cursor) return LPARSEPARSER_EOI; step(); goto begin; }
        	ANY             { goto comment; }

	*/
block_comment:
	/*!re2c
		BEGIN_COMMENT   { nested++; goto block_comment; }
		END_COMMENT     { if(--nested == 0) goto begin; else goto block_comment; }
		NL              { if(eof == cursor) throw GrinGoException("error: unclosed block comment"); step(); goto block_comment; }
        	ANY             { goto block_comment; }
	*/
}

LparseLexer::~LparseLexer()
{
}

