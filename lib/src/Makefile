CC      = g++
CFLAGS  = -O3 -Wall -I../include -DNDEBUG
#CFLAGS  = -O0 -Wall -g -I.
LDFLAGS = 

SOURCES = $(wildcard *.cpp) lparselexer.cpp lparseparser_impl.cpp
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))

all: ../../bin/libgringo.a

../../bin/libgringo.a: $(OBJECTS)
	$(AR) cvrs $@ $(OBJECTS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

lparselexer.cpp: lparselexer.re
	re2c -o $@ $<

lemon: lemon.c
	gcc -o $@ $<

lparseparser_impl.cpp lparseparser_impl.h: lparseparser_impl.y lemon lempar.c
	./lemon lparseparser_impl.y
	rm lparseparser_impl.out
	mv lparseparser_impl.c lparseparser_impl.cpp
	touch lparseparser_impl.h lparseparser_impl.cpp

clean:
	rm -f $(OBJECTS) lparseparser_impl.h lparseparser_impl.cpp lparselexer.cpp lemon gringo

depend: $(SOURCES) true
	makedepend -Y -- $(CFLAGS) -- $(SOURCES) &> /dev/null
	cat Makefile | sed 's/ \.\/gringo\.h//g' > Makefile.bak
	mv Makefile.bak Makefile
true:

# DO NOT DELETE

aggregateliteral.o: ../include/aggregateliteral.h ../include/gringo.h
aggregateliteral.o: ../include/literal.h ../include/printable.h
aggregateliteral.o: ../include/expandable.h ../include/conditionalliteral.h
aggregateliteral.o: ../include/groundable.h ../include/term.h
aggregateliteral.o: ../include/node.h ../include/value.h
aggregateliteral.o: ../include/grounder.h ../include/output.h
aggregateliteral.o: ../include/dependencygraph.h ../include/indexeddomain.h
aggregateliteral.o: ../include/evaluator.h
assignmentliteral.o: ../include/assignmentliteral.h ../include/gringo.h
assignmentliteral.o: ../include/literal.h ../include/printable.h
assignmentliteral.o: ../include/term.h ../include/constant.h
assignmentliteral.o: ../include/value.h ../include/indexeddomain.h
assignmentliteral.o: ../include/dlvgrounder.h ../include/grounder.h
assignmentliteral.o: ../include/output.h
basicprogramevaluator.o: ../include/basicprogramevaluator.h
basicprogramevaluator.o: ../include/gringo.h ../include/evaluator.h
basicprogramevaluator.o: ../include/output.h ../include/value.h
basicprogramevaluator.o: ../include/grounder.h ../include/node.h
conditionalliteral.o: ../include/conditionalliteral.h ../include/gringo.h
conditionalliteral.o: ../include/literal.h ../include/printable.h
conditionalliteral.o: ../include/groundable.h ../include/expandable.h
conditionalliteral.o: ../include/predicateliteral.h ../include/value.h
conditionalliteral.o: ../include/grounder.h ../include/output.h
conditionalliteral.o: ../include/term.h ../include/node.h
conditionalliteral.o: ../include/indexeddomain.h ../include/dlvgrounder.h
constant.o: ../include/constant.h ../include/gringo.h ../include/term.h
constant.o: ../include/printable.h ../include/value.h ../include/grounder.h
constant.o: ../include/output.h
dependencygraph.o: ../include/dependencygraph.h ../include/gringo.h
dependencygraph.o: ../include/printable.h ../include/predicateliteral.h
dependencygraph.o: ../include/literal.h ../include/groundable.h
dependencygraph.o: ../include/expandable.h ../include/value.h
dependencygraph.o: ../include/node.h ../include/grounder.h
dependencygraph.o: ../include/output.h ../include/scc.h
dlvgrounder.o: ../include/dlvgrounder.h ../include/gringo.h
dlvgrounder.o: ../include/grounder.h ../include/output.h ../include/value.h
dlvgrounder.o: ../include/groundable.h ../include/literal.h
dlvgrounder.o: ../include/printable.h ../include/indexeddomain.h
evaluator.o: ../include/evaluator.h ../include/gringo.h ../include/grounder.h
evaluator.o: ../include/output.h ../include/value.h ../include/node.h
functionterm.o: ../include/functionterm.h ../include/gringo.h
functionterm.o: ../include/term.h ../include/printable.h ../include/value.h
gringolexer.o: ../include/gringolexer.h ../include/gringo.h
gringooutput.o: ../include/gringooutput.h ../include/gringo.h
gringooutput.o: ../include/output.h ../include/value.h
gringoparser.o: ../include/gringoparser.h ../include/gringo.h
gringoparser.o: ../include/gringolexer.h
grounder.o: ../include/grounder.h ../include/gringo.h ../include/output.h
grounder.o: ../include/value.h ../include/statement.h ../include/printable.h
grounder.o: ../include/groundable.h ../include/constant.h ../include/term.h
grounder.o: ../include/predicateliteral.h ../include/literal.h
grounder.o: ../include/expandable.h ../include/dependencygraph.h
grounder.o: ../include/scc.h ../include/gringooutput.h ../include/evaluator.h
grounder.o: ../include/node.h
indexeddomain.o: ../include/indexeddomain.h ../include/gringo.h
indexeddomain.o: ../include/value.h ../include/grounder.h ../include/output.h
indexeddomain.o: ../include/dlvgrounder.h ../include/literal.h
indexeddomain.o: ../include/printable.h ../include/constant.h
indexeddomain.o: ../include/term.h
literal.o: ../include/literal.h ../include/gringo.h ../include/printable.h
lparselexer.o: ../include/lparselexer.h ../include/gringo.h
lparselexer.o: ../include/gringolexer.h ../include/lparseparser.h
lparselexer.o: ../include/gringoparser.h lparseparser_impl.h
lparseoutput.o: ../include/lparseoutput.h ../include/gringo.h
lparseoutput.o: ../include/output.h ../include/value.h
lparseparser.o: ../include/lparseparser.h ../include/gringo.h
lparseparser.o: ../include/gringoparser.h lparseparser_impl.h
lparseparser.o: ../include/lparselexer.h ../include/gringolexer.h
lparseparser.o: ../include/grounder.h ../include/output.h ../include/value.h
lparseparser_impl.o: ../include/gringo.h ../include/lparseparser.h
lparseparser_impl.o: ../include/gringoparser.h ../include/literal.h
lparseparser_impl.o: ../include/printable.h ../include/aggregateliteral.h
lparseparser_impl.o: ../include/expandable.h ../include/predicateliteral.h
lparseparser_impl.o: ../include/groundable.h ../include/value.h
lparseparser_impl.o: ../include/conditionalliteral.h
lparseparser_impl.o: ../include/relationliteral.h
lparseparser_impl.o: ../include/assignmentliteral.h ../include/term.h
lparseparser_impl.o: ../include/constant.h ../include/functionterm.h
lparseparser_impl.o: ../include/rangeterm.h ../include/multipleargsterm.h
lparseparser_impl.o: ../include/normalrule.h ../include/statement.h
lparseparser_impl.o: ../include/grounder.h ../include/output.h
multipleargsterm.o: ../include/multipleargsterm.h ../include/gringo.h
multipleargsterm.o: ../include/term.h ../include/printable.h
multipleargsterm.o: ../include/value.h
node.o: ../include/node.h ../include/gringo.h ../include/value.h
node.o: ../include/scc.h ../include/printable.h
normalrule.o: ../include/normalrule.h ../include/gringo.h
normalrule.o: ../include/statement.h ../include/printable.h
normalrule.o: ../include/groundable.h ../include/expandable.h
normalrule.o: ../include/literal.h ../include/predicateliteral.h
normalrule.o: ../include/value.h ../include/dependencygraph.h
normalrule.o: ../include/node.h ../include/grounder.h ../include/output.h
normalrule.o: ../include/dlvgrounder.h ../include/evaluator.h
output.o: ../include/output.h ../include/gringo.h ../include/value.h
output.o: ../include/node.h ../include/grounder.h
predicateliteral.o: ../include/dependencygraph.h ../include/gringo.h
predicateliteral.o: ../include/printable.h ../include/predicateliteral.h
predicateliteral.o: ../include/literal.h ../include/groundable.h
predicateliteral.o: ../include/expandable.h ../include/value.h
predicateliteral.o: ../include/assignmentliteral.h
predicateliteral.o: ../include/relationliteral.h ../include/grounder.h
predicateliteral.o: ../include/output.h ../include/constant.h
predicateliteral.o: ../include/term.h ../include/node.h
predicateliteral.o: ../include/rangeterm.h ../include/rangeliteral.h
predicateliteral.o: ../include/indexeddomain.h ../include/dlvgrounder.h
predicateliteral.o: ../include/multipleargsterm.h ../include/evaluator.h
printable.o: ../include/printable.h ../include/gringo.h
rangeliteral.o: ../include/rangeliteral.h ../include/gringo.h
rangeliteral.o: ../include/literal.h ../include/printable.h
rangeliteral.o: ../include/constant.h ../include/term.h ../include/value.h
rangeliteral.o: ../include/indexeddomain.h ../include/dlvgrounder.h
rangeliteral.o: ../include/grounder.h ../include/output.h
rangeterm.o: ../include/rangeterm.h ../include/gringo.h ../include/term.h
rangeterm.o: ../include/printable.h ../include/value.h
relationliteral.o: ../include/relationliteral.h ../include/gringo.h
relationliteral.o: ../include/literal.h ../include/printable.h
relationliteral.o: ../include/term.h ../include/value.h
relationliteral.o: ../include/indexeddomain.h
rule.o: ../include/statement.h ../include/gringo.h ../include/printable.h
rule.o: ../include/groundable.h
scc.o: ../include/scc.h ../include/gringo.h ../include/printable.h
scc.o: ../include/statement.h ../include/groundable.h ../include/grounder.h
scc.o: ../include/output.h ../include/value.h ../include/literal.h
scc.o: ../include/evaluator.h ../include/basicprogramevaluator.h
smodelsoutput.o: ../include/smodelsoutput.h ../include/gringo.h
smodelsoutput.o: ../include/output.h ../include/value.h ../include/grounder.h
term.o: ../include/term.h ../include/gringo.h ../include/printable.h
value.o: ../include/value.h ../include/gringo.h
lparselexer.o: ../include/lparselexer.h ../include/gringo.h
lparselexer.o: ../include/gringolexer.h ../include/lparseparser.h
lparselexer.o: ../include/gringoparser.h lparseparser_impl.h
lparseparser_impl.o: ../include/gringo.h ../include/lparseparser.h
lparseparser_impl.o: ../include/gringoparser.h ../include/literal.h
lparseparser_impl.o: ../include/printable.h ../include/aggregateliteral.h
lparseparser_impl.o: ../include/expandable.h ../include/predicateliteral.h
lparseparser_impl.o: ../include/groundable.h ../include/value.h
lparseparser_impl.o: ../include/conditionalliteral.h
lparseparser_impl.o: ../include/relationliteral.h
lparseparser_impl.o: ../include/assignmentliteral.h ../include/term.h
lparseparser_impl.o: ../include/constant.h ../include/functionterm.h
lparseparser_impl.o: ../include/rangeterm.h ../include/multipleargsterm.h
lparseparser_impl.o: ../include/normalrule.h ../include/statement.h
lparseparser_impl.o: ../include/grounder.h ../include/output.h
