// 
// Copyright (c) 2006-2007, Benjamin Kaufmann
// 
// This file is part of Clasp. See http://www.cs.uni-potsdam.de/clasp/ 
// 
// Clasp is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Clasp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Clasp; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
#include <clasp/include/lparse_reader.h>
#include <clasp/include/satelite.h>
#include <clasp/include/unfounded_check.h>
#include "options.h"
#include "enumerator.h"
#include "timer.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <numeric>
#include <signal.h>

#if defined (_MSC_VER) && _MSC_VER >= 1200
//#define CHECK_HEAP 1;
#include <crtdbg.h>
#endif

using namespace std;
using namespace Clasp;

struct ClaspApp {
	Options				options;
	Solver				solver;
	CTimer				t_pre;
	CTimer				t_solve;
	CTimer				t_all;
	enum Mode {
		ASP_MODE,
		SAT_MODE
	}							mode;
	int		run(int argc, char** argv);
private:
	enum State { start_read, start_pre, start_solve, end_read, end_pre, end_solve };
	void	setState(State s);
	void	printSatStats()						const;
	void	printLpStats()						const;
	void	printAspStats(bool more)	const;
	std::string shortName(const std::string& str) {
		if (str.size() < 40) return str;
		std::string r = "...";
		r.append(str.end()-38, str.end());
		return r;
	}
	static void printSatEliteProgress(uint32 i, SatElite::SatElite::Options::Action a) {
		static const char pro[] = {'/', '-', '\\', '|' };
		if			(a==SatElite::SatElite::Options::pre_start)		cout << '|';
		else if	(a==SatElite::SatElite::Options::iter_start)	cout << '\b' << pro[i%4];
		else if (a==SatElite::SatElite::Options::pre_done)		cout << '\b' << "Done";
		else if (a==SatElite::SatElite::Options::pre_stopped)	cout << '\b' << "Stop";
	}
	std::istream& getStream() {
		if (!options.file.empty()) {
			if (inFile_.is_open()) return inFile_;
			inFile_.open(options.file.c_str());
			if (!inFile_) {
				throw std::runtime_error("Can not read from '"+options.file+"'"); 
			}
			return inFile_;
		}
		return std::cin;
	}
	bool solve();
	bool parseLparse();
	bool readSat();
	std::auto_ptr<LparseStats>	lpStats_;
	std::auto_ptr<PreproStats>	preStats_;
	std::auto_ptr<Enumerator>		enum_;
	std::ifstream	inFile_;
} clasp_g;

// return values
const int S_UNKNOWN				=	0;
const int S_SATISFIABLE		= 10;
const int S_UNSATISFIABLE = 20;
const int S_ERROR					= EXIT_FAILURE;
const int S_MEMORY				= 107;

static void sigHandler(int) {
	// ignore further signals
	signal(SIGINT, SIG_IGN);    // Ctrl + C
	signal(SIGTERM, SIG_IGN);   // kill (but not kill -9)
	bool satM = clasp_g.mode == ClaspApp::SAT_MODE;
	printf("\n%s*** INTERRUPTED! ***\n", (satM?"c ":""));
	clasp_g.t_all.Stop();
	printf("%sTime      : %s\n", (satM?"c ":""), clasp_g.t_all.Print().c_str());	
	printf("%sModels    : %u\n", (satM?"c ":""), (uint32)clasp_g.solver.stats.models);	
	printf("%sChoices   : %u\n", (satM?"c ":""), (uint32)clasp_g.solver.stats.choices);	
	printf("%sConflicts : %u\n", (satM?"c ":""), (uint32)clasp_g.solver.stats.conflicts);	
	printf("%sRestarts  : %u\n", (satM?"c ":""), (uint32)clasp_g.solver.stats.restarts);
	exit(S_UNKNOWN);
}

