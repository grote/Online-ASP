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

#include "scc.h"
#include "statement.h"
#include "grounder.h"
#include "literal.h"
#include "evaluator.h"
#include "basicprogramevaluator.h"

using namespace NS_GRINGO;

SCC::SCC() : type_(FACT), edges_(0), eval_(0)
{
}

Evaluator *SCC::getEvaluator()
{
	if(!eval_)
	{
		switch(type_)
		{
			case SCC::FACT:
				eval_ = new Evaluator();
				break;
			case SCC::BASIC:
				eval_ = new BasicProgramEvaluator();
				break;
			case SCC::NORMAL:
				eval_ = new Evaluator();
				break;
		}
	}
	return eval_;
}

void SCC::print(std::ostream &out)
{
	if(rules_.size() > 0)
	{
		switch(type_)
		{
			case SCC::FACT:
				out << "% fact program:" << std::endl;
				break;
			case SCC::BASIC:
				out << "% basic program:" << std::endl;
				break;
			case SCC::NORMAL:
				out << "% normal program:" << std::endl;
				break;
		}
		for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
		{
			out << (*it) << std::endl;;
		}
	}
}

bool SCC::check(Grounder *g)
{
	VarVector free;
	LiteralVector unsolved;
	// conditionals have to be omega restricted
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
	{
		if(!(*it)->checkO(unsolved))
		{
			std::cerr << "the following rule cannot not be grounded, ";
			std::cerr << "non domain predicates : { ";
			bool comma = false;
			for(LiteralVector::iterator it = unsolved.begin(); it != unsolved.end(); it++)
			{
				if(comma)
					std::cerr << ", ";
				else
					comma = true;
				std::cerr << *it;
			}
			std::cerr << " }" << std::endl;
			std::cerr << "	" << rules_.back() << std::endl;
			return false;
		}
	}
	// check if scc is lambda restricted
	for(StatementVector::iterator ok = rules_.begin(); ok != rules_.end();)
	{
		for(; ok != rules_.end(); ok++)
		{
			if(!(*ok)->check(free))
				break;
			(*ok)->finish();
		}
		if(ok != rules_.end())
		{
			StatementVector::iterator it = ok;
			for(it++; it != rules_.end(); it++)
			{
				// maybe we get new domain predicates so normalizing has to be restarted
				if((*it)->check(free))
				{
					std::swap(*ok, *it);
					break;
				}
			}
			if(it == rules_.end())
			{
				std::cerr << "the following rule cannot not be grounded, ";
				std::cerr << "weakly restricted varibles: { ";
				bool comma = false;
				for(VarVector::iterator it = free.begin(); it != free.end(); it++)
				{
					if(comma)
						std::cerr << ", ";
					else
						comma = true;
					std::cerr << *g->getVarString(*it);
				}
				std::cerr << " }" << std::endl;
				std::cerr << "	" << rules_.back() << std::endl;
				return false;
			}
		}
	}
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
		(*it)->evaluate();
	return true;
}

StatementVector *SCC::getStatements()
{
	return &rules_;
}

SCC::SCCType SCC::getType()
{
	return type_;
}

SCC::~SCC()
{
	if(eval_)
		delete eval_;
}

