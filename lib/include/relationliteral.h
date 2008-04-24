#ifndef RELATIONLITERAL_H
#define RELATIONLITERAL_H

#include <gringo.h>
#include <literal.h>

namespace NS_GRINGO
{
	class RelationLiteral : public Literal
	{
	public:
		enum RelationType { EQ, NE, GT, GE, LE, LT };
	public:
		RelationLiteral(RelationType type, Term *a, Term *b);
		RelationLiteral(RelationLiteral &r);
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void getVars(VarSet &vars, VarsType type);
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual void normalize(Grounder *g, Expandable *r);
		virtual void finish();
		virtual void preprocess(Expandable *e);
		virtual bool solved();
		virtual bool isFact();
		virtual Literal* clone();
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual ~RelationLiteral();
	protected:
		RelationType type_;
		Term *a_;
		Term *b_;
	};
}

#endif

