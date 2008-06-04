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

	class IndexedDomainDefault : public IndexedDomain
	{
		typedef __gnu_cxx::hash_map<ValueVector, std::vector<const ValueVector*>, Value::VectorHash> ValueVectorMap;
		//typedef std::map<ValueVector, std::vector<const ValueVector*> > ValueVectorMap;
	public:
		IndexedDomainDefault(ValueVectorSet &domain, VarSet &index, ConstantVector &param);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainDefault();
	protected:
		ValueVector currentIndex_;
                ValueVectorMap domain_;
		std::vector<std::pair<int, int> > bind_;
		std::vector<std::pair<int, int> > index_;
		std::vector<const ValueVector*>::iterator current_, end_;
	};

	class IndexedDomainNewDefault : public IndexedDomain
	{
		typedef __gnu_cxx::hash_map<ValueVector, std::vector<ValueVector>, Value::VectorHash> ValueVectorMap;
		//typedef std::map<ValueVector, std::vector<const ValueVector*> > ValueVectorMap;
	public:
		IndexedDomainNewDefault(ValueVectorSet &domain, VarSet &index, const TermVector &param);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainNewDefault();
	protected:
		ValueVector currentIndex_;
                ValueVectorMap domain_;
		//std::vector<std::pair<int, int> > bind_;
		VarVector bind_;
		//std::vector<std::pair<int, int> > index_;
		VarVector index_;
		std::vector<ValueVector>::iterator current_, end_;
	};


	class IndexedDomainFullMatch : public IndexedDomain
	{
	public:
		IndexedDomainFullMatch(ValueVectorSet &domain, ConstantVector &param);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainFullMatch();
	protected:
		const ValueVectorSet &domain_;
		IntVector bind_;
		ValueVectorSet::const_iterator current_;
	};
}

#endif

