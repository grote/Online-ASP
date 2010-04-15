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

	externals_.push_back(UidValueSet());
	externals_uids_.push_back(IntSet());
	new_facts_.push_back(IntSet());

	socket_ = NULL;
	debug_ = false;
}

void ExternalKnowledge::initialize(NS_OUTPUT::Output* output) {
	output_ = static_cast<NS_OUTPUT::IClaspOutput*>(output);
}

void ExternalKnowledge::addExternal(GroundAtom external, int uid) {
	externals_.at(step_).insert(external);
	externals_uids_.at(step_).insert(uid);
}

bool ExternalKnowledge::checkExternal(NS_OUTPUT::Object* object) {
	assert(dynamic_cast<NS_OUTPUT::Atom*>(object));
	NS_OUTPUT::Atom* atom = static_cast<NS_OUTPUT::Atom*>(object);

	// try to find atom in externals
	for(std::vector<UidValueSet>::iterator i = externals_.begin(); i != externals_.end(); ++i) {
		if(i->find(std::make_pair(atom->predUid_, atom->values_)) != i->end())
			return true;
	}
	return false;
}

IntSet* ExternalKnowledge::getExternalsUids() {
	return &externals_uids_.at(step_);
}

void ExternalKnowledge::storeModel(std::string model) {
	model_ = model;
}

bool ExternalKnowledge::get(gringo::Grounder* grounder) {
	debug_ = grounder->options().debug;
	if(debug_) std::cerr << "Getting external knowledge..." << std::endl;

	if(not socket_) startSocket(25277);
	
	boost::asio::streambuf b;
	
	try {
		boost::system::error_code error;
		boost::asio::write(*socket_, boost::asio::buffer(model_+char(0)), boost::asio::transfer_all(), error);

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

void ExternalKnowledge::startSocket(int port) {
	using boost::asio::ip::tcp;

	if(debug_) std::cerr << "Starting socket..." << std::endl;

	try {
		boost::asio::io_service io_service;

		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
		socket_ = new tcp::socket(io_service);
		acceptor.accept(*socket_);
	}
	catch (std::exception& e) {
		std::cerr << "Warning: " << e.what() << std::endl;
	}
}

void ExternalKnowledge::addNewFact(NS_OUTPUT::Object* fact) {
	new_facts_.at(step_).insert(fact->uid_);
}

IntSet* ExternalKnowledge::getAssumptions() {
	IntSet* result = new IntSet(externals_uids_.at(step_-1));

	// TODO return externals from all steps
	for(IntSet::iterator i = new_facts_.at(step_-1).begin(); i != new_facts_.at(step_-1).end(); ++i) {
		result->erase(*i);
	}

	return result;
}

void ExternalKnowledge::endStep() {
	for(IntSet::iterator i = new_facts_.at(step_).begin(); i != new_facts_.at(step_).end(); ++i) {
		if(externals_uids_.at(step_).erase(*i) == 0) {
			output_->unfreezeAtom(*i);
		}
	}

	for(IntSet::iterator i = externals_uids_.at(step_).begin(); i != externals_uids_.at(step_).end(); ++i) {
		output_->printExternalRule(*i);
	}

	step_++;

	externals_.push_back(UidValueSet());
	externals_uids_.push_back(IntSet());
	new_facts_.push_back(IntSet());
}
