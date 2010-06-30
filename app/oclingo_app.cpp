// 
// Copyright (c) 2010, Torsten Grote
// Copyright (c) 2009, Benjamin Kaufmann
// 
// This file is part of gringo. See http://www.cs.uni-potsdam.de/gringo/ 
// 
// gringo is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// gringo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with gringo; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "oclingo_app.h"
#include <gringo/lparseparser.h>
#include <gringo/claspoutput.h>

using namespace std;
namespace gringo {

namespace {
// class for using gringos output as clasps input
struct oClingoBridge : public Clasp::Input {
	typedef std::auto_ptr<gringo::Grounder> GrounderPtr;
	typedef std::auto_ptr<gringo::LparseParser> ParserPtr;
	typedef std::auto_ptr<NS_OUTPUT::Output> OutputPtr;
	typedef Clasp::MinimizeConstraint* MinConPtr;

	oClingoBridge(const GringoOptions& opts, Streams& str, bool clingoMode) : online(clingoMode) {
		grounder.reset(new gringo::Grounder(opts.grounderOptions));
		parser.reset(new gringo::LparseParser(grounder.get(), str.streams));
		if(online) {
			out.reset(new NS_OUTPUT::IClaspOutput(0, opts.shift));
		} else {
			out.reset(new NS_OUTPUT::ClaspOutput(0, opts.shift));
		}
	}

	Format format() const { return Clasp::Input::SMODELS; }

	MinConPtr getMinimize(Clasp::Solver& s, Clasp::ProgramBuilder* api, bool heu) {
		return api ? api->createMinimize(s, heu) : 0;
	}

	void getAssumptions(Clasp::LitVec& a) {
		(void)a;

		if (online) {
			const Clasp::AtomIndex& i = *solver->strategies().symTab.get();
			a.push_back(i.find(static_cast<NS_OUTPUT::IClaspOutput*>(out.get())->getIncUid())->lit);

			// make external facts false
			IntSet assumptions = out.get()->getExternalKnowledge()->getAssumptions();

			for(IntSet::iterator ass = assumptions.begin(); ass != assumptions.end(); ++ass) {
				Clasp::Atom* atom = i.find(*ass);
				if(atom) { // atom is not in AtomIndex if hidden with #hide
					a.push_back(~atom->lit);
					// create conflict to skip solving for next step
					if(out.get()->getExternalKnowledge()->controllerNeedsNewStep())
						a.push_back(atom->lit);
				}
			}
		}
	}

	bool read(Clasp::Solver& s, Clasp::ProgramBuilder* api, int) {
		static_cast<NS_OUTPUT::ClaspOutput*>(out.get())->setProgramBuilder(api);
		solver = &s;
		if (parser.get()) {
			if (!parser->parse(out.get())) throw gringo::GrinGoException("Error: Parsing failed.");
			grounder->prepare(true);
			parser.reset(0);
		}

		if(online) {
			ExternalKnowledge* ext = out.get()->getExternalKnowledge();

			if(solver->hasConflict()) {
				ext->sendToClient("Error: The solver detected a conflict, so program is not satisfiable anymore.");
			}
			else {
				// add new facts and check for termination condition
				if(!ext->addInput())
					return false; // exit if received #stop.

				// do new step if there's no model or controller needs new step
				if(!ext->hasModel() || ext->needsNewStep()) {
					grounder->ground();
					ext->endStep();
				} else {
					ext->endIteration();
				}
			}
			ext->get();
		} else {
			grounder->ground();
			grounder.reset(0);
			out.reset(0);
		}
		return true;
	}

	GrounderPtr            grounder;
	ParserPtr              parser;
	OutputPtr              out;
	Clasp::Solver*         solver;
	bool online;
};
}

void oClingoApp::configureInOut(Streams& s) {
	using namespace Clasp;
	in_.reset(0);
	facade_ = 0;
	if (clingo_.claspMode) {
		s.open(generic.input);
		if (generic.input.size() > 1) { messages.warning.push_back("Only first file will be used"); }
		in_.reset(new StreamInput(*s.streams[0], detectFormat(*s.streams[0])));
	}
	else {
		GringoApp::addConstStream(s);
		s.open(generic.input);

		oClingoBridge* bridge = new oClingoBridge(opts, s, !clingo_.clingoMode);
		gringo_out_ = bridge->out.get();
		bridge->out.get()->setExternalKnowledge(new ExternalKnowledge(bridge->grounder.get(), gringo_out_, &solver_));
		in_.reset(bridge);
	}
	if (config_.onlyPre) {
		if (clingo_.claspMode || clingo_.clingoMode) {
			generic.verbose = 0;
		}
		else { warning("Option '--pre' is ignored in incremental setting!"); config_.onlyPre = false; }
	}
	if (in_->format() == Input::SMODELS) {
		out_.reset(new AspOutput(cmdOpts_.basic.asp09));
		if (cmdOpts_.basic.asp09){ generic.verbose = 0; }
	}
	else if (in_->format() == Input::DIMACS) { out_.reset(new SatOutput()); }
	else if (in_->format() == Input::OPB)    { out_.reset(new PbOutput(generic.verbose > 1));  }
}

int oClingoApp::doRun() {
	using namespace Clasp;
	if (opts.onlyGround) {
		opts.stats = cmdOpts_.basic.stats != 0;
		return GringoApp::doRun();
	}
	if (cmdOpts_.basic.stats > 1) {
		solver_.stats.solve.enableJumpStats();
	}
	Streams s;
	configureInOut(s);
	ClaspFacade clasp; facade_ = &clasp;
	timer_[0].start();
	if (clingo_.claspMode || clingo_.clingoMode) {
		clingo_.iStats = false;
		clasp.solve(*in_, config_, this);
	}
	else {
		clasp.solveIncremental(*in_, config_, clingo_.online, this);
	}
	timer_[0].stop();
	printResult(reason_end);
	if      (clasp.result() == ClaspFacade::result_unsat) return S_UNSATISFIABLE;
	else if (clasp.result() == ClaspFacade::result_sat)   return S_SATISFIABLE;
	else                                                  return S_UNKNOWN;
}

void oClingoApp::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f) {
	ClingoApp::state(e, f);

	if(!clingo_.claspMode && !clingo_.clingoMode) {
		// make sure we are not adding new input while doing propagation in preprocessing
		if(f.state() == Clasp::ClaspFacade::state_solve && e == Clasp::ClaspFacade::event_state_enter) {
			gringo_out_->getExternalKnowledge()->addPostPropagator();
		}
		else if(f.state() == Clasp::ClaspFacade::state_solve && e == Clasp::ClaspFacade::event_state_exit) {
			gringo_out_->getExternalKnowledge()->removePostPropagator();
		}
	}
}

void oClingoApp::event(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f) {
	ClingoApp::event(e, f);

	if(!clingo_.claspMode && !clingo_.clingoMode && e == Clasp::ClaspFacade::event_model) {
		string model = "";
		assert(solver_.strategies().symTab.get());
		const Clasp::AtomIndex& index = *solver_.strategies().symTab;
		for (Clasp::AtomIndex::const_iterator it = index.begin(); it != index.end(); ++it) {
			if(solver_.value(it->second.lit.var()) == trueValue(it->second.lit) && !it->second.name.empty()) {
				model += it->second.name + " ";
			}
		}
		gringo_out_->getExternalKnowledge()->sendModel(model);
	}
}


}
