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

#include <gringo.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <lparseparser.h>
#include <lparseconverter.h>
#include <grounder.h>
#include <smodelsoutput.h>
#include <lparseoutput.h>
#include <pilsoutput.h>
#include <gringoexception.h>

#ifdef WITH_CLASP
#	include <claspoutput.h>
#endif

// evil hack :)
NS_GRINGO::GrinGoParser* parser = 0;
NS_GRINGO::NS_OUTPUT::Output* output = 0;

bool convert = false;
void start_grounding()
{
	bool success = parser->parse(output);
	output->setStatistic(parser->getStatistic());
	if(success)
	{
		//parser->getStatistic()->print(&std::cout);
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

#ifdef WITH_CLASP
#	include "clasp_main.h"
#endif

using namespace NS_GRINGO;


int main(int argc, char *argv[])
{
	char *arg0 = argv[0];
	bool files = false;
#ifdef WITH_CLASP
	enum Format {SMODELS, GRINGO, CLASP, LPARSE} format = SMODELS;
#else
	enum Format {SMODELS, GRINGO, LPARSE} format = SMODELS;
#endif
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
#ifdef WITH_CLASP
			else if(strcmp(argv[1], "-c") == 0)
			{
				format = CLASP;
			}
#endif
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
#ifdef WITH_CLASP
				std::cerr << "Usage: " << arg0 << " (gringo option|file)* [--[ clasp options]] " << std::endl << std::endl;
#else
				std::cerr << "Usage: " << arg0 << " (gringo option|file)*" << std::endl << std::endl;
#endif
				std::cerr << "gringo options are: " << std::endl;
				std::cerr << "	--convert   : convert already ground program" << std::endl;
				std::cerr << "	--const c=v : Pass constant c equal value v to grounder" << std::endl;
#ifdef WITH_CLASP
				std::cerr << "	-c          : Use internal interface to clasp" << std::endl;
#endif
				std::cerr << "	-l          : Print smodels output" << std::endl;
				std::cerr << "	-p          : Print plain lparse-like output" << std::endl;
				std::cerr << "	-g          : Print experimental ASPils output" << std::endl;
				std::cerr << "	The default output is smodels output (-l)" << std::endl;
#ifdef WITH_CLASP
				int   argc_c = 2;
				char *argv_c[] = { arg0, argv[1] };
				std::cerr << std::endl << "clasp options are: " << std::endl;
				clasp_main(argc_c, argv_c);
#endif
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
				parser->getStatistic()->print(&std::cout);
				output = new NS_OUTPUT::PilsOutput(&std::cout);
				start_grounding();
				break;
#ifdef WITH_CLASP
			case CLASP:
				clasp_main(argc, argv);
				break;
#endif
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

