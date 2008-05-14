// 
// Copyright (c) 2006-2007, Benjamin Kaufmann
// 
// This file is part of Clasp. See http://www.cs.uni-potsdam.de/clasp/ 
// 
// Clasp is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Clasp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Clasp; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
#include <clasp/include/solve_algorithms.h>
#include <clasp/include/solver.h>
#include <clasp/include/smodels_constraints.h>
#include <cmath>
using std::log;
namespace Clasp { 

ModelPrinter::~ModelPrinter() {}
void ModelPrinter::printModel(Clasp::Solver&) {}
static ModelPrinter noPrinter_s;

/////////////////////////////////////////////////////////////////////////////////////////
// SolveParams
/////////////////////////////////////////////////////////////////////////////////////////
SolveParams::SolveParams() 
	: reduceBase_(3.0), reduceRInc_(1.1), reduceDInc_(1.0)
	, restartInc_(1.5)
	, randProp_(0.0)
	, printer_(&noPrinter_s)
	, restartBase_(100)
	, randRuns_(0), randConflicts_(0)
	, shuffleFirst_(0), shuffleNext_(0)
	, restartBounded_(false)
	, reduceOnRestart_(false) {
}

/////////////////////////////////////////////////////////////////////////////////////////
// Restarts
/////////////////////////////////////////////////////////////////////////////////////////
namespace {
// Implements clasp's configurable restart-strategies.
// Note: Currently all restart-strategies can be easily implemented using one function.
// In the future, as new restart strategies emerge, the function should be replaced
// with a strategy hierarchy
uint64 restart(uint32 k, uint32 base, double inc) {
	if (base != 0) {
		if (inc != 0) {
			return static_cast<uint64>(base * pow(inc, (double)k));
		}
		while (k) {
			uint32 nk = static_cast<uint32>(log((double)k) / log(2.0)) + 1;
			if (k == ((1u << nk) - 1)) {
				return base * (1u << (nk-1));
			}
			k -= 1u << (nk-1);
			++k;
		}
		return base;
	}
	return static_cast<uint64>(-1);
}
}
/////////////////////////////////////////////////////////////////////////////////////////
// solve
/////////////////////////////////////////////////////////////////////////////////////////
namespace {
bool backtrackFromModel(Solver& s, const SolveParams& p) {
	p.printer()->printModel(s);
	if (s.strategies().satPrePro.get() && s.strategies().satPrePro->hasSymModel()) {
		return true;
	}
	MinimizeConstraint* c = s.strategies().minimizer;
	return !c ? s.backtrack() : c->backtrackFromModel(s);
}
}

bool solve(Solver& s, uint32 maxAs, const SolveParams& p) {
	if (s.hasConflict()) return false;
	// ASP-specific. A SAT-Solver typically uses a fraction of the initial number
	// of clauses. For ASP this does not work as good, because the number of
	// clauses resulting from Clark's completion is far greater than the number
	// of rules contained in the logic program and thus maxLearnts tends to be
	// to large if based on number of clauses.
	// Of course, one could increase maxLearntDiv (e.g. by a factor 4) to compensate
	// for this fact. Clasp uses a different approach: it uses the number of heads
	// plus the number of bodies as basis for maxLearnt. 
	// Translated into clauses: From Clark's completion Clasp counts only the 
	// clauses that are not necessarily binary.
	double maxLearnts = p.reduceBase() != 0 ? (uint32)std::max(10.0, s.numVars() / p.reduceBase()) : -1.0;
	if (maxLearnts != -1.0 && maxLearnts < 1.0) { maxLearnts = 1.0; }
	uint32 asFound	= 0;
	ValueRep result = value_free;
	uint32 randRuns	= p.randRuns();
	double randProp	= randRuns == 0 ? p.randomPropability() : 1.0;
	uint64 maxCfl		= randRuns == 0 ? restart(0, p.restartBase(), p.restartInc()) : p.randConflicts();
	uint32 shuffle	= p.shuffleBase();
	bool sp					= s.strategies().saveProgress;
	while (result == value_free) {
		result = s.search(maxCfl, maxLearnts, p.reduceDInc(), randProp);
		if (result == value_true) {
			if ( !backtrackFromModel(s, p) ) {
				result = value_false;
			}
			else if (maxAs == 0 || ++asFound != maxAs) {
				result = value_free;
				randRuns = 0;
				randProp = p.randomPropability();
				// Afert the first solution was found we can either disable restarts completely
				// or limit them to the current backtrack level. Full restarts are
				// no longer possible - the solver does not record found models and
				// therefore could recompute them after a complete restart.
				maxCfl = p.restartBounded() 
					? restart((uint32)s.stats.restarts, p.restartBase(), p.restartInc())
					: static_cast<uint64>(-1);
			}
		}
		else if (result == value_free){
			if (randRuns == 0) {
				if (maxLearnts != -1.0 && (p.reduceRInc() > 1.0 || s.numLearntConstraints() > (.5*maxLearnts))) {
					if (p.reduceRestart()) {
						s.reduceLearnts(.33f);
					}
					double inc = p.reduceRInc() > 1.0 ? p.reduceRInc() : p.reduceDInc();
					maxLearnts = std::min(maxLearnts*inc, (double)std::numeric_limits<LitVec::size_type>::max());
				}
				if (++s.stats.restarts == shuffle) {
					shuffle += p.shuffleNext();
					s.shuffleOnNextSimplify();
				}
				maxCfl = restart((uint32)s.stats.restarts, p.restartBase(), p.restartInc());
				
			}
			else if (--randRuns == 0) {
				maxCfl = restart((uint32)s.stats.restarts, p.restartBase(), p.restartInc());
				randProp = p.randomPropability();
			}	
		}
	}
	s.undoUntil(0);
	s.strategies().saveProgress = sp;
	return result == value_true;
}

bool solve(Solver& s, const LitVec& assumptions, uint32 maxAs, const SolveParams& p) {
	if (s.hasConflict()) return false;
	if (!assumptions.empty() && !s.simplify(false)) {
		return false;
	}
	for (LitVec::size_type i = 0; i != assumptions.size(); ++i) {
		Literal p = assumptions[i];
		if (!s.isTrue(p) && (s.isFalse(p) || !s.assume(p) || !s.propagate())) {
			s.undoUntil(0);
			return false;
		}
	}
	s.setRootLevel(s.decisionLevel());
	bool ret = solve(s, maxAs, p);
	s.clearAssumptions();
	return ret;
}

}
