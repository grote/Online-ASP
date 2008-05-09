#include "conditionalliteral.h"
#include "predicateliteral.h"
#include "grounder.h"
#include "term.h"
#include "node.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "value.h"

using namespace NS_GRINGO;
		
ConditionalLiteral::ConditionalLiteral(PredicateLiteral *pred, LiteralVector *conditionals) : Literal(), pred_(pred), conditionals_(conditionals), weight_(0), grounder_(0)
{
	Literal::setNeg(pred_->getNeg());
}

void ConditionalLiteral::setWeight(Term *w)
{
	weight_ = w;
}

void ConditionalLiteral::setNeg(bool neg)
{
	pred_->setNeg(neg);
	Literal::setNeg(neg);
}

void ConditionalLiteral::appendLiteral(Literal *l)
{
	if(!conditionals_)
		conditionals_ = new LiteralVector();
	conditionals_->push_back(l);
}

void ConditionalLiteral::normalize(Grounder *g, Expandable *e)
{
	if(conditionals_)
	{
		size_t l = conditionals_->size();
		for(size_t i = 0; i < l; i++)
			(*conditionals_)[i]->normalize(g, this);
	}
	// only remove the rangeterms but not normalize cause its possible that pred_ is already complete
	pred_->removeRangeTerms(g, this);
}

void ConditionalLiteral::getVars(VarSet &vars, VarsType type)
{
	switch(type)
	{
		case VARS_ALL:
			pred_->getVars(vars);
			if(conditionals_)
			{
				for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
					(*it)->getVars(vars, VARS_ALL);
			}
			if(weight_)
				weight_->getVars(vars);
			break;
		case VARS_PROVIDED:
			assert(false);
			break;
		case VARS_GLOBAL:
			if(conditionals_ != 0)
			{
				VarSet t1, t2;
				// get vars needed by the literal
				pred_->getVars(t1);
				for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
				{
					// conditionals may both provide and need literals
					// i.e. a(X,Y) : p(X, I) : I == Y + 1 = (X+2)
					// here X and I are provided vars but Y is needed
					(*it)->getVars(t1, VARS_NEEDED);
					(*it)->getVars(t2, VARS_PROVIDED);
				}
				if(weight_)
					weight_->getVars(t1);
				// all vars not provided by conditionals are GLOBAL
				for(VarSet::iterator it = t1.begin(); it != t1.end(); it++)
					if(t2.find(*it) == t2.end())
						vars.insert(*it);
			}
			else
			{
				pred_->getVars(vars);
				if(weight_)
					weight_->getVars(vars);
			}
			break;
		case VARS_NEEDED:
			// aggregate literals will call with VARS_GLOBAL
			assert(false);
			break;
	}
}

bool ConditionalLiteral::checkO(LiteralVector &unsolved)
{
	if(conditionals_ == 0)
		return true;
	for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
	{
		if(!(*it)->solved())
			unsolved.push_back(*it);
	}
	return unsolved.size() == 0;
}

Node *ConditionalLiteral::createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo)
{
	Node *n = pred_->createNode(dg, prev, todo);
	if(conditionals_)
	{
		for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
			(*it)->createNode(dg, prev, ADD_BODY_DEP);
	}
	return n;
}

void ConditionalLiteral::print(std::ostream &out)
{
	out << pred_;
	if(conditionals_)
		for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
			out << " : " << *it;
	if(weight_)
		out << " = " << weight_;
}

void ConditionalLiteral::start()
{
	current_ = 0;
}

bool ConditionalLiteral::hasNext()
{
	return current_ < values_.size();
}

void ConditionalLiteral::next()
{
	current_++;
}

ValueVector &ConditionalLiteral::getValues()
{
	return values_[current_];
}

bool ConditionalLiteral::hasWeight()
{
	return weight_ != 0;
}

int ConditionalLiteral::getWeight()
{
	return weights_[current_];
}

void ConditionalLiteral::reset()
{
	pred_->reset();
}

void ConditionalLiteral::finish()
{
	pred_->finish();
}

