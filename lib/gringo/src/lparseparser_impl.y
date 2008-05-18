%include {   

#include "gringo.h"

#include "lparseparser.h"

// literals
#include "literal.h"
#include "aggregateliteral.h"  
#include "predicateliteral.h"
#include "conditionalliteral.h"
#include "relationliteral.h"
#include "assignmentliteral.h"

// terms
#include "term.h"
#include "constant.h"
#include "functionterm.h"
#include "rangeterm.h"
#include "multipleargsterm.h"

// statements
#include "normalrule.h"
#include "weightedstatement.h"

// misc
#include "grounder.h"

using namespace NS_GRINGO;

#define DELETE_PTR(X) { if(X) delete (X); }
#define DELETE_PTRVECTOR(T, X) { if(X){ for(T::iterator it = (X)->begin(); it != (X)->end(); it++) delete (*it); delete (X); } }

}  

%name lparseparser
%token_prefix LPARSEPARSER_

%extra_argument { LparseParser *pParser }

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
%type rule     { Statement* }
%type minimize { Statement* }
%type maximize { Statement* }
%type compute  { Statement* }
%destructor rule     { DELETE_PTR($$) }
%destructor minimze  { DELETE_PTR($$) }
%destructor maximize { DELETE_PTR($$) }
%destructor compute  { DELETE_PTR($$) }

%type body  { LiteralVector* }
%destructor body { DELETE_PTR($$) }

%type conditional_list { LiteralVector* }
%destructor conditional_list { DELETE_PTRVECTOR(LiteralVector, $$) }

%type weight_term        { ConditionalLiteral* }
%type constr_term        { ConditionalLiteral* }
%type constraint_literal { ConditionalLiteral* }
%type constraint_atom    { ConditionalLiteral* }
%destructor constraint_literal { DELETE_PTR($$) }
%destructor constraint_atom    { DELETE_PTR($$) }
%destructor weight_term        { DELETE_PTR($$) }
%destructor constr_term        { DELETE_PTR($$) }

%type predicate          { PredicateLiteral* }
%destructor predicate          { DELETE_PTR($$) }

%type body_atom          { Literal* }
%type head_atom          { Literal* }
%type body_literal       { Literal* }
%type relation_literal   { Literal* }
%type conditional        { Literal* }

%destructor body_atom        { DELETE_PTR($$) }
%destructor head_atom        { DELETE_PTR($$) }
%destructor body_literal     { DELETE_PTR($$) }
%destructor relation_literal { DELETE_PTR($$) }
%destructor conditional      { DELETE_PTR($$) }


%type aggregate      { AggregateLiteral* }
%type aggregate_atom { AggregateLiteral* }
%destructor aggregate      { DELETE_PTR($$) }
%destructor aggregate_atom { DELETE_PTR($$) }

%type termlist  { TermVector* }
%type ntermlist { TermVector* }
%destructor termlist  { DELETE_PTRVECTOR(TermVector, $$) }
%destructor ntermlist { DELETE_PTRVECTOR(TermVector, $$) }

%type term       { Term* }
%type range_term { Term* }
%destructor term       { DELETE_PTR($$) }
%destructor range_term { DELETE_PTR($$) }

%type arg_list { Term* }
%destructor arg_list { DELETE_PTR($$) }

%type weight_list  { ConditionalLiteralVector* }
%type nweight_list { ConditionalLiteralVector* }
%type constr_list  { ConditionalLiteralVector* }
%type nconstr_list { ConditionalLiteralVector* }
%destructor weight_list  { DELETE_PTRVECTOR(ConditionalLiteralVector, $$) }
%destructor nweight_list { DELETE_PTRVECTOR(ConditionalLiteralVector, $$) }
%destructor constr_list  { DELETE_PTRVECTOR(ConditionalLiteralVector, $$) }
%destructor nconstr_list { DELETE_PTRVECTOR(ConditionalLiteralVector, $$) }

%type variable_list { StringVector* }
%destructor variable_list { DELETE_PTR($$) }

