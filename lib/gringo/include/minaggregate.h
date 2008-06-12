#ifndef MINAGGREGATE_H
#define MINAGGREGATE_H

#include <gringo.h>
#include <aggregateliteral.h>

namespace NS_GRINGO
{
	class MinAggregate : public AggregateLiteral
	{
	public:
		MinAggregate(ConditionalLiteralVector *literals);
		MinAggregate(const MinAggregate &a);
		virtual Literal *clone() const;
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual void match(Grounder *g, int &lower, int &upper, int &fixed);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual ~MinAggregate();
	};
}

#endif

