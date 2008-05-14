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

#ifndef CLASP_UNFOUNDED_CHECK_H_INCLUDED
#define CLASP_UNFOUNDED_CHECK_H_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif
#include <clasp/include/solver.h>
#include <clasp/include/literal.h> 
#include <clasp/include/program_builder.h>
#include <clasp/include/constraint.h>
#include <deque>
namespace Clasp {
class LoopFormula;


//! Clasp's default unfounded set checker.
/*!
 * \ingroup solver
 * Searches for unfounded atoms by checking the positive dependency graph (PDG)
 */
class DefaultUnfoundedCheck : public PostPropagator, private Constraint {
public:
	//! Defines the supported reasons for explaining assignments
	enum ReasonStrategy {
		common_reason,		/*!< one reason for each unfounded set but one clause for each atom */
		distinct_reason,	/*!< distinct reason and clause for each unfounded atom */
		shared_reason,		/*!< one shared loop formula for each unfounded set */
		only_reason,			/*!< store only the reason but don't learn a nogood */
		no_reason					/*!< do not compute reasons - only valid in no-learning mode */
	};
	
	DefaultUnfoundedCheck(ReasonStrategy r = common_reason);
	~DefaultUnfoundedCheck();

	ReasonStrategy reasonStrategy() const {
		return strategy_;
	}
	void clear();
	
	//! initialize unfounded set checker with logic program implicitly given in progAtoms and progBodies
	bool init(Solver& s, const AtomList& progAtoms, const BodyList& progBodies, Var startAtom = 0);

	//! number of nodes in the positive dependency graph
	LitVec::size_type nodes() const { return atoms_.size() + bodies_.size(); }
	
	const Solver& solver() const { return *solver_; }
	
	// base interface
	void reset();
	bool propagate();
	
	// public - so that tests can access the data-structures
	struct UfsNode;
	struct UfsAtomNode;
	struct UfsBodyNode;
	typedef PodVector<UfsNode*>::type NodeVec;
	typedef PodVector<UfsAtomNode*>::type AtomVec;
	typedef PodVector<UfsBodyNode*>::type BodyVec;
	
	//! Base class for nodes in the PDG.
	struct UfsNode {
		UfsNode(Literal a_lit, uint32 a_scc) 
			: lit(a_lit), scc(a_scc)
			, pickedOrTodo(0), typeOrUnfounded(0) { }
		Literal	lit;									/*!< literal in the solver */
		uint32	scc							:	30;	/*!< component number */
		uint32	pickedOrTodo		: 1;	/*!< in picked (body) resp. todo (atom) queue */
		uint32	typeOrUnfounded	: 1;	/*!< in unfounded (atom) queue resp. type of body (normal/extended) */
	};
	
	//! An atom in the PDG
	struct UfsAtomNode : public UfsNode {
		UfsAtomNode(Literal a_lit, uint32 a_scc) : UfsNode(a_lit, a_scc), source(0), watch(0) {}
		BodyVec preds;				/*!< predecessors: [other scc, same scc] */
		BodyVec succs;				/*!< successors from same scc */
		UfsBodyNode* source;	/*!< current source */
		UfsBodyNode* watch;		/*!< current watch, either source_ or last well known source  */
		//! returns true if atom is currently in todo-Queue
		bool inTodoQueue()			const { return pickedOrTodo != 0; }
		//! returns true if atom is currently in unfounded-Queue
		bool inUnfoundedQueue() const { return typeOrUnfounded != 0; }
		//! adds a successor-edge between this atom and the body n (i.e. this atoms appears positively in n) 
		void addSuccessor(UfsBodyNode* n) {
			assert(scc == n->scc);
			succs.push_back( n );
		}
		//! adds a predecessor-edge between this atom and the body n (i.e. n defines this atom)
		void addPredecessor(UfsBodyNode* n) {
			BodyVec::iterator insPos = scc != n->scc ? preds.begin() : preds.end();
			preds.insert(insPos, n);	
		}
	};
	//! A body in the PDG
	struct UfsBodyNode : public UfsNode {
		friend class DefaultUnfoundedCheck;
		UfsBodyNode(Literal a_lit, uint32 a_scc) : UfsNode(a_lit, a_scc), circWeight(0), watches(0) { }
		AtomVec preds;					/*!< predecessors from same scc */
		AtomVec succs;					/*!< successors: [same scc, other scc] */
		weight_t circWeight;		/*!< weight needed from preds in same scc
														 <= 0: source
														  > 0: remove source */
		uint32 watches;					/*!< number of atoms watching this body */
		//! returns true if body was already visited during one algorithm
		bool picked()		const { return pickedOrTodo != 0; }
		//! returns true if body is the body of an extended rule
		bool extended() const { return typeOrUnfounded != 0; }
		//! adds a successor-edge between this body and n (i.e. this body defines n)
		void addSuccessor(UfsAtomNode* n) {
			AtomVec::iterator insPos = scc == n->scc ? succs.begin() : succs.end();
			succs.insert(insPos, n);	
		}
		//! adds a predecessor-edge between this body and the atom n (i.e. n appears positively in this body)
		void addPredecessor(UfsAtomNode* n) {
			assert(n->scc == scc);
			preds.push_back(n);
		}
		//! notifies this body about the fact that the atom a is now sourced
		/*!
		 * returns true only if adding a source to atom a triggered the
		 * transition from ineligible to eligible source
		 */
		bool addSource(UfsAtomNode* a) {
			if (!extended() || static_cast<ExtendedBody*>(this)->weights == 0) {
				return --circWeight == 0;
			}
			ExtendedBody* self = static_cast<ExtendedBody*>(this);
			weight_t w = self->weights[std::find(preds.begin(), preds.end(), a) - preds.begin()];
			return (circWeight -= w) <= 0  && (circWeight + w) > 0;
		}
		//! notifies this body about the fact that one of its predecessors lost its source
		/*!
		 * returns true if body is no longer a valid source
		 */
		bool removeSource(UfsAtomNode* a) {
			if (!extended()) { return ++circWeight == 1; }
			ExtendedBody* self = static_cast<ExtendedBody*>(this);
			if (self->weights == 0) { return ++circWeight == 1 || self->trivWeight < 0; }
			weight_t w = self->weights[std::find(preds.begin(), preds.end(), a) - preds.begin()];
			return ((circWeight += w) > 0 && (circWeight - w) <= 0)
				|| self->trivWeight < 0;
		}
		
