#include "lparseconverter.h"
#include "lparseconverter_impl.h"
#include "plainlparselexer.h"
#include "grounder.h"
#include "funcsymbol.h"

using namespace NS_GRINGO;

void *lparseconverterAlloc(void *(*mallocProc)(size_t));
void lparseconverterFree(void *p, void (*freeProc)(void*));
void lparseconverter(void *yyp, int yymajor, std::string* yyminor, LparseConverter *pParser);

LparseConverter::LparseConverter(std::istream* in) : GrinGoParser(), output_(0)
{
	lexer_  = new PlainLparseLexer();
        pParser = lparseconverterAlloc (malloc);
	streams_.push_back(in);
}

LparseConverter::LparseConverter(std::vector<std::istream*> &in) : GrinGoParser()
{
	lexer_  = new PlainLparseLexer();
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
	output_->finalize();

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

NS_OUTPUT::Output *LparseConverter::getOutput()
{
	return output_;
}

int LparseConverter::createPred(std::string *id, int arity)
{
	std::pair<SignatureHash::iterator, bool> res = predHash_.insert(std::make_pair(Signature(id, arity), (int)pred_.size()));
	if(res.second)
	{
		pred_.push_back(Signature(id, arity));
		output_->addSignature();
	}
	return res.first->second;
}

