#ifndef LPARSELEXER_H
#define LPARSELEXER_H

#include <gringo.h>
#include <gringolexer.h>

namespace NS_GRINGO
{
	class LparseLexer : public GrinGoLexer
	{
	public:
		LparseLexer(std::istream *in = &std::cin);
		int lex(std::string *&val);
		virtual ~LparseLexer();
	};
}

#endif

