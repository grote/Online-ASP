#include "lparseoutput.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;
		
LparseOutput::LparseOutput(std::ostream *out) : Output(out)
{
}

void LparseOutput::initialize(SignatureVector *pred)
{
	Output::initialize(pred);
}

void LparseOutput::print(NS_OUTPUT::Object *r)
{
	r->print_plain(*out_);
}

void LparseOutput::finalize()
{
}

LparseOutput::~LparseOutput()
{
}

