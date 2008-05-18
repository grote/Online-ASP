#include "rangeliteral.h"
#include "constant.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "grounder.h"

using namespace NS_GRINGO;

RangeLiteral::RangeLiteral(Constant *var, Term *lower, Term *upper) : Literal(), var_(var), lower_(lower), upper_(upper)
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
		{
			// even though this cannot happen
			VarSet needed;
			lower_->getVars(needed);
			upper_->getVars(needed);
			int uid = var_->getUID();
			if(needed.find(uid) == needed.end())
				vars.insert(uid);
			break;
		}
		case VARS_GLOBAL:
		case VARS_ALL:
			lower_->getVars(vars);
			upper_->getVars(vars);
			var_->getVars(vars);
			break;
		case VARS_NEEDED:
			lower_->getVars(vars);
			upper_->getVars(vars);
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
	return ((int)lower_->getValue() <= (int)var_->getValue() && (int)var_->getValue() <= (int)upper_->getValue());
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
		return new IndexedDomainRange(var_->getUID(), lower_->getValue(), upper_->getValue());
	else
		return new IndexedDomainMatchOnly(this);
}

void RangeLiteral::preprocess(Grounder *g, Expandable *e)
{
	lower_->preprocess(lower_, g, e);
	upper_->preprocess(upper_, g, e);
}

NS_OUTPUT::Object *RangeLiteral::convert()
{
	assert(false);
}

RangeLiteral::RangeLiteral(RangeLiteral &r) : var_((Constant*)r.var_->clone()), lower_(r.lower_->clone()), upper_(r.upper_->clone())
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
	if(lower_)
		delete lower_;
	if(upper_)
		delete upper_;
}

