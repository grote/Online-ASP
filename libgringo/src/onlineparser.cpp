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
	stepped_ = false;
	volatile_ = false;
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

		// stop at ENDSTEP because socket stream will not return EOI
		while(token != ONLINEPARSER_ENDSTEP)
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

void OnlineParser::addFact(NS_OUTPUT::Atom* atom) {
	if(output_->getExternalKnowledge()->checkFact(atom)) {
		output_->getExternalKnowledge()->addNewFact(atom, getLexer()->getLine());
	}
	else if(output_->getExternalKnowledge()->controllerNeedsNewStep()) {
		// hold fact back, because it will be declared external in the next step
		output_->getExternalKnowledge()->addPrematureFact(atom);
	}
	else {
		std::stringstream warning_msg;
		warning_msg << "Warning: Fact in line " << getLexer()->getLine() << " has not been declared external and was not added.\n";

		std::cerr << warning_msg.str() << std::endl;
		output_->getExternalKnowledge()->sendToClient(warning_msg.str());
	}
}

void OnlineParser::addIntegrity(NS_OUTPUT::Integrity* integrity) {
	if(volatile_) {
		addDeltaObject(integrity->body_);
	}

	output_->print(integrity);
}

void OnlineParser::addRule(NS_OUTPUT::Rule* rule) {
	if(output_->getExternalKnowledge()->checkFact(rule->head_)) {
		output_->getExternalKnowledge()->addNewAtom(rule->head_, getLexer()->getLine());
	}

	if(volatile_) {
		addDeltaObject(rule->body_);
	}

	output_->print(rule);
}

void OnlineParser::addDeltaObject(NS_OUTPUT::Object* object) {
	assert(dynamic_cast<NS_OUTPUT::Conjunction*>(object));
	NS_OUTPUT::Conjunction* body = static_cast<NS_OUTPUT::Conjunction*>(object);

	NS_OUTPUT::DeltaObject* delta = new NS_OUTPUT::DeltaObject();
	body->lits_.push_back(delta);
}

void OnlineParser::setCumulative() {
	volatile_ = false;
}

void OnlineParser::setVolatile() {
	volatile_ = true;
}

void OnlineParser::setStep(int step) {
	if(stepped_) {
		std::stringstream warning_msg;
		warning_msg << "Warning: New '#step " << step << ".' without prior '#endstep.' encountered. Ignoring....\n";

		std::cerr << warning_msg.str() << std::endl;
		output_->getExternalKnowledge()->sendToClient(warning_msg.str());
	} else {
		stepped_ = true;
		output_->getExternalKnowledge()->setControllerStep(step);
	}
}

void OnlineParser::terminate() {
	terminated_ = true;
}

bool OnlineParser::isTerminated() {
	return terminated_;
}

int OnlineParser::addSignature() {
	output_->addSignature();
	return 0;
}

OnlineParser::~OnlineParser()
{
	delete lexer_;
	onlineparserFree(pParser, free);
}

GrinGoLexer *OnlineParser::getLexer()
{
	return lexer_;
}
