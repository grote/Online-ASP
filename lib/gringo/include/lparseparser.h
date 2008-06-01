#ifndef LPARSEPARSER_H
#define LPARSEPARSER_H

#include <gringo.h>
#include <gringoparser.h>

namespace NS_GRINGO
{
	class LparseParser : public GrinGoParser 
	{
	public:
		LparseParser(std::vector<std::istream*> &in);
		LparseParser(std::istream* = &std::cin);
		Grounder *parse();
		GrinGoLexer *getLexer();
		Grounder *getGrounder() { return grounder_; }
		virtual ~LparseParser();
	private:
		LparseLexer *lexer_;
		Grounder *grounder_;
		std::vector<std::istream*> streams_;
		void *pParser;
	};
}
#endif

