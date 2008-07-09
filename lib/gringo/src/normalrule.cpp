// Copyright (c) 2008, Roland Kaminski
//
// This file is part of GrinGo.
//
// GrinGo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrinGo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrinGo.  If not, see <http://www.gnu.org/licenses/>.

#include "normalrule.h"
#include "literal.h"
#include "predicateliteral.h"
#include "statementdependencygraph.h"
#include "literaldependencygraph.h"
#include "grounder.h"
#include "output.h"
#include "dlvgrounder.h"
#include "evaluator.h"
#include "output.h"
#include "gringoexception.h"
#include "indexeddomain.h"
#include "constant.h"

using namespace NS_GRINGO;

#ifdef WITH_ICLASP
NormalRule::NormalRule(Literal *head, LiteralVector *body) : Statement(), head_(head), body_(body), ground_(1), dg_(0), grounder_(0)
#else
NormalRule::NormalRule(Literal *head, LiteralVector *body) : Statement(), head_(head), body_(body), dg_(0), grounder_(0)
#endif
{
}

void NormalRule::buildDepGraph(SDG *dg)
{
	SDGNode *rn = dg->createStatementNode(this);
	if(head_)
		head_->createNode(dg, rn, Literal::ADD_HEAD_DEP);
	else
	{
		// we have an integrity constraint " :- B", which can be modeled by "r :- B, not r",
		// where r is a unique identifier
		// this means the rule depends negativly on itself
		rn->addDependency(rn, true);
	}
	if(body_)
	{
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
			(*it)->createNode(dg, rn, Literal::ADD_BODY_DEP);
	}
}

void NormalRule::print(std::ostream &out)
{
	if(head_)
		out << head_;
	if(body_)
	{
		out << " :- ";
		bool comma = false;
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
		{
			if(comma)
				out << ", ";
			else
				comma = true;
			out << (*it);
		}
	}
	out << ".";
}

bool NormalRule::checkO(LiteralVector &unsolved)
{
	unsolved.clear();
	// there are some literals (predicates with conditionals) which must have domainrestricted parts
	if(head_)
		if(!head_->checkO(unsolved))
			return false;
	if(body_)
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
			if(!(*it)->checkO(unsolved))
				return false;
	return true;
}

bool NormalRule::check(VarVector &free)
{
	free.clear();
	if(dg_)
		delete dg_;
	dg_ = new LDG();
	LDGBuilder dgb(dg_);
	if(head_)
		dgb.addHead(head_);
	if(body_)
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
			dgb.addToBody(*it);
	
	dgb.create();
	dg_->check(free);

	return free.size() == 0;
}

void NormalRule::getVars(VarSet &vars) const
{
	if(head_)
		head_->getVars(vars);
	if(body_)
		for(LiteralVector::const_iterator it = body_->begin(); it != body_->end(); it++)
			(*it)->getVars(vars);
}

void NormalRule::addDomain(PredicateLiteral *pl)
{
	if(!body_)
		body_ = new LiteralVector();
	body_->push_back(pl);
}

void NormalRule::reset()
{
	if(head_)
		head_->reset();
}

void NormalRule::finish()
{
	if(head_)
		head_->finish();
}

void NormalRule::evaluate()
{
	if(head_)
		head_->evaluate();
}

void NormalRule::getRelevantVars(VarVector &relevant)
{
	// TODO: there must be a better way to do this!!!
	VarSet global;
	global.insert(dg_->getGlobalVars().begin(), dg_->getGlobalVars().end());
	VarSet all;
	if(head_)
		head_->getVars(all);
	if(body_)
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
		{
			if(!(*it)->solved())
				(*it)->getVars(all);
		}
	for(VarSet::iterator it = all.begin(); it != all.end(); it++)
		if(global.find(*it) != global.end())
			relevant.push_back(*it);
}

bool NormalRule::ground(Grounder *g, GroundStep step)
{
#ifdef WITH_ICLASP
	if(!ground_)
		return true;
#endif
	switch(step)
	{
		case PREPARE:
			// if there are no varnodes we can do sth simpler
			if(body_ && dg_->hasVarNodes())
			{
				//std::cerr << "creating grounder for: " << this << std::endl;
				VarVector relevant;
				getRelevantVars(relevant);
				dg_->sortLiterals(body_);
				grounder_ = new DLVGrounder(g, this, body_, dg_, relevant);
			}
			else
				grounder_ = 0;
			break;
		case REINIT:
			if(grounder_)
				grounder_->reinit(dg_);
			break;
		case GROUND:
			if(grounder_)
				grounder_->ground();
			else
			{
				if(body_)
					for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
						if(!(*it)->match(g))
							return true;
				grounded(g);
			}
#ifdef WITH_ICLASP
			if(ground_ == 2)
				ground_ = 0;
#endif
			break;
		case RELEASE:
#ifdef WITH_ICLASP
			if(grounder_)
				grounder_->release();
#else
			if(dg_)
			{
				delete dg_;
				dg_ = 0;
			}
			if(grounder_)
			{
				delete grounder_;
				grounder_ = 0;
			}
#endif
			break;
	}
	if(step != GROUND)
	{
		if(head_)
			head_->ground(g, step);
		if(body_)
			for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
				(*it)->ground(g, step);
	}
	return true;
}

