#include "funcsymbol.h"
#include "value.h"

using namespace NS_GRINGO;

FuncSymbol::FuncSymbol(const std::string* s, const ValueVector& args) : name_(s), args_(args)
{
}

FuncSymbol::~FuncSymbol()
{
}

bool FuncSymbol::operator==(const FuncSymbol& a) const
{
	if (name_ != a.name_)
		return false;
	if (args_.size() != a.args_.size())
		return false;

	for (ValueVector::const_iterator i = args_.begin(), j = a.args_.begin(); i != args_.end(); ++i,++j)
		if (*i != *j)
			return false;
	return true;
}

size_t FuncSymbol::getHash() const
{
	size_t hash = 0;
	size_t x = 0;

	Value::VectorHash t;
	hash = t(args_);

	hash = (hash << 4) + int(name_);
	if((x = hash & 0xF0000000L) != 0)
	{
		hash ^= (x >> 24);
	}
	hash &= ~x;
	return hash;
}

void FuncSymbol::print(std::ostream& out) const
{
	out << *name_ << "(";
	for (unsigned int i = 0; i != args_.size()-1; ++i)
		out << args_[i] << ",";
	out << args_[args_.size()-1] << ")";
}

const std::string* FuncSymbol::getName() const
{
	return name_;
}

const ValueVector& FuncSymbol::getValues() const
{
	return args_;
}
