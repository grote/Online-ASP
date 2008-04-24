#ifndef GRINGOPARSER_H
#define GRINGOPARSER_H

#include <gringo.h>

namespace NS_GRINGO
{
	class GrinGoParser 
	{
	public:
		GrinGoParser() : error_(false), grounder_(0) {} ;
		virtual ~GrinGoParser() {}
		virtual GrinGoLexer *getLexer() = 0;
		virtual void handleError();
		virtual bool getError();
		Grounder *getGrounder() { return grounder_; }
		virtual Grounder *parse() = 0;
	protected:
		bool error_;
		Grounder *grounder_;
	};
}

#endif

