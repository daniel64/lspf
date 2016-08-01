/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify x
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

using namespace std ;

class hilight
{
	public:
		string   hl_language ;
		bool     hl_abend    ;
		int      hl_oBrac1   ;
		int      hl_oBrac2   ;
		int      hl_oIf      ;
		int      hl_oDo      ;
		bool     hl_oComment ;
		char     hl_Quote    ;
		bool     hl_oQuote   ;
		bool     hl_continue ;
		bool     hl_ifLogic  ;
		bool     hl_doLogic  ;
		bool     hl_Paren    ;
		hilight()
		{
			hl_language = ""    ;
			hl_abend    = false ;
			hl_oBrac1   = 0     ;
			hl_oBrac2   = 0     ;
			hl_oComment = false ;
			hl_Quote    = ' '   ;
			hl_oQuote   = false ;
			hl_continue = false ;
			hl_ifLogic  = false ;
			hl_doLogic  = false ;
			hl_Paren    = false ;
		}
		void hl_clear()
		{
			hl_language = ""    ;
			hl_abend    = false ;
			hl_oBrac1   = 0     ;
			hl_oBrac2   = 0     ;
			hl_oComment = false ;
			hl_Quote    = ' '   ;
			hl_oQuote   = false ;
			hl_continue = false ;
			hl_ifLogic  = false ;
			hl_doLogic  = false ;
			hl_Paren    = false ;
		}
} ;

class keyw
{
	public:
	       int kw_len ;
	       int kw_col ;
} ;

void addASMHilight( hilight &, const string &, string & ) ;
void addCppHilight( hilight &, const string &, string & ) ;
void addRxxHilight( hilight &, const string &, string & ) ;
void addOthHilight( hilight &, const string &, string & ) ;
void addDefHilight( hilight &, const string &, string & ) ;

