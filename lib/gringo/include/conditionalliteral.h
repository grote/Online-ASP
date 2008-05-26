#ifndef CONDITIONALLITERAL_H
#define CONDITIONALLITERAL_H

#include <gringo.h>
#include <literal.h>
#include <groundable.h>
#include <expandable.h>

namespace NS_GRINGO
{
	class ConditionalLiteral : public Literal, public Groundable, public Expandable
	{

	public:
		ConditionalLiteral(PredicateLiteral *pred, LiteralVector *conditionals);
		ConditionalLiteral(ConditionalLiteral &p);
		virtual Literal* clone();
		void setWeight(Term *w);
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars);
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual bool solved();
		virtual bool isFact();
		virtual void finish();
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual void ground(Grounder *g);
		virtual void grounded(Grounder *g);
		virtual void appendLiteral(Literal *l, bool materm = false);
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual NS_OUTPUT::Object *convert();
		virtual double heuristicValue();
		bool check(VarVector &free);
		int getUid();
		bool isEmpty();
		void start();
		bool hasWeight();
		bool hasNext();
		void setNeg(bool neg);
		void next();
		void cacheVariables();
		ValueVector &getValues();
		int getWeight();
		virtual ~ConditionalLiteral();
	protected:
		PredicateLiteral *pred_;
		LiteralVector    *conditionals_;
		Term             *weight_;
		DLVGrounder      *grounder_;
		LDG              *dg_;

		std::vector<ValueVector> values_;
		std::vector<int>         weights_;
		size_t                   current_;
	};
}

#endif