%type const_term { Value* }
%type constant   { Value* }
%destructor const_term { DELETE_PTR($$) }
%destructor constant   { DELETE_PTR($$) }

%left PLUS MINUS.
%left TIMES DIVIDE MOD.

// this will define the symbols in the header 
// even though they are not used in the rules
%nonassoc ERROR EOI.

%start_symbol start

start ::= program.

program ::= program rule(rule) DOT. { if(rule) pParser->getGrounder()->addStatement(rule); }
program ::= program SHOW show_list DOT.
program ::= program HIDE hide_list DOT.
program ::= program DOMAIN domain_list DOT.
program ::= program CONST IDENTIFIER(id) ASSIGN const_term(term) DOT. { pParser->getGrounder()->setConstValue(*id, term); DELETE_PTR(id); }
program ::= .

show_list ::= show_list COMMA show_predicate.
show_list ::= show_predicate.

hide_list ::= .           { pParser->getGrounder()->hideAll(); }
hide_list ::= nhide_list.

nhide_list ::= nhide_list COMMA hide_predicate.
nhide_list ::= hide_predicate.

domain_list ::= domain_list COMMA domain_predicate.
domain_list ::= domain_predicate.

show_predicate ::= IDENTIFIER(id) LPARA variable_list(list) RPARA.   { pParser->getGrounder()->setVisible(id, list ? list->size() : 0, true); DELETE_PTR(list); }
hide_predicate ::= IDENTIFIER(id) LPARA variable_list(list) RPARA.   { pParser->getGrounder()->setVisible(id, list ? list->size() : 0, false); DELETE_PTR(list); }
domain_predicate ::= IDENTIFIER(id) LPARA variable_list(list) RPARA. { pParser->getGrounder()->addDomains(id, list); }

variable_list(res) ::= variable_list(list) COMMA VARIABLE(var). { res = list; res->push_back(*var); delete var; }
variable_list(res) ::= VARIABLE(var).                           { res = new StringVector(); res->push_back(*var); delete var; }

rule(res) ::= head_atom(head) IF body(body). { res = new NormalRule(head, body); }
rule(res) ::= head_atom(head).               { res = new NormalRule(head, 0); }
rule(res) ::= IF body(body).                 { res = new NormalRule(0, body); }
rule(res) ::= maximize(min).                 { res = min; }
rule(res) ::= minimize(max).                 { res = max; }
rule(res) ::= compute(comp).                 { res = comp; }

body(res) ::= body(body) COMMA body_literal(literal). { res = body; res->push_back(literal);  }
body(res) ::= body_literal(literal).                  { res = new LiteralVector(); res->push_back(literal); }

conditional(res) ::= predicate(pred).       { res = pred; }
conditional(res) ::= relation_literal(rel). { res = rel; }

conditional_list(res) ::= conditional_list(list) DDOT conditional(cond). { res = list ? list : new LiteralVector(); res->push_back(cond); }
conditional_list(res) ::= .                                              { res = 0; }

body_literal(res) ::= body_atom(atom).     { res = atom; }
body_literal(res) ::= NOT body_atom(atom). { res = atom; res->setNeg(true); }
body_literal(res) ::= relation_literal(rel). { res = rel; }

constraint_literal(res) ::= constraint_atom(atom).     { res = atom; }
constraint_literal(res) ::= NOT constraint_atom(atom). { res = atom; res->setNeg(true); }

body_atom(res) ::= predicate(pred).      { res = pred; }
body_atom(res) ::= aggregate_atom(atom). { res = atom; }

// assignment literals are not realy relation literals but they rae used like them
relation_literal(res) ::= VARIABLE(a) ASSIGN term(b). { res = new AssignmentLiteral(new Constant(Constant::VAR, pParser->getGrounder(), a), b); }
relation_literal(res) ::= term(a) EQ term(b).         { res = new RelationLiteral(RelationLiteral::EQ, a, b); }
relation_literal(res) ::= term(a) NE term(b).         { res = new RelationLiteral(RelationLiteral::NE, a, b); }
relation_literal(res) ::= term(a) GT term(b).         { res = new RelationLiteral(RelationLiteral::GT, a, b); }
relation_literal(res) ::= term(a) GE term(b).         { res = new RelationLiteral(RelationLiteral::GE, a, b); }
relation_literal(res) ::= term(a) LT term(b).         { res = new RelationLiteral(RelationLiteral::LT, a, b); }
relation_literal(res) ::= term(a) LE term(b).         { res = new RelationLiteral(RelationLiteral::LE, a, b); }

