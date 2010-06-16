// Copyright (c) 2010, Torsten Grote
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

#ifndef EXTERNALKNOWLEDGE_H
#define EXTERNALKNOWLEDGE_H

#include <gringo/gringo.h>
#include <gringo/gringoexception.h>
#include <gringo/value.h>

namespace gringo
{
	class ExternalKnowledge
	{
	public:
		ExternalKnowledge();
		void get(gringo::Grounder* grounder, NS_OUTPUT::Output* output);
		void add(GroundAtom external, int uid);
		bool checkExternal(NS_OUTPUT::Object* object);
	private:
		UidValueMap externals_;
	};
}

#endif

