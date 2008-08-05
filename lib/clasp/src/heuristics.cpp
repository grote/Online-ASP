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
#include <clasp/include/heuristics.h>
#include <algorithm>
namespace Clasp { namespace {

// our own rng for heuristics
uint32 heuRand_g = 1;
inline uint32 heuRand() {
	return( ((heuRand_g = heuRand_g * 214013L + 2531011L) >> 16) & 0x7fff );
}
}
/////////////////////////////////////////////////////////////////////////////////////////
// Lookahead selection strategies
/////////////////////////////////////////////////////////////////////////////////////////
void Lookahead::startInit(const Solver& s) {
	deps_.clear();
	scores_.clear();
	var_ = startVar_ = 0;
	if (type_ == auto_lookahead) {
		uint32 numAtoms = 0, numBodies = 0;
		// Note: numAtoms + numBodies is not necessarily equal to numVars.
		// Because of equivalence-preprocessing some variables can be both
		// an atom and a body at the same time.
		for (Var v = 1; v <= s.numVars(); ++v) {
			numAtoms	+= isAtom(s.type(v));
			numBodies	+= isBody(s.type(v));
		}
		type_ = (numAtoms<<1) < numBodies
			? atom_lookahead
			: (numBodies<<1) < numAtoms
			? body_lookahead
			: hybrid_lookahead;
	}
	switch(type_) {
		case atom_lookahead: varTypes_ = Var_t::atom_var; break;
		case body_lookahead: varTypes_ = Var_t::body_var; break;
		case hybrid_lookahead: varTypes_ = Var_t::atom_body_var; break;
		default:
			assert(false && "ups!");
			break;
	};
	if (select_) select_->startInit(s);
}

void Lookahead::simplify(Solver& s, LitVec::size_type st) {
	if (s.numFreeVars() > 0) {
		while (s.value(startVar_) != value_free) ++startVar_;
		if (var_ < startVar_)
			var_ = startVar_;
	}
	// simplify the problem by applying failed literal detection
	s.failedLiteral(var_, scores_, varTypes_, type_ != hybrid_lookahead, deps_);
	if (select_) {
		select_->simplify(s, st);
	}
}


Literal Lookahead::doSelect(Solver& s) {
	// no need to call s.failedLiteral on decision level 0, because that was already done
	// in Lookahead::simplify!
	if (s.decisionLevel() > 0 && s.failedLiteral(var_, scores_, varTypes_, type_ != hybrid_lookahead, deps_)) {
		return Literal();
	}
	if (!select_) {
		// No literal failed - select literal with best heuristic value
		return heuristic(s);
	}
	else {
		// We are working in decoration mode - selecting is not our business
		for (VarVec::size_type i = 0, end = deps_.size(); i != end; ++i) {
			scores_[deps_[i]].clear();
		}
		deps_.clear();
		select_->select(s);
		return Literal();
	}
}

void Lookahead::checkScore(uint32& min, uint32& max, Var v, const VarScore& vs, Literal& r) {
	uint32 vMin,vMax;
	vs.score(vMax, vMin);
	if (vMin > min || (vMin == min && vMax > max)) {
		min = vMin;
		max = vMax;
		r = Literal(v, vs.prefSign());
	}
}
Literal Lookahead::heuristic(Solver& s) {
	Literal choice;
	if (type_ == hybrid_lookahead) {
		uint32 max = 0;
		for (LitVec::size_type i = 0; i < deps_.size(); ++i) {
			uint32 vMin,vMax;
			VarScore& vs = scores_[deps_[i]];
			vs.score(vMax, vMin);
			if (vMax > max) {
				max = vMax;
				choice = Literal(deps_[i], vs.prefSign());
			}
			vs.clear();
		}
	}
	else {
		uint32 min = 0;
		uint32 max = 0;
		for (LitVec::size_type i = 0; i < deps_.size(); ++i) {
			Var v = deps_[i];
			VarScore& vs = scores_[v];
			if (!vs.tested()) {			// no literal of v tested
				s.lookahead(negLit(v), scores_, deps_, varTypes_, false);
				if (vs.score(negLit(v)) >= min) {
					s.lookahead(posLit(v), scores_, deps_, varTypes_, false);
					checkScore(min, max, v, vs, choice);
				}
			}
			else if (!vs.testedBoth()) {	// one literal of v tested
				if (vs.tested(posLit(v)) && vs.score(posLit(v)) >= min) {
					assert(!vs.tested(negLit(v)));
					s.lookahead(negLit(v), scores_, deps_, varTypes_, false);
					checkScore(min, max, v, vs, choice);
				}
				else if (vs.tested(negLit(v)) && vs.score(negLit(v)) >= min) {
					assert(!vs.tested(posLit(v)));
					s.lookahead(posLit(v), scores_, deps_, varTypes_, false);
					checkScore(min, max, v, vs, choice);
				}
			}
			else {														// both literals of v tested
				checkScore(min, max, v, vs, choice);
			}
			vs.clear();
		}
	}
	deps_.clear();
	if (isSentinel(choice)) {
		// can only happen if body-lookahead is used. In that case if the program contains
		// choice rules it may happen, that all bodies are assigned but the assignment is not
		// total -> select the first free atom
		for (Var i = 1; i <= s.numVars(); ++i) {
			if (s.value(i) == value_free) {
				choice = s.preferredLiteralByType(i);
				break;
			}
		}
	}
	return choice;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Lookback selection strategies
/////////////////////////////////////////////////////////////////////////////////////////
uint32 momsScore(const Solver& s, Var v) {
	uint32 sc;
	if (s.numBinaryConstraints()) {
		uint32 s1 = s.estimateBCP(posLit(v), 0) - 1;
		uint32 s2 = s.estimateBCP(negLit(v), 0) - 1;
		sc = ((s1 * s2)<<10) + (s1 + s2);
	}
	else {
		// problem does not contain binary constraints - fall back to counting watches
		uint32 s1 = s.numWatches(posLit(v));
		uint32 s2 = s.numWatches(negLit(v));
		sc = ((s1 * s2)<<10) + (s1 + s2);
	}
	return sc;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Berkmin selection strategy
/////////////////////////////////////////////////////////////////////////////////////////
#define BERK_NUM_CANDIDATES 5
#define BERK_CACHE_GROW 2.0
#define BERK_TOP_LEVEL_MOMS 1

ClaspBerkmin::ClaspBerkmin(uint32 maxB, bool loops) 
	: topConflict_(uint32(-1))
	, topLoop_(uint32(-1))
	, front_(1) 
	, cacheSize_(5)
	, decay_(0)
	, numVsids_(0)
	, maxBerkmin_(maxB == 0 ? uint32(-1) : maxB)
	, loops_(loops) {
}

void ClaspBerkmin::startInit(const Solver& s) {
	score_.clear();
	score_.resize(s.numVars()+1);
	
	cache_.clear();
	cacheSize_ = 5;
	cacheFront_ = cache_.end();
	
	freeLits_.clear();
	freeLoopLits_.clear();
	topConflict_ = topLoop_ = (uint32)-1;
	
	front_ = 1;
	decay_ = numVsids_ = 0;
}

void ClaspBerkmin::newConstraint(const Solver&, const Literal* first, LitVec::size_type size, ConstraintType t) {
	if (t != Constraint_t::native_constraint) {
		for (; size--; ++first) {
			score_[first->var()].occ_ += 1 - (first->sign() + first->sign());
		}
	}
}

void ClaspBerkmin::updateReason(const Solver&, const LitVec& lits, Literal r) {
	for (LitVec::size_type i = 0; i < lits.size(); ++i) {
		++score_[lits[i].var()].activity(decay_);
	}
	++score_[r.var()].activity(decay_);
}

void ClaspBerkmin::undoUntil(const Solver&, LitVec::size_type) {
	topConflict_ = topLoop_ = static_cast<uint32>(-1);
	front_ = 1;
	cache_.clear();
	cacheFront_ = cache_.end();
	if (cacheSize_ > 5 && numVsids_ > 0 && (numVsids_*3) < cacheSize_) {
		cacheSize_ = static_cast<uint32>(cacheSize_/BERK_CACHE_GROW);
	}
	numVsids_ = 0;
}

bool ClaspBerkmin::hasTopUnsat(Solver& s, uint32& maxIdx, uint32 minIdx, ConstraintType t) {
	while (maxIdx != minIdx) {
		const LearntConstraint& c = s.getLearnt(maxIdx-1);
		if (c.type() == t && !c.isSatisfied(s, freeLits_)) {
			return true;
		}
		--maxIdx;
		freeLits_.clear();
	}
	return false;
}

bool ClaspBerkmin::hasTopUnsat(Solver& s) {
	topConflict_	= std::min(s.numLearntConstraints(), topConflict_);
	topLoop_			= std::min(s.numLearntConstraints(), topLoop_);
	freeLoopLits_.clear();
	freeLits_.clear();
	if (topConflict_ > topLoop_ && hasTopUnsat(s, topConflict_, topLoop_, Constraint_t::learnt_conflict)) {
		return true;
	}
	if (loops_ && topLoop_ > topConflict_ && hasTopUnsat(s, topLoop_, topConflict_, Constraint_t::learnt_loop)) {
		freeLits_.swap(freeLoopLits_);
	}
	uint32 stopAt = topConflict_ < maxBerkmin_ ? 0 : topConflict_ - maxBerkmin_;
	while (topConflict_ != stopAt) {
		const LearntConstraint& c = s.getLearnt(topConflict_-1);
		if (c.type() == Constraint_t::learnt_conflict && !c.isSatisfied(s, freeLits_)) {
			break;
		}
		else if (loops_ && freeLoopLits_.empty() && c.type() == Constraint_t::learnt_loop && !c.isSatisfied(s, freeLits_)) {
			topLoop_	= topConflict_;
			freeLits_.swap(freeLoopLits_);
		}
		--topConflict_;
		freeLits_.clear();
	}
	if (freeLoopLits_.empty())	topLoop_ = topConflict_;
	if (freeLits_.empty())			freeLoopLits_.swap(freeLits_);
	return !freeLits_.empty();
}

Literal ClaspBerkmin::doSelect(Solver& s) {
	if ( (decay_ += (((s.stats.choices + 1) & 511) == 0)) == std::numeric_limits<uint32>::max() ) {
		// rescale decay counters
		for (Scores::size_type i = 0, end = score_.size(); i != end; ++i) {
			score_[i].activity_ >>= ((decay_ - score_[i].decay_)<<1);
			score_[i].decay_ = 0;
		}
		decay_ = 0;
	}
	if (hasTopUnsat(s)) {
		assert(!freeLits_.empty());
		Var	candidates[BERK_NUM_CANDIDATES];
		candidates[0] = freeLits_[0].var();
		uint32 c = 1;
		uint32	ms	= static_cast<uint32>(-1);
		uint32	ls	= 0;
		for (LitVec::size_type i = 1; i < freeLits_.size(); ++i) {
			Var v = freeLits_[i].var();
			assert(s.value(v) == value_free);
			if ( score_[v].activity(decay_) > score_[candidates[0]].activity(decay_) ) {
				candidates[0] = v;
				c = 1;
				ms	= static_cast<uint32>(-1);
			}
			else if ( score_[v].activity_ == score_[candidates[0]].activity_) {
				if (ms == static_cast<uint32>(-1)) ms = momsScore(s, candidates[0]);
				if ((ls = momsScore(s,v)) > ms ) {
					candidates[0] = v;
					c = 1;
					ms	= ls;
				}
				else if (ls == ms && c != BERK_NUM_CANDIDATES) {
					candidates[c++] = v;
				}
			}
		}
		return selectLiteral(s, (c == 1 ? candidates[0] : candidates[heuRand()%c]), false);
	}
	else {
		return selectVsids(s);
	}
}



Literal ClaspBerkmin::selectVsids(Solver& s) {
	++numVsids_;
	Pos end = cache_.end();
	while (cacheFront_ != end && s.value(*cacheFront_) != value_free) { ++cacheFront_; }
	if (cacheFront_ != end) {
		return selectLiteral(s, *cacheFront_, true);
	}
	for (; s.value( front_ ) != value_free; ++front_);	// Pre: At least one unassigned var!
	Var var = front_;
#if defined(BERK_TOP_LEVEL_MOMS) && BERK_TOP_LEVEL_MOMS == 1
	if (s.stats.conflicts != 0 || (numVsids_>50&&s.numFreeVars()>9999)) {		// populate cache with most active vars
#endif
		if (!cache_.empty() && cacheSize_ < s.numFreeVars()/10) {
			cacheSize_ = static_cast<uint32>( (cacheSize_*BERK_CACHE_GROW) + .5 );
		}
		cache_.clear();
		GreaterActivity comp(score_, decay_);
		Var v = var;
		LitVec::size_type cs = std::min(cacheSize_, s.numFreeVars());
		for (;;) {										// add first cs free variables to cache
			cache_.push_back(v);
			std::push_heap(cache_.begin(), cache_.end(), comp);
			if (cache_.size() == cs) break;
			while ( s.value(++v) != value_free );	// skip over assigned vars
		} 
		for (v = cs == cacheSize_ ? v+1 : s.numVars()+1; v <= s.numVars(); ++v) {
			// replace vars with low activity
			if (s.value(v) == value_free && comp(v, cache_[0])) {
				std::pop_heap(cache_.begin(), cache_.end(), comp);
				cache_.back() = v;
				std::push_heap(cache_.begin(), cache_.end(), comp);
			}
		}
		std::sort_heap(cache_.begin(), cache_.end(), comp);
		cacheFront_ = cache_.begin();
		var = cache_[0];
#if defined(BERK_TOP_LEVEL_MOMS) && BERK_TOP_LEVEL_MOMS == 1
	}
	else {													// no conflicts, no activities. Select based on MOMS
		uint32 ms = momsScore(s, var);
		uint32 ls = 0;
		for (Var v = front_+1; v <= s.numVars(); ++v) {
			if (s.value(v) == value_free && (ls = momsScore(s, v)) > ms) {
				var = v;
				ms	= ls;
			}
		}
	}
#endif
	return selectLiteral(s, var, true);
}

Literal ClaspBerkmin::selectLiteral(Solver& s, Var var, bool vsids) {
	int32 w0 = vsids ? (int32)s.estimateBCP(posLit(var), 5) : score_[var].occ_;
	int32 w1 = vsids ? (int32)s.estimateBCP(negLit(var), 5) : 0;
	if ( s.strategies().saveProgress && std::abs(w0-w1) > 32 ) {
		s.setPreferredValue(var, (w0-w1)<0?value_false:value_true);
	}
	return w0 != w1 
		? Literal(var, (w0-w1)<0)
		: s.preferredLiteralByType(var);
}


/////////////////////////////////////////////////////////////////////////////////////////
// ClaspVmtf selection strategy
/////////////////////////////////////////////////////////////////////////////////////////
ClaspVmtf::ClaspVmtf(LitVec::size_type mtf, bool loops) : MOVE_TO_FRONT(mtf), loops_(loops) { 
}


void ClaspVmtf::startInit(const Solver& s) {
	score_.clear();
	score_.resize(s.numVars()+1, VarInfo(vars_.end()));
}

void ClaspVmtf::endInit(const Solver& s) {
	vars_.clear();
	for (Var v = 1; v <= s.numVars(); ++v) {
		if (s.value(v) == value_free) {
			score_[v].activity_ = momsScore(s, v);
			score_[v].pos_ = vars_.insert(vars_.end(), v);
		}
	}
	vars_.sort(LessLevel(s, score_));
	for (VarList::iterator it = vars_.begin(); it != vars_.end(); ++it) {
		score_[*it].activity_ = 0;
	}
	front_ = vars_.begin();
	decay_ = 0;
}

void ClaspVmtf::simplify(Solver& s, LitVec::size_type i) {
	for (; i < s.numAssignedVars(); ++i) {
		if (score_[s.assignment()[i].var()].pos_ != vars_.end()) {
			vars_.erase(score_[s.assignment()[i].var()].pos_);
			score_[s.assignment()[i].var()].pos_ = vars_.end();
		}
	}
	front_ = vars_.begin();
}

void ClaspVmtf::newConstraint(const Solver& s, const Literal* first, LitVec::size_type size, ConstraintType t) {
	if (t != Constraint_t::native_constraint) {
		LessLevel comp(s, score_);
		VarVec::size_type maxMove = t == Constraint_t::learnt_conflict ? MOVE_TO_FRONT : MOVE_TO_FRONT/2;
		for (LitVec::size_type i = 0; i < size; ++i, ++first) {
			Var v = first->var(); 
			score_[v].occ_ += 1 - (((int)first->sign())<<1);
			if (t == Constraint_t::learnt_conflict || loops_) {
				++score_[v].activity(decay_);
				if (mtf_.size() < maxMove) {
					mtf_.push_back(v);
					std::push_heap(mtf_.begin(), mtf_.end(), comp);
				}
				else if (comp(v, mtf_[0])) {
					assert(s.level(v) <= s.level(mtf_[0]));
					std::pop_heap(mtf_.begin(), mtf_.end(), comp);
					mtf_.back() = v;
					std::push_heap(mtf_.begin(), mtf_.end(), comp);
				}
			}
		}
		for (VarVec::size_type i = 0; i != mtf_.size(); ++i) {
			Var v = mtf_[i];
			if (score_[v].pos_ != vars_.end()) {
				vars_.splice(vars_.begin(), vars_, score_[v].pos_);
			}
		}
		mtf_.clear();
		front_ = vars_.begin();
	}	
}

Literal ClaspVmtf::getLiteral(const Solver& s, Var v) const {
	return score_[v].occ_== 0
		? s.preferredLiteralByType(v)
		: Literal(v, score_[v].occ_ < 0 );
}

void ClaspVmtf::undoUntil(const Solver&, LitVec::size_type) {
	front_ = vars_.begin();
}

void ClaspVmtf::updateReason(const Solver&, const LitVec&, Literal r) {
	++score_[r.var()].activity(decay_);
}

Literal ClaspVmtf::doSelect(Solver& s) {
	decay_ += ((s.stats.choices + 1) & 511) == 0;
	for (; s.value(*front_) != value_free; ++front_);
	Literal c;
	if (s.numFreeVars() > 1) {
		VarList::iterator v2 = front_;
		uint32 distance = 0;
		do {
			++v2;
			++distance;
		} while (s.value(*v2) != value_free);
		c = (score_[*front_].activity(decay_) + (distance<<1)+ 3) > score_[*v2].activity(decay_)
				? getLiteral(s, *front_)
				: getLiteral(s, *v2);
	}
	else {
		c = getLiteral(s, *front_);
	}
	return c;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ClaspVsids selection strategy
/////////////////////////////////////////////////////////////////////////////////////////
const double vsidsVarDecay_g = 1 / 0.95;

ClaspVsids::ClaspVsids(bool loops) 
	: vars_(GreaterActivity(score_)) 
	, inc_(1.0)
	, scoreLoops_(loops) {}

void ClaspVsids::startInit(const Solver& s) {
	score_.clear();
	score_.resize(s.numVars()+1);
}
void ClaspVsids::endInit(const Solver& s) {
	vars_.clear();
	inc_ = 1.0;
	double maxS = 0.0;
	for (Var v = 1; v <= s.numVars(); ++v) {
		if (s.value(v) == value_free) {
			// initialize activity to moms-score
			score_[v].first = momsScore(s, v);
			if (score_[v].first > maxS) {
				maxS = score_[v].first;
			}
			vars_.push(v);
		}
	}
	for (VarVec::size_type i = 0; i != score_.size(); ++i) {
		score_[i].first /= maxS;
	}
}

void ClaspVsids::simplify(Solver& s, LitVec::size_type i) {
	for (; i < s.numAssignedVars(); ++i) {
		vars_.remove(s.assignment()[i].var());
	}
}

void ClaspVsids::newConstraint(const Solver&, const Literal* first, LitVec::size_type size, ConstraintType t) {
	if (t != Constraint_t::native_constraint) {
		for (LitVec::size_type i = 0; i < size; ++i, ++first) {
			score_[first->var()].second += 1 - (first->sign() + first->sign());
			if (t == Constraint_t::learnt_conflict || scoreLoops_) {
				updateVarActivity(first->var());
			}
		}
		if (t == Constraint_t::learnt_conflict) {
			inc_ *= vsidsVarDecay_g;
		}
	}
}

void ClaspVsids::updateReason(const Solver&, const LitVec&, Literal r) {
	if (r.var() != 0) updateVarActivity(r.var());
}

void ClaspVsids::undoUntil(const Solver& s , LitVec::size_type st) {
	const LitVec& a = s.assignment();
	for (; st < a.size(); ++st) {
		if (!vars_.is_in_queue(a[st].var())) {
			vars_.push(a[st].var());
		}
	}
}

Literal ClaspVsids::doSelect(Solver& s) {
	Var var;
	while ( s.value(vars_.top()) != value_free ) {
		vars_.pop();
	}
	var = vars_.top();
	return score_[var].second == 0
		? s.preferredLiteralByType(var)
		: Literal( var, score_[var].second < 0 );
}
}
