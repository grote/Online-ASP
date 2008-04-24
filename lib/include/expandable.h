#ifndef EXPANDABLE_H
#define EXPANDABLE_H

#include <gringo.h>

namespace NS_GRINGO
{
	class Expandable
	{
	public:
		virtual void appendLiteral(Literal *l) = 0;
	};
}

#endif 
