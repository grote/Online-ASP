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
		void initialize(NS_OUTPUT::Output* output);
		void addExternal(GroundAtom external, int uid);
		bool checkExternal(NS_OUTPUT::Object* object);
		IntSet* getExternalsUids();
		void storeModel(std::string);
		bool get(gringo::Grounder* grounder);
		void startSocket(int port);
		void addNewFact(NS_OUTPUT::Object* fact);
		IntSet* getAssumptions();
		void endStep();

	private:
		std::vector<UidValueSet> externals_;
		std::vector<IntSet> externals_uids_;
		std::vector<IntSet> new_facts_;
		NS_OUTPUT::IClaspOutput* output_;
		boost::asio::ip::tcp::socket* socket_;
		bool socket_started_;
		int step_;
		std::string model_;
		bool debug_;
	};
}

#endif

