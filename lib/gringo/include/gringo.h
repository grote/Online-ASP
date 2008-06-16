#ifndef GRINGO_H
#define GRINGO_H

#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <stack>
#include <cmath>
#include <cfloat>
#include <typeinfo>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <memory>

namespace NS_GRINGO
{
	class GlobalStorage;

	class LDGBuilder;
	typedef std::vector<LDGBuilder*> LDGBuilderVector;

	class LDG;
	typedef std::vector<LDG*> LDGVector;

	typedef std::pair<std::string *, int> Signature;
	typedef std::vector<Signature> SignatureVector;
	class Evaluator;


	class Expandable;
	class Groundable;

	class IndexedDomain;
	typedef std::vector<IndexedDomain*> IndexedDomainVector;

	class Value;
	typedef std::vector<Value> ValueVector;

	enum MatchStatus { SuccessfulMatch, FailureOnNextMatch, FailureOnFirstMatch };
	
	class DLVGrounder;

	class Printable;
	class DependencyRelation;

	enum VarsType { VARS_PROVIDED, VARS_NEEDED, VARS_GLOBAL, VARS_ALL };
	typedef std::set<int> VarSet;
	typedef std::set<int> IntSet;
	typedef std::vector<int> VarVector;
	typedef std::vector<int> IntVector;

	class Constant;
	typedef std::vector<std::string*> StringVector;
	typedef std::vector<Constant*> ConstantVector;

	class SCC;
	typedef std::vector<SCC*> SCCVector;
	typedef std::set<SCC*> SCCSet;

	class AggregateLiteral;

	class DependencyGraph;

	class GrinGoParser;
	class LparseParser;

	class GrinGoLexer;
	class LparseLexer;
	class PlainLparseLexer;

	class Grounder;

	class Literal;
	typedef std::vector<Literal*> LiteralVector;
	typedef std::set<Literal*> LiteralSet;

	class Node;
	typedef std::vector<Node*> NodeVector;

	class Statement;
	typedef std::vector<Statement*> StatementVector;

	class Term;
	typedef std::vector<Term*> TermVector;
	
	class ConditionalLiteral;
	typedef std::vector<ConditionalLiteral*> ConditionalLiteralVector;

	class PredicateLiteral;
	typedef std::vector<PredicateLiteral*> PredicateLiteralVector;

	class FuncSymbol;
	typedef std::vector<FuncSymbol* > FuncSymbolVector;
	
	class ConditionalLiteralTarget;

	static const char *NL = "\n";

	namespace NS_OUTPUT
	{
		class Output;
		class Object;
		class Atom;
	}
}

#endif

