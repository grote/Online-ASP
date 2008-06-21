#ifndef SMODELSCONVERTER_H
#define SMODELSCONVERTER_H

#include <gringo.h>
#include <output.h>

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class SmodelsConverter : public Output
		{
		public:
			SmodelsConverter(std::ostream *out);
			virtual void initialize(SignatureVector *pred);
			virtual void print(Object *o);
			int getFalse() const;
			virtual ~SmodelsConverter();
		protected:
			virtual void printBasicRule(int head, const IntVector &pos, const IntVector &neg) = 0;
			virtual void printConstraintRule(int head, int bound, const IntVector &pos, const IntVector &neg) = 0;
			virtual void printChoiceRule(const IntVector &head, const IntVector &pos, const IntVector &neg) = 0;
			virtual void printWeightRule(int head, int bound, const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg) = 0;
			virtual void printMinimizeRule(const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg) = 0;
			virtual void printDisjunctiveRule(const IntVector &head, const IntVector &pos, const IntVector &neg) = 0;
			virtual void printComputeRule(int models, const IntVector &pos, const IntVector &neg) = 0;
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
			void handleSum(bool body, Aggregate *a, int &l, int &u);
			void handleMin(Aggregate *a, int &l, int &u);
			void handleMax(Aggregate *a, int &l, int &u);
		private:
			bool negBoundsWarning_;
			int false_;
			IntVector compute_;
			IntVector head_;
			IntVector pos_, neg_, wPos_, wNeg_;
			IntVector posA_, negA_, wPosA_, wNegA_;
		};
	}
}

#endif

