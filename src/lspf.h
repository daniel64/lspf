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

#define NDEBUG
// #define DEBUG1 1
// #define DEBUG2 1

#include <string>
#include <stack>
#include <fstream>
#include <dlfcn.h>
#include <panel.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem.hpp>
#include <set>
#include <assert.h>
#include <signal.h>

#define LSPF_VERSION "1.2.0"
#define LSPF_VERSION_MAJ 1
#define LSPF_VERSION_REV 2
#define LSPF_VERSION_MOD 0

#define noop

using uint = unsigned int ;

using namespace std ;
using namespace boost::posix_time ;

//
// ***************************************** ******************* **************************************
// ***************************************** Start custom values **************************************
// ***************************************** ******************* **************************************
//
// Customisable values below.  Also included in the setup.cpp program for the ISPS profile pool.
// Recompile setup.cpp and run after changes.
//
// ~ character represents the user's home directory and will be resolved at setup run time.
//
// ZUPROF   - User subdirectory where the lspf user-specific profiles/tables are found (ISPS, profiles, clipboard).
//            ZUPROF will be prefixed with the user's home directory at runtime (ZUPROF in ISPSPROF contains the prefix).
// ZSYSPATH - Where the system-specific files are found.
// ZLDPATH  - Where the application classes are located (concatenation allowed).
// SLOG     - Location of the user's system log file.  Can use ~ character.
// ALOG     - Location of the user's application log file. Can use ~ character.
// MLIB     - Search path for messages. Can use ~ character.
// PLIB     - Search path for panels. Can use ~ character.
// SLIB     - Search path for file tailoring skeletons. Can use ~ character.
// LLIB     - Search path for program load.  Can use ~ character.
// TLIB     - Search path for tables. Can use ~ character.
// TABL     - Table ouput path. Can use the ~ character (concatination allowed).
// FTFILE   - File tailoring output.
// MUSR     - User search path for messages. Can use ~ character (concatination allowed).
// PUSR     - User search path for panels. Can use ~ character (concatination allowed).
// SUSR     - User search path for file tailoring skeletons. Can use ~ character (concatination allowed).
// LUSR     - User search path for program load.  Can use ~ character.
// TUSR     - User search path for tables. Can use ~ character (concatination allowed).
// TABU     - User table ouput path. Can use the ~ character (concatination allowed).
// FTFILU   - User file tailoring output.
// ZREXPATH - Location of the rexx execs (conatenation allowed).  Can use ~ character.
// ZSPOOL   - Name of the directory to store command and job output.  Can use ~ character.
// FLDHIST  - Field history table directory.  Can use the ~ character.
// ZMAINPGM - Name of the initial program to invoke.  This is treated as a SELECT PANEL().
// ZMAINPAN - Name of the initial selection panel to invoke ( ie. SELECT PANEL(ZMAINPAN) ).
// ZPANLPGM - Name of the program invoked on the SELECT PANEL service.
// ZEDITPGM - Name of the editor program to invoke.
// ZBRPGM   - Name of the browser program to invoke.
// ZVIEWPGM - Name of the viewer program to invoke (usually same as the default editor program).
// ZFLSTPGM - Name of the file list program to invoke.
// ZHELPPGM - Name of the tutorial/help program to invoke.
// ZOREXPGM - Name of the ooRexx interpreter call program to invoke.
// ZSHELPGM - Name of the shell interpreter program to invoke.
// ZFHSTPGM - Field history program.
// DATEF    - Date format (DD/MM/YY, DD.MM.YY, YY/MM/DD or YY.MM.DD)
// ZMAXSCRN - Maximum number of split screens allowed (Greater than 8 and the screen will not be displayed in the Screen[] status area).
// EDREC_SZ - Size of the edit recovery table.
// MXTAB_SZ - Maximum number of rows allowed in an lspf table (must be less than 16,777,216).
// EDLCTAB  - Default edit line command table for when one is not specified.
// EDSWMAC  - Edit site-wide macro.
// EDMAXPRF - Maximum number of Edit profiles kept.  If exceeded, deleted on a least-used basis.
// HTOP     - Default tutorial for new profiles.
// REXX_SUP - Add support for the panel *REXX statement, PANEXIT and )REXX file tailoring statement.  1 compiles in support, 0 leaves it out.
//            Adding REXX support also enables native ISPF panel support via REXX panconv called internally.
// SWIND    - Year window for adding century to 2-digit year.  yy <= SWIND, 21st century, yy > SWIND 20th century.
//

