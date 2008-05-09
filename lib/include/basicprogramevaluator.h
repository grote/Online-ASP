#ifndef BASICPROGRAMEVALUATOR_H
#define BASICPROGRAMEVALUATOR_H

#include <gringo.h>
#include <evaluator.h>
#include <output.h>
#include <ext/hash_map>

namespace NS_GRINGO
{
	/**
	 * \brief Evaluator backend which calculates the single answer set for a basic program
	 */
	class BasicProgramEvaluator : public Evaluator
	{
	private:
		/// Type used to assign an id to a (variable free) predicate
                typedef __gnu_cxx::hash_map<ValueVector, std::pair<Node*, int>, Value::VectorHash> AtomHash;
		/// Type used to store the ids to all possible predicates
		typedef std::vector<AtomHash> AtomLookUp;
		/// Type used to store the basic program
		typedef std::vector<IntVector> RuleVector;
		/// Type used to efficiently propagate literals
		typedef std::vector<IntVector> Watches;
		/// Type used to store the facts
		typedef std::vector<bool> Facts;
	public:
		/// Constructor
		BasicProgramEvaluator();
		void initialize(Grounder *g);
		void add(NS_OUTPUT::Object *r);
		void evaluate();
		/// Destructor
		~BasicProgramEvaluator();
	private:
		/**
		 * \brief Helper function used to add atoms
		 * This function returns the id of the atom. If a new atoms is added 
		 * a new id is assigned.
		 *
		 * \param r The atom
		 * \return Returns the id of the atom
		 */
		int add(NS_OUTPUT::Atom *r);
		/**
		 * \brief Helper function to add facts.
		 * \param r The fact
		 */
		void add(NS_OUTPUT::Fact *r);
		/**
		 * \brief Helper function to add basic rules.
		 * \param r The rule
		 */
		void add(NS_OUTPUT::Rule *r);
		/**
		 * \brief Helper function to add bodies.
		 * \param r The body
		 */
		void add(NS_OUTPUT::Conjunction *r);
	private:
		/// Stores facts which have to be propagated
		IntVector  propQ_;
		/// Stores the ids of all atoms (predicates)
		AtomLookUp atomHash_;
		/// Stores the facts
		Facts      facts_;
		/// Stores a pointer to a rule with an atom which is currently no fact
		Watches    watches_;
		/// Stores the basic program
		RuleVector rules_;
		//StringVector debug_;
	};
}

#endif

