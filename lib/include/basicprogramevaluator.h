#ifndef BASICPROGRAMEVALUATOR_H
#define BASICPROGRAMEVALUATOR_H

#include <gringo.h>
#include <evaluator.h>
#include <output.h>
#include <ext/hash_map>

namespace NS_GRINGO
{
	class BasicProgramEvaluator : public Evaluator
	{
	private:
                typedef __gnu_cxx::hash_map<ValueVector, std::pair<Node*, int>, Value::VectorHash> AtomHash;
		typedef std::vector<AtomHash> AtomLookUp;
		typedef std::vector<IntVector> RuleVector;
		typedef std::vector<IntVector> Watches;
		typedef std::vector<bool> Facts;
	public:
		BasicProgramEvaluator();
		void initialize(Grounder *g);
		int add(NS_OUTPUT::Atom *r);
		void add(NS_OUTPUT::Object *r);
		void evaluate();
		~BasicProgramEvaluator();
	private:
		void add(NS_OUTPUT::Fact *r);
		void add(NS_OUTPUT::Rule *r);
		void add(NS_OUTPUT::Conjunction *r);
	private:
		IntVector  propQ_;
		AtomLookUp atomHash_;
		Facts      facts_;
		Watches    watches_;
		RuleVector rules_;
		//StringVector debug_;
	};
}

#endif

