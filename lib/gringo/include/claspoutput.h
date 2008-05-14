#ifndef CLASPOUTPUT_H
#define CLASPOUTPUT_H

#include <gringo.h>
#include <output.h>

namespace Clasp
{
	class ProgramBuilder;
}

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class ClaspOutput : public Output
		{
		public:
			ClaspOutput(Clasp::ProgramBuilder *b);
			void initialize(Grounder *g);
			void print(NS_OUTPUT::Object *o);
			void finalize();
			~ClaspOutput();
			int newUid();
		private:
			void print(NS_OUTPUT::Fact *r);
			void print(NS_OUTPUT::Rule *r);
			void print(NS_OUTPUT::Integrity *r);
			void printBody(NS_OUTPUT::Aggregate *r);
			void printHead(int B, NS_OUTPUT::Aggregate *r);
			void printBody(int headId, NS_OUTPUT::ObjectVector &body);
			void printWeightRule(int head, int bound, NS_OUTPUT::ObjectVector &lits, IntVector &weights);
		private:
			Clasp::ProgramBuilder *b_;
			int false_;
		};
	}
}

#endif

