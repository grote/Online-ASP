#include "weightedstatement.h"
#include "dependencygraph.h"
#include "conditionalliteral.h"
#include "node.h"
#include "output.h"
#include "grounder.h"
#include "evaluator.h"

using namespace NS_GRINGO;
		
WeightedStatement::WeightedStatement(Type type, ConditionalLiteralVector *literals, bool setSemantics, int number) : type_(type), setSemantics_(setSemantics), number_(number)
{
	std::swap(literals_, *literals);
	delete literals;
}

void WeightedStatement::buildDepGraph(DependencyGraph *dg)
{
	// the order of optimize statements is important
	Node *prev = dg->createStatementNode(this, type_ != COMPUTE);
	for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
	{
		// since there cant be any cycles through these statements 
		// its ok to add only positive dependencies
		// even though negative dependencies wont hurt
		Node *p = (*it)->createNode(dg, prev, Literal::ADD_NOTHING);
		assert(p);
		prev->addDependency(p);
	}
}

void WeightedStatement::getVars(VarSet &vars) const
{
	// noone needs to know the vars in this statement
	//for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
	//	(*it)->getVars(vars, VARS_ALL);
}

bool WeightedStatement::checkO(LiteralVector &unsolved)
{
	for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
	{
		if(!(*it)->checkO(unsolved))
			return false;
	}
	return true;
}

bool WeightedStatement::check(VarVector &free)
{
	free.clear();
	for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
		(*it)->check(free);

	return free.size() == 0;
}

void WeightedStatement::preprocess(Grounder *g)
{
	for(size_t i = 0; i < literals_.size(); i++)
		literals_[i]->preprocess(g, this);
}

void WeightedStatement::appendLiteral(Literal *l, ExpansionType type)
{
	assert(dynamic_cast<ConditionalLiteral*>(l));
	literals_.push_back(static_cast<ConditionalLiteral*>(l));
}

void WeightedStatement::reset()
{
	// nothing todo
}

void WeightedStatement::finish()
{
	// nothing todo
}

void WeightedStatement::evaluate()
{
	// nothing todo
}

namespace
{
	struct Hash
	{
		Value::VectorHash hash;
		size_t operator()(const std::pair<int, ValueVector> &k) const
		{
			return (size_t)k.first + hash(k.second);
		}
	};
	struct Equal
	{
		Value::VectorEqual equal;
		size_t operator()(const std::pair<int, ValueVector> &a, const std::pair<int, ValueVector> &b) const
		{
			return a.first == b.first && equal(a.second, b.second);
		}
	};
	typedef __gnu_cxx::hash_set<std::pair<int, ValueVector>, Hash, Equal> UidValueSet;
}
void WeightedStatement::grounded(Grounder *g)
{
	NS_OUTPUT::ObjectVector lits;
	IntVector weights;
	if(setSemantics_)
	{
		UidValueSet set;
		for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
		{
			ConditionalLiteral *p = *it;
			if(!set.insert(std::make_pair(p->getUid(), p->getValues())).second)
			{
				p->remove();
				continue;
			}
			for(p->start(); p->hasNext(); p->next())
			{
				lits.push_back(p->convert());
				weights.push_back(1);
			}
		}
	}
	else
	{
		for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
		{
			ConditionalLiteral *p = *it;
			for(p->start(); p->hasNext(); p->next())
			{
				if(p->getWeight() == 0)
				{
					p->remove();
					continue;
				}
				lits.push_back(p->convert());
				weights.push_back(p->getWeight());
			}
		}
	}

	switch(type_)
	{
		case COMPUTE:
		{
			NS_OUTPUT::Compute *c = new NS_OUTPUT::Compute(lits, number_);
			g->getEvaluator()->add(c);
			break;
		}
		case MINIMIZE:
		{
			NS_OUTPUT::Optimize *c = new NS_OUTPUT::Optimize(NS_OUTPUT::Optimize::MINIMIZE, lits, weights);
			g->getEvaluator()->add(c);
			break;
		}
		case MAXIMIZE:
		{
			NS_OUTPUT::Optimize *c = new NS_OUTPUT::Optimize(NS_OUTPUT::Optimize::MAXIMIZE, lits, weights);
			g->getEvaluator()->add(c);
			break;
		}
	}
}

bool WeightedStatement::ground(Grounder *g)
{
	for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
		(*it)->ground(g);
	grounded(g);
	return true;
}

void WeightedStatement::addDomain(PredicateLiteral *pl)
{
	assert(false);
	//delete pl;
}

void WeightedStatement::print(std::ostream &out)
{
	switch(type_)
	{
		case COMPUTE:
			out << "compute ";
			break;
		case MINIMIZE:
			out << "minimize ";
			break;
		case MAXIMIZE:
			out << "maximize ";
			break;
	}
	out << "{";
	bool comma = false;
	for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		out << *it;
	}
	out << "}";
}

WeightedStatement::~WeightedStatement()
{
	for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
		delete *it;
}

