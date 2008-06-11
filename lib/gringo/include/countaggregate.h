#ifndef COUNTAGGREGATE_H
#define COUNTAGGREGATE_H

#include <gringo.h>
#include <aggregateliteral.h>

namespace NS_GRINGO
{
	class CountAggregate : public AggregateLiteral
	{
	public:
		CountAggregate(ConditionalLiteralVector *literals);
		CountAggregate(const CountAggregate &a);
		virtual Literal *clone() const;
		virtual void match(Grounder *g, int &lower, int &upper);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual ~CountAggregate();
	};
}

#endif

