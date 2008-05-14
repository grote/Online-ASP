#ifndef DLVGROUNDER_H
#define DLVGROUNDER_H

#include <gringo.h>
//#include <dependencyrelation.h>

namespace NS_GRINGO
{
	class DLVGrounder
	{
	public:
		DLVGrounder(Grounder *g, Groundable *r, LiteralVector &lit, VarSet &glob, VarSet &relevant);
		DLVGrounder(Grounder *g, Groundable *r, LiteralVector &lit, VarSet &glob, VarSet &relevant, VarSet &index);
		void debug();
		void ground();
		~DLVGrounder();
	private:
		int closestBinder(int l, VarVector &vars, std::map<int,int> &firstBinder);
                void sortLiterals();
		void cacheVariables();
		void calcDependency();
	public:
		Grounder *g_;
		Groundable *r_;
		LiteralVector &lit_;
		IndexedDomainVector dom_;
		std::vector<VarVector> var_;
		std::vector<VarVector> dep_;
		//DependencyRelation rel_;
		VarVector closestBinderVar_;
		VarVector closestBinderDep_;
		VarVector closestBinderRel_;
		VarVector global_;
		VarVector relevant_;
	};
}

#endif

