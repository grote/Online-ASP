#ifndef PREDICATELITERAL_H
#define PREDICATELITERAL_H

#include <gringo.h>
#include <literal.h>
#include <groundable.h>
#include <expandable.h>
#include <value.h>

namespace NS_GRINGO
{
	class PredicateLiteral : public Literal
	{
	public:
		PredicateLiteral(std::string *id, TermVector *variables);
		PredicateLiteral(PredicateLiteral &p, Term *t = 0);
		virtual Literal* clone();
		void setWeight(Term *w);
		void normalizeConditionals(Grounder *g);
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars, VarsType type);
		virtual bool checkO(LiteralVector &unsolved);
		virtual void normalize(Grounder *g, Expandable *r);
		virtual void ground(Grounder *g);
		virtual void reset();
		virtual bool solved();
		virtual bool isFact();
		virtual void finish();
		virtual void evaluate();
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual NS_OUTPUT::Object *convert();
		NS_OUTPUT::Object *convert(ValueVector &values);
		std::string *getId();
		TermVector *getArgs();
		int getUid();
		int getArity();
		void addDomain(ValueVector &values);
		void getVars(VarSet &vars);
		ValueVector &getValues();
		virtual ~PredicateLiteral();
	protected:
		Node          *predNode_;
		std::string   *id_;
		TermVector    *variables_;

		ValueVector matchValues_;
		ValueVector values_;
	};
}

#endif

