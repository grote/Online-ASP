#include "literal.h"

using namespace NS_GRINGO;

Literal::Literal() : neg_(false)
{
}

void Literal::setNeg(bool neg)
{
	neg_ = neg;
}

bool Literal::getNeg()
{
	return neg_;
}
		
void Literal::ground(Grounder *g)
{
}

void Literal::evaluate()
{
}

Literal::~Literal()
{

}


