#include <iostream>
#include <fstream>
#include <lparseparser.h>
#include <grounder.h>
#include <smodelsoutput.h>
#include <lparseoutput.h>

using namespace NS_GRINGO;

int main(int argc, char *argv[])
{
	bool smodels = false;
	bool lparse  = false;
	std::vector<std::istream*> streams;
	if(argc > 1 && strcmp(argv[1], "-l") == 0)
	{
		argc--;
		argv++;
		smodels = true;
	}
	else if(argc > 1 && strcmp(argv[1], "-p") == 0)
	{
		argc--;
		argv++;
		lparse = true;
	}
	if(argc == 1)
		streams.push_back(new std::istream(std::cin.rdbuf()));
	else
		for(int i = 1; i < argc; i++)
			streams.push_back(new std::fstream(argv[i]));
	
	LparseParser parser(streams);
	Grounder *grounder = parser.parse();
	if(grounder)
	{
		std::cerr << "parsing successful" << std::endl;
		if(smodels)
		{
			NS_OUTPUT::SmodelsOutput output(&std::cout);
			grounder->start(output);
		}
		else if(lparse)
		{
			NS_OUTPUT::LparseOutput output(&std::cout);
			grounder->start(output);
		}
		else
		{
			grounder->start();
		}

		delete grounder;
	}
	else
	{
		std::cerr << "parsing failed" << std::endl;
	}

	for(std::vector<std::istream*>::iterator it = streams.begin(); it != streams.end(); it++)
		delete *it;

	return 0;
}

