#include "value.h"
#include <ext/hash_map>

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

int Value::compare(const Value &b) const
{
	if(type_ < b.type_)
		return -1;
	if(type_ > b.type_)
		return 1;

	switch(type_)
	{
		case UNDEF:
			assert(false);
		case INT:
			return intValue_ - b.intValue_;
		case STRING:
			return (int)stringValue_ - (int)b.stringValue_;
		case FUNCSYMBOL:
			return (int)funcSymbol_ - (int)b.funcSymbol_;
	}
	assert(false);
}

Value::operator int()
{
	if(type_ == INT)
		return intValue_;
	return 0;
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
	return compare(b) == 0;
}

bool Value::operator!=(const Value &b) const
{
	return compare(b) != 0;
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
			{
				v.funcSymbol_->print(out);
			}

	}
	return out;
}

