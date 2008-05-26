#include "assignmentliteral.h"
#include "term.h"
#include "constant.h"
#include "value.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "grounder.h"
#include "literaldependencygraph.h"

using namespace NS_GRINGO;

AssignmentLiteral::AssignmentLiteral(Constant *c, Term *t) : Literal(), c_(c), t_(t)
{

}

Node *AssignmentLiteral::createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo)
{
	return 0;
}

void AssignmentLiteral::createNode(LDGBuilder *dg, bool head)
{
	assert(!head);
	VarSet needed, provided;
	t_->getVars(needed);
	c_->getVars(provided);
	dg->createStaticNode(this, needed, provided);
}

void AssignmentLiteral::print(std::ostream &out)
{
	out << c_ << " = " << t_;
}

bool AssignmentLiteral::isFact()
{
	return true;
}

bool AssignmentLiteral::solved()
{
	return true;
}

void AssignmentLiteral::getVars(VarSet &vars)
{
	c_->getVars(vars);
	t_->getVars(vars);
}

bool AssignmentLiteral::checkO(LiteralVector &unsolved)
{
	return true;
}

void AssignmentLiteral::reset()
{
	assert(false);
}

void AssignmentLiteral::finish()
{
	assert(false);
}

bool AssignmentLiteral::match(Grounder *g)
{
	return c_->getValue() == t_->getValue();
}

void AssignmentLiteral::preprocess(Grounder *g, Expandable *e)
{
	t_->preprocess(this, t_, g, e);
}

namespace
{
	class IndexedDomainAssign : public IndexedDomain
	{
	public:
		IndexedDomainAssign(int var, Term *t);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainAssign();
	protected:
		int var_;
		Term *t_;
	};

	IndexedDomainAssign::IndexedDomainAssign(int var, Term *t) : var_(var), t_(t)
	{
	}
	
	void IndexedDomainAssign::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		g->g_->setValue(var_, t_->getValue(), binder);
		status = SuccessfulMatch;
	}

	void IndexedDomainAssign::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		status = FailureOnNextMatch;
	}

	IndexedDomainAssign::~IndexedDomainAssign()
	{
	}
}

IndexedDomain *AssignmentLiteral::createIndexedDomain(VarSet &index)
{
	if(index.find(c_->getUID()) == index.end())
		return new IndexedDomainAssign(c_->getUID(), t_);
	else
		return new IndexedDomainMatchOnly(this);
}

AssignmentLiteral::AssignmentLiteral(AssignmentLiteral &r) : c_((Constant*)r.c_->clone()), t_(r.t_->clone())
{
}

Literal* AssignmentLiteral::clone()
{
	return new AssignmentLiteral(*this);
}

NS_OUTPUT::Object *AssignmentLiteral::convert()
{
	assert(false);
}

AssignmentLiteral::~AssignmentLiteral()
{
	if(c_)
		delete c_;
	if(t_)
		delete t_;
}

