// Copyright (c) 2010, Torsten Grote
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

%include {

#include <gringo/gringo.h>

#include <gringo/onlineparser.h>

// terms
#include <gringo/term.h>
#include <gringo/value.h>
#include <gringo/funcsymbol.h>

// misc
#include <gringo/output.h>
#include <gringo/grounder.h>

using namespace gringo;
using namespace NS_OUTPUT;

#define STRING(x) (pParser->getGrounder()->createString(x))
#define PRED(x,a) (pParser->getGrounder()->createPred(x,a))
#define FUNCSYM(x) (pParser->getGrounder()->createFuncSymbol(x))
#define DELETE_PTR(X) { if(X) delete (X); }
}  

%name onlineparser
%stack_size 0
%token_prefix ONLINEPARSER_

%extra_argument { OnlineParser *pParser }

%parse_failure {
	pParser->handleError();
}

%syntax_error {
	pParser->handleError();
}

// token type/destructor for terminals
%token_type { std::string* }
%token_destructor { DELETE_PTR($$) }

%type head_atom		{ Object* }
%type predicate		{ Object* }
%type constant_list	{ ValueVector* }
%type constant		{ Value* }
%type number		{ int }

%destructor head_atom		{ DELETE_PTR($$) }
%destructor predicate		{ DELETE_PTR($$) }
%destructor constant_list	{ DELETE_PTR($$) }
%destructor constant		{ DELETE_PTR($$) }
%destructor number			{ }

%left MINUS.

// this will define the symbols in the header 
// even though they are not used in the rules
%nonassoc ERROR EOI.

%start_symbol start

start ::= program.

program ::= program fact DOT.
program ::= program ENDSTEP DOT.
program ::= program STOP DOT.
program ::= .

fact ::= head_atom(head) IF .	{ Fact r(head); pParser->addExternal(&r); }
fact ::= head_atom(head).		{ Fact r(head); pParser->addExternal(&r); }

head_atom(res) ::= predicate(pred). { res = pred; }

predicate(res) ::= IDENTIFIER(id) LPARA constant_list(list) RPARA.		{ res = new Atom(false, PRED(STRING(id), list->size()), *list); DELETE_PTR(list); }
predicate(res) ::= IDENTIFIER(id).										{ res = new Atom(false, PRED(STRING(id), 0)); }
predicate(res) ::= MINUS IDENTIFIER(id) LPARA constant_list(list) RPARA.{ id->insert(id->begin(), '-'); res = new Atom(false, PRED(STRING(id), list->size()), *list); DELETE_PTR(list); }
predicate(res) ::= MINUS IDENTIFIER(id).								{ id->insert(id->begin(), '-'); res = new Atom(false, PRED(STRING(id), 0)); }

constant_list(res) ::= constant_list(list) COMMA constant(val). { res = list; res->push_back(*val); DELETE_PTR(val); }
constant_list(res) ::= constant(val).                           { res = new ValueVector(); res->push_back(*val); DELETE_PTR(val); }

constant(res) ::= IDENTIFIER(id). { res = new Value(Value::STRING, STRING(id)); }
constant(res) ::= number(n).      { res = new Value(Value::INT, n); }
constant(res) ::= STRING(id).     { res = new Value(Value::STRING, STRING(id)); }
constant(res) ::= IDENTIFIER(id) LPARA constant_list(list) RPARA. { res = new Value(Value::FUNCSYMBOL, FUNCSYM(new FuncSymbol(STRING(id), *list))); DELETE_PTR(list); }

number(res) ::= NUMBER(n).       { res = atol(n->c_str()); DELETE_PTR(n); }
number(res) ::= MINUS NUMBER(n). { res = -atol(n->c_str()); DELETE_PTR(n); }
