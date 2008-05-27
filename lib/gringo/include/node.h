#ifndef NODE_H
#define NODE_H

#include <gringo.h>
#include <value.h>

namespace NS_GRINGO
{
	class Node
	{
		friend class DependencyGraph;
	public:
		Node(int uid = 0, Statement *rule = 0);
		NodeVector *getDependency();
		NodeVector *getNegDependency();
		Statement* getStatement();
		void addDependency(Node *n, bool neg = false);
		bool complete();
		bool solved();
		void setSolved(bool solved);
		void reset();
		void finish();
		int getUid() const;
		void evaluate();
		bool hasFacts();
		void addFact(const ValueVector &values);
		bool isFact(const ValueVector &values);
		bool inDomain(const ValueVector &values);
		void addDomain(const ValueVector &values);
		void removeDomain(const ValueVector &values);
		ValueVectorSet &getDomain();
		virtual ~Node();
	protected:
		int uid_;
		int defines_;
		int lowlink_;
		int index_;
		bool stacked_;
		bool done_;
		SCC *scc_;
		Statement  *rule_;
		NodeVector dependency_;
		NodeVector negDependency_;
		ValueVectorSet facts_;
		ValueVectorSet domain_;
		bool solved_;
	};
}

#endif

