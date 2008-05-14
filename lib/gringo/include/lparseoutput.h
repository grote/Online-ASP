#ifndef LPARSEOUTPUT_H
#define LPARSEOUTPUT_H

#include <gringo.h>
#include <output.h>

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class LparseOutput : public NS_OUTPUT::Output
		{
		public:
			LparseOutput(std::ostream *out);
			virtual void initialize(Grounder *g);
			virtual void print(NS_OUTPUT::Object *o);
			virtual void finalize();
			virtual ~LparseOutput();
		};
	}
}

#endif

