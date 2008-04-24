#ifndef SCC_H
#define SCC_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	class SCC : public Printable
	{
		friend class DependencyGraph;
	public:
		enum SCCType { FACT, BASIC, NORMAL};
	public:
		SCC();
		void print(std::ostream &out);
		Evaluator *getEvaluator();
		bool check(Grounder *g);
		StatementVector *getStatements();
		SCCType getType();
		virtual ~SCC();
	protected:
		SCCType type_;
		int edges_;
		StatementVector rules_;
		SCCSet sccs_;
		Evaluator *eval_;
	};
}

#endif
