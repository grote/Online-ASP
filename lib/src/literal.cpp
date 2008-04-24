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

void Literal::getVars(VarSet &vars, VarsType type, VarVector &glob)
{
	VarSet temp;
	getVars(temp, type);
	for(VarSet::iterator it = temp.begin(); it != temp.end(); it++)
	{
		if(std::binary_search(glob.begin(), glob.end(), *it))
			vars.insert(*it);
	}
}

Literal::~Literal()
{

}


