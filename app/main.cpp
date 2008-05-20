#include <iostream>
#include <fstream>
#include <lparseparser.h>
#include <grounder.h>
#include <smodelsoutput.h>
#include <lparseoutput.h>
#include <claspoutput.h>
#include <clasp/include/solver.h>
#include <clasp/include/solve_algorithms.h>
#include <clasp/include/unfounded_check.h>

using namespace NS_GRINGO;

struct StdOutPrinter : Clasp::ModelPrinter 
{
        StdOutPrinter() : index(0) 
	{
	}
        const Clasp::AtomIndex* index;
        void printModel(Clasp::Solver& s) {
                if (index) 
		{
                        std::cout << "Answer: " << s.stats.models << "\n";
                        for (Clasp::AtomIndex::size_type i = 0; i != index->size(); ++i) {
                                if (s.value((*index)[i].lit.var()) == trueValue((*index)[i].lit) && !(*index)[i].name.empty()) {
                                        std::cout << (*index)[i].name << " ";
                                }
                        }
                        std::cout << "\n";
                        std::cout << std::flush;
                }
        }
};

int main(int argc, char *argv[])
{
	bool smodels = false;
	bool lparse  = false;
	bool clasp   = false;
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
	else if(argc > 1 && strcmp(argv[1], "-c") == 0)
	{
		argc--;
		argv++;
		clasp = true;
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
			Clasp::SolveParams p;
			Clasp::Solver s;
			Clasp::DefaultUnfoundedCheck *ufs = new Clasp::DefaultUnfoundedCheck();
			Clasp::ProgramBuilder b;
			b.startProgram(ufs);
			/*
			b.startProgram(ufs)
				.setAtomName(1, "a").setAtomName(2, "b")
				.startRule(Clasp::CHOICERULE).addHead(2).endRule()
				.startRule(Clasp::CHOICERULE).addHead(1).addToBody(2, false).endRule()
				.startRule().addHead(1).addToBody(2, false).endRule();
			*/
			NS_OUTPUT::ClaspOutput o(&b);
			grounder->start(o);
			b.endProgram(s);
			delete grounder;

			StdOutPrinter pr;
			pr.index = &b.stats.index;
                        p.setModelPrinter(pr);
			Clasp::solve(s, 1, p);
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

	for(std::vector<std::istream*>::iterator it = streams.begin(); it != streams.end(); it++)
		delete *it;

	return 0;
}

