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

#ifndef PREDICATELITERAL_H
#define PREDICATELITERAL_H

#include <gringo.h>
#include <literal.h>
#include <groundable.h>
#include <expandable.h>
#include <value.h>

namespace NS_GRINGO
{
	class PredicateLiteral : public Literal
	{
	public:
		PredicateLiteral(Grounder *g, std::string *id, TermVector *variables);
		PredicateLiteral(const PredicateLiteral &p);
		virtual Literal* clone() const;
		void setWeight(Term *w);
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual bool solved();
		virtual bool isFact();
		bool isFact(const ValueVector &values);
		virtual void finish();
		virtual void evaluate();
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		bool match(const ValueVector &values);
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual NS_OUTPUT::Object *convert();
		virtual double heuristicValue();
		NS_OUTPUT::Object *convert(const ValueVector &values);
		std::string *getId();
		TermVector *getArgs();
		int getUid();
		int getArity();
		void addDomain(ValueVector &values);
		Domain *getDomain() const;
		const ValueVector &getValues();
		virtual ~PredicateLiteral();
	protected:
		int           uid_;
		Domain        *predNode_;
		std::string   *id_;
		TermVector    *variables_;

		ValueVector values_;
	};
}

#endif

