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

#ifdef WITH_CLASP
#ifndef CLASPOUTPUT_H
#define CLASPOUTPUT_H

#include <gringo.h>
#include <smodelsconverter.h>
#include <clasp/include/lparse_reader.h>

namespace Clasp
{
	class ProgramBuilder;
}

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		class ClaspOutput : public SmodelsConverter
		{
		public:
			ClaspOutput(Clasp::ProgramBuilder *b, Clasp::LparseReader::TransformMode tf);
			void initialize(SignatureVector *pred);
			void finalize();
			int newUid();
			Clasp::LparseStats &getStats();
			~ClaspOutput();
		protected:
			void printBasicRule(int head, const IntVector &pos, const IntVector &neg);
			void printConstraintRule(int head, int bound, const IntVector &pos, const IntVector &neg);
			void printChoiceRule(const IntVector &head, const IntVector &pos, const IntVector &neg);
			void printWeightRule(int head, int bound, const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg);
			void printMinimizeRule(const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg);
			void printDisjunctiveRule(const IntVector &head, const IntVector &pos, const IntVector &neg);
			void printComputeRule(int models, const IntVector &pos, const IntVector &neg);
		private:
			Clasp::LparseStats stats_;
			Clasp::ProgramBuilder *b_;
			Clasp::LparseReader::TransformMode tf_;
		};
	}
}

#endif
#endif

