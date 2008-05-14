#ifndef VALUE_H
#define VALUE_H

#include <gringo.h>
#include <ext/hash_set>
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
		enum Type { INT, STRING, UNDEF };
		/// Hash function object for single values
		struct SingleHash
		{
			/**
			 * \brief Hash function for single values
			 * \param key The value
			 * \return The hash
			 */
			inline size_t operator() (const Value &key) const;
		};
		/// Hash function object for tuples of values
		struct PairHash
		{
			/**
			 * \brief Hash function for tuples of values
			 * \param key The tuple
			 * \return The hash
			 */
			inline size_t operator() (const std::pair<const Value, const Value> &key) const;
		};
		/// Hash function object for arrays of values
		struct VectorHash
		{
			/**
			 * \brief Hash function for arrays of values
			 * \param key The array
			 * \return The hash
			 */
			inline size_t operator() (const std::vector<Value> &key) const;
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
	typedef __gnu_cxx::hash_set<ValueVector, Value::VectorHash> ValueVectorSet;
	//typedef std::set<ValueVector> ValueVectorSet;

	size_t Value::hashValue() const
	{
		switch(type_)
		{
			case INT:
				return (size_t)intValue_;
			case STRING:
				// TODO: change this see todo list
				return __gnu_cxx::__stl_hash_string(stringValue_->c_str());
			default:
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

