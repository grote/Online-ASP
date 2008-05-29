#ifndef EXPANDABLE_H
#define EXPANDABLE_H

#include <gringo.h>

namespace NS_GRINGO
{
	/// Interface which allows to expand objects by literals
	class Expandable
	{
	public:
		enum ExpansionType { MATERM, RANGETERM, COMPLEXTERM };
	public:
		/**
		 * \brief Appends a literal to the object
		 * \param l The literal
		 */
		virtual void appendLiteral(Literal *l, ExpansionType type) = 0;
	};
}

#endif 
