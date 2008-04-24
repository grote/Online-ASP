#include "normalrule.h"
#include "literal.h"
#include "predicateliteral.h"
#include "dependencygraph.h"
#include "node.h"
#include "grounder.h"
#include "output.h"
#include "dlvgrounder.h"
#include "evaluator.h"
#include "output.h"

using namespace NS_GRINGO;

NormalRule::NormalRule(Literal *head, LiteralVector *body) : Statement(), head_(head), body_(body)
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

void NormalRule::normalize(Grounder *g)
{
	if(body_)
	{
		// the bodyvector may realloc
		size_t l = body_->size();
		for(size_t i = 0; i < l; i++)
			(*body_)[i]->normalize(g, this);
	}
	if(head_)
		head_->normalize(g, this);
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
	VarSet needed, provided;
	// all global vars in head are needed
	if(head_)
		head_->getVars(needed, VARS_GLOBAL);
	if(body_)
	{
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
		{
			(*it)->getVars(needed, VARS_NEEDED);
			(*it)->getVars(provided, VARS_PROVIDED);
		}
	}

	for(VarSet::iterator it = needed.begin(); it != needed.end(); it++)
		if(provided.find(*it) == provided.end())
			free.push_back(*it);

	return free.size() == 0;
}

void NormalRule::getVars(VarSet &vars)
{
	if(head_)
		head_->getVars(vars, VARS_ALL);
	if(body_)
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
			(*it)->getVars(vars, VARS_ALL);
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

void NormalRule::getRelevantVars(VarSet &relevant, VarSet &glob)
{
	VarVector global(glob.begin(), glob.end());
	if(head_)
		head_->getVars(relevant, VARS_ALL, global);
	if(body_)
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
		{
			if(!(*it)->solved())
				(*it)->getVars(relevant, VARS_ALL, global);
		}
	//relevant.resize(glob.size());
	//std::copy(glob.begin(), glob.end(), relevant.begin());
}

void NormalRule::getGlobalVars(VarSet &glob)
{
	// extract global vars
	if(head_)
		head_->getVars(glob, VARS_GLOBAL);
	if(body_)
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
			(*it)->getVars(glob, VARS_GLOBAL);
}

bool NormalRule::ground(Grounder *g)
{
	if(body_)
	{
		VarSet glob, relevant;
		getGlobalVars(glob);
		getRelevantVars(relevant, glob);
		DLVGrounder data(g, this, *body_, glob, relevant);
		//std::cerr << this << std::endl;
		//data.debug();
		data.ground();
	}
	else
	{
		assert(head_);
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

	if(head_)
	{
		// TODO: if an aggregate is used in the head and doesnt support any literals the rule can be skipped
		//       if the head is trivially false we get an integrity constraint
		//       a method like truthValue->{true, false, undef} could be used here too
		// some heads may involve local grounding
		head_->ground(g);
		// nothing has to be done if the head is already a fact
		// it could even lead to problems with the basic program evaluator 
		// this is not tested
		if(head_->isFact())
			return;
		head = head_->convert();
		hasHead = true;
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
			// TODO: if the literal is true we dont have to add it
			//       there should be used another method like truthValue->{true, false, undef}
			//       false cant (should not) occur and only undef literals have to be added
			if(!(*it)->isFact())
				body.push_back((*it)->convert());
		}
	}
	if(!hasHead && body.size() == 0)
	{
		// TODO: thats not very nice
		std::cerr << "inconsistency found" << std::endl;
		exit(0);
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
		NormalRuleExpander(Grounder *g, LiteralVector *r) : g_(g), r_(r)
		{
		}
		void appendLiteral(Literal *l)
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
	protected:
		Grounder      *g_;
		LiteralVector *r_;
	};
}

void NormalRule::preprocess(Grounder *g)
{
	if(body_)
		for(size_t i = 0; i < body_->size(); i++)
			(*body_)[i]->preprocess(this);
	if(head_)
	{
		NormalRuleExpander nre(g, body_);
		head_->preprocess(&nre);
	}
}

void NormalRule::appendLiteral(Literal *l)
{
	if(!body_)
		body_ = new LiteralVector();	
	body_->push_back(l);
}

NormalRule::~NormalRule()
{
	if(head_)
		delete head_;
	if(body_)
	{
		for(LiteralVector::iterator it = body_->begin(); it != body_->end(); it++)
			delete *it;
		delete body_;
	}
}

