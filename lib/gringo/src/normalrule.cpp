#include "normalrule.h"
#include "literal.h"
#include "predicateliteral.h"
#include "dependencygraph.h"
#include "literaldependencygraph.h"
#include "node.h"
#include "grounder.h"
#include "output.h"
#include "dlvgrounder.h"
#include "evaluator.h"
#include "output.h"

using namespace NS_GRINGO;

NormalRule::NormalRule(Literal *head, LiteralVector *body) : Statement(), head_(head), body_(body), dg_(0)
{
}

void NormalRule::buildDepGraph(DependencyGraph *dg)
{
	Node *rn = dg->createStatementNode(this);
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
	if(dg_)
		delete dg_;
	dg_ = new LDG();
	free.clear();
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

bool NormalRule::ground(Grounder *g)
{
	if(body_)
	{
		// if there are no varnodes we can do sth simpler
		if(!dg_->hasVarNodes())
		{
			for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
			{
				if(!(*it)->match(g))
					return true;
			}
			grounded(g);
		}
		else
		{
			//std::cerr << "creating grounder for: " << this << std::endl;
			VarVector relevant;
			getRelevantVars(relevant);
			DLVGrounder data(g, this, body_->size(), dg_, relevant);
			//data.debug();
			data.ground();
		}
	}
	else
	{
		grounded(g);
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
		// TODO: if the head is already a fact this rule is useless!!! of course 
		// this may lead into trouble with choice rules no aggregate may claim 
		// that it is a fact (at least for now)
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
	if(body_)
		for(size_t i = 0; i < body_->size(); i++)
			(*body_)[i]->preprocess(g, this);
	if(head_)
	{
		NormalRuleExpander nre(this, g, body_);
		head_->preprocess(g, &nre);
	}
	//std::cerr << this << std::endl;
}

void NormalRule::appendLiteral(Literal *l, ExpansionType type)
{
	if(!body_)
		body_ = new LiteralVector();	
	body_->push_back(l);
}

NormalRule::~NormalRule()
{
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

