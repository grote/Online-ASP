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
#ifndef CLASP_OPTIONS_H_INCLUDED
#define CLASP_OPTIONS_H_INCLUDED

#ifdef _MSC_VER
#pragma warning (disable : 4200) // nonstandard extension used : zero-sized array
#pragma once
#endif

#include <string>
#include <utility>
#include <iosfwd>
#include "program_opts/program_options.h"
#include <clasp/include/solver.h>
#include <clasp/include/solve_algorithms.h>

const char* const VERSION = "1.1.0";

namespace Clasp {

class Options {
private:
	enum OutputFormat {SMODELS_OUT, GRINGO_OUT, CLASP_OUT, TEXT_OUT, ICLASP_OUT};
public:
	Options();
	void setDefaults();
	bool parse(int argc, char** argv, std::ostream& os, Solver& s);
	const std::string& getError() const {
		return error_;
	}
	const std::string getWarning() const {
		return warning_;
	}
	
	// common stuff
	bool             help;             // Default: false
	bool             version;          // Defailt: false
	bool             stats;            // Default: false
	std::vector<std::string> files;    // Default: "" -> read from stdin

	// gringo stuff
	bool             grounder;         // Default: true
	bool             verbose;          // Default: false
	int              imin;             // Default: 1
	int              imax;             // Default: std::numeric_limits<int>::max()
	int              ifixed;           // Default: -1
	bool             iunsat;           // Default: false
	bool             convert;          // Default: false
	bool             keepLearnts;      // Default: true
	bool             keepHeuristic;    // Default: false
	bool             bindersplitting;  // Default: true
	bool             ibase;            // Default: false
	std::vector<std::string> consts;


	bool             smodelsOut;
	int              aspilsOut;
	bool             claspOut;
	bool             iclaspOut;
	bool             textOut;
	OutputFormat     outf;             // Default: depends on build

	// clasp stuff
	SolveParams      solveParams;
	std::vector<int> optVals;          // Values for the optimization function
	std::vector<int> satPreParams;     // Params for the SatElite-preprocessor
	std::string      heuristic;        // Default: berkmin
	std::string      cons;             // Default: ""
	int              seed;             // Default: -1 -> use default seed
	int              transExt;         // Default: 0 -> do not transform extended rules
	int              eqIters;          // Default: -1 -> run eq-preprocessing to fixpoint
	int              numModels;        // Default: 1
	int              lookahead;        // Default: lookahead_no
	int              loopRep;          // Default: common
	int              optimize;         // Default: 0
	bool             quiet;            // Default: false
	bool             suppModels;       // Default: false
	bool             initialLookahead; // Default: false
	bool             ccmExp;           // Default: false
	bool             dimacs;           // Default: false
private:
	std::vector<double> delDefault() const {
		std::vector<double> v; v.push_back(3.0); v.push_back(1.1); v.push_back(3.0);
		return v;
	}
	std::vector<double> restartDefault() const {
		std::vector<double> v; v.push_back(100.0); v.push_back(1.5); v.push_back(0.0);
		return v;
	}
	void initOptions(ProgramOptions::OptionGroup& allOpts, ProgramOptions::OptionGroup& hidden);
	void checkCommonOptions(const ProgramOptions::OptionValues&);
	bool setSolverStrategies(Solver& s, const ProgramOptions::OptionValues&);
	bool setSolveParams(Solver& s, const ProgramOptions::OptionValues&);
	DecisionHeuristic* createHeuristic(const std::vector<int>&) const;
	void printHelp(const ProgramOptions::OptionGroup& opts, std::ostream& os) const;
	void printVersion(std::ostream& os) const;
	std::string error_;
	std::string warning_;
};
}
#endif
