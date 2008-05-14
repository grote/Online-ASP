#ifndef GROUNDABLE_H
#define GROUNDABLE_H

#include <gringo.h>

namespace NS_GRINGO
{
	/**
	 * \brief Interface for groundable objects
	 */
	class Groundable
	{
	public:
		/**
		 * \brief This method is called after the object was grounded
		 * \param g Reference to the grounder
		 */
		virtual void grounded(Grounder *g) = 0;
	};
}

#endif

