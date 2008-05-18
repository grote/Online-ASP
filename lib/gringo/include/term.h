#ifndef TERM_H
#define TERM_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	/**
	 * \brief Base class of all Terms
	 *
	 * Terms are variables, strings or functions. They are used by most literals. 
	 * During grounding every term will be evaluated to a value.
	 */
	class Term : public Printable
	{
	public:
		/**
		 * \brief Constructor - does nothing
		 */
		Term();
		/**
		 * \brief Retrieve all Variables in term
		 * \param vars stores the result
		 */
		virtual void getVars(VarSet &vars) = 0;
		/**
		 * \brief Return true if the Term is complex
		 *
		 * Everything except variables, ids, strings has to be complex
		 * \return Returns true if the term is complex
		 */
		virtual bool isComplex() = 0;
		/**
		 * \brief Returns the Value of the term under the current substitution
		 * \return The value
		 */
		virtual Value getValue() = 0;
		/**
		 * \brief Creates a copy of the term
		 * \return The copy
		 */
		virtual Term* clone() = 0;
		/**
		 * \brief Static preprocess of the term
		 *
		 * Currently rangeterms are removed and replaced by rangeliterals.
		 * \param p Reference to the place where the current term is stored
		 * \param g The grounder
		 * \param e Expandable object
		 */
		virtual void preprocess(Term *&p, Grounder *g, Expandable *e) = 0;
		/**
		 * \brief Virtual Destructor
		 */
		virtual ~Term();
	};
}

#endif

