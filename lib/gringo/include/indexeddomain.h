#ifndef INDEXEDDOMAIN_H
#define INDEXEDDOMAIN_H

#include <gringo.h>
#include <value.h>
#include <ext/hash_map>

namespace NS_GRINGO
{
	class IndexedDomain
	{
	public:
		IndexedDomain();
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status) = 0;
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status) = 0;
		virtual ~IndexedDomain();
	};

	class IndexedDomainMatchOnly : public IndexedDomain
	{
	public:
		IndexedDomainMatchOnly(Literal *l);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainMatchOnly();
	protected:
		Literal *l_;
	};

	class IndexedDomainNewDefault : public IndexedDomain
	{
		typedef __gnu_cxx::hash_map<ValueVector, ValueVector, Value::VectorHash> ValueVectorMap;
	public:
		IndexedDomainNewDefault(ValueVectorSet &domain, VarSet &index, const TermVector &param);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainNewDefault();
	protected:
		ValueVector currentIndex_;
                ValueVectorMap domain_;
		VarVector bind_;
		VarVector index_;
		ValueVector::iterator current_, end_;
	};
}

#endif

