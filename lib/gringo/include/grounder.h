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

#ifndef GROUNDER_H
#define GROUNDER_H

#include <gringo.h>
#include <output.h>
#include <globalstorage.h>

namespace NS_GRINGO
{
	class Grounder : public GlobalStorage
	{
	protected:
		typedef std::pair<std::string*, StringVector*> DomainPredicate;
		typedef std::vector<DomainPredicate> DomainVector;
		typedef std::map<std::string*, int> VariableMap;
		typedef std::map<std::string*, std::pair<bool, Term*> > ConstTerms;
	public:
		Grounder(NS_OUTPUT::Output *output);
		void addStatement(Statement *rule);
		void addDomains(std::string *id, std::vector<StringVector*>* list);
		void buildDepGraph();
		void start();
		void addDomains();
		void reset(bool warn);
		void ground();
		void addSCC(SCC *scc);
		void addTrueNegation(std::string *id, int arity);
		int getVar(std::string *var);
		std::string *createUniqueVar();
		std::string *getVarString(int uid);
		int registerVar(std::string *var);
		// access the current substitution
		Value getValue(int var);
		void setValue(int var, const Value &val, int binder);
		void setTempValue(int var, const Value &val);
		// access binders
		int getBinder(int var);
		void setConstValue(std::string *id, Term *p);
		Value getConstValue(std::string *id);
		void preprocess();
		NS_OUTPUT::Output *getOutput();
		Evaluator *getEvaluator();
		virtual ~Grounder();
	protected:
		void addDomains(std::string *id, std::vector<StringVector*>::iterator pos, std::vector<StringVector*>::iterator end, StringVector &list);
	protected:
		int internalVars_;
		SCCVector sccs_;
		VariableMap varMap_;
		DomainVector domains_;
		StatementVector rules_;
		DependencyGraph *depGraph_;
		NS_OUTPUT::Output *output_;
		Evaluator *eval_;
		ConstTerms constTerms_;
		ValueVector substitution_;
		IntVector binder_;
		std::set<Signature> trueNegPred_;
	};
}

#endif

