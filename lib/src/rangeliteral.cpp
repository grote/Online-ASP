#include "rangeliteral.h"
#include "constant.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "grounder.h"

using namespace NS_GRINGO;

RangeLiteral::RangeLiteral(Constant *var, int lower, int upper) : Literal(), var_(var), lower_(lower), upper_(upper)
{
}

Node *RangeLiteral::createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo)
{
	return 0;
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

void RangeLiteral::normalize(Grounder *g, Expandable *r)
{
}

void RangeLiteral::getVars(VarSet &vars, VarsType type)
{
	switch(type)
	{
		case VARS_PROVIDED:
		case VARS_GLOBAL:
		case VARS_ALL:
			var_->getVars(vars);
			break;
		case VARS_NEEDED:
			break;
	}
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
	return (lower_ <= (int)var_->getValue() && (int)var_->getValue() <= upper_);
}

namespace
{
	class IndexedDomainRange : public IndexedDomain
	{
	public:
		IndexedDomainRange(int var, int lower, int upper);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainRange();
	protected:
		int var_;
		int current_;
		int lower_;
		int upper_;
	};

	IndexedDomainRange::IndexedDomainRange(int var, int lower, int upper) : IndexedDomain(), var_(var), current_(0), lower_(lower), upper_(upper)
	{
	}

	void IndexedDomainRange::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		if(lower_ <= upper_)
		{
			current_ = lower_;
			g->g_->setValue(var_, Value(current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnFirstMatch;
	}

	void IndexedDomainRange::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		if(current_ < upper_)
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
	// nothing todo
}

NS_OUTPUT::Object *RangeLiteral::convert()
{
	assert(false);
}

RangeLiteral::RangeLiteral(RangeLiteral &r) : var_((Constant*)r.var_->clone()), lower_(r.lower_), upper_(r.upper_)
{
}

Literal* RangeLiteral::clone()
{
	return new RangeLiteral(*this);
}

RangeLiteral::~RangeLiteral()
{
	if(var_)
		delete var_;
}

