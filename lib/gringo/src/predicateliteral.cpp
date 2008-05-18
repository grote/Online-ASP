#include "dependencygraph.h"
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
		
PredicateLiteral::PredicateLiteral(std::string *id, TermVector *variables) : Literal(), predNode_(0), id_(id), variables_(variables), matchValues_(variables ? variables->size() : 0)
{
}

void PredicateLiteral::normalize(Grounder *g, Expandable *r)
{
	// since negative or incomplete literals dont provide vars there is no need to normalize them
	if(!getNeg() && predNode_->complete())
	{
		assert(r);
		if(variables_)
		{
			for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
			{
				if((*it)->isComplex())
				{
					std::string var = g->createUniqueVar();
					// its better to use an assignmentliteral than a relationliteral here
					// q(X,Y), p(X,X+Y) could be transformed into
					// q(X,Y), I=X+Y, p(X,I)
					// here only p has to be matched
					//r->appendLiteral(new RelationLiteral(RelationLiteral::EQ, new Constant(Constant::VAR, g, new std::string(var)), *it));
					r->appendLiteral(new AssignmentLiteral(new Constant(Constant::VAR, g, new std::string(var)), *it));
					*it = new Constant(Constant::VAR, g, new std::string(var));
				}
			}
		}
	}
}

void PredicateLiteral::getVars(VarSet &vars, VarsType type)
{
	switch(type)
	{
		case VARS_ALL:
			getVars(vars);
			break;
		case VARS_PROVIDED:
			if(!getNeg() && predNode_->complete())
				getVars(vars);
			break;
		case VARS_GLOBAL:
			getVars(vars);
			break;
		case VARS_NEEDED:
			if(getNeg() || !predNode_->complete())
				getVars(vars);
			break;
	}
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
	// incomplete prediactes match everything
	if(!predNode_->complete())
		return true;
	// check if current assignment is satisfied wrt the domain
	bool match = true;
	if(variables_)
	{
		for(int i = 0; i < (int)variables_->size(); i++)
			matchValues_[i] = (*variables_)[i]->getValue();
		match = getDomain().find(matchValues_) != getDomain().end();
	}
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
	return new NS_OUTPUT::Atom(getNeg(), predNode_, values);
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
		bool freeVars = false;
		ConstantVector param(variables_->size());
		ConstantVector::iterator paramIt = param.begin();
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++, paramIt++)
		{
			assert(dynamic_cast<Constant*>(*it));
			(*paramIt) = static_cast<Constant*>(*it);
			if(!freeVars && index.find((*paramIt)->getUID()) == index.end())
				freeVars = true;
			
		}
		if(freeVars)
			return new IndexedDomainDefault(getDomain(), index, param);
		else
			return new IndexedDomainMatchOnly(this);
	}
	else
		return new IndexedDomainMatchOnly(this);
}

PredicateLiteral::PredicateLiteral(PredicateLiteral &p, Term *t) : predNode_(p.predNode_), id_(new std::string(*p.id_)), matchValues_(p.matchValues_.size()), values_(p.values_.size())
{
	if(p.variables_)
	{
		variables_ = new TermVector();
		for(TermVector::iterator it = p.variables_->begin(); it != p.variables_->end(); it++)
		{
			if(*it == t)
				variables_->push_back(*it);
			else
				variables_->push_back((*it)->clone());
		}
	}
	else
		variables_ = 0;
}

Literal* PredicateLiteral::clone()
{
	return new PredicateLiteral(*this);
}

PredicateLiteral::~PredicateLiteral()
{
	if(id_)
		delete id_;
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
		// remove multiple args terms
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
		{
			MultipleArgsTerm *t = dynamic_cast<MultipleArgsTerm*>(*it);
			if(t)
			{
				Term *f = t->pop();
				if(t->argc() == 0)
				{
					delete t;
					*it = f;
				}
				else
				{
					// clone everything except t
					e->appendLiteral(new PredicateLiteral(*this, t), true);
					// replace t by extracted literal
					*it = f;
				}
			}
		}
		// remove range terms
		if(variables_)
		{
			for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
			{
				(*it)->preprocess(*it, g, e);
			}
		}
	}
	if((*id_)[0] == '-')
		g->addTrueNegation(*id_, variables_ ? variables_->size() : 0);
}

TermVector *PredicateLiteral::getArgs()
{
	return variables_;
}


int PredicateLiteral::getUid()
{
	return predNode_->getUid();
}

std::string *PredicateLiteral::getId()
{
	return id_;
}

int PredicateLiteral::getArity()
{
	return variables_->size();
}

void PredicateLiteral::getVars(VarSet &vars)
{
	if(variables_)
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
			(*it)->getVars(vars);
}

ValueVectorSet &PredicateLiteral::getDomain()
{
	return predNode_->getDomain();
}

