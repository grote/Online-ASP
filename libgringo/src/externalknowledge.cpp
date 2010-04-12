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

#include <gringo/externalknowledge.h>
#include <gringo/onlineparser.h>

using namespace gringo;

ExternalKnowledge::ExternalKnowledge()
{

}

void ExternalKnowledge::add(GroundAtom external, int uid) {
	externals_.insert(make_pair(external, uid));
}

bool ExternalKnowledge::checkExternal(NS_OUTPUT::Object* object) {
	assert(dynamic_cast<NS_OUTPUT::Atom*>(object));
	NS_OUTPUT::Atom* atom = static_cast<NS_OUTPUT::Atom*>(object);

	// try to find atom in externals
	if(externals_.find(std::make_pair(atom->predUid_, atom->values_)) == externals_.end()) {
		return false;
	}
	else {
		return true;
	}
}

IntSet* ExternalKnowledge::getExternalIDs() {
	IntSet* result = new IntSet();

	for(UidValueMap::iterator i = externals_.begin(); i != externals_.end(); ++i) {
		result->insert(i->second);
	}

	return result;
}


void ExternalKnowledge::get(gringo::Grounder* grounder, NS_OUTPUT::Output* output)
{
	output_ = static_cast<NS_OUTPUT::IClaspOutput*>(output);

	std::cout << std::endl << "Please enter external ground facts below:" << std::endl;

	OnlineParser parser(grounder, &std::cin);
	if(!parser.parse(output))
		throw gringo::GrinGoException("Parsing failed.");
}

void ExternalKnowledge::addNewFact(NS_OUTPUT::Object* fact) {
	new_facts_.insert(fact->uid_);
	output_->unfreezeAtom(fact->uid_);
}

IntSet* ExternalKnowledge::getNewFacts() {
	return &new_facts_;
}
