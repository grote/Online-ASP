#ifndef VALUE_H
#define VALUE_H

#include <gringo.h>
#include <ext/hash_set>
//#include <toolbox/allocator.h>

namespace NS_GRINGO
{
	class Value
	{
	public:
		enum Type { INT, STRING, UNDEF };
		struct SingleHash
		{
			inline size_t operator() (const Value &key) const;
		};
		struct PairHash
		{
			inline size_t operator() (const std::pair<const Value, const Value> &key) const;
		};
		struct VectorHash
		{
			inline size_t operator() (const std::vector<Value> &key) const;
		};	
	public:
		Value();
		Value(const Value &v);
		Value(std::string *value);
		Value(int intValue);
		inline size_t hashValue() const;
		int compare(const Value &b) const;
		bool operator<(const Value &b) const;
		bool operator>(const Value &b) const;
		bool operator==(const Value &b) const;
		bool operator<=(const Value &b) const;
		bool operator>=(const Value &b) const;
		bool operator!=(const Value &b) const;
		operator int();
		friend std::ostream &operator<<(std::ostream &out, const Value &v);
	public:
		Type type_;
		union
		{
			int intValue_;
			std::string *stringValue_;
		};
	};
	
	std::ostream &operator<<(std::ostream &out, const Value &v);

	typedef __gnu_cxx::hash_set<ValueVector, Value::VectorHash> ValueVectorSet;
	//typedef std::set<ValueVector> ValueVectorSet;

	size_t Value::hashValue() const
	{
		switch(type_)
		{
			case INT:
				return (size_t)intValue_;
			case STRING:
				return __gnu_cxx::__stl_hash_string(stringValue_->c_str());
			default:
				*((int*)0) = 10;
				// this shouldnt happen
				assert(false);
		}
	}

	size_t Value::SingleHash::operator() (const Value &key) const
	{
		return key.hashValue();
	}

	size_t Value::PairHash::operator() (const std::pair<const Value, const Value> &key) const
	{
		size_t hash;
		size_t x;
		hash = key.first.hashValue();
		if((x = hash & 0xF0000000L) != 0)
			hash ^= (x >> 24);
		hash&= ~x;
		hash = (hash << 4) + key.second.hashValue();
		if((x = hash & 0xF0000000L) != 0)
			hash ^= (x >> 24);
		hash &= ~x;
		return hash;
	}

	size_t Value::VectorHash::operator() (const std::vector<Value> &key) const
	{
		size_t hash = 0;
		size_t x = 0;
		for(std::vector<Value>::const_iterator it = key.begin(); it != key.end(); it++, x++)
		{
			hash = (hash << 4) + it->hashValue();
			if((x = hash & 0xF0000000L) != 0)
			{
				hash ^= (x >> 24);
			}
			hash &= ~x;
		}
		return hash;
	}

}

#endif

