#include "lparseparser.h"
#include "lparseparser_impl.h"
#include "lparselexer.h"
#include "grounder.h"

using namespace NS_GRINGO;

void *lparseparserAlloc(void *(*mallocProc)(size_t));
void lparseparserFree(void *p, void (*freeProc)(void*));
void lparseparser(void *yyp, int yymajor, std::string* yyminor, LparseParser *pParser);

LparseParser::LparseParser(std::istream* in) : GrinGoParser(), grounder_(0)
{
	lexer_  = new LparseLexer();
        pParser = lparseparserAlloc (malloc);
	streams_.push_back(in);
}

LparseParser::LparseParser(std::vector<std::istream*> &in) : GrinGoParser()
{
	lexer_  = new LparseLexer();
        pParser = lparseparserAlloc (malloc);
	streams_ = in;
}

bool LparseParser::parse(NS_OUTPUT::Output *output)
{
	int token;
	std::string *lval;
	grounder_ = new Grounder(output);
	for(std::vector<std::istream*>::iterator it = streams_.begin(); it != streams_.end(); it++)
	{
		lexer_->reset(*it);
		while((token = lexer_->lex(lval)) != LPARSEPARSER_EOI)
		{
			lparseparser(pParser, token, lval, this);
		}
	}
	lparseparser(pParser, 0, lval, this);
	if(getError())
	{
		delete grounder_;
		grounder_ = 0;
		return false;
	}
	return true;
}

LparseParser::~LparseParser()
{
	delete lexer_;
        lparseparserFree(pParser, free);
}

GrinGoLexer *LparseParser::getLexer()
{
	return lexer_;
}

