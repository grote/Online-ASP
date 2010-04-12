// Copyright (c) 2010, Torsten Grote
// Copyright (c) 2008, Roland Kaminski
//
// This file is part of GrinGo.
//
// GrinGo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrinGo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrinGo.  If not, see <http://www.gnu.org/licenses/>.

#include <gringo/onlineparser.h>
#include "onlineparser_impl.h"
#include <gringo/onlinelexer.h>
#include <gringo/grounder.h>

using namespace gringo;

void *onlineparserAlloc(void *(*mallocProc)(size_t));
void onlineparserFree(void *p, void (*freeProc)(void*));
void onlineparser(void *yyp, int yymajor, std::string* yyminor, OnlineParser *pParser);

OnlineParser::OnlineParser(Grounder *g, std::istream* in) : GrinGoParser(), grounder_(g), output_(0)
{
	lexer_  = new OnlineLexer();
		pParser = onlineparserAlloc (malloc);
	streams_.push_back(in);
	terminated_ = false;
}

OnlineParser::OnlineParser(std::istream* in) : GrinGoParser(), output_(0)
{
	lexer_  = new OnlineLexer();
		pParser = onlineparserAlloc (malloc);
	streams_.push_back(in);
	grounder_ = NULL; // TODO
	terminated_ = false;
}

OnlineParser::OnlineParser(Grounder *g, std::vector<std::istream*> &in) : GrinGoParser(), grounder_(g), output_(0)
{
	lexer_  = new OnlineLexer();
		pParser = onlineparserAlloc (malloc);
	streams_ = in;
	terminated_ = false;
}

bool OnlineParser::parse(NS_OUTPUT::Output *output)
{
	output_ = static_cast<NS_OUTPUT::IClaspOutput*>(output);
	int token;
	std::string *lval;
	if(grounder_)
		grounder_->setOutput(output);
	for(std::vector<std::istream*>::iterator it = streams_.begin(); it != streams_.end(); it++)
	{
		lexer_->reset(*it);
		token = lexer_->lex(lval);
		while(token != ONLINEPARSER_EOI)
		{
			onlineparser(pParser, token, lval, this);
			token = lexer_->lex(lval);
		}
	}
	onlineparser(pParser, 0, lval, this);
	if(getError())
		return false;
	return true;
}

void OnlineParser::addExternal(NS_OUTPUT::Fact* fact) {
	if(!output_->getExternalKnowledge()->checkExternal(fact->head_)) {
		error_ = true;
		std::cerr << "Error: Fact in line " << getLexer()->getLine() << " was not declared external.\n";
		return;
	}
	
	// add Atom to get a uid assigned to it
	output_->addAtom(static_cast<NS_OUTPUT::Atom*>(fact->head_));
	
	output_->getExternalKnowledge()->addNewFact(fact->head_);
	
	// add fact to program and assign a uid to it
	output_->print(fact);
}

void OnlineParser::terminate() {
	terminated_ = true;
}

OnlineParser::~OnlineParser()
{
	delete lexer_;
	onlineparserFree(pParser, free);
	if(not terminated_) {
//		throw gringo::GrinGoException("Did not read \"#endstep.\" or \"#stop.\".");
	}
}

GrinGoLexer *OnlineParser::getLexer()
{
	return lexer_;
}
