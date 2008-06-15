#include "lparseoutput.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;
		
LparseOutput::LparseOutput(std::ostream *out) : Output(out)
{
}

void LparseOutput::initialize(SignatureVector *pred)
{
	Output::initialize(pred);
	if(hideAll_)
		*out_ << "#hide." << std::endl;
	for(std::map<Signature, bool>::iterator it = hide_.begin(); it  != hide_.end(); it++)
	{
		if(hideAll_ != it->second)
		{
			*out_ << (hideAll_ ? "#show " : "#hide ") << *(it->first.first);
			if(it->first.second > 0)
			{
				*out_ << "(X0";
				for(int i=1; i < it->first.second; i++)
					*out_ << ", X" << i;
				*out_ << ")";
			}
			*out_ << "." << std::endl;
		}
	}
}

void LparseOutput::print(NS_OUTPUT::Object *r)
{
	r->print_plain(this, *out_);
}

void LparseOutput::finalize()
{
}

LparseOutput::~LparseOutput()
{
}

