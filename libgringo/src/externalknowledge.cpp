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

#include <gringo/externalknowledge.h>
#include <gringo/grounder.h>
#include <gringo/onlineparser.h>

using namespace gringo;

ExternalKnowledge::ExternalKnowledge() {
	step_ = 0;
	socket_ = NULL;
	debug_ = false;
	port_ = 25277;
}

ExternalKnowledge::~ExternalKnowledge() {
	if(socket_) {
		socket_->close();
		delete socket_;
	}
}

void ExternalKnowledge::initialize(NS_OUTPUT::Output* output) {
	output_ = static_cast<NS_OUTPUT::IClaspOutput*>(output);
}

void ExternalKnowledge::addExternal(GroundAtom external, int uid) {
	external_preds_.insert(external);
	externals_.insert(uid);
}

void ExternalKnowledge::startSocket(int port) {
	using boost::asio::ip::tcp;

	if(debug_) std::cerr << "Starting socket..." << std::endl;

	try {
		tcp::acceptor acceptor(io_service_, tcp::endpoint(tcp::v4(), port));
		socket_ = new tcp::socket(io_service_);
		acceptor.accept(*socket_);

		if(debug_) std::cerr << "Client connected..." << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "Warning: " << e.what() << std::endl;
	}
}

void ExternalKnowledge::sendModel(std::string model) {
	std::stringstream ss;
	ss << "Step: " << step_ << "\n" << model;

	sendToClient(ss.str());
}

void ExternalKnowledge::sendToClient(std::string msg) {
	if(not socket_) startSocket(port_);

	try {
		boost::asio::write(*socket_, boost::asio::buffer(msg+char(0)), boost::asio::transfer_all());
	}
	catch (std::exception& e) {
		std::cerr << "Warning: Could not send answer set. " << e.what() << std::endl;
	}
}

bool ExternalKnowledge::get(gringo::Grounder* grounder) {
	debug_ = grounder->options().debug;
	if(debug_) std::cerr << "Getting external knowledge..." << std::endl;

	sendToClient("Input:\n");
	
	boost::asio::streambuf b;
	
	try {
		boost::system::error_code error;

		boost::asio::read_until(*socket_, b, char(0), error);
	
		if(error == boost::asio::error::eof)
			throw gringo::GrinGoException("Connection closed cleanly by client.");
		else if(error)
			throw boost::system::system_error(error);
	}
	catch (std::exception& e) {
		std::cerr << "Warning: " << e.what() << std::endl;
	}

	std::istream is(&b);
	OnlineParser parser(grounder, &is);

	if(!parser.parse(output_))
		throw gringo::GrinGoException("Parsing failed.");

	if(parser.isTerminated())
		return false;
	else
		return true;
}

bool ExternalKnowledge::checkFact(NS_OUTPUT::Object* object) {
	assert(dynamic_cast<NS_OUTPUT::Atom*>(object));
	NS_OUTPUT::Atom* atom = static_cast<NS_OUTPUT::Atom*>(object);
	std::pair<int, ValueVector> pred = std::make_pair(atom->predUid_, atom->values_);

	// try to find atom in external predicates
	if(external_preds_.find(pred) != external_preds_.end() ||
	   external_preds_old_.find(pred) != external_preds_old_.end()) {
		return true;
	}
	return false;
}

bool ExternalKnowledge::addNewFact(NS_OUTPUT::Object* fact, int line=0) {
	if(facts_old_.find(fact->uid_) != facts_old_.end()) {
		std::stringstream error_msg;
		error_msg << "Warning: Fact in line " << line << " was already added." << std::endl;
		std::cerr << error_msg.str() << std::endl;
		sendToClient(error_msg.str());
		return false;
	}

	facts_.insert(fact->uid_);
	externals_.erase(fact->uid_);
	return true;
}

IntSet* ExternalKnowledge::getAssumptions() {
	return &externals_old_;
}

void ExternalKnowledge::endStep() {
	for(IntSet::iterator i = facts_.begin(); i != facts_.end(); ++i) {
		if(externals_old_.erase(*i) > 0) {
			// fact was declared external earlier -> needs unfreezing
			output_->unfreezeAtom(*i);
		}
	}

	for(IntSet::iterator i = externals_.begin(); i != externals_.end(); ++i) {
		// freeze new external atoms
		output_->printExternalRule(*i);
	}

	step_++;

	// TODO forgetExternals
	bool unfreeze_old_externals_ = false;

	if(unfreeze_old_externals_) {
		for(IntSet::iterator i = externals_old_.begin(); i != externals_old_.end(); ++i) {
			output_->unfreezeAtom(*i);
		}
		external_preds_old_.swap(external_preds_);
		externals_old_.swap(externals_);
	}
	else {
		external_preds_old_.insert(external_preds_.begin(), external_preds_.end());
		externals_old_.insert(externals_.begin(), externals_.end());
	}
	external_preds_.clear();
	externals_.clear();

	facts_old_.insert(facts_.begin(), facts_.end());
	facts_.clear();
}
