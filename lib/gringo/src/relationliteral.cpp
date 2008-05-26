#include "relationliteral.h"
#include "term.h"
#include "value.h"
#include "indexeddomain.h"
#include "literaldependencygraph.h"

using namespace NS_GRINGO;

RelationLiteral::RelationLiteral(RelationType type, Term *a, Term *b) : Literal(), type_(type), a_(a), b_(b)
{

}

Node *RelationLiteral::createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo)
{
	return 0;
}

void RelationLiteral::createNode(LDGBuilder *dg, bool head)
{
	assert(!head);
	VarSet needed, provided;
	a_->getVars(needed);
	if(b_)
		b_->getVars(needed);
	dg->createStaticNode(this, needed, provided);
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

void RelationLiteral::getVars(VarSet &vars)
{
	a_->getVars(vars);
	b_->getVars(vars);
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

void RelationLiteral::preprocess(Grounder *g, Expandable *e)
{
	a_->preprocess(this, a_, g, e);
	b_->preprocess(this, b_, g, e);
}

double RelationLiteral::heuristicValue()
{
	// match it as soon as possible
	return 0;
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

