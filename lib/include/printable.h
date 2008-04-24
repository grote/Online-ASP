#ifndef PRINTABLE_H
#define PRINTABLE_H

#include <gringo.h>

namespace NS_GRINGO
{
	class Printable
	{
	public:
		Printable();
		virtual ~Printable();
		virtual void print(std::ostream &stream) = 0;
		friend std::ostream &operator<<(std::ostream &stream, Printable &p);
		friend std::ostream &operator<<(std::ostream &stream, Printable *p);
	};

}

#endif

