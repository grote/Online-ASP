#include "funcsymbolterm.h"
#include "value.h"
#include "grounder.h"

using namespace NS_GRINGO;

FuncSymbolTerm::FuncSymbolTerm(Grounder* g, std::string* s, TermVector* tl) : Term(), grounder_(g), funcSymbol_(s), termList_(tl)
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
	// grounder can clean this up
	grounder_->addFunctionSymbolString(s);
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


	return Value(s);
}

FuncSymbolTerm::FuncSymbolTerm(FuncSymbolTerm &f) : grounder_(f.grounder_), funcSymbol_(new std::string(*f.funcSymbol_))
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
	//delete funcSymbol_;
	for (TermVector::iterator i = termList_->begin(); i != termList_->end(); ++i)
		delete (*i);
	delete termList_;
}