head_atom(res) ::= predicate(pred).      { res = pred; }
head_atom(res) ::= aggregate_atom(atom). { res = atom; }

constraint_atom(res) ::= predicate(pred) conditional_list(list). { res = new ConditionalLiteral(pred, list); }

predicate(res) ::= IDENTIFIER(id) LPARA termlist(list) RPARA. { res = new PredicateLiteral(id, list); }
predicate(res) ::= IDENTIFIER(id).                            { res = new PredicateLiteral(id, new TermVector()); }

aggregate_atom(res) ::= term(l) aggregate(aggr) term(u). { res = aggr; aggr->setBounds(l, u); }
aggregate_atom(res) ::= aggregate(aggr) term(u).         { res = aggr; aggr->setBounds(0, u); }
aggregate_atom(res) ::= term(l) aggregate(aggr).         { res = aggr; aggr->setBounds(l, 0); }
aggregate_atom(res) ::= aggregate(aggr).                 { res = aggr; aggr->setBounds(0, 0); }

termlist(res) ::= ntermlist(list). { res = list; }
termlist(res) ::= .                { res = new TermVector(); }
ntermlist(res) ::= termlist(list) COMMA arg_list(term). { res = list; res->push_back(term); }
ntermlist(res) ::= arg_list(term).                      { res = new TermVector(); res->push_back(term); }

arg_list(res) ::= arg_list(list) SEMI range_term(term). { res = MultipleArgsTerm::create(list, term); }
arg_list(res) ::= range_term(term).                     { res = term; }

// the problem here is that with a lookahead of one the first argument of a range_term cant be a more complex structure
// cause in this case it cant be distinuished from a non constant term (at least the cant do)
range_term(res) ::= constant(l) DOTS const_term(u). { res = new RangeTerm(*l, *u); delete l; delete u; }
range_term(res) ::= term(term).                     { res = term; }

term(res) ::= VARIABLE(x).   { res = new Constant(Constant::VAR, pParser->getGrounder(), x); }
term(res) ::= IDENTIFIER(x). { res = new Constant(Constant::ID, pParser->getGrounder(), x); }
term(res) ::= STRING(x).     { res = new Constant(Constant::ID, pParser->getGrounder(), x); }
term(res) ::= NUMBER(x).     { res = new Constant(Constant::NUM, pParser->getGrounder(), x); }
term(res) ::= LPARA term(a) RPARA.    { res = a; }
term(res) ::= term(a) MOD term(b).    { res = new FunctionTerm(FunctionTerm::MOD, a, b); }
term(res) ::= term(a) PLUS term(b).   { res = new FunctionTerm(FunctionTerm::PLUS, a, b); }
term(res) ::= term(a) TIMES term(b).  { res = new FunctionTerm(FunctionTerm::TIMES, a, b); }
term(res) ::= term(a) MINUS term(b).  { res = new FunctionTerm(FunctionTerm::MINUS, a, b); }
term(res) ::= term(a) DIVIDE term(b). { res = new FunctionTerm(FunctionTerm::DIVIDE, a, b); }
term(res) ::= ABS LPARA term(a) RPARA. { res = new FunctionTerm(FunctionTerm::ABS, a); }

constant(res) ::= IDENTIFIER(x). { res = pParser->getGrounder()->createConstValue(x); }
constant(res) ::= NUMBER(x).     { res = new Value(atol(x->c_str())); DELETE_PTR(x); }

