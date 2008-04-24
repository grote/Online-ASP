#ifndef GROUNDABLE_H
#define GROUNDABLE_H

#include <gringo.h>

namespace NS_GRINGO
{
	class Groundable
	{
	public:
		virtual void grounded(Grounder *g) = 0;
	};
}

#endif

