/***************************************************************************
 *                                                                         *
 *    GrinGoLparseParser.h    This file is part of GrinGo                  *
 *                                                                         *
 *    Copyright (C) 2007 Sven Thiele, University of Potsdam, Germany       *
 *                                                                         *
 *    This program is free software; you can redistribute it and/or        *
 *    modify it under the terms of the GNU General Public License          *
 *    as published by the Free Software Foundation; either version 2       *
 *    of the License, or (at your option) any later version.               *
 *                                                                         *
 *    This program is distributed in the hope that it will be useful,      *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *    GNU General Public License for more details.                         *
 *                                                                         *
 *    You should have received a copy of the GNU General Public License    *
 *                                                                         * 
 ***************************************************************************/

#ifndef LPARSEPARSER_H
#define LPARSEPARSER_H

#include <gringo.h>
#include <gringoparser.h>

namespace NS_GRINGO
{
	class LparseParser : public GrinGoParser 
	{
	public:
		LparseParser(std::vector<std::istream*> &in);
		LparseParser(std::istream* = &std::cin);
		Grounder *parse();
		virtual ~LparseParser();
		GrinGoLexer *getLexer();
	private:
		LparseLexer *lexer_;
		std::vector<std::istream*> streams_;
		void *pParser;
	};
}
#endif

