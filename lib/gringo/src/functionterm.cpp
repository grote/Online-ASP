#include "functionterm.h"
#include "value.h"

using namespace NS_GRINGO;

FunctionTerm::FunctionTerm(FunctionType type, Term *a, Term *b) : Term(), type_(type), a_(a), b_(b)
{
}

void FunctionTerm::print(std::ostream &out)
{
	switch(type_)
	{
		case ABS:
			out << "abs" << "(" << a_ << ")";
			break;
		case PLUS:
			out << "(" << a_ << " + " << b_ << ")";
			break;
		case MINUS:
			out << "(" << a_ << " - " << b_ << ")";
			break;
		case TIMES:
			out << "(" << a_ << " * " << b_ << ")";
			break;
		case DIVIDE:
			out << "(" << a_ << " / " << b_ << ")";
			break;
		case MOD:
			out << "(" << a_ << " mod " << b_ << ")";
			break;
	}
}

void FunctionTerm::getVars(VarSet &vars)
{
	a_->getVars(vars);
	if(b_)
		b_->getVars(vars);
}

void FunctionTerm::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
	a_->preprocess(l, a_, g, e);
	if(b_)
		b_->preprocess(l, b_, g, e);
}

bool FunctionTerm::isComplex()
{
	return true;
}

Value FunctionTerm::getValue()
{
	switch(type_)
	{
		case ABS:
			return Value(abs(a_->getValue()));
		case PLUS:
			return Value(a_->getValue() + b_->getValue());
		case MINUS:
			return Value(a_->getValue() - b_->getValue());
		case TIMES:
			return Value(a_->getValue() * b_->getValue());
		case DIVIDE:
			return Value(a_->getValue() / b_->getValue());
		case MOD:
			return Value(a_->getValue() % b_->getValue());
	}
	assert(false);
}

FunctionTerm::FunctionTerm(FunctionTerm &f) : type_(f.type_), a_(f.a_->clone()), b_(f.b_ ? f.b_->clone() : 0)
{
}

Term* FunctionTerm::clone()
{
	return new FunctionTerm(*this);
}

FunctionTerm::~FunctionTerm()
{
	if(a_)
		delete a_;
	if(b_)
		delete b_;

}

