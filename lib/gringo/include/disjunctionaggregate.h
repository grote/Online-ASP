#ifndef DISJUNCTIONAGGREGATE_H
#define DISJUNCTIONAGGREGATE_H

#include <gringo.h>
#include <aggregateliteral.h>

namespace NS_GRINGO
{
	class DisjunctionAggregate : public AggregateLiteral
	{
	public:
		DisjunctionAggregate(ConditionalLiteralVector *literals);
		DisjunctionAggregate(const DisjunctionAggregate &a);
		virtual Literal *clone() const;
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual bool match(Grounder *g);
		virtual void match(Grounder *g, int &lower, int &upper, int &fixed);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual ~DisjunctionAggregate();
	public:
	       	static Literal *createHead(ConditionalLiteralVector *list);
	};
}

#endif

