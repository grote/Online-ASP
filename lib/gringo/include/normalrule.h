#ifndef NORMALRULE_H
#define NORMALRULE_H

#include <gringo.h>
#include <statement.h>
#include <expandable.h>

namespace NS_GRINGO
{
	/// This class is used to represent normal rules including integrity constraints and facts
	class NormalRule : public Statement, public Expandable
	{
	public:
		/**
		 * \brief Constructor
		 * \param head The head of the rule
		 * \param body The body of the rule
		 */
		NormalRule(Literal *head, LiteralVector *body);
		/// Destructor
		virtual ~NormalRule();
		// implemented from base class or interface
		virtual void getVars(VarSet &vars);
		virtual void buildDepGraph(DependencyGraph *dg);
		virtual void print(std::ostream &out);
		virtual bool checkO(LiteralVector &unsolved);
		virtual bool check(VarVector &free);
		virtual void reset();
		virtual void preprocess(Grounder *g);
		virtual void appendLiteral(Literal *l, ExpansionType type);
		virtual void finish();
		virtual void evaluate();
		virtual void grounded(Grounder *g);
		virtual bool ground(Grounder *g);
		virtual void addDomain(PredicateLiteral *pl);
	private:
		/**
		 * \brief Calculate the relevant vars in the body
		 * \param relevant The resulting set of relevant vars
		 */
		void getRelevantVars(VarVector &relevant);
	public:
		/// The haed
		Literal *head_;
		/// The body
		LiteralVector *body_;
		/// The LiteralDependencyGraph
		LDG *dg_;
	};
}

#endif

