#include <globalstorage.h>

using namespace NS_GRINGO;

GlobalStorage::GlobalStorage()
{
}

GlobalStorage::~GlobalStorage()
{
	// TODO: do i really need pointers????
	StringVector v(stringHash_.begin(), stringHash_.end());
	stringHash_.clear();
	for(StringVector::iterator it = v.begin(); it != v.end(); it++)
		delete *it;
	FuncSymbolVector x(funcHash_.begin(), funcHash_.end());
	funcHash_.clear();
	for(FuncSymbolVector::iterator it = x.begin(); it != x.end(); it++)
		delete *it;
}

std::string *GlobalStorage::createString(const std::string &s2)
{
	return createString(new std::string(s2));
}

std::string *GlobalStorage::createString(std::string *s)
{
	std::pair<StringHash::iterator, bool> res = stringHash_.insert(s);
	if(!res.second)
		delete s;
	return *res.first;
}

FuncSymbol *GlobalStorage::createFuncSymbol(FuncSymbol* fn)
{
	std::pair<FuncSymbolHash::iterator, bool> res = funcHash_.insert(fn);
	if (!res.second)
		delete fn;
	return *res.first;
}

int GlobalStorage::createPred(std::string *id, int arity)
{
	std::pair<SignatureHash::iterator, bool> res = predHash_.insert(std::make_pair(Signature(id, arity), (int)pred_.size()));
	if(res.second)
		pred_.push_back(Signature(id, arity));
	return res.first->second;
}

SignatureVector *GlobalStorage::getPred()
{
	return &pred_;
}

