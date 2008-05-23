#ifndef LITERALDEPENDENCYGRAPH_H
#define LITERALDEPENDENCYGRAPH_H

#include <gringo.h>

namespace NS_GRINGO
{
	class LiteralDependencyGraph
	{
	private:
		struct Node;
		typedef std::vector<Node*>   NodeVector;
		typedef std::set<Node*>      NodeSet;
		typedef std::map<int, Node*> VarNodeMap;
		struct Node
		{
			Node(Literal *l) : l_(l), var_(0)
			{
			}
			Node(int var) : l_(0), var_(var)
			{
			}
			Literal *l_;
			int var_;
			NodeSet out_;
			NodeSet in_;
		};
	public:
		LiteralDependencyGraph(Literal *head, LiteralVector *body);
		void getUnboundVars(VarVector &free);
		~LiteralDependencyGraph();
	protected:
		VarNodeMap varMap_;
		NodeVector litNodes_;
		NodeVector varNodes_;
	};
}

#endif

