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

const char *GRINGO_VERSION = "2.0.0";

// evil hack :)
NS_GRINGO::GrinGoParser* parser = 0;
NS_GRINGO::NS_OUTPUT::Output* output = 0;
NS_GRINGO::Grounder *grounder = 0;


int  imin        = 1;
int  imax        = std::numeric_limits<int>::max();
bool iunsat      = false;
bool convert     = false;
bool incremental = false;

bool keepLearnts   = true;
bool keepHeuristic = false;

void start_grounding()
{
	bool success = parser->parse(output);
	if(success)
	{
		std::cerr << "Parsing successful" << std::endl;
		if(!convert)
		{
			grounder->start();
		}
	}
	else
	{
		throw NS_GRINGO::GrinGoException("Error: Parsing failed.");
	}
}

#ifdef WITH_CLASP
#	include "clasp_main.h"
#endif

using namespace NS_GRINGO;

int readNum(int &argc, char **&argv, const char *msg)
{
	if(argc > 2)
	{
		char *endptr;
		int num = strtol(argv[2], &endptr, 10);
		if(endptr != argv[2] && !*endptr)
		{
			argc--;
			argv++;
			return num;
		}
	}
	throw GrinGoException(msg);
}

int main(int argc, char *argv[])
{
	Grounder::Options opts;
	char *arg0   = argv[0];
	bool files   = false;

#ifdef WITH_ICLASP
	enum Format {SMODELS, GRINGO, CLASP, LPARSE, ICLASP} format = ICLASP;
#elif defined WITH_CLASP
	enum Format {SMODELS, GRINGO, CLASP, LPARSE} format = CLASP;
#else
	enum Format {SMODELS, GRINGO, LPARSE} format = SMODELS;
#endif
	std::stringstream *ss = new std::stringstream();
	std::vector<std::istream*> streams;
	streams.push_back(ss);
	unsigned int normalForm = 7;
	try
	{
		while(argc > 1)
		{
			if(strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
			{
				std::cout << "GrinGo " << GRINGO_VERSION << std::endl;
				std::cout << "Copyright (C) Roland Kaminski" << std::endl;
				std::cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << std::endl;
				std::cout << "GrinGo is free software: you are free to change and redistribute it." << std::endl;
				std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
#ifdef WITH_CLASP
				std::cout << "clasp " << VERSION << std::endl;
				std::cout << "Copyright (C) Benjamin Kaufmann" << std::endl;
				std::cout << "License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>" << std::endl;
				std::cout << "clasp is free software: you are free to change and redistribute it." << std::endl;
				std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
#endif

				for(std::vector<std::istream*>::iterator it = streams.begin(); it != streams.end(); it++)
					delete *it;
				return 0;
			}
			else if(strcmp(argv[1], "--ground") == 0 || strcmp(argv[1], "-G") == 0)
			{
				convert = true;
			}
			else if(strcmp(argv[1], "-a") == 0)
			{
				format = GRINGO;
				if(argc > 2)
				{
					std::istringstream isst;
					isst.str( argv[2] );
					char t;
					if(isst >> normalForm && !isst.get(t))
						if (normalForm >= 1 && normalForm <= 7)
						{
							argc--;
							argv++;
						}
				}
			}
			else if(strcmp(argv[1], "-l") == 0)
			{
				format = SMODELS;
			}
			else if(strcmp(argv[1], "--bindersplitting=off") == 0)
			{
				opts.binderSplit = false;
			}
			else if(strcmp(argv[1], "--bindersplitting=on") == 0)
			{
				opts.binderSplit = true;
			}
			else if(strcmp(argv[1], "-t") == 0)
			{
				format = LPARSE;
			}
			else if(strcmp(argv[1], "--verbose") == 0 || strcmp(argv[1], "-V") == 0)
			{
				opts.verbose = true;
			}
#ifdef WITH_ICLASP
			else if(strcmp(argv[1], "--iheuristic=forget") == 0)
			{
				keepHeuristic = false;
			}
			else if(strcmp(argv[1], "--iheuristic=keep") == 0)
			{
				keepHeuristic = true;
			}
			else if(strcmp(argv[1], "--inogoods=forget") == 0)
			{
				keepLearnts = false;
			}
			else if(strcmp(argv[1], "--inogoods=keep") == 0)
			{
				keepLearnts = true;
			}
			else if(strcmp(argv[1], "--imin") == 0)
			{
				imin = readNum(argc, argv, "error: number expected after option --imin");
			}
			else if(strcmp(argv[1], "--imax") == 0)
			{
				imax = readNum(argc, argv, "error: number expected after option --imax");
			}
			else if(strcmp(argv[1], "--ifixed") == 0)
			{
				opts.ifixed = readNum(argc, argv, "error: number expected after option --ifixed");
			}
			else if(strcmp(argv[1], "--iunsat") == 0)
			{
				iunsat = true;
			}
			else if(strcmp(argv[1], "--clasp") == 0 || strcmp(argv[1], "-C") == 0)
			{
				format = CLASP;
			}
#endif
			else if(strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--constant") == 0)
			{
				if(argc == 2)
					throw GrinGoException("error: constant missing");
				argc--;
				argv++;
				*ss << "#const " << argv[1] << "." << std::endl;
			}
			else if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
			{
#ifdef WITH_CLASP
				std::cerr << "Usage: " << arg0 << " (gringo option|file)* [--[ clasp options]] " << std::endl << std::endl;
#else
				std::cerr << "Usage: " << arg0 << " (gringo option|file)*" << std::endl << std::endl;
#endif
				std::cerr << "gringo options are: " << std::endl;
				std::cerr << "  -h, --help     : Print this help message" << std::endl;
				std::cerr << "  -v, --version  : Print version information" << std::endl;
				std::cerr << "  -G, --ground   : Convert already ground program" << std::endl;
				std::cerr << "  -c, " << std::endl;
				std::cerr << "  --constant c=v : Pass constant c equal value v to grounder" << std::endl;
				std::cerr << "  -l             : Print lparse output format" << std::endl;
				std::cerr << "  -t             : Print plain text format" << std::endl;
				std::cerr << "  -a [1-7]       : Print experimental ASPils output" << std::endl;
			        std::cerr << "                   Give an optional normalform number from 1 to 7 (7 if none)" << std::endl;
			        std::cerr << "  --bindersplitting=on|off " << std::endl;
				std::cerr << "                 :  Enable or disable bindersplitting" << std::endl;
#ifdef WITH_ICLASP
				std::cerr << "  -C, --clasp    : Use non incremental interface to clasp" << std::endl << std::endl;

				std::cerr << "Incremental grounding" << std::endl;
				std::cerr << "  --imax <num>   : Maximum number of incremental steps" << std::endl;
				std::cerr << "  --imin <num>   : Minimum number of incremental steps" << std::endl;
				std::cerr << "  --iunsat       : Stop after first unsatisfiable solution" << std::endl;
				std::cerr << "  --inogoods=keep|forget" << std::endl;
				std::cerr << "                 : Keep learnt nogoods in next step (default: keep)" << std::endl;
				std::cerr << "  --iheuristic=keep|forget" << std::endl;
				std::cerr << "                 : Keep heuristic information in next step (default: forget)" << std::endl;
#else
				std::cerr << std::endl << "Incremental grounding" << std::endl;
#endif
				std::cerr << "  --ifixed <num> : Fixed number of incremental steps" << std::endl << std::endl;
#ifdef WITH_ICLASP
				std::cerr << "The default output is the incremantal clasp interface" << std::endl;
#elif defined WITH_CLASP
				std::cerr << "The default output is the internal clasp interface" << std::endl;
#else
				std::cerr << "The default output is lparse output (-l)" << std::endl;
#endif

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
#ifdef WITH_ICLASP
		if(format == ICLASP)
		{
			incremental  = true;
			opts.ifixed = -1;
		}
#endif
		if(!files)
			streams.push_back(new std::istream(std::cin.rdbuf()));
		argv[0] = arg0;
		if(convert && incremental)
			throw GrinGoException("Error: cannot use both --ground and incremental grounding.");

		if(convert)
			parser = new LparseConverter(streams);
		else
		{
			grounder = new Grounder(opts);
			parser   = new LparseParser(grounder, streams);
		}
		
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
				output = new NS_OUTPUT::PilsOutput(&std::cout, normalForm);
				start_grounding();
				break;
#ifdef WITH_ICLASP
			case ICLASP:
				clasp_main(argc, argv);
				break;
#endif
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
	if(grounder)
		delete grounder;
	if(output)
		delete output;
	if(parser)
		delete parser;
	for(std::vector<std::istream*>::iterator it = streams.begin(); it != streams.end(); it++)
		delete *it;

	return 0;
}

