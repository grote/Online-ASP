#include "rangeterm.h"
#include "value.h"

using namespace NS_GRINGO;

RangeTerm::RangeTerm(int lower, int upper) : Term(), lower_(lower), upper_(upper)
{
}

void RangeTerm::getVars(VarSet &vars)
{
}

bool RangeTerm::isComplex()
{
	return false;
}

void RangeTerm::print(std::ostream &out)
{
	out << lower_ << ".." << upper_;
}

int RangeTerm::getLower()
{
	return lower_;
}

int RangeTerm::getUpper()
{
	return upper_;
}

Value RangeTerm::getValue()
{
	assert(false);
}

RangeTerm::RangeTerm(RangeTerm &r) : lower_(r.lower_), upper_(r.upper_)
{
}

Term* RangeTerm::clone()
{
	return new RangeTerm(*this);
}

RangeTerm::~RangeTerm()
{
}

