// Copyright (c) 2008, Roland Kaminski
//
// This file is part of GrinGo.
//
// GrinGo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrinGo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrinGo.  If not, see <http://www.gnu.org/licenses/>.

#include "statistic.h"

using namespace NS_GRINGO;

Statistic::Statistic()
{
}

void Statistic::print(std::ostream* out) const
{
	*out << "rules: " << rules << " facts: " << facts << " integrityConstraints: " << integrityConstraints <<
		" disjunctions: " << disjunctions << " sum: " << sum << " count: " << count << " min: " << min <<
		" max: " << max  << " lowerBounds: " << lowerBounds <<
		" upperBounds: " << upperBounds << " minimize: " << minimize << " maximize: " << maximize << std::endl;

	*out << "negativeValues: " << negativeValues_ << " varInWeight?: " << varInWeight_ << " negative in weights: " << negative_ << std::endl;
}

void Statistic::possibleNegativeVariable()
{
	std::cout << "possNegativeVar" << std::endl;
	if (weightList_)
		varInWeight_ = true;
}

void Statistic::possibleNegative()
{
	std::cout << "possNegative" << std::endl;
	if (weightList_)
	{
		negative_ = true;
	}
	else
		negativeValues_ = true;
}


void Statistic::startWeightList()
{
	std::cout << "startWeightList" << std::endl;
	weightList_ = true;
}

void Statistic::endWeightList()
{
	std::cout << "EndWeightList" << std::endl;
	weightList_ = false;
}

Statistic::~Statistic()
{

}

