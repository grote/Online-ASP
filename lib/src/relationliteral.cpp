#include "relationliteral.h"
#include "term.h"
#include "value.h"
#include "indexeddomain.h"

using namespace NS_GRINGO;

RelationLiteral::RelationLiteral(RelationType type, Term *a, Term *b) : Literal(), type_(type), a_(a), b_(b)
{

}

Node *RelationLiteral::createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo)
{
	return 0;
}

void RelationLiteral::print(std::ostream &out)
{
	out << a_;
	switch(type_)
	{ 
		case EQ:
			out << " == ";
			break;
		case NE:
			out << " != ";
			break;
		case GT:
			out << " > ";
			break;
		case GE:
			out << " >= ";
			break;
		case LE:
			out << " <= ";
			break;
		case LT:
			out << " < ";
			break;
	}
	out << b_;
}

bool RelationLiteral::isFact()
{
	return true;
}

bool RelationLiteral::solved()
{
	return true;
}

void RelationLiteral::normalize(Grounder *g, Expandable *r)
{
	// nothing todo
}

void RelationLiteral::getVars(VarSet &vars, VarsType type)
{
	switch(type)
	{
		case VARS_PROVIDED:
			// relation literals dont provide vars
			break;
		case VARS_GLOBAL:
		case VARS_ALL:
		case VARS_NEEDED:
			a_->getVars(vars);
			b_->getVars(vars);
			break;
	}
}

bool RelationLiteral::checkO(LiteralVector &unsolved)
{
	return true;
}

void RelationLiteral::reset()
{
	assert(false);
}

void RelationLiteral::finish()
{
	assert(false);
}

bool RelationLiteral::match(Grounder *g)
{
	switch(type_)
	{ 
		case EQ:
			return a_->getValue() == b_->getValue();
		case NE:
			return a_->getValue() != b_->getValue();
		case GT:
			return a_->getValue() > b_->getValue();
		case GE:
			return a_->getValue() >= b_->getValue();
		case LE:
			return a_->getValue() <= b_->getValue();
		case LT:
			return a_->getValue() < b_->getValue();
	}
	assert(false);
}

void RelationLiteral::preprocess(Expandable *e)
{
	// nothing todo
}

IndexedDomain *RelationLiteral::createIndexedDomain(VarSet &index)
{
	return new IndexedDomainMatchOnly(this);
}

RelationLiteral::RelationLiteral(RelationLiteral &r) : type_(r.type_), a_(r.a_->clone()), b_(r.b_->clone())
{
}

Literal* RelationLiteral::clone()
{
	return new RelationLiteral(*this);
}

NS_OUTPUT::Object *RelationLiteral::convert()
{
	assert(false);
}

RelationLiteral::~RelationLiteral()
{
	if(a_)
		delete a_;
	if(b_)
		delete b_;
}

