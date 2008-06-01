#ifndef LPARSECONVERTER_H
#define LPARSECONVERTER_H

#include <gringo.h>
#include <gringoparser.h>

namespace NS_GRINGO
{
	class LparseConverter : public GrinGoParser 
	{
	public:
		LparseConverter(std::vector<std::istream*> &in);
		LparseConverter(std::istream* = &std::cin);
		bool parse(NS_OUTPUT::Output *output);
		GrinGoLexer *getLexer();
		virtual ~LparseConverter();
	private:
		PlainLparseLexer *lexer_;
		NS_OUTPUT::Output *output_;
		std::vector<std::istream*> streams_;
		void *pParser;
	};
}
#endif

