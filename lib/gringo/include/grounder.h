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
		struct string_hash
		{
			inline size_t operator()(const std::string* a) const;
		};
		struct string_equal
		{
			inline bool operator()(const std::string* a, const std::string* b) const;
		};
		typedef __gnu_cxx::hash_set<std::string*, string_hash, string_equal> StringHash;
		typedef std::pair<std::string*, StringVector*> DomainPredicate;
		typedef std::vector<DomainPredicate> DomainVector;
		typedef std::map<std::string*, int> VariableMap;
	public:
		Grounder();
		void addStatement(Statement *rule);
		void addDomains(std::string *id, StringVector* list);
		void buildDepGraph();
		void start(NS_OUTPUT::Output &output);
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
		void setConstValue(std::string *id, Value *v);
		Value *createConstValue(std::string *id);
		Value *createStringValue(std::string *id);
		std::string *createString(std::string *s);
		std::string *createString(const std::string &s);
		void preprocess();
		NS_OUTPUT::Output *getOutput();
		Evaluator *getEvaluator();
		void hideAll();
		void setVisible(std::string *id, int arity, bool visible);
		bool isVisible(int uid);
		bool isVisible(std::string *id, int arity);
		SignatureVector *getPred();
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
		std::map<std::string*, Value*> const_;
		std::vector<Value> substitution_;
		std::vector<int> binder_;
		std::set<Signature> trueNegPred_;
		
		/// pool of all strings used
		StringHash stringHash_;

		// visibility of predicates: move to output
		bool hideAll_;
		std::map<Signature, bool> hide_;
		// constructed from hide_
		std::vector<bool> visible_;

	};

	bool Grounder::string_equal::operator()(const std::string *a, const std::string *b) const
	{
		return *a == *b;
	}

	size_t Grounder::string_hash::operator()(const std::string* a) const
	{
		return __gnu_cxx::__stl_hash_string(a->c_str());
	}
}

#endif

