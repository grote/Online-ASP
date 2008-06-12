#ifndef WEIGHTEDSTATEMENT_H
#define WEIGHTEDSTATEMENT_H

#include <gringo.h>
#include <statement.h>
#include <expandable.h>

namespace NS_GRINGO
{
	/**
	 * \brief A generic class for statements with weights
	 *
	 * Includes compute, minimize and maximize statements. Weights in compute statements are ignored.
	 */
	class WeightedStatement : public Statement, public Expandable
	{
	public:
		enum Type { COMPUTE, MINIMIZE, MAXIMIZE };
	public:
		/// Constructor
		WeightedStatement(Type type, ConditionalLiteralVector *literals, bool setSemantics, int number = 0);
		virtual void buildDepGraph(DependencyGraph *dg);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		virtual bool check(VarVector &free);
		virtual void preprocess(Grounder *g);
		virtual void reset();
		virtual void finish();
		virtual void evaluate();
		virtual bool ground(Grounder *g);
		virtual void addDomain(PredicateLiteral *pl);
		virtual void print(std::ostream &out);
		virtual void grounded(Grounder *g);
		virtual void appendLiteral(Literal *l, ExpansionType type);
		/// Destructor
		virtual ~WeightedStatement();
	protected:
		Type type_;
		bool setSemantics_;
		int number_;
		ConditionalLiteralVector literals_;
	};
}

#endif

