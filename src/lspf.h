/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify
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

#include <string>
#include <stack>
#include <fstream>
#include <panel.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition.hpp>

#define DEBUG1 1
// #define DEBUG2 1
#define MOD_NAME lspf

typedef unsigned int uint ;

using namespace std;
using namespace boost::posix_time;

// Customisable values below.  Also included in the setup.cpp program for the ISPS profile pool.  Recompile setup.cpp and run after changes
// HOME     - Home directory of the user
// ZSPROF   - Where the user's ISPS profile is found.  Needs to be fully qualified.  All other variables are contained in this member
// ZUPROF   - Where the user-specific files are found (profiles, tables, panels etc)
// ZSYSPATH - Where the system-specific files are found.
// ZLDPATH  - Where the application classes are located (concatenation allowed)
// SLOG     - Location of the system log file (fully qualified)
// ALOG     - Location of the application log file (fully qualified)
// ZREXPATH - Location of the rexx execs (conatenation allowed)
// ZMAINPGM - Name of the initial program to invoke
// ZPANLPGM - Name of the program invoked on the SELECT PANEL service
// ZEDITPGM - Name of the editor program to invoke
// ZBRPGM   - Name of the browser program to invoke
// ZVIEWPGM - Name of the viewer program to invoke
// ZFLSTPGM - Name of the file list program to invoke
// ZHELPPGM - Name of the tutorial/help program to invoke
// ZOREXPGM - Name of the oorexx interpreter call program to invoke
// ZSHELP   - Name of the system help member (ZPLIB/help concatenatin searched for this)
// ZMAXSCRN - Maximum number of split screens allowed (Greater than 8 and the screen will not be displayed in the Screen[] status area.)
// ZWAIT    - Wait time to check if the application has gone into a wait-for-user-response (normally a few ms)
// ZMAXWAIT - Max wait time to terminate the application if it has not gone into a wait-for-user-response (application may be looping)

#define HOME            "/home/daniel"
#define ZSPROF          HOME "/.lspf"
#define ZUPROF          HOME "/.lspf"
#define ZSYSPATH        HOME "/lspf"
#define ZLDPATH         ZSYSPATH "/Apps:" ZSYSPATH "/Apps2"
#define MLIB            ZUPROF "/mlib:" ZSYSPATH "/mlib"
#define PLIB            ZUPROF "/plib:" ZSYSPATH "/plib"
#define TLIB            ZUPROF "/tlib:" ZSYSPATH "/tlib"
#define ZREXPATH        HOME "/rexx:" ZSYSPATH "/rexx"
#define SLOG            HOME "/.lspf/lspflog"
#define ALOG            HOME "/.lspf/appllog"
#define ZRFLTBL         "LSRPLIST"
#define ZMAINPGM        "PMAIN0A"
#define ZPANLPGM        "PDPANLA"
#define ZEDITPGM        "PEDIT01"
#define ZBRPGM          "PBRO01A"
#define ZVIEWPGM        "PVIEW0A"
#define ZFLSTPGM        "PFLST0A"
#define ZHELPPGM        "PTUTORA"
#define ZOREXPGM        "POREXX1"
#define ZSHELP          "HPSPF01"
#define ZRFLPGM         "PLRFLST1"
#define ZMAXSCRN        8
#define ZWAIT           5
#define ZMAXWAIT        1000
// ***************************************** End custom values **************************************

// ***************************************** CUA defaults *******************************************
#define KAB     "YLN"
#define KABSL   "BLN"
#define KABU    "WHN"
#define KAMT    "RHN"
#define KAWF    "BHN"
#define KCT     "YHN"
#define KCEF    "TLU"
#define KCH     "BHN"
#define KDT     "GLN"
#define KET     "THN"
#define KEE     "YHR"
#define KFP     "GLN"
#define KFK     "BLN"
#define KIMT    "WHN"
#define KLEF    "TLU"
#define KLID    "GLN"
#define KLI     "WLN"
#define KNEF    "TLU"
#define KNT     "GLN"
#define KPI     "BLN"
#define KPIN    "GLN"
#define KPT     "BLN"
#define KPS     "THN"
#define KPAC    "WLN"
#define KPUC    "BLN"
#define KRP     "WHN"
#define KSI     "WHN"
#define KSAC    "WLN"
#define KSUC    "BLN"
#define KVOI    "TLN"
#define KWMT    "YHN"
#define KWT     "RHN"
#define KWASL   "BLN"

// ***************************************** End defaults *******************************************

