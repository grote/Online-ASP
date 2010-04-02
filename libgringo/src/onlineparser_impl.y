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

// literals
#include <gringo/predicateliteral.h>
#include <gringo/conditionalliteral.h>

// terms
#include <gringo/term.h>
#include <gringo/constant.h>
#include <gringo/functionterm.h>
#include <gringo/funcsymbolterm.h>

// statements
#include <gringo/normalrule.h>

// misc
#include <gringo/grounder.h>

using namespace gringo;

#define OUTPUT (pParser->getGrounder()->getOutput())
#define GROUNDER (pParser->getGrounder())
#define STRING(x) (pParser->getGrounder()->createString(x))
#define DELETE_PTR(X) { if(X) delete (X); }
#define DELETE_PTRVECTOR(T, X) { if(X){ for(T::iterator it = (X)->begin(); it != (X)->end(); it++) delete (*it); delete (X); } }
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

// token types/destructors for nonterminals
%type fact     { Statement* }
%destructor rule     { DELETE_PTR($$) }

%type predicate { PredicateLiteral* }
%destructor predicate { DELETE_PTR($$) }

%type head_atom          { Literal* }
%destructor head_atom    { DELETE_PTR($$) }

%type const_termlist { TermVector* }
%destructor const_termlist { DELETE_PTRVECTOR(TermVector, $$) }

%type term       { Term* }
%type const_term { Term* }
%destructor term       { DELETE_PTR($$) }
%destructor const_term { DELETE_PTR($$) }

%left DOTS.
%left OR.
%left XOR.
%left AND.
%left PLUS MINUS.
%left TIMES DIVIDE MOD.
%right POWER.
%left UMINUS TILDE.

// this will define the symbols in the header 
// even though they are not used in the rules
%nonassoc ERROR EOI OR2.

%start_symbol start

start ::= program.

program ::= program fact(fact) DOT.            { if(fact) pParser->getGrounder()->addStatement(fact); }
program ::= program ENDSTEP DOT.
program ::= program STOP DOT.
program ::= .

fact(res) ::= head_atom(head) IF .           { res = new NormalRule(head, 0); }
fact(res) ::= head_atom(head).               { res = new NormalRule(head, 0); }

head_atom(res) ::= predicate(pred). { res = new ConditionalLiteral(pred, 0); }

predicate(res) ::= IDENTIFIER(id) LPARA const_termlist(list) RPARA.       { res = new PredicateLiteral(GROUNDER, STRING(id), list); }
predicate(res) ::= IDENTIFIER(id).                                        { res = new PredicateLiteral(GROUNDER, STRING(id), new TermVector()); }
predicate(res) ::= MINUS IDENTIFIER(id) LPARA const_termlist(list) RPARA. { id->insert(id->begin(), '-'); res = new PredicateLiteral(GROUNDER, STRING(id), list); }
predicate(res) ::= MINUS IDENTIFIER(id).                                  { id->insert(id->begin(), '-'); res = new PredicateLiteral(GROUNDER, STRING(id), new TermVector()); }

const_termlist(res) ::= const_termlist(list) COMMA const_term(term). { res = list; res->push_back(term); }
const_termlist(res) ::= const_term(term).                            { res = new TermVector(); res->push_back(term); }

const_term(res) ::= IDENTIFIER(x). { if(false) res = new Constant(Value(Value::STRING, STRING(x))); }
const_term(res) ::= STRING(x).     { if(false) res = new Constant(Value(Value::STRING, STRING(x))); }
const_term(res) ::= NUMBER(x).     { res = new Constant(Value(Value::INT, atol(x->c_str()))); DELETE_PTR(x); }
const_term(res) ::= LPARA const_term(a) RPARA.           { res = a; }
const_term(res) ::= const_term(a) MOD const_term(b).     { res = new FunctionTerm(FunctionTerm::MOD, a, b); }
const_term(res) ::= const_term(a) PLUS const_term(b).    { res = new FunctionTerm(FunctionTerm::PLUS, a, b); }
const_term(res) ::= const_term(a) TIMES const_term(b).   { res = new FunctionTerm(FunctionTerm::TIMES, a, b); }
const_term(res) ::= const_term(a) POWER const_term(b).   { res = new FunctionTerm(FunctionTerm::POWER, a, b); }
const_term(res) ::= const_term(a) MINUS const_term(b).   { res = new FunctionTerm(FunctionTerm::MINUS, a, b); }
const_term(res) ::= const_term(a) DIVIDE const_term(b).  { res = new FunctionTerm(FunctionTerm::DIVIDE, a, b); }
const_term(res) ::= MINUS const_term(b). [UMINUS]        { res = new FunctionTerm(FunctionTerm::MINUS, new Constant(Value(Value::INT, 0)), b); }
const_term(res) ::= ABS LPARA const_term(a) RPARA.       { res = new FunctionTerm(FunctionTerm::ABS, a); }
const_term(res) ::= IDENTIFIER(id) LPARA const_termlist(list) RPARA. { if(false) res = new FuncSymbolTerm(GROUNDER, STRING(id), list); }
