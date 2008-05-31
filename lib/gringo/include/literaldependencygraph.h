#ifndef LITERALDEPENDENCYGRAPH_H
#define LITERALDEPENDENCYGRAPH_H

#include <gringo.h>

namespace NS_GRINGO
{
	class LDG
	{
		friend class LDGBuilder;
	private:
		struct LiteralNode;
		struct VarNode;

		typedef std::vector<VarNode*>     VarNodeVector;
		typedef std::vector<LiteralNode*> LiteralNodeVector;
		
		struct LiteralNode
		{
			LiteralNode(Literal *l, bool head);
			Literal *l_;
			int done_;
			int in_;
			bool head_;
			VarNodeVector out_;
			LDGVector sub_;
		};
		struct VarNode
		{
			VarNode(int var);
			bool done_;
			int var_;
			LiteralNodeVector out_;
		};
		struct LiteralData
		{
			LiteralData(LiteralNode *n);
			LiteralNode *n_;
			VarVector needed_;
			VarVector provided_;
		};

		typedef std::map<Literal*, LiteralData*> LiteralDataMap;
	public:
		LDG();

		const VarVector &getGlobalVars() const;
		const VarVector &getParentVars() const;
		void check(VarVector &free);

		void start(LiteralSet &list);
		void propagate(Literal *l, LiteralSet &list);

		const VarVector &getNeededVars(Literal *l) const;
		const VarVector &getProvidedVars(Literal *l) const;

		~LDG();
	protected:
		VarVector         parentVars_;
		VarVector         globalVars_;
		LiteralNodeVector litNodes_;
		VarNodeVector     varNodes_;
		LiteralDataMap    litMap_;
	};

	class LDGBuilder
	{
	protected:
		typedef std::map<int, LDG::VarNode*> VarNodeMap;
		typedef std::map<int, LDG::LiteralNode*> LiteralNodeMap;

		struct GraphNode
		{
			bool head_;
			Literal *l_;
			LDGBuilderVector sub_;
			GraphNode(Literal *l, bool head);
		};
		typedef std::vector<GraphNode*> GraphNodeVector;
	public:
		LDGBuilder(LDG *dg);
		void addHead(Literal *l);
		void addToBody(Literal *l);
		void addGraph(LDGBuilder *dg);

		void create();

		void createGraphNode(Literal *l, bool head);
		void createNode(Literal *l, bool head, const VarSet &needed, const VarSet &provided);

		~LDGBuilder();

	protected:
		LDG::LiteralNode *createLiteralNode(Literal *l, bool head);
		LDG::VarNode *createVarNode(int var);
		void createSubGraph(LDGBuilder *parent, bool head, LDG::LiteralNode *l);

	protected:
		LDGBuilder *parent_;
		LDG::LiteralNode *parentNode_;
		LDG *dg_;
		LiteralVector head_;
		LiteralVector body_;
		VarNodeMap varNodes_;
		GraphNodeVector graphNodes_;
	};

}

#endif

