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

void ConditionalLiteral::getVars(VarSet &vars) const
{
	pred_->getVars(vars);
	if(conditionals_)
		for(LiteralVector::const_iterator it = conditionals_->begin(); it != conditionals_->end(); it++)
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

bool ConditionalLiteral::match(Grounder *g)
{
	assert(false);
}

void ConditionalLiteral::start()
{
	current_ = 0;
}

void ConditionalLiteral::remove()
{
	std::swap(values_[current_], values_.back());
	values_.pop_back();
	if(weight_)
	{
		std::swap(weights_[current_], weights_.back());
		weights_.pop_back();
	}
	current_--;
}

bool ConditionalLiteral::hasNext()
{
	return current_ < values_.size();
}

void ConditionalLiteral::next()
{
	current_++;
}

bool ConditionalLiteral::isFact()
{
	return pred_->isFact(values_[current_]);
}

bool ConditionalLiteral::match()
{
	return pred_->match(values_[current_]);
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

int ConditionalLiteral::count()
{
	return values_.size();
}

void ConditionalLiteral::reset()
{
	pred_->reset();
}

void ConditionalLiteral::finish()
{
	pred_->finish();
}

bool ConditionalLiteral::solved()
{
	return pred_->solved();
}

void ConditionalLiteral::ground(Grounder *g)
{
	weights_.clear();
	values_.clear();
	if(conditionals_)
	{
		if(!grounder_)
		{
			//std::cerr << "creating grounder for: " << this << std::endl;
			grounder_ = new DLVGrounder(g, this, conditionals_->size(), dg_, dg_->getGlobalVars());
		}
		
		grounder_->ground();
	}
	else
	{
		grounded(g);
	}
}

void ConditionalLiteral::grounded(Grounder *g)
{
	ValueVector values;
	values.reserve(pred_->getArgs()->size());
	for(TermVector::iterator it = pred_->getArgs()->begin(); it != pred_->getArgs()->end(); it++)
		values.push_back((*it)->getValue());
	if(weight_)
		weights_.push_back(weight_->getValue());
	values_.push_back(ValueVector());

	std::swap(values_.back(), values);
}

bool ConditionalLiteral::isEmpty()
{
	return values_.size() == 0;
}

IndexedDomain *ConditionalLiteral::createIndexedDomain(VarSet &index)
{
	assert(false);
}

ConditionalLiteral::ConditionalLiteral(const ConditionalLiteral &p) : pred_(p.clone_ ? static_cast<PredicateLiteral*>(p.pred_->clone()) : p.pred_), weight_(p.weight_ ? p.weight_->clone() : 0), grounder_(0), dg_(0), clone_(true)
{
	if(p.conditionals_)
	{
		conditionals_ = new LiteralVector();
		for(LiteralVector::const_iterator it = p.conditionals_->begin(); it != p.conditionals_->end(); it++)
			conditionals_->push_back((*it)->clone());
	}
	else
		conditionals_ = 0;
}

Literal* ConditionalLiteral::clone() const
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
	DisjunctiveConditionalLiteralExpander cle(this, a, e);
	pred_->preprocess(g, &cle);
	if(conditionals_)
	{
		for(size_t i = 0; i < conditionals_->size(); i++)
			(*conditionals_)[i]->preprocess(g, this);
	}
}

void ConditionalLiteral::preprocess(Grounder *g, Expandable *e)
{
	ConditionalLiteralExpander cle(this, e, conditionals_);
	pred_->preprocess(g, &cle);
	if(conditionals_)
	{
		for(size_t i = 0; i < conditionals_->size(); i++)
			(*conditionals_)[i]->preprocess(g, this);
	}
	if(weight_)
		weight_->preprocess(this, weight_, g, e);
}

double ConditionalLiteral::heuristicValue()
{
	assert(false);
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

