#ifndef GROUNDER_H
#define GROUNDER_H

#include <gringo.h>
#include <output.h>
#include <ext/hash_set>

namespace NS_GRINGO
{
	class Grounder
	{
	protected:
		typedef std::pair<std::string*, StringVector*> DomainPredicate;
		typedef std::vector<DomainPredicate> DomainVector;
		typedef std::map<std::string, int> VariableMap;
	public:
		Grounder();
		void addStatement(Statement *rule);
		void addDomains(std::string *id, StringVector* list);
		void buildDepGraph();
		void start(NS_OUTPUT::Output &output);
		void start();
		void addDomains();
		void reset();
		void ground();
		void addSCC(SCC *scc);
		void addTrueNegation(const std::string &id, int arity);
		int getVar(std::string *var);
		std::string createUniqueVar();
		std::string getVarString(int uid);
		int registerVar(std::string *var);
		// access the current substitution
		Value getValue(int var);
		void setValue(int var, const Value &val, int binder);
		void setTempValue(int var, const Value &val);
		// access binders
		int getBinder(int var);
		void setConstValue(const std::string &id, Value *v);
		Value *createConstValue(std::string *id);
		Value *createStringValue(std::string *id);
		void preprocess();
		NS_OUTPUT::Output *getOutput();
		Evaluator *getEvaluator();
		void hideAll();
		void setVisible(std::string *id, int arity, bool visible);
		bool isVisible(int uid);
		bool isVisible(const std::string &id, int arity);
		std::vector<std::pair<std::string, int> > *getPred();
		virtual ~Grounder();
	protected:
		int internalVars_;
		SCCVector sccs_;
		VariableMap varMap_;
		DomainVector domains_;
		StatementVector rules_;
		DependencyGraph *depGraph_;
		NS_OUTPUT::Output *output_;
		Evaluator *eval_;
		std::map<std::string, Value*> const_;
		std::vector<Value> substitution_;
		std::vector<int> binder_;
		std::vector<std::string*> strings_;
		bool hideAll_;
		std::map<std::pair<std::string, int>, bool> hide_;
		// constructed from hide_
		std::vector<bool> visible_;
		std::set<Signature> trueNegPred_;
	};
}

#endif