void ClaspApp::setState(State s) {
	bool q = options.quiet;
	const int width = 13;
	switch (s) {
		case start_read:
			t_pre.Reset(); t_pre.Start();
			if (!q) cout << (mode == ASP_MODE ?"":"c ") << left << setw(width) << "Reading" << ": ";
			break;
		case end_read:
			t_pre.Stop();
			if (!q) cout << "Done(" << t_pre.Print() << "s)\n";
			break;
		case start_pre:
			t_pre.Reset(); t_pre.Start();
			if (!q) cout << (mode == ASP_MODE ?"":"c ") << left << setw(width) << "Preprocessing" << ": ";
			break;
		case end_pre:
			t_pre.Stop();
			if (!q) cout << (options.satPreParams[0]==0?"Done(":"(") << t_pre.Print() << "s)\n";
			break;
		case start_solve:
			t_solve.Start();
			cout << (mode == ASP_MODE ?"":"c ") << "Solving...\n";
			break;
		case end_solve:
			t_solve.Stop();
			break;
	default:;
	}
}

void ClaspApp::printSatStats() const {
	const SolverStatistics& st = solver.stats;
	cout << "c Models       : " << st.models << "\n";
	cout << "c Time         : " << t_all.Print() << "s (Solve: " << t_solve.Print() <<"s)\n";
	if (!options.stats) return;
	cout << "c Choices      : " << st.choices << "\n";
	cout << "c Conflicts    : " << st.conflicts <<"\n";
	cout << "c Restarts     : " << st.restarts << "\n";
	cout << "c Variables    : " << solver.numVars() << " (Eliminated: " << solver.numEliminatedVars() << ")\n";
	cout << "c Clauses      : " << st.native[0] << "\n";
	cout << "c   Binary     : " << st.native[1] << "\n";
	cout << "c   Ternary    : " << st.native[2] << "\n";
	cout << "c Lemmas       : " << st.learnt[0] << "\n";
	cout << "c   Binary     : " << st.learnt[1] << "\n";
	cout << "c   Ternary    : " << st.learnt[2] << "\n";
}

static double percent(uint32 y, uint32 from) {
	if (from == 0) return 0;
	return (static_cast<double>(y)/from)*100.0;
}
static double percent(const uint32* arr, uint32 idx) {
	return percent(arr[idx], arr[0]);
}
static double compAverage(uint64 x, uint64 y) {
	if (!x || !y) return 0.0;
	return static_cast<double>(x) / static_cast<double>(y);
}

void ClaspApp::printLpStats() const {
	const PreproStats& ps = *preStats_;
	const LparseStats& lp	= *lpStats_;
	const Options& o = options;
	uint32 r = lp.rules[0] == 0
		? std::accumulate(lp.rules, lp.rules+OPTIMIZERULE+1, 0)
		: lp.rules[0]+lp.rules[1]+lp.rules[OPTIMIZERULE];	
	uint32 a = lp.atoms[0] + lp.atoms[1];
	cout << left << setw(12) << "Atoms" << ": " << setw(6) << a;
	if (lp.atoms[1] != 0) {
		cout << " (Original: " << lp.atoms[0] << " Auxiliary: " << lp.atoms[1] << ")";
	}
	cout << "\n";
	cout << left << setw(12) << "Rules" << ": " << setw(6) << r << " (";
	for (int i = 1; i != OPTIMIZERULE; ++i) {
		if (lp.rules[i] != 0) {
			if (i != 1) cout << " ";
			cout << i << ": " << lp.rules[i];
			if (lp.rules[0] != 0) {
				cout << "/";
				if (i == 1) cout << lp.rules[1] + lp.rules[0];
				else if ((i == 3 && (o.transExt&LparseReader::transform_choice) !=0))	cout << 0;
				else if ((i == 2 || i == 5) &&(o.transExt&LparseReader::transform_weight)!=0)	cout << 0;
				else cout << lp.rules[i];
			}
		}
	}
	if (lp.rules[OPTIMIZERULE] != 0) {
		cout << " " << OPTIMIZERULE << ": " << lp.rules[OPTIMIZERULE];
	}
	cout << ")" << "\n";
	cout << left << setw(12) << "Bodies" << ": " << ps.bodies << "\n";
	if (ps.numEqs() != 0) {
		cout << left << setw(12) << "Equivalences" << ": " << setw(6) << ps.numEqs() << " (Atom=Atom: " << ps.numEqs(Var_t::atom_var)
				 << " Body=Body: "	 << ps.numEqs(Var_t::body_var)
				 << " Other: " << (ps.numEqs() - ps.numEqs(Var_t::body_var) - ps.numEqs(Var_t::atom_var))
				 << ")" << "\n";
	}	
	cout << left << setw(12) << "Tight" << ": ";
	if (!o.suppModels) {
		if (ps.sccs == 0) {
			cout << "Yes";
		}
		else {
			cout << setw(6) << "No"  << " (SCCs: " << ps.sccs << " Nodes: " 
					 << ps.ufsNodes << ")";
		}
	}
	else {
		cout << "N/A";
	}
	cout << "\n";	
}

