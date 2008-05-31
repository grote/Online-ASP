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
		enum AggregateType { SUM, TIMES, COUNT, MIN, MAX, DISJUNCTION, CONJUNCTION };
	public:
		AggregateLiteral(AggregateType type, ConditionalLiteralVector *literals);
		AggregateLiteral(const AggregateLiteral &a);
		virtual Literal* clone() const;
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual void finish();
		virtual bool solved();
		virtual bool isFact();
		virtual void ground(Grounder *g);
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual void print(std::ostream &out);
		virtual void appendLiteral(Literal *l, ExpansionType type);
		virtual NS_OUTPUT::Object *convert();
		virtual double heuristicValue();
		ConditionalLiteralVector *getLiterals();
		void setBounds(Term *lower, Term *upper);
		Term *getLower() const;
		Term *getUpper() const;
		AggregateType getType() const;
		virtual ~AggregateLiteral();
	public:
	       	static Literal *createHead(ConditionalLiteralVector *list);
	       	static Literal *createBody(PredicateLiteral *pred, LiteralVector *list);
	protected:
		AggregateType type_;
		ConditionalLiteralVector *literals_;
		Term *lower_;
		Term *upper_;
		ConditionalLiteralVector::iterator current_;
	};
}

#endif
