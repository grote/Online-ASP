#ifndef AGGREGATELITERAL_H
#define AGGREGATELITERAL_H

#include <gringo.h>
#include <literal.h>
#include <expandable.h>

namespace NS_GRINGO
{
	class AggregateLiteral : public Literal, public Expandable
	{
	public:
		enum AggregateType { SUM, TIMES, COUNT, MIN, MAX };
	public:
		AggregateLiteral(AggregateType type, ConditionalLiteralVector *literals);
		AggregateLiteral(AggregateLiteral &a);
		virtual Literal* clone();
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void getVars(VarSet &vars, VarsType type, VarVector &glob);
		virtual void getVars(VarSet &vars, VarsType type);
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual void normalize(Grounder *g, Expandable *r);
		virtual void finish();
		virtual bool solved();
		virtual bool isFact();
		virtual void ground(Grounder *g);
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual void print(std::ostream &out);
		virtual void appendLiteral(Literal *l);
		virtual NS_OUTPUT::Object *convert();
		ConditionalLiteralVector *getLiterals();
		void setBounds(Term *lower, Term *upper);
		Term *getLower();
		Term *getUpper();
		AggregateType getType();
		virtual ~AggregateLiteral();
	protected:
		AggregateType type_;
		ConditionalLiteralVector *literals_;
		Term *lower_;
		Term *upper_;
		ConditionalLiteralVector::iterator current_;
	};
}

#endif
