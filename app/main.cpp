#include <iostream>
#include <fstream>
#include <lparseparser.h>
#include <lparseconverter.h>
#include <grounder.h>
#include <smodelsoutput.h>
#include <lparseoutput.h>
#include <claspoutput.h>
#include <gringooutput.h>

// evil hack :)
NS_GRINGO::LparseConverter *converter = 0;
NS_GRINGO::Grounder *grounder = 0;
bool success = false;
void start_grounding(NS_GRINGO::NS_OUTPUT::Output &output)
{
	if(grounder)
	{
		grounder->start(output);
		delete grounder;
	}
	else
	{
		success = converter->parse(&output);
	}
}

#include "clasp_main.h"

using namespace NS_GRINGO;

int main(int argc, char *argv[])
{
	char *arg0 = argv[0];
	bool smodels = false;
	bool lparse  = false;
	bool clasp   = false;
	bool convert = false;
	std::vector<std::istream*> streams;
	while(argc > 1)
	{
		if(strcmp(argv[1], "--convert") == 0)
		{
			argc--;
			argv++;
			convert = true;
		}
		else if(strcmp(argv[1], "-l") == 0)
		{
			argc--;
			argv++;
			smodels = true;
		}
		else if(strcmp(argv[1], "-p") == 0)
		{
			argc--;
			argv++;
			lparse = true;
		}
		else if(strcmp(argv[1], "-c") == 0)
		{
			argc--;
			argv++;
			clasp = true;
		}
		else if(strcmp(argv[1], "--help") == 0)
		{
			std::cerr << "Usage: " << arg0 << " [gringo options] [files] [--[ clasp options]] " << std::endl << std::endl;
			std::cerr << "gringo options are: " << std::endl;
			std::cerr << "	-c : Use internal interface to clasp" << std::endl;
			std::cerr << "	-l : Print smodels output" << std::endl;
			std::cerr << "	-p : Print plain lparse-like output" << std::endl;
			std::cerr << "	The default output is something strange :)" << std::endl << std::endl;
			std::cerr << "clasp options are: " << std::endl;
			int   argc_c = 2;
			char *argv_c[] = { arg0, argv[1] };
			clasp_main(argc_c, argv_c);
			return 0;
		}
		else
			break;
	}
	if(argc == 1)
		streams.push_back(new std::istream(std::cin.rdbuf()));
	else
		while(argc > 1)
		{
			argc--;
			argv++;
			if(strcmp(argv[0], "--") == 0)
				break;
			else
				streams.push_back(new std::fstream(argv[0]));
		}
	argv[0] = arg0;
	
	if(convert)
	{
		LparseConverter parser(streams);
		converter = &parser;
		if(smodels)
		{
			NS_OUTPUT::SmodelsOutput output(&std::cout);
			success = parser.parse(&output);
		}
		else if(lparse)
		{
			NS_OUTPUT::LparseOutput output(&std::cout);
			success = parser.parse(&output);
		}
		else if(clasp)
		{
			clasp_main(argc, argv);
		}
		else
		{
			NS_OUTPUT::GrinGoOutput output(&std::cout);
			success = parser.parse(&output);
		}
		if(success)
			std::cerr << "parsing successful" << std::endl;
		else
			std::cerr << "parsing failed" << std::endl;
	}
	else
	{
		LparseParser parser(streams);
		grounder = parser.parse();
		if(grounder)
		{
			std::cerr << "parsing successful" << std::endl;
			if(smodels)
			{
				NS_OUTPUT::SmodelsOutput output(&std::cout);
				grounder->start(output);
				delete grounder;
			}
			else if(lparse)
			{
				NS_OUTPUT::LparseOutput output(&std::cout);
				grounder->start(output);
				delete grounder;
			}
			else if(clasp)
			{
				clasp_main(argc, argv);
			}
			else
			{
				grounder->start();
				delete grounder;
			}
		}
		else
		{
			std::cerr << "parsing failed" << std::endl;
		}
	}

	for(std::vector<std::istream*>::iterator it = streams.begin(); it != streams.end(); it++)
		delete *it;

	return 0;
}

