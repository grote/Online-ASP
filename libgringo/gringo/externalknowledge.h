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

#include <gringo/gringo.h>
#include <gringo/gringoexception.h>
#include <gringo/value.h>

#include <clasp/solver.h>
#include <clasp/constraint.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace gringo
{
	namespace NS_OUTPUT
	{
		class IClaspOutput;
		struct Fact;
	}

	class ExternalKnowledge
	{
	public:
		ExternalKnowledge(Grounder* grounder, NS_OUTPUT::Output* output, Clasp::Solver* solver);
		virtual ~ExternalKnowledge();
		void addPostPropagator();
		void removePostPropagator();
		void addExternal(GroundAtom external, int uid);
		void startSocket(int port);
		void sendModel(std::string);
		bool hasModel();
		void sendToClient(std::string msg);
		int poll();
		void get();
		void readUntilHandler(const boost::system::error_code& e, size_t bytesT);
		bool addInput();
		bool checkFact(NS_OUTPUT::Object* object);
		void addNewFact(NS_OUTPUT::Atom* atom, int line);
		void addNewAtom(NS_OUTPUT::Object* object, int line);
		void addPrematureFact(NS_OUTPUT::Atom* atom);
		bool needsNewStep();
		bool controllerNeedsNewStep();
		IntSet getAssumptions();
		void endIteration();
		void endStep();
		int eraseUidFromExternals(UidValueMap* ext, int uid);
		void setControllerStep(int step);
	protected:
		struct PostPropagator : public Clasp::PostPropagator {
		public:
			PostPropagator(ExternalKnowledge* ext);
			bool propagate(Clasp::Solver &s);
			uint32 priority() const { return Clasp::PostPropagator::priority_lowest - 1; }
		private:
			ExternalKnowledge* ext_;
		};

	private:
		NS_OUTPUT::IClaspOutput* output_;
		Grounder* grounder_;
		Clasp::Solver* solver_;

		// external fact handling
		UidValueMap externals_;
		UidValueMap externals_old_;
		std::vector<IntSet> externals_per_step_;
		IntSet facts_;
		IntSet facts_old_;
		std::vector<NS_OUTPUT::Atom*> premature_facts_;

		// socket stuff
		boost::asio::io_service io_service_;
		boost::asio::ip::tcp::socket* socket_;
		boost::asio::streambuf b_;
		int port_;
		bool reading_;
		bool new_input_;

		PostPropagator* post_;
		bool my_post_;
		bool solver_stopped_;

		int step_;
		int controller_step_;
		bool model_;
		bool debug_;
	};
}

#endif

