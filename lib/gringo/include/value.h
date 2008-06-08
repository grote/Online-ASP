#ifndef VALUE_H
#define VALUE_H

#include <gringo.h>
#include <ext/hash_set>
#include "funcsymbol.h"
//#include <toolbox/allocator.h>

namespace NS_GRINGO
{
	/**
	 * \brief Class to store values of terms
	 * If strings are stored in a value this class does not take ownership of the string.
	 * The memory of the string has to be freed elsewhere.
	 */
	class Value
	{
	public:
		/// The type o a value
		enum Type { INT, STRING, FUNCSYMBOL, UNDEF };
		/// Hash function object for arrays of values
		struct VectorHash
		{
			/**
			 * \brief Hash function for arrays of values
			 * \param key The array
			 * \return The hash
			 */
			inline size_t operator() (const ValueVector &key) const;
		};	
		/// Comaparison function object for arrays of values
		struct VectorEqual
		{
			/**
			 * \brief Comparison function for arrays of values
			 * \return The hash
			 */
			inline bool operator() (const ValueVector &a, const ValueVector &b) const;
		};	
	public:
		/**
		 * \brief Constructor initializing the value to undef
		 */
		Value();
		/**
		 * \brief Copy constructor
		 */
		Value(const Value &v);
		/**
		 * \brief Creates a value encapsulationg a string
		 * \param value The string
		 */
		Value(std::string *value);
		/**
		 * \brief Creates a value encapsulationg an int
		 * \param intValue The int
		 */
		Value(int intValue);
		/**
		 * \brief Creates a value encapsulating a function symbol
		 * \param fn The function symbol
		 */
		Value(const FuncSymbol* fn);
		/**
		 * \brief Calculates a hash for the value
		 * \return The Hash
		 */
		inline size_t hashValue() const;
		/**
		 * Function used to compare values
		 * \param b The other value
		 * \return Return an int less then 0 if the the value is lower than b, 0 if the values are equal or an int > 0 if the value is greater than b
		 */
		int compare(const Value &b) const;
		/**
		 * \brief Function used to compare Values in hash_sets or hash_maps.
		 * This function doesnt throw an exception if the types of the values are distinct
		 * \return The result of the comparisson
		 */
		bool equal_set(const Value &b) const;
		/**
		 * \brief Operator to compare Values
		 * \return The result of the comparisson
		 */
		bool operator<(const Value &b) const;
		/**
		 * \brief Operator to compare Values
		 * \return The result of the comparisson
		 */
		bool operator>(const Value &b) const;
		/**
		 * \brief Operator to compare Values
		 * \return The result of the comparisson
		 */
		bool operator==(const Value &b) const;
		/**
		 * \brief Operator to compare Values
		 * \return The result of the comparisson
		 */
		bool operator<=(const Value &b) const;
		/**
		 * \brief Operator to compare Values
		 * \return The result of the comparisson
		 */
		bool operator>=(const Value &b) const;
		/**
		 * \brief Operator to compare Values
		 * \return The result of the comparisson
		 */
		bool operator!=(const Value &b) const;
		/**
		 * \brief Operator casting any value to int
		 * \return if the current value is a string or undef 0 is returned otherwise the value of the int is returned
		 */
		operator int();
	public:
		/// The type of the value
		Type type_;
		/// Union for all possible value types
		union
		{
			/// The int value
			int intValue_;
			/// The string value
			std::string *stringValue_;
			/// The FuncSymbol value
			const FuncSymbol *funcSymbol_;
		};
	};
	
	/**
	 * \brief Operator to easily print values
	 * \param out The output stream
	 * \param v The value
	 * \return Returns a reference to the output stream
	 */
	std::ostream &operator<<(std::ostream &out, const Value &v);
	
	/// Type to efficiently access values
	typedef __gnu_cxx::hash_set<ValueVector, Value::VectorHash, Value::VectorEqual> ValueVectorSet;

	size_t Value::hashValue() const
	{
		switch(type_)
		{
			case INT:
				return (size_t)intValue_;
			case STRING:
				return (size_t)stringValue_;
			case FUNCSYMBOL:
				return funcSymbol_->getHash();
			default:
				// this shouldnt happen
				assert(false);
		}
	}

	bool Value::VectorEqual::operator() (const ValueVector &a, const ValueVector &b) const
	{
		if(a.size() != b.size())
			return false;
		for(ValueVector::const_iterator i = a.begin(), j = b.begin(); i != a.end(); i++, j++)
		{
			if(!i->equal_set(*j))
				return false;
		}
		return true;
	}	
	
	size_t Value::VectorHash::operator() (const ValueVector &key) const
	{
		size_t hash = 0;
		size_t x = 0;
		for(ValueVector::const_iterator it = key.begin(); it != key.end(); it++, x++)
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

