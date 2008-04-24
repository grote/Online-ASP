#ifndef LITERAL_H
#define LITERAL_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	class Literal : public Printable
	{
	public:
		enum DependencyAdd { ADD_BODY_DEP, ADD_HEAD_DEP, ADD_NOTHING };
	public:
		Literal();
		void setNeg(bool neg);
		bool getNeg();
		virtual void getVars(VarSet &vars, VarsType type, VarVector &glob);
		virtual void getVars(VarSet &vars, VarsType type) = 0;
		virtual bool checkO(LiteralVector &unsolved) = 0;
		virtual void normalize(Grounder *g, Expandable *r) = 0;
		virtual void preprocess(Expandable *e) = 0;
		virtual void reset() = 0;
		virtual void finish() = 0;
		virtual void evaluate();
		virtual bool solved() = 0;
		virtual bool isFact() = 0;
		virtual Literal* clone() = 0;
		virtual void ground(Grounder *g);
		virtual IndexedDomain *createIndexedDomain(VarSet &index) = 0;
		virtual bool match(Grounder *g) = 0;
		virtual NS_OUTPUT::Object *convert() = 0;
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo) = 0;
 		virtual ~Literal();
	protected:
		bool neg_;
		
	};
}

#endif

