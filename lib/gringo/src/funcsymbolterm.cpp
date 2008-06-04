#include "funcsymbolterm.h"
#include "value.h"
#include "grounder.h"
#include "funcsymbol.h"

using namespace NS_GRINGO;

FuncSymbolTerm::FuncSymbolTerm(Grounder* g, std::string* s, TermVector* tl) : Term(), name_(s), termList_(tl), grounder_(g)
{
}

void FuncSymbolTerm::print(std::ostream &out)
{
	out << *name_ << "(";
	for (unsigned int i = 0; i != termList_->size()-1; ++i)
	{
		out << *(*termList_)[i] << ",";
	}
	out << *(*termList_)[termList_->size()-1] << ")";
}

void FuncSymbolTerm::getVars(VarSet &vars) const
{
	for (TermVector::const_iterator i = termList_->begin(); i != termList_->end(); ++i)
		(*i)->getVars(vars);
}

void FuncSymbolTerm::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
	for (TermVector::iterator i = termList_->begin(); i != termList_->end(); ++i)
		(*i)->preprocess(l, (*i), g, e);
}

bool FuncSymbolTerm::isComplex()
{
	// TODO: change this to false in the new implementation of functionsymbols
	//       i want to keep a working version in the trunk so its better to 
	//       return true for now
	return true;
}

Value FuncSymbolTerm::getValue()
{
	ValueVector args;
	for (unsigned int i = 0; i != termList_->size(); ++i)
	{
		args.push_back((*termList_)[i]->getValue());
	}

	FuncSymbol* funcSymbol = new FuncSymbol(name_, args);

	funcSymbol = grounder_->createFuncSymbol(funcSymbol);
	return Value(funcSymbol);
}

FuncSymbolTerm::FuncSymbolTerm(const FuncSymbolTerm &f) : name_(f.name_), grounder_(f.grounder_)
{
	termList_ = new TermVector();
	for (TermVector::const_iterator i = f.termList_->begin(); i != f.termList_->end(); ++i)
	{
		termList_->push_back((*i)->clone());
	}
}

Term* FuncSymbolTerm::clone() const
{
	return new FuncSymbolTerm(*this);
}
bool FuncSymbolTerm::unify(const Value& t, const VarVector& boundVariables, const VarVector& freeVariables,
       	ValueVector& boundSubstitution, ValueVector& freeSubstitutions) const
{
	if (t.type_ == Value::FUNCSYMBOL)
	{
		const std::string* name = t.funcSymbol_->getName();
		if (name != name_)
			return false;
		const ValueVector& values = t.funcSymbol_->getValues();
		if (values.size() != termList_->size())
			return false;
		TermVector::const_iterator term = termList_->begin();
		for (ValueVector::const_iterator i = values.begin(); i != values.end(); ++i, ++term)
		{
			if (!(*term)->unify(*i, boundVariables, freeVariables, boundSubstitution, freeSubstitutions))
				return false;
		}
		return true;
	}
	else
		return false;

}

FuncSymbolTerm::~FuncSymbolTerm()
{
	for (TermVector::iterator i = termList_->begin(); i != termList_->end(); ++i)
		delete (*i);
	delete termList_;
}