const_term(res) ::= constant(c).   { res = c; }
const_term(res) ::= STRING(x).     { res = pParser->getGrounder()->createStringValue(x); }
const_term(res) ::= LPARA const_term(a) RPARA.          { res = a; }
const_term(res) ::= const_term(a) MOD const_term(b).    { res = new Value(*a % *b); DELETE_PTR(a); DELETE_PTR(b); }
const_term(res) ::= const_term(a) PLUS const_term(b).   { res = new Value(*a + *b); DELETE_PTR(a); DELETE_PTR(b); }
const_term(res) ::= const_term(a) TIMES const_term(b).  { res = new Value(*a * *b); DELETE_PTR(a); DELETE_PTR(b); }
const_term(res) ::= const_term(a) MINUS const_term(b).  { res = new Value(*a - *b); DELETE_PTR(a); DELETE_PTR(b); }
const_term(res) ::= const_term(a) DIVIDE const_term(b). { res = new Value(*a / *b); DELETE_PTR(a); DELETE_PTR(b); }
const_term(res) ::= ABS LPARA const_term(a) RPARA.      { res = a; }

aggregate(res) ::= SUM LBRAC weight_list(list) RBRAC.   { res = new AggregateLiteral(AggregateLiteral::SUM, list); }
aggregate(res) ::= MIN LBRAC weight_list(list) RBRAC.   { res = new AggregateLiteral(AggregateLiteral::MIN, list); }
aggregate(res) ::= MAX LBRAC weight_list(list) RBRAC.   { res = new AggregateLiteral(AggregateLiteral::MAX, list); }
aggregate(res) ::= TIMES LBRAC weight_list(list) RBRAC. { res = new AggregateLiteral(AggregateLiteral::TIMES, list); }
aggregate(res) ::= COUNT LBRAC constr_list(list) RBRAC. { res = new AggregateLiteral(AggregateLiteral::COUNT, list); }
aggregate(res) ::= LSBRAC weight_list(list) RSBRAC.     { res = new AggregateLiteral(AggregateLiteral::SUM, list); }
aggregate(res) ::= LBRAC constr_list(list) RBRAC.       { res = new AggregateLiteral(AggregateLiteral::COUNT, list); }

compute(res)  ::= COMPUTE  LBRAC  constr_list(list) RBRAC.  { res = new WeightedStatement(WeightedStatement::COMPUTE, list); }
minimize(res) ::= MINIMIZE LBRAC  constr_list(list) RBRAC.  { res = new WeightedStatement(WeightedStatement::MINIMIZE, list); }
minimize(res) ::= MINIMIZE LSBRAC weight_list(list) RSBRAC. { res = new WeightedStatement(WeightedStatement::MINIMIZE, list); }
maximize(res) ::= MAXIMIZE LBRAC  constr_list(list) RBRAC.  { res = new WeightedStatement(WeightedStatement::MAXIMIZE, list); }
maximize(res) ::= MAXIMIZE LSBRAC weight_list(list) RSBRAC. { res = new WeightedStatement(WeightedStatement::MAXIMIZE, list); }

weight_list(res) ::= nweight_list(list). { res = list; }
weight_list(res) ::= .                   { res = new ConditionalLiteralVector(); }
nweight_list(res) ::= nweight_list(list) COMMA weight_term(term). { res = list; res->push_back(term); }
nweight_list(res) ::= weight_term(term).                          { res = new ConditionalLiteralVector(); res->push_back(term); }

weight_term(res) ::= constraint_literal(literal) ASSIGN term(term). { res = literal; res->setWeight(term); }
weight_term(res) ::= constraint_literal(literal).                   { res = literal; res->setWeight(new Constant(Constant::NUM, pParser->getGrounder(), new std::string("1"))); }

constr_list(res) ::= nconstr_list(list). { res = list; }
constr_list(res) ::= .                   { res = new ConditionalLiteralVector(); }
nconstr_list(res) ::= nconstr_list(list) COMMA constr_term(term). { res = list; res->push_back(term); }
nconstr_list(res) ::= constr_term(term).                          { res = new ConditionalLiteralVector(); res->push_back(term); }

constr_term(res) ::= constraint_literal(literal).  { res = literal; res->setWeight(new Constant(Constant::NUM, pParser->getGrounder(), new std::string("1"))); }