void ClaspApp::printAspStats(bool more) const {
	cout.precision(1);
	cout.setf(ios_base::fixed, ios_base::floatfield);
	const SolverStatistics& st = solver.stats;
	cout << "\n";
	uint64 enumerated = st.models;
	uint64 models			= enum_.get() != 0 ? enum_->numModels(solver) : enumerated;
	cout	<< left << setw(12) << "Models" << ": ";
	if (more) {
		char buf[64];
		int wr		= sprintf(buf, "%llu", models);
		buf[wr]		= '+';
		buf[wr+1]	= 0;
		cout << setw(6) << buf;
	}
	else {
		cout << setw(6) << models;
	}
	if (enumerated != models && options.stats) {
		cout << " (Enumerated: " << enumerated << ")";
	}
	cout	<< "\n";
	cout	<< left << setw(12) << "Time"	 << ": " << setw(6) << t_all.Print()
				<< " (" << "Solving: " << t_solve.Print() << ")" << "\n";
	if (!options.stats) return;
	cout << left << setw(12) << "Choices"				<< ": " << st.choices		<< "\n";
	cout << left << setw(12) << "Conflicts"			<< ": " << st.conflicts << "\n";
	cout << left << setw(12) << "Restarts"			<< ": " << st.restarts	<< "\n";
	cout << "\n";
	printLpStats();
	cout << "\n";
	uint32 other = st.native[0] - st.native[1] - st.native[2];
	cout << left << setw(12) << "Variables" << ": " << setw(6) << solver.numVars() 
				<< " (Eliminated: " << right << setw(4) << solver.numEliminatedVars() << ")" << "\n";
	cout	<< left << setw(12) << "Constraints" << ": " << setw(6) << st.native[0]
				<< " (Binary: " << right << setw(4) << percent(st.native, 1) <<  "% "
				<< "Ternary: "	<< right << setw(4) << percent(st.native, 2) <<  "% "
				<< "Other: "		<< right << setw(4) << percent(other, st.native[0]) << "%)"
				<< "\n";
	other = st.learnt[0] - st.learnt[1] - st.learnt[2];
	cout	<< left << setw(12) << "Lemmas" << ": " << setw(6) << st.learnt[0]
				<< " (Binary: "	<< right << setw(4) << percent(st.learnt, 1) << "% "
				<< "Ternary: "	<< right << setw(4) << percent(st.learnt, 2) <<  "% "
				<< "Other: "		<< right << setw(4) << percent(other, st.learnt[0]) << "%)"
				<< "\n";
	cout << left << setw(12) << "  Conflicts"			<< ": " << setw(6) << st.learnt[0]-st.loops 
				<< " (Average Length: " << compAverage(st.lits[0], st.learnt[0]-st.loops) << ")\n";
	cout << left << setw(12) << "  Loops"	<< ": " << setw(6) << st.loops 
				<< " (Average Length: " << compAverage(st.lits[1], st.loops) << ")\n";
#if MAINTAIN_JUMP_STATS == 1
	cout << "\n";
	cout << "Backtracks          : " << st.conflicts-st.jumps << "\n";
	cout << "Backjumps           : " << st.jumps << " ( Bounded: " << st.bJumps << " )\n";
	cout << "Skippable Levels    : " << st.jumpSum << "\n"; 
	cout << "Levels skipped      : " << st.jumpSum - st.boundSum << " ("
			<< 100*((st.jumpSum - st.boundSum)/std::max(1.0, (double)st.jumpSum)) << "%)" << "\n";
	cout << "Max Jump Length     : " << st.maxJump << " ( Executed: " << st.maxJumpEx << " )\n";
	cout << "Max Bound Length    : " << st.maxBound << "\n";
	cout << "Average Jump Length : " << st.jumpSum/std::max(1.0,(double)st.jumps) << " ( Executed: " << (st.jumpSum-st.boundSum)/std::max(1.0,(double)st.jumps) << " )\n";
	cout << "Average Bound Length: " << (st.boundSum)/std::max(1.0,(double)st.bJumps) << "\n";
	cout << "Average Model Length: " << (st.modLits)/std::max(1.0, (double)st.models) << "\n";
#endif
	cout << endl;
}

