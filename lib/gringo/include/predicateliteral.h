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
		PredicateLiteral(Grounder *g, std::string *id, TermVector *variables);
		PredicateLiteral(const PredicateLiteral &p);
		virtual Literal* clone() const;
		void setWeight(Term *w);
		virtual Node *createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual bool solved();
		virtual bool isFact();
		bool isFact(const ValueVector &values);
		virtual void finish();
		virtual void evaluate();
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		bool match(const ValueVector &values);
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual NS_OUTPUT::Object *convert();
		virtual double heuristicValue();
		NS_OUTPUT::Object *convert(ValueVector &values);
		std::string *getId();
		TermVector *getArgs();
		int getUid();
		int getArity();
		void addDomain(ValueVector &values);
		ValueVector &getValues();
		virtual ~PredicateLiteral();
	protected:
		Node          *predNode_;
		std::string   *id_;
		TermVector    *variables_;
		int           uid_;

		ValueVector matchValues_;
		ValueVector values_;
	};
}

#endif

