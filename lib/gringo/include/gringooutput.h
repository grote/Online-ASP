#ifndef GRINGOOUTPUT_H
#define GRINGOOUTPUT_H

#include <gringo.h>
#include <output.h>

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class GrinGoOutput : public Output
		{
		public:
			GrinGoOutput(std::ostream *out);
			virtual void initialize(Grounder *g);
			virtual void print(NS_OUTPUT::Object *o);
			virtual void finalize();
			virtual ~GrinGoOutput();
		};
	}
}

#endif

