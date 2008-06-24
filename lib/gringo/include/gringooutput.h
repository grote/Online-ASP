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

#ifndef GRINGOOUTPUT_H
#define GRINGOOUTPUT_H

#include <gringo.h>
#include <output.h>

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class GrinGoOutput : public Output
		{
		public:
			GrinGoOutput(std::ostream *out);
			virtual void initialize(SignatureVector *pred);
			virtual void print(NS_OUTPUT::Object *o);
			virtual void finalize();
			virtual void addOptimizedID(unsigned int id);
			virtual ~GrinGoOutput();
		protected:
			IntVector optimizedIDs_;
		};
	}
}

#endif

