#ifndef CLASPOUTPUT_H
#define CLASPOUTPUT_H

#include <gringo.h>
#include <smodelsconverter.h>
#include <clasp/include/lparse_reader.h>

namespace Clasp
{
	class ProgramBuilder;
}

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class ClaspOutput : public SmodelsConverter
		{
		public:
			ClaspOutput(Clasp::ProgramBuilder *b, Clasp::LparseReader::TransformMode tf);
			void initialize(SignatureVector *pred);
			void finalize();
			int newUid();
			Clasp::LparseStats &getStats();
			~ClaspOutput();
		protected:
			void printBasicRule(int head, const IntVector &pos, const IntVector &neg);
			void printConstraintRule(int head, int bound, const IntVector &pos, const IntVector &neg);
			void printChoiceRule(const IntVector &head, const IntVector &pos, const IntVector &neg);
			void printWeightRule(int head, int bound, const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg);
			void printMinimizeRule(const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg);
			void printDisjunctiveRule(const IntVector &head, const IntVector &pos, const IntVector &neg);
			void printComputeRule(int models, const IntVector &pos, const IntVector &neg);
		private:
			Clasp::LparseStats stats_;
			Clasp::ProgramBuilder *b_;
			Clasp::LparseReader::TransformMode tf_;
		};
	}
}

#endif