void NormalRule::grounded(Grounder *g)
{
	bool hasHead;
	NS_OUTPUT::Object *head;
	NS_OUTPUT::ObjectVector body;
	Evaluator *eval = g->getEvaluator();

	if(head_ && head_->match(g))
	{
		// NOTE: if the head is already a fact this rule is useless!!! of course 
		// except if the head is a choice head since choice heads are always facts
		// currently aggregates are only facts if all their literals are facts
		if(head_->isFact())
			return;
		hasHead = true;
		head = head_->convert();
	}
	else
	{
		hasHead = false;
		head    = 0;
	}
	// cache variables in solved predicates
	if(body_)
	{
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
		{
			if(!(*it)->isFact())
			{
				// flatten conjunctions
				NS_OUTPUT::Object *c = (*it)->convert();
				if(dynamic_cast<NS_OUTPUT::Conjunction*>(c))
				{
					body.insert(
						body.end(), 
						static_cast<NS_OUTPUT::Conjunction*>(c)->lits_.begin(),
						static_cast<NS_OUTPUT::Conjunction*>(c)->lits_.end());
					static_cast<NS_OUTPUT::Conjunction*>(c)->lits_.clear();
					delete c;
				}
				else
					body.push_back(c);
			}
		}
	}
	if(!hasHead && body.size() == 0)
	{
		// TODO: thats not very nice find a better solution
		NS_OUTPUT::Conjunction *c = new NS_OUTPUT::Conjunction(body);
		NS_OUTPUT::Integrity  *i = new NS_OUTPUT::Integrity(c);
		eval->add(i);
	}
	else if(!hasHead)
	{
		NS_OUTPUT::Conjunction *c = new NS_OUTPUT::Conjunction(body);
		NS_OUTPUT::Integrity  *i = new NS_OUTPUT::Integrity(c);
		eval->add(i);
	}
	else if(body.size() == 0)
	{
		NS_OUTPUT::Fact *f = new NS_OUTPUT::Fact(head);
		eval->add(f);
	}
	else
	{
		NS_OUTPUT::Conjunction *c = new NS_OUTPUT::Conjunction(body);
		NS_OUTPUT::Rule *r = new NS_OUTPUT::Rule(head, c);
		eval->add(r);
	}
}

// this looks odd so i hide it :)
namespace
{
	class NormalRuleExpander : public Expandable
	{
	public:
		NormalRuleExpander(NormalRule *n, Grounder *g, LiteralVector *r) : n_(n), g_(g), r_(r)
		{
		}
		void appendLiteral(Literal *l, ExpansionType type)
		{
			if(type == MATERM)
			{
				LiteralVector *r;
				if(r_)
				{
					r = new LiteralVector();
					for(LiteralVector::iterator it = r_->begin(); it != r_->end(); it++)
						r->push_back((*it)->clone());
				}
				else
					r = 0;
				g_->addStatement(new NormalRule(l, r));
			}
			else
			{
				n_->appendLiteral(l, type);
			}
				
		}
	protected:
		NormalRule    *n_;
		Grounder      *g_;
		LiteralVector *r_;
	};
}

void NormalRule::preprocess(Grounder *g)
{
	if(head_)
	{
		NormalRuleExpander nre(this, g, body_);
		head_->preprocess(g, &nre);
	}
	if(body_)
		for(size_t i = 0; i < body_->size(); i++)
			(*body_)[i]->preprocess(g, this);
	//std::cerr << this << std::endl;
}

void NormalRule::appendLiteral(Literal *l, ExpansionType type)
{
	if(!body_)
		body_ = new LiteralVector();	
	body_->push_back(l);
}

#ifdef WITH_ICLASP

namespace
{
	class IndexedDomainInc : public IndexedDomain
	{
	public:
		IndexedDomainInc(int uid) : uid_(uid)
		{
		}
		void firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
		{
			//std::cerr << "matching with: " << Value(g->g_->getIncStep()) << std::endl;
			g->g_->setValue(uid_, Value(g->g_->getIncStep()), binder);
			status = SuccessfulMatch;
		}
		void nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
		{
			status = FailureOnNextMatch;
		}
		virtual ~IndexedDomainInc()
		{
		}
	private:
		int uid_;
	};
	
