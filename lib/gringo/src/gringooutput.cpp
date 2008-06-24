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

#include "gringooutput.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;
		
GrinGoOutput::GrinGoOutput(std::ostream *out) : Output(out)
{
}

const char* HEADER = "1";
const char* VERSION = "1";

void GrinGoOutput::initialize(SignatureVector *pred)
{
	Output::initialize(pred);
	*out_ << HEADER << " " << VERSION << " " << "normalform" << " " << "0 0" << NL;
}

void GrinGoOutput::print(NS_OUTPUT::Object *r)
{
	r->print(this, *out_);
}

void GrinGoOutput::finalize()
{
	//TODO: write the optimize statement for all added sums
	*out_ << "0 0" << NL;
}

GrinGoOutput::~GrinGoOutput()
{
}

