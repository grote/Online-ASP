#include "dependencygraph.h"
#include "literaldependencygraph.h"
#include "predicateliteral.h"
#include "assignmentliteral.h"
#include "relationliteral.h"
#include "grounder.h"
#include "constant.h"
#include "term.h"
#include "node.h"
#include "rangeterm.h"
#include "rangeliteral.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "value.h"
#include "multipleargsterm.h"
#include "output.h"
#include "evaluator.h"

using namespace NS_GRINGO;
		
PredicateLiteral::PredicateLiteral(Grounder *g, std::string *id, TermVector *variables) : Literal(), predNode_(0), id_(id), variables_(variables), uid_(g->createPred(id, variables->size())), matchValues_(variables ? variables->size() : 0)
{
}

PredicateLiteral::PredicateLiteral(const PredicateLiteral &p) : predNode_(p.predNode_), id_(p.id_), matchValues_(p.matchValues_.size()), values_(p.values_.size())
{
        if(p.variables_)
        {
                variables_ = new TermVector();
                for(TermVector::iterator it = p.variables_->begin(); it != p.variables_->end(); it++)
                                variables_->push_back((*it)->clone());
        }
        else
                variables_ = 0;
}

bool PredicateLiteral::checkO(LiteralVector &unsolved)
{
	return true;
}

Node *PredicateLiteral::createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo)
{
	predNode_ = dg->createPredicateNode(this);
	switch(todo)
	{
		case ADD_BODY_DEP:
			prev->addDependency(predNode_, getNeg());
			break;
		case ADD_HEAD_DEP:
			predNode_->addDependency(prev);
			break;
		case ADD_NOTHING:
			break;
	}
	return predNode_;
}

void PredicateLiteral::createNode(LDGBuilder *dg, bool head)
{
	VarSet needed, provided;
	if(head || getNeg() || !predNode_->complete())
	{
		if(variables_)
			for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
				(*it)->getVars(needed);
	}
	else
	{
		if(variables_)
			for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
				if((*it)->isComplex())
					(*it)->getVars(needed);
				else
					(*it)->getVars(provided);
	}
	dg->createNode(this, head, needed, provided);
}

void PredicateLiteral::print(std::ostream &out)
{
	if(getNeg())
		out << "not ";
	out << *id_;
	if(getArity() > 0)
	{
		out << "(";
		bool comma = false;
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
		{
			if(comma)
				out << ", ";
			else
				comma = true;
			out << *it;
		}
		out << ")";
	}
}

ValueVector &PredicateLiteral::getValues()
{
	return values_;
}

void PredicateLiteral::reset()
{
	if(!getNeg())
		predNode_->reset();
}

void PredicateLiteral::finish()
{
	if(!getNeg())
		predNode_->finish();
}

void PredicateLiteral::evaluate()
{
	if(!getNeg())
		predNode_->evaluate();
}

void PredicateLiteral::ground(Grounder *g)
{
}

bool PredicateLiteral::isFact()
{
	// a precondition for this method is that the predicate is not false!
	if(predNode_->solved())
		return true;
	values_.clear();
	for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
		values_.push_back((*it)->getValue());
	if(getNeg())
	{
		assert(!predNode_->isFact(values_));
		if(predNode_->complete())
			return !predNode_->inDomain(values_);
		else
			return false;
	}
	else
		return predNode_->isFact(values_);
}

bool PredicateLiteral::solved()
{
	return predNode_->solved();
}

