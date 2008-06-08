#ifndef GRINGOEXCEPTION
#define GRINGOEXCEPTION

#include <gringo.h>
#include <exception>

namespace NS_GRINGO
{
	class GrinGoException : public std::exception
	{
	public:
		GrinGoException(const std::string &message) : message_(message)
		{
		}
		const char* what() const throw()
		{
			return message_.c_str();
		}
		virtual ~GrinGoException() throw()
		{
		}
	private:
		const std::string message_;
	};
}

#endif
