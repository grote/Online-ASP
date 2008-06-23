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

#ifndef CONJUNCTIONAGGREGATE_H
#define CONJUNCTIONAGGREGATE_H

#include <gringo.h>
#include <aggregateliteral.h>

namespace NS_GRINGO
{
	class ConjunctionAggregate : public AggregateLiteral
	{
	public:
		ConjunctionAggregate(ConditionalLiteralVector *literals);
		ConjunctionAggregate(const ConjunctionAggregate &a);
		virtual Literal *clone() const;
		virtual bool match(Grounder *g);
		virtual void match(Grounder *g, int &lower, int &upper, int &fixed);
		virtual void print(std::ostream &out);
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual NS_OUTPUT::Object *convert();
		virtual ~ConjunctionAggregate();
	public:
	       	static Literal *createHead(ConditionalLiteralVector *list);
	       	static Literal *createBody(PredicateLiteral *pred, LiteralVector *list);
	};
}

#endif

