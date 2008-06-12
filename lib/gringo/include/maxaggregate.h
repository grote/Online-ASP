#ifndef MAXAGGREGATE_H
#define MAXAGGREGATE_H

#include <gringo.h>
#include <aggregateliteral.h>

namespace NS_GRINGO
{
	class MaxAggregate : public AggregateLiteral
	{
	public:
		MaxAggregate(ConditionalLiteralVector *literals);
		MaxAggregate(const MaxAggregate &a);
		virtual Literal *clone() const;
		virtual void match(Grounder *g, int &lower, int &upper);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual ~MaxAggregate();
	};
}

#endif

