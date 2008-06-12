#ifndef SUMAGGREGATE_H
#define SUMAGGREGATE_H

#include <gringo.h>
#include <aggregateliteral.h>

namespace NS_GRINGO
{
	class SumAggregate : public AggregateLiteral
	{
	public:
		SumAggregate(ConditionalLiteralVector *literals);
		SumAggregate(const SumAggregate &a);
		virtual Literal *clone() const;
		virtual void match(Grounder *g, int &lower, int &upper, int &fixed);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual ~SumAggregate();
	};
}

#endif

