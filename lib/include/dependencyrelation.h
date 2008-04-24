#ifndef DEPENDENCYRELATION_H
#define DEPENDENCYRELATION_H

namespace NS_GRINGO
{
	class DependencyRelation
	{
	public:
		inline DependencyRelation(int size);
		inline void add(int a, int b);
		inline bool contains(int a, int b);
	protected:
		std::vector<bool> relation_;
	};

	DependencyRelation::DependencyRelation(int size) : relation_(size*(size-1)/2, false)
	{
	}

	void DependencyRelation::add(int a, int b)
	{
		if(a == b)
			return;
		if(a > b)
			assert(false);
		relation_[relation_.size() - (b*(b+1))/2 + a] = true;
		
	}

	bool DependencyRelation::contains(int a, int b)
	{
		if(a == b)
			return true;
		if(a > b)
			return false;
		return relation_[relation_.size() - (b*(b+1))/2 + a];
	}
}

#endif

