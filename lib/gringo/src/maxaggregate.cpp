#include "maxaggregate.h"
#include "conditionalliteral.h"
#include "term.h"
#include "value.h"
#include "output.h"
#include "dlvgrounder.h"
#include "grounder.h"
#include "indexeddomain.h"
#include "constant.h"

using namespace NS_GRINGO;

MaxAggregate::MaxAggregate(ConditionalLiteralVector *literals) : AggregateLiteral(literals)
{
}

void MaxAggregate::match(Grounder *g, int &lower, int &upper, int &fixed)
{
	fact_ = true;
	lower = INT_MAX;
	upper = INT_MIN;
	fixed = INT_MIN;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(!p->match())
			{
				p->remove();
				continue;
			}
			int weight = p->getWeight();
			if(p->isFact())
				fixed = std::max(fixed, weight);
			else
			{
				fact_ = false;
				lower = std::min(lower, weight);
				upper = std::max(upper, weight);
			}
		}
	}

	if(lower == INT_MAX || fixed > lower)
		lower = fixed;
	upper = std::max(upper, fixed);
}

void MaxAggregate::print(std::ostream &out)
{
	if(lower_)
		out << lower_ << " ";
	out << "max [";
	bool comma = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		out << *it;
	}
	out << "]";
	if(upper_)
		out << " " << upper_;
}

namespace
{
	class IndexedDomainMaxAggregate : public IndexedDomain
	{
	protected:
		typedef std::set<int> IntSet;
	public:
		IndexedDomainMaxAggregate(MaxAggregate *l, int var);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainMaxAggregate();
	protected:
		MaxAggregate *l_;
		int var_;
		IntSet set_;
		IntSet::iterator current_;
	};

	IndexedDomainMaxAggregate::IndexedDomainMaxAggregate(MaxAggregate *l, int var) : l_(l), var_(var)
	{
		
	}

	void IndexedDomainMaxAggregate::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		int lower, upper, fixed;
		l_->match(g->g_, lower, upper, fixed);
		set_.clear();
		ConditionalLiteralVector *l = l_->getLiterals();
		for(ConditionalLiteralVector::iterator it = l->begin(); it != l->end(); it++)
		{
			ConditionalLiteral *p = *it;
			for(p->start(); p->hasNext(); p->next())
			{
				// insert all possible bindings for the bound
				int weight = p->getWeight();
				if(weight > fixed)
					set_.insert(weight);
			}
		}
		set_.insert(fixed);

		current_ = set_.begin();
		g->g_->setValue(var_, Value(*current_), binder);
		status = SuccessfulMatch;
	}

	void IndexedDomainMaxAggregate::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		current_++;
		if(current_ != set_.end())
		{
			g->g_->setValue(var_, Value(*current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnNextMatch;
	}

	IndexedDomainMaxAggregate::~IndexedDomainMaxAggregate()
	{
	}
}

IndexedDomain *MaxAggregate::createIndexedDomain(VarSet &index)
{
	if(equal_)
	{
		if(index.find(equal_->getUID()) != index.end())
		{
			return new IndexedDomainMatchOnly(this);
		}
		else
		{
			return new IndexedDomainMaxAggregate(this, equal_->getUID());
		}
	}
	else
		return new IndexedDomainMatchOnly(this);
}

MaxAggregate::MaxAggregate(const MaxAggregate &a) : AggregateLiteral(a)
{
}

NS_OUTPUT::Object *MaxAggregate::convert()
{
	NS_OUTPUT::ObjectVector lits;
	IntVector weights;
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
	{
		ConditionalLiteral *p = *it;
		for(p->start(); p->hasNext(); p->next())
		{
			lits.push_back(p->convert());
			weights.push_back(p->getWeight());
		}
	}
	NS_OUTPUT::Aggregate *a;
	if(lower_ && upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::MIN, lower_->getValue(), lits, weights, upper_->getValue());
	else if(lower_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::MIN, lower_->getValue(), lits, weights);
	else if(upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::MIN, lits, weights, upper_->getValue());
	else
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::MIN, lits, weights);

	return a;
}

Literal *MaxAggregate::clone() const
{
	return new MaxAggregate(*this);
}

MaxAggregate::~MaxAggregate()
{
}

