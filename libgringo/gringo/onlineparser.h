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

#ifndef ONLINEPARSER_H
#define ONLINEPARSER_H

#include <gringo/gringo.h>
#include <gringo/claspoutput.h>
#include <gringo/gringoparser.h>
#include <gringo/onlinelexer.h>

namespace gringo
{
	class OnlineParser : public GrinGoParser
	{
	public:
		OnlineParser(Grounder *g, std::vector<std::istream*> &in);
		OnlineParser(Grounder *g, std::istream* = &std::cin);
		OnlineParser(std::istream* = &std::cin);
		bool parse(NS_OUTPUT::Output *output);
		void addFact(NS_OUTPUT::Atom *atom);
		void addIntegrity(NS_OUTPUT::Integrity *integrity);
		void addRule(NS_OUTPUT::Rule *rule);
		void addDeltaObject(NS_OUTPUT::Object* object);
		void setCumulative();
		void setVolatile();
		GrinGoLexer *getLexer();
		Grounder *getGrounder() { return grounder_; }
		void setStep(int step);
		void terminate();
		bool isTerminated();
		int addSignature();
		virtual ~OnlineParser();
	private:
		OnlineLexer* lexer_;
		Grounder* grounder_;
		NS_OUTPUT::IClaspOutput* output_;
		std::vector<std::istream*> streams_;
		bool stepped_;
		bool volatile_;
		bool terminated_;
		void *pParser;
	};
}
#endif