map<string, void(*)(hilight &, const string &, string &)> hiRoutine = { { "ASM",     addASMHilight },
									{ "CPP",     addCppHilight },
									{ "DEFAULT", addDefHilight },
									{ "OTHER",   addOthHilight },
									{ "PANEL",   addDefHilight },
									{ "REXX",    addRxxHilight } } ;
 map<string, keyw> keywList1 = {
  { "alignas",           { 7,  N_RED  } },
  { "alignof",           { 7,  N_RED  } },
  { "and",               { 3,  N_RED  } },
  { "and_eq",            { 6,  N_RED  } },
  { "asm",               { 3,  N_RED  } },
  { "atomic_cancel",     { 13, N_RED  } },
  { "atomic_commit",     { 13, N_RED  } },
  { "atomic_noexcept",   { 15, N_RED  } },
  { "auto",              { 4,  N_RED  } },
  { "bitand",            { 6,  N_RED  } },
  { "bitor",             { 5,  N_RED  } },
  { "bool",              { 4,  N_RED  } },
  { "break",             { 5,  N_RED  } },
  { "case",              { 4,  N_RED  } },
  { "catch",             { 5,  N_RED  } },
  { "char",              { 4,  N_RED  } },
  { "char16_t",          { 8,  N_RED  } },
  { "char32_t",          { 8,  N_RED  } },
  { "class",             { 5,  N_RED  } },
  { "compl",             { 5,  N_RED  } },
  { "concept",           { 7,  N_RED  } },
  { "const",             { 5,  N_RED  } },
  { "constexpr",         { 9,  N_RED  } },
  { "const_cast",        { 10, N_RED  } },
  { "continue",          { 8,  N_RED  } },
  { "decltype",          { 8,  N_RED  } },
  { "default",           { 7,  N_RED  } },
  { "delete",            { 6,  N_RED  } },
  { "do",                { 2,  N_RED  } },
  { "double",            { 6,  N_RED  } },
  { "dynamic_cast",      { 12, N_RED  } },
  { "else",              { 4,  N_RED  } },
  { "enum",              { 4,  N_RED  } },
  { "explicit",          { 8,  N_RED  } },
  { "export",            { 6,  N_RED  } },
  { "extern",            { 6,  N_RED  } },
  { "false",             { 5,  N_RED  } },
  { "float",             { 5,  N_RED  } },
  { "for",               { 3,  N_RED  } },
  { "friend",            { 6,  N_RED  } },
  { "goto",              { 4,  N_RED  } },
  { "if",                { 2,  N_RED  } },
  { "inline",            { 6,  N_RED  } },
  { "int",               { 3,  N_RED  } },
  { "import",            { 6,  N_RED  } },
  { "long",              { 4,  N_RED  } },
  { "module",            { 6,  N_RED  } },
  { "mutable",           { 7,  N_RED  } },
  { "namespace",         { 9,  N_RED  } },
  { "new",               { 3,  N_RED  } },
  { "noexcept",          { 8,  N_RED  } },
  { "not",               { 3,  N_RED  } },
  { "not_eq",            { 6,  N_RED  } },
  { "nullptr",           { 7,  N_RED  } },
  { "operator",          { 8,  N_RED  } },
  { "or",                { 2,  N_RED  } },
  { "or_eq",             { 5,  N_RED  } },
  { "private",           { 7,  N_RED  } },
  { "protected",         { 9,  N_RED  } },
  { "public",            { 6,  N_RED  } },
  { "register",          { 8,  N_RED  } },
  { "reinterpret_cast",  { 16, N_RED  } },
  { "requires",          { 8,  N_RED  } },
  { "return",            { 6,  N_RED  } },
  { "short",             { 5,  N_RED  } },
  { "signed",            { 6,  N_RED  } },
  { "sizeof",            { 6,  N_RED  } },
  { "static",            { 6,  N_RED  } },
  { "static_assert",     { 13, N_RED  } },
  { "static_cast",       { 11, N_RED  } },
  { "struct",            { 6,  N_RED  } },
  { "switch",            { 6,  N_RED  } },
  { "synchronized",      { 12, N_RED  } },
  { "template",          { 8,  N_RED  } },
  { "this",              { 4,  N_RED  } },
  { "thread_local",      { 12, N_RED  } },
  { "throw",             { 5,  N_RED  } },
  { "true",              { 4,  N_RED  } },
  { "try",               { 3,  N_RED  } },
  { "typedef",           { 7,  N_RED  } },
  { "typeid",            { 6,  N_RED  } },
  { "typename",          { 8,  N_RED  } },
  { "union",             { 5,  N_RED  } },
  { "unsigned",          { 8,  N_RED  } },
  { "using",             { 5,  N_RED  } },
  { "virtual",           { 7,  N_RED  } },
  { "void",              { 4,  N_RED  } },
  { "volatile",          { 8,  N_RED  } },
  { "wchar_t",           { 7,  N_RED  } },
  { "while",             { 5,  N_RED  } },
  { "xor",               { 3,  N_RED  } },
  { "xor_eq",            { 6,  N_RED  } } } ;


 map<string, keyw> keywList2 = {
  { "ADDRESS",           { 7,  N_RED  } },
  { "ARG",               { 3,  N_RED  } },
  { "CALL",              { 4,  N_RED  } },
  { "DO",                { 2,  N_RED  } },
  { "DROP",              { 4,  N_RED  } },
  { "ELSE",              { 4,  N_RED  } },
  { "END",               { 3,  N_RED  } },
  { "EXIT",              { 4,  N_RED  } },
  { "EXPOSE",            { 6,  N_RED  } },
  { "FORWARD",           { 7,  N_RED  } },
  { "GUARD",             { 5,  N_RED  } },
  { "IF",                { 2,  N_RED  } },
  { "INTERPRET",         { 9,  N_RED  } },
  { "ITERATE",           { 7,  N_RED  } },
  { "LEAVE",             { 5,  N_RED  } },
  { "LOOP",              { 4,  N_RED  } },
  { "NOP",               { 3,  N_RED  } },
  { "NUMERIC",           { 7,  N_RED  } },
  { "OPTIONS",           { 7,  N_RED  } },
  { "PARSE",             { 5,  N_RED  } },
  { "PROCEDURE",         { 9,  N_RED  } },
  { "PULL",              { 4,  N_RED  } },
  { "PUSH",              { 4,  N_RED  } },
  { "QUEUE",             { 5,  N_RED  } },
  { "RAISE",             { 5,  N_RED  } },
  { "REPLY",             { 5,  N_RED  } },
  { "RETURN",            { 6,  N_RED  } },
  { "SAY",               { 3,  N_RED  } },
  { "SELECT",            { 6,  N_RED  } },
  { "SIGNAL",            { 6,  N_RED  } },
  { "TRACE",             { 5,  N_RED  } },
  { "USE",               { 3,  N_RED  } },
  { "ABBREV",            { 6,  N_WHITE  } },
  { "COPIES",            { 6,  N_WHITE  } },
  { "FORMAT",            { 6,  N_WHITE  } },
  { "POS",               { 3,  N_WHITE  } },
  { "ABS",               { 3,  N_WHITE  } },
  { "COUNTSTR",          { 8,  N_WHITE  } },
  { "FUZZ",              { 4,  N_WHITE  } },
  { "QUALIFY",           { 7,  N_WHITE  } },
  { "TRANSLATE",         { 9,  N_WHITE  } },
  { "C2D",               { 3,  N_WHITE  } },
  { "INDEX",             { 5,  N_WHITE  } },
  { "QUEUED",            { 6,  N_WHITE  } },
  { "TRUNC",             { 5,  N_WHITE  } },
  { "C2X",               { 3,  N_WHITE  } },
  { "INSERT",            { 6,  N_WHITE  } },
  { "RANDOM",            { 6,  N_WHITE  } },
  { "USERID",            { 6,  N_WHITE  } },
  { "BITAND",            { 6,  N_WHITE  } },
  { "DATATYPE",          { 8,  N_WHITE  } },
  { "JUSTIFY",           { 7,  N_WHITE  } },
  { "REVERSE",           { 7,  N_WHITE  } },
  { "VALUE",             { 5,  N_WHITE  } },
  { "BITOR",             { 5,  N_WHITE  } },
  { "DATE",              { 4,  N_WHITE  } },
  { "LASTPOS",           { 7,  N_WHITE  } },
  { "RIGHT",             { 5,  N_WHITE  } },
  { "VERIFY",            { 6,  N_WHITE  } },
  { "BITXOR",            { 6,  N_WHITE  } },
  { "DELSTR",            { 6,  N_WHITE  } },
  { "LEFT",              { 4,  N_WHITE  } },
  { "SIGN",              { 4,  N_WHITE  } },
  { "WORD",              { 4,  N_WHITE  } },
  { "B2X",               { 3,  N_WHITE  } },
  { "DELWORD",           { 7,  N_WHITE  } },
  { "LENGTH",            { 6,  N_WHITE  } },
  { "SOURCELINE",        { 10, N_WHITE  } },
  { "WORDINDEX",         { 9,  N_WHITE  } },
  { "CENTER",            { 6,  N_WHITE  } },
  { "DIGITS",            { 6,  N_WHITE  } },
  { "LINEIN",            { 6,  N_WHITE  } },
  { "SPACE",             { 5,  N_WHITE  } },
  { "WORDLENGTH",        { 10, N_WHITE  } },
  { "CHANGESTR",         { 9,  N_WHITE  } },
  { "D2C",               { 3,  N_WHITE  } },
  { "LINEOUT",           { 7,  N_WHITE  } },
  { "STREAM",            { 6,  N_WHITE  } },
  { "WORDPOS",           { 7,  N_WHITE  } },
  { "CHARIN",            { 6,  N_WHITE  } },
  { "D2X",               { 3,  N_WHITE  } },
  { "LINES",             { 5,  N_WHITE  } },
  { "STRIP",             { 5,  N_WHITE  } },
  { "WORDS",             { 5,  N_WHITE  } },
  { "CHAROUT",           { 7,  N_WHITE  } },
  { "ERRORTEXT",         { 9,  N_WHITE  } },
  { "LINESIZE",          { 8,  N_WHITE  } },
  { "SUBSTR",            { 6,  N_WHITE  } },
  { "XRANGE",            { 6,  N_WHITE  } },
  { "CHARS",             { 5,  N_WHITE  } },
  { "EXTERNALS",         { 9,  N_WHITE  } },
  { "MAX",               { 3,  N_WHITE  } },
  { "SUBWORD",           { 7,  N_WHITE  } },
  { "X2B",               { 3,  N_WHITE  } },
  { "COMPARE",           { 7,  N_WHITE  } },
  { "FIND",              { 4,  N_WHITE  } },
  { "MIN",               { 3,  N_WHITE  } },
  { "SYMBOL",            { 6,  N_WHITE  } },
  { "X2C",               { 3,  N_WHITE  } },
  { "CONDITION",         { 9,  N_WHITE  } },
  { "FORM",              { 4,  N_WHITE  } },
  { "OVERLAY",           { 7,  N_WHITE  } },
  { "TIME",              { 4,  N_WHITE  } },
  { "X2D",               { 3,  N_WHITE  } } } ;
