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
			virtual void initialize(SignatureVector *pred);
			virtual void print(NS_OUTPUT::Object *o);
			virtual void finalize();
			virtual ~GrinGoOutput();
		};
	}
}

#endif

