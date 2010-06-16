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

ExternalKnowledge::ExternalKnowledge(Grounder* grounder, NS_OUTPUT::Output* output, bool keep_externals) {
	output_ = static_cast<NS_OUTPUT::IClaspOutput*>(output);
	grounder_ = grounder;
	solver_ = NULL;
	keep_externals_ = keep_externals;

	externals_per_step_.push_back(IntSet());
	externals_per_step_.push_back(IntSet());

	socket_ = NULL;
	port_ = 25277;
	reading_ = false;
	new_input_ = false;

	post_ = new ExternalKnowledge::PostPropagator(this);
	solver_stopped_ = false;

	step_ = 1;
	controller_step_ = 0;
	model_ = false;
	debug_ = grounder_->options().debug;
}

ExternalKnowledge::~ExternalKnowledge() {
	if(socket_) {
		socket_->close();
		delete socket_;
	}
	// don't delete post_ because it now belongs to solver
}

void ExternalKnowledge::initialize(Clasp::Solver* s) {
	if(not solver_) {
		solver_ = s;
		solver_->addPost(post_);
	}
}

void ExternalKnowledge::addExternal(GroundAtom external, int uid) {
	externals_.insert(std::make_pair(external, uid));
	externals_per_step_.at(step_).insert(uid);
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
	if(result) {
		std::cerr << "Polled for input and started " << result << " handler." << std::endl;
		if(solver_stopped_) {
			// solver was already stopped, don't try to stop it again
			result = 0;
		}
		solver_stopped_ = true;
	}
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
	// solver can be stopped again
	solver_stopped_ = false;
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

	if(!new_input_ && model_ && step_ > 1) {
		io_service_.reset();
		io_service_.run_one();
	}

	if(new_input_) {
		new_input_ = false;

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

void ExternalKnowledge::addNewFact(NS_OUTPUT::Atom* atom, int line=0) {
	addNewAtom(atom, line);

	// add fact to program
	NS_OUTPUT::Fact* fact = new NS_OUTPUT::Fact(atom);
	output_->print(fact);

	delete fact;
}

void ExternalKnowledge::addNewAtom(NS_OUTPUT::Object* object, int line=0) {
	assert(dynamic_cast<NS_OUTPUT::Atom*>(object));
	NS_OUTPUT::Atom* atom = static_cast<NS_OUTPUT::Atom*>(object);

	// add Atom to get a uid assigned to it
	output_->addAtom(atom);

	if(facts_old_.find(atom->uid_) != facts_old_.end()) {
		std::stringstream error_msg;
		error_msg << "Warning: Fact in line " << line << " was already added." << std::endl;
		std::cerr << error_msg.str() << std::endl;
		sendToClient(error_msg.str());
	}
	else {
		facts_.insert(atom->uid_);
		eraseUidFromExternals(&externals_, atom->uid_);
	}
}

void ExternalKnowledge::addPrematureFact(NS_OUTPUT::Atom* atom) {
	premature_facts_.push_back(atom);
}

bool ExternalKnowledge::needsNewStep() {
	return !premature_facts_.empty() ||	// has facts waiting to be added
			step_ == 1 ||				// is first iteration
			controller_step_ >= step_;	// controller wants to progress step count
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

	// set model to false not only after completed step, but also after iterations
	model_ = false;

	std::cerr << "Controller Step: " << controller_step_ << std::endl;
}

void ExternalKnowledge::endStep() {
	// add previously added premature facts
	for(std::vector<NS_OUTPUT::Atom*>::iterator i = premature_facts_.begin(); i!= premature_facts_.end(); ++i) {
		if(checkFact(*i)) {
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
		// freeze new external atoms, facts have been deleted already in addNewAtom()
		output_->printExternalRule(i->second, i->first.first);
	}

	// unfreeze old externals for simplification by clasp
	if(!keep_externals_) {
		for(IntSet::iterator i = externals_per_step_.at(controller_step_).begin(); i != externals_per_step_.at(controller_step_).end(); ++i) {
			if(eraseUidFromExternals(&externals_old_, *i) > 0) {
				// external was still old, so needs unfreezing
				output_->unfreezeAtom(*i);
			}
		}
		externals_per_step_.at(controller_step_).clear();
	}
	externals_old_.insert(externals_.begin(), externals_.end());
	externals_.clear();

	step_++;
	externals_per_step_.push_back(IntSet());
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

void ExternalKnowledge::endControllerStep() {
	controller_step_++;
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
