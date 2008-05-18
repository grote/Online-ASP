#ifndef SMODELSOUTPUT_H
#define SMODELSOUTPUT_H

#include <gringo.h>
#include <output.h>
#include <value.h>
#include <ext/hash_map>

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class SmodelsOutput : public Output
		{
		public:
			SmodelsOutput(std::ostream *out);
			void initialize(Grounder *g);
			void print(NS_OUTPUT::Object *o);
			void finalize();
			~SmodelsOutput();
		private:
			void print(NS_OUTPUT::Fact *r);
			void print(NS_OUTPUT::Rule *r);
			void print(NS_OUTPUT::Integrity *r);
			void print(NS_OUTPUT::Optimize *r);
			void print(NS_OUTPUT::Compute *r);
			void printBody(NS_OUTPUT::Aggregate *r);
			void printHead(int B, NS_OUTPUT::Aggregate *r);
			void printBody(int headId, NS_OUTPUT::ObjectVector &body);
			void printWeightRule(int head, int bound, NS_OUTPUT::ObjectVector &lits, IntVector &weights);
		private:
			int false_;
			IntVector compute_;
		};
	}
}

#endif

