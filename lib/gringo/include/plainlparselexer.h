#ifndef PLAINLPARSELEXER_H
#define PLAINLPARSELEXER_H

#include <gringo.h>
#include <gringolexer.h>

namespace NS_GRINGO
{
	class PlainLparseLexer : public GrinGoLexer
	{
	public:
		PlainLparseLexer();
		int lex(std::string *&val);
		virtual ~PlainLparseLexer();
	};
}

#endif

