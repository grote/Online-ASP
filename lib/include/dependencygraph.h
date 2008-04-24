#ifndef DEPENDENCYGRAPH_H
#define DEPENDENCYGRAPH_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	class DependencyGraph
	{
	protected:
		typedef std::map<std::pair<std::string, int>, Node*> PredicateMap;
	public:
		DependencyGraph();
		Node *createStatementNode(Statement *r);
		Node *createPredicateNode(PredicateLiteral *pred);
		Node *createPredicateNode();
		void calcSCCs();
		bool check(Grounder *g);
		std::vector<std::pair<std::string, int> > *getPred();
		virtual ~DependencyGraph();
	protected:
		void calcSCCDep(Node *v1, SCC *scc, bool &root);
		void tarjan(Node *v1, std::stack<Node*> &stack, int &index);
	protected:
		int uids_;
		NodeVector ruleNodes_;
		NodeVector predicateNodes_;
		PredicateMap predicateMap_;
		std::vector<std::pair<std::string, int> > pred_;
		SCCVector sccs_;
		SCCSet sccRoots_;
	};
}

#endif

