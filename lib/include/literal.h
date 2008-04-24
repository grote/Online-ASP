#ifndef LITERAL_H
#define LITERAL_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	/**
	 * \brief Base class for all literals that can be handled by the grounder
	 */
	class Literal : public Printable
	{
	public:
		/// Type used during creation of the dependency graph
		enum DependencyAdd 
		{ 
			ADD_BODY_DEP, ///< Used while processing body literals
			ADD_HEAD_DEP, ///< Used while processing head literals
			ADD_NOTHING   ///< Used if no dependency has to be added
		};
	public:
		/// Constructor initializes neg to false
		Literal();
		/**
		 * \brief Sets the sign of the literal
		 * \param neg The new sign
		 */
		void setNeg(bool neg);
		/**
		 * \brief Retrieve the sign ofthe literal.
		 * \return Returns the sign
		 */
		bool getNeg();
		/**
		 * \brief Determines the vars of the literal wrt. a set of global variables
		 *
		 * The set of global variables has to be ordered. The default implementation 
		 * calls getVars() and intersects the result with the global variables.
		 * \param vars Reference to a set used to store the result
		 * \param type Used to filter variables
		 * \param glob A Set of global variables
		 */
		virtual void getVars(VarSet &vars, VarsType type, VarVector &glob);
		/**
		 * \brief Retrieve the vars of the literal
		 * \param vars Reference to a set used to store the result
		 * \param type Used to filter variables
		 */
		virtual void getVars(VarSet &vars, VarsType type) = 0;
		/**
		 * \brief Used to check omega restricted parts of literals.
		 *
		 * Some literals like conditionals in symbolic sets require 
		 * omega-restricted parts. This method can be used to ensure this
		 * property. This method is called after preprocess().
		 * \param unsolved Reference to a vector stroring all non-omega-restricted literals
		 * \return Returns true if check passed
		 */
		virtual bool checkO(LiteralVector &unsolved) = 0;
		/**
		 * \brief Normalizes the literal.
		 *
		 * For easier handling while grounding some literals have to be normalized.
		 * Currently the following two transformations are applied:
		 *  - Range terms are removed and replaced and complex terms and
		 *  - Complex terms are removed from solved predicates.
		 * This method is called after checkO().
		 * \param g Reference to the grounder
		 * \param r Reference to the object that is normalized
		 */
		virtual void normalize(Grounder *g, Expandable *r) = 0;
		/**
		 * \brief This method is used for static preprocessing.
		 *
		 * Currently this method is used to remove all terms of the form: a;b.
		 * \param Reference to the object that is preprocessed
		 */
		virtual void preprocess(Expandable *e) = 0;
		/**
		 * \brief This method is called to undo a call to finish() and evaluate()
		 */
		virtual void reset() = 0;
		/**
		 * \brief Used to inform the literal that it is finished
		 *
		 * After for example a normal rule is grounded the head literal is finished.
		 * Internally a counter is decremented and if this counter hits zero the literal
		 * is complete or solved.
		 */
		virtual void finish() = 0;
		/**
		 * \brief Used to inform the literal that it is evaluated
		 *
		 * This method method is called after a sub program is grounded.
		 * Example: After a basic program is evaluated this method is called for all
		 * head literals of the rules in the basic program. And then all head literal 
		 * are solved.
		 */
		virtual void evaluate();
		/**
		 * \brief Returns if the literal has a solved domain
		 * \return Returns true if the literal has a solved domain
		 */
		virtual bool solved() = 0;
		/**
		 * \brief Returns if the literal is a fact wrt. the current assignment
		 * \return Returns true if the literal is a fact
		 */
		virtual bool isFact() = 0;
		/**
		 * \brief Clones the literal
		 * \return Pointer to the new copied literal
		 */
		virtual Literal* clone() = 0;
		/**
		 * \brief Triggers local grounding
		 *
		 * Some literals may need local grounding. For example Aggregates in the head 
		 * with conditionals have to be grounded independently of the body of the rule.
		 * The default implementation does nothing.
		 */
		virtual void ground(Grounder *g);
		/**
		 * \brief Creates an index on the domain of the literal
		 * \param index Stores a set to the variables that have to be indexed.
		 * \return Returns the new indexed domain
		 */
		virtual IndexedDomain *createIndexedDomain(VarSet &index) = 0;
		/**
		 * \brief Determines if the literal can be satisfied wrt. the current substitution
		 * \param g Reference to the grounder
		 * \return returns true if the literal matches.
		 */
		virtual bool match(Grounder *g) = 0;
		/**
		 * \brief Converts the literal into a representation that can be hadled by the evaluator/output classes.
		 * \return Returns the representation 
		 */
		virtual NS_OUTPUT::Object *convert() = 0;
		/**
		 * \brief Adds the literal to the dependency graph
		 * \param dg The dependency graph
		 * \param prev The parent node
		 * \param todo The type of dependencies that still have to be added
		 * \return Returns the node that was created if any otherwise zero
		 */
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo) = 0;
		/// Destructor
 		virtual ~Literal();
	protected:
		/// The sign of the literal
		bool neg_;
	};
}

#endif

