#ifndef DEPENDENCYGRAPH_H
#define DEPENDENCYGRAPH_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	class DependencyGraph
	{
	protected:
		typedef std::map<Signature, Node*> PredicateMap;
	public:
		DependencyGraph();
		Node *createStatementNode(Statement *r, bool preserveOrder = false);
		Node *createPredicateNode(PredicateLiteral *pred);
		Node *createPredicateNode();
		NodeVector &getPredNodes();
		void calcSCCs();
		bool check(Grounder *g);
		SignatureVector *getPred();
		virtual ~DependencyGraph();
	protected:
		void calcSCCDep(Node *v1, SCC *scc, bool &root);
		void tarjan(Node *v1, std::stack<Node*> &stack, int &index);
	protected:
		int uids_;
		NodeVector ruleNodes_;
		NodeVector predicateNodes_;
		PredicateMap predicateMap_;
		SignatureVector pred_;
		SCCVector sccs_;
		SCCSet sccRoots_;
		Node *last_;
	};
}

#endif

