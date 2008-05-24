#include "constant.h"
#include "grounder.h"
//debug
#include "funcsymbolterm.h"

using namespace NS_GRINGO;
		
Constant::Constant(int value) : Term(), type_(NUM), g_(0), value_(value), uid_(0)
{
}

Constant::Constant(ConstantType type, Grounder *g, std::string *value) : Term(), type_(type), g_(g), uid_(type == VAR ? g->registerVar(value) : 0)
{
	switch(type_)
	{
		case ID:
		{
			Value *v = g->createConstValue(value);
			if(v->type_ == Value::INT)
			{
				value_.type_     = Value::INT;
				value_.intValue_ = v->intValue_;
				type_            = NUM;
			}
			else
			{
				value_.type_        = Value::STRING;
				value_.stringValue_ = v->stringValue_;
				type_               = STRING;
			}

			delete v;
			break;
		}
		case VAR:
		case STRING:
		{
			value_.type_        = Value::STRING;
			value_.stringValue_ = value;
			break;
		}
		case NUM: 
		{
			value_.type_     = Value::INT;
			value_.intValue_ = atol(value->c_str());
			delete value;
			break;
		}
		default:
		{
		}
	}
}

void Constant::getVars(VarSet &vars)
{
	if(type_ == VAR)
		vars.insert(uid_);
}

bool Constant::isComplex()
{
	return false;
}

void Constant::print(std::ostream &out)
{
	switch(type_)
	{
		case VAR:
		case ID:
		case STRING:
		{
			out << *(value_.stringValue_);
			break;
		}
		case NUM: 
		{
			out << value_.intValue_;
			break;
		}
		default:
		{
		}
	}
}

Value Constant::getValue()
{
	if(type_ == VAR)
		return g_->getValue(uid_);
	else
	{
		//assert(type_ == FUNCSYMBOL);
		if (type_ == FUNCSYMBOL)
		{
			std::cout << "ich bin in Constant::getValue obwohl ich eine FuncSymbolTerm Instanz habe, FUCK" << std::endl;
			std::cout << std::flush;
			return dynamic_cast<FuncSymbolTerm*>(this)->getValue();
		}
		else
			return value_;
	}
}

int Constant::getUID()
{
	return uid_;
}

Constant::~Constant()
{
}

void Constant::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
	//nothing todo
}

Constant::Constant(Constant &c) : type_(c.type_), g_(c.g_), value_(c.value_), uid_(c.uid_)
{
}

Term* Constant::clone() 
{
	return new Constant(*this);
}

