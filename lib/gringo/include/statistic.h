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

#ifndef STATISTIC_H
#define STATISTIC_H

#include <gringo.h>

namespace NS_GRINGO
{
	/**
	 * \brief Class for statistics mainly used for ASPils Output
	 */
	class Statistic
	{
	public:
		/// Constructor
		Statistic();
		virtual void print(std::ostream* out) const;
		void startWeight();
		void endWeight();
		void startBound();
		void endBound();
		void possibleNegativeVariable();
		void possibleNegative();
		/// Destructor
		virtual ~Statistic();

	public:
		unsigned int rules;
		unsigned int facts;
		unsigned int integrityConstraints;
		unsigned int disjunctions;
		unsigned int sum;
		unsigned int count;
		unsigned int min;
		unsigned int max;
		bool lowerBounds;
		bool upperBounds;
		bool minimize;
		bool maximize;

		// true if negative values exist in the program out of weights
		bool negativeValues_;
		// true if negative values exist in weights
		bool negative_;
		// true if a variable exists in a weight
		bool varInWeight_;
	private:
		bool weight_;
		bool bound_;
	};
}

#endif