		//! returns true if this body currently can be a source
		bool canBeSource() const {
			return circWeight <= 0 && (!extended() || static_cast<const ExtendedBody*>(this)->trivWeight >= 0);
		}
		//! adds predecessors without source to unfounded-queue.
		/*!
		 * returns true if this body currently can be a source.
		 * Otherwise adds from its predecessors those without a source to unfounded-queue.
		 */
		bool canBeSource(UfsAtomNode* a, DefaultUnfoundedCheck* ufs) {
			if (ufs->solver().isFalse(lit)) return false;
			if (scc != a->scc || canBeSource()) return true;
			if (!extended()) {
				// this body can only be a source, if we can re-establish the sources of its predecessors
				for (AtomVec::const_iterator z = preds.begin(); z != preds.end(); ++z) {
					if ((*z)->source == 0 && !ufs->solver().isFalse((*z)->lit)) {
						ufs->enqueueUnfounded(*z);
					}
				}
				return false;
			}
			return static_cast<ExtendedBody*>(this)->canBeSourceExt(a, ufs);
		}
		//! destroys this body object
		void destroy();
		//! checks whether this body (or some of its predecessors) is a reason for the active unfounded set
		void computeReason(DefaultUnfoundedCheck* ufs, bool invSign);
	private:

		// checks whether this (extended) body is still a valid source.
		// This function is called whenever predecessor of an extended rule became false. 
		// If the body is no langer a valid source, it is added to ufs's invalid-Queue
		// PRE: extended() == true
		void triggerCheckInvalid(Literal, uint32 data, DefaultUnfoundedCheck* ufs) {
			assert(extended());
			ExtendedBody* self = static_cast<ExtendedBody*>(this);
			if ( (data & 1) != 0 ) {	// from other scc
				weight_t w = self->weights ? self->weights[data >> 1] : 1;
				self->circWeight	+= w;
				self->trivWeight	-= w;
			}
			if (watches > 0 && self->trivWeight < 0) {
				ufs->invalid_.push_back(this);
			}
		}
		// A successor of a choice rule became false. Check if that successor
		// is currently sourced by the choice rule. If so, explicitly remove the 
		// source. Note: For other body-types this check is not necessary, because
		// setting a head to false falsifies all its bodies which implicitly triggers
		// the removal of the head's source.
		void triggerRemSource(Literal, uint32 idx, DefaultUnfoundedCheck* ufs) {
			if (succs[idx]->source == this) {
				assert(ufs->solver().isFalse(succs[idx]->lit) && succs[idx]->watch == this);
				succs[idx]->source = 0;
				ufs->sourceQueue_.push_back(succs[idx]);
			}
		}
	};
	//! Some more data needed to handle cardinality- and weight constraints
	struct ExtendedBody : public UfsBodyNode {
		explicit ExtendedBody(Literal eq, uint32 scc, weight_t lb) 
			: UfsBodyNode(eq, scc), weights(0), bound(lb), trivWeight(0) {
			typeOrUnfounded = 1;
		}
		~ExtendedBody() { delete [] weights; }
		//! \see UfsBodyNode::canBeSource
		bool canBeSourceExt(UfsAtomNode* a, DefaultUnfoundedCheck* ufs);
		LitVec		lits;				/*!< Literals from other sccs */
		weight_t* weights;		/*!< weights if body of a weight rule */
		weight_t	bound;			/*!< lower bound */
		weight_t	trivWeight;	/*!< reachable weight counting only non-false subgoals from different sccs
													 >= 0 : source
													 < 0	: remove source	*/
	};	
	//! inefficient - only used for testing
	UfsBodyNode* bodyNode(Literal b) const {
		for (BodyVec::size_type i = 0; i < bodies_.size(); ++i) {
			if (bodies_[i]->lit == b) { return bodies_[i]; }
		}
		return 0;
	}
	//! inefficient - only used for testing
	UfsAtomNode* atomNode(Literal h) const {
		for (AtomVec::size_type i = 0; i < atoms_.size(); ++i) {
			if (atoms_[i]->lit == h) { return atoms_[i]; }
		}
		return 0;
	}
private:	
	// A watch-structure used to notify extended bodies affected by literal assignments
	struct ExtendedWatch {
		typedef void (UfsBodyNode::*Notify)(Literal p, uint32 data, DefaultUnfoundedCheck* ufs);
		ExtendedWatch(UfsBodyNode* b, Notify fn, uint32 data)
			: body_(b), notify_(fn), data_(data) {}
		void setData(uint32 d) { data_ = d; }
		void notify(Literal p, DefaultUnfoundedCheck* ufs) const {
			(body_->*notify_)(p, data_, ufs);
		}
	private:
		UfsBodyNode*	body_;
		Notify				notify_;
		uint32				data_;
	};
	typedef std::deque<UfsAtomNode*> Queue;
	typedef PodVector<ExtendedWatch>::type ExtWatches;
// -------------------------------------------------------------------------------------------	
// constraint interface
	PropResult propagate(const Literal& p, uint32&, Solver& s);
	void reason(const Literal&, LitVec&);
	ConstraintType type() const { return Constraint_t::native_constraint; }	
// -------------------------------------------------------------------------------------------
// Graph initialization
	bool relevantPrgAtom(PrgAtomNode* a) const { return a->hasVar() && a->scc() != PrgNode::noScc && !solver_->isFalse(a->literal()); }
	bool relevantPrgBody(PrgBodyNode* b) const { return b->hasVar() && !solver_->isFalse(b->literal()); }
	UfsAtomNode* addAtom(PrgAtomNode*);
	UfsBodyNode* addBody(PrgBodyNode*, const AtomList& progAtoms);
	UfsBodyNode* addExtendedBody(PrgBodyNode*, const AtomList&);
	void addEdges(UfsBodyNode* b, UfsAtomNode* h);
	void addEdges(UfsAtomNode* h, UfsBodyNode* b, weight_t w);
// -------------------------------------------------------------------------------------------	
// Finding unfounded sets
	bool findUnfoundedSet();
	bool findSource(UfsAtomNode* head);
	void setSource(UfsBodyNode* body, UfsAtomNode* head);
	void removeSource(UfsBodyNode* body);
	void propagateSource();
// -------------------------------------------------------------------------------------------	
// Propagating unfounded sets
	bool assertAtom();
	bool assertSet();	
	bool assertAtom(Literal a);
	void computeReason(bool invSign = false);
	void createLoopFormula();
// -------------------------------------------------------------------------------------------	
// helpers
	void addExtendedWatch(Literal p, const ExtendedWatch& w);
	void enqueueTodo(UfsAtomNode* head) {
		if (head->pickedOrTodo == 0) {
			todo_.push_back(head);
			head->pickedOrTodo = 1;
		}
	}
	void enqueueUnfounded(UfsAtomNode* head) {
		if (head->typeOrUnfounded == 0) {
			unfounded_.push_back(head);
			head->typeOrUnfounded = 1;
		}
	}
	UfsAtomNode* dequeueTodo() {
		UfsAtomNode* head = todo_.front();
		todo_.pop_front();
		head->pickedOrTodo = 0;
		return head;
	}
	UfsAtomNode* dequeueUnfounded() {
		UfsAtomNode* head = unfounded_.front();
		unfounded_.pop_front();
		head->typeOrUnfounded = 0;
		return head;
	}	
	bool inReason(Literal p)				const { return solver_->data(p.var()) != 0; }
	void markBodyLitInReason(Literal p)		{ solver_->data(p.var()) |= 1; }
	void markAtomLitInReason(Literal p)		{ solver_->data(p.var()) |= 1; pickedAtoms_.push_back(p); }

// -------------------------------------------------------------------------------------------	
	AtomVec					atoms_;				// Atoms of the positive dependency graph
	BodyVec					bodies_;			// Bodies of the positive dependency graph
	BodyVec					invalid_;			// These bodies became invalid during unit propagation - they can no longer be source
	ExtWatches			extWatches_;	// Watches for handling choice-/cardinality- and weight rules
	AtomVec					sourceQueue_;	// Used during source-pointer propagation
	Queue						todo_;				// Atom that recently lost their source
	Queue						unfounded_;		// Atoms that are unfounded wrt the current assignment (limited to one scc)
	BodyVec					picked_;			// Bodies seen during reason computation
	LitVec					pickedAtoms_;	// Literals in a reason from non-false extended bodies
	LitVec					loopAtoms_;		// only used if strategy_ == shared_reason
	Solver*					solver_;			// my solver
	LitVec*					reasons_;			// only used if strategy_ == only_reason. reasons_[v] reason why v is unfounded
	ClauseCreator*	activeClause_;// activeClause_[0] is the current unfounded atom
	ReasonStrategy	strategy_;		// what kind of reasons to compute?
};
}
#endif
