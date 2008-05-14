#ifndef PRINTABLE_H
#define PRINTABLE_H

#include <gringo.h>

namespace NS_GRINGO
{
	/**
	 * Interface for Printable objects like terms, literals or statements.
	 * This interface is used to print out an internal representation of the problem instance.
	 */
	class Printable
	{
	public:
		/**
		 * \brief Constructor - does nothing
		 */
		Printable();
		/**
		 * \brief Virtual destructor
		 */
		virtual ~Printable();
		/**
		 * \brief Prints the object on the given stream
		 * \param stream The output stream
		 */
		virtual void print(std::ostream &stream) = 0;
	};
	/**
	 * \brief Easier printing of the object
	 * Internally the function Printable::print is called.
	 *
	 * \param stream The output stream
	 * \param p The Printable
	 * \return Returns a reference to the given stream
	 */
	std::ostream &operator<<(std::ostream &stream, Printable &p);
	/**
	 * \brief Easier printing of the object
	 * Internally the function Printable::print is called.
	 *
	 * \param stream The output stream
	 * \param p The Printable
	 * \return Returns a reference to the given stream
	 */
	std::ostream &operator<<(std::ostream &stream, Printable *p);

}

#endif

