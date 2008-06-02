#ifndef GRINGOPARSER_H
#define GRINGOPARSER_H

#include <gringo.h>

namespace NS_GRINGO
{
	class GrinGoParser 
	{
	public:
		GrinGoParser() : error_(false) {} ;
		virtual ~GrinGoParser() {}
		virtual GrinGoLexer *getLexer() = 0;
		virtual bool parse(NS_OUTPUT::Output *output) = 0;
		virtual void handleError();
		virtual bool getError();
	protected:
		bool error_;
	};
}

#endif

