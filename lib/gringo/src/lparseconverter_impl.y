%include {

#include "gringo.h"

#include "lparseconverter.h"

#include "output.h"

using namespace NS_GRINGO;

#define GROUNDER (pParser->getGrounder())
#define STRING(x) (pParser->getGrounder()->createString(x))
#define DELETE_PTR(X) { if(X) delete (X); }
#define DELETE_PTRVECTOR(T, X) { if(X){ for(T::iterator it = (X)->begin(); it != (X)->end(); it++) delete (*it); delete (X); } }
}  

%name lparseconverter
%token_prefix LPARSECONVERTER_

%extra_argument { LparseConverter *pConverter }

%parse_failure {
	pConverter->handleError();
}

%syntax_error {
	pConverter->handleError();
}

// token type/destructor for terminals
%token_type { std::string* }
%token_destructor { DELETE_PTR($$) }

// this will define the symbols in the header 
// even though they are not used in the rules
%nonassoc ERROR EOI.

%start_symbol start

start ::= program.

program ::= program rule DOT.     
program ::= program SHOW show_list DOT.
program ::= program HIDE hide_list DOT.
program ::= .

show_list ::= show_list COMMA show_predicate.
show_list ::= show_predicate.

hide_list ::= .           
hide_list ::= nhide_list.

nhide_list ::= nhide_list COMMA hide_predicate.
nhide_list ::= hide_predicate.

show_predicate ::= IDENTIFIER.                                 
hide_predicate ::= IDENTIFIER.                                 
show_predicate ::= IDENTIFIER LPARA variable_list RPARA. 
hide_predicate ::= IDENTIFIER LPARA variable_list RPARA. 

variable_list ::= variable_list COMMA VARIABLE. 
variable_list ::= VARIABLE.                           

rule ::= head_atom IF body. 
rule ::= head_atom IF .           
rule ::= head_atom.               
rule ::= IF body.                 
rule ::= IF.                            
rule ::= maximize.                 
rule ::= minimize.                 
rule ::= compute.                 

body ::= body COMMA body_literal. 
body ::= body_literal.                  

body_literal ::= body_atom.     
body_literal ::= NOT body_atom. 

constraint_literal ::= predicate.     
constraint_literal ::= NOT predicate. 

body_atom ::= predicate.      
body_atom ::= aggregate_atom. 

head_atom ::= aggregate_atom. 
head_atom ::= disjunction.    

disjunction ::= disj_list. 

disj_list ::= disj_list DISJUNCTION predicate. 
disj_list ::= predicate.                             

predicate ::= IDENTIFIER LPARA constant_list RPARA.       
predicate ::= IDENTIFIER.                                       
predicate ::= MINUS IDENTIFIER LPARA constant_list RPARA. 
predicate ::= MINUS IDENTIFIER.                                 

aggregate_atom ::= NUMBER aggregate NUMBER. 
aggregate_atom ::= aggregate NUMBER.
aggregate_atom ::= NUMBER aggregate.         
aggregate_atom ::= aggregate.                 

constant_list ::= nconstant_list. 
constant_list ::= .                     
nconstant_list ::= constant_list COMMA constant. 
nconstant_list ::= constant.                           

constant ::= IDENTIFIER. 
constant ::= NUMBER.     
constant ::= STRING.     

aggregate ::= SUM LBRAC weight_list RBRAC.   
aggregate ::= MIN LBRAC weight_list RBRAC.   
aggregate ::= MAX LBRAC weight_list RBRAC.   
aggregate ::= TIMES LBRAC weight_list RBRAC. 
aggregate ::= COUNT LBRAC constr_list RBRAC. 
aggregate ::= LSBRAC weight_list RSBRAC.     
aggregate ::= LBRAC constr_list RBRAC.       

compute  ::= COMPUTE LBRAC constr_list RBRAC.            
compute  ::= COMPUTE NUMBER LBRAC constr_list RBRAC.  
minimize ::= MINIMIZE LBRAC  constr_list RBRAC.          
minimize ::= MINIMIZE LSBRAC weight_list RSBRAC.         
maximize ::= MAXIMIZE LBRAC  constr_list RBRAC.          
maximize ::= MAXIMIZE LSBRAC weight_list RSBRAC.         

weight_list ::= nweight_list. 
weight_list ::= .                   
nweight_list ::= nweight_list COMMA weight_literal. 
nweight_list ::= weight_literal.                          

weight_literal ::= constraint_literal ASSIGN NUMBER. 
weight_literal ::= constraint_literal.                  

constr_list ::= nconstr_list. 
constr_list ::= .                   
nconstr_list ::= nconstr_list COMMA constraint_literal. 
nconstr_list ::= constraint_literal.                                        

