#include "rangeliteral.h"
#include "constant.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "grounder.h"
#include "literaldependencygraph.h"

using namespace NS_GRINGO;

RangeLiteral::RangeLiteral(Constant *var, Term *lower, Term *upper) : Literal(), var_(var), lower_(lower), upper_(upper)
{
}

Node *RangeLiteral::createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo)
{
	return 0;
}

void RangeLiteral::createNode(LDGBuilder *dg, bool head)
{
	assert(!head);
	VarSet needed, provided;
	upper_->getVars(needed);
	lower_->getVars(needed);
	var_->getVars(provided);
	dg->createNode(this, head, needed, provided);
}

void RangeLiteral::print(std::ostream &out)
{
	out << var_ << " = [" << lower_ << ", " << upper_ << "]";
}

bool RangeLiteral::isFact()
{
	return true;
}

bool RangeLiteral::solved()
{
	return true;
}

void RangeLiteral::getVars(VarSet &vars) const
{
	lower_->getVars(vars);
	upper_->getVars(vars);
	var_->getVars(vars);
}

bool RangeLiteral::checkO(LiteralVector &unsolved)
{
	return true;
}

void RangeLiteral::reset()
{
	assert(false);
}

void RangeLiteral::finish()
{
	assert(false);
}

bool RangeLiteral::match(Grounder *g)
{
	return ((int)lower_->getValue() <= (int)var_->getValue() && (int)var_->getValue() <= (int)upper_->getValue());
}

namespace
{
	class IndexedDomainRange : public IndexedDomain
	{
	public:
		IndexedDomainRange(int var, Term *lower, Term *upper);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainRange();
	protected:
		int var_;
		int current_;
		int end_;
		Term *lower_;
		Term *upper_;
	};

	IndexedDomainRange::IndexedDomainRange(int var, Term *lower, Term *upper) : IndexedDomain(), var_(var), current_(0), end_(0), lower_(lower), upper_(upper)
	{
	}

	void IndexedDomainRange::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		end_     = upper_->getValue();
		current_ = lower_->getValue();
		if(current_ <= end_)
		{
			g->g_->setValue(var_, Value(current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnFirstMatch;
	}

	void IndexedDomainRange::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		if(current_ < end_)
		{
			current_++;
			g->g_->setValue(var_, Value(current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnNextMatch;
	}

	IndexedDomainRange::~IndexedDomainRange()
	{
	}

}

IndexedDomain *RangeLiteral::createIndexedDomain(VarSet &index)
{
	if(index.find(var_->getUID()) == index.end())
		return new IndexedDomainRange(var_->getUID(), lower_, upper_);
	else
		return new IndexedDomainMatchOnly(this);
}

void RangeLiteral::preprocess(Grounder *g, Expandable *e)
{
	lower_->preprocess(this, lower_, g, e);
	upper_->preprocess(this, upper_, g, e);
}

NS_OUTPUT::Object *RangeLiteral::convert()
{
	assert(false);
}

RangeLiteral::RangeLiteral(const RangeLiteral &r) : var_((Constant*)r.var_->clone()), lower_(r.lower_->clone()), upper_(r.upper_->clone())
{
}

Literal* RangeLiteral::clone() const
{
	return new RangeLiteral(*this);
}

double RangeLiteral::heuristicValue()
{
	// hard to estimate cause in general the domainsize is hard to predict
	return DBL_MAX;
}

RangeLiteral::~RangeLiteral()
{
	if(var_)
		delete var_;
	if(lower_)
		delete lower_;
	if(upper_)
		delete upper_;
}