int ClaspApp::run(int argc, char** argv) {
	if (!options.parse(argc, argv, std::cout, solver)) {
		throw std::runtime_error( options.getError() );
	}
	if (!options.getWarning().empty()) {
		cerr << options.getWarning() << endl;
	}
	if (options.help || options.version) {
		return EXIT_SUCCESS;
	}
	if (options.seed != -1) { Clasp::srand(options.seed); }
	mode = options.dimacs ? SAT_MODE : ASP_MODE;
	if (options.satPreParams[0] != 0) {
		// enable and configure the sat preprocessor
		SatElite::SatElite* pre = new SatElite::SatElite(solver);
		pre->options.maxIters = options.satPreParams[0];
		pre->options.maxOcc		= options.satPreParams[1];
		pre->options.maxTime	= options.satPreParams[2];
		pre->options.elimPure	= options.numModels == 1;
		pre->options.verbose	= options.quiet ? 0 : printSatEliteProgress;
		solver.strategies().satPrePro.reset(pre);
	}
	cout << (mode == ASP_MODE ?"":"c ") << "clasp version " << VERSION << " \n";
	getStream();
	cout << (mode == ASP_MODE ?"":"c ") << "Reading from " << (options.file.empty()?"stdin":shortName(options.file).c_str()) << "\n";
	t_all.Start();
	bool more = solve();
	t_all.Stop();
	if (enum_.get()) {
		enum_.get()->report(solver);
	}
	if (mode == ASP_MODE) {
		printAspStats(more);
	}
	else {
		cout << "s " << (solver.stats.models != 0 ? "SATISFIABLE" : "UNSATISFIABLE") << "\n";
		printSatStats();
	}
	return solver.stats.models != 0 ? S_SATISFIABLE : S_UNSATISFIABLE;
}

bool ClaspApp::solve() {
#ifdef WITH_ICLASP
	if(incremental)
	{
		lpStats_.reset(new LparseStats());
		preStats_.reset(new PreproStats());

		ProgramBuilder api;
		api.startProgram(options.suppModels
			? 0
			: new DefaultUnfoundedCheck(DefaultUnfoundedCheck::ReasonStrategy(options.loopRep))
			//, 0
			, (uint32)options.eqIters
		);
		output = new NS_GRINGO::NS_OUTPUT::IClaspOutput(&api, LparseReader::TransformMode(options.transExt));
		if(parser->parse(output))
			std::cerr << "Parsing successful" << std::endl;
		else
			throw NS_GRINGO::GrinGoException("Error: Parsing failed.");
		setState(start_solve);
		bool more = false;
		bool ret  = false;
		do
		{
			//solver.reduceLearnts(1.0);
			api.updateProgram();
			grounder->iground();
			ret = api.endProgram(solver, options.initialLookahead, true);
			if(ret)
			{
				StdOutPrinter printer;
				if (!options.quiet) {
					printer.index = &api.stats.index;
					options.solveParams.enumerator()->setPrinter(&printer);
				}
				LitVec assumptions;
				assumptions.push_back(api.stats.index[static_cast<NS_GRINGO::NS_OUTPUT::IClaspOutput*>(output)->getIncUid()].lit);
				more = Clasp::solve(solver, assumptions, options.numModels, options.solveParams);
				ret = solver.stats.models > 0;
			}
		}
		while(inum-- > 1 || !ret);
		setState(end_solve);

		*lpStats_ = static_cast<NS_GRINGO::NS_OUTPUT::ClaspOutput*>(output)->getStats();
		return more;
	}
#endif
	bool res = mode == ASP_MODE ? parseLparse() : readSat();
	if (res) {
		StdOutPrinter printer;
		if (!options.quiet) {
			if (mode == ASP_MODE) printer.index	= &preStats_->index;
			options.solveParams.enumerator()->setPrinter(&printer);
		}
		setState(start_solve);
		res = Clasp::solve(solver, options.numModels, options.solveParams);
		setState(end_solve);
	}
	return res;
}