#define ZUPROF          "/.lspf"
#define ZSYSPATH        "/home/daniel/lspf"
#define ZLDPATH         ZSYSPATH "/Apps:" ZSYSPATH "/Apps2"
#define MLIB            "~" ZUPROF "/mlib:" ZSYSPATH "/mlib"
#define PLIB            "~" ZUPROF "/plib:" ZSYSPATH "/plib:~" ZUPROF "/help/plib:" ZSYSPATH "/help/plib"
#define SLIB            "~" ZUPROF "/slib:" ZSYSPATH "/slib"
#define LLIB            ZLDPATH
#define TLIB            "~" ZUPROF ":~" ZUPROF "/tlib:" ZSYSPATH "/tlib"
#define TABL            "~" ZUPROF "/tlib"
#define FTFILE          ""
#define MUSR            ""
#define PUSR            ""
#define SUSR            ""
#define LUSR            ""
#define TUSR            ""
#define TABU            ""
#define FTFILU          ""
#define ZREXPATH        "~/rexx:" ZSYSPATH "/rexx"
#define ZSPOOL          "~" ZUPROF "/spool"
#define FLDHIST         "~" ZUPROF "/fldhist"
#define SLOG            "~/.lspf/lspflog"
#define ALOG            "~/.lspf/appllog"
#define ZRFLTBL         "LSRPLIST"
#define ZMAINPGM        "PMAIN0A"
#define ZMAINPAN        "PMAINP01"
#define ZPANLPGM        "PDPANLA"
#define ZEDITPGM        "PEDIT01"
#define ZBRPGM          "PBRO01A"
#define ZVIEWPGM        "PEDIT01"
#define ZFLSTPGM        "PFLST0A"
#define ZHELPPGM        "PTUTORA"
#define ZOREXPGM        "POREXX1"
#define ZSHELPGM        "PSHELL0"
#define ZRFLPGM         "PLRFLST1"
#define ZFHSTPGM        "PLFHIST1"
#define DATEF           "DD/MM/YY"
#define ZMAXSCRN        8
#define EDREC_SZ        8
#define MXTAB_SZ        500000
#define EDLCTAB         ""
#define EDSWMAC         ""
#define EDMAXPRF        35
#define HTOP            "LSPH0001"
#define REXX_SUP        1
#define SWIND           65
// ***************************************** ***************** **************************************
// ***************************************** End custom values **************************************
// ***************************************** ***************** **************************************


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
#define KIWF    "BLN"
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

#if REXX_SUP == 1
#define HAS_REXX_SUPPORT 1
#include <oorexxapi.h>
#endif

namespace lspf {

struct lspfCommand
{
	string Command ;
	int    RC ;
	vector<string> reply ;
} ;

enum msgSET
{
	COND,
	UNCOND
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
	NON_SHARE
} ;

enum enqDISP
{
	SHR,
	EXC
} ;

enum enqSCOPE
{
	LOCAL,
	GLOBAL
} ;

enum WAIT_REASON
{
	WAIT_NONE,
	WAIT_OUTPUT,
	WAIT_SELECT,
	WAIT_USER
} ;

enum dataType
{
	INTEGER,
	STRING,
	ERROR
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

enum vTYPE
{
	SYSTEM,
	USER
} ;

enum attType
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
	INPUT,
	IWF,
	LEF,
	LID,
	LI,
	NEF,
	NONE,
	NT,
	PI,
	PIN,
	PT,
	PS,
	PAC,
	PUC,
	RP,
	SC,
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
	OUTPUT,
	TEXT
} ;

enum USR_ACTIONS
{
	USR_END,
	USR_ENTER,
	USR_EXIT,
	USR_CANCEL,
	USR_RETURN
} ;

}


enum VEDIT_TYPE
{
	VED_NONE,
	VED_IDATE,
	VED_STDDATE,
	VED_ITIME,
	VED_STDTIME,
	VED_JDATE,
	VED_JSTD,
	VED_USER
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
		string  user   ;
		string  mod    ;
		int     csrpos ;
		uint    taskid ;
		uint    ptid   ;
		uint    RC     ;
		uint    RSN    ;
		uint    maxRC  ;
		uint    debug  ;
		bool    abend  ;
		bool    sCall  ;
		bool    panel  ;
		bool    ft     ;
		bool    dialog ;

	errblock()
	{
		msgid  = "" ;
		msg1   = "" ;
		sline  = "" ;
		dline  = nullptr ;
		val1   = "" ;
		val2   = "" ;
		val3   = "" ;
		udata  = "" ;
		user   = "" ;
		mod    = "" ;
		csrpos = 1  ;
		taskid = 0  ;
		ptid   = 0  ;
		RC     = 0  ;
		RSN    = 0  ;
		maxRC  = 0  ;
		debug  = 0  ;
		abend  = false ;
		sCall  = false ;
		panel  = false ;
		ft     = false ;
		dialog = false ;
	}

	errblock( int i ) : errblock()
	{
		taskid = i ;
	}

