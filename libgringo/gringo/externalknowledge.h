// Copyright (c) 2010, Torsten Grote
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

#ifndef EXTERNALKNOWLEDGE_H
#define EXTERNALKNOWLEDGE_H

#include <boost/asio.hpp>
#include <gringo/gringo.h>
#include <gringo/gringoexception.h>
#include <gringo/value.h>

namespace gringo
{
	namespace NS_OUTPUT
	{
		class IClaspOutput;
	}

	class ExternalKnowledge
	{
	public:
		ExternalKnowledge();
		virtual ~ExternalKnowledge();
		void initialize(NS_OUTPUT::Output* output);
		void addExternal(GroundAtom external, int uid);
		void startSocket(int port);
		void sendModel(std::string);
		void sendToClient(std::string msg);
		bool get(gringo::Grounder* grounder);
		bool checkFact(NS_OUTPUT::Object* object);
		bool addNewFact(NS_OUTPUT::Object* fact, int line);
		IntSet* getAssumptions();
		void endStep();

	private:
		UidValueSet external_preds_;
		UidValueSet external_preds_old_;
		IntSet externals_;
		IntSet externals_old_;
		IntSet facts_;
		IntSet facts_old_;
		NS_OUTPUT::IClaspOutput* output_;
		boost::asio::ip::tcp::socket* socket_;
		bool socket_started_;
		int step_;
		int port_;
		bool debug_;
		boost::asio::io_service io_service_;
	};
}

#endif