bool ClaspApp::readSat() {
	std::istream& in = getStream();
	setState(start_read);
	bool res = parseDimacs(in, solver, options.numModels == 1);
	setState(end_read);
	if (res) {
		setState(start_pre);
		res = solver.endAddConstraints(options.initialLookahead);
		setState(end_pre);
	}
	return res;
}

bool ClaspApp::parseLparse() {
	std::istream& in = getStream();
	lpStats_.reset(new LparseStats());
	preStats_.reset(new PreproStats());
	//LparseReader reader;
	//reader.setTransformMode(LparseReader::TransformMode(options.transExt));
	ProgramBuilder api;
	api.startProgram(options.suppModels
		? 0
		: new DefaultUnfoundedCheck(DefaultUnfoundedCheck::ReasonStrategy(options.loopRep))
		, (uint32)options.eqIters
	);
	setState(start_read);
	bool ret = true;
	output = new NS_GRINGO::NS_OUTPUT::ClaspOutput(&api, LparseReader::TransformMode(options.transExt));
	start_grounding();
	//bool ret = reader.parse(in, api);
	setState(end_read);
	if (!ret) return ret;
	if (api.hasMinimize() || !options.cons.empty()) {
		if (api.hasMinimize() && !options.cons.empty()) {
			cerr << "Warning: Optimize statements are ignored because of '" << options.cons << "'!" << endl;
		}
		if (options.numModels != 0) {
			!options.cons.empty() 
				? cerr << "Warning: For computing consequences clasp must be called with '--number=0'!" << endl
				: cerr << "Warning: For computing optimal solutions clasp must be called with '--number=0'!" << endl;
		}
	}
	setState(start_pre);
	ret = api.endProgram(solver, options.initialLookahead, false);
	//*lpStats_ = reader.stats;
	*lpStats_ = static_cast<NS_GRINGO::NS_OUTPUT::ClaspOutput*>(output)->getStats();
	api.stats.moveTo(*preStats_);
	if (!options.cons.empty()) {
		enum_.reset(new CBConsequences(solver, &preStats_->index, options.cons == "brave" ? CBConsequences::brave_consequences : CBConsequences::cautious_consequences, options.quiet));
		options.solveParams.setEnumerator( *enum_ );
	}
	else if (api.hasMinimize()) {
		MinimizeConstraint* c = api.createMinimize(solver);
		c->setMode( MinimizeConstraint::Mode(options.optimize & 1), options.optimize == 2 );
		if (!options.optVals.empty()) {
			uint32 m = std::min((uint32)options.optVals.size(), c->numRules());
			for (uint32 x = 0; x != m; ++x) {
				c->setOptimum(x, options.optVals[x]);
			}
		}
		enum_.reset(new MinimizeEnumerator(c));
		options.solveParams.setEnumerator( *enum_ );
	}
	ret = ret && solver.endAddConstraints(options.initialLookahead);
	setState(end_pre);
	return ret;
}

int clasp_main(int argc, char** argv) {
#if defined (_MSC_VER) && defined (CHECK_HEAP) && _MSC_VER >= 1200 
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) |
				  _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF |
	_CRTDBG_CHECK_ALWAYS_DF);

	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
#endif
	try {
		signal(SIGINT, sigHandler);    // Ctrl + C
		signal(SIGTERM, sigHandler);   // kill (but not kill -9)
		return clasp_g.run(argc, argv);
	} 
	catch(const ReadError& e) {
		cerr << "Failed!\nError(" << e.line_ << "): " << e.what() << endl;
		return S_ERROR;
	}
	catch (const std::bad_alloc&) {
		cerr << "\nclasp ERROR: out of memory"  << endl;
		return S_MEMORY;
	}
	catch (const std::exception& e) {
		cerr << "\nclasp ERROR: " << e.what() << endl;
		return S_ERROR;
	}
	return S_UNKNOWN;
}
