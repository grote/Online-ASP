#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <gringo.h>

namespace NS_GRINGO
{
	/**
	 * \brief Basic evaluator backend
	 *
	 * The default implemation passes the given object to output and registers
	 * the head of the rules with the domain of the corresponding literals.
	 */
	class Evaluator
	{
	public:
		/**
		 * \brief Constructor
		 */
		Evaluator();
		/**
		 * \brief Initializes the evaluator
		 * \param g The grounder
		 */
		virtual void initialize(Grounder *g);
		/**
		 * \brief Adds an object to the evaluator
		 * \param r The object
		 */
		virtual void add(NS_OUTPUT::Object *r);
		/**
		 * \brief Starts the evaluation
		 *
		 * The grounder calls this method after all objects of a subprogram have been added.
		 */
		virtual void evaluate();
		/**
		 * \brief Destructor
		 */
		virtual ~Evaluator();
	public:
		/// Pointer to the grounder
		Grounder          *g_;
		/// Pointer to the output class
		NS_OUTPUT::Output *o_;
	};
}

#endif

