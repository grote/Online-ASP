#ifndef CONSTANT_H
#define CONSTANT_H

#include <gringo.h>
#include <term.h>
#include <value.h>

namespace NS_GRINGO
{
	class Constant : public Term
	{
	public:
		enum ConstantType { ID, VAR, NUM };
	public:
		Constant(int value);
		Constant(ConstantType type, Grounder *g, std::string *value);
		Constant(const Constant &c);
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual Value getConstValue();
		virtual Value getValue();
		int getUID();
		virtual Term* clone() const;
		virtual bool unify(const Value& t, const VarVector& boundVariables, const VarVector& freeVariables,
			       	ValueVector& boundSubstitution, ValueVector& freeSubstitutions) const;
 		virtual ~Constant();
	protected:
		ConstantType type_;
		Grounder *g_;
		Value value_;
		int uid_;
	};
}

#endif

