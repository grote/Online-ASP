#ifndef SMODELSOUTPUT_H
#define SMODELSOUTPUT_H

#include <gringo.h>
#include <smodelsconverter.h>

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class SmodelsOutput : public SmodelsConverter
		{
		public:
			SmodelsOutput(std::ostream *out);
			void initialize(SignatureVector *pred);
			void finalize();
			~SmodelsOutput();
		protected:
			void printBasicRule(int head, const IntVector &pos, const IntVector &neg);
			void printConstraintRule(int head, int bound, const IntVector &pos, const IntVector &neg);
			void printChoiceRule(const IntVector &head, const IntVector &pos, const IntVector &neg);
			void printWeightRule(int head, int bound, const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg);
			void printMinimizeRule(const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg);
			void printDisjunctiveRule(const IntVector &head, const IntVector &pos, const IntVector &neg);
			void printComputeRule(int models, const IntVector &pos, const IntVector &neg);
		private:
			int models_;
			IntSet compNeg_, compPos_;
		};
	}
}

#endif

