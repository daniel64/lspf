#include <string>
#include <stack>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <panel.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition.hpp>

enum msgSET   { COND, UNCOND }   ;
enum tbSAVE   { WRITE, NOWRITE } ;
enum tbREP    { REPLACE, NOREPLACE } ;
enum tbDISP   { SHARE, EXCLUSIVE }   ;
enum dataType { INTEGER, STRING, ERROR } ;
enum nameCHCK { CHECK, NOCHECK }     ;
enum readCHCK { ROCHECK, NOROCHECK } ;
enum vcMODE   { LOCATE, MOVE }       ;
enum poolType { SHARED, PROFILE, BOTH, ASIS } ;
enum srCOND   { s_EQ, s_NE, s_LE, s_LT, s_GE, s_GT } ;
enum vTYPE    { SYSTEM, USER } ;

enum cuaType  { AB, ABSL, ABU, AMT, AWF, CT, CEF, CH, DT, ET, EE, FP, FK, IMT, LEF, LID, LI, NEF, NT, PI, PIN, 
                PT, PS, PAC, PUC, RP, SI, SAC, SUC, VOI, WMT, WT, WASL, CHAR, DATAIN, DATAOUT, GRPBOX, OUTPUT, TEXT } ;

// #define DEBUG1 1
// #define DEBUG2 1
#define MOD_NAME lspf

typedef unsigned int uint ;

using namespace std;
using namespace boost::posix_time;


// ***************************************** Start custom values **************************************************************************
//
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
// ZWAIT    - Wait time is ms to check if the application has gone into a wait-for-user-response (normally a few ms)
// ZMAXWAIT - Max wait time in ms to terminate the application if it has not gone into a wait-for-user-response (application may be looping)
//

#define HOME            "/home/daniel"
#define ZSPROF		HOME "/.lspf"
#define ZUPROF		HOME "/.lspf"
#define ZSYSPATH	HOME "/lspf"
#define ZLDPATH		"/tmp/path1:" ZSYSPATH "/Appls:" ZSYSPATH "/Appls2:/tmp/path2:/tmp/path3:/tmp/path4"
#define MLIB		ZUPROF "/mlib:" ZSYSPATH "/mlib"
#define PLIB		ZUPROF "/plib:" ZSYSPATH "/plib"
#define TLIB		ZUPROF "/tlib:" ZSYSPATH "/tlib"
#define ZREXPATH	HOME "/rexx:" ZSYSPATH "/rexx"
#define SLOG		"/tmp/syslog"
#define ALOG		"/tmp/appllog"
#define ZMAINPGM 	"PMAIN0A"
#define ZPANLPGM 	"PDPANLA"
#define ZEDITPGM 	"PEDIT01"
#define ZBRPGM 		"PBRO01A"
#define ZVIEWPGM 	"PVIEW0A"
#define ZFLSTPGM 	"PFLST0A"
#define ZHELPPGM	"PTUTORA"
#define ZOREXPGM	"POREXX1"
#define ZSHELP		"HPSPF01"
#define ZMAXSCRN	8
#define ZWAIT           5
#define ZMAXWAIT        1000
// ***************************************** End custom values *********************************************************

// ***************************************** CUA defaults **************************************************************
#define KAB     "YLN"
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


#define BLACK		COLOR_PAIR(0)
#define RED		COLOR_PAIR(1)
#define GREEN		COLOR_PAIR(2)
#define YELLOW		COLOR_PAIR(3)
#define BLUE		COLOR_PAIR(4)
#define MAGENTA 	COLOR_PAIR(5)
#define TURQ		COLOR_PAIR(6)
#define WHITE		COLOR_PAIR(7)

#define N_BLACK		0
#define N_RED		1
#define N_GREEN		2
#define N_YELLOW	3
#define N_BLUE		4
#define N_MAGENTA	5
#define N_TURQ		6
#define N_WHITE		7
#define B_RED		8
#define B_GREEN		9
#define B_YELLOW	10
#define B_BLUE		11
#define B_MAGENTA	12
#define B_TURQ		13
#define B_WHITE		14
#define R_RED		15
#define R_GREEN		16
#define R_YELLOW	17
#define R_BLUE		18
#define R_MAGENTA	19
#define R_TURQ		20
#define R_WHITE		21
#define U_RED		22
#define U_GREEN		23
#define U_YELLOW	24
#define U_BLUE		25
#define U_MAGENTA	26
#define U_TURQ		27
#define U_WHITE		28
#define P_RED		29
#define P_GREEN		30
#define P_YELLOW	31
#define P_BLUE		32
#define P_MAGENTA	33
#define P_TURQ		34
#define P_WHITE		35

#define _quotes( a ) #a
#define quotes( a ) _quotes( a )

#define log(t, s) LOGOUT << microsec_clock::universal_time() << \
                " " << left( quotes(MOD_NAME), 10 ) << " " << t << " " << s

#ifdef DEBUG1
#define debug1( s ) LOGOUT << microsec_clock::universal_time() << " " << left( quotes(MOD_NAME), 10 ) << \
                " D line: "  << __LINE__  << " >>L1 Function: " << __FUNCTION__ << " -  " << s
#else
#define debug1( s )
#endif

#ifdef DEBUG2
#define debug2( s ) LOGOUT << microsec_clock::universal_time() << " " << left( quotes(MOD_NAME), 10 ) << \
                " D line: "  << __LINE__  << " >>L2 Function: " << __FUNCTION__ << " -  " << s
#else
#define debug2( s )
#endif