#define BLACK           COLOR_PAIR(0)
#define RED             COLOR_PAIR(1)
#define GREEN           COLOR_PAIR(2)
#define YELLOW          COLOR_PAIR(3)
#define BLUE            COLOR_PAIR(4)
#define MAGENTA         COLOR_PAIR(5)
#define TURQ            COLOR_PAIR(6)
#define WHITE           COLOR_PAIR(7)

#define N_BLACK         0
#define N_RED           1
#define N_GREEN         2
#define N_YELLOW        3
#define N_BLUE          4
#define N_MAGENTA       5
#define N_TURQ          6
#define N_WHITE         7
#define B_RED           8
#define B_GREEN         9
#define B_YELLOW        10
#define B_BLUE          11
#define B_MAGENTA       12
#define B_TURQ          13
#define B_WHITE         14
#define R_RED           15
#define R_GREEN         16
#define R_YELLOW        17
#define R_BLUE          18
#define R_MAGENTA       19
#define R_TURQ          20
#define R_WHITE         21
#define U_RED           22
#define U_GREEN         23
#define U_YELLOW        24
#define U_BLUE          25
#define U_MAGENTA       26
#define U_TURQ          27
#define U_WHITE         28
#define P_RED           29
#define P_GREEN         30
#define P_YELLOW        31
#define P_BLUE          32
#define P_MAGENTA       33
#define P_TURQ          34
#define P_WHITE         35
#define P_FF            255

struct lspfCommand
{
	string Command       ;
	int    RC            ;
	vector<string> reply ;
} ;

enum msgSET
{
	COND,
	UNCOND
} ;

enum s_paths
{
	s_ZMLIB,
	s_ZPLIB,
	s_ZTLIB
} ;

enum tbWRITE
{
	WRITE,
	NOWRITE
} ;

enum tbREP
{
	REPLACE,
	NOREPLACE
} ;

enum tbDISP
{
	SHARE,
	EXCLUSIVE
} ;

enum dataType
{
	INTEGER,
	STRING,
	ERROR
} ;

enum nameCHCK
{
	CHECK,
	NOCHECK
} ;

enum vdType
{
	DEFINED,
	IMPLICIT,
	ALL
} ;

enum readCHCK
{
	ROCHECK,
	NOROCHECK
} ;

enum vcMODE
{
	LOCATE,
	MOVE
} ;

enum poolType
{
	SHARED,
	PROFILE,
	BOTH,
	ASIS
} ;

enum srCOND
{
	s_EQ,
	s_NE,
	s_LE,
	s_LT,
	s_GE,
	s_GT
} ;

enum vTYPE
{
	SYSTEM,
	USER
} ;

enum cuaType
{
	AB,
	ABSL,
	ABU,
	AMT,
	AWF,
	CT,
	CEF,
	CH,
	DT,
	ET,
	EE,
	FP,
	FK,
	IMT,
	LEF,
	LID,
	LI,
	NEF,
	NT,
	PI,
	PIN,
	PT,
	PS,
	PAC,
	PUC,
	RP,
	SI,
	SAC,
	SUC,
	VOI,
	WMT,
	WT,
	WASL,
	CHAR,
	DATAIN,
	DATAOUT,
	GRPBOX,
	OUTPUT,
	TEXT
} ;


