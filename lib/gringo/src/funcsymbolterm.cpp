#include "funcsymbolterm.h"
#include "value.h"
#include "grounder.h"

using namespace NS_GRINGO;

FuncSymbolTerm::FuncSymbolTerm(Grounder* g, std::string* s, TermVector* tl) : Term(), funcSymbol_(s), termList_(tl), grounder_(g)
{
}

void FuncSymbolTerm::print(std::ostream &out)
{
	out << *funcSymbol_ << "(";
	for (unsigned int i = 0; i != termList_->size()-1; ++i)
	{
		out << *(*termList_)[i] << ",";
	}
	out << *(*termList_)[termList_->size()-1] << ")";
}

void FuncSymbolTerm::getVars(VarSet &vars)
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
	return true;
}

Value FuncSymbolTerm::getValue()
{
	std::string* s = new std::string();
	*s = *funcSymbol_;
	*s += "(";
	std::ostringstream ss;
	for (unsigned int i = 0; i != termList_->size()-1; ++i)
	{
		ss << (*termList_)[i]->getValue() << ",";
	}
	ss << (*termList_)[termList_->size()-1]->getValue();
	*s += ss.str();
	*s += ")";

	// grounder can clean this up
	// if the string already exists s is freed and the old pointer is returned
	// if the string is new s is inserted into the set of all strings and
	// the same pointer is returned
	s = grounder_->createString(s);

	return Value(s);
}

FuncSymbolTerm::FuncSymbolTerm(FuncSymbolTerm &f) : funcSymbol_(f.funcSymbol_), grounder_(f.grounder_)
{
	termList_ = new TermVector();
	for (TermVector::const_iterator i = f.termList_->begin(); i != f.termList_->end(); ++i)
	{
		termList_->push_back((*i)->clone());
	}
}

Term* FuncSymbolTerm::clone()
{
	return new FuncSymbolTerm(*this);
}

FuncSymbolTerm::~FuncSymbolTerm()
{
	for (TermVector::iterator i = termList_->begin(); i != termList_->end(); ++i)
		delete (*i);
	delete termList_;
}