	class LambdaLiteral : public Literal
	{
	public:
		LambdaLiteral(Constant *c) : c_(c)
		{
		}
		LambdaLiteral(const LambdaLiteral &l) : c_(static_cast<Constant*>(l.c_->clone()))
		{
		}
		void getVars(VarSet &vars) const
		{
			c_->getVars(vars);
		}
		bool checkO(LiteralVector &unsolved)
		{
			return true;
		}
		void preprocess(Grounder *g, Expandable *e)
		{
		}
		void reset()
		{
			assert(false);
		}
		void finish()
		{
			assert(false);
		}
		void evaluate()
		{
			assert(false);
		}
		bool solved()
		{
			return true;
		}
		bool isFact()
		{
			return true;
		}
		Literal* clone() const
		{
			return new LambdaLiteral(*this);
		}
		IndexedDomain *createIndexedDomain(VarSet &index)
		{
			if(index.find(c_->getUID()) != index.end())
			{
				return new IndexedDomainMatchOnly(this);
			}
			else
			{
				return new IndexedDomainInc(c_->getUID());
			}
		}
		bool match(Grounder *g)
		{
			return (int)c_->getValue() == g->getIncStep();
		}
		NS_OUTPUT::Object *convert()
		{
			assert(false);
		}
		SDGNode *createNode(SDG *dg, SDGNode *prev, DependencyAdd todo)
		{
			// a rule with a lambda predicate is a normal logic program
			// cause otherwise ill get problems with the basicprogram evaluater
			/*
			assert(todo == ADD_BODY_DEP);
			prev->addDependency(prev, true);
			*/
			return 0;
		}
		void createNode(LDGBuilder *dg, bool head)
		{
			assert(!head);
			VarSet needed, provided;
			c_->getVars(provided);
			dg->createNode(this, head, needed, provided);
		}
		double heuristicValue()
		{
			return 0;
		}
		void print(std::ostream &out)
		{
			out << "lambda(" << c_ << ")";
		}
 		virtual ~LambdaLiteral()
		{
			delete c_;
		}
	private:
		Constant *c_;
	};

	class DeltaLiteral : public Literal
	{
	public:
		DeltaLiteral(Constant *c) : c_(c)
		{
		}
		DeltaLiteral(const DeltaLiteral &l) : c_(static_cast<Constant*>(l.c_->clone()))
		{
		}
		void getVars(VarSet &vars) const
		{
			c_->getVars(vars);
		}
		bool checkO(LiteralVector &unsolved)
		{
			return true;
		}
		void preprocess(Grounder *g, Expandable *e)
		{
		}
		void reset()
		{
			assert(false);
		}
		void finish()
		{
			assert(false);
		}
		void evaluate()
		{
			assert(false);
		}
		bool solved()
		{
			return false;
		}
		bool isFact()
		{
			return false;
		}
		Literal* clone() const
		{
			return new DeltaLiteral(*this);
		}
		IndexedDomain *createIndexedDomain(VarSet &index)
		{
			if(index.find(c_->getUID()) != index.end())
			{
				return new IndexedDomainMatchOnly(this);
			}
			else
			{
				return new IndexedDomainInc(c_->getUID());
			}
		}
		bool match(Grounder *g)
		{
			return (int)c_->getValue() == g->getIncStep();
		}
		NS_OUTPUT::Object *convert()
		{
			return new NS_OUTPUT::DeltaObject();
		}
		SDGNode *createNode(SDG *dg, SDGNode *prev, DependencyAdd todo)
		{
			// a rule with a goal is always a normal logic program
			assert(todo == ADD_BODY_DEP);
			prev->addDependency(prev, true);
			return 0;
		}
		void createNode(LDGBuilder *dg, bool head)
		{
			assert(!head);
			VarSet needed, provided;
			c_->getVars(provided);
			dg->createNode(this, head, needed, provided);
		}
		double heuristicValue()
		{
			return 0;
		}
		void print(std::ostream &out)
		{
			out << "delta(" << c_ << ")";
		}
 		virtual ~DeltaLiteral()
		{
			delete c_;
		}
	private:
		Constant *c_;
	};
}

void NormalRule::setIncPart(Grounder *g, IncPart part, std::string *var)
{
	switch(part)
	{
		case BASE:
			ground_ = 2;
			break;
		case LAMBDA:
			appendLiteral(new LambdaLiteral(new Constant(Constant::VAR, g, var)), Expandable::COMPLEXTERM);
			break;
		case DELTA:
			appendLiteral(new DeltaLiteral(new Constant(Constant::VAR, g, var)), Expandable::COMPLEXTERM);
			break;
		default:
			break;
	}
}
#endif

NormalRule::~NormalRule()
{
	if(grounder_)
		delete grounder_;
	if(dg_)
		delete dg_;
	if(head_)
		delete head_;
	if(body_)
	{
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
			delete *it;
		delete body_;
	}
}

