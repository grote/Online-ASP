#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <gringo.h>

namespace NS_GRINGO
{
	class Evaluator
	{
	public:
		Evaluator();
		virtual void initialize(Grounder *g);
		virtual void add(NS_OUTPUT::Object *r);
		virtual void evaluate();
		virtual ~Evaluator();
	public:
		Grounder          *g_;
		NS_OUTPUT::Output *o_;
	};
}

#endif

