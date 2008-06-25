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

#ifndef GLOBALSTORAGE
#define GLOBALSTORAGE

#include <gringo.h>
#include <funcsymbol.h>

namespace NS_GRINGO
{
	class GlobalStorage
	{
	protected:
		struct string_hash
		{
			inline size_t operator()(const std::string* a) const;
		};
		struct string_equal
		{
			inline bool operator()(const std::string* a, const std::string* b) const;
		};
		struct funcSym_hash
		{
			inline size_t operator()(const FuncSymbol* a) const;
		};
		struct funcSym_equal
		{
			inline bool operator()(const FuncSymbol* a, const FuncSymbol* b) const;
		};
		struct sig_hash
		{
			inline size_t operator()(const Signature &a) const;
		};
		typedef __gnu_cxx::hash_set<std::string*, string_hash, string_equal> StringHash;
		typedef __gnu_cxx::hash_set<FuncSymbol*, funcSym_hash, funcSym_equal> FuncSymbolHash;
		typedef __gnu_cxx::hash_map<Signature, int, sig_hash> SignatureHash;
	public:
		GlobalStorage();
		
		std::string *createString(std::string *s);
		std::string *createString(const std::string &s);
		FuncSymbol* createFuncSymbol(FuncSymbol* fn);
		int createPred(std::string *id, int arity);

		SignatureVector *getPred();

		virtual ~GlobalStorage();
	protected:
		StringHash stringHash_;
		FuncSymbolHash funcHash_;
		SignatureHash predHash_;
		SignatureVector pred_;

	};
	
	bool GlobalStorage::string_equal::operator()(const std::string *a, const std::string *b) const
	{
		return *a == *b;
	}

	size_t GlobalStorage::string_hash::operator()(const std::string* a) const
	{
		return __gnu_cxx::__stl_hash_string(a->c_str());
	}

	bool GlobalStorage::funcSym_equal::operator()(const FuncSymbol* a, const FuncSymbol* b) const
	{
		return *a == *b;
	}

	size_t GlobalStorage::funcSym_hash::operator()(const FuncSymbol* a) const
	{
		return a->getHash();
	}

	size_t GlobalStorage::sig_hash::operator()(const Signature &s) const
	{
		return (size_t)s.first + (size_t)s.second;
	}
}

#endif
