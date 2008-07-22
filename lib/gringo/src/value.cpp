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

#include "value.h"
#include "gringoexception.h"

using namespace NS_GRINGO;

Value::Value(int intValue) : type_(INT), intValue_(intValue)
{
}

Value::Value() : type_(UNDEF), intValue_(0)
{
}

Value::Value(std::string *value) : type_(STRING), stringValue_(value)
{
}

Value::Value(const FuncSymbol* funcSymbol) : type_(FUNCSYMBOL), funcSymbol_(funcSymbol)
{
}

Value::Value(const Value &v)
{
	type_ = v.type_;
	switch(type_)
	{
		case UNDEF:
			intValue_ = 0;
			break;
		case STRING:
			stringValue_ = v.stringValue_;
			break;
		case INT:
			intValue_ = v.intValue_;
			break;
		case FUNCSYMBOL:
			funcSymbol_ = v.funcSymbol_;
			break;
	}
}

bool Value::equal_set(const Value &b) const
{
	if(type_ != b.type_)
		return false;

	switch(type_)
	{
		case UNDEF:
			assert(false);
		case INT:
			return intValue_ == b.intValue_;
		case STRING:
			return stringValue_ == b.stringValue_;
		case FUNCSYMBOL:
			return funcSymbol_ == b.funcSymbol_;
	}
	assert(false);
}

int Value::compare(const Value &b) const
{
	if(type_ != b.type_)
		throw GrinGoException("error comparing different types");

	switch(type_)
	{
		case UNDEF:
			assert(false);
		case INT:
			return intValue_ - b.intValue_;
		case STRING:
			return stringValue_ - b.stringValue_ == 0 ? 0 : (stringValue_ - b.stringValue_ > 0 ? 1 : -1);
		case FUNCSYMBOL:
			return funcSymbol_ - b.funcSymbol_ == 0 ? 0 : (funcSymbol_ - b.funcSymbol_ > 0 ? 1 : -1);
	}
	assert(false);
}

Value::operator int()
{
	switch(type_)
	{
		case UNDEF:
			assert(false);
		case INT:
			return intValue_;
		case STRING:
			throw GrinGoException("error trying to convert string to int");
		case FUNCSYMBOL:
			throw GrinGoException("error trying to convert functionsymbol to int");
	}
	assert(false);
}

bool Value::operator<(const Value &b) const
{
	return compare(b) < 0;
}

bool Value::operator>(const Value &b) const
{
	return compare(b) > 0;
}

bool Value::operator==(const Value &b) const
{
	return equal_set(b);
}

bool Value::operator!=(const Value &b) const
{
	return !equal_set(b);
}

bool Value::operator<=(const Value &b) const
{
	return compare(b) <= 0;
}

bool Value::operator>=(const Value &b) const
{
	return compare(b) >= 0;
}

std::ostream &NS_GRINGO::operator<<(std::ostream &out, const Value &v)
{
	switch(v.type_)
	{
		case Value::UNDEF:
			out << "undef";
			break;
		case Value::INT:
			out << v.intValue_;
			break;
		case Value::STRING:
			out << (*v.stringValue_);
			break;
		case Value::FUNCSYMBOL:
			v.funcSymbol_->print(out);
			break;

	}
	return out;
}