bool ConditionalLiteral::isFact()
{
	// not used yet
	assert(false);
}

bool ConditionalLiteral::solved()
{
	// not used yet
	assert(false);
}

bool ConditionalLiteral::match(Grounder *g)
{
	return pred_->match(g);
}

void ConditionalLiteral::ground(Grounder *g)
{
	if(conditionals_)
	{
		if(!grounder_)
		{
			VarSet all, glob, index;
			getVars(all, VARS_ALL);
			for(VarSet::iterator it = all.begin(); it != all.end(); it++)
			{
				if(g->getBinder(*it) != -1)
					index.insert(*it);
				else
					glob.insert(*it);
			}
			grounder_ = new DLVGrounder(g, this, *conditionals_, glob, glob, index);
		}
		
		weights_.clear();
		values_.clear();
		grounder_->ground();
	}
	else
	{
		values_.resize(1);
		values_.back().clear();
		if(weight_)
			weights_.resize(1);
		cacheVariables();
	}
}

void ConditionalLiteral::cacheVariables()
{
	values_.back().resize(pred_->getArgs()->size());
	ValueVector::iterator valIt = values_.back().begin();
	for(TermVector::iterator it = pred_->getArgs()->begin(); it != pred_->getArgs()->end(); it++, valIt++)
		*valIt = (*it)->getValue();
	if(weight_)
		weights_.back() = weight_->getValue();
}

void ConditionalLiteral::grounded(Grounder *g)
{
	values_.push_back(ValueVector());
	if(weight_)
		weights_.push_back(0);
	cacheVariables();
}

bool ConditionalLiteral::isEmpty()
{
	return values_.size() == 0;
}

IndexedDomain *ConditionalLiteral::createIndexedDomain(VarSet &index)
{
	return new IndexedDomainMatchOnly(this);
}

ConditionalLiteral::ConditionalLiteral(ConditionalLiteral &p) : pred_((PredicateLiteral*)p.pred_->clone()), weight_(p.weight_ ? p.weight_->clone() : 0), grounder_(0)
{
	if(p.conditionals_)
	{
		conditionals_ = new LiteralVector();
		for(LiteralVector::iterator it = p.conditionals_->begin(); it != p.conditionals_->end(); it++)
			conditionals_->push_back((*it)->clone());
	}
	else
		conditionals_ = 0;
}

Literal* ConditionalLiteral::clone()
{
	return new ConditionalLiteral(*this);
}

ConditionalLiteral::~ConditionalLiteral()
{
	if(pred_)
		delete pred_;
	if(conditionals_)
	{
		for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
		{
			delete *it;
		}
		delete conditionals_;
	}
	if(weight_)
		delete weight_;
	if(grounder_)
		delete grounder_;
}

namespace
{
	class ConditionalLiteralExpander : public Expandable
	{
	public:
		ConditionalLiteralExpander(Expandable *e, LiteralVector *c) : e_(e), c_(c)
		{
		}
		void appendLiteral(Literal *l)
		{
			LiteralVector *c;
			if(c_)
			{
				c = new LiteralVector();
				for(LiteralVector::iterator it = c_->begin(); it != c_->end(); it++)
					c->push_back((*it)->clone());
			}
			else
				c = 0;
			e_->appendLiteral(new ConditionalLiteral((PredicateLiteral*)l, c));
		}
	protected:
		Expandable    *e_;
		LiteralVector *c_;
	};
}

NS_OUTPUT::Object *ConditionalLiteral::convert()
{
	return pred_->convert(getValues());
}

void ConditionalLiteral::preprocess(Grounder *g, Expandable *e)
{
	if(conditionals_)
	{
		for(size_t i = 0; i < conditionals_->size(); i++)
			(*conditionals_)[i]->preprocess(g, this);
	}
	ConditionalLiteralExpander cle(e, conditionals_);
	pred_->preprocess(g, &cle);
}

int ConditionalLiteral::getUid()
{
	return pred_->getUid();
}

