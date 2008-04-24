#ifndef NORMALRULE_H
#define NORMALRULE_H

#include <gringo.h>
#include <statement.h>
#include <expandable.h>

namespace NS_GRINGO
{
	class NormalRule : public Statement, public Expandable
	{
	public:
		NormalRule(Literal *head, LiteralVector *body);
		virtual void getVars(VarSet &vars);
		virtual void buildDepGraph(DependencyGraph *dg);
		virtual void print(std::ostream &out);
		virtual bool checkO(LiteralVector &unsolved);
		virtual bool check(VarVector &free);
		virtual void reset();
		virtual void normalize(Grounder *g);
		virtual void preprocess(Grounder *g);
		virtual void appendLiteral(Literal *l);
		virtual void finish();
		virtual void evaluate();
		virtual void grounded(Grounder *g);
		virtual bool ground(Grounder *g);
		virtual void addDomain(PredicateLiteral *pl);
		virtual ~NormalRule();
	private:
		void getGlobalVars(VarSet &glob);
		void getRelevantVars(VarSet &relevant, VarSet &glob);
	public:
		Literal *head_;
		LiteralVector *body_;
	};
}

#endif

