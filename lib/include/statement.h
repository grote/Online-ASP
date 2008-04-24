#ifndef STATEMENT_H
#define STATEMENT_H

#include <gringo.h>
#include <printable.h>
#include <groundable.h>

namespace NS_GRINGO
{
	class Statement : public Printable, public Groundable
	{
	public:
		Statement();
		virtual void buildDepGraph(DependencyGraph *dg) = 0;
		virtual void getVars(VarSet &vars) = 0;
		virtual bool checkO(LiteralVector &unsolved) = 0;
		virtual bool check(VarVector &free) = 0;
		virtual void normalize(Grounder *g) = 0;
		virtual void preprocess(Grounder *g) = 0; 
		virtual void reset() = 0;
		virtual void finish() = 0;
		virtual void evaluate() = 0;
		virtual bool ground(Grounder *g) = 0;
		virtual void addDomain(PredicateLiteral *pl) = 0;
		virtual ~Statement();
	};
}

#endif

