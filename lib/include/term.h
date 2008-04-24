#ifndef TERM_H
#define TERM_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	class Term : public Printable
	{
	public:
		Term();
		virtual void getVars(VarSet &vars) = 0;
		virtual bool isComplex() = 0;
		virtual Value getValue() = 0;
		virtual Term* clone() = 0;

		virtual ~Term();
		
	};
}

#endif

