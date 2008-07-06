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

#include <globalstorage.h>
#include <domain.h>

using namespace NS_GRINGO;

GlobalStorage::GlobalStorage()
{
}

Domain *GlobalStorage::getDomain(int uid) const
{
	return domains_[uid];
}

DomainVector *GlobalStorage::getDomains() const
{
	return const_cast<DomainVector*>(&domains_);
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
	for(DomainVector::iterator it = domains_.begin(); it != domains_.end(); it++)
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
	{
		pred_.push_back(Signature(id, arity));
		domains_.push_back(new Domain());
	}
	return res.first->second;
}

SignatureVector *GlobalStorage::getPred()
{
	return &pred_;
}

