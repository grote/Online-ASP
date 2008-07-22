// Copyright (c) 2008, Roland Kaminski
//
// This file is part of GrinGo.
//
// GrinGo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrinGo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrinGo.  If not, see <http://www.gnu.org/licenses/>.

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

	hash = (hash << 4) + size_t(name_);
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
