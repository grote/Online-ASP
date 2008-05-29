#include "conditionalliteral.h"
#include "predicateliteral.h"
#include "grounder.h"
#include "term.h"
#include "node.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "value.h"
#include "literaldependencygraph.h"
#include "aggregateliteral.h"

using namespace NS_GRINGO;
		
ConditionalLiteral::ConditionalLiteral(PredicateLiteral *pred, LiteralVector *conditionals) : Literal(), pred_(pred), conditionals_(conditionals), weight_(0), grounder_(0), dg_(0), clone_(true)
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

void ConditionalLiteral::appendLiteral(Literal *l, ExpansionType type)
{
	if(!conditionals_)
		conditionals_ = new LiteralVector();
	conditionals_->push_back(l);
}

void ConditionalLiteral::getVars(VarSet &vars)
{
	pred_->getVars(vars);
	if(conditionals_)
		for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
			(*it)->getVars(vars);
	if(weight_)
		weight_->getVars(vars);
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

bool ConditionalLiteral::check(VarVector &free)
{
	if(dg_)
		delete dg_;
	dg_ = new LDG();
	LDGBuilder dgb(dg_);
	dgb.addHead(pred_);
	if(conditionals_)
		for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
			dgb.addToBody(*it);
	dgb.create();
	dg_->check(free);
	
	return free.size() == 0;
}

void ConditionalLiteral::createNode(LDGBuilder *dgb, bool head)
{
	if(dg_)
		delete dg_;
	dg_ = new LDG();
	LDGBuilder *subDg = new LDGBuilder(dg_);
	subDg->addHead(pred_);
	if(conditionals_)
		for(LiteralVector::iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
			subDg->addToBody(*it);
	dgb->addGraph(subDg);
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
			//std::cerr << "creating grounder for: " << this << std::endl;
			grounder_ = new DLVGrounder(g, this, *conditionals_, dg_, dg_->getGlobalVars());
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

ConditionalLiteral::ConditionalLiteral(ConditionalLiteral &p) : pred_(p.clone_ ? static_cast<PredicateLiteral*>(p.pred_->clone()) : p.pred_), weight_(p.weight_ ? p.weight_->clone() : 0), grounder_(0), dg_(0), clone_(true)
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
	if(dg_)
		delete dg_;
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
		ConditionalLiteralExpander(ConditionalLiteral *l, Expandable *e, LiteralVector *c) : l_(l), e_(e), c_(c)
		{
		}
		void appendLiteral(Literal *l, ExpansionType type)
		{
			if(type == MATERM)
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
				e_->appendLiteral(new ConditionalLiteral((PredicateLiteral*)l, c), type);
			}
			else
			{
				l_->appendLiteral(l, type);
			}
		}
	protected:
		ConditionalLiteral *l_;
		Expandable         *e_;
		LiteralVector      *c_;
	};

	class DisjunctiveConditionalLiteralExpander : public Expandable
	{
	public:
		DisjunctiveConditionalLiteralExpander(ConditionalLiteral *l, AggregateLiteral *a, Expandable *e) : l_(l), a_(a), e_(e)
		{
		}
		void appendLiteral(Literal *l, ExpansionType type)
		{
			switch(type)
			{
				case MATERM:
					l_->clonePredicate(false);
					e_->appendLiteral(a_->clone(), type);
					l_->clonePredicate(true);
					l_->setPredicate(static_cast<PredicateLiteral*>(l));
					break;
				case RANGETERM:
					e_->appendLiteral(l, type);
					break;
				default:
					l_->appendLiteral(l, type);
					break;
			}
		}
	protected:
		ConditionalLiteral *l_;
		AggregateLiteral   *a_;
		Expandable         *e_;
	};
}

void ConditionalLiteral::clonePredicate(bool clone)
{
	clone_ = clone;
}

void ConditionalLiteral::setPredicate(PredicateLiteral* pred)
{
	pred_ = pred;
}

NS_OUTPUT::Object *ConditionalLiteral::convert()
{
	return pred_->convert(getValues());
}

void ConditionalLiteral::preprocessDisjunction(Grounder *g, AggregateLiteral *a, Expandable *e)
{
	assert(!weight_);
	if(conditionals_)
	{
		for(size_t i = 0; i < conditionals_->size(); i++)
			(*conditionals_)[i]->preprocess(g, this);
	}
	DisjunctiveConditionalLiteralExpander cle(this, a, e);
	pred_->preprocess(g, &cle);
}

void ConditionalLiteral::preprocess(Grounder *g, Expandable *e)
{
	if(conditionals_)
	{
		for(size_t i = 0; i < conditionals_->size(); i++)
			(*conditionals_)[i]->preprocess(g, this);
	}
	ConditionalLiteralExpander cle(this, e, conditionals_);
	pred_->preprocess(g, &cle);
	if(weight_)
		weight_->preprocess(this, weight_, g, e);
}

double ConditionalLiteral::heuristicValue()
{
	return DBL_MAX;
}

int ConditionalLiteral::getUid()
{
	return pred_->getUid();
}

bool ConditionalLiteral::hasConditionals()
{
	return conditionals_ ? conditionals_->size() > 0 : false;
}

PredicateLiteral *ConditionalLiteral::toPredicateLiteral()
{
	PredicateLiteral *pred = pred_;
	pred_ = 0;
	delete this;
	return pred;
}

