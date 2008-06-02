#ifndef GLOBALSTORAGE
#define GLOBALSTORAGE

#include <gringo.h>
#include <funcsymbol.h>
#include <ext/hash_set>
#include <ext/hash_map>

namespace NS_GRINGO
{
	class GlobalStorage
	{
	private:
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
	private:
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
