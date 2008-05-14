#include "printable.h"

using namespace NS_GRINGO;

Printable::Printable()
{
}

Printable::~Printable()
{
}

std::ostream &NS_GRINGO::operator<<(std::ostream &stream, Printable &p)
{
	p.print(stream);
	return stream;
}

std::ostream &NS_GRINGO::operator<<(std::ostream &stream, Printable *p)
{
	p->print(stream);
	return stream;
}

