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
			void initialize(SignatureVector *pred);
			void print(Object *o);
			void finalize();
			~SmodelsOutput();
		protected:
			void printBasicRule(int head, const IntVector &pos, const IntVector &neg);
			void printConstraintRule(int head, int bound, const IntVector &pos, const IntVector &neg);
			void printChoiceRule(const IntVector &head, const IntVector &pos, const IntVector &neg);
			void printWeightRule(int head, int bound, const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg);
			void printMinimizeRule(const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg);
			void printDisjunctiveRule(const IntVector &head, const IntVector &pos, const IntVector &neg);
		private:
			void print(Fact *r);
			void print(Rule *r);
			void print(Integrity *r);
			void print(Optimize *r);
			void print(Compute *r);
			void printHead(Aggregate *a);
			void printBody(Aggregate *a);
			void handleHead(Object *o);
			void handleBody(ObjectVector &body);
			void handleAggregate(ObjectVector &lits);
			void handleAggregate(ObjectVector &lits, IntVector &weights);
			void handleCount(Aggregate *a, int &l, int &u);
			void handleSum(Aggregate *a, int &l, int &u);
		private:
			int false_;
			int models_;
			IntVector compute_;
			IntVector head_;
			IntVector pos_, neg_, wPos_, wNeg_;
			IntVector posA_, negA_, wPosA_, wNegA_;
		};
	}
}

#endif

