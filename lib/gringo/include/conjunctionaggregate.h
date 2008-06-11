#ifndef CONJUNCTIONAGGREGATE_H
#define CONJUNCTIONAGGREGATE_H

#include <gringo.h>
#include <aggregateliteral.h>

namespace NS_GRINGO
{
	class ConjunctionAggregate : public AggregateLiteral
	{
	public:
		ConjunctionAggregate(ConditionalLiteralVector *literals);
		ConjunctionAggregate(const ConjunctionAggregate &a);
		virtual Literal *clone() const;
		virtual bool match(Grounder *g);
		virtual void match(Grounder *g, int &lower, int &upper);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual ~ConjunctionAggregate();
	public:
	       	static Literal *createHead(ConditionalLiteralVector *list);
	       	static Literal *createBody(PredicateLiteral *pred, LiteralVector *list);
	};
}

#endif