bool PredicateLiteral::match(Grounder *g)
{
	assert(!predNode_->solved() || predNode_->complete());
	// incomplete prediactes match everything
	// except if they contain some values that are facts!!!
	if(!predNode_->complete())
	{
		if(getNeg() && predNode_->hasFacts())
		{
			for(int i = 0; i < (int)variables_->size(); i++)
				matchValues_[i] = (*variables_)[i]->getValue();
			if(predNode_->isFact(matchValues_))
				return false;
		}
		else
			return true;
	}
	// check if current assignment is satisfied wrt the domain
	bool match = true;
	if(variables_)
	{
		for(int i = 0; i < (int)variables_->size(); i++)
			matchValues_[i] = (*variables_)[i]->getValue();
	}
	match = predNode_->inDomain(matchValues_);
	if(getNeg())
	{
		if(match && (predNode_->solved() || predNode_->isFact(matchValues_)))
			return false;
	}
	else
	{
		if(!match)
			return false;
	}
	return true;
}

NS_OUTPUT::Object * PredicateLiteral::convert(ValueVector &values)
{
	return new NS_OUTPUT::Atom(getNeg(), predNode_, uid_, values);
}

NS_OUTPUT::Object *PredicateLiteral::convert()
{
	// the method isFact or ground has to be called before this method
	/*
	values_.clear();
	for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
		values_.push_back((*it)->getValue());
	*/
	return convert(values_);
}

void PredicateLiteral::addDomain(ValueVector &values)
{
	predNode_->addDomain(values);
}

IndexedDomain *PredicateLiteral::createIndexedDomain(VarSet &index)
{
	if(!predNode_->complete() || getNeg())
		return new IndexedDomainMatchOnly(this);
	if(variables_)
	{
		ConstantVector param(variables_->size());
		ConstantVector::iterator paramIt = param.begin();
		VarSet free;
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++, paramIt++)
		{
			assert(dynamic_cast<Constant*>(*it));
			(*paramIt) = static_cast<Constant*>(*it);
			int uid = (*paramIt)->getUID();
			if(uid > 0 && index.find(uid) == index.end())
				free.insert(uid);
			
		}
		if(free.size() > 0)
		{
			if(free.size() == param.size())
				return new IndexedDomainFullMatch(predNode_->getDomain(), param);
			else
				return new IndexedDomainDefault(predNode_->getDomain(), index, param);
		}
		else
			return new IndexedDomainMatchOnly(this);
	}
	else
		return new IndexedDomainMatchOnly(this);
}

Literal* PredicateLiteral::clone() const
{
	return new PredicateLiteral(*this);
}

PredicateLiteral::~PredicateLiteral()
{
	if(variables_)
	{
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
		{
			delete *it;
		}
		delete variables_;
	}
}

void PredicateLiteral::preprocess(Grounder *g, Expandable *e)
{
	if(variables_)
	{
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
			(*it)->preprocess(this, *it, g, e);
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
			if((*it)->isComplex())
			{
				std::string *var = g->createUniqueVar();
				e->appendLiteral(new AssignmentLiteral(new Constant(Constant::VAR, g, var), *it), Expandable::COMPLEXTERM);
				*it = new Constant(Constant::VAR, g, var);
			}
	}
	if((*id_)[0] == '-')
		g->addTrueNegation(id_, variables_ ? variables_->size() : 0);
}

TermVector *PredicateLiteral::getArgs()
{
	return variables_;
}


int PredicateLiteral::getUid()
{
	return uid_;
}

std::string *PredicateLiteral::getId()
{
	return id_;
}

int PredicateLiteral::getArity()
{
	return variables_->size();
}

double PredicateLiteral::heuristicValue()
{
	// TODO: thats too simple !!! i want something better
	if(!getNeg())
	{
		if(predNode_->complete())
		{
			if(variables_ && variables_->size() > 0)
				return pow(predNode_->getDomain().size(), 1.0 / variables_->size());
			else
				return 0;
		}
		else
			// the literal doenst help at all
			return DBL_MAX;
	}
	else if(predNode_->solved())
	{
		// the literal can be used to check if the assignment is valid
		return 0;
	}
	else
		return DBL_MAX;
}


void PredicateLiteral::getVars(VarSet &vars) const
{
	if(variables_)
		for(TermVector::const_iterator it = variables_->begin(); it != variables_->end(); it++)
			(*it)->getVars(vars);
}

