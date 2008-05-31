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
		RelationLiteral(const RelationLiteral &r);
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual void finish();
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual bool solved();
		virtual bool isFact();
		virtual Literal* clone() const;
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual double heuristicValue();
		virtual ~RelationLiteral();
	protected:
		RelationType type_;
		Term *a_;
		Term *b_;
	};
}

#endif

