#include "evaluator.h"
#include "grounder.h"
#include "output.h"
#include "node.h"

using namespace NS_GRINGO;
		
Evaluator::Evaluator() : g_(0), o_(0)
{
}

void Evaluator::initialize(Grounder *g)
{
	g_    = g;
	o_    = g->getOutput();
}

void Evaluator::add(NS_OUTPUT::Object *r)
{
	Finalize<NS_OUTPUT::Object> f(r);
	r->addUid(o_);
	r->addDomain();
	o_->print(r);
}

void Evaluator::evaluate()
{

}

Evaluator::~Evaluator()
{
}

