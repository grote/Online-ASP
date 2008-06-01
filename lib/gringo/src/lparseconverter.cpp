#include "lparseconverter.h"
#include "lparseconverter_impl.h"
#include "plainlparselexer.h"
#include "grounder.h"

using namespace NS_GRINGO;

void *lparseconverterAlloc(void *(*mallocProc)(size_t));
void lparseconverterFree(void *p, void (*freeProc)(void*));
void lparseconverter(void *yyp, int yymajor, std::string* yyminor, LparseConverter *pParser);

LparseConverter::LparseConverter(std::istream* in) : GrinGoParser(), output_(0)
{
	lexer_  = new PlainLparseLexer(0);
        pParser = lparseconverterAlloc (malloc);
	streams_.push_back(in);
}

LparseConverter::LparseConverter(std::vector<std::istream*> &in) : GrinGoParser()
{
	lexer_  = new PlainLparseLexer(0);
        pParser = lparseconverterAlloc (malloc);
	streams_ = in;
}

bool LparseConverter::parse(NS_OUTPUT::Output *output)
{
	output_ = output;
	int token;
	std::string *lval;
	for(std::vector<std::istream*>::iterator it = streams_.begin(); it != streams_.end(); it++)
	{
		lexer_->reset(*it);
		while((token = lexer_->lex(lval)) != LPARSECONVERTER_EOI)
		{
			lparseconverter(pParser, token, lval, this);
		}
	}
	lparseconverter(pParser, 0, lval, this);

	return !getError();
}

LparseConverter::~LparseConverter()
{
	delete lexer_;
        lparseconverterFree(pParser, free);
}

GrinGoLexer *LparseConverter::getLexer()
{
	return lexer_;
}