	void clear()
	{
		msgid  = "" ;
		msg1   = "" ;
		sline  = "" ;
		dline  = nullptr ;
		val1   = "" ;
		val2   = "" ;
		val3   = "" ;
		udata  = "" ;
		mod    = "" ;
		RC     = 0  ;
		RSN    = 0  ;
		maxRC  = 0  ;
		csrpos = 1  ;
		debug  = 0  ;
		panel  = false ;
		ft     = false ;
		dialog = false ;
	}

	void setRC( uint i )
	{
		RC = i ;
	}

	void setRSN( uint i )
	{
		RSN = i ;
	}

	uint getRC()
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

	bool RSN0()
	{
		return ( RSN == 0 ) ;
	}

	bool RSN4()
	{
		return ( RSN == 4 ) ;
	}

	void setmaxRC( uint i )
	{
		maxRC = i ;
	}

	void setmaxRC()
	{
		maxRC = max( RC, maxRC ) ;
	}

	uint getmaxRC()
	{
		return maxRC ;
	}

	void setRCmax()
	{
		RC = maxRC ;
	}

	void setDebugMode()
	{
		debug = 1 ;
	}

	void setDebugMode( uint lvl )
	{
		debug = lvl ;
	}

	void clearDebugMode()
	{
		debug = 0 ;
	}

	bool debugMode()
	{
		return ( debug > 0 ) ;
	}

	uint debugLevel()
	{
		return debug ;
	}

	bool debugLevel1()
	{
		return ( debug >= 1 ) ;
	}

	bool debugLevel2()
	{
		return ( debug >= 2 ) ;
	}

	bool debugLevel3()
	{
		return ( debug >= 3 ) ;
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
		return RC > 11 ;
	}

	bool ok()
	{
		return RC < 12 ;
	}

	void seterror()
	{
		RC    = 20 ;
		msgid = "" ;
	}

	void setcall( const string& s )
	{
		msg1 = s ;
	}

	void setabend( const string& s1,
		       const string& s2 = "",
		       const string& s3 = "",
		       const string& s4 = "" )
	{
		msg1  = s1 ;
		msgid = s1 ;
		val1  = s2 ;
		val2  = s3 ;
		val3  = s4 ;
		RC    = 20 ;
		mod   = "" ;
	}

	void setcall( const string& m,
		      const string& s1,
		      const string& s2,
		      uint i=20 )
	{
		mod   = m  ;
		msg1  = s1 ;
		msgid = s2 ;
		val1  = "" ;
		val2  = "" ;
		val3  = "" ;
		RC    = i  ;
	}

	void setcall( const string& m,
		      const string& s1,
		      const string& s2,
		      const string& s3,
		      uint i=20 )
	{
		mod   = m  ;
		msg1  = s1 ;
		msgid = s2 ;
		val1  = s3 ;
		val2  = "" ;
		val3  = "" ;
		RC    = i  ;
	}

	void setcall( const string& m,
		      const string& s1,
		      const string& s2,
		      const string& s3,
		      const string& s4,
		      uint i=20 )
	{
		mod   = m  ;
		msg1  = s1 ;
		msgid = s2 ;
		val1  = s3 ;
		val2  = s4 ;
		val3  = "" ;
		RC    = i  ;
	}

	void setcall( const string& m,
		      const string& s1,
		      const string& s2,
		      const string& s3,
		      const string& s4,
		      const string& s5,
		      uint i=20 )
	{
		mod   = m  ;
		msg1  = s1 ;
		msgid = s2 ;
		val1  = s3 ;
		val2  = s4 ;
		val3  = s5 ;
		RC    = i  ;
	}

	void seterrid( const string& m,
		       const string& s1,
		       uint i=20 )
	{
		mod   = m  ;
		msgid = s1 ;
		val1  = "" ;
		val2  = "" ;
		val3  = "" ;
		RC    = i  ;
		if ( panel || ft ) { copysrc() ; }
	}

	void seterrid( const string& m,
		       const string& s1,
		       const string& s2,
		       uint i=20 )
	{
		mod   = m  ;
		msgid = s1 ;
		val1  = s2 ;
		val2  = "" ;
		val3  = "" ;
		RC    = i  ;
		if ( panel || ft ) { copysrc() ; }
	}

	void seterrid( const string& m,
		       const string& s1,
		       const string& s2,
		       const string& s3,
		       uint i=20 )
	{
		mod   = m  ;
		msgid = s1 ;
		val1  = s2 ;
		val2  = s3 ;
		val3  = "" ;
		RC    = i  ;
		if ( panel || ft ) { copysrc() ; }
	}

	void seterrid( const string& m,
		       const string& s1,
		       const string& s2,
		       const string& s3,
		       const string& s4,
		       uint i=20 )
	{
		mod   = m  ;
		msgid = s1 ;
		val1  = s2 ;
		val2  = s3 ;
		val3  = s4 ;
		RC    = i  ;
		if ( panel || ft ) { copysrc() ; }
	}

