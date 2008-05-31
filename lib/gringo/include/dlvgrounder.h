#ifndef DLVGROUNDER_H
#define DLVGROUNDER_H

#include <gringo.h>

namespace NS_GRINGO
{
	class DLVGrounder
	{
	public:
		DLVGrounder(Grounder *g, Groundable *r, int lits, LDG *dg, const VarVector &relevant);
		void ground();
		void debug();
		~DLVGrounder();
	private:
		int closestBinder(int l, VarVector &vars, std::map<int,int> &firstBinder);
                void sortLiterals(LDG *dg);
		void calcDependency();
	public:
		Grounder *g_;
		Groundable *r_;
		LiteralVector lit_;
		IndexedDomainVector dom_;
		std::vector<VarVector> var_;
		std::vector<VarVector> dep_;
		VarVector closestBinderVar_;
		VarVector closestBinderDep_;
		VarVector closestBinderRel_;
		VarVector global_;
		VarVector relevant_;
	};
}

#endif

