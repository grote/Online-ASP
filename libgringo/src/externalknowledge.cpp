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
	output_ = NULL;
	grounder_ = NULL;
	solver_ = NULL;

	socket_ = NULL;
	port_ = 25277;
	reading_ = false;
	new_input_ = false;

	post_ = new ExternalKnowledge::PostPropagator(this);
	solver_stopped_ = false;

	step_ = 0;
	model_ = false;
	debug_ = false;
}

ExternalKnowledge::~ExternalKnowledge() {
	if(socket_) {
		socket_->close();
		delete socket_;
	}
	// don't delete post_ because it now belongs to solver
}

void ExternalKnowledge::initialize(NS_OUTPUT::Output* output, Grounder* grounder, Clasp::Solver* s) {
	if(not output_)
		output_ = static_cast<NS_OUTPUT::IClaspOutput*>(output);
	if(not grounder_) {
		grounder_ = grounder;
		debug_ = grounder_->options().debug;
	}
	if(not solver_) {
		solver_ = s;
		solver_->addPost(post_);
	}
}

void ExternalKnowledge::addExternal(GroundAtom external, int uid) {
	externals_.insert(std::make_pair(external, uid));
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

	model_ = true;
}

bool ExternalKnowledge::hasModel() {
	return model_;
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

int ExternalKnowledge::poll() {
	io_service_.reset();
	int result = io_service_.poll_one();
	if(result)
		std::cerr << "Polled for input and started " << result << " handler." << std::endl;

	return result;
}

void ExternalKnowledge::get() {
	try {
		if(!reading_) {
			if(debug_) std::cerr << "Getting external knowledge..." << std::endl;

			sendToClient("Input:\n");

			std::cerr << "Reading from socket..." << std::endl;
			reading_ = true;
			boost::asio::async_read_until(*socket_, b_, char(0), boost::bind(&ExternalKnowledge::readUntilHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}
	catch (std::exception& e) {
		std::cerr << "Warning: " << e.what() << std::endl;
	}
}

void ExternalKnowledge::readUntilHandler(const boost::system::error_code& e, size_t bytesT) {
	if (!e)	{
		new_input_ = true;
		reading_ = false;
	}
	else if(e == boost::asio::error::eof)
		throw gringo::GrinGoException("Connection closed cleanly by client.");
	else
		throw boost::system::system_error(e);
}

bool ExternalKnowledge::addInput() {
	if(model_)
		sendToClient("End of Step.\n");

	if(!new_input_ && model_ && step_) {
		io_service_.reset();
		io_service_.run_one();
	}

	if(new_input_) {
		new_input_ = false;
		model_ = false;

		std::istream is(&b_);
		OnlineParser parser(grounder_, &is);

		if(!parser.parse(output_))
			throw gringo::GrinGoException("Parsing failed.");

		if(parser.isTerminated()) {
			socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
			return false;
		}
	}
	return true;
}

bool ExternalKnowledge::checkFact(NS_OUTPUT::Object* object) {
	assert(dynamic_cast<NS_OUTPUT::Atom*>(object));
	NS_OUTPUT::Atom* atom = static_cast<NS_OUTPUT::Atom*>(object);
	std::pair<int, ValueVector> pred = std::make_pair(atom->predUid_, atom->values_);

	// try to find atom in external predicates
	if(externals_.find(pred) != externals_.end() ||
	   externals_old_.find(pred) != externals_old_.end()) {
		return true;
	}
	return false;
}

void ExternalKnowledge::addNewFact(NS_OUTPUT::Fact* fact, int line=0) {
	// add Atom to get a uid assigned to it
	output_->addAtom(static_cast<NS_OUTPUT::Atom*>(fact->head_));

	if(facts_old_.find(fact->head_->uid_) != facts_old_.end()) {
		std::stringstream error_msg;
		error_msg << "Warning: Fact in line " << line << " was already added." << std::endl;
		std::cerr << error_msg.str() << std::endl;
		sendToClient(error_msg.str());
	}
	else {
		facts_.insert(fact->head_->uid_);
		eraseUidFromExternals(&externals_, fact->head_->uid_);

		// add fact to program
		output_->print(fact);
	}
	delete fact;
}

void ExternalKnowledge::addPrematureFact(NS_OUTPUT::Fact* fact) {
	premature_facts_.push_back(fact);
}

bool ExternalKnowledge::hasFactsWaiting() {
	return !premature_facts_.empty();
}

bool ExternalKnowledge::isFirstIteration() {
	return !step_;
}

IntSet ExternalKnowledge::getAssumptions() {
	IntSet result;

	for(UidValueMap::iterator i = externals_old_.begin(); i != externals_old_.end(); ++i) {
		result.insert(i->second);
	}

	return result;
}

void ExternalKnowledge::endIteration() {
	for(IntSet::iterator i = facts_.begin(); i != facts_.end(); ++i) {
		if(eraseUidFromExternals(&externals_old_, *i) > 0) {
			// fact was declared external earlier -> needs unfreezing
			output_->unfreezeAtom(*i);
		}
	}

	facts_old_.insert(facts_.begin(), facts_.end());
	facts_.clear();
}

void ExternalKnowledge::endStep() {
	// add previously added premature facts
	for(std::vector<NS_OUTPUT::Fact*>::iterator i = premature_facts_.begin(); i!= premature_facts_.end(); ++i) {
		if(checkFact((*i)->head_)) {
			addNewFact(*i);
		} else {
			std::cerr << "Warning: Fact added last step was still not declared external and is now lost." << std::endl;
			sendToClient("Warning: Fact added last step was still not declared external and is now lost.");
			delete *i;
		}
	}
	premature_facts_.clear();

	endIteration();

	for(UidValueMap::iterator i = externals_.begin(); i != externals_.end(); ++i) {
		// freeze new external atoms, facts have been deleted already in addNewFact()
		output_->printExternalRule(i->second, i->first.first);
	}

	step_++;

	// TODO forgetOldExternals
	bool unfreeze_old_externals_ = false;

	if(unfreeze_old_externals_) {
		for(UidValueMap::iterator i = externals_old_.begin(); i != externals_old_.end(); ++i) {
			output_->unfreezeAtom(i->second);
		}
		externals_old_.swap(externals_);
	}
	else {
		externals_old_.insert(externals_.begin(), externals_.end());
	}
	externals_.clear();

}

// TODO is there any better way to do it?
int ExternalKnowledge::eraseUidFromExternals(UidValueMap* ext, int uid) {
	std::vector<UidValueMap::iterator> del;

	for(UidValueMap::iterator i = ext->begin(); i != ext->end(); ++i) {
		if(i->second == uid) {
			del.push_back(i);
		}
	}

	for(std::vector<UidValueMap::iterator>::iterator i = del.begin(); i != del.end(); ++i) {
		ext->erase(*i);
	}

	return del.size();
}


//////////////////////////////////////////////////////////////////////////////
// ExternalKnowledge::PostPropagator
//////////////////////////////////////////////////////////////////////////////

ExternalKnowledge::PostPropagator::PostPropagator(ExternalKnowledge* ext) {
	ext_ = ext;
}

bool ExternalKnowledge::PostPropagator::propagate(Clasp::Solver &s) {
	if(ext_->poll()) {
		std::cerr << "Stopping solver..." << std::endl;
		s.setStopConflict();
	}

	// TODO fix bug where iClingo stops and just UNSATISFIABLE is returned

	if(s.hasConflict())	{
		assert(s.hasConflict());
		return false;
	}
	return true;
}
