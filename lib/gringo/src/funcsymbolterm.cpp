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
	//TODO
	return false;
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

FuncSymbolTerm::FuncSymbolTerm(FuncSymbolTerm &f) : name_(f.name_), grounder_(f.grounder_)
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

