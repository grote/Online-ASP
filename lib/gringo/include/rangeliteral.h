#ifndef RANGELITERAL_H
#define RANGELITERAL_H

#include <gringo.h>
#include <literal.h>

namespace NS_GRINGO
{
	class RangeLiteral : public Literal
	{
	public:
		RangeLiteral(Constant *var, int lower, int upper);
		RangeLiteral(RangeLiteral &r);
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void getVars(VarSet &vars, VarsType type);
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual void normalize(Grounder *g, Expandable *r);
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
		int lower_;
		int upper_;
	};
}

#endif

