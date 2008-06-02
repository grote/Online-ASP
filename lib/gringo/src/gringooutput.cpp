#include "gringooutput.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;
		
GrinGoOutput::GrinGoOutput(std::ostream *out) : Output(out)
{
}

void GrinGoOutput::initialize(SignatureVector *pred)
{
	Output::initialize(pred);
}

void GrinGoOutput::print(NS_OUTPUT::Object *r)
{
	r->print(*out_);
}

void GrinGoOutput::finalize()
{
}

GrinGoOutput::~GrinGoOutput()
{
}

