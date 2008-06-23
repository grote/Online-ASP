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

#ifndef MINAGGREGATE_H
#define MINAGGREGATE_H

#include <gringo.h>
#include <aggregateliteral.h>

namespace NS_GRINGO
{
	class MinAggregate : public AggregateLiteral
	{
	public:
		MinAggregate(ConditionalLiteralVector *literals);
		MinAggregate(const MinAggregate &a);
		virtual Literal *clone() const;
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual void match(Grounder *g, int &lower, int &upper, int &fixed);
		virtual void print(std::ostream &out);
		virtual NS_OUTPUT::Object *convert();
		virtual ~MinAggregate();
	};
}

#endif