class errblock
{
	public:
		string  msgid  ;
		string  msg1   ;
		string  sline  ;
		const string* dline ;
		string  val1   ;
		string  val2   ;
		string  val3   ;
		string  udata  ;
		int     taskid ;
		int     RC     ;
		int     maxRC  ;
		bool    debug  ;
		bool    abend  ;
	errblock()
	{
		msgid  = "" ;
		msg1   = "" ;
		sline  = "" ;
		dline  = NULL ;
		val1   = "" ;
		val2   = "" ;
		val3   = "" ;
		udata  = "" ;
		taskid = 0  ;
		RC     = 0  ;
		maxRC  = 0  ;
		debug  = false ;
		abend  = false ;
	}
	void clear()
	{
		msgid  = "" ;
		msg1   = "" ;
		sline  = "" ;
		dline  = NULL ;
		val1   = "" ;
		val2   = "" ;
		val3   = "" ;
		udata  = "" ;
		RC     = 0  ;
		maxRC  = 0  ;
		debug  = false ;
		abend  = false ;
	}
	void setRC( int i )
	{
		RC = i ;
	}
	int getRC()
	{
		return RC ;
	}
	bool RC0()
	{
		return ( RC == 0 ) ;
	}
	bool RC4()
	{
		return ( RC == 4 ) ;
	}
	bool RC8()
	{
		return ( RC == 8 ) ;
	}
	bool RC12()
	{
		return ( RC == 12 ) ;
	}
	void setmaxRC( int i )
	{
		maxRC = i ;
	}
	void setmaxRC()
	{
		maxRC = max( RC, maxRC ) ;
	}
	int getmaxRC()
	{
		return maxRC ;
	}
	void setDebugMode()
	{
		debug = true ;
	}
	bool debugMode()
	{
		return debug ;
	}
	void setAbending()
	{
		abend = true ;
	}
	bool abending()
	{
		return abend ;
	}
	bool error()
	{
		return RC > 8 ;
	}
	void setcall( const string& s )
	{
		msg1 = s ;
	}
	void setcall( const string& s1, const string& s2, int i=20 )
	{
		msg1  = s1 ;
		msgid = s2 ;
		val1  = "" ;
		val2  = "" ;
		val3  = "" ;
		RC    = i  ;
	}
	void setcall( const string& s1, const string& s2, const string& s3, int i=20 )
	{
		msg1  = s1 ;
		msgid = s2 ;
		val1  = s3 ;
		val2  = "" ;
		val3  = "" ;
		RC    = i  ;
	}
	void setcall( const string& s1, const string& s2, const string& s3, const string& s4, int i=20 )
	{
		msg1  = s1 ;
		msgid = s2 ;
		val1  = s3 ;
		val2  = s4 ;
		val3  = "" ;
		RC    = i  ;
	}
	void setcall( const string& s1, const string& s2, const string& s3, const string& s4, const string& s5, int i=20 )
	{
		msg1  = s1 ;
		msgid = s2 ;
		val1  = s3 ;
		val2  = s4 ;
		val3  = s5 ;
		RC    = i  ;
	}
	void seterror()
	{
		RC    = 20 ;
	}
	void seterror( const string& s1, int i=20 )
	{
		msgid = "PSYE019D" ;
		val1  = s1 ;
		val2  = "" ;
		RC    = i  ;
	}
	void seterror( const string& s1, const string& s2, int i=20 )
	{
		msgid = "PSYE019D" ;
		val1  = s1 ;
		val2  = s2 ;
		RC    = i  ;
	}
	void seterrid( const string& s, int i=20 )
	{
		msgid = s  ;
		val1  = "" ;
		val2  = "" ;
		val3  = "" ;
		RC    = i  ;
	}
	void seterrid( const string& s1, const string& s2, int i=20 )
	{
		msgid = s1 ;
		val1  = s2 ;
		val2  = "" ;
		val3  = "" ;
		RC    = i  ;
	}
	void seterrid( const string& s1, const string& s2, const string& s3, int i=20 )
	{
		msgid = s1 ;
		val1  = s2 ;
		val2  = s3 ;
		val3  = "" ;
		RC    = i  ;
	}
	void seterrid( const string& s1, const string& s2, const string& s3, const string& s4, int i=20 )
	{
		msgid = s1 ;
		val1  = s2 ;
		val2  = s3 ;
		val3  = s4 ;
		RC    = i  ;
	}
	void setsrc( const string& s )
	{
		sline = s ;
		dline = NULL ;
	}
	void setsrc( const string * p )
	{
		dline = p  ;
		sline = "" ;
	}
	string getsrc()
	{
		if ( dline ) { return *dline ; }
		else         { return  sline ; }
	}
	void clearsrc()
	{
		dline = NULL ;
		sline = ""   ;
	}
	void setval( const string& s1 )
	{
		val1 = s1 ;
	}
	void setval( const string& s1, const string& s2 )
	{
		val1 = s1 ;
		val2 = s2 ;
	}
	void setval( const string& s1, const string& s2, const string& s3 )
	{
		val1 = s1 ;
		val2 = s2 ;
		val3 = s3 ;
	}
	void setUserData( const string& s )
	{
		udata = s ;
	}
	string& getUserData()
	{
		return udata ;
	}
} ;


#define _quotes( a ) #a
#define quotes( a ) _quotes( a )

#define llog(t, s) LOGOUT << microsec_clock::local_time() << \
		" " << left( quotes(MOD_NAME), 10 ) << " " << t << " " << s

#ifdef DEBUG1
#define debug1( s ) LOGOUT << microsec_clock::local_time() << " " << left( quotes(MOD_NAME), 10 ) << \
		" D line: "  << __LINE__  << " >>L1 Function: " << __FUNCTION__ << " -  " << s
#else
#define debug1( s )
#endif

#ifdef DEBUG2
#define debug2( s ) LOGOUT << microsec_clock::local_time() << " " << left( quotes(MOD_NAME), 10 ) << \
		" D line: "  << __LINE__  << " >>L2 Function: " << __FUNCTION__ << " -  " << s
#else
#define debug2( s )
#endif
