#include <gringo.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <lparseparser.h>
#include <lparseconverter.h>
#include <grounder.h>
#include <smodelsoutput.h>
#include <lparseoutput.h>
#include <claspoutput.h>
#include <gringooutput.h>
#include <gringoexception.h>

// evil hack :)
NS_GRINGO::GrinGoParser* parser = 0;
NS_GRINGO::NS_OUTPUT::Output* output = 0;

bool convert = false;
void start_grounding()
{
	bool success = parser->parse(output);
	if(success)
	{
		std::cerr << "Parsing successful" << std::endl;
		if(!convert)
		{
			NS_GRINGO::Grounder *grounder = static_cast<NS_GRINGO::LparseParser*>(parser)->getGrounder();
			std::auto_ptr<NS_GRINGO::Grounder> grounderM(grounder);
			grounder->start();
		}
	}
	else
	{
		std::cerr << "Parsing failed" << std::endl;
	}
}

#include "clasp_main.h"

using namespace NS_GRINGO;


int main(int argc, char *argv[])
{
	char *arg0 = argv[0];
	bool files = false;
	enum Format {SMODELS, GRINGO, CLASP, LPARSE} format = SMODELS;
	std::stringstream *ss = new std::stringstream();
	std::vector<std::istream*> streams;
	streams.push_back(ss);
	try
	{
		while(argc > 1)
		{
			if(strcmp(argv[1], "--convert") == 0)
			{
				convert = true;
			}
			else if(strcmp(argv[1], "-g") == 0)
			{
				format = GRINGO;
			}
			else if(strcmp(argv[1], "-l") == 0)
			{
				format = SMODELS;
			}
			else if(strcmp(argv[1], "-p") == 0)
			{
				format = LPARSE;
			}
			else if(strcmp(argv[1], "-c") == 0)
			{
				format = CLASP;
			}
			else if(strcmp(argv[1], "--const") == 0)
			{
				if(argc == 2)
					throw GrinGoException("error: constant missing");
				argc--;
				argv++;
				*ss << "#const " << argv[1] << "." << std::endl;
			}
			else if(strcmp(argv[1], "--help") == 0)
			{
				std::cerr << "Usage: " << arg0 << " (gringo option|file)* [--[ clasp options]] " << std::endl << std::endl;
				std::cerr << "gringo options are: " << std::endl;
				std::cerr << "	--convert   : convert already ground program" << std::endl;
				std::cerr << "	--const c=v : Pass constant c equal value v to grounder" << std::endl;
				std::cerr << "	-c          : Use internal interface to clasp" << std::endl;
				std::cerr << "	-l          : Print smodels output" << std::endl;
				std::cerr << "	-p          : Print plain lparse-like output" << std::endl;
				std::cerr << "	-g          : Print experimental output" << std::endl;
				std::cerr << "	The default output is smodels output (-l)" << std::endl << std::endl;
				std::cerr << "clasp options are: " << std::endl;
				int   argc_c = 2;
				char *argv_c[] = { arg0, argv[1] };
				clasp_main(argc_c, argv_c);
				for(std::vector<std::istream*>::iterator it = streams.begin(); it != streams.end(); it++)
					delete *it;
				return 0;
			}
			else if(strcmp(argv[1], "--") == 0)
			{
				argc--;
				argv++;
				break;
			}
			else if(strncmp(argv[1], "-", 1) == 0)
			{
				throw GrinGoException(std::string("error: unknown option: ") + argv[1]);
			}
			else
			{
				streams.push_back(new std::fstream(argv[1]));
				if(streams.back()->fail())
					throw GrinGoException(std::string("error: could not open file: ") + argv[1]);
				files = true;
			}
			argc--;
			argv++;
		}
		if(!files)
			streams.push_back(new std::istream(std::cin.rdbuf()));
		argv[0] = arg0;
		
		if(convert)
			parser = new LparseConverter(streams);
		else
			parser = new LparseParser(streams);
		
		switch(format)
		{
			case SMODELS:
				output = new NS_OUTPUT::SmodelsOutput(&std::cout);
				start_grounding();
				break;
			case LPARSE:
				output = new NS_OUTPUT::LparseOutput(&std::cout);
				start_grounding();
				break;
			case GRINGO:
				output = new NS_OUTPUT::GrinGoOutput(&std::cout);
				start_grounding();
				break;
			case CLASP:
				clasp_main(argc, argv);
				break;
		}
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	if(output)
		delete output;
	if(parser)
		delete parser;
	for(std::vector<std::istream*>::iterator it = streams.begin(); it != streams.end(); it++)
		delete *it;

	return 0;
}

