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
	private:
		typedef std::pair<std::string*, StringVector*> DomainPredicate;
		typedef std::vector<DomainPredicate> DomainPredicateVector;
		typedef std::map<std::string*, int> VariableMap;
		typedef std::map<std::string*, std::pair<bool, Term*> > ConstTerms;
		typedef std::vector<std::pair<std::pair<IncPart, std::string*>, int> > IncParts;
	public:
		Grounder();
		void setOutput(NS_OUTPUT::Output *output);
		void addStatement(Statement *rule);
		void addDomains(std::string *id, std::vector<StringVector*>* list);
		void start();
#ifdef WITH_ICLASP
		void iground();
		int getIncStep() const;
#endif
		void addProgram(Program *scc);
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
		/// Adds a domain that never occurs in any head
		void addZeroDomain(Domain *d);
		void setIncPart(IncPart part, std::string *var);
		~Grounder();
	private:
		void buildDepGraph();
		void addDomains();
		void reset();
		void check();
		void ground();
		void addDomains(std::string *id, std::vector<StringVector*>::iterator pos, std::vector<StringVector*>::iterator end, StringVector &list);
	private:
		bool inc_;
		IncParts incParts_;
		int incStep_;

		int internalVars_;
		ProgramVector sccs_;
		VariableMap varMap_;
		DomainPredicateVector domains_;
		StatementVector rules_;
		NS_OUTPUT::Output *output_;
		Evaluator *eval_;
		ConstTerms constTerms_;
		ValueVector substitution_;
		IntVector binder_;
		std::set<Signature> trueNegPred_;
	};
}

#endif

