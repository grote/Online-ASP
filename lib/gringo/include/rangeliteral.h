#ifndef RANGELITERAL_H
#define RANGELITERAL_H

#include <gringo.h>
#include <literal.h>

namespace NS_GRINGO
{
	class RangeLiteral : public Literal
	{
	public:
		RangeLiteral(Constant *var, Term *lower, Term *upper);
		RangeLiteral(RangeLiteral &r);
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void getVars(VarSet &vars);
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual void finish();
		virtual bool solved();
		virtual bool isFact();
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual NS_OUTPUT::Object *convert();
		virtual Literal* clone();
		virtual void print(std::ostream &out);
		virtual ~RangeLiteral();
	protected:
		Constant *var_;
		Term *lower_;
		Term *upper_;
	};
}

#endif