	void setval( const string& s1 )
	{
		if ( val1 == "" )
		{
			val1 = s1 ;
		}
		else if ( val2 == "" )
		{
			val2 = s1 ;
		}
		else if ( val3 == "" )
		{
			val3 = s1 ;
		}
	}

	void setval( const string& s1,
		     const string& s2 )
	{
		if ( val1 == "" )
		{
			val1 = s1 ;
			val2 = s2 ;
		}
		else if ( val2 == "" )
		{
			val2 = s1 ;
			val3 = s2 ;
		}
		else if ( val3 == "" )
		{
			val3 = s1 ;
		}
	}

	void setval( const string& s1,
		     const string& s2,
		     const string& s3 )
	{
		if ( val1 == "" )
		{
			val1 = s1 ;
			val2 = s2 ;
			val3 = s3 ;
		}
		else if ( val2 == "" )
		{
			val2 = s1 ;
			val3 = s2 ;
		}
		else if ( val3 == "" )
		{
			val3 = s1 ;
		}
	}

	void setsrc( const string& s )
	{
		sline = s ;
		dline = nullptr ;
	}

	void setsrc( const string* p )
	{
		dline = p  ;
		sline = "" ;
	}

	void copysrc()
	{
		if ( dline )
		{
			sline = *dline ;
			sline.erase( 0, sline.find_first_not_of( ' ' ) ) ;
			dline = nullptr ;
		}
	}

	string getsrc()
	{
		return ( dline ) ? *dline : sline ;
	}

	void setpanelsrc()
	{
		ft     = false ;
		panel  = true  ;
		dialog = false ;
	}

	bool panelsrc()
	{
		return panel ;
	}

	void setftsrc()
	{
		ft     = true  ;
		panel  = false ;
		dialog = false ;
	}

	void setftsrc( const string& s )
	{
		setftsrc() ;
		sline = s ;
	}

	bool ftsrc()
	{
		return ft ;
	}

	void setdialogsrc()
	{
		ft     = false ;
		panel  = false ;
		dialog = true  ;
	}

	bool dialogsrc()
	{
		return dialog ;
	}

	void clearsrc()
	{
		ft     = false ;
		panel  = false ;
		dialog = false ;
		dline  = nullptr ;
		sline  = ""   ;
	}

	void setUserData( const string& s )
	{
		udata = s ;
	}

	const string& getUserData()
	{
		return udata ;
	}

	const string& geterrid()
	{
		return msgid ;
	}

	void setServiceCall()
	{
		sCall = true ;
	}

	bool ServiceCall()
	{
		return sCall ;
	}

	void setcsrpos( int p )
	{
		csrpos = p ;
	}

	int getcsrpos()
	{
		return csrpos ;
	}

	void settask( uint i )
	{
		taskid = i ;
	}

	void setmod( const string& m )
	{
		mod = m ;
	}
} ;


#define _quotes( a ) #a
#define quotes( a ) _quotes( a )

#define TASKID() 0

#define llog(t, s)                                         \
{                                                          \
lg->lock() ;                                               \
(*lg) << microsec_clock::local_time() <<                   \
" " << left( modname(), 10 ) <<                            \
" " << d2ds( TASKID(), 5 ) << " " << t << " " << s ;       \
lg->unlock() ;                                             \
}

#ifdef DEBUG1
#define debug1( s )                                        \
{                                                          \
lg->lock() ;                                               \
(*lg) << microsec_clock::local_time() <<                   \
" " << left( modname(), 10 ) <<                            \
" " << d2ds( TASKID(), 5 ) <<                              \
" D line: "  << __LINE__  <<                               \
" >>L1 Function: " << __FUNCTION__ <<                      \
" -  " << s ;                                              \
lg->unlock() ;                                             \
}
#else
#define debug1( s )
#endif


#ifdef DEBUG2
#define debug2( s )                                        \
{                                                          \
lg->lock() ;                                               \
(*lg) << microsec_clock::local_time() <<                   \
" " << left( modname(), 10 ) <<                            \
" " << d2ds( TASKID(), 5 ) <<                              \
" D line: "  << __LINE__  <<                               \
" >>L2 Function: " << __FUNCTION__ <<                      \
" -  " << s ;                                              \
lg->unlock() ;                                             \
}
#else
#define debug2( s )
#endif

#define LSPF_APP_MAKER( name )                         \
extern "C"                                             \
{                                                      \
	pApplication* maker() { return new name ; }    \
	void destroy( pApplication* p ) { delete p ; } \
}

#define PANEXIT_MAKER( name )                          \
extern "C"                                             \
{                                                      \
	panload* pan_make() { return new name ; }      \
	void pan_destroy( panload* p ) { delete p ; }  \
}

#define TRACE_INFO() __FUNCTION__

