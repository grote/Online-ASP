#ifndef FUNCSYMBOL_H
#define FUNCSYMBOL_H

#include <gringo.h>

namespace NS_GRINGO
{
	/**
	 * \brief class of function symbols
	 *
	 * consists of a string and a list of values 
	 * Is used in Value
	 */
	class FuncSymbol
	{
	public:
		/**
		 * \brief Constructor - f(a,b,c(1,2,3))    *s="f"
		 * doest not take ownership of s
		 */
		FuncSymbol(const std::string* s, const ValueVector& args);
		/**
		 * \brief simple hash function
		 * Merges the hash value of the argument vector with the hash value of the function name
		 */
		size_t getHash() const;
		/**
		 * \brief Comparison operator
		 */
		bool operator==(const FuncSymbol& a) const;
		/*+
		 * \brief Prints the function symbol
		 * \param out The output stream
		 */
		void print(std::ostream& out) const;
		/**
		 * \brief Virtual Destructor
		 */
		virtual ~FuncSymbol();
	protected:
		const std::string*	name_;
		ValueVector				args_;
	};
}

#endif

