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

#define E_RED      0x05
#define E_GREEN    0x06
#define E_YELLOW   0x07
#define E_BLUE     0x08
#define E_MAGENTA  0x09
#define E_TURQ     0x0A
#define E_WHITE    0x0B

#define G_RED      0x0E
#define G_WHITE    0x0F

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
			hl_clear() ;
		}
		void hl_clear()
		{
			hl_language = ""    ;
			hl_abend    = false ;
			hl_oBrac1   = 0     ;
			hl_oBrac2   = 0     ;
			hl_oIf      = 0     ;
			hl_oDo      = 0     ;
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

void addASMHilight( hilight&, const string&, string& ) ;
void addCppHilight( hilight&, const string&, string& ) ;
void addRxxHilight( hilight&, const string&, string& ) ;
void addOthHilight( hilight&, const string&, string& ) ;
void addPanHilight( hilight&, const string&, string& ) ;
void addDefHilight( hilight&, const string&, string& ) ;

map<string, void(*)(hilight&, const string&, string&)> hiRoutine = { { "ASM",     addASMHilight },
								     { "CPP",     addCppHilight },
								     { "DEFAULT", addDefHilight },
								     { "OTHER",   addOthHilight },
								     { "PANEL",   addPanHilight },
								     { "REXX",    addRxxHilight } } ;
map<string, keyw> keywList1 = {
  { "alignas",           { 7,  E_RED  } },
  { "alignof",           { 7,  E_RED  } },
  { "and",               { 3,  E_RED  } },
  { "and_eq",            { 6,  E_RED  } },
  { "asm",               { 3,  E_RED  } },
  { "atomic_cancel",     { 13, E_RED  } },
  { "atomic_commit",     { 13, E_RED  } },
  { "atomic_noexcept",   { 15, E_RED  } },
  { "auto",              { 4,  E_RED  } },
  { "bitand",            { 6,  E_RED  } },
  { "bitor",             { 5,  E_RED  } },
  { "bool",              { 4,  E_RED  } },
  { "break",             { 5,  E_RED  } },
  { "case",              { 4,  E_RED  } },
  { "catch",             { 5,  E_RED  } },
  { "char",              { 4,  E_RED  } },
  { "char16_t",          { 8,  E_RED  } },
  { "char32_t",          { 8,  E_RED  } },
  { "class",             { 5,  E_RED  } },
  { "compl",             { 5,  E_RED  } },
  { "concept",           { 7,  E_RED  } },
  { "const",             { 5,  E_RED  } },
  { "constexpr",         { 9,  E_RED  } },
  { "const_cast",        { 10, E_RED  } },
  { "continue",          { 8,  E_RED  } },
  { "decltype",          { 8,  E_RED  } },
  { "default",           { 7,  E_RED  } },
  { "delete",            { 6,  E_RED  } },
  { "do",                { 2,  E_RED  } },
  { "double",            { 6,  E_RED  } },
  { "dynamic_cast",      { 12, E_RED  } },
  { "else",              { 4,  E_RED  } },
  { "enum",              { 4,  E_RED  } },
  { "explicit",          { 8,  E_RED  } },
  { "export",            { 6,  E_RED  } },
  { "extern",            { 6,  E_RED  } },
  { "false",             { 5,  E_RED  } },
  { "float",             { 5,  E_RED  } },
  { "for",               { 3,  E_RED  } },
  { "friend",            { 6,  E_RED  } },
  { "goto",              { 4,  E_RED  } },
  { "if",                { 2,  E_RED  } },
  { "inline",            { 6,  E_RED  } },
  { "int",               { 3,  E_RED  } },
  { "import",            { 6,  E_RED  } },
  { "long",              { 4,  E_RED  } },
  { "module",            { 6,  E_RED  } },
  { "mutable",           { 7,  E_RED  } },
  { "namespace",         { 9,  E_RED  } },
  { "new",               { 3,  E_RED  } },
  { "noexcept",          { 8,  E_RED  } },
  { "not",               { 3,  E_RED  } },
  { "not_eq",            { 6,  E_RED  } },
  { "nullptr",           { 7,  E_RED  } },
  { "operator",          { 8,  E_RED  } },
  { "or",                { 2,  E_RED  } },
  { "or_eq",             { 5,  E_RED  } },
  { "private",           { 7,  E_RED  } },
  { "protected",         { 9,  E_RED  } },
  { "public",            { 6,  E_RED  } },
  { "register",          { 8,  E_RED  } },
  { "reinterpret_cast",  { 16, E_RED  } },
  { "requires",          { 8,  E_RED  } },
  { "return",            { 6,  E_RED  } },
  { "short",             { 5,  E_RED  } },
  { "signed",            { 6,  E_RED  } },
  { "sizeof",            { 6,  E_RED  } },
  { "static",            { 6,  E_RED  } },
  { "static_assert",     { 13, E_RED  } },
  { "static_cast",       { 11, E_RED  } },
  { "struct",            { 6,  E_RED  } },
  { "switch",            { 6,  E_RED  } },
  { "synchronized",      { 12, E_RED  } },
  { "template",          { 8,  E_RED  } },
  { "this",              { 4,  E_RED  } },
  { "thread_local",      { 12, E_RED  } },
  { "throw",             { 5,  E_RED  } },
  { "true",              { 4,  E_RED  } },
  { "try",               { 3,  E_RED  } },
  { "typedef",           { 7,  E_RED  } },
  { "typeid",            { 6,  E_RED  } },
  { "typename",          { 8,  E_RED  } },
  { "union",             { 5,  E_RED  } },
  { "unsigned",          { 8,  E_RED  } },
  { "using",             { 5,  E_RED  } },
  { "virtual",           { 7,  E_RED  } },
  { "void",              { 4,  E_RED  } },
  { "volatile",          { 8,  E_RED  } },
  { "wchar_t",           { 7,  E_RED  } },
  { "while",             { 5,  E_RED  } },
  { "xor",               { 3,  E_RED  } },
  { "xor_eq",            { 6,  E_RED  } } } ;


map<string, keyw> keywList2 = {
  { "ADDRESS",           { 7,  E_RED  } },
  { "ARG",               { 3,  E_RED  } },
  { "CALL",              { 4,  E_RED  } },
  { "DO",                { 2,  E_RED  } },
  { "DROP",              { 4,  E_RED  } },
  { "ELSE",              { 4,  E_RED  } },
  { "END",               { 3,  E_RED  } },
  { "EXIT",              { 4,  E_RED  } },
  { "EXPOSE",            { 6,  E_RED  } },
  { "FORWARD",           { 7,  E_RED  } },
  { "GUARD",             { 5,  E_RED  } },
  { "IF",                { 2,  E_RED  } },
  { "INTERPRET",         { 9,  E_RED  } },
  { "ITERATE",           { 7,  E_RED  } },
  { "LEAVE",             { 5,  E_RED  } },
  { "LOOP",              { 4,  E_RED  } },
  { "NOP",               { 3,  E_RED  } },
  { "NUMERIC",           { 7,  E_RED  } },
  { "OPTIONS",           { 7,  E_RED  } },
  { "PARSE",             { 5,  E_RED  } },
  { "PROCEDURE",         { 9,  E_RED  } },
  { "PULL",              { 4,  E_RED  } },
  { "PUSH",              { 4,  E_RED  } },
  { "QUEUE",             { 5,  E_RED  } },
  { "RAISE",             { 5,  E_RED  } },
  { "REPLY",             { 5,  E_RED  } },
  { "RETURN",            { 6,  E_RED  } },
  { "SAY",               { 3,  E_RED  } },
  { "SELECT",            { 6,  E_RED  } },
  { "SIGNAL",            { 6,  E_RED  } },
  { "TRACE",             { 5,  E_RED  } },
  { "USE",               { 3,  E_RED  } },
  { "ABBREV",            { 6,  E_WHITE  } },
  { "COPIES",            { 6,  E_WHITE  } },
  { "FORMAT",            { 6,  E_WHITE  } },
  { "POS",               { 3,  E_WHITE  } },
  { "ABS",               { 3,  E_WHITE  } },
  { "COUNTSTR",          { 8,  E_WHITE  } },
  { "FUZZ",              { 4,  E_WHITE  } },
  { "QUALIFY",           { 7,  E_WHITE  } },
  { "TRANSLATE",         { 9,  E_WHITE  } },
  { "C2D",               { 3,  E_WHITE  } },
  { "INDEX",             { 5,  E_WHITE  } },
  { "QUEUED",            { 6,  E_WHITE  } },
  { "TRUNC",             { 5,  E_WHITE  } },
  { "C2X",               { 3,  E_WHITE  } },
  { "INSERT",            { 6,  E_WHITE  } },
  { "RANDOM",            { 6,  E_WHITE  } },
  { "USERID",            { 6,  E_WHITE  } },
  { "BITAND",            { 6,  E_WHITE  } },
  { "DATATYPE",          { 8,  E_WHITE  } },
  { "JUSTIFY",           { 7,  E_WHITE  } },
  { "REVERSE",           { 7,  E_WHITE  } },
  { "VALUE",             { 5,  E_WHITE  } },
  { "BITOR",             { 5,  E_WHITE  } },
  { "DATE",              { 4,  E_WHITE  } },
  { "LASTPOS",           { 7,  E_WHITE  } },
  { "RIGHT",             { 5,  E_WHITE  } },
  { "VERIFY",            { 6,  E_WHITE  } },
  { "BITXOR",            { 6,  E_WHITE  } },
  { "DELSTR",            { 6,  E_WHITE  } },
  { "LEFT",              { 4,  E_WHITE  } },
  { "SIGN",              { 4,  E_WHITE  } },
  { "WORD",              { 4,  E_WHITE  } },
  { "B2X",               { 3,  E_WHITE  } },
  { "DELWORD",           { 7,  E_WHITE  } },
  { "LENGTH",            { 6,  E_WHITE  } },
  { "SOURCELINE",        { 10, E_WHITE  } },
  { "WORDINDEX",         { 9,  E_WHITE  } },
  { "CENTER",            { 6,  E_WHITE  } },
  { "DIGITS",            { 6,  E_WHITE  } },
  { "LINEIN",            { 6,  E_WHITE  } },
  { "SPACE",             { 5,  E_WHITE  } },
  { "WORDLENGTH",        { 10, E_WHITE  } },
  { "CHANGESTR",         { 9,  E_WHITE  } },
  { "D2C",               { 3,  E_WHITE  } },
  { "LINEOUT",           { 7,  E_WHITE  } },
  { "STREAM",            { 6,  E_WHITE  } },
  { "WORDPOS",           { 7,  E_WHITE  } },
  { "CHARIN",            { 6,  E_WHITE  } },
  { "D2X",               { 3,  E_WHITE  } },
  { "LINES",             { 5,  E_WHITE  } },
  { "STRIP",             { 5,  E_WHITE  } },
  { "WORDS",             { 5,  E_WHITE  } },
  { "CHAROUT",           { 7,  E_WHITE  } },
  { "ERRORTEXT",         { 9,  E_WHITE  } },
  { "LINESIZE",          { 8,  E_WHITE  } },
  { "SUBSTR",            { 6,  E_WHITE  } },
  { "XRANGE",            { 6,  E_WHITE  } },
  { "CHARS",             { 5,  E_WHITE  } },
  { "EXTERNALS",         { 9,  E_WHITE  } },
  { "MAX",               { 3,  E_WHITE  } },
  { "SUBWORD",           { 7,  E_WHITE  } },
  { "X2B",               { 3,  E_WHITE  } },
  { "COMPARE",           { 7,  E_WHITE  } },
  { "FIND",              { 4,  E_WHITE  } },
  { "MIN",               { 3,  E_WHITE  } },
  { "SYMBOL",            { 6,  E_WHITE  } },
  { "X2C",               { 3,  E_WHITE  } },
  { "CONDITION",         { 9,  E_WHITE  } },
  { "FORM",              { 4,  E_WHITE  } },
  { "OVERLAY",           { 7,  E_WHITE  } },
  { "TIME",              { 4,  E_WHITE  } },
  { "X2D",               { 3,  E_WHITE  } } } ;


map<string, keyw> keywList3 = {
  { ")PANEL",          { 6,  E_WHITE } },
  { ")ABC",            { 4,  E_WHITE } },
  { ")ABCINIT",        { 8,  E_WHITE } },
  { ")ABCPROC",        { 8,  E_WHITE } },
  { ")ATTR",           { 5,  E_WHITE } },
  { ")INCLUDE",        { 8,  E_WHITE } },
  { ")BODY",           { 5,  E_WHITE } },
  { ")INIT",           { 5,  E_WHITE } },
  { ")REINIT",         { 7,  E_WHITE } },
  { ")PROC",           { 5,  E_WHITE } },
  { ")FIELD",          { 6,  E_WHITE } },
  { ")HELP",           { 5,  E_WHITE } },
  { ")PNTS",           { 5,  E_WHITE } },
  { ")END",            { 4,  E_WHITE } },
  { ")COMMENT",        { 8,  E_TURQ  } },
  { ")ENDCOMMENT",     { 11, E_TURQ  } },
  { "PDC",             { 3,  E_RED   } },
  { "ACTION",          { 6,  E_RED   } },
  { "IF",              { 2,  E_RED   } },
  { "ELSE",            { 4,  E_RED   } },
  { "VGET",            { 4,  E_RED   } },
  { "VPUT",            { 4,  E_RED   } },
  { "VER",             { 3,  E_RED   } },
  { "TRANS",           { 5,  E_RED   } },
  { "TRUNC",           { 5,  E_RED   } } } ;
