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

#include <clasp/include/unfounded_check.h>
#include <clasp/include/clause.h>
#include <algorithm>
namespace Clasp {

/////////////////////////////////////////////////////////////////////////////////////////
// DefaultUnfoundedCheck - Construction/Destruction
/////////////////////////////////////////////////////////////////////////////////////////
DefaultUnfoundedCheck::DefaultUnfoundedCheck(ReasonStrategy r)
	: solver_(0) 
	, activeClause_(0)
	, strategy_(r) {
	reasons_ = 0;
}
DefaultUnfoundedCheck::~DefaultUnfoundedCheck() {
	clear();
}

void DefaultUnfoundedCheck::UfsBodyNode::destroy() {
	if (!extended())	{ delete this; }
	else							{ delete static_cast<ExtendedBody*>(this); }
}


void DefaultUnfoundedCheck::clear() {
	for (BodyVec::iterator it = bodies_.begin(); it != bodies_.end(); ++it) {
		(*it)->destroy();
	}
	for (AtomVec::iterator it = atoms_.begin(); it != atoms_.end(); ++it) {
		delete *it;
	}
	atoms_.clear();
	bodies_.clear();
	invalid_.clear();
	extWatches_.clear();
	delete activeClause_;
	delete [] reasons_;
	reasons_ = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// DefaultUnfoundedCheck - Base interface
/////////////////////////////////////////////////////////////////////////////////////////
void DefaultUnfoundedCheck::reset() {
	assert(loopAtoms_.empty());
	while (!todo_.empty()) {
		todo_.back()->pickedOrTodo = 0;
		todo_.pop_back();
	}
	while (!unfounded_.empty()) {
		unfounded_.back()->typeOrUnfounded = 0;
		unfounded_.pop_back();
	}
	while (!sourceQueue_.empty()) {
		sourceQueue_.back()->source = sourceQueue_.back()->watch;
		sourceQueue_.pop_back();
	}
	invalid_.clear();
	activeClause_->clear();
}

bool DefaultUnfoundedCheck::propagate() {
	return strategy_ != no_reason 
		? assertAtom()
		: assertSet();
}

/////////////////////////////////////////////////////////////////////////////////////////
// DefaultUnfoundedCheck - Constraint interface
/////////////////////////////////////////////////////////////////////////////////////////
// a (relevant) literal was assigned. Check which node is affected and update it accordingly
Constraint::PropResult DefaultUnfoundedCheck::propagate(const Literal& p, uint32& data, Solver& s) {
	assert(&s == solver_); (void)s;
	uint32 index	= data >> 1;
	uint32 type		= data & 1;
	if (type == 0) {
		// a normal bodies became false - mark as invalid if necessary
		assert(index < bodies_.size());
		if (bodies_[index]->watches > 0) { invalid_.push_back(bodies_[index]); }
	}
	else {
		// literal is associated with an ExtendedWatch - notify it 
		assert(index < extWatches_.size());
		extWatches_[index].notify(p, this);
	}
	return PropResult(true, true);	// always keep the watch
}
// only used if strategy_ == only_reason
void DefaultUnfoundedCheck::reason(const Literal& p, LitVec& r) {
	assert(strategy_ == only_reason && reasons_);
	r.assign(reasons_[p.var()-1].begin(), reasons_[p.var()-1].end());
}

/////////////////////////////////////////////////////////////////////////////////////////
// DefaultUnfoundedCheck / Dependency graph initialization
/////////////////////////////////////////////////////////////////////////////////////////
// creates a positive-body-atom-dependency graph (PBADG) 
// from the body-atom-dependency-graph given in prgAtoms. 
// The PBADG contains a node for each non-trivially connected atom A and
// a node for each body B, s.th. there is a non-trivially connected atom A with
// B in body(A).
// Pre : a->ignore	= 0 for all new and relevant atoms a
// Pre : b->visited = 1 for all new and relevant bodies b
// Post: a->ignore	= 1 for all atoms that were added to the PBADG
// Post: b->visited = 0 for all bodies that were added to the PBADG
// Note: During initialization the root-member of a node in the BADG is used 
// to store the id of the corresponding node in the PBADG. 
bool DefaultUnfoundedCheck::init(Solver& s, const AtomList& prgAtoms, const BodyList& prgBodies, Var startAtom) {
	assert(!solver_ || solver_ == &s);
	solver_ = &s;
	if (!activeClause_) { activeClause_ = new ClauseCreator(solver_); }
	// add all relevant new atoms
	AtomList::const_iterator atomsEnd = prgAtoms.end();
	for (AtomList::const_iterator h = prgAtoms.begin()+startAtom; h != atomsEnd; ++h) {
		if (relevantPrgAtom(*h)) {
			UfsAtomNode* hn = addAtom(*h);
			VarVec::iterator bodiesEnd = (*h)->preds.end();
			for (VarVec::iterator b = (*h)->preds.begin(); b != bodiesEnd; ++b) {
				PrgBodyNode* prgBody = prgBodies[*b];
				if (relevantPrgBody(prgBody)) {
					UfsBodyNode* bn = addBody(prgBody, prgAtoms);
					addEdges(bn, hn);
					if (prgBody->type() == CHOICERULE) {
						// Since b is a choice rule, setting the atom hn to false will not automatically
						// falsify b and thus will not trigger the removal of hn's source if hn.source == bn.
						// We therefore add a watch and check if hn is sourced by bn whenever hn becomes false.
						addExtendedWatch(~hn->lit, ExtendedWatch(bn, &UfsBodyNode::triggerRemSource, (uint32)bn->succs.size()-1));
					}
				}
			}
		}
	}
	propagateSource();
	// check for initially unfounded atoms
	for (AtomVec::iterator it = atoms_.begin(); it != atoms_.end(); ++it) {
		if (!solver_->isFalse((*it)->lit)
			&& (*it)->source == 0
			&& !solver_->force(~(*it)->lit, 0)) {
			return false;
		}
	}
	return true;
}

// if necessary, creates a new atom node and adds it to the PBADG
DefaultUnfoundedCheck::UfsAtomNode* DefaultUnfoundedCheck::addAtom(PrgAtomNode* h) {
	if (!h->ignore()) {	// first time we see this atom -> create node
		atoms_.push_back( new UfsAtomNode(h->literal(), h->scc()) );
		h->setIgnore(true); // from now on, ignore this atom
		h->setRoot( (uint32)atoms_.size() - 1 );
		solver_->setFrozen(h->var(), true);
	}
	return atoms_[ h->root() ];	
}

// if necessary, creates and initializes a new body node and adds it to the PBADG
DefaultUnfoundedCheck::UfsBodyNode* DefaultUnfoundedCheck::addBody(PrgBodyNode* b, const AtomList& prgAtoms) {
	if (b->visited() == 1) {	// first time we see this body - create node
		RuleType rt = b->type();
		solver_->setFrozen(b->var(), true);
		if (b->scc() != PrgNode::noScc && (rt == CONSTRAINTRULE || rt == WEIGHTRULE)) {
			return addExtendedBody(b, prgAtoms);
		}
		b->setVisited(0);
		UfsBodyNode* bn = new UfsBodyNode(b->literal(), b->scc());	
		bodies_.push_back( bn );
		b->setRoot( (uint32)bodies_.size() - 1 );
		solver_->addWatch(~b->literal(), this, ((uint32)bodies_.size() - 1)<<1);
		for (uint32 p = 0; p != b->posSize(); ++p) {
			PrgAtomNode* pred = prgAtoms[b->pos(p)];
			if (relevantPrgAtom(pred) && pred->scc() == bn->scc) {
				UfsAtomNode* hn = addAtom(pred);
				addEdges(hn, bn, 1);
			}
		}
	}
	return bodies_[ b->root() ];
}

// The body b is the body of a CONSTRAINT or WEIGHTRULE. Creates an extended body node
// and adds it to PBADG.
DefaultUnfoundedCheck::UfsBodyNode* DefaultUnfoundedCheck::addExtendedBody(PrgBodyNode* b, const AtomList& prgAtoms) {
	assert(b->visited() == 1);
	b->setVisited(0);
	b->setRoot((uint32)bodies_.size());
	solver_->addWatch(~b->literal(), this, ((uint32)bodies_.size())<<1);
	ExtendedBody* bn = new ExtendedBody(b->literal(), b->scc(), b->bound());
	bodies_.push_back( bn );
	WeightVec sccWeights, otherWeights;
	ExtendedWatch watch(bn, &UfsBodyNode::triggerCheckInvalid, 0);
	for (uint32 p = 0; p != b->posSize(); ++p) {
		PrgAtomNode* pred = prgAtoms[b->pos(p)];
		if (pred->hasVar() && !solver_->isFalse(pred->literal())) {
			weight_t w = b->weight(p, true);
			if (pred->scc() == b->scc()) {
				UfsAtomNode* hn = addAtom(pred);
				addEdges(hn, bn, w);
				addExtendedWatch(~pred->literal(), watch);
				if (b->type() == WEIGHTRULE) { sccWeights.push_back(w); }
			}
			else {
				bn->trivWeight += w;
				watch.setData( (static_cast<uint32>(bn->lits.size())<<1) + 1 );
				bn->lits.push_back(pred->literal());
				addExtendedWatch(~pred->literal(), watch);
				if (b->type() == WEIGHTRULE) { otherWeights.push_back(w); }
			}
		}
	}
	for (uint32 n = 0; n != b->negSize(); ++n) {
		PrgAtomNode* pred = prgAtoms[b->neg(n)];
		if (pred->hasVar() && !solver_->isTrue(pred->literal())) {
			weight_t w = b->weight(n, false);
			bn->trivWeight += w;
			watch.setData( (static_cast<uint32>(bn->lits.size())<<1) + 1 );
			bn->lits.push_back(~pred->literal());
			addExtendedWatch(pred->literal(), watch);
			if (b->type() == WEIGHTRULE) { otherWeights.push_back(w); }
		}
	}
	bn->circWeight = b->bound() - bn->trivWeight;
	bn->trivWeight -= b->bound();
	if (b->type() == WEIGHTRULE && (sccWeights.size() + otherWeights.size()) != 0) {
		// +1 to store sum of weights
		weight_t sumW = 0;
		bn->weights = new weight_t[(sccWeights.size()+otherWeights.size()) + 1];
		for (LitVec::size_type i = 0; i != sccWeights.size(); ++i) { 
			bn->weights[i] = sccWeights[i];
			sumW					+= sccWeights[i];
		}
		for (LitVec::size_type i = 0; i != otherWeights.size(); ++i) {
			bn->weights[i+sccWeights.size()] = otherWeights[i];
			sumW += otherWeights[i];
		}
		bn->weights[ sccWeights.size() + otherWeights.size() ] = sumW;
	}
	return bn;
}

// Adds a bidirectional (unlabeled) edge between b and h.
// Note:
//	A bodies unlabeled edges are partitioned in such a way, that successors
//	from the same scc precede those from other sccs.
//	
//	An atoms unlabeled edges are partitioned the other way round, i.e. bodies
//	from other sccs precede those that are in the same scc as the head.
// 
void DefaultUnfoundedCheck::addEdges(UfsBodyNode* b, UfsAtomNode* h) {
	h->addPredecessor(b);
	b->addSuccessor(h);
	if (h->source == 0 && (b->scc != h->scc || (b->extended() && b->circWeight <= 0)) ) {
		h->source = h->watch = b;
		++b->watches;
		sourceQueue_.push_back(h);
	}
}

// Adds a bidirectional zero-edge between b and h, iff b and h are in the same scc.
void DefaultUnfoundedCheck::addEdges(UfsAtomNode* h, UfsBodyNode* b, weight_t w) {
	if (h->scc == b->scc) {
		b->addPredecessor(h);
		h->addSuccessor(b);
		b->circWeight += w;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// DefaultUnfoundedCheck - finding unfounded sets
/////////////////////////////////////////////////////////////////////////////////////////
bool DefaultUnfoundedCheck::findUnfoundedSet() {
	// first: remove all sources that were recently falsified
	if (!sourceQueue_.empty()) propagateSource();
	for (VarVec::size_type i = 0; i < invalid_.size(); ++i) { 
		removeSource(invalid_[i]); 
	}
	invalid_.clear();
	assert(sourceQueue_.empty() && unfounded_.empty());
	// second: try to re-establish sources.
	while (!todo_.empty()) {
		UfsAtomNode* head = dequeueTodo();
		assert(head->scc != PrgNode::noScc );
		if (head->source == 0 && !solver_->isFalse(head->lit) && !findSource(head)) {
			return true;	// found an unfounded set - contained in unfounded_
		}
		assert(sourceQueue_.empty());
	}
	return false;			// no unfounded sets
}

// searches a new source for the atom node head.
// If a new source is found the function returns true.
// Otherwise the function returns false and unfounded_ contains head
// as well as all atoms with no source that circularily depend on head.
bool DefaultUnfoundedCheck::findSource(UfsAtomNode* head) {
	assert(unfounded_.empty());
	enqueueUnfounded(head);				// unfounded, unless we find a new source
	Queue noSourceYet;
	bool changed = false;
	while (!unfounded_.empty()) {
		head = unfounded_.front();
		if (head->source == 0) {	// no source
			unfounded_.pop_front();	// note: current atom is still marked	
			BodyVec::const_iterator it	= head->preds.begin();
			BodyVec::const_iterator end = head->preds.end();
			for (; it != end && !(*it)->canBeSource(head, this); ++it) 
				;	// find source, add un-sourced atoms to unfounded-queue
			if (it != end) {
				head->typeOrUnfounded = 0;	// found a new source,
				setSource(*it, head);				// set the new source
				propagateSource();					// and propagate it forward
				changed = true;							// may source atoms in noSourceYet!
			}
			else {
				noSourceYet.push_back(head);// no source found
			}
		}
		else {	// head has a source
			dequeueUnfounded();
		}
	}	// while unfounded_.emtpy() == false
	std::swap(noSourceYet, unfounded_);
	if (changed) {
		// remove all atoms that have a source as they are not unfounded
		Queue::iterator it, j = unfounded_.begin();
		for (it = unfounded_.begin(); it != unfounded_.end(); ++it) {
			if ( (*it)->source != 0 )		{ (*it)->typeOrUnfounded = 0; }
			else												{	*j++ = *it; }
		}
		unfounded_.erase(j, unfounded_.end());
	}
	return unfounded_.empty();
}

// sets body as source for head if necessary.
// PRE: value(body) != value_false
// POST: source(head) != 0
void DefaultUnfoundedCheck::setSource(UfsBodyNode* body, UfsAtomNode* head) {
	assert(!solver_->isFalse(body->lit));
	if (head->source == 0) {
		head->source = body;
		++body->watches;
		if (head->watch) {
			--head->watch->watches;
		}
		head->watch = body;
		sourceQueue_.push_back(head);
	}
}

// For each successor h of body sets source(h) to 0 if source(h) == body.
// If source(h) == 0, adds h to the todo-Queue.
// This function is called for each body that became invalid during propagation.
void DefaultUnfoundedCheck::removeSource(UfsBodyNode* body) {
	for (AtomVec::const_iterator it = body->succs.begin(); it != body->succs.end(); ++it) {
		UfsAtomNode* head = *it;
		if (head->source == body) {
			assert(head->watch == body);
			head->source = 0;
			sourceQueue_.push_back(head);
		}
		if (head->source == 0) {
			enqueueTodo(head);
			assert(head->watch != 0);
		}
	}
	propagateSource();
}	

// propagates recently set source pointers within one strong component.
void DefaultUnfoundedCheck::propagateSource() {
	for (LitVec::size_type i = 0; i < sourceQueue_.size(); ++i) {
		UfsAtomNode* head = sourceQueue_[i];
		BodyVec::const_iterator it = head->succs.begin();
		if (head->source != 0) {
			// propagate a newly added source-pointer
			for (; it != head->succs.end(); ++it) {
				if ((*it)->addSource(head) && !solver_->isFalse((*it)->lit)) {
					AtomVec::const_iterator aIt, aEnd;
					for (aIt = (*it)->succs.begin(), aEnd = (*it)->succs.end(); aIt != aEnd; ++aIt) {
						setSource(*it, *aIt);
					}
				}
			}
		}
		else {
			// propagate the removal of a source-pointer within this scc
			for (; it != head->succs.end(); ++it) {
				if ((*it)->removeSource(head)) {
					AtomVec::const_iterator aIt = (*it)->succs.begin();
					AtomVec::const_iterator aEnd = (*it)->succs.end();
					for (; aIt != aEnd && (*aIt)->scc == (*it)->scc; ++aIt) {
						if ((*aIt)->source == (*it)) {
							(*aIt)->source = 0;
							sourceQueue_.push_back(*aIt);
						}
					}
				}
			}
		}
	}
	sourceQueue_.clear();
}


// checks whether an extended body can be a source and adds un-sourced atoms
// to unfounded queue if not.
bool DefaultUnfoundedCheck::ExtendedBody::canBeSourceExt(UfsAtomNode*, DefaultUnfoundedCheck* ufs) {
	trivWeight = 0;
	circWeight = 0;
	weight_t triv = 0;
	for (LitVec::size_type i = 0; i != lits.size(); ++i) {
		if (!ufs->solver().isFalse(lits[i])) { 
			triv += weights ? weights[preds.size()+i] : 1;
		}
	}
	trivWeight = triv - bound;
	circWeight = bound - triv;
	uint32 unfounded = 0;
	for (AtomVec::const_iterator it = preds.begin(); it != preds.end(); ++it) {
		if ( (*it)->source != 0 ) {
			assert(!ufs->solver().isFalse((*it)->lit));
			circWeight -= weights ? weights[it-preds.begin()] : 1;
		}
		else if (!ufs->solver().isFalse((*it)->lit) && !(*it)->inUnfoundedQueue() && triv < bound) {
			++unfounded;
			ufs->enqueueUnfounded(*it);
		}
	}
	if (circWeight <= 0) {
		// since this body is a source, remove from unfounded queue all those atoms
		// that were added in the preceding for-loop.
		while (unfounded--) {
			ufs->unfounded_.back()->typeOrUnfounded = 0;
			ufs->unfounded_.pop_back();
		}
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////
// DefaultUnfoundedCheck - Propagating unfounded sets
/////////////////////////////////////////////////////////////////////////////////////////
// asserts all atoms of the unfounded set, then propagates
bool DefaultUnfoundedCheck::assertSet() {
	assert(strategy_ == no_reason);
	activeClause_->clear();
	while (findUnfoundedSet()) {
		while (!unfounded_.empty() && solver_->force(~unfounded_[0]->lit, 0)) {
			dequeueUnfounded();
		}
		if (!unfounded_.empty() || !unitPropagate(*solver_)) {
			return false;
		}
	}
	return true;
}

// as long as an unfounded set U is not empty,
// - asserts the first non-false atom
// - propagates
// - removes the atom from U
bool DefaultUnfoundedCheck::assertAtom() {
	while(findUnfoundedSet()) {
		activeClause_->clear();
		while (!unfounded_.empty()) {
			if (!solver_->isFalse(unfounded_[0]->lit) && !assertAtom(unfounded_[0]->lit)) {
				return false;
			}
			dequeueUnfounded();
		}
		if (!loopAtoms_.empty()) {
			createLoopFormula();
		}
	}
	return true;
}

// asserts an unfounded atom using the selected reason strategy
bool DefaultUnfoundedCheck::assertAtom(Literal a) {
	bool conflict = solver_->isTrue(a);
	if (strategy_ == distinct_reason || activeClause_->empty()) {
		// first atom of unfounded set or distinct reason for each atom requested.
		activeClause_->startAsserting(Constraint_t::learnt_loop, conflict ? a : ~a);
		computeReason(conflict || strategy_ == only_reason);
		if (conflict) {
			LitVec cfl;
			activeClause_->swap(cfl);
			assert(!cfl.empty());
			solver_->setConflict( cfl );
			return false;
		}
		else if (strategy_ == only_reason) {
			if (reasons_ == 0) reasons_ = new LitVec[solver_->numVars()];
			reasons_[a.var()-1].assign(activeClause_->lits().begin()+1, activeClause_->lits().end());
			solver_->force( ~a, this );
		}
		else if (strategy_ != shared_reason || activeClause_->size() < 4) {
			activeClause_->end();
		}
		else {
			loopAtoms_.push_back(~a);
			solver_->force(~a, 0);
		}
	}
	// subsequent atoms
	else if (conflict) {
		if (!loopAtoms_.empty()) {
			createLoopFormula();
		}
		if (strategy_ == only_reason) {
			// we already have the reason for the conflict, only add a
			LitVec cfl;
			(*activeClause_)[0] = a;
			activeClause_->swap(cfl);
			solver_->setConflict( cfl );
			return false;
		}
		else {
			// we have a clause - create reason for conflict by inverting literals
			LitVec cfl;
			cfl.push_back(a);
			for (LitVec::size_type i = 1; i < activeClause_->size(); ++i) {
				if (~(*activeClause_)[i] != a) {
					cfl.push_back( ~(*activeClause_)[i] );
				}
			}
			solver_->setConflict( cfl );
			return false;
		}
	}
	else if (strategy_ == only_reason) {
		reasons_[a.var()-1].assign(activeClause_->lits().begin()+1, activeClause_->lits().end());
		solver_->force( ~a, this );
	}
	else if (strategy_ != shared_reason || activeClause_->size() < 4) {
		(*activeClause_)[0] = ~a;
		activeClause_->end();
	}
	else {
		solver_->force(~a, 0);
		loopAtoms_.push_back(~a);
	}
	if ( (conflict = !unitPropagate(*solver_)) == true && !loopAtoms_.empty()) {
		createLoopFormula();
	}
	return !conflict;
}

void DefaultUnfoundedCheck::createLoopFormula() {
	assert(activeClause_->size() > 3);
	if (loopAtoms_.size() == 1) {
		(*activeClause_)[0] = loopAtoms_[0];
		Constraint* ante;
		activeClause_->end(&ante);
		assert(ante != 0 && solver_->isTrue(loopAtoms_[0]) && solver_->reason(loopAtoms_[0].var()).isNull());
		const_cast<Antecedent&>(solver_->reason(loopAtoms_[0].var())) = ante;
	}
	else {
		LoopFormula* lf = LoopFormula::newLoopFormula(*solver_, &(*activeClause_)[1], (uint32)activeClause_->size() - 1, (uint32)activeClause_->secondWatch()-1, (uint32)loopAtoms_.size()); 
		solver_->addLearnt(lf);
		for (VarVec::size_type i = 0; i < loopAtoms_.size(); ++i) {
			assert(solver_->isTrue(loopAtoms_[i]) && solver_->reason(loopAtoms_[i].var()).isNull());
			const_cast<Antecedent&>(solver_->reason(loopAtoms_[i].var())) = lf;
			lf->addAtom(loopAtoms_[i], *solver_);
		}
		lf->updateHeuristic(*solver_);
	}
	loopAtoms_.clear();
}


// computes the reason why a set of atoms is unfounded
void DefaultUnfoundedCheck::computeReason(bool invSign) {
	uint32 ufsScc = unfounded_[0]->scc;
	(void)ufsScc;
	for (LitVec::size_type i = 0; i < unfounded_.size(); ++i) {
		assert(ufsScc == unfounded_[i]->scc);
		if (!solver_->isFalse(unfounded_[i]->lit)) {
			UfsAtomNode* a = unfounded_[i];
			for (BodyVec::iterator it = a->preds.begin(), end = a->preds.end(); it != end; ++it) {
				if ((*it)->pickedOrTodo == 0 && !inReason((*it)->lit)) {
					picked_.push_back(*it);
					(*it)->pickedOrTodo = 1;
					(*it)->computeReason(this, invSign);
				}
			}
		}
	}	
	assert(solver_->decisionLevel() == 0 || solver_->level((*activeClause_)[activeClause_->secondWatch()].var()) == solver_->decisionLevel());
	for (BodyVec::iterator it = picked_.begin(); it != picked_.end(); ++it) {
		(*it)->pickedOrTodo = false;
		solver_->data((*it)->lit.var()) &= ~3u;
	}
	for (LitVec::iterator it = pickedAtoms_.begin(); it != pickedAtoms_.end(); ++it) {
		solver_->data(it->var()) &= ~3u;
	}
	picked_.clear();
	pickedAtoms_.clear();
}

void DefaultUnfoundedCheck::UfsBodyNode::computeReason(DefaultUnfoundedCheck* ufs, bool invSign) {
	if (ufs->solver().isFalse(lit)) {
		if (ufs->solver().level(lit.var()) == 0) return;	// Skip level 0 reason
		if (scc == ufs->unfounded_[0]->scc) {
			if (!extended()) {
				// body is only a reason if it does not depend on the atoms from the unfounded set
				for (AtomVec::const_iterator it = preds.begin(); it != preds.end(); ++it) {
					if ( (*it)->typeOrUnfounded && !ufs->solver().isFalse((*it)->lit)) { return; }
				}
			}
			else {
				ExtendedBody* self	= static_cast<ExtendedBody*>(this);
				weight_t sumW				= self->weights ? self->weights[ preds.size() + self->lits.size() ] : (weight_t) (preds.size() + self->lits.size());
				weight_t lb					= self->bound;
				for (AtomVec::const_iterator it = preds.begin(); it != preds.end(); ++it) {
					if ( (*it)->typeOrUnfounded && !ufs->solver().isFalse((*it)->lit)) { 
						sumW -= self->weights ? self->weights[std::find(preds.begin(), preds.end(), *it) - preds.begin()] : 1;
						if (sumW < lb) { return; }
					}
				}
			}	
		}
		Literal bl = lit;
		if (!invSign || (bl = ~bl) != ufs->activeClause_->lits().front()) {
			assert(ufs->inReason(bl) == false);
			ufs->activeClause_->add(bl);
			ufs->markBodyLitInReason(bl);
		}
	}
	else if (extended() && !canBeSource()) {
		ExtendedBody* self = static_cast<ExtendedBody*>(this);
		Literal a = ufs->activeClause_->lits().front();
		for (LitVec::size_type i = 0, end = self->lits.size(); i != end; ++i) {
			Literal x = self->lits[i];
			if (ufs->solver().isFalse(x) && ufs->solver().level(x.var()) > 0) {
				if ((!invSign || (x = ~x) != a) && !ufs->inReason(x)) { 
					ufs->activeClause_->add(x); 
					ufs->markAtomLitInReason(x);
				}
			}
		}
		for (AtomVec::const_iterator z = preds.begin(); z != preds.end(); ++z) {
			Literal x = (*z)->lit;
			if (ufs->solver().isFalse(x) && ufs->solver().level(x.var()) > 0) {
				if ((!invSign || (x = ~x) != a) && !ufs->inReason(x)) { 
					ufs->activeClause_->add(x); 
					ufs->markAtomLitInReason(x);
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// DefaultUnfoundedCheck - Helpers
/////////////////////////////////////////////////////////////////////////////////////////
void DefaultUnfoundedCheck::addExtendedWatch(Literal p, const ExtendedWatch& w) {
	uint32 d = (static_cast<uint32>(extWatches_.size())<<1) + 1;
	extWatches_.push_back(w);
	solver_->addWatch(p, this, d);
}
}
