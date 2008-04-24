#ifndef GRINGOLPARSELEXER_H
#define GRINGOLPARSELEXER_H

#include <gringo.h>

namespace NS_GRINGO
{
	class GrinGoLexer
	{
	public:
		GrinGoLexer(std::istream *in = &std::cin);
		std::string getToken();
		void reset(std::istream *i = &std::cin);
		int getLine();
		void step();
		int getColumn();
		virtual ~GrinGoLexer();

	protected:
		void fill(int n);

	protected:
		std::istream *in;
		int bufmin;
		int bufsize;
		char *buffer, *start, *offset, *cursor, *limit, *marker, *eof;
		unsigned int line;
	};
}

#endif

