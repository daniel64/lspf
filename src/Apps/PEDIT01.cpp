/*  Compile with ::                                                                                   */
/* g++  -shared -fPIC -std=c++11 -Wl,-soname,libPEDIT01.so -lboost_regex -o libPEDIT01.so PEDIT01.cpp */
/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify
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

/* PDF-like editor program                                                                   */
/*                                                                                           */
/* Current status:                                                                           */
/* Most basic functions work okay                                                            */
/* Very basic and incomplete rexx edit macro support                                         */
/*                                                                                           */
/*                                                                                           */
/* ZRC/ZRSN codes returned                                                                   */
/*   0/0  Okay - No saves made                                                               */
/*   0/4  Okay - Data saved on at least one occation                                         */
/*   0/8  Okay - Data saved to an alternate name due to an abnormal termination              */
/*   4/0  File is currently being edited.  Edit aborted.                                     */
/*   4/4  Tabs found but option is not to convert.  Edit aborted.                            */
/*   8/4  Cannot use edit on file.  Browse instead (not yet implemented)                     */
/*   8/8  Error saving data                                                                  */

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <vector>
#include <queue>

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "eHilight.cpp"
#include <oorexxapi.h>
#include "PEDIT01.h"


using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PEDIT01

#define CLINESZ     8
#define maxURID     iline::maxURID
#define Global_Undo iline::Global_Undo
#define Global_Redo iline::Global_Redo
#define profUndoOn  iline::setUNDO[ taskid() ]
#define Global_File_level iline::Global_File_level

map<int, int> maxURID ;
map<int, bool> iline::setUNDO   ;
map<int, bool> iline::Redo_data ;
map<int, stack<int>>Global_Undo ;
map<int, stack<int>>Global_Redo ;
map<int, stack<int>>Global_File_level ;

set<string>PEDIT01::EditList          ;
edit_find PEDIT01::Global_efind_parms ;


void PEDIT01::application()
{
	llog( "I", "Application PEDIT01 starting.  Parms are "+ PARM <<endl ) ;

	errblock err ;

	string * pt  ;

	string panel ;
	string rfile ;
	string w1    ;
	string w2    ;

	initialise() ;

	rfile = ""   ;

	vcopy( "ZEDABRC", ZFILE, MOVE ) ;
	if ( ZFILE != "" )
	{
		rfile = ZFILE ;
		ZFILE = ZFILE.erase( ZFILE.size()-8 ) ;
		showEditRecovery() ;
		if ( ZRC == 0 )
		{
			vcopy( "ZCMD4", pt, LOCATE ) ;
			if ( (*pt) == "CANCEL" )
			{
				remove( rfile ) ;
				verase( "ZEDABRC", PROFILE ) ;
			}
			else if ( (*pt) != "DEFER" )
			{
				abendRecovery = true ;
				Edit() ;
				remove( rfile ) ;
				verase( "ZEDABRC", PROFILE ) ;
				abendRecovery = false ;
				rfile = ZFILE ;
			}
			else
			{
				rfile = ZFILE ;
			}
		}
	}

	ZFILE = "" ;

	if ( PARM != "" )
	{
		ZFILE = parseString( err, PARM, "FILE()" ) ;
		if ( err.error() )
		{
			uabend( "PEDT015", PARM ) ;
			return  ;
		}
		if ( ZFILE != "" && is_directory( ZFILE ) )
		{
			vput( "ZFILE", PROFILE ) ;
		}
		panel = parseString( err, PARM, "PANEL()" ) ;
		if ( err.error() )
		{
			uabend( "PEDT015", PARM ) ;
			return  ;
		}
		optMacro = parseString( err, PARM, "MACRO()" ) ;
		if ( err.error() )
		{
			uabend( "PEDT015", PARM ) ;
			return  ;
		}
		optProfile = parseString( err, PARM, "PROFILE()" ) ;
		if ( err.error() )
		{
			uabend( "PEDT015", PARM ) ;
			return  ;
		}
		optPreserve = ( parseString( err, PARM, "PRESERVE" ) == "OK" ) ;
		if ( err.error() || PARM != "" )
		{
			uabend( "PEDT015", PARM ) ;
			return  ;
		}
		if ( !optPreserve )
		{
			vcopy( "ZEDPRSPS", pt, LOCATE ) ;
			if ( RC == 0 && *pt == "YES" ) { optPreserve = true ; }
		}
		if ( ZFILE != "" && rfile == ZFILE )
		{
			cleanup() ;
			return    ;
		}
	}

	if ( ZFILE == "" || is_directory( ZFILE ) )
	{
		while ( true )
		{
			showEditEntry() ;
			if ( ZRC > 0 ) { break ; }
			vcopy( "SHOWDIR", pt, LOCATE ) ;
			if ( (*pt) == "YES" )
			{
				vreplace( "ZEDPRSPS", optPreserve   ? "YES" : "NO" ) ;
				vreplace( "ZEDTABSS", optNoConvTabs ? "YES" : "NO" ) ;
				vput( "ZEDPRSPS ZEDTABSS", SHARED ) ;
				vcopy( "ZFLSTPGM", pt, LOCATE ) ;
				select( "PGM(" + (*pt) + ") PARM(" + ZFILE + ")" ) ;
			}
			else
			{
				Edit() ;
			}
			vput( "ZFILE", PROFILE ) ;
		}
	}
	else
	{
		Edit() ;
	}
	cleanup() ;
}


void PEDIT01::initialise()
{
	string * pt ;

	control( "ABENDRTN", static_cast<void (pApplication::*)()>(&PEDIT01::cleanup_custom) ) ;
	control( "TIMEOUT", "DISABLE" ) ;

	vdefine( "ZCMD  ZVERB", &zcmd, &zverb ) ;
	vdefine( "ZAREA ZSHADOW ZAREAT ZFILE", &ZAREA, &ZSHADOW, &ZAREAT, &ZFILE ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &ZAREAW, &ZAREAD ) ;
	vdefine( "ZSCROLLA ZCOL1 ZCOL2", &ZSCROLLA, &ZCOL1, &ZCOL2 ) ;
	vdefine( "ZAPPLID  ZEDPROF ZEDPROFT ZHOME", &zapplid, &ZEDPROF, &ZEDPROFT, &zhome ) ;
	vdefine( "TYPE STR OCC LINES ", &TYPE, &STR, &OCC, &LINES ) ;
	vdefine( "EEIMAC EEPROF EETABSS EEPRSPS EECCAN", &EEIMAC, &EEPROF, &EETABSS, &EEPRSPS, &EECCAN ) ;

	vget( "ZEDPROF ZEDPROFT", PROFILE ) ;
	vget( "ZAPPLID ZHOME", SHARED ) ;

	pquery( "PEDIT012", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "PQUERY" ) ; }

	if ( ZEDPROF == "" ) { ZEDPROF = "DEFAULT" ; }

	ZDATAW = ZAREAW - CLINESZ ;
	ZASIZE = ZAREAW * ZAREAD  ;

	sdg.assign( ZDATAW, N_GREEN )   ;
	sdy.assign( ZDATAW, N_YELLOW )  ;
	sdw.assign( ZDATAW, N_WHITE )   ;
	sdr.assign( ZDATAW, N_RED )     ;
	sdb.assign( ZDATAW, B_BLUE )    ;
	sdrh.assign( ZDATAW, B_RED )    ;
	sdyh.assign( ZDATAW, B_YELLOW ) ;
	sdmk.assign( ZDATAW, B_WHITE )  ;

	slg.assign( CLINESZ, N_GREEN )   ;
	sly.assign( CLINESZ, N_YELLOW )  ;
	slw.assign( CLINESZ, N_WHITE )   ;
	slr.assign( CLINESZ, N_RED )     ;
	slb.assign( CLINESZ, B_BLUE )    ;
	slrh.assign( CLINESZ, B_RED )    ;
	slyh.assign( CLINESZ, B_YELLOW ) ;

	div.assign( ZAREAW-1, '-' )      ;

	clipBoard = "DEFAULT"  ;
	clipTable = "EDITCLIP" ;
	ZAHELP    = "HEDIT01"  ;

	maxURID[ taskid() ] = 0 ;

	abendRecovery = false ;
	optPreserve   = false ;
	optNoConvTabs = false ;
	optConfCancel = false ;

	vget( "ZEDPRSPS ZEDTABSS", SHARED ) ;
	vget( "EECCAN", PROFILE ) ;

	vcopy( "ZEDTABSS", pt, LOCATE ) ;
	if ( RC == 0 && *pt == "YES" ) { optNoConvTabs = true ; }

	vcopy( "EECCAN", pt, LOCATE ) ;
	if ( RC == 0 && *pt == "/" )   { optConfCancel = true ; }
}


void PEDIT01::Edit()
{
	string t      ;
	string zedalt ;

	RC = 0 ;
	pcmd.clear() ;
	defNames.clear() ;

	if ( Global_efind_parms.f_fset )
	{
		fcx_parms = Global_efind_parms ;
	}

	if ( optProfile != "" ) { ZEDPROF = optProfile ; }
	getEditProfile( ZEDPROF )  ;

	readFile() ;
	if ( RC > 0 )
	{
		if ( !pcmd.msgset() ) { pcmd.set_msg( "PSYS011E" ) ; }
		setmsg( pcmd.get_msg() ) ;
		return ;
	}

	if ( ZEDPROFT == "Y" && optProfile == "" )
	{
		ZEDPROF = determineLang() ;
		getEditProfile( ZEDPROF ) ;
	}

	zedalt = "" ;
	vget( "ZEDALT", SHARED ) ;
	if ( RC == 0 )
	{
		vcopy( "ZEDALT", zedalt, MOVE ) ;
	}
	if ( zedalt == "" ) { zedalt = ZFILE ; }

	CURFLD = "ZCMD" ;
	CURPOS = 1      ;

	hlight.hl_clear() ;
	miBlock.clear()   ;

	rebuildZAREA  = true  ;
	rebuildShadow = false ;
	macroRunning  = false ;
	creActive     = false ;
	cutActive     = false ;
	copyActive    = false ;
	pasteActive   = false ;
	colsOn        = false ;
	hideExcl      = false ;
	abendComplete = false ;
	termEdit      = false ;
	XTabz         = 0     ;
	ptopLine      = 0     ;
	nestLevel     = 0     ;
	aRow          = 0     ;
	aCol          = 0     ;

	placeCursor( 0, 0 )   ;

	t = ( optMacro != "" ) ? optMacro : ZEDPIMAC ;
	if ( t != "NONE" )
	{
		pcmd.set_cmd( t, defNames ) ;
		if ( pcmd.error() )
		{
			setmsg( pcmd.get_msg() ) ;
			return ;
		}
		if ( pcmd.isMacro() )
		{
			run_macro( true ) ;
			if ( pcmd.error() )
			{
				termEdit = false ;
			}
		}
		else
		{
			actionPrimCommand1() ;
			if ( !pcmd.error() && !pcmd.isActioned() )
			{
				actionPrimCommand2() ;
				if ( !pcmd.isActioned() )
				{
					vreplace( "ZSTR1", pcmd.get_cmd() ) ;
					pcmd.set_msg( "PEDM012M", 20 ) ;
				}
			}
			pcmd.cond_reset() ;
		}
		if ( !profLock && !wordpos( t, "CANCEL END" ) ) { profIMAC = t ; }
	}

	while ( !termEdit )
	{
		if      ( rebuildZAREA  ) { fill_dynamic_area() ; }
		else if ( rebuildShadow ) { ZSHADOW = CSHADOW   ; }
		rebuildShadow = false ;
		protNonDisplayChars() ;

		vreplace( "ZEDALT", zedalt ) ;

		ZCOL1 = d2ds( startCol, 5 ) ;
		ZCOL2 = d2ds( startCol+ZDATAW-1, 5 ) ;

		positionCursor() ;

		if ( optFindPhrase && profFindPhrase && fcx_parms.f_fset )
		{
			hiliteFindPhrase() ;
		}

		zcmd = pcmd.condget_cmd() ;
		if ( zcmd != "" && !pcmd.error() )
		{
			CURFLD = "ZCMD" ;
			CURPOS = zcmd.size() + 1;
		}
		pcmd.reset() ;

		display( "PEDIT012", pcmd.get_msg(), CURFLD, CURPOS ) ;

		if ( RC == 8 )
		{
			if ( upper( zcmd ) != "SAVE" ) { zcmd = "" ; }
			termEdit = true ;
		}

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		pcmd.clear_msg() ;
		clearCursor()    ;

		creActive   = false ;
		cutActive   = false ;
		copyActive  = false ;
		pasteActive = false ;

		if ( wordpos( upper( zcmd ), "CAN CANCEL" ) )
		{
			if ( showConfirmCancel() ) { break ; }
		}

		if ( ZCURFLD == "ZAREA" )
		{
			CURFLD = ZCURFLD ;
			CURPOS = ZCURPOS ;
			aRow   = ((ZCURPOS-1) / ZAREAW + 1)    ;
			aCol   = ((ZCURPOS-1) % ZAREAW + 1)    ;
			aURID  = s2data.at( aRow-1 ).ipos_URID ;
			aLCMD  = ( aCol > 0 && aCol < 9 )      ;
		}
		else
		{
			CURFLD = "ZCMD" ;
			CURPOS = 1      ;
			aRow   = 0      ;
			aCol   = 0      ;
			aURID  = 0      ;
			aLCMD  = false  ;
		}
		storeCursor( aURID, aCol-CLINESZ+startCol-2 ) ;

		pcmd.set_cmd( zcmd, defNames ) ;
		if ( pcmd.error() || pcmd.deactive() ) { continue ; }

		if ( pcmd.isMacro() )
		{
			run_macro() ;
			if ( termEdit && !pcmd.error() )
			{
				break ;
			}
			termEdit = false ;
			continue ;
		}

		getZAREAchanges() ;

		updateData() ;
		if ( pcmd.error() )  { termEdit = false ; continue ; }

		actionZVERB() ;

		setLineLabels() ;
		if ( pcmd.error() )  { termEdit = false ; continue ; }

		actionPrimCommand1() ;
		if ( pcmd.error() )  { termEdit = false ; continue ; }

		processNewInserts()  ;

		actionLineCommands() ;
		if ( pcmd.error() )  { termEdit = false ; continue ; }

		actionPrimCommand2() ;
		if ( pcmd.error() )  { termEdit = false ; continue ; }

		if ( termEdit && !pcmd.error() )
		{
			if ( !termOK() ) { termEdit = false ; }
		}

		cleanupRedoStacks() ;
	}

	releaseDynamicStorage() ;

	saveEditProfile( ZEDPROF ) ;
	EditList.erase( ZFILE )    ;
	Global_efind_parms = fcx_parms ;
}


void PEDIT01::showEditEntry()
{
	ZRC = 0  ;
	pcmd.clear() ;

	display( "PEDIT011", pcmd.get_msg(), "ZCMD1" ) ;
	if ( RC == 8 ) { ZRC = 4 ; return ; }

	optProfile    = EEPROF ;
	optMacro      = EEIMAC ;
	optPreserve   = ( EEPRSPS == "/" ) ;
	optNoConvTabs = ( EETABSS == "/" ) ;
	optConfCancel = ( EECCAN  == "/" ) ;
}


void PEDIT01::showEditRecovery()
{
	ZRC = 0  ;
	pcmd.clear() ;

	display( "PEDIT014", pcmd.get_msg(), "ZCMD4" ) ;
	if ( RC == 8 ) { ZRC = 4 ; return ; }

	optPreserve   = ( EEPRSPS == "/" ) ;
	optNoConvTabs = ( EETABSS == "/" ) ;
}


bool PEDIT01::showConfirmCancel()
{
	if ( !optConfCancel || !fileChanged ) { return true ; }

	addpop( "", 2, 4 ) ;

	display( "PEDIT016", "", "ZCMD6" ) ;
	if ( RC == 8 ) { rempop() ; termEdit = false ; return false ; }

	rempop() ;
	return true ;
}


bool PEDIT01::termOK()
{
	if ( !fileChanged || (!profASave && !profSaveP) )
	{
		return true ;
	}
	if ( !profASave && pcmd.cmd_not( "SAVE" ) )
	{
		setmsg( "PEDT011O" ) ;
		return false ;
	}
	if ( saveFile() ) { setmsg( "PEDT011P" ) ; }
	else              { return false         ; }
	return true ;
}


void PEDIT01::readFile()
{
	int i   ;
	int j   ;
	int p   ;
	int pos ;

	bool invChars   ;
	bool lowrOnRead ;

	string zdate  ;
	string ztimel ;
	string inLine ;
	string fname  ;

	iline * p_iline ;

	const string tod = " Top of Data **********" ;
	const string bod = " Bottom of Data **********" ;

	vector<string> Notes ;
	vector<string> Msgs  ;

	vector<iline *>::iterator it ;

	boost::system::error_code ec ;

	fname = ZFILE ;
	if ( abendRecovery )
	{
		fname      += ".##abend" ;
		fileChanged = true ;
	}
	else
	{
		fileChanged = false ;
	}

	if ( EditList.count( ZFILE ) > 0 )
	{
		pcmd.set_msg( "PEDT014A" ) ;
		ZRESULT = "File In Use" ;
		RC      = 4 ;
		ZRC     = 4 ;
		ZRSN    = 0 ;
		return      ;
	}

	topLine      = 0 ;
	startCol     = 1 ;
	maxCol       = 1 ;
	Level        = 0 ;
	tabsOnRead   = false ;
	lowrOnRead   = false ;
	invChars     = false ;

	data.clear() ;
	maxURID[ taskid() ] = 0 ;

	if ( !exists( fname ) )
	{
		p_iline          = new iline( taskid() ) ;
		p_iline->il_type = LN_TOD ;
		p_iline->put_idata( centre( tod, ZAREAW, '*' ), 0 ) ;
		data.push_back( p_iline ) ;
		for ( i = 0 ; i < ZAREAD-3 ; i++ )
		{
			p_iline = new iline( taskid() ) ;
			p_iline->il_type  = LN_ISRT ;
			p_iline->put_idata( "" )  ;
			data.push_back( p_iline ) ;
		}
		p_iline = new iline( taskid() ) ;
		p_iline->il_type = LN_BOD ;
		p_iline->put_idata( centre( bod, ZAREAW, '*' ), 0 ) ;
		data.push_back( p_iline )  ;
		it = data.begin() ;
		(*it)->clear_Global_Undo() ;
		(*it)->clear_Global_Redo() ;
		(*it)->clear_Global_File_level() ;
		(*it)->reset_Redo_data() ;
		EditList.insert( ZFILE ) ;
		addSpecial( 'N', topLine, Notes ) ;
		saveLevel = 0 ;
		return        ;
	}

	std::ifstream fin( fname.c_str() ) ;

	if ( !fin.is_open() )
	{
		ZRESULT = "Open Error" ;
		RC      = 16 ;
		return ;
	}

	p_iline          = new iline( taskid() ) ;
	p_iline->il_type = LN_TOD ;
	p_iline->put_idata( centre( tod, ZAREAW, '*' ), 0 ) ;
	data.push_back( p_iline ) ;

	while ( getline( fin, inLine ) )
	{
		p_iline = new iline( taskid() ) ;
		pos = inLine.find( '\t' ) ;
		while ( pos != string::npos )
		{
			tabsOnRead = true ;
			if ( optNoConvTabs ) { break ; }
			j   = profXTabz - (pos % profXTabz ) ;
			inLine.replace( pos, 1,  j, ' ' )  ;
			pos = inLine.find( '\t', pos + 1 ) ;
		}
		if ( !lowrOnRead )
		{
			if ( any_of( inLine.begin(), inLine.end(),
				[]( char c )
				{
					return ( islower( c ) ) ;
				} ) )
			{
				lowrOnRead = true ;
			}
		}
		if ( !invChars )
		{
			if ( any_of( inLine.begin(), inLine.end(),
				[]( char c )
				{
					return ( !isprint( c ) ) ;
				} ) )
			{
				invChars = true ;
			}
		}
		if ( maxCol < inLine.size() ) { maxCol = inLine.size() ; }
		p_iline->il_type = LN_FILE ;
		p_iline->put_idata( inLine, 0 ) ;
		data.push_back( p_iline ) ;
	}
	if ( tabsOnRead )
	{
		pcmd.set_msg( optNoConvTabs ? "PEDT014N" : "PEDT014W", 4 ) ;
	}
	if ( data.size() == 1 )
	{
		for ( i = 0 ; i < ZAREAD-3 ; i++ )
		{
			p_iline = new iline( taskid() ) ;
			p_iline->il_type  = LN_ISRT ;
			p_iline->put_idata( "" )  ;
			data.push_back( p_iline ) ;
		}
		p_iline = new iline( taskid() ) ;
		p_iline->il_type = LN_BOD ;
		p_iline->put_idata( centre( bod, ZAREAW, '*' ), 0 ) ;
		data.push_back( p_iline )    ;
		it = data.begin() ;
		(*it)->clear_Global_Undo() ;
		(*it)->clear_Global_Redo() ;
		(*it)->clear_Global_File_level() ;
		(*it)->reset_Redo_data()   ;
		EditList.insert( ZFILE )          ;
		addSpecial( 'N', topLine, Notes ) ;
		saveLevel = 0 ;
		fin.close()   ;
		return        ;
	}
	fin.close() ;
	maxCol++    ;
	p_iline = new iline( taskid() ) ;
	p_iline->il_type = LN_BOD ;
	p_iline->put_idata( centre( bod, ZAREAW, '*' ), 0 ) ;
	data.push_back( p_iline ) ;

	if ( tabsOnRead && !optNoConvTabs && !profXTabs )
	{
		Msgs.push_back( "-CAUTION- Profile changed to XTABS ON (from XTABS OFF) because the" )  ;
		Msgs.push_back( "          data contains tabs" )  ;
		profXTabs = true ;
	}
	else if ( tabsOnRead && optNoConvTabs && profXTabs )
	{
		Msgs.push_back( "-CAUTION- Profile changed to XTABS OFF (from XTABS ON) because the" )  ;
		Msgs.push_back( "          data contain tabs but option is not to convert to spaces" )  ;
		profXTabs = false ;
	}
	else if ( !tabsOnRead && profXTabs )
	{
		Msgs.push_back( "-CAUTION- Profile changed to XTABS OFF (from XTABS ON) because the" )  ;
		Msgs.push_back( "          data does not contain tabs" )  ;
		profXTabs = false ;
	}

	if ( invChars )
	{
		Msgs.push_back( "-CAUTION- Data contains invalid (non-display) characters.  Use command" )  ;
		Msgs.push_back( "          ===> FIND P'.' to position cursor to these" ) ;
	}

	if ( lowrOnRead && profCaps )
	{
		Msgs.push_back( "-CAUTION- Profile changed to CAPS OFF (from CAPS ON) because the" )  ;
		Msgs.push_back( "          data contains lower case characters" ) ;
		profCaps = false ;
	}
	else if ( !lowrOnRead && !profCaps )
	{
		Msgs.push_back( "-CAUTION- Profile changed to CAPS ON (from CAPS OFF) because the" )  ;
		Msgs.push_back( "          data does not contain lower case characters" ) ;
		profCaps = true ;
	}

	if ( !profUndoOn )
	{
		Msgs.push_back( "-Warning- UNDO/REDO commands are not available until you change" )  ;
		Msgs.push_back( "          your edit profile using the command SETUNDO ON" ) ;
	}

	if ( profRecover && !abendRecovery )
	{
		vcopy( "ZJ4DATE", zdate, MOVE ) ;
		vcopy( "ZTIMEL", ztimel, MOVE ) ;
		p = ZFILE.find_last_of( '/' )   ;
		copy_file( ZFILE, recoverLoc + ZFILE.substr( p+1 ) + "-" + zdate + "-" + ztimel, ec ) ;
		if ( ec.value() != boost::system::errc::success )
		{
			pcmd.set_msg( "PEDT012F" ) ;
		}
	}

	addSpecial( 'M', topLine, Msgs  ) ;
	addSpecial( 'N', topLine, Notes ) ;

	it = data.begin() ;
	(*it)->clear_Global_Undo() ;
	(*it)->clear_Global_Redo() ;
	(*it)->clear_Global_File_level() ;
	(*it)->reset_Redo_data()   ;

	saveLevel = 0 ;
	EditList.insert( ZFILE ) ;
	if ( profHilight && profLang == "AUTO" )
	{
		detLang = determineLang() ;
	}
}


bool PEDIT01::saveFile()
{
	int i ;

	string * pt ;

	string t1 ;
	string f  ;

	f = ZFILE ;
	vector<iline *>::iterator it ;
	std::ofstream fout( f.c_str() ) ;

	if ( !fout.is_open() )
	{
		ZRESULT = "Open for output Error" ;
		RC      = 16 ;
		ZRC     = 8  ;
		ZRSN    = 8  ;
		pcmd.set_msg( "PEDT011Q" ) ;
		return false ;
	}

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->isValidFile() ) { continue ; }
		pt = (*it)->get_idata_ptr() ;
		if ( !optPreserve ) { (*it)->set_idata_trim() ; }
		if ( profXTabs )
		{
			t1 = "" ;
			for ( i = 0 ; i < pt->size() ; i++ )
			{
				if ( (i % profXTabz == 0)         &&
				     pt->size() > (i+profXTabz-1) &&
				     pt->compare( i, profXTabz, string( profXTabz, ' ' ) ) == 0 )
				{
					t1.push_back( '\t' )  ;
					i = i + profXTabz - 1 ;
				}
				else if ( pt->at( i ) != ' ' ) { break ; }
				else                           { t1.push_back( ' ' ) ; }

			}
			if ( i < pt->size() )
			{
				t1 += pt->substr( i ) ;
			}
			fout << t1 <<endl ;
		}
		else
		{
			fout << *pt <<endl ;
		}
	}
	fileChanged = false ;
	fout.close() ;
	if ( fout.fail() ) { pcmd.set_msg( "PEDT011Q" ) ; ZRC = 8 ; ZRSN = 8 ; return false ; }

	if ( profUndoOn )
	{
		if ( !profUndoKeep )
		{
			removeRecoveryData() ;
		}
		it        = data.begin() ;
		saveLevel = (*it)->get_Global_File_level() ;
	}
	pcmd.set_msg( "PEDT011P", 4 ) ;
	ZRC  = 0 ;
	ZRSN = 4 ;
	return true ;
}


void PEDIT01::fill_dynamic_area()
{
	// s2data is only valid while the data vector has not changed since building the screen
	// ie. after fill_dynamic_area until the end of procedure processNewInserts()

	// BUG:  NULLS ON should put nulls at the end-of-data to the right margin, even if there is more
	//       data after.

	int i  ;
	int j  ;
	int l  ;
	int ln ;
	int sl ;
	int fl ;
	int elines ;

	uint dl ;

	ipos ip ;

	string t1 ;
	string t2 ;
	string t3 ;
	string t4 ;
	string lcc ;

	string * pt1 ;

	const char nulls( 0x00 ) ;

	const string  din( 1, 0x01 ) ;
	const string dout( 1, 0x02 ) ;

	vector<iline * >::iterator it ;

	ZAREA   = "" ;
	ZSHADOW = "" ;

	ZAREA.reserve( ZASIZE )   ;
	ZSHADOW.reserve( ZASIZE ) ;

	t1.reserve( ZDATAW )   ;
	t2.reserve( 2*ZDATAW ) ;
	t3.reserve( 2*ZDATAW ) ;
	t4.reserve( 2*ZDATAW ) ;

	s2data.clear() ;

	if ( data.at( topLine )->il_deleted ) { topLine = getNextDataLine( topLine ) ; }

	fl = getFileLine( topLine ) ;
	for ( i = 0 ; i < ZAREAD ; i++ ) { s2data[ i ] = ip ; lchar[ i ] = ZDATAW ; }

	for ( sl = 0, dl = topLine ; dl < data.size() ; dl++ )
	{
		it = getLineItr( dl ) ;
		if ( (*it)->il_deleted ) { continue ; }
		if ( (*it)->is_file() )  { fl++     ; }
		if ( hideExcl && (*it)->il_excl )
		{
			if ( dl == topLine )
			{
				dl = getLastEX( dl ) ;
				topLine = dl ;
			}
			else
			{
				dl = getLastEX( dl ) ;
			}
			fl = getFileLine( dl ) + 1 ;
			continue ;
		}
		ip.clear() ;
		if ( (*it)->il_excl )
		{
			elines          = getExBlockSize( dl ) ;
			ip.ipos_dl      = dl ;
			ip.ipos_URID    = (*it)->il_URID ;
			s2data.at( sl ) = ip ;
			t1 = copies( "-  ", (ZAREAW - 30)/3 - 2 ) + d2ds( elines ) + " Line(s) Not Displayed" ;
			t2 = (*it)->il_lcc == "" ? "- - - " : left( (*it)->il_lcc, 6 ) ;
			ZAREA   += din + t2 + dout + substr( t1, 1, ZAREAW-8 ) ;
			ZSHADOW += slr + sdw ;
			if ( ZAREA.size() >= ZASIZE ) { break ; }
			sl++ ;
			if ( dl == topLine )
			{
				dl      = getFirstEX( dl ) ;
				topLine = dl               ;
			}
			dl = dl + getDataBlockSize( dl ) - 1 ;
			fl = getFileLine( dl ) + 1  ;
			continue ;
		}
		if ( (*it)->il_lcc != "" )
		{
			lcc      = left( (*it)->il_lcc, 6 ) ;
			ZSHADOW += slr ;
		}
		else
		{
			if ( (*it)->hasLabel() )
			{
				lcc      = left( (*it)->getLabel(), 6 ) ;
				ZSHADOW += slr ;
			}
			else if ( (*it)->il_status != LS_NONE )
			{
				switch ( (*it)->il_status )
				{
				case LS_CHNG:
					lcc = "==CHG>" ; ZSHADOW += slr ; break ;
				case LS_ERROR:
					lcc = "==ERR>" ; ZSHADOW += slr ; break ;
				case LS_UNDO:
					lcc = "=UNDO>" ; ZSHADOW += slr ; break ;
				case LS_REDO:
					lcc = "=REDO>" ; ZSHADOW += slr ; break ;
				}
			}
			else
			{
				switch ( (*it)->il_type )
				{
				case LN_FILE:
					lcc = d2ds( fl, 6 ) ; ZSHADOW += slg ; break ;
				case LN_NOTE:
					lcc = "=NOTE=" ; ZSHADOW += slr ; break ;
				case LN_INFO:
					lcc = "======" ; ZSHADOW += slr ; break ;
				case LN_MSG:
					lcc = "==MSG>" ; ZSHADOW += slr ; break ;
				case LN_COL:
					lcc = "=COLS>" ; ZSHADOW += slr ; break ;
				case LN_PROF:
					lcc = "=PROF>" ; ZSHADOW += slr ; break ;
				case LN_TABS:
					lcc = "=TABS>" ; ZSHADOW += slr ; break ;
				case LN_MASK:
					lcc = "=MASK>" ; ZSHADOW += slr ; break ;
				case LN_BNDS:
					lcc = "=BNDS>" ; ZSHADOW += slr ; break ;
				case LN_ISRT:
					lcc = "''''''" ; ZSHADOW += slr ; break ;
				case LN_TOD:
				case LN_BOD:
					lcc = "******" ; ZSHADOW += slr ; break ;
				}
			}
		}
		if ( (*it)->is_file() || (*it)->is_isrt() )
		{
			if ( (*it)->il_hex || profHex )
			{
				t1 = (*it)->get_idata() ;
				ln = (*it)->get_idata_len() - startCol + 1 ;
				if      ( ln < 0 )      { ln = 0      ; }
				else if ( ln > ZDATAW ) { ln = ZDATAW ; }
				if ( !profNulls )
				{
					t1 = substr( t1, startCol, ZDATAW ) ;
				}
				else
				{
					if ( t1.size() > (startCol - 1) )
					{
						t1.erase( 0, startCol-1 ) ;
					}
					else
					{
						if ( profNullA ) { t1 = ""                    ; }
						else             { t1 = string( ZDATAW, ' ' ) ; }
					}
					if ( !profNullA && t1.size() < ZDATAW )
					{
						if ( t1 == "" || t1.back() != ' ' )
						{
							t1.push_back( ' ' ) ;
							ln++ ;
						}
					}
					t1.resize( ZDATAW, nulls ) ;
					lchar[ ZAREA.size() / ZAREAW ] = ln ;
				}
				ZAREA += din + lcc + din + t1 ;
				if      ( (*it)->is_isrt() ) { ZSHADOW += sdyh                          ; }
				else if ( (*it)->il_mark   ) { ZSHADOW.back() = R_RED ; ZSHADOW += sdmk ; }
				else                         { ZSHADOW += sdy                           ; }
				ip.ipos_dl   = dl ;
				ip.ipos_URID = (*it)->il_URID ;
				s2data.at( sl ) = ip ;
				if ( ZAREA.size() >= ZASIZE ) { break ; }
				if ( profVert )
				{
					t2 = cs2xs( t1 ) ;
					t3 = string( ZDATAW, '2' ) ;
					t4 = string( ZDATAW, '0' ) ;
					for ( i = 0, l = 0 ; l < (ln * 2) ; i++, l++ )
					{
						t3[ i ] = t2[ l ] ;
						l++ ;
						t4[ i ] = t2[ l ] ;
					}
				}
				else
				{
					t3  = cs2xs( t1.substr( 0, t1.find( nulls ) ) ) ;
					t3 += copies( "20", (ZDATAW*2-t3.size())/2 ) ;
					t4  = t3.substr( ZDATAW ) ;
					t3.erase( ZDATAW ) ;
				}
				ZAREA   += "       " + din + t3 ;
				ZSHADOW += slw + sdg  ;
				ip.ipos_hex = 1 ;
				sl++ ;
				s2data.at( sl ) = ip ;
				if ( ZAREA.size() >= ZASIZE ) { break ; }
				ZAREA   += "       " + din + t4 ;
				ZSHADOW += slw + sdg ;
				ip.ipos_hex = 2 ;
				sl++ ;
				s2data.at( sl ) = ip ;
				if ( ZAREA.size() >= ZASIZE ) { break ; }
				ZAREA   += dout + div ;
				ZSHADOW += slw  + sdw ;
				ip.ipos_hex = 0    ;
				ip.ipos_div = true ;
				sl++ ;
				s2data.at( sl ) = ip ;
				if ( ZAREA.size() >= ZASIZE ) { break ; }
				sl++ ;
			}
			else
			{
				if ( !profNulls )
				{
					t1 = substr( (*it)->get_idata(), startCol, ZDATAW ) ;
				}
				else
				{
					t1 = (*it)->get_idata() ;
					ln = (*it)->get_idata_len() - startCol + 1 ;
					if      ( ln < 0 )      { ln = 0      ; }
					else if ( ln > ZDATAW ) { ln = ZDATAW ; }
					if ( t1.size() > (startCol - 1) )
					{
						t1.erase( 0, startCol-1 ) ;
					}
					else
					{
						if ( profNullA ) { t1 = ""                    ; }
						else             { t1 = string( ZDATAW, ' ' ) ; }
					}
					if ( !profNullA && t1.size() < ZDATAW )
					{
						if ( t1 != "" && t1.back() != ' ' )
						{
							t1.push_back( ' ' ) ;
							ln++ ;
						}
					}
					t1.resize( ZDATAW, nulls ) ;
					lchar[ ZAREA.size() / ZAREAW ] = ln ;
				}
				ZAREA += din + lcc + din + t1 ;
				if      ( (*it)->is_isrt() ) { ZSHADOW += sdyh                          ; }
				else if ( (*it)->il_mark   ) { ZSHADOW.back() = R_RED ; ZSHADOW += sdmk ; }
				else                         { ZSHADOW += sdy                           ; }
				ip.ipos_dl      = dl ;
				ip.ipos_URID    = (*it)->il_URID ;
				s2data.at( sl ) = ip ;
				sl++ ;
			}
		}
		else
		{
			switch ( (*it)->il_type )
			{
			case LN_COL:
				ZAREA   += din + lcc + dout + getColumnLine() ;
				ZSHADOW += sdw ;
				break ;

			case LN_BNDS:
				t1 = string( LeftBnd-1, ' ' ) + "<" ;
				if ( RightBnd > 0 ) { t1 += string( RightBnd-t1.size()-1, ' ' ) + ">" ; }
				ZAREA   += din + lcc + din + substr( t1, startCol, ZDATAW ) ;
				ZSHADOW += sdr ;
				break ;

			case LN_MASK:
				(*it)->put_idata( getMaskLine() ) ;
				ZAREA   += din + lcc + din + substr( (*it)->get_idata(), startCol, ZDATAW ) ;
				ZSHADOW += sdr ;
				break ;

			case LN_PROF:
				l   = ZSHADOW.size()  ;
				j   = profLang.size() ;
				pt1 = (*it)->get_idata_ptr() ;
				ZAREA   += din + lcc + dout + substr( *pt1, 1, ZDATAW ) ;
				ZSHADOW += sdw ;
				if ( profHilight && profLang != "AUTO" )
				{
					if ( (*it)->il_profln == 3 )
					{
						ZSHADOW.replace( pt1->find( "HILIGHT " )+l+8, j, j, N_RED ) ;
					}
				}
				if ( (*it)->il_profln == 0 && ZEDPROFT != "Y" )
				{
					j = ZEDPROF.size() ;
					ZSHADOW.replace( l+4, j, j, N_RED ) ;
				}
				break ;

			case LN_TABS:
				(*it)->put_idata( tabsLine ) ;
				ZAREA   += din + lcc + din + substr( (*it)->get_idata(), startCol, ZDATAW ) ;
				ZSHADOW += sdr ;
				break ;

			default:
				ZAREA += din + lcc + dout + substr( (*it)->get_idata(), 1, ZDATAW ) ;
				(*it)->is_tod_or_bod() ? ZSHADOW += sdb : ZSHADOW += sdw ;
			}
			ip.ipos_dl   = dl ;
			ip.ipos_URID = (*it)->il_URID ;
			s2data.at( sl ) = ip ;
			sl++ ;
		}
		if ( ZAREA.size() >= ZASIZE ) { break ; }
	}

	ZAREA.resize( ZASIZE, ' ' ) ;
	ZSHADOW.resize( ZASIZE, N_GREEN ) ;

	if ( profHilight && !hlight.hl_abend )
	{
		fill_hilight_shadow() ;
	}

	if ( colsOn )
	{
		t1 = dout + "=COLS> " + getColumnLine() ;
		ZAREA.replace( 0, ZAREAW, t1 ) ;
		ZSHADOW.replace( 0, CLINESZ, slw ) ;
		ZSHADOW.replace( 8, ZDATAW, sdw ) ;
	}

	CAREA   = ZAREA   ;
	CSHADOW = ZSHADOW ;
	rebuildZAREA = false ;
}


void PEDIT01::protNonDisplayChars()
{
	// Protect non-display characters in ZAREA with by replacing with datain attribute and store original in XAREA.
	// lchar is the last line position to replace.  After this we may have nulls in ZAREA due to the NULLS command

	int i ;
	int j ;
	int k ;

	const char din(0x01) ;

	XAREA = string( ZASIZE, ' ' ) ;
	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		k = i * ZAREAW + 8 ;
		for ( j = 0 ; j < lchar[ i ] ; j++, k++ )
		{
			if ( !isprint( ZAREA[ k ] ) )
			{
				XAREA[ k ] = ZAREA[ k ] ;
				ZAREA[ k ] = din        ;
			}
		}
	}
}


void PEDIT01::fill_hilight_shadow()
{
	// Build il_Shadow starting at the first invalid shadow line in data, backing up
	// to the line after the position where there are no open brackets/comments.
	// Stop when bottom of ZAREA reached.  Invalidate the shadow line after the last line
	// on the screen to continue building after a scroll.

	// il_vShadow - true if there is a valid shadow line for this data line (stored in il_Shadow)
	// il_wShadow - true if no open brackets, comments, if or do statements (optional), quotes or
	//              a continuation at the end of the file line for this shadow line

	int i  ;
	int ll ;
	int w  ;

	uint dl ;

	string t ;

	hlight.hl_language = detLang ;

	for ( i = ZAREAD-1 ; i >= 0 ; i-- )
	{
		ll = s2data.at( i ).ipos_dl ;
		if ( ll > 0 ) { break ; }
	}

	for ( w = 0, dl = 1 ; dl <= ll ; dl++ )
	{
		if ( !data.at( dl )->isValidFile() ) { continue ; }
		if ( !data.at( dl )->il_vShadow )    { break    ; }
		if (  data.at( dl )->il_wShadow )    { w = dl   ; }
	}
	if ( dl <= ll )
	{
		hlight.hl_oBrac1   = 0 ;
		hlight.hl_oBrac2   = 0 ;
		hlight.hl_oIf      = 0 ;
		hlight.hl_oDo      = 0 ;
		hlight.hl_ifLogic  = profIfLogic ;
		hlight.hl_doLogic  = profDoLogic ;
		hlight.hl_Paren    = profParen   ;
		hlight.hl_oComment = false ;
		for ( dl = w + 1 ; dl <= ll ; dl++ )
		{
			if ( !data.at( dl )->isValidFile() ) { continue ; }
			addHilight( lg, hlight, data.at( dl )->get_idata(), data.at( dl )->il_Shadow ) ;
			if ( hlight.hl_abend ) { pcmd.set_msg( "PEDT013E" ) ; return ; }
			data.at( dl )->il_vShadow = true ;
			data.at( dl )->il_wShadow = ( hlight.hl_oBrac1 == 0 &&
						      hlight.hl_oBrac2 == 0 &&
						      hlight.hl_oIf    == 0 &&
						      hlight.hl_oDo    == 0 &&
						     !hlight.hl_continue    &&
						     !hlight.hl_oQuote      &&
						     !hlight.hl_oComment ) ;
		}
		dl = getValidFileLineNext( dl ) ;
		if ( dl != 0 )
		{
			data.at( dl )->il_vShadow = false ;
			data.at( dl )->il_wShadow = false ;
		}
	}
	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		if ( s2data.at( i ).ipos_hex > 0 ||
			   s2data.at( i ).ipos_div ) { continue ; }
		dl = s2data.at( i ).ipos_dl ;
		if ( !data.at( dl )->isValidFile() ||
			    data.at( dl )->il_mark ||
			    data.at( dl )->il_excl )  { continue ; }
		if ( data.at( dl )->specialLabel() )
		{
			ZSHADOW.replace( (ZAREAW*i + CLINESZ), ZDATAW, string( ZDATAW, B_BLUE ) ) ;
		}
		else
		{
			t = data.at( dl )->il_Shadow ;
			if ( startCol > 1 ) { t.erase( 0, startCol-1 ) ; }
			if ( t.size() > ZDATAW ) { t.resize( ZDATAW, N_GREEN ) ; }
			ZSHADOW.replace( (ZAREAW*i + CLINESZ), t.size(), t ) ;
		}
	}
}


void PEDIT01::clr_hilight_shadow()
{
	for_each( data.begin(), data.end(),
		[](iline *& a)
		{
			a->il_vShadow = false ;
			a->il_wShadow = false ;
		} ) ;
	hlight.hl_clear() ;
}


void PEDIT01::getZAREAchanges()
{
	// Put back non-display characters stored in XAREA (replaced by datain attribute).  Set touched/changed byte.

	// Algorithm for getting the line command:
	//    Remove leading digits and spaces (these are ignored)
	//    Find last changed character
	//    If cursor is on the field and after this point, use cursor positon to truncate field (characters after cursor position are ignored if not changed)
	//    Else use this changed character position to truncate field (trailing unchanged characters are ignored)

	// Strip off any remaining leading "- - - " for excluded lines
	// Strip off any remaining leading "''''''" for new insert lines
	// Strip off label characters from the line command if label '.' has been overwritten or command has been
	// entered after the label (label+space+lcc)

	int i   ;
	int j   ;
	int l   ;
	int off ;
	int sp  ;
	int ep  ;

	bool touched ;
	bool changed ;

	uint dl ;

	string lcc ;
	string lab ;

	const char duserMod(0x03) ;
	const char ddataMod(0x04) ;

	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		off     = i * ZAREAW + 7 ;
		touched = false ;
		changed = false ;
		for ( l = off + 1, j = 0 ; j < ZDATAW ; j++, l++ )
		{
			if ( XAREA[ l ] != ' ' )
			{
				if      ( ZAREA[ l ] == duserMod ) { touched = true ; }
				else if ( ZAREA[ l ] == ddataMod ) { changed = true ; }
				ZAREA[ l ] = XAREA[ l ] ;
			}
		}
		if ( changed )
		{
			ZAREA[ off ] = ddataMod ;
		}
		else if ( touched )
		{
			ZAREA[ off ] = duserMod ;
		}
	}

	sTouched.clear() ;
	sChanged.clear() ;

	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		dl            = s2data.at( i ).ipos_dl ;
		off           = i * ZAREAW ;
		sTouched[ i ] = false      ;
		sChanged[ i ] = false      ;
		if ( ZAREA[ off ] == ddataMod )
		{
			lcc = "" ;
			if ( data.at( dl )->il_lcc == "" && !data.at( dl )->hasLabel() )
			{
				for ( j = off + 1 ; j < off + 6 ; j++ )
				{
					if ( ZAREA[ j ] == ' ' || isdigit( ZAREA[ j ] ) )
					{
						ZAREA[ j ] = ' ' ;
						continue ;
					}
					break ;
				}
				sp = j - off ;
				if ( aLCMD && aRow == ( i + 1 ) )
				{
					for ( j = off + 6 ; j > off ; j-- )
					{
						if ( j < ( aCol + off - 1 ) || ( ZAREA[ j ] != CAREA[ j ] ) )
						{
							break ;
						}
					}

				}
				else
				{
					for ( j = (off + 6) ; j > off ; j-- )
					{
						if ( ZAREA[ j ] != CAREA[ j ] )  { break ; }
					}

				}
				ep = j - off + 1 ;
				if ( sp < ep ) { lcc = ZAREA.substr( (sp + off), (ep - sp) ) ; }
			}
			else
			{
				lcc = ZAREA.substr( (1 + off), 6 ) ;
			}
			if ( data.at( dl )->il_excl )
			{
				for ( j = 0 ; j < lcc.size() ; j++ )
				{
					if      ( j%2 == 0 && lcc[ j ] == '-' ) { lcc[ j ] = ' ' ; }
					else if ( j%2 == 1 && lcc[ j ] != ' ' ) { break          ; }
				}
			}
			trim( iupper( lcc ) ) ;
			if ( lcc == "" )
			{
				data.at( dl )->resetFileStatus() ;
			}
			else if ( data.at( dl )->is_isrt() )
			{
				lcc = strip( lcc, 'L', '\'' ) ;
			}
			else if ( !data.at( dl )->is_file() )
			{
				lcc = strip( lcc, 'L', '=' ) ;
			}
			else
			{
				lab = data.at( dl )->getLabel() ;
				if ( lab != "" && ( lcc.front() != '.' || lcc.find( ' ' ) != string::npos ) )
				{
					for ( j = 0 ; j < lab.size() && j < lcc.size() ; j++ )
					{
						if ( lab[ j ] == lcc[ j ] ) { lcc[ j ] = ' ' ; }
					}
					if ( trim( lcc ) == "" )
					{
						data.at( dl )->il_lcc = "." ;
					}
				}
			}
			if ( datatype( lcc, 'W' ) ) { lcc = "" ; }
			if ( lcc == "" )
			{
				if ( data.at( dl )->il_lcc == "" ) { data.at( dl )->clearLabel() ; }
				else                               { data.at( dl )->il_lcc = ""  ; }
			}
			else
			{
				data.at( dl )->il_lcc = lcc ;
			}
			rebuildZAREA = true ;
		}
		if ( ZAREA[ off + 7 ] == duserMod )
		{
			sTouched[ i ] = true ;
		}
		else if ( ZAREA[ off + 7 ] == ddataMod )
		{
			sChanged[ i ] = true ;
		}
	}

}


void PEDIT01::updateData()
{
	// Update the data vector with changes made to ZAREA.

	// First, process any changed hex display lines and update the character data with any changes.
	// If only one of the hex lines is visible, use the low 4-bits from the character data (for HEX VERT)

	// Update shadow variable (0x00 until EOD, then 0xFF) and convert nulls to spaces in ZAREA for
	// any changes past the old EOD (old EOD is max of lchar,shadow0xFF).  This is in case NULLS is ON.

	// In the shadow variable, 0xFE indicates a character delete, 0xFF indicates nulls->spaces conversion.

	// BUG: Does not work correctly for adding nulls in hex display

	int d ;
	int i ;
	int j ;
	int k ;
	int n ;
	int m ;
	int p ;

	int ln  ;
	int l1  ;
	int l1e ;
	int l1h ;
	int l2  ;
	int l3  ;

	string s ;
	string t ;

	const char din(0x01) ;

	vector<iline *>::iterator it ;

	Level++ ;
	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		if ( !sChanged[ i ] || s2data.at( i ).ipos_hex == 0 ) { continue ; }
		ln  = i - s2data.at(i).ipos_hex ;
		l1  = ln * ZAREAW + CLINESZ ;
		l1h = l1 + ZDATAW/2 - 1     ;
		l1e = l1 + ZDATAW - 1 ;
		l2  = l1 + ZAREAW     ;
		l3  = l2 + ZAREAW     ;
		m   = 0               ;
		sChanged[ ln ] = true ;
		if ( profVert )
		{
			if ( s2data.at( i ).ipos_hex == 1 ) { sChanged[ i+1 ] = false ; }
			for ( j = l2, k = l1 ; k <= l1e ; j++, k++ )
			{
				s = ZAREA[ j ] ;
				t = CAREA[ j ] ;
				if ( s == " " ) { s = t ; }
				if ( j+ZAREAW < ZASIZE )
				{
					s.push_back( ZAREA[ j+ZAREAW ] ) ;
					t.push_back( CAREA[ j+ZAREAW ] ) ;
					if ( s[ 1 ] == ' ' ) { s[ 1 ] = t[ 1 ] ; }
				}
				else
				{
					s.push_back( cs2xs( ZAREA[ k ] )[ 1 ] ) ;
					t.push_back( cs2xs( CAREA[ k ] )[ 1 ] ) ;
				}
				if ( !ishex( s ) ) { pcmd.set_msg( "PEDT011K" ) ; return  ; }
				if ( s == t ) { continue ; }
				ZAREA[ k ] = xs2cs( s )[ 0 ] ;
				m          = k - l1 ;
				rebuildZAREA = true ;
			}
		}
		else
		{
			if ( s2data.at( i ).ipos_hex == 1 ) { j = l2 ; k = l1    ; n = l1h ; }
			else                                { j = l3 ; k = l1h+1 ; n = l1e ; }
			for ( ; k <= n ; j=j+2, k++ )
			{
				s = ZAREA.substr( j, 2 ) ;
				t = CAREA.substr( j, 2 ) ;
				if ( s[ 0 ] == ' ' ) { s[ 0 ] = t[ 0 ] ; }
				if ( s[ 1 ] == ' ' ) { s[ 1 ] = t[ 1 ] ; }
				if ( !ishex( s ) ) { pcmd.set_msg( "PEDT011K" ) ; return  ; }
				if ( s == t ) { continue ; }
				ZAREA[ k ] = xs2cs( s )[ 0 ] ;
				m          = k - l1 ;
				rebuildZAREA = true ;
			}
		}
		n = ZSHADOW.find_last_not_of( 0xFF, l1e ) ;
		if ( n == string::npos || n < l1 )
		{
			n = lchar[ ln ] ;
		}
		else
		{
			n = n - l1 + 1  ;
			if ( n == ZDATAW ) { n = lchar[ ln ] ; }
		}
		if ( m >= n )
		{
			for ( j = n ; j < m ; j++ )
			{
				if ( ZAREA[ l1+j ] == 0x00 )
				{
					ZAREA[ l1+j ] = 0x20 ;
				}
			}
			n = m + 1 ;
		}
		ZSHADOW.replace( l1, n, n, 0x00 ) ;
		ZSHADOW.replace( l1+n, ZDATAW-n, ZDATAW-n, 0xFF ) ;
	}

	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		if ( s2data.at( i ).ipos_URID == 0 || s2data.at( i ).ipos_hex > 0 ) { continue ; }
		it = getLineItr( s2data.at( i ).ipos_dl ) ;
		if ( sTouched[ i ] )
		{
			if ( profCaps )
			{
				if ( (*it)->set_idata_upper( Level ) )
				{
					fileChanged  = true ;
					rebuildZAREA = true ;
				}
			}
			if ( !optPreserve )
			{
				if ( (*it)->set_idata_trim( Level ) )
				{
					fileChanged  = true ;
					rebuildZAREA = true ;
				}
			}
			ZAREA[ i * ZAREAW + 7 ] = din ;
			continue ;
		}
		if ( !sChanged[ i ] ) { continue ; }
		if (  (*it)->is_bnds() )
		{
			t = ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ) ;
			p = LeftBnd - startCol ;
			if ( p >= 0 && p < ZDATAW && t[ p ] == ' ' ) { LeftBnd  = 1 ; }
			p = RightBnd - startCol ;
			if ( p >= 0 && p < ZDATAW && t[ p ] == ' ' ) { RightBnd = 0 ; }
			p = 0 ;
			while ( true )
			{
				p = t.find( '<', p ) ;
				if ( p == string::npos ) { break ; }
				if ( p != string::npos && p != LeftBnd - startCol )
				{
					LeftBnd = startCol + p ;
					break ;
				}
				p++ ;
			}
			p = 0 ;
			while ( true )
			{
				p = t.find( '>', p ) ;
				if ( p == string::npos ) { break ; }
				if ( p != string::npos && p != RightBnd - startCol )
				{
					RightBnd = startCol + p ;
					break ;
				}
				p++ ;
			}
			if ( RightBnd <= LeftBnd ) { RightBnd = 0 ; }
		}
		else
		{
			(*it)->resetFileStatus() ;
			t = (*it)->get_idata()   ;
			s = ZSHADOW.substr( CLINESZ+(i*ZAREAW), ZDATAW ) ;
			d = countc( s, 0xFE ) ;
			k = s.find_last_not_of( "\xFE\xFF" ) ;
			s = ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ) ;
			if ( t.size() < startCol && k == string::npos ) { continue ; }
			if ( t.size() <= (startCol+ZDATAW-1) )
			{
				if ( k == string::npos ) { s = ""           ; }
				else                     { s.erase( k+1+d ) ; }
				t = substr( t, 1, startCol-1 ) + s ;
				if ( !optPreserve ) { trim_right( t ) ; }
			}
			else
			{
				t = substr( t, 1, startCol-1 ) + s + t.substr( ZDATAW+startCol-1 ) ;
				if ( d > 0 )
				{
					pcmd.set_msg( "PEDT014O", 4 ) ;
					t.erase( startCol-1+ZDATAW-d, d ) ;
				}
			}
			if ( profCaps ) { iupper( t ) ; }
			if ( (*it)->is_mask() )
			{
				maskLine = t ;
				(*it)->put_idata( maskLine ) ;
			}
			else if ( (*it)->is_tabs() )
			{
				for ( j = 0 ; j < t.size() ; j++ )
				{
					if ( t[ j ] != ' ' && t[ j ] != '*' && t[ j ] != '-' )
					{
						t[ j ] = ' ' ;
					}
				}
				tabsLine = t ;
				(*it)->put_idata( tabsLine ) ;
			}
			else if ( (*it)->is_isrt() )
			{
				(*it)->put_idata( t ) ;
			}
			else if ( (*it)->put_idata( t, Level ) )
			{
				fileChanged = true ;
			}
		}
		rebuildZAREA = true ;
	}
}


void PEDIT01::processNewInserts()
{
	// Remove new insert lines that have not been changed/touched (unless line command has been entered)
	// Add a new line below inserted lines when changed/touched and cursor is on the line still

	// s2data no longer valid after this routine as it changes the data vector
	// BUG: Still used in various places after this routine!!

	int i ;
	int k ;

	iline * p_iline ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	Level++ ;
	for ( i = ZAREAD-1 ; i >= 0 ; i-- )
	{
		if ( s2data.at( i ).ipos_URID == 0 || s2data.at( i ).ipos_hex > 0 ) { continue ; }
		it = getLineItr( s2data.at( i ).ipos_dl ) ;
		if ( !(*it)->is_isrt() ) { continue ; }
		if ( !sTouched[ i ] && !sChanged[ i ] )
		{
			if ( (*it)->il_lcc != "" ) { continue ; }
			delete *it ;
			it = data.erase( it ) ;
			it = getValidDataLineNext( it ) ;
			placeCursor( (*it)->il_URID, 3 ) ;
		}
		else
		{
			(*it)->convert_to_file( Level ) ;
			if ( (*it)->il_URID == aURID )
			{
				k = (*it)->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
				if ( k == string::npos )
				{
					itt = getNextFileLine( it ) ;
					if ( itt != data.end() )
					{
						k = (*itt)->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
					}
					if ( k == string::npos )
					{
						itt = getPrevFileLine( it ) ;
						if ( itt != data.end() )
						{
							k = (*itt)->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
							if ( k == string::npos ) { k = 0 ; }
						}
						else
						{
							k = 0 ;
						}
					}
				}
				p_iline = new iline( taskid() ) ;
				p_iline->il_type = LN_ISRT ;
				p_iline->put_idata( getMaskLine() ) ;
				p_iline->set_idata_minsize( k ) ;
				it = data.insert( ++it, p_iline ) ;
				if ( k != string::npos ) { placeCursor( p_iline->il_URID, 4, k ) ; }
				else                     { placeCursor( p_iline->il_URID, 2 )    ; }
			}
			fileChanged = true ;
		}
		rebuildZAREA = true ;
	}
}


void PEDIT01::actionPrimCommand1()
{
	// Action primary command.  These commands are executed before line command processing

	int i  ;
	int j  ;
	int ws ;
	int p1 ;

	uint dl ;

	string cmd   ;
	string fname ;
	string w     ;
	string w1    ;
	string w2    ;
	string w3    ;
	string w4    ;
	string lab1  ;
	string lab2  ;
	string wall  ;
	string t     ;

	queue<string> rlist ;
	vector<string> Info ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator its ;
	vector<iline * >::iterator ite ;

	vector<ipline> vip ;
	ipline ip ;

	map<P_CMDS,pcmd_format>::iterator itp ;

	if ( zverb == "RFIND" || zverb == "RCHANGE" ) { return ; }

	cmd = pcmd.get_cmd() ;
	ws  = pcmd.get_cmd_words() ;

	if ( ws == 0 ) { return ; }

	if ( pcmd.isLine_cmd() )
	{
		if ( aURID == 0 ) { pcmd.set_msg( "PEDT013X" ) ; return ; }
		dl = s2data.at( aRow-1 ).ipos_dl ;
		if ( data.at( dl )->il_lcc != "" ) { pcmd.set_msg( "PEDT013Y" ) ; return ; }
		data.at( dl )->il_lcc = cmd ;
		rebuildZAREA = true ;
		return ;
	}

	w1 = upper( word( cmd, 1 ) ) ;
	w2 = upper( word( cmd, 2 ) ) ;
	w3 = upper( word( cmd, 3 ) ) ;
	w4 = upper( word( cmd, 4 ) ) ;

	itp = PCMDform.find( pcmd.p_cmd ) ;

	if ( itp->second.noptions1 != -1 && ws < itp->second.noptions1 + 1 )
	{
		pcmd.set_msg( "PEDT015E" ) ;
		return ;
	}

	if ( itp->second.noptions2 != -1 && ws > itp->second.noptions2 + 1 )
	{
		pcmd.set_msg( "PEDT015D" ) ;
		return ;
	}

	switch ( pcmd.p_cmd )
	{
	case PC_BROWSE:
			p1 = ZFILE.find_last_of( '/' ) ;
			control( "ERRORS", "RETURN" ) ;
			browse( ZFILE.substr( 0, p1+1 ) + word( cmd, 2 ) ) ;
			control( "ERRORS", "CANCEL" ) ;
			break ;

	case PC_COPY:
			if ( pcmd.get_cmd_words() > 1 )
			{
				p1    = ZFILE.find_last_of( '/' ) ;
				fname = ZFILE.substr( 0, p1+1 ) + word( pcmd.get_cmd(), 2 ) ;
				if ( !exists( fname ) )
				{
					vreplace( "ZSTR1", fname ) ;
					pcmd.set_msg( "PEDT015K" ) ;
					break ;
				}
				pcmd.set_userdata( fname ) ;
			}
			copyActive = true ;
			break ;

	case PC_CREATE:
			if ( words( cmd ) != 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
			creFile = word( cmd, 2 ) ;
			if ( creFile.front() != '/' )
			{
				p1 = ZFILE.find_last_of( '/' ) ;
				if ( p1 != string::npos )
				{
					creFile = ZFILE.substr( 0, p1+1 ) + creFile ;
				}
			}
			if ( !findword( w1, "REPLACE REP REPL" ) && exists( creFile ) )
			{
				pcmd.set_msg( "PEDT014R" ) ;
				break ;
			}
			creActive = true ;
			break ;

	case PC_CUT:
			if ( w2 == "DISPLAY" )
			{
				manageClipboard() ;
				break ;
			}
			cutActive  = true ;
			cutReplace = profCutReplace ;
			clipBoard  = "DEFAULT" ;
			wall       = upper( subword( cmd, 2 ) ) ;

			p1 = wordpos( "DEFAULT", wall ) ;
			if ( p1 > 0 ) { idelword( wall, p1, 1 ) ; }

			p1 = wordpos( "APPEND", wall )  ;
			if ( p1 > 0 ) { cutReplace = false ; idelword( wall, p1, 1 ) ; }
			else
			{
				p1 = wordpos( "REPLACE", wall )  ;
				if ( p1 > 0 ) { idelword( wall, p1, 1 ) ; }
				else
				{
					p1 = wordpos( "R", wall )  ;
					if ( p1 > 0 ) { idelword( wall, p1, 1 ) ; }
				}
			}
			p1 = wordpos( "MARKED", wall ) ;
			if ( p1 > 0 )
			{
				idelword( wall, p1, 1 ) ;
				if ( wall != "" )
				{
					if ( words( wall ) == 1 ) { clipBoard = trim( wall )   ;         }
					else                      { pcmd.set_msg( "PEDT013A" ) ; break ; }
				}
				vip.clear() ;
				for ( its = data.begin() ; its != data.end() ; its++ )
				{
					if ( (*its)->isValidFile() && (*its)->il_mark )
					{
						ip.ip_type = LN_FILE ;
						ip.ip_data = (*its)->get_idata() ;
						vip.push_back( ip ) ;
					}
				}
				copyToClipboard( vip ) ;
				cutActive = false ;
				break ;
			}
			if ( wall != "" )
			{
				if ( words( wall ) == 1 ) { clipBoard = trim( wall )   ; }
				else                      { pcmd.set_msg( "PEDT013A" ) ; }
			}
			break ;

	case PC_EDIT:
			w2 = word( cmd, 2 ) ;
			if ( w2.front() == '/' )
			{
				edit( w2 ) ;
			}
			else
			{
				p1 = ZFILE.find_last_of( '/' ) ;
				control( "ERRORS", "RETURN" ) ;
				edit( ZFILE.substr( 0, p1+1 ) + w2 ) ;
				control( "ERRORS", "CANCEL" ) ;
			}
			break ;

	case PC_EDITSET:
			vreplace( "ZPOSFCX", profPosFcx     ? "/" : ""  ) ;
			vreplace( "ZCUTDEF", profCutReplace ? "2" : "1" ) ;
			vreplace( "ZPSTDEF", profPasteKeep  ? "2" : "1" ) ;
			addpop( "", 5, 5 ) ;
			while ( true )
			{
				display( "PEDIT019" ) ;
				if ( RC == 0 ) { continue ; }
				vcopy( "ZCMD9", t, MOVE ) ;
				if ( t != "CANCEL" )
				{
					vcopy( "ZPOSFCX", t, MOVE ) ;
					profPosFcx     = ( t == "/" ) ;
					vcopy( "ZCUTDEF", t, MOVE ) ;
					profCutReplace = ( t == "2" ) ;
					vcopy( "ZPSTDEF", t, MOVE ) ;
					profPasteKeep  = ( t == "2" ) ;
				}
				break ;
			}
			rempop() ;
			break ;

	case PC_PASTE:
			pasteActive = true  ;
			pasteKeep   = profPasteKeep ;
			clipBoard   = "DEFAULT" ;
			wall        = upper( subword( cmd, 2 ) ) ;

			p1 = wordpos( "DEFAULT", wall ) ;
			if ( p1 > 0 ) { idelword( wall, p1, 1 ) ; }

			p1 = wordpos( "KEEP", wall )  ;
			if ( p1 > 0 ) { pasteKeep = true ; idelword( wall, p1, 1 ) ; }
			else
			{
				p1 = wordpos( "DELETE", wall )  ;
				if ( p1 > 0 ) { idelword( wall, p1, 1 ) ; }
			}

			if ( wall != "" )
			{
				if ( words( wall ) == 1 ) { clipBoard = wall          ; }
				else                      { pcmd.set_msg( "PEDT013B" ) ; }
			}
			break ;

	case PC_RESET:
			pcmd.setActioned() ;
			wall = upper( cmd ) ;
			for ( j = 0, i = 2 ; i <= ws ; i++ )
			{
				w = word( wall, i ) ;
				if ( w.front() == '.' )
				{
					( ++j == 1 ) ? lab1 = w : lab2 = w ;
				}
				else
				{
					if ( aliasNames.count( w ) > 0 )
					{
						w = aliasNames[ w ] ;
					}
					if ( !findword( w, "ALL CLEAN CHA CMD ERR FIND LAB H UNDO REDO SPE T X" ) )
					{
						     pcmd.set_msg( "PEDT011" ) ;
						     return ;
					}
					rlist.push( w ) ;
				}
			}
			if ( j == 0 )
			{
				its = data.begin() ;
				ite = data.end()   ;
			}
			else if ( j == 2 )
			{
				if ( !getLabelItr( lab1, its, i ) || !getLabelItr( lab2, ite, j ) )
				{
					pcmd.set_msg( "PEDT011F" ) ;
					break ;
				}
				if ( i > j )
				{
					swap( its, ite ) ;
				}
				ite++ ;
			}
			else
			{
				pcmd.set_msg( "PEDT014U" ) ;
				break ;
			}
			if ( rlist.empty() )
			{
				removeSpecialLines( its, ite ) ;
				for_each( its, ite,
					[](iline *& a)
					{
						a->resetFilePrefix() ;
						a->clearLcc() ;
					} ) ;
				hideExcl      = false ;
				optFindPhrase = false ;
				lcmds.clear() ;
			}
			it = its ;
			while ( !rlist.empty() )
			{
				its = it ;
				w   = rlist.front() ;
				rlist.pop()         ;
				if ( w == "ALL" )
				{
					removeSpecialLines( its, ite ) ;
					for_each( its, ite,
						[](iline *& a)
						{
							a->resetFilePrefix() ;
							a->clearLcc() ;
							a->clearLabel() ;
						} ) ;
					hideExcl = false ;
				}
				else if ( w2 == "CLEAN" )
				{
					removeSpecialLines( its, ite ) ;
					for_each( its, ite,
						[](iline *& a)
						{
							a->resetFilePrefix() ;
							a->clearLcc() ;
						} ) ;
					lcmds.clear() ;
					cleanupData() ;
					break         ;
				}
				else if ( w == "CMD" )
				{
					for_each( its, ite,
						[](iline *& a)
						{
							a->clearLcc() ;
						} ) ;
				}
				else if ( w == "CHA" )
				{
					for_each( its, ite,
						[](iline *& a)
						{
							a->clrChngStatus() ;
						} ) ;
				}
				else if ( w == "ERR" )
				{
					for_each( its, ite,
						[](iline *& a)
						{
							a->clrErrorStatus() ;
						} ) ;
				}
				else if ( w == "LAB" )
				{
					for_each( its, ite,
						[](iline *& a)
						{
							a->clearLabel() ;
						} ) ;
				}
				else if ( w == "FIND" )
				{
					optFindPhrase = false ;
				}
				else if ( w == "H" )
				{
					hideExcl = false ;
				}
				else if ( w == "T" )
				{
					for_each( its, ite,
						[](iline *& a)
						{
							a->resetMarked() ;
						} ) ;
				}
				else if ( w == "UNDO" )
				{
					for_each( its, ite,
						[](iline *& a)
						{
							a->clrUndoStatus() ;
						} ) ;
				}
				else if ( w == "REDO" )
				{
					for_each( its, ite,
						[](iline *& a)
						{
							a->clrRedoStatus() ;
						} ) ;
				}
				else if ( w == "SPE" )
				{
					removeSpecialLines( its, ite ) ;
				}
				else if ( w == "X" )
				{
					for_each( its, ite,
						[](iline *& a)
						{
							a->il_excl = false ;
						} ) ;
				}
			}
			rebuildZAREA = true ;
			break ;

	default:
			break ;
	}
}


void PEDIT01::actionPrimCommand2()
{
	// Action primary command.  These commands are executed after line command processing

	int i  ;
	int j  ;
	int ws ;
	int p1 ;
	int p2 ;
	int tCol ;
	int tLb  ;
	int tRb  ;
	int lin1 ;
	int lin2 ;

	uint tTop ;

	char locDir ;

	string cmd  ;
	string t1   ;
	string t2   ;
	string w    ;
	string w1   ;
	string w2   ;
	string w3   ;
	string w4   ;
	string wall ;
	string lab1 ;
	string lab2 ;

	bool firstc ;

	bool delAll ;
	bool delX   ;
	bool delNX  ;
	bool delln1 ;
	bool delln2 ;

	defName dn  ;
	D_PARMS dp  ;

	cmd_range r ;

	vector<string> Prof  ;
	vector<string> Info  ;
	vector<string> Msgs  ;
	vector<string> tdata ;

	vector<bool>srt_asc ;

	vector<iline *>::iterator it  ;
	vector<iline *>::iterator its ;
	vector<iline *>::iterator ite ;

	map<string,stack<defName>>::iterator ita ;
	map<string,stack<defName>>::iterator itb ;

	if ( zverb == "RFIND" || zverb == "RCHANGE" ) { return ; }
	if ( pcmd.isMacro() ) { return ; }

	cmd = pcmd.get_cmd() ;
	ws  = pcmd.get_cmd_words() ;

	if ( ws == 0 ) { return ; }

	w1 = upper( word( cmd, 1 ) ) ;
	w2 = upper( word( cmd, 2 ) ) ;
	w3 = upper( word( cmd, 3 ) ) ;
	w4 = upper( word( cmd, 4 ) ) ;

	switch ( pcmd.p_cmd )
	{
	case PC_AUTOSAVE:
			pcmd.setActioned() ;
			if ( w2 == "ON" || w2 == "" )
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT015D" ) ; break ; }
				profASave = true ;
			}
			else
			{
				if ( w2 == "OFF" ) { w2 = w3 ; }
				else if ( ws > 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				if ( w2 == "PROMPT" || w2 == "" )
				{
					profSaveP = true ;
				}
				else if ( w2 == "NOPROMPT" )
				{
					profSaveP = false ;
					pcmd.set_msg( "PEDT014I", 4 ) ;
				}
				else
				{
					pcmd.set_msg( "PEDT011" ) ;
					break ;
				}
				profASave = false ;
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_BOUNDS:
			if      ( w2 == "*" )           { tLb = LeftBnd     ; }
			else if ( w2 == ""  )           { tLb = 1 ; tRb = 0 ; }
			else if ( datatype( w2, 'W' ) ) { tLb = ds2d( w2 )  ; }
			else    { pcmd.set_msg( "PEDT011" ) ; break ; }
			if      ( w3 == "*" )           { tRb = RightBnd    ; }
			else if ( w3 == ""  )           { tRb = 0           ; }
			else if ( datatype( w3, 'W' ) ) { tRb = ds2d( w3 )  ; }
			else    { pcmd.set_msg( "PEDT011" ) ; break ; }
			if ( tRb > 0 && tLb >= tRb ) { pcmd.set_msg( "PEDT011" ) ; break ; }
			LeftBnd  = tLb ;
			RightBnd = tRb ;
			rebuildZAREA = true ;
			break ;

	case PC_CAPS:
			pcmd.setActioned() ;
			if      ( w2 == "ON" || ws == 1 ) { profCaps = true  ; }
			else if ( w2 == "OFF" )           { profCaps = false ; }
			else { pcmd.set_msg( "PEDT011" ) ; break ; }
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_CHANGE:
			pcmd.setActioned() ;
			if ( !setFindChangeExcl( 'C' ) ) { break ; }
			Level++ ;
			tTop   = topLine  ;
			tCol   = startCol ;
			firstc = true     ;
			i      = 0        ;
			fcx_parms.f_ch_occs = 0 ;
			fcx_parms.f_ch_errs = 0 ;
			while ( true )
			{
				actionFind() ;
				if ( fcx_parms.f_error || !fcx_parms.f_success ) { break  ; }
				i++ ;
				if ( firstc )
				{
					moveColumn( fcx_parms.f_offset ) ;
					tCol = startCol                  ;
					if ( fcx_parms.findReverse() )
					{
						placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset ) ;
					}
					else
					{
						placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset + fcx_parms.f_cstring.size() ) ;
					}
					moveTopline( fcx_parms.f_URID ) ;
					tTop = topLine ;
					if ( macroRunning )
					{
						mRow = getFileLine( fcx_parms.f_dl ) + 1 ;
						mCol = fcx_parms.f_offset + 1 ;
						miBlock.setcursor = true ;
					}
					firstc = false ;
				}
				actionChange() ;
				if ( !fcx_parms.f_chngall ) { break  ; }
				startCol = 1 ;
				topLine  = getLine( fcx_parms.f_URID ) ;
				aCol     = fcx_parms.f_offset + fcx_parms.f_cstring.size() + CLINESZ ;
			}
			topLine  = tTop ;
			startCol = tCol ;
			fcx_parms.f_ch_occs = i ;
			( i > 0 ) ? setChangedMsg() : setNotFoundMsg() ;
			rebuildZAREA = true ;
			break ;

	case PC_COLUMN:
			if      ( w2 == "ON"  ) { colsOn = true    ; }
			else if ( w2 == "OFF" ) { colsOn = false   ; }
			else if ( w2 == ""    ) { colsOn = !colsOn ; }
			else                    { pcmd.set_msg( "PEDT011" ) ; break ; }
			rebuildZAREA = true ;
			break ;

	case PC_COMPARE:
			compareFiles( subword( cmd, 2 ) ) ;
			break ;

	case PC_DEFINE:
			pcmd.setActioned() ;
			if ( DefParms.count( w3 ) == 0 )
			{
				pcmd.set_msg( "PEDT015M", 20 ) ;
				break ;
			}
			if ( findword( w3, "NOP RESET DISABLED" ) && ( w4 != "" ) )
			{
				pcmd.set_msg( "PEDT015O", 20 ) ;
				break ;
			}
			ita = defNames.find( w2 ) ;
			if ( ita == defNames.end() && PrimCMDS.count( w2 ) > 0 )
			{
				ita = defNames.find( PrimCMDS[ w2 ].truename ) ;
			}
			if ( ita != defNames.end() )
			{
				if ( ita->second.top().disabled )
				{
					pcmd.set_msg( "PEDT015P" ) ;
					break ;
				}
				else if ( ita->second.top().alias )
				{
					itb = defNames.find( ita->second.top().name ) ;
					if ( itb != defNames.end() && itb->second.top().disabled )
					{
						pcmd.set_msg( "PEDT015P" ) ;
						break ;
					}
				}
			}
			dp = DefParms[ w3 ] ;
			switch ( dp )
			{
			case DF_MACRO:
				if ( w4 != "CMD" && w4 != "" )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				dn.cmd = true ;
				defNames[ w2 ].push( dn ) ;
				break ;

			case DF_ALIAS:
				if ( w4 == "" )
				{
					pcmd.set_msg( "PEDT015N", 20 ) ;
					break ;
				}
				if ( ita != defNames.end() )
				{
					if ( ita->second.top().deactive() )
					{
						pcmd.set_msg( "PEDT015U", 8 ) ;
						break ;
					}
					else if ( ita->second.top().alias )
					{
						w4  = ita->second.top().name ;
					}
				}
				if ( PrimCMDS.count( w4 ) > 0 ) { w4 = PrimCMDS[ w4 ].truename ; }
				itb = defNames.find( w4 ) ;
				if ( itb != defNames.end() && itb->second.top().deactive() )
				{
					pcmd.set_msg( "PEDT015U", 8 ) ;
					break ;
				}
				dn.name  = w4 ;
				dn.alias = true ;
				defNames[ w2 ].push( dn ) ;
				break ;

			case DF_NOP:
			case DF_DISABLED:
				if ( ita == defNames.end() )
				{
					if ( PrimCMDS.count( w2 ) == 0 )
					{
						pcmd.set_msg( "PEDT015Q" ) ;
						break ;
					}
					dp == DF_NOP ? dn.nop = true : dn.disabled = true ;
					defNames[ w2 ].push( dn ) ;
				}
				else if ( ita->second.top().alias )
				{
					dp == DF_NOP ? dn.nop = true : dn.disabled = true ;
					defNames[ ita->second.top().name ].push( dn ) ;
				}
				else
				{
					dp == DF_NOP ? dn.nop = true : dn.disabled = true ;
					defNames[ w2 ].push( dn ) ;
				}
				break ;

			case DF_RESET:
				if ( ita == defNames.end() )
				{
					pcmd.set_msg( "PEDT015T", 8 ) ;
				}
				else if ( ita->second.top().alias )
				{
					itb = defNames.find( ita->second.top().name ) ;
					if ( itb != defNames.end() && itb->second.top().nop )
					{
						if ( itb->second.size() == 1 )
						{
							defNames.erase( itb ) ;
						}
						else
						{
							itb->second.pop() ;
							if ( itb->second.top().nop ) { pcmd.setRC( 4 ) ; }
						}
					}
					else
					{
						if ( ita->second.size() == 1 ) { defNames.erase( ita ) ; }
						else                           { ita->second.pop()     ; }
					}
				}
				else if ( ita->second.size() == 1 )
				{
					defNames.erase( ita ) ;
				}
				else if ( ita->second.top().nop )
				{
					ita->second.pop() ;
					if ( ita->second.top().nop ) { pcmd.setRC( 4 ) ; }
				}
				else
				{
					ita->second.pop() ;
				}
			}
			break ;

	case PC_DELETE:
			pcmd.setActioned() ;
			if ( macroRunning && getLabelIndex( ".ZFIRST" ) == -2 )
			{
				pcmd.setRC( 8 ) ;
				break ;
			}
			delAll = false ;
			delX   = false ;
			delNX  = false ;
			delln1 = false ;
			delln2 = false ;
			wall   = upper( cmd ) ;
			for ( j = 0, i = 2 ; i <= ws ; i++ )
			{
				w = word( wall, i ) ;
				if ( macroRunning )
				{
					if ( datatype( w, 'W' ) )
					{
						if      ( !delln1 ) { lin1 = ds2d ( w ) ; delln1 = true ; }
						else if ( !delln2 ) { lin2 = ds2d ( w ) ; delln2 = true ; }
						else
						{
							pcmd.set_msg( "PEDT011", 20 ) ;
							break ;
						}
						continue ;
					}
				}
				if ( w.front() == '.' )
				{
					( ++j == 1 ) ? lab1 = w : lab2 = w ;
				}
				else
				{
					if ( aliasNames.count( w ) > 0 )
					{
						w = aliasNames[ w ] ;
					}
					if      ( w == "ALL" && !delAll         ) { delAll = true ; }
					else if ( w == "X"   && !delX && !delNX ) { delX   = true ; }
					else if ( w == "NX"  && !delX && !delNX ) { delNX  = true ; }
					else
					{
						     pcmd.set_msg( "PEDT011", 20 ) ;
						     break ;
					}
				}
			}
			if ( macroRunning )
			{
				delAll = true ;
				if ( delln1 && ( !delX && !delNX ) )
				{
					delX  = true ;
					delNX = true ;
				}
			}
			if ( (j == 0  && !delAll) ||
				   (j == 0  &&  delAll && ( !delX && !delNX) ) ||
				   (j >  0  &&  delln1) ||
				   (!delAll && !delX   && !delNX ) )
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			if ( j > 0 && ( !delX && !delNX ) )
			{
				delX  = true ;
				delNX = true ;
			}
			if ( j == 0 )
			{
				if ( delln1 )
				{
					if ( delln2 && (lin1 > lin2) ) { swap( lin1, lin2 ) ; }
					its = getLineItr( getDataLine( lin1 ) ) ;
					if ( delln2 )
					{
						ite = getLineItr( getDataLine( lin2 ) ) ;
					}
					else
					{
						ite = its ;
					}
					if ( (*its)->is_bod() || (*ite)->is_bod() )
					{
						pcmd.set_msg( "PEDT015G" ) ;
						break ;
					}
					ite++ ;
				}
				else
				{
					its = data.begin() ;
					ite = data.end()   ;
				}
			}
			else if ( j == 1 && macroRunning )
			{
				if ( !getLabelItr( lab1, its, i ) )
				{
					pcmd.set_msg( "PEDT011F", 20 ) ;
					break ;
				}
				ite = its ;
				ite++     ;
			}
			else if ( j == 2 )
			{
				if ( !getLabelItr( lab1, its, i ) || !getLabelItr( lab2, ite, j ) )
				{
					pcmd.set_msg( "PEDT011F", 20 ) ;
					break ;
				}
				if ( i > j )
				{
					swap( its, ite ) ;
				}
				it = its ;
				ite++    ;
			}
			else
			{
				pcmd.set_msg( "PEDT014U", 20 ) ;
				break ;
			}
			Level++ ;
			for ( i = 0, it = its ; it != ite ; it++ )
			{
				if ( (*it)->il_deleted || (*it)->is_tod() )  { continue ; }
				if ( (*it)->is_bod() )  { break    ; }
				if ( delX && (*it)->il_excl )
				{
					if ( (*it)->isValidFile() ) { fileChanged = true ; }
					(*it)->set_deleted( Level ) ;
				}
				else if ( delNX && !(*it)->il_excl )
				{
					if ( (*it)->isValidFile() ) { fileChanged = true ; }
					(*it)->set_deleted( Level ) ;
				}
				else
				{
					continue ;
				}
				i++ ;
			}
			( i == 0 ) ? Level-- : rebuildZAREA = true ;
			LINES = d2ds( i )  ;
			i > 0 ? pcmd.set_msg( "PEDT011M", 0 ) : pcmd.set_msg( "PEDT011M", 4 ) ;
			break ;

	case PC_EXCLUDE:
			pcmd.setActioned() ;
			if ( w2 == "ALL" && ws == 2 )
			{
				for_each( data.begin(), data.end(),
					[](iline *& a)
					{
						if ( !a->is_bod() && !a->is_tod() && !a->il_deleted )
						{
							a->il_excl = true ;
						}
					} ) ;
				topLine      = 0    ;
				rebuildZAREA = true ;
				break               ;
			}
			if ( !setFindChangeExcl( 'X' ) ) { break ; }
			fcx_parms.f_excl = 'N' ;
			actionFind() ;
			if ( fcx_parms.f_error ) { break ; }
			if ( fcx_parms.f_success )
			{
				moveColumn( fcx_parms.f_offset ) ;
				placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset ) ;
				moveTopline( fcx_parms.f_URID ) ;
				fcx_parms.f_occurs  = 1 ;
				fcx_parms.f_lines   = 1 ;
				setExcludedMsg() ;
				fcx_parms.f_ex_occs = 1 ;
				fcx_parms.f_ex_lnes = 1 ;
				if ( macroRunning )
				{
					mRow = getFileLine( fcx_parms.f_dl ) + 1 ;
					mCol = fcx_parms.f_offset + 1 ;
					miBlock.setcursor = true ;
				}
			}
			else if ( fcx_parms.f_occurs > 0 && fcx_parms.f_dir == 'A' )
			{
				fcx_parms.f_ex_occs = fcx_parms.f_occurs ;
				fcx_parms.f_ex_lnes = fcx_parms.f_lines  ;
				moveColumn( fcx_parms.f_offset ) ;
				placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset ) ;
				moveTopline( fcx_parms.f_URID ) ;
				setExcludedMsg() ;
				if ( macroRunning )
				{
					mRow = getFileLine( fcx_parms.f_dl ) + 1 ;
					mCol = fcx_parms.f_offset + 1 ;
					miBlock.setcursor = true ;
				}
			}
			else
			{
				setNotFoundMsg() ;
				fcx_parms.f_ex_occs = 0 ;
				fcx_parms.f_ex_lnes = 0 ;
			}
			rebuildZAREA = true ;
			break ;

	case PC_FIND:
			pcmd.setActioned() ;
			if ( !setFindChangeExcl( 'F' ) ) { break ; }
			actionFind()       ;
			if ( fcx_parms.f_error ) { break ; }
			if ( pcmd.msgset() ) { break ; }
			if ( fcx_parms.f_success )
			{
				moveColumn( fcx_parms.f_offset ) ;
				placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset ) ;
				moveTopline( fcx_parms.f_URID ) ;
				setFoundMsg() ;
				if ( pcmd.isSeek() )
				{
					fcx_parms.f_sk_occs = 1 ;
					fcx_parms.f_sk_lnes = 1 ;
				}
				else
				{
					fcx_parms.f_fd_occs = 1 ;
					fcx_parms.f_fd_lnes = 1 ;
				}
				if ( macroRunning )
				{
					mRow = getFileLine( fcx_parms.f_dl ) + 1 ;
					mCol = fcx_parms.f_offset + 1 ;
					miBlock.setcursor = true ;
				}
			}
			else if ( fcx_parms.f_occurs > 0 && fcx_parms.f_dir == 'A' )
			{
				if ( pcmd.isSeek() )
				{
					fcx_parms.f_sk_occs = fcx_parms.f_occurs ;
					fcx_parms.f_sk_lnes = fcx_parms.f_lines  ;
				}
				else
				{
					fcx_parms.f_fd_occs = fcx_parms.f_occurs ;
					fcx_parms.f_fd_lnes = fcx_parms.f_lines  ;
				}
				moveColumn( fcx_parms.f_offset ) ;
				placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset ) ;
				moveTopline( fcx_parms.f_URID ) ;
				setFoundMsg() ;
				fcx_parms.f_dir     = 'N'  ;
				fcx_parms.f_success = true ;
				if ( macroRunning )
				{
					mRow = getFileLine( fcx_parms.f_dl ) + 1 ;
					mCol = fcx_parms.f_offset + 1 ;
					miBlock.setcursor = true ;
				}
			}
			else
			{
				setNotFoundMsg() ;
				if ( pcmd.isSeek() )
				{
					fcx_parms.f_sk_occs = 0 ;
					fcx_parms.f_sk_lnes = 0 ;
				}
				else
				{
					fcx_parms.f_fd_occs = 0 ;
					fcx_parms.f_fd_lnes = 0 ;
				}
			}
			rebuildZAREA = true ;
			break ;

	case PC_FLIP:
			pcmd.setActioned() ;
			wall = upper( cmd ) ;
			for ( j = 0, i = 2 ; i <= ws ; i++ )
			{
				w = word( wall, i ) ;
				if ( w.front() != '.' ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				( ++j == 1 ) ? lab1 = w : lab2 = w ;
			}
			if ( j == 0 )
			{
				its = data.begin() ;
				ite = data.end()   ;
			}
			else if ( j == 2 )
			{
				if ( !getLabelItr( lab1, its, i ) || !getLabelItr( lab2, ite, j ) )
				{
					pcmd.set_msg( "PEDT011F" ) ;
					break ;
				}
				if ( i > j )
				{
					swap( its, ite ) ;
				}
				ite++ ;
			}
			else
			{
				pcmd.set_msg( "PEDT014U" ) ;
				break ;
			}
			for_each( its, ite,
				[](iline *& a)
				{
					if ( !a->is_bod() && !a->is_tod() && !a->il_deleted )
					{
						a->il_excl = !a->il_excl ;
					}
				} ) ;
			rebuildZAREA = true ;
			break ;

	case PC_HEX:
			pcmd.setActioned() ;
			if ( w2 == "OFF" )
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT015D" ) ; break ; }
				profHex  = false ;
				profVert = true  ;
				for_each( data.begin(), data.end(),
					[](iline *& a)
					{
						a->il_hex = false ;
					} ) ;
				rebuildZAREA = true ;
			}
			else
			{
				if ( w2 == "ON" )  { w2 = w3                  ; }
				else if ( ws > 2 ) { pcmd.set_msg( "PEDT015D" ) ; break ; }
				if ( w2 == "VERT" || w2 == "" )
				{
					profVert = true ;
				}
				else if ( w2 == "DATA" )
				{
					profVert = false ;
				}
				else { pcmd.set_msg( "PEDT011" ) ; break ; }
				profHex      = true ;
				rebuildZAREA = true ;
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_HIDE:
			pcmd.setActioned() ;
			if ( aliasNames.count( w2 ) > 0 )
			{
				w2 = aliasNames[ w2 ] ;
			}
			if ( w2 != "X" )
			{
				pcmd.set_msg( "PEDT011" ) ;
				break ;
			}
			hideExcl     = true ;
			rebuildZAREA = true ;
			break ;

	case PC_HILIGHT:
			if ( w2 == "" )
			{
				vreplace( "ZPROFLG", profLang ) ;
				vreplace( "ZPROFHI", profHilight    ? "YES" : "NO" ) ;
				vreplace( "ZPROFIF", profIfLogic    ? "YES" : "NO" ) ;
				vreplace( "ZPROFDO", profDoLogic    ? "YES" : "NO" ) ;
				vreplace( "ZPARMTC", profParen      ? "/" : ""  ) ;
				vreplace( "ZHIFIND", profFindPhrase ? "/" : ""  ) ;
				vreplace( "ZHICURS", profCsrPhrase  ? "/" : ""  ) ;
				addpop( "", 5, 5 ) ;
				while ( true )
				{
					display( "PEDIT01A" ) ;
					if ( RC == 0 ) { continue ; }
					vcopy( "ZCMDA", t1, MOVE ) ;
					if ( t1 != "CANCEL" )
					{
						vcopy( "ZPROFLG", profLang, MOVE ) ;
						vcopy( "ZPROFHI", t1, MOVE )  ;
						profHilight = ( t1 == "YES" ) ;
						vcopy( "ZPROFIF", t1, MOVE )  ;
						profIfLogic = ( t1 == "YES" ) ;
						vcopy( "ZPROFDO", t1, MOVE )  ;
						profDoLogic = ( t1 == "YES" ) ;
						vcopy( "ZPARMTC", t1, MOVE )  ;
						profParen   = ( t1 == "/" )    ;
						vcopy( "ZHIFIND", t1, MOVE )   ;
						profFindPhrase = ( t1 == "/" ) ;
						vcopy( "ZHICURS", t1, MOVE )   ;
						profCsrPhrase = ( t1 == "/" )  ;
						optFindPhrase = profFindPhrase ;
						buildProfLines( Prof )  ;
						updateProfLines( Prof ) ;
						detLang = ( profLang != "AUTO" ) ? profLang : determineLang() ;
						clr_hilight_shadow()    ;
						rebuildZAREA   = true   ;
					}
					break ;
				}
				rempop() ;
				break    ;
			}
			if ( w2 == "RESET" )
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				profIfLogic  = false  ;
				profDoLogic  = false  ;
				profLang     = "AUTO" ;
				detLang      = determineLang() ;
				rebuildZAREA = true     ;
				buildProfLines( Prof )  ;
				updateProfLines( Prof ) ;
				break ;
			}
			t1 = upper( cmd ) ;
			p1 = wordpos( "PAREN", t1 ) ;
			if ( p1 > 0 )
			{
				idelword( t1, p1, 1 ) ;
				ws = words( t1 )   ;
				w2 = word( t1, 2 ) ;
				w3 = word( t1, 3 ) ;
			}
			if ( aliasNames.find( w2 ) != aliasNames.end() )
			{
				w2 = aliasNames[ w2 ] ;
			}
			if ( findword( w2, "ON OFF LOGIC NOLOGIC IFLOGIC DOLOGIC" ) )
			{
				if ( ws > 3 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				if ( w3 != "" && w3 != "AUTO" && !addHilight( w3 ) ) { pcmd.set_msg( "PEDT013Q" ) ; break ; }
			}
			else
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				if ( w2 != "" && w2 != "AUTO" && !addHilight( w2 ) ) { pcmd.set_msg( "PEDT013Q" ) ; break ; }
				w3 = w2   ;
				w2 = "ON" ;
			}
			if ( w2 == "OFF" )
			{
				profHilight  = false ;
				if ( p1 > 0 ) { profParen = !profParen ; }
				rebuildZAREA = true  ;
				buildProfLines( Prof )  ;
				updateProfLines( Prof ) ;
				break ;
			}
			profIfLogic = ( w2 == "IFLOGIC" || w2 == "LOGIC" ) ;
			profDoLogic = ( w2 == "DOLOGIC" || w2 == "LOGIC" ) ;
			if ( w3 != "" && w3 != "AUTO" )
			{
				profLang = w3 ;
				detLang  = w3 ;
			}
			else
			{
				profLang = "AUTO" ;
				detLang  = determineLang() ;
			}
			clr_hilight_shadow() ;
			if ( p1 > 0 ) { profParen = !profParen ; }
			profHilight  = true  ;
			rebuildZAREA = true  ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_LOCATE:
			pcmd.setActioned() ;
			if ( macroRunning && getLabelIndex( ".ZFIRST" ) == -2 )
			{
				pcmd.setRC( 8 ) ;
				break ;
			}
			tTop = topLine ;
			if ( ws == 2 )
			{
				if ( w2.front() == '.' )
				{
					p1 = getLabelIndex( w2 ) ;
					if      ( p1 == -1 ) { pcmd.set_msg( "PEDT011F", 4 ) ; break ; }
					else if ( p1 == -2 ) { pcmd.set_msg( "PEDT011F", 4 ) ; break ; }
					topLine      = p1   ;
					data.at( p1 )->il_excl = false ;
					ptopLine     = tTop ;
					rebuildZAREA = true ;
					break ;
				}
				else if ( w2.size() < 9 && datatype( w2, 'W' ) )
				{
					if ( w2 == "0" )
					{
						topLine = 0 ;
					}
					else
					{
						p1      = ds2d( w2 )        ;
						topLine = getDataLine( p1 ) ;
					}
					ptopLine     = tTop ;
					rebuildZAREA = true ;
					break ;
				}
				else if ( w2 == "*" )
				{
					topLine = ptopLine  ;
					ptopLine     = tTop ;
					rebuildZAREA = true ;
					break ;
				}
			}
			locDir = ' ' ;
			w1     = ""  ;
			for ( j = 0, i = 2 ; i <= ws ; i++ )
			{
				w = upper( word( cmd, i ) ) ;
				if ( w.front() != '.' )
				{
					if ( findword( w, "FIRST LAST NEXT PREV" ) )
					{
						if ( locDir != ' ' )
						{
							pcmd.set_msg( "PEDT011" ) ;
							break ;
						}
						locDir = w.front() ;
					}
					else if ( w1 != "" )
					{
						pcmd.set_msg( "PEDT011" ) ;
						break ;
					}
					else
					{
						w1 = w ;
					}
					continue ;
				}
				( ++j == 1 ) ? lab1 = w : lab2 = w ;
			}
			if ( j == 0 )
			{
				p1 = 0 ;
				p2 = data.size() - 2 ;
			}
			else if ( j == 2 )
			{
				p1 = getLabelIndex( lab1 ) ;
				p2 = getLabelIndex( lab2 ) ;
				if ( p1 < 0 || p2 < 0 )
				{
					pcmd.set_msg( "PEDT011F" ) ;
					break ;
				}
				if ( p1 > p2 )
				{
					swap( p1, p2 ) ;
				}
			}
			else
			{
				pcmd.set_msg( "PEDT014U" ) ;
				break ;
			}
			if ( aliasNames.count( w1 ) > 0 )
			{
				w1 = aliasNames[ w1 ] ;
			}
			if ( findword( w1, "SPE CHA ERR X INFO LAB MSG NOTE T" ) )
			{
				topLine = getNextSpecial( p1, p2, locDir, w1.front() ) ;
			}
			else if ( w1 == "CMD" )
			{
				topLine = getNextSpecial( p1, p2, locDir, 'K' ) ;
			}
			else if ( findword( w1, "UNDO REDO" ) )
			{
				topLine = getNextSpecial( p1, p2, locDir, 'U' ) ;
			}
			else
			{
				pcmd.set_msg( "PEDT011" ) ;
				break ;
			}
			rebuildZAREA = true ;
			ptopLine     = tTop ;
			break ;

	case PC_IMACRO:
			pcmd.setActioned() ;
			if ( profLock )
			{
				pcmd.set_msg( "PEDT015H", 4 ) ;
			}
			else if ( !isvalidName( w2 ) )
			{
				pcmd.set_msg( "PEDT011" ) ;
			}
			else
			{
				profIMAC = w2 ;
				buildProfLines( Prof )  ;
				updateProfLines( Prof ) ;
			}
			break ;

	case PC_NULLS:
			pcmd.setActioned() ;
			if ( w2 == "OFF" )
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				profNullA = false ;
				profNulls = false ;
			}
			else
			{
				if ( w2 == "ON" )  { w2 = w3                  ; }
				else if ( ws > 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				if ( w2 == "STD" || w2 == "" )
				{
					profNulls = true  ;
					profNullA = false ;
				}
				else if ( w2 == "ALL" )
				{
					profNulls = true  ;
					profNullA = true  ;
				}
				else { pcmd.set_msg( "PEDT011" ) ; break ; }
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			rebuildZAREA = true ;
			break ;

	case PC_PRESERVE:
			if ( w2 == "ON" || w2 == "" )
			{
				optPreserve = true ;
			}
			else if ( w2 == "OFF" )
			{
				optPreserve = false ;
			}
			else { pcmd.set_msg( "PEDT011" ) ; break ; }
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_PROFILE:
			pcmd.setActioned() ;
			if ( ws == 1 )
			{
				removeProfLines() ;
				buildProfLines( Prof ) ;
				addSpecial( 'P', topLine, Prof ) ;
				rebuildZAREA = true  ;
			}
			else if ( ws == 3 && w2 == "USE" && w3 == "TYPE" )
			{
				saveEditProfile( ZEDPROF ) ;
				ZEDPROF = determineLang()  ;
				getEditProfile( ZEDPROF )  ;
				clr_hilight_shadow()       ;
				rebuildZAREA = true        ;
				ZEDPROFT = "Y"             ;
				vput( "ZEDPROFT", PROFILE) ;
			}
			else if ( ws == 3 && findword( w2, "DEL DELETE" ) )
			{
				delEditProfile( w3 ) ;
				break ;
			}
			else if ( ws != 2 )           { pcmd.set_msg( "PEDT011" ) ; break ; }
			else if ( w2 == "LOCK"   )    { saveEditProfile( ZEDPROF ) ; profLock = true ; }
			else if ( w2 == "UNLOCK" )    { profLock = false         ; }
			else if ( w2 == "RESET"  )    { resetEditProfile()       ; }
			else if ( isvalidName( w2 ) )
			{
				saveEditProfile( ZEDPROF ) ;
				getEditProfile( w2 ) ;
				clr_hilight_shadow() ;
				rebuildZAREA = true  ;
				ZEDPROFT = "N"       ;
				vput( "ZEDPROFT", PROFILE) ;
			}
			else    { pcmd.set_msg( "PEDT011" ) ; break ; }
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_RECOVERY:
			if ( w2 == "OFF" )
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				profRecover  = false ;
			}
			else
			{
				if ( ws > 4 ) { pcmd.set_msg( "PEDT011" ) ; break      ; }
				if ( w2 == "ON" )  { w2 = w3 ; w3 = word( cmd, 4 )     ; }
				else if ( ws > 3 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				else               { w3  = word( cmd, 3 )              ; }
				if ( w2 == "" )
				{
					profRecover = true ;
				}
				else if ( w2 == "PATH" )
				{
					if ( w3 == "" ) { pcmd.set_msg( "PEDT011" ) ; break ; }
					recoverLoc  = w3 ;
					profRecover = true ;
					if ( recoverLoc.back() != '/' ) { recoverLoc += '/' ; }
				}
				else { pcmd.set_msg( "PEDT011" ) ; break ; }
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_REDO:
			if ( w2 == "" ) { actionREDO() ; }
			else if ( w2 == "ALL" )
			{
				while ( actionREDO() ) ;
			}
			else { pcmd.set_msg( "PEDT011" ) ; break ; }
			break ;

	case PC_SAVE:
			pcmd.setActioned() ;
			if ( !saveFile() ) { break                         ; }
			if ( termEdit )    { setmsg( "PEDT011P" )          ; }
			else               { pcmd.set_msg( "PEDT011P", 4 ) ; }
			break ;

	case PC_SETUNDO:
			if ( w2 == "ON" || w2 == "KEEP" )
			{
				profUndoOn   = true ;
				profUndoKeep = ( w2 == "KEEP" ) ;
				if ( !fileChanged )
				{
					it        = data.begin() ;
					saveLevel = (*it)->get_Global_File_level() ;
				}
			}
			else if ( w2 == "OFF" )
			{
				profUndoOn   = false ;
				pcmd.set_msg( "PEDT011U" ) ;
				removeRecoveryData() ;
				saveLevel    = -1    ;
			}
			else if ( w2 == "CLEAR" )
			{
				removeRecoveryData() ;
				if ( fileChanged || !profUndoOn )
				{
					saveLevel = -1 ;
				}
				else
				{
					saveLevel = 0 ;
				}
			}
			else
			{
				pcmd.set_msg( "PEDT011" ) ;
				break ;
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_SORT:
			int nsort ;
			int s1    ;
			size_t s2 ;
			int ln    ;
			bool sortOrder ;
			r.c_vlab = true ;
			r.c_vcol = true ;
			wall     = upper( subword( cmd, 2 ) ) ;
			nsort    = 1    ;                      // just one sort field supported for now
			i = wordpos( "D", wall ) ;
			if ( i > 0 )
			{
				idelword( wall, i, 1 )     ;
				srt_asc.push_back( false ) ;
			}
			else
			{
				i = wordpos( "A", wall ) ;
				if ( i > 0 )
				{
					idelword( wall, i, 1 )    ;
					srt_asc.push_back( true ) ;
				}
			}
			srt_asc.push_back( true ) ;
			if ( !setCommandRange( wall, r ) ) { break ; }
			if ( r.c_sidx == -1 ) { r.c_sidx = 1 ; r.c_eidx = data.size() - 2 ; }
			copyFileData( tdata, r.c_sidx, r.c_eidx ) ;
			s1 = (r.c_scol == 0) ? 0 : r.c_scol-1 ;
			s2 = (r.c_ecol == 0) ? 0 : r.c_ecol-1 ;
			if (  LeftBnd  > 1 &&
			    ((r.c_scol > 0 && r.c_scol < LeftBnd) ||
			     (r.c_ecol > 0 && r.c_ecol < LeftBnd) ) )
			{
				pcmd.set_msg( "PEDT014F" ) ;
				break ;
			}
			if (  RightBnd > 1 &&
			    ((r.c_scol > 0 && r.c_scol > RightBnd) ||
			     (r.c_ecol > 0 && r.c_ecol > RightBnd) ) )
			{
				pcmd.set_msg( "PEDT014G" ) ;
				break ;
			}
			if ( s2 == 0 )
			{
				s2 = ( RightBnd > 1 ) ? RightBnd - 1 : string::npos ;
			}
			ln = ( s2 == string::npos ) ? string::npos : s2 - s1 ;
			stable_sort( tdata.begin(), tdata.end(),
				[ &s1, &s2, &ln, &srt_asc, &nsort ]( const string& a, const string& b )
				{
					for ( int i = 0 ; i < nsort ; i++ )
					{
						 if ( a.size() <= s1 ) { return false ; }
						 if ( srt_asc[ i ] )
						 {
							 return a.substr( s1, ln ) < b.substr( s1, ln ) ;
						 }
						 else
						 {
							 return a.substr( s1, ln ) > b.substr( s1, ln ) ;
						 }
					}
					return false ;
				} ) ;
			Level++ ;
			sortOrder = true ;
			it        = getLineItr( r.c_sidx ) ;
			for ( i = 0 ; i < tdata.size() ; i++, it++ )
			{
				if ( !(*it)->isValidFile() ) { i-- ; continue ; }
				t1 = tdata.at( i ) ;
				t2 = (*it)->get_idata() ;
				if ( RightBnd > 0 )
				{
					t1 = substr( t2, 1, LeftBnd-1 ) +
					     substr( t1, LeftBnd, (RightBnd - LeftBnd + 1) ) +
					     substr( t2, RightBnd+1 ) ;
				}
				else if ( LeftBnd > 1 )
				{
					t1 = substr( t2, 1, LeftBnd-1 ) +
					     substr( t1, LeftBnd ) ;
				}
				trim_right( t1 ) ;
				if ( (*it)->put_idata( t1, Level ) )
				{
					fileChanged = true  ;
					sortOrder   = false ;
				}
			}
			if ( sortOrder ) { pcmd.set_msg( "PEDT014E", 4 ) ; }
			else             { rebuildZAREA = true           ; }
			break ;

	case PC_TABS:
			if ( w2 == "OFF" )
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				profTabs  = false ;
				profATabs = false ;
			}
			else
			{
				if ( ws > 3 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				if ( w2 == "ON" )  { w2 = w3                           ; }
				else if ( ws > 2 ) { pcmd.set_msg( "PEDT011" ) ; break ; }
				if ( w2 == "STD" || w2 == "" )
				{
					profTabs  = true  ;
					profATabs = false ;
				}
				else if ( w2 == "ALL" )
				{
					profTabs  = true ;
					profATabs = true ;
				}
				else if ( w2.size() == 1 )
				{
					tabsChar = w2.front() ;
				}
				else { pcmd.set_msg( "PEDT011" ) ; break ; }
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_UNDO:
			if ( w2 == "" ) { actionUNDO() ; }
			else if ( w2 == "ALL" )
			{
				while ( actionUNDO() ) ;
			}
			else { pcmd.set_msg( "PEDT011" ) ; break ; }
			break ;

	case PC_XTABS:
			if ( w2 == "ON" )
			{
				profXTabs = true ;
				if ( !tabsOnRead )
				{
					Msgs.push_back( "-WARNING- Tabs have been set on when no tabs where found in the file" ) ;
					addSpecial( 'M', topLine, Msgs ) ;
				}
			}
			else if ( w2 == "OFF" )
			{
				profXTabs = false ;
				if ( tabsOnRead )
				{
					Msgs.push_back( "-WARNING- Tabs have been set off when tabs where found in the file" ) ;
					addSpecial( 'M', topLine, Msgs ) ;
				}
			}
			else if ( datatype( w2, 'W' ) && w2.size() < 3 )
			{
				XTabz = ds2d( w2 ) ;
				if ( XTabz == 0 ) { XTabz = 1 ; }
				Msgs.push_back( "-WARNING- Tabs size has been changed.  This will come into effect the next" ) ;
				Msgs.push_back( "          time the data is read.  Saving the data will use the old value."  ) ;
				addSpecial( 'M', topLine, Msgs ) ;
			}
			else { pcmd.set_msg( "PEDT011" ) ; break ; }
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;
	}
}


void PEDIT01::actionLineCommands()
{
	// For each line in the command vector, action the line command
	// If there have been any delete commands (including moves), clear all shadow lines to force a rebuild

	if ( !checkLineCommands() ) { return ; }

	for_each( data.begin(), data.end(),
		[](iline *& a)
		{
			a->clearLcc() ;
		} ) ;

	for ( auto itc = lcmds.begin() ; itc != lcmds.end() ; itc++ )
	{
		actionLineCommand( itc ) ;
	}

	if ( any_of( lcmds.begin(), lcmds.end(),
		[](lcmd& a)
		{
			return a.lcmd_CMDSTR.front() == 'M' || a.lcmd_CMDSTR.front() == 'D' ;
		} ) )
	{
		clr_hilight_shadow() ;
	}
	lcmds.clear() ;
}


void PEDIT01::actionLineCommand( vector<lcmd>::iterator itc )
{
	// Action the line command
	// For copy/move/repeat preserve flags: file, note, prof, col, excl, hex

	// Treat an excluded block as though it were just one line for the purposes of the Rpt value

	int i  ;
	int j  ;
	int k  ;
	int l  ;
	int p1 ;

	uint dl ;

	bool overlayOK ;
	bool splitOK   ;
	bool shiftOK   ;
	bool csrPlaced ;

	string tmp1   ;
	string tmp2   ;
	string inLine ;
	string fname  ;

	std::ifstream fin ;

	vector<ipline> vip ;
	ipline ip ;

	iline * p_iline ;

	vector<string> tdata ;

	vector<ipline>::iterator new_end  ;
	vector<iline * >::iterator il_it  ;
	vector<iline * >::iterator il_its ;
	vector<iline * >::iterator il_ite ;

	if ( itc->lcmd_Rpt == -1 )
	{
		itc->lcmd_Rpt = findword( itc->lcmd_CMDSTR, "( (( )) ) < << >> >" ) ? 2 : 1 ;
	}

	switch ( itc->lcmd_CMD )
	{
	case LC_BOUNDS:
		Level++ ;
		il_its  = getLineItr( itc->lcmd_sURID ) ;
		p_iline = new iline( taskid() ) ;
		p_iline->il_type = LN_BNDS ;
		p_iline->put_idata( "", Level ) ;
		il_its = data.insert( il_its, p_iline ) ;
		placeCursor( (*il_its)->il_URID, 1 ) ;
		rebuildZAREA = true ;
		break ;

	case LC_COLS:
		Level++ ;
		il_its  = getLineItr( itc->lcmd_sURID ) ;
		p_iline = new iline( taskid() ) ;
		p_iline->il_type = LN_COL ;
		p_iline->put_idata( "", Level ) ;
		il_its = data.insert( il_its, p_iline ) ;
		il_its = getNextDataLine( il_its )      ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		rebuildZAREA = true ;
		break ;

	case LC_C:
	case LC_M:
	case LC_CC:
	case LC_MM:
		if ( !itc->lcmd_swap )
		{
			Level++ ;
		}
		vip.clear() ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			copyPrefix( ip, (*il_its) ) ;
			ip.ip_data = (*il_its)->get_idata() ;
			vip.push_back( ip ) ;
		}
		overlayOK = true  ;
		csrPlaced = false ;
		if ( itc->lcmd_cut )
		{
			if ( !copyToClipboard( vip ) ) { break ; }
		}
		else if ( itc->lcmd_create )
		{
			createFile( getLine( itc->lcmd_sURID ), getLine( itc->lcmd_eURID ) ) ;
		}
		else
		{
			if ( itc->lcmd_ABOW == 'O' )
			{
				new_end = remove_if( vip.begin(), vip.end(),
					[](const ipline& a)
					{
						return !a.is_file() ;
					} ) ;
				vip.erase( new_end, vip.end() ) ;
				il_its = getLineItr( itc->lcmd_dURID ) ;
				il_ite = getLineItr( itc->lcmd_lURID, il_its ) ;
				il_ite++ ;
				j = 0    ;
				k = 0    ;
				if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
				for ( ; il_its != il_ite ; il_its++ )
				{
					if ( !(*il_its)->isValidFile() ) { continue ; }
					tmp1 = overlay1( vip[ j ].ip_data, (*il_its)->get_idata(), overlayOK ) ;
					(*il_its)->put_idata( tmp1, Level ) ;
					j++ ;
					if ( j == vip.size() ) { j = 0 ; k++ ; }
					fileChanged = true ;
				}
				if ( k == 0 && j < vip.size() ) { pcmd.set_msg( "PEDT012C" ) ; }
			}
			else
			{
				il_ite = getLineItr( itc->lcmd_dURID ) ;
				if ( itc->lcmd_ABOW == 'B' ) { il_ite-- ; }
				for ( j = 0 ; j < vip.size() ; j++ )
				{
					p_iline  = new iline( taskid() ) ;
					if ( itc->lcmd_CMD == LC_M || itc->lcmd_CMD == LC_MM )
					{
						copyPrefix( p_iline, vip[ j ], true ) ;
					}
					else
					{
						copyPrefix( p_iline, vip[ j ] ) ;
					}
					p_iline->put_idata( vip[ j ].ip_data, Level ) ;
					il_ite++ ;
					il_ite = data.insert( il_ite, p_iline ) ;
					if ( !csrPlaced && aLCMD )
					{
						placeCursor( (*il_ite)->il_URID, 1 ) ;
						csrPlaced = true ;
					}
					if ( p_iline->is_file() ) { fileChanged = true ; }
				}
			}
		}
		if ( overlayOK && (itc->lcmd_CMD == LC_M || itc->lcmd_CMD == LC_MM) )
		{
			il_its = getLineItr( itc->lcmd_sURID ) ;
			il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				(*il_its)->set_deleted( Level ) ;
				if ( (*il_its)->is_file() ) {  fileChanged = true ; }
			}
		}
		if ( !overlayOK && itc->lcmd_CMDSTR.front() == 'M' ) { pcmd.set_msg( "PEDT012D" ) ; }
		rebuildZAREA = true ;
		break ;

	case LC_D:
	case LC_DD:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		for ( il_it = il_its ; il_it != il_ite ; il_it++ )
		{
			if ( (*il_it)->il_deleted ) { continue           ; }
			if ( (*il_it)->is_file()  ) { fileChanged = true ; }
			(*il_it)->set_deleted( Level ) ;
		}
		il_it = getValidDataLineNext( il_its )  ;
		if ( aLCMD ) { placeCursor( (*il_it)->il_URID, 1 ) ; }
		rebuildZAREA = true ;
		break ;

	case LC_F:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( j = 0 ; j < itc->lcmd_Rpt ; j++ )
		{
			if (  (*il_its)->is_bod() )   { break                     ; }
			if ( !(*il_its)->il_excl )    { break                     ; }
			if (  (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
			(*il_its)->il_excl = false ;
			il_its++ ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_HX:
	case LC_HXX:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			(*il_its)->il_hex = !(*il_its)->il_hex ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_I:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		k = (*il_its)->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
		if ( k == string::npos )
		{
			il_it = getNextFileLine( il_its ) ;
			if ( il_it != data.end() )
			{
				k = (*il_it)->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
			}
			if ( k == string::npos )
			{
				il_it = getPrevFileLine( il_its );
				if ( il_it != data.end() )
				{
					k = (*il_it)->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
					if ( k == string::npos ) { k = 0 ; }
				}
				else
				{
					k = 0 ;
				}
			}
		}
		csrPlaced = false ;
		for ( j = 0 ; j < itc->lcmd_Rpt ; j++ )
		{
			p_iline = new iline( taskid() ) ;
			p_iline->il_type = LN_ISRT ;
			p_iline->put_idata( getMaskLine() ) ;
			il_its++ ;
			il_its = data.insert( il_its, p_iline ) ;
			if ( !csrPlaced && aLCMD )
			{
				placeCursor( p_iline->il_URID, 4, k ) ;
				p_iline->set_idata_minsize( k ) ;
				csrPlaced = true ;
			}
		}
		rebuildZAREA = true ;
		break ;

	case LC_L:
		dl = getLine( itc->lcmd_sURID ) ;
		dl = getLastEX( dl ) ;
		if ( aLCMD ) { placeCursor( itc->lcmd_sURID, 1 ) ; }
		for ( j = 0 ; j < itc->lcmd_Rpt ; j++ )
		{
			if (  data.at( dl )->is_tod() )   { break ; }
			if (  data.at( dl )->il_deleted ) { j-- ; dl = getPrevDataLine( dl ) ; continue ; }
			if ( !data.at( dl )->il_excl )    { break ; }
			data.at( dl )->il_excl = false ;
			dl = getPrevDataLine( dl ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_LC:
	case LC_LCC:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			if ( (*il_its)->set_idata_lower( Level ) ) { fileChanged = true ; }
		}
		rebuildZAREA = true ;
		break ;

	case LC_MASK:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		p_iline          = new iline( taskid() ) ;
		p_iline->il_type = LN_MASK ;
		p_iline->put_idata( getMaskLine(), Level ) ;
		il_its = data.insert( il_its, p_iline ) ;
		rebuildZAREA = true ;
		break ;

	case LC_MD:
	case LC_MDD:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		csrPlaced = false ;
		for ( il_it = il_its ; il_it != il_ite ; il_it++ )
		{
			if ( (*il_it)->is_bod()   ) { break    ; }
			if ( (*il_it)->il_deleted ) { continue ; }
			if ( (*il_it)->is_file()  ) { continue ; }
			p_iline = new iline( taskid() ) ;
			p_iline->il_type = LN_FILE ;
			if ( (*il_it)->is_col() )
			{
				tmp1 = getColumnLine( 1 ) ;
			}
			else if ( (*il_it)->is_bnds() )
			{
				tmp1 = getMaskLine() ;
			}
			else
			{
				tmp1 = (*il_it)->get_idata() ;
			}
			p_iline->put_idata( tmp1, Level ) ;
			(*il_it)->set_deleted( Level ) ;
			il_it = data.insert( il_it, p_iline ) ;
			fileChanged = true ;
			if ( !csrPlaced && aLCMD )
			{
				placeCursor( (*il_it)->il_URID, 1 ) ;
				csrPlaced = true ;
			}
			il_it++  ;
			il_ite = getLineItr( itc->lcmd_eURID ) ;
			if ( il_it == il_ite ) { break ; }
			il_ite++ ;
			il_ite++ ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_MN:
	case LC_MNN:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		csrPlaced = false ;
		for ( il_it = il_its ; il_it != il_ite ; il_it++ )
		{
			if ( (*il_it)->is_bod()   ) { break    ; }
			if ( (*il_it)->il_deleted ) { continue ; }
			if ( (*il_it)->is_note() )  { continue ; }
			p_iline = new iline( taskid() ) ;
			p_iline->il_type = LN_NOTE ;
			p_iline->put_idata( (*il_it)->get_idata(), Level ) ;
			(*il_it)->set_deleted( Level ) ;
			if ( (*il_it)->is_file() ) { fileChanged = true ; }
			il_it = data.insert( il_it, p_iline ) ;
			if ( !csrPlaced && aLCMD )
			{
				placeCursor( (*il_it)->il_URID, 1 ) ;
				csrPlaced = true ;
			}
			il_it++  ;
			il_ite = getLineItr( itc->lcmd_eURID ) ;
			if ( il_it == il_ite ) { break ; }
			il_ite++ ;
			il_ite++ ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_R:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_it  = il_its ;
		tmp1   = (*il_it)->get_idata() ;
		copyPrefix( ip, (*il_it ) ) ;
		if ( (*il_it)->is_file() ) { fileChanged = true ; }
		csrPlaced = false ;
		for ( j = 0 ; j < itc->lcmd_Rpt ; j++ )
		{
			p_iline = new iline( taskid() ) ;
			copyPrefix( p_iline, ip ) ;
			p_iline->put_idata( tmp1, Level ) ;
			il_it++ ;
			il_it = data.insert( il_it, p_iline ) ;
			if ( !csrPlaced && aLCMD )
			{
				placeCursor( (*il_it)->il_URID, 1 ) ;
				csrPlaced = true ;
			}
		}
		rebuildZAREA = true ;
		break ;

	case LC_RR:
		Level++ ;
		vip.clear() ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		csrPlaced = false ;
		for ( il_it = il_its ; ; il_it++ )
		{
			if ( (*il_it)->is_bod()   ) { break              ; }
			if ( (*il_it)->il_deleted ) { continue           ; }
			if ( (*il_it)->is_file()  ) { fileChanged = true ; }
			copyPrefix( ip, (*il_it ) ) ;
			if ( il_it == il_its )
			{
				if ( ip.is_isrt() )
				{
					placeCursor( (*il_it)->il_URID, 2 ) ;
					csrPlaced = true ;
				}
			}
			ip.ip_data = (*il_it)->get_idata() ;
			vip.push_back( ip ) ;
			if ( il_it == il_ite ) { break ; }
		}
		for ( j = 0 ; j < itc->lcmd_Rpt ; j++ )
		{
			for ( k = 0 ; k < vip.size() ; k++ )
			{
				p_iline = new iline( taskid() ) ;
				copyPrefix( p_iline, vip[ k ] ) ;
				p_iline->put_idata( vip[ k ].ip_data, Level ) ;
				il_ite++ ;
				il_ite = data.insert( il_ite, p_iline ) ;
				if ( !csrPlaced && aLCMD )
				{
					placeCursor( (*il_ite)->il_URID, 1 ) ;
					csrPlaced = true ;
				}
			}
		}
		rebuildZAREA = true ;
		break ;

	case LC_S:
	case LC_SI:
		j = 65535 ;
		l = 0     ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( !(*il_its)->il_excl ) { break ; }
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		for ( il_it = il_its ; il_it != il_ite ; il_it++ )
		{
			if ( (*il_it)->il_deleted ) { continue ; }
			k = (*il_it)->get_idata_ptr()->find_first_not_of( ' ' ) ;
			if ( k != string::npos ) { j = min( j, k ) ; }
		}
		for ( il_it = il_its ; il_it != il_ite ; il_it++ )
		{
			if ( (*il_it)->il_deleted ) { continue ; }
			k = (*il_it)->get_idata_ptr()->find_first_not_of( ' ' ) ;
			if ( k == j || k == string::npos )
			{
				(*il_it)->il_excl = false ;
				if ( itc->lcmd_CMD == LC_S )
				{
					l++ ;
					if ( l == itc->lcmd_Rpt ) { break ; }
				}
			}
		}
		break ;

	case LC_TABS:
		Level++ ;
		il_its  = getLineItr( itc->lcmd_sURID ) ;
		p_iline = new iline( taskid() ) ;
		p_iline->il_type = LN_TABS      ;
		p_iline->put_idata( tabsLine, Level ) ;
		il_its = data.insert( il_its, p_iline ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		rebuildZAREA = true ;
		break ;

	case LC_TJ:
	case LC_TJJ:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		tmp1   = (*il_its)->get_idata() ;
		(*il_its)->set_deleted( Level ) ;
		il_its++ ;
		il_ite++ ;
		for ( il_it = il_its ; il_it != il_ite ; il_it++ )
		{
			if ( !(*il_it)->isValidFile() ) { continue ; }
			tmp1 = tmp1 + " " + strip( (*il_it)->get_idata() ) ;
			(*il_it)->set_deleted( Level ) ;
		}
		if ( tmp1 != "" )
		{
			p_iline = new iline( taskid() ) ;
			p_iline->il_type = LN_FILE      ;
			p_iline->put_idata( tmp1, Level ) ;
			il_it = data.insert( il_its, p_iline ) ;
			k     = p_iline->get_idata_ptr()->find_first_not_of( ' ' ) ;
			if ( aLCMD ) { placeCursor( p_iline->il_URID, 4, k ) ; }
			fileChanged = true ;
		}
		break ;

	case LC_TR:
	case LC_TRR:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			if ( (*il_its)->set_idata_trim( Level ) )
			{
				fileChanged = true ;
			}
		}
		rebuildZAREA = true ;
		break ;

	case LC_TS:
		Level++ ;
		il_its  = getLineItr( itc->lcmd_sURID ) ;
		splitOK = textSplitData( *(*il_its)->get_idata_ptr(), tmp1, tmp2 ) ;
		if ( aURID == (*il_its)->il_URID && aCol > CLINESZ )
		{
			if ( (*il_its)->isValidFile() )
			{
				if ( (*il_its)->put_idata( tmp1, Level ) )
				{
					fileChanged = true ;
				}
				p_iline = new iline( taskid() ) ;
				p_iline->il_type = LN_ISRT ;
				p_iline->put_idata( getMaskLine() ) ;
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
				if ( splitOK )
				{
					p_iline = new iline( taskid() ) ;
					p_iline->il_type = LN_FILE ;
					p_iline->put_idata( tmp2, Level ) ;
					il_its++ ;
					il_its = data.insert( il_its, p_iline ) ;
				}
			}
		}
		else
		{
			p_iline = new iline( taskid() ) ;
			p_iline->il_type = LN_ISRT ;
			p_iline->put_idata( getMaskLine() ) ;
			il_its++ ;
			il_its = data.insert( il_its, p_iline ) ;
		}
		restoreCursor()     ;
		rebuildZAREA = true ;
		break ;

	case LC_T:
	case LC_TT:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			(*il_its)->toggleMarked() ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_TX:
	case LC_TXX:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted  ) { continue ; }
			(*il_its)->il_excl = !(*il_its)->il_excl ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_UC:
	case LC_UCC:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			if ( (*il_its)->set_idata_upper( Level ) ) { fileChanged = true ; }
		}
		rebuildZAREA = true ;
		break ;

	case LC_XI:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		j = (*il_its)->get_idata_ptr()->find_first_not_of( ' ' ) ;
		(*il_its)->il_excl = true ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		while ( j != string::npos )
		{
			il_its++ ;
			if (  (*il_its)->is_bod() )   { break    ; }
			if (  (*il_its)->il_deleted ) { continue ; }
			if ( !(*il_its)->is_file() )  { (*il_its)->il_excl = true ; continue ; }
			k = (*il_its)->get_idata_ptr()->find_first_not_of( ' ' ) ;
			if ( k != string::npos && k < j ) { break ; }
			(*il_its)->il_excl = true ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_X:
	case LC_XX:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted  ) { continue ; }
				(*il_its)->il_excl = true ;
			}
			rebuildZAREA = true ;
			break ;

	case LC_SRTC:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		k = (*il_its)->get_idata_ptr()->find_first_not_of( ' ' ) ;
		itc->lcmd_Rpt = (itc->lcmd_Rpt-1) * profXTabz + (profXTabz - k % profXTabz) ;
	case LC_SRC:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		(*il_its)->put_idata( rshiftCols( itc->lcmd_Rpt, (*il_its)->get_idata_ptr()), Level ) ;
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SRCC:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted  ) { continue ; }
			(*il_its)->put_idata( rshiftCols( itc->lcmd_Rpt, (*il_its)->get_idata_ptr()), Level ) ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SRTCC:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			k = (*il_its)->get_idata_ptr()->find_first_not_of( ' ' ) ;
			l = (itc->lcmd_Rpt-1) * profXTabz + (profXTabz - k % profXTabz) ;
			(*il_its)->put_idata( rshiftCols( l, (*il_its)->get_idata_ptr()), Level ) ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SLTC:
		il_its = getLineItr( itc->lcmd_sURID ) ;
		k = (*il_its)->get_idata_ptr()->find_first_not_of( ' ', LeftBnd-1 ) ;
		if ( k % profXTabz == 0 ) { itc->lcmd_Rpt = itc->lcmd_Rpt * profXTabz ; }
		else                      { itc->lcmd_Rpt = (itc->lcmd_Rpt-1) * profXTabz + (k % profXTabz) ; }
		itc->lcmd_Rpt = min( itc->lcmd_Rpt, k ) ;
	case LC_SLC:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		(*il_its)->put_idata( lshiftCols( itc->lcmd_Rpt, (*il_its)->get_idata_ptr()), Level ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SLCC:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			(*il_its)->put_idata( lshiftCols( itc->lcmd_Rpt, (*il_its)->get_idata_ptr()), Level ) ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SLTCC:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			k = (*il_its)->get_idata_ptr()->find_first_not_of( ' ', LeftBnd-1 ) ;
			if ( k % profXTabz == 0 ) { l = itc->lcmd_Rpt * profXTabz ; }
			else                      { l = (itc->lcmd_Rpt-1) * profXTabz + (k % profXTabz) ; }
			l = min( l, k ) ;
			(*il_its)->put_idata( lshiftCols( l, (*il_its)->get_idata_ptr()), Level ) ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SRD:
		Level++ ;
		il_its  = getLineItr( itc->lcmd_sURID ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		shiftOK = rshiftData( itc->lcmd_Rpt, (*il_its)->get_idata_ptr(), tmp1 ) ;
		if ( (*il_its)->put_idata( tmp1, Level ) )
		{
			fileChanged  = true ;
		}
		if ( !shiftOK ) { (*il_its)->setErrorStatus() ; pcmd.set_msg( "PEDT014B" ) ; }
		rebuildZAREA = true ;
		break ;

	case LC_SRDD:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted  ) { continue ; }
			shiftOK = rshiftData( itc->lcmd_Rpt, (*il_its)->get_idata_ptr(), tmp1 ) ;
			if ( (*il_its)->put_idata( tmp1, Level ) )
			{
				fileChanged  = true ;
			}
			if ( !shiftOK ) { (*il_its)->setErrorStatus() ; pcmd.set_msg( "PEDT014B" ) ; }
		}
		rebuildZAREA = true ;
		break ;

	case LC_SLD:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		shiftOK = lshiftData( itc->lcmd_Rpt, (*il_its)->get_idata_ptr(), tmp1 ) ;
		if ( (*il_its)->put_idata( tmp1, Level ) )
		{
			fileChanged  = true ;
		}
		if ( !shiftOK ) { (*il_its)->setErrorStatus() ; pcmd.set_msg( "PEDT014B" ) ; }
		rebuildZAREA = true ;
		break ;

	case LC_SLDD:
		Level++ ;
		il_its = getLineItr( itc->lcmd_sURID ) ;
		il_ite = getLineItr( itc->lcmd_eURID, il_its ) ;
		il_ite++ ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		for ( ; il_its != il_ite ; il_its++ )
		{
			if ( (*il_its)->il_deleted ) { continue ; }
			shiftOK = lshiftData( itc->lcmd_Rpt, (*il_its)->get_idata_ptr(), tmp1 ) ;
			if ( (*il_its)->put_idata( tmp1, Level ) )
			{
				fileChanged  = true ;
			}
			if ( !shiftOK ) { (*il_its)->setErrorStatus() ; pcmd.set_msg( "PEDT014B" ) ; }
		}
		rebuildZAREA = true ;
		break ;

	case LC_COPY:
		Level++;
		i = 0  ;
		j = -1 ;
		k = -1 ;
		if ( pcmd.get_cmd_words() == 1 )
		{
			display( "PEDIT015", "", "ZCMD5" ) ;
			if ( RC == 8 ) { break ; }
			vcopy( "LINE1", tmp1, MOVE ) ;
			if ( tmp1 != "" ) { j = ds2d( tmp1 ) ; }
			vcopy( "LINE2", tmp1, MOVE ) ;
			if ( tmp1 != "" ) { k = ds2d( tmp1 ) ; }
			vcopy( "ZFILE2", fname, MOVE ) ;
		}
		else
		{
			fname = pcmd.get_userdata() ;
		}
		vreplace( "ZSTR1", fname ) ;
		fin.open( fname.c_str() )  ;
		if ( !fin.is_open() )
		{
			pcmd.set_msg( "PEDT014S", 20 ) ;
			break ;
		}
		while ( getline( fin, inLine ) )
		{
			i++ ;
			if ( i < j ) { continue ; }
			if ( k > -1 && i > k ) { break ; }
			p1 = inLine.find( '\t' ) ;
			while ( p1 != string::npos && !optNoConvTabs )
			{
				j  = profXTabz - (p1 % profXTabz ) ;
				inLine.replace( p1, 1,  j, ' ' ) ;
				p1 = inLine.find( '\t', p1 + 1 ) ;
			}
			ip.ip_data = inLine ;
			vip.push_back( ip ) ;
		}
		fin.close() ;
		if ( vip.size() == 0 )
		{
			pcmd.set_msg( "PEDT015L", 8 ) ;
			break ;
		}
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( itc->lcmd_ABOW == 'B' ) { il_its-- ; }
		addSpecial( 'I', il_its, left( "== Start of inserted file ", ZDATAW, '=' ) ) ;
		il_its++ ;
		for ( j = 0 ; j < vip.size() ; j++ )
		{
			p_iline          = new iline( taskid() ) ;
			p_iline->il_type = LN_FILE ;
			p_iline->put_idata( vip[ j ].ip_data, Level ) ;
			il_its++ ;
			il_its = data.insert( il_its, p_iline ) ;
		}
		addSpecial( 'I', il_its, left( "== End of inserted file ", ZDATAW, '=' ) ) ;
		vreplace( "ZEDLNES", d2ds( vip.size() ) ) ;
		pcmd.set_msg( "PEDT015J", 0 ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_PASTE:
		Level++;
		getClipboard( vip ) ;
		if ( vip.size() == 0 )
		{
			pcmd.set_msg( "PEDT015L", 8 ) ;
			break ;
		}
		il_its = getLineItr( itc->lcmd_sURID ) ;
		if ( itc->lcmd_ABOW == 'B' ) { il_its-- ; }
		for ( j = 0 ; j < vip.size() ; j++ )
		{
			p_iline          = new iline( taskid() ) ;
			p_iline->il_type = LN_FILE ;
			p_iline->put_idata( vip[ j ].ip_data, Level ) ;
			il_its++ ;
			il_its = data.insert( il_its, p_iline ) ;
		}
		vreplace( "ZEDLNES", d2ds( vip.size() ) ) ;
		vreplace( "CLIPNAME", clipBoard ) ;
		pcmd.set_msg( "PEDT013D", 0 ) ;
		if ( aLCMD ) { placeCursor( (*il_its)->il_URID, 1 ) ; }
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	default:
			llog( "E", "Invalid line command "<< itc->lcmd_CMDSTR <<endl );
	}
}


void PEDIT01::actionZVERB()
{
	int t  ;
	int p1 ;
	int p2 ;

	uint dl  ;
	uint row ;

	string w1 ;

	if ( zverb == "DOWN" )
	{
		t = 0 ;
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			topLine = data.size() - 1 ;
			for ( ; topLine > 0 ; topLine-- )
			{
				if ( data.at( topLine )->il_deleted ) { continue ; }
				if ( data.at( topLine )->il_excl )
				{
					topLine = getFirstEX( topLine ) ;
					if ( hideExcl )     { topLine = getPrevDataLine( topLine ) ; }
					if ( topLine == 0 ) { break ; }
				}
				if ( data.at( topLine )->isValidFile() &&
				    (data.at( topLine )->il_hex || profHex ) )
				{
					t = t + 4 ;
					if ( t > ZAREAD-4 ) { break ; }
				}
				else
				{
					t++ ;
					if ( t > ZAREAD-1 ) { break ; }
				}
			}
		}
		else
		{
			for ( ; topLine < ( data.size() - 1 ) ; topLine++ )
			{
				if ( data.at( topLine )->il_deleted ) { continue ; }
				if ( data.at( topLine )->il_excl )
				{
					topLine = getLastEX( topLine ) ;
					if ( hideExcl ) { topLine = getNextDataLine( topLine ) ; }
					if ( topLine == data.size() -1 ) { break ; }
				}
				if ( data.at( topLine )->isValidFile()       &&
				    (data.at( topLine )->il_hex || profHex ) &&
				    !datatype( ZSCROLLA, 'W' ) )
				{
					t = t + 4 ;
				}
				else
				{
					t++ ;
				}
				if ( t > ZSCROLLN ) { break ; }
			}
		}
	}
	else if ( zverb == "UP" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			topLine = 0 ;
		}
		else
		{
			t = 0 ;
			for ( ; topLine > 0 ; topLine-- )
			{
				if ( data.at( topLine )->il_deleted ) { continue ; }
				if ( data.at( topLine )->il_excl )
				{
					topLine = getFirstEX( topLine ) ;
					if ( hideExcl ) { topLine = getPrevDataLine( topLine ) ; }
					if ( topLine == 0 ) { break ; }
				}
				if ( data.at( topLine )->isValidFile()       &&
				    (data.at( topLine )->il_hex || profHex ) &&
				    !datatype( ZSCROLLA, 'W' ) )
				{
					t = t + 4 ;
				}
				else
				{
					t++ ;
				}
				if ( t > ZSCROLLN ) { break ; }
			}
		}
	}
	else if ( zverb == "LEFT" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			startCol = 1 ;
		}
		else
		{
			startCol = startCol - ZSCROLLN ;
			if ( startCol < 1 ) { startCol = 1 ; }
		}
	}
	else if ( zverb == "RIGHT" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			startCol = maxCol - ZAREAW ;
		}
		else if ( ZSCROLLA == "CSR" )
		{
			if ( ZSCROLLN == CLINESZ )
			{
				startCol += ZDATAW ;
			}
			else
			{
				startCol += ZSCROLLN - CLINESZ ;
			}
		}
		else
		{
			startCol += ZSCROLLN ;
		}
		if ( startCol < 1 ) { startCol = 1 ; }
	}
	else if ( zverb == "RCHANGE" )
	{
		w1 = upper( word( pcmd.get_cmd(), 1 ) ) ;
		if ( findword( w1, "C CHA CHG CHANGE" ) )
		{
			if ( !setFindChangeExcl( 'C' ) ) { return ; }
		}
		else if ( !fcx_parms.f_cset )
		{
			pcmd.set_msg( "PEDT013N" ) ;
			return ;
		}
		if ( aURID == fcx_parms.f_URID &&
		     fcx_parms.f_success       &&
		     fcx_parms.f_offset == (aCol + startCol - CLINESZ - 2) )
		{
			actionChange() ;
		}
		else
		{
			actionFind() ;
			if ( fcx_parms.f_error ) { return ; }
			if ( fcx_parms.f_success )
			{
				actionChange() ;
			}
		}
		if ( fcx_parms.f_success )
		{
			moveColumn( fcx_parms.f_offset ) ;
			if ( fcx_parms.findReverse() )
			{
				placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset ) ;
			}
			else
			{
				placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset + fcx_parms.f_cstring.size() ) ;
			}
			moveTopline( fcx_parms.f_URID ) ;
			setChangedMsg() ;
		}
		else
		{
			setNotFoundMsg() ;
		}
		fcx_parms.f_URID = 0 ;
		rebuildZAREA = true ;
	}
	else if ( zverb == "RFIND" )
	{
		w1 = upper( word( pcmd.get_cmd(), 1 ) ) ;
		if ( findword( w1, "C CHA CHG CHANGE" ) )
		{
			if ( !setFindChangeExcl( 'C' ) ) { return ; }
		}
		else if ( w1 == "F" || w1 == "FIND" )
		{
			if ( !setFindChangeExcl( 'F' ) ) { return ; }
		}
		else if ( !fcx_parms.f_fset )
		{
			pcmd.set_msg( "PEDT013M" ) ;
			return ;
		}
		actionFind() ;
		if ( fcx_parms.f_error ) { return ; }
		if ( fcx_parms.f_success )
		{
			moveColumn( fcx_parms.f_offset ) ;
			placeCursor( fcx_parms.f_URID, 4, fcx_parms.f_offset ) ;
			moveTopline( fcx_parms.f_URID ) ;
			setFoundMsg() ;
		}
		else
		{
			setNotFoundMsg() ;
		}
		rebuildZAREA = true ;
	}
	else if ( aRow > 0 && s2data.at( aRow-1 ).ipos_URID == 0 )
	{
		placeCursor( 0, 0 ) ;
	}
	else if ( aRow > 0 && s2data.at( aRow-1 ).ipos_URID > 0 )
	{
		if ( s2data.at( aRow-1 ).ipos_hex > 0 )
		{
			row = aRow ;
			if ( aLCMD )
			{
				row = row - s2data.at( aRow-1 ).ipos_hex ;
				placeCursor( row, 1 ) ;
				return ;
			}
			p1 = aCol - CLINESZ - 1 ;
			if ( profVert )
			{
				if ( s2data.at( aRow-1 ).ipos_hex == 2 ) { row-- ; }
			}
			else
			{
				if ( p1 % 2 == 1 ) { p1-- ; }
			}
			placeCursor( row, 5, p1 ) ;
			return ;
		}
		dl = s2data.at( aRow-1 ).ipos_dl ;
		if ( (data.at( dl)->il_hex || profHex) && !s2data.at( aRow-1 ).ipos_div )
		{
			if ( aLCMD )
			{
				row = aRow ;
				placeCursor( row, 1 ) ;
				return ;
			}
			row = aRow + 1 ;
			p1  = aCol - CLINESZ - 1 ;
			if ( !profVert )
			{
				p1 = 2*p1 ;
				if ( p1 >= ZDATAW ) { p1 = p1 - ZDATAW ; row++ ; }
			}
			placeCursor( row, 5, p1 ) ;
			return ;
		}
		if ( s2data.at( aRow-1 ).ipos_div )
		{
			if ( aRow == ZAREAD ) { return ; }
			row = aRow + 1 ;
			placeCursor( row, 1 ) ;
			return ;
		}
		if ( profTabs && tabsLine != "" && getTabLocation( p1 ) )
		{
			if ( p1 > aCol+startCol-CLINESZ-2 )
			{
				row = aRow ;
				placeCursor( row, 4, p1 ) ;
				return ;
			}
			if ( aRow == ZAREAD )
			{
				if ( data.at( topLine )->il_excl ) { topLine = getLastEX( topLine ) ; }
				topLine = getNextDataLine( topLine ) ;
				row     = aRow ;
				placeCursor( row, 4, p1 ) ;
			}
			else
			{
				row = aRow + 1 ;
				dl  = s2data.at( aRow ).ipos_dl ;
				if ( dl == 0 )
				{
					placeCursor( 0, 0 ) ;
					return ;
				}
				placeCursor( row, 4, p1 ) ;
			}
		}
		else
		{
			if ( aRow == ZAREAD )
			{
				if ( data.at( topLine )->il_excl ) { topLine = getLastEX( topLine ) ; }
				topLine = getNextDataLine( topLine ) ;
				row     = aRow ;
				placeCursor( row, 3 ) ;
				rebuildZAREA = true   ;
				return ;
			}
			row = aRow + 1 ;
			p1  = data.at( dl )->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
			dl  = s2data.at( aRow ).ipos_dl ;
			if ( dl == 0 )
			{
				placeCursor( 0, 0 ) ;
				return ;
			}
			p2  = data.at( dl )->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
			if ( p2 == string::npos )
			{
				if ( p1 == string::npos ) { placeCursor( row, 2 )     ; }
				else                      { placeCursor( row, 4, p1 ) ; }
			}
			else { placeCursor( row, 4, p2 ) ; }
		}
	}
}


void PEDIT01::setLineLabels()
{
	// Process line labels before any commands.  This allows us to enter labels at the same time as a command
	// containing those labels.  Remove the label error if 'RESET' or 'RESET CMD' command entered so
	// it doesn't block them from being actioned.

	string w1 ;
	string w2 ;

	for ( auto it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_lcc.size() > 0 && (*it)->il_lcc.front() == '.' )
		{
			if ( !checkLabel1( (*it)->il_lcc ) ) { pcmd.set_msg( "PEDT014" )  ; break ; }
			if ( !(*it)->is_file() )             { pcmd.set_msg( "PEDT011E" ) ; break ; }
			for ( auto itt = data.begin() ; itt != data.end() ; itt++ )
			{
				if ( (*itt)->clearLabel( (*it)->il_lcc ) ) { break ; }
			}
			(*it)->setLabel( (*it)->il_lcc ) ;
			(*it)->clearLcc()   ;
			rebuildZAREA = true ;
		}
	}

	if ( pcmd.msgset() && pcmd.cmdset() )
	{
		w1 = upper( word( pcmd.get_cmd(), 1 ) )    ;
		w2 = upper( subword( pcmd.get_cmd(), 2 ) ) ;
		if ( aliasNames.count( w2 ) > 0 ) { w2 = aliasNames[ w2 ] ; }
		if ( PrimCMDS.count( w1 ) > 0 && PrimCMDS[ w1 ].p_cmd == PC_RESET && ( w2 == "" || w2 == "CMD" ) )
		{
			pcmd.clear_msg() ;
		}
	}
}


bool PEDIT01::checkLineCommands()
{
	// For each line in the data vector, check the line commands entered and build the command vector
	// Treat commands in XBlock that are on an exluded block, as a block command (eg R, <, >, etc)

	// Special processing for TJ on non-excluded lines, as TJn implies TJ(n+1)

	// cmd   - command block
	// tcmd  - temporary command block for storing cmd when complete, and a non-abo command entered
	//         ie. Block/single commands without A/B/O bewteen block/single commands with A/B/O
	// abo   - contains a, b or o M/C positions, including OO block command, and xK commands
	// tabo  - temporary abo command block used during AK, BK, OK and OOK operations (pushed to tabos stack)
	// tabos - command block stack for storing tabo for AK, BK, OK and OOK operations

	// Check for overlapping commands (eg. Cn, Dn, Mn, On ... don't conflict with the next command)

	// Translate a swap (W/WW) into two move commands (done at the same Level for UNDO/REDO)

	lcmd cmd  ;
	lcmd tcmd ;
	lcmd abo  ;
	lcmd tabo ;
	stack<lcmd> tabos ;

	int rept  ;
	int adist ;
	int rdist ;

	string lcc ;

	bool cmd_inuse    ;
	bool abo_inuse    ;
	bool cmd_complete ;
	bool abo_complete ;
	bool abo_block    ;
	bool abo_k        ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	lcmds.clear() ;

	cmd_inuse    = false ;
	abo_inuse    = false ;
	cmd_complete = false ;
	abo_complete = false ;
	abo_block    = false ;
	abo_k        = false ;

	adist = 1  ;
	rdist = -1 ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_deleted    ) { continue ; }
		adist++ ;
		if ( (*it)->il_lcc == ""  ) { continue ; }
		if ( rdist != -1 && rdist >= adist )
		{
			pcmd.set_msg( "PEDT014C" ) ;
			break ;
		}
		rdist = -1 ;
		if ( !formLineCmd( (*it)->il_lcc, lcc, rept ) )
		{
			if ( !pcmd.msgset() ) { pcmd.set_msg( "PEDT011Y" ) ; }
			break ;
		}
		if ( (*it)->isSpecial() && SPLcmds.count( lcc ) == 0 ) { pcmd.set_msg( "PEDT013" ) ; break ; }
		if ( (*it)->is_tod() && TODcmds.count( lcc ) == 0 )    { pcmd.set_msg( "PEDT013" ) ; break ; }
		if ( (*it)->is_bod() && BODcmds.count( lcc ) == 0 )    { pcmd.set_msg( "PEDT013" ) ; break ; }
		if ( ABOWList.count( lcc ) > 0 )
		{
			if ( abokLCMDS.count( lcc ) > 0 )
			{
				abo_k = true ;
				lcc   = abokLCMDS[ lcc ] ;
			}
			else
			{
				abo_k = false ;
			}
			if ( !abo_inuse )
			{
				if ( cmd_inuse && !cmd_complete ) { pcmd.set_msg( "PEDT011Y" ) ; break ; }
				abo.lcmd_ABOW = lcc.front() ;
				if ( lcc == "A" && (*it)->il_excl )
				{
					abo.lcmd_sURID = getLastEX( it ) ;
				}
				else
				{
					abo.lcmd_sURID = (*it)->il_URID ;
				}
				abo.lcmd_Rpt   = rept ;
				abo_inuse      = true ;
				if ( OWBlock.count( lcc) > 0 )
				{
					abo_block = true ;
				}
				else
				{
					if ( lcc == "O" || lcc == "W" )
					{
						abo.lcmd_eURID = getLastURID( it, rept ) ;
					}
					if ( abo_k ) { tabos.push( abo ) ; abo_inuse = false ; abo.lcmd_clear() ; }
					else         { abo_complete = true ; }
				}
			}
			else
			{
				if ( OWBlock.count( lcc ) == 0 ||
				     !abo_block                ||
				      abo.lcmd_eURID > 0       ||
				     lcc.front() != abo.lcmd_ABOW )  { pcmd.set_msg( "PEDT011Y" ) ; break ; }
				if ( (*it)->il_excl )
				{
					abo.lcmd_eURID = getLastEX( it ) ;
				}
				else
				{
					abo.lcmd_eURID = (*it)->il_URID ;
				}
				if ( abo_k ) { tabos.push( abo ) ; abo_inuse = false ; abo.lcmd_clear() ; }
				else         { abo_complete = true ; }
			}
		}
		else if ( BLKcmds.count( lcc ) > 0 )
		{
			if ( cmd_inuse && !cmd_complete && cmd.lcmd_CMD != LineCMDS[ lcc ] )
			{
				pcmd.set_msg( "PEDT011Y" ) ;
				break ;
			}
			if ( !cmd_inuse )
			{
				cmd.lcmd_CMD    = LineCMDS[ lcc ] ;
				cmd.lcmd_CMDSTR = lcc ;
				cmd.lcmd_sURID  = (*it)->il_URID  ;
				cmd.lcmd_Rpt    = rept ;
				cmd_inuse       = true ;
			}
			else
			{
				if ( (cmd.lcmd_Rpt != -1 && rept != -1) && cmd.lcmd_Rpt != rept )
				{
					pcmd.set_msg( "PEDT012B" ) ;
					break ;
				}
				if ( (cmd_complete && ABOKReq.count( lcc) > 0 ) )
				{
					pcmd.set_msg( "PEDT011Y" ) ;
					break ;
				}
				if ( cmd_complete )
				{
					tcmd = cmd ;
					cmd.lcmd_clear() ;
					cmd.lcmd_CMD    = LineCMDS[ lcc ] ;
					cmd.lcmd_CMDSTR = lcc ;
					cmd.lcmd_sURID  = (*it)->il_URID  ;
					cmd.lcmd_Rpt    = rept  ;
					cmd_inuse       = true  ;
					cmd_complete    = false ;
				}
				else
				{
					if ( cmd.lcmd_Rpt == -1 ) { cmd.lcmd_Rpt = rept ; }
					if ( (*it)->il_excl )
					{
						cmd.lcmd_eURID = getLastEX( it ) ;
					}
					else
					{
						cmd.lcmd_eURID = (*it)->il_URID ;
					}
					if ( ABOKReq.count( lcc ) > 0 )
					{
						cmd_complete = true ;
					}
					else
					{
						lcmds.push_back( cmd ) ;
						if ( tcmd.lcmd_sURID > 0 )
						{
							cmd = tcmd           ;
							cmd_inuse    = true  ;
							cmd_complete = true  ;
							tcmd.lcmd_clear()    ;
						}
						else
						{
							cmd.lcmd_clear()     ;
							cmd_inuse    = false ;
							cmd_complete = false ;
						}
					}
				}
			}
		}
		else
		{
			if ( Chkdist.count( lcc ) > 0 ) { adist = 1 ; rdist = rept ; }
			else                            { rdist = -1               ; }
			if ( (cmd_inuse && !cmd_complete) ||
			     (cmd_complete && ABOKReq.count( lcc ) > 0 ) )
			{
				pcmd.set_msg( "PEDT011Y" ) ;
				break ;
			}
			if ( cmd_complete )
			{
				tcmd = cmd ;
				cmd.lcmd_clear() ;
			}
			if ( lcc == "TJ" && !(*it)->il_excl )
			{
				if ( rept == -1 ) { rept = 1 ; }
				rept++ ;
			}
			cmd.lcmd_sURID = (*it)->il_URID ;
			if ( lcc == "I" && (*it)->il_excl )
			{
				cmd.lcmd_sURID = getLastURID( it, 1 ) ;
			}
			cmd.lcmd_eURID  = getLastURID( it, rept ) ;
			cmd.lcmd_Rpt    = rept ;
			cmd.lcmd_CMDSTR = lcc  ;
			if ( (*it)->il_excl )
			{
				if ( XBlock.count( lcc ) > 0 )
				{
					cmd.lcmd_CMDSTR += lcc ;
					if ( aliasLCMDS.count( cmd.lcmd_CMDSTR ) > 0 )
					{
						cmd.lcmd_CMDSTR = aliasLCMDS[ cmd.lcmd_CMDSTR ] ;
					}
					cmd.lcmd_eURID = getLastEX( it ) ;
				}
			}
			cmd.lcmd_CMD = LineCMDS[ cmd.lcmd_CMDSTR ] ;
			if ( ABOKReq.count( lcc ) > 0 )
			{
				cmd_complete = true ;
				cmd_inuse    = true ;
			}
			else
			{
				lcmds.push_back( cmd ) ;
				cmd.lcmd_clear()       ;
				if ( tcmd.lcmd_sURID > 0 ) { cmd = tcmd ; tcmd.lcmd_clear() ; }
				else                       { cmd_complete = false           ; }
			}
		}
		if ( cmd_complete && abo_complete )
		{
			while ( !tabos.empty() )
			{
				tabo = cmd ;
				tabo.lcmd_ABOW   = tabos.top().lcmd_ABOW  ;
				tabo.lcmd_dURID  = tabos.top().lcmd_sURID ;
				tabo.lcmd_lURID  = tabos.top().lcmd_eURID ;
				tabo.lcmd_CMD    = LC_C ;
				tabo.lcmd_CMDSTR = "C"  ;
				lcmds.push_back( tabo ) ;
				tabos.pop()             ;
			}
			cmd.lcmd_ABOW  = abo.lcmd_ABOW  ;
			cmd.lcmd_dURID = abo.lcmd_sURID ;
			cmd.lcmd_lURID = abo.lcmd_eURID ;
			if ( cmd.lcmd_ABOW == 'W' )
			{
				if ( cmd.lcmd_CMDSTR.front() != 'M' )
				{
					pcmd.set_msg( "PEDT014H" ) ;
					break            ;
				}
				cmd.lcmd_ABOW  = 'B'   ;
				lcmds.push_back( cmd ) ;
				cmd.lcmd_dURID = cmd.lcmd_sURID ;
				cmd.lcmd_lURID = cmd.lcmd_eURID ;
				cmd.lcmd_sURID = abo.lcmd_sURID ;
				cmd.lcmd_eURID = abo.lcmd_eURID ;
				cmd.lcmd_swap  = true  ;
				cmd.lcmd_ABOW  = 'A'   ;
			}
			lcmds.push_back( cmd ) ;
			cmd.lcmd_clear()     ;
			cmd_inuse    = false ;
			cmd_complete = false ;
			abo.lcmd_clear()     ;
			abo_inuse    = false ;
			abo_complete = false ;
			abo_block    = false ;
		}
		else if ( cutActive && cmd_complete )
		{
			cmd.lcmd_cut = true  ;
			lcmds.push_back( cmd ) ;
			cmd.lcmd_clear()     ;
			cmd_inuse    = false ;
			cmd_complete = false ;
			cutActive    = false ;
		}
		else if ( creActive && cmd_complete )
		{
			cmd.lcmd_create = true ;
			lcmds.push_back( cmd ) ;
			cmd.lcmd_clear()     ;
			cmd_inuse    = false ;
			cmd_complete = false ;
			creActive    = false ;
		}
		else if ( pasteActive && abo_complete )
		{
			abo.lcmd_CMD = LC_PASTE ;
			lcmds.push_back( abo )  ;
			abo.lcmd_clear()     ;
			abo_inuse    = false ;
			abo_complete = false ;
			abo_block    = false ;
			pasteActive  = false ;
		}
		else if ( copyActive && abo_complete )
		{
			abo.lcmd_CMD = LC_COPY ;
			lcmds.push_back( abo ) ;
			abo.lcmd_clear()     ;
			abo_inuse    = false ;
			abo_complete = false ;
			abo_block    = false ;
			copyActive   = false ;
		}
	}
	if ( it != data.end() )
	{
		placeCursor( (*it)->il_URID, 1 ) ;
		return false ;
	}
	if ( cmd_inuse )
	{
		if ( cmd_complete ) { pcmd.set_msg( "PEDT014P", 8 ) ; }
		else                { pcmd.set_msg( "PEDT011W", 8 ) ; }
		return false ;
	}
	if (  abo_inuse     ) { pcmd.set_msg( "PEDT014Q" ) ; return false ; }
	if ( !tabos.empty() ) { pcmd.set_msg( "PEDT014Q" ) ; return false ; }
	if (  cutActive     )
	{
		cmd.lcmd_CMD         = LC_C  ;
		cmd.lcmd_cut         = true  ;
		cmd.lcmd_sURID       = data.at( 0 )->il_URID ;
		cmd.lcmd_eURID       = data.at( data.size()-1 )->il_URID ;
		cutActive            = false ;
		lcmds.push_back( cmd )       ;
	}
	if ( creActive )
	{
		cmd.lcmd_CMD         = LC_C  ;
		cmd.lcmd_create      = true  ;
		cmd.lcmd_sURID       = data.at( 0 )->il_URID ;
		cmd.lcmd_eURID       = data.at( data.size()-1 )->il_URID ;
		creActive            = false ;
		lcmds.push_back( cmd )       ;
	}
	if ( pasteActive )
	{
		if ( !any_of( data.begin(), data.end(),
			[](iline *& a)
			{
				return !a->il_deleted && a->is_file() ;
			} ) )
		{
			abo.lcmd_sURID = data.at( data.size() - 1 )->il_URID ;
			abo.lcmd_ABOW  = 'B'      ;
			abo.lcmd_CMD   = LC_PASTE ;
			lcmds.push_back( abo ) ;
			pasteActive  = false   ;
		}
		else
		{
			pcmd.set_msg( "PEDT013F" ) ;
			return false ;
		}
	}
	if ( copyActive )
	{
		if ( !any_of( data.begin(), data.end(),
			[](iline *& a)
			{
				return !a->il_deleted && a->is_file() ;
			} ) )
		{
			abo.lcmd_sURID = data.at( data.size() - 1 )->il_URID ;
			abo.lcmd_ABOW  = 'B'     ;
			abo.lcmd_CMD   = LC_COPY ;
			lcmds.push_back( abo ) ;
			copyActive     = false ;
		}
		else
		{
			pcmd.set_msg( "PEDT015I" ) ;
			return false ;
		}
	}
	return true ;
}


bool PEDIT01::actionUNDO()
{
	// Get the current data level from Global_Undo and call undo_idata() for all records
	// that match that level.  Move level from Global_Undo to Global_Redo

	// If any file lines have been undone, remove the top Global_File_level entry and
	// reset fileChanged indicator

	// Move the top line to the first undone change or top-of-data, if the line is on the first page.

	int  lvl   ;
	uint tURID ;

	bool isFile  ;

	if ( !profUndoOn ) { pcmd.set_msg( "PEDT011U" ) ; return false ; }

	auto it = data.begin() ;
	lvl = (*it)->get_Global_Undo_level() ;
	if ( lvl < 1 )
	{
		pcmd.set_msg( "PEDT017", 4 ) ;
		return false ;
	}

	isFile = false ;
	tURID  = 0 ;
	(*it)->move_Global_Undo2Redo() ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->get_idata_level() == lvl )
		{
			(*it)->undo_idata()    ;
			(*it)->setUndoStatus() ;
			if ( (*it)->is_file() ) { isFile = true ; }
			if ( tURID == 0 )
			{
				tURID = (*it)->il_URID ;
			}
			rebuildZAREA = true ;
		}
	}

	Level = lvl ;
	if ( isFile )
	{
		it = data.begin() ;
		(*it)->remove_Global_File_level() ;
		fileChanged = ( (*it)->get_Global_File_level() != saveLevel ) ;
	}
	clr_hilight_shadow() ;

	if ( tURID > 0 && !URIDOnScreen( tURID, topLine ) )
	{
		topLine = URIDOnScreen( tURID, 0 ) ? 0 : topLine = getLine( tURID ) ;
	}
	return true ;
}


bool PEDIT01::actionREDO()
{
	// Get the current data level from Global_Redo and call redo_idata() for all records
	// that match that level.  Move level from Global_Redo to Global_Undo

	// If any file lines have been redone, update the Global_File_level from the Global_Undo stack and
	// reset fileChanged indicator

	// Move the top line to the first redone change or top-of-data, if the line is on the first page.

	int  lvl   ;
	uint tURID ;

	bool isFile ;

	if ( !profUndoOn ) { pcmd.set_msg( "PEDT011U" ) ; return false ; }

	auto it = data.begin() ;
	lvl = (*it)->get_Global_Redo_level() ;
	if ( lvl < 1 )
	{
		pcmd.set_msg( "PEDT018", 4 ) ;
		return false ;
	}

	isFile = false ;
	tURID  = 0 ;
	(*it)->move_Global_Redo2Undo() ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->get_idata_Redo_level() == lvl )
		{
			(*it)->redo_idata()    ;
			(*it)->setRedoStatus() ;
			if ( (*it)->is_file() ) { isFile = true ; }
			if ( tURID == 0 )
			{
				tURID = (*it)->il_URID ;
			}
			rebuildZAREA = true ;
		}
	}
	Level = lvl ;
	if ( isFile )
	{
		it = data.begin() ;
		(*it)->set_Global_File_level() ;
		fileChanged = ( (*it)->get_Global_File_level() != saveLevel ) ;
	}
	clr_hilight_shadow() ;

	if ( tURID > 0 && !URIDOnScreen( tURID, topLine ) )
	{
		topLine = URIDOnScreen( tURID, 0 ) ? 0 : topLine = getLine( tURID ) ;
	}
	return true ;
}


bool PEDIT01::setFindChangeExcl( char type )
{
	int p1 ;
	int p2 ;
	int p3 ;

	bool t_dir  ;
	bool t_excl ;
	bool t_mtch ;
	bool t_str1 ;
	bool t_str2 ;

	string cmd  ;
	string rcmd ;
	string w1   ;
	string pic  ;
	string pre1 ;
	string pre2 ;

	edit_find t ;
	cmd_range r ;

	const string f_keywexcl = "X NX" ;
	const string f_keywdir  = "NEXT PREV FIRST LAST ALL" ;
	const string f_keywmtch = "CHARS PRE PREFIX SUF SUFFIX WORD" ;

	t_dir  = false ;
	t_excl = false ;
	t_mtch = false ;
	t_str1 = false ;
	t_str2 = false ;

	r.c_vlab = true ;
	r.c_vcol = true ;

	pcmd.clear_msg() ;
	cmd = subword( pcmd.get_cmd(), 2 ) ;
	trim( cmd ) ;
	if ( cmd.size() == 0 )
	{
		pcmd.set_msg( "PEDT013U" ) ;
		return false ;
	}

	rcmd = "" ;

	while ( cmd.size() > 0 )
	{
		w1 = upper( word( cmd, 1 ) ) ;
		p1 = w1.find_first_of( "'\"" ) ;
		if ( p1 != string::npos )
		{
			p2 = cmd.find( w1[ p1 ], p1+1 ) ;
			if ( p2 == string::npos )
			{
				pcmd.set_msg( "PEDT011H" ) ;
				return false ;
			}
			p3   = cmd.find( ' ', p2 ) ;
			pre1 = cmd.substr( 0, p1 ) ;
			pre2 = cmd.substr( p2+1, p3-p2-1 ) ;
			trim( pre2 ) ;
			if ( pre1 != "" && pre2 != "" )
			{
				pcmd.set_msg( "PEDT011I" ) ; return false ;
			}
			pre1 += pre2 ;
			iupper( pre1 ) ;
			if ( !t_str1 )
			{
				t_str1 = true ;
				if      ( pre1 == ""   ) { t.f_text = true ; }
				else if ( pre1 == "T"  ) { t.f_text = true ; }
				else if ( pre1 == "C"  ) { t.f_asis = true ; }
				else if ( pre1 == "X"  ) { t.f_hex  = true ; }
				else if ( pre1 == "P"  ) { t.f_pic  = true ; }
				else if ( pre1 == "R"  ) { t.f_reg  = true ; }
				else if ( pre1 == "RC" ) { t.f_reg  = true ; t.f_asis = true ; }
				else                     { pcmd.set_msg( "PEDT011I" ) ; return false ; }
				t.f_string = cmd.substr( (p1+1), (p2-p1-1) ) ;
				if ( t.f_text ) { iupper( t.f_string ) ; }
			}
			else if ( !t_str2 )
			{
				if ( type != 'C' ) { pcmd.set_msg( "PEDT011B" ) ; return false ; }
				t_str2 = true ;
				if      ( pre1 == ""   ) { t.f_ctext = true ; }
				else if ( pre1 == "T"  ) { t.f_ctext = true ; }
				else if ( pre1 == "C"  ) { t.f_casis = true ; }
				else if ( pre1 == "X"  ) { t.f_chex  = true ; }
				else if ( pre1 == "P"  ) { t.f_cpic  = true ; }
				else                     { pcmd.set_msg( "PEDT011I" ) ; return false ; }
				t.f_cstring = cmd.substr( (p1+1), (p2-p1-1) ) ;
				if ( profCaps && t.f_ctext ) { iupper( t.f_cstring ) ; }
			}
			else
			{
				pcmd.set_msg( "PEDT011" ) ;
				return false ;
			}
			cmd.erase( 0, p3 ) ;
			trim_left( cmd )   ;
			continue ;
		}
		if ( findword( w1, f_keywdir ) )
		{
			if ( t_dir ) { pcmd.set_msg( "PEDT013J" ) ; return false ; }
			t_dir   = true ;
			t.f_dir = w1.front() ;
		}
		else if ( findword( w1, f_keywexcl ) )
		{
			if ( t_excl || type == 'X' ) { pcmd.set_msg( "PEDT013J" ) ; return false ; }
			t_excl   = true ;
			t.f_excl = w1.front() ;
		}
		else if ( findword( w1, f_keywmtch ) )
		{
			if ( t_mtch ) { pcmd.set_msg( "PEDT013J" ) ; return false ; }
			t_mtch   = true ;
			t.f_mtch = w1.front() ;
		}
		else if ( datatype( w1, 'W' ) || w1.front() == '.' )
		{
			rcmd += " " + w1 ;
		}
		else if ( !t_str1 )
		{
			t_str1 = true ;
			if ( w1 == "*" )
			{
				if ( fcx_parms.f_string == "" ) { pcmd.set_msg( "PEDT011C" ) ; return false ; }
				t.f_string = fcx_parms.f_string ;
				t.f_regreq = fcx_parms.f_regreq ;
				t.f_mtch   = fcx_parms.f_mtch   ;
				t.f_text   = fcx_parms.f_text   ;
				t.f_asis   = fcx_parms.f_asis   ;
				t.f_hex    = fcx_parms.f_hex    ;
				t.f_pic    = fcx_parms.f_pic    ;
			}
			else
			{
				t.f_text   = true ;
				t.f_string = w1   ;
			}
		}
		else if ( !t_str2 )
		{
			if ( type != 'C' ) { pcmd.set_msg( "PEDT011B" ) ; return false ; }
			t.f_cstring = profCaps ? w1 : word( cmd, 1 ) ;
			t.f_ctext   = true ;
			t_str2      = true ;
		}
		else
		{
			pcmd.set_msg( "PEDT013J" ) ;
			return false ;
		}
		idelword( cmd, 1, 1 ) ;
		trim( cmd ) ;
	}

	if ( t.f_string == "" || ( type == 'C' && !t_str2 ) )
	{
		pcmd.set_msg( "PEDT011D" ) ;
		return false ;
	}
	t.f_ostring = t.f_string ;

	if ( t.f_reg ) { t.f_regreq = true ; }

	if ( !setCommandRange( rcmd, r ) ) { return false ; }
	t.f_slab = r.c_slab ;
	t.f_elab = r.c_elab ;
	t.f_scol = r.c_scol ;
	t.f_ecol = r.c_ecol ;
	t.f_ocol = r.c_ocol ;

	if ( t.f_hex )
	{
		if ( !ishex( t.f_string ) ) { pcmd.set_msg( "PEDT011K" ) ; return false ; }
		t.f_string = xs2cs( t.f_string ) ;
		t.f_asis   = true                ;
		iupper( t.f_ostring )            ;
	}

	if ( t.f_chex )
	{
		if ( !ishex( t.f_cstring ) ) { pcmd.set_msg( "PEDT011K" ) ; return false ; }
		t.f_cstring = xs2cs( t.f_cstring ) ;
		t.f_casis   = true                 ;
	}

	if ( t.f_cpic )
	{
		// =  don't change character
		// <  change to lower case alphabetic
		// >  change to upper case alphabetic
		if ( t.f_reg )
		{
			pcmd.set_msg( "PEDT016A" ) ;
			return false ;
		}
		for ( int i = 0 ; i < t.f_cstring.size() ; i++ )
		{
			switch ( t.f_cstring[ i ] )
			{
				case '=':
				case '<':
				case '>':
					break ;
				case '@':
				case '#':
				case '$':
				case '¬':
				case '.':
				case '-':
					pcmd.set_msg( "PEDT015X" ) ;
					return false ;
			}
		}
		if ( t.f_cstring.size() != t.f_string.size() )
		{
			pcmd.set_msg( "PEDT015Y" ) ;
			return false ;
		}
	}

	if ( t.f_pic )
	{
		pic = "" ;
		// =  any character                   .  invalid characters (dot, x'2E')
		// @  alphabetic characters           -  non-numeric characters
		// #  numeric characters              <  lower case alphabetics
		// $  special characters              >  upper case alphabetics
		// ¬  non-blank characters (x'AC')    *  any number of non-blank characters
		for ( int i = 0 ; i < t.f_string.size() ; i++ )
		{
			switch ( t.f_string[ i ] )
			{
				case '*':
					if ( t.f_cpic )
					{
						pcmd.set_msg( "PEDT015Z" ) ;
						return false ;
					}
					pic += "[^[:blank:]]*" ;
					break ;
				case '=':
					pic += "." ;
					break ;
				case '@':
					pic += "[[:alpha:]]" ;
					break ;
				case '#':
					pic += "[[:digit:]]" ;
					break ;
				case '$':
					pic += "[^[:blank:]^[:alpha:]^[:digit:]]" ;
					break ;
				case '¬':
					pic += "[^[:blank:]]" ;
					break ;
				case '.':
					pic += "[^[:print:]]" ;
					break ;
				case '-':
					pic += "[^[:digit:]]" ;
					break ;
				case '<':
					pic += "[a-z]" ;
					break ;
				case '>':
					pic += "[A-Z]" ;
					break ;
				default:
					pic += t.f_string[ i ] ;
					break ;
			}
		}
		t.f_string = pic  ;
		t.f_regreq = true ;
	}

	switch ( t.f_mtch )
	{
		case 'P':
			t.f_string = "\\b" + t.f_string + "\\w" ;
			t.f_regreq = true ;
			break ;
		case 'S':
			t.f_string = "\\w" + t.f_string + "\\b" ;
			t.f_regreq = true ;
			break ;
		case 'W':
			t.f_string = "\\b" + t.f_string + "\\b" ;
			t.f_regreq = true ;
	}

	t.f_fset = true ;
	if ( type == 'C')
	{
		t.f_cset = true ;
		if ( t.f_dir == 'A' )
		{
			t.f_dir     = 'F'  ;
			t.f_chngall = true ;
		}
	}
	else if ( type == 'X')
	{
		t.f_exclude = true ;
	}
	t.f_ssize = t.f_string.size() ;
	fcx_parms = t ;

	return true ;
}


void PEDIT01::hiliteFindPhrase()
{
	// c1,c2 are the column limits of the find, set by bnds, col, line length and initial cursor position
	// hilite FIND phrase between normal limits (bounds, range) between first and last line on screen

	int i    ;
	int ln   ;
	int s1   ;
	int c1   ;
	int c2   ;
	int p1   ;
	int scol ;
	int ecol ;

	uint dl  ;
	uint sdl ;
	uint edl ;

	string str ;

	bool found ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	boost::regex  regexp  ;
	boost::smatch results ;

	if ( fcx_parms.f_regreq )
	{
		try
		{
			if ( fcx_parms.f_asis )
			{
				regexp.assign( fcx_parms.f_string ) ;
			}
			else
			{
				regexp.assign( fcx_parms.f_string, boost::regex_constants::icase ) ;
			}
		}
		catch  ( boost::regex_error& e )
		{
			fcx_parms.f_error = true ;
			pcmd.set_msg( "PEDT011N" ) ;
			return ;
		}
	}

	sdl = ( fcx_parms.f_slab != "" ) ? getLabelIndex( fcx_parms.f_slab ) : topLine ;
	edl = ( fcx_parms.f_elab != "" ) ? getLabelIndex( fcx_parms.f_elab ) : data.size() - 2 ;

	scol = max( fcx_parms.f_scol, LeftBnd ) - 1 ;
	if ( fcx_parms.f_ecol == 0 )
	{
		ecol = ( RightBnd > 0 ) ? RightBnd - 1 : ecol = 0 ;
	}
	else
	{
		ecol = ( RightBnd > 0 ) ? min( fcx_parms.f_ecol, RightBnd ) - 1 : fcx_parms.f_ecol - 1 ;
	}

	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		dl = s2data.at( i ).ipos_dl ;

		if ( s2data.at( i ).ipos_URID == 0 ||
		     s2data.at( i ).ipos_hex > 0   ||
		     s2data.at( i ).ipos_div       ||
		     dl < sdl                      ||
		     !data[ dl ]->isValidFile()    ||
		      data[ dl ]->il_excl )     { continue ; }

		if ( dl > edl ) { break ; }

		c1 = 0 ;
		c2 = data[ dl ]->get_idata_len() - 1 ;

		if ( scol > c1 )             { c1 = scol ; }
		if ( ecol > 0 && ecol < c2 ) { c2 = ecol ; }

		if ( c1 > c2 ) { continue ; }

		if ( fcx_parms.f_regreq )
		{
			itss = data[ dl ]->get_idata_begin() ;
			std::advance( itss, c1 ) ;
			itse = itss ;
			std::advance( itse, c2-c1+1 ) ;
			while ( regex_search( itss, itse, results, regexp ) )
			{
				if ( fcx_parms.f_ocol && itss != results[ 0 ].first )
				{
					break ;
				}
				for ( p1 = c1 ; itss != results[ 0 ].first ; itss++ ) { p1++ ; }
				c1    = p1 + 1 ;
				itss  = results[ 0 ].first ;
				if ( itss == itse ) { break ; }
				itss++ ;
				str = results[ 0 ] ;
				ln  = str.size()   ;
				s1  = p1 - startCol + 1 ;
				if ( s1 < 0 )
				{
					ln = ln + s1 ;
					if ( ln <= 0 ) { continue ; }
					s1 = 0 ;
				}
				ZSHADOW.replace( (ZAREAW*i + CLINESZ + s1 ), ln, string( ln, R_WHITE ) ) ;
				if ( fcx_parms.f_ocol ) { break ; }
			}
		}
		else
		{
			while ( true )
			{
				found = false ;
				if ( fcx_parms.f_asis )
				{
					p1 = data[ dl ]->get_idata_ptr()->find( fcx_parms.f_string, c1 )  ;
				}
				else
				{
					p1 = upper( data[ dl ]->get_idata()).find( fcx_parms.f_string, c1 ) ;
				}
				if ( p1 != string::npos )
				{
					if ( fcx_parms.f_ocol )
					{
						if ( p1 == fcx_parms.f_scol-1 ) { found = true ; }
					}
					else
					{
						if ( p1 + fcx_parms.f_ssize - 1 <= c2 ) { found = true ; }
						c1 = p1 + 1 ;
					}
				}
				if ( !found ) { break ; }
				ln = fcx_parms.f_ssize ;
				s1 = p1 - startCol + 1 ;
				if ( s1 < 0 )
				{
					ln = ln + s1 ;
					if ( ln <= 0 ) { continue ; }
					s1 = 0 ;
				}
				ZSHADOW.replace( (ZAREAW*i + CLINESZ + s1 ), ln, string( ln, R_WHITE ) ) ;
				if ( fcx_parms.f_ocol ) { break ; }
			}
		}
	}
}


void PEDIT01::actionFind()
{
	// dl has to be a uint so the corect getNextDataLine routine is called ( int is for URID )
	// c1,c2 are the column limits of the find, set by bnds, col, line length and initial cursor position

	// If running under an edit macro, use mRow, mCol instead of aRow, aCol

	int Col  ;
	int c1   ;
	int c2   ;
	int p1   ;
	int p2   ;
	int scol ;
	int ecol ;
	int URID ;
	int offset ;

	uint dl  ;
	uint sdl ;
	uint edl ;

	bool found ;
	bool fline ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	boost::regex  regexp  ;
	boost::smatch results ;

	Col = macroRunning ? ( mCol + 9 ) : aCol ;

	if ( Col == 0 && fcx_parms.f_dir == 'P' && topLine <= getDataLine( 1 ) )
	{
		fcx_parms.f_dir = 'L' ;
	}

	if ( !fcx_parms.f_success && ( fcx_parms.f_dir == 'N' || fcx_parms.f_dir == 'P' ) )
	{
		if ( fcx_parms.f_pURID    == aURID &&
		     fcx_parms.f_pCol     == ( aCol + startCol ) &&
		     fcx_parms.f_ptopLine == topLine )
		{
			fcx_parms.f_dir = ( fcx_parms.f_dir == 'N' ) ? 'F' : 'L' ;
		}
	}

	fcx_parms.f_pURID    = aURID ;
	fcx_parms.f_pCol     = aCol + startCol ;
	fcx_parms.f_ptopLine = topLine ;

	fcx_parms.reset() ;

	offset = 0 ;
	sdl    = 0 ;
	edl    = data.size() - 1 ;

	if ( Col > 0 )
	{
		offset = startCol + Col - CLINESZ - 1 ;
	}

	if ( fcx_parms.f_dir == 'F' || fcx_parms.f_dir == 'A' || fcx_parms.f_dir == 'L' )
	{
		offset = 0 ;
	}

	if ( fcx_parms.f_regreq )
	{
		try
		{
			if ( fcx_parms.f_asis )
			{
				regexp.assign( fcx_parms.f_string ) ;
			}
			else
			{
				regexp.assign( fcx_parms.f_string, boost::regex_constants::icase ) ;
			}
		}
		catch  ( boost::regex_error& e )
		{
			fcx_parms.f_error = true ;
			pcmd.set_msg( "PEDT011N" ) ;
			return ;
		}
	}

	if ( fcx_parms.f_dir == 'F' )
	{
		dl   = 1 ;
		aRow = 0 ;
	}
	else if ( fcx_parms.f_dir == 'A' )
	{
		dl = 1 ;
	}
	else if ( fcx_parms.f_dir == 'L' )
	{
		dl = data.size() - 2 ;
	}
	else if ( macroRunning )
	{
		dl = getDataLine( mRow ) ;
	}
	else if ( aRow == 0 )
	{
		dl = ( fcx_parms.f_dir == 'P' ) ? topLine - 1 : topLine ;
		if ( fcx_parms.f_dir == 'N' && data[ dl ]->is_bod() )
		{
			fcx_parms.f_searched = true ;
			return ;
		}
	}
	else
	{
		URID = s2data.at( aRow-1 ).ipos_URID ;
		dl   = s2data.at( aRow-1 ).ipos_dl   ;
		if ( fcx_parms.f_dir == 'N' && ( URID == 0 || data[ dl ]->is_bod() ) )
		{
			fcx_parms.f_searched = true ;
			return ;
		}
		if ( fcx_parms.f_dir == 'P' && ( URID == 0 || data[ dl ]->is_tod() ) )
		{
			dl = data.size() - 1 ;
		}
	}

	if ( fcx_parms.f_slab != "" )
	{
		sdl = getLabelIndex( fcx_parms.f_slab ) ;
		edl = getLabelIndex( fcx_parms.f_elab ) ;
	}

	sdl = getValidFileLineNext( sdl ) ;
	edl = getValidFileLinePrev( edl ) ;

	if ( dl < sdl )
	{
		fcx_parms.f_top = true ;
		if ( fcx_parms.findReverse() )
		{
			fcx_parms.f_searched = true ;
			return ;
		}
		dl     = sdl ;
		offset = 0   ;
	}
	else if ( dl > edl )
	{
		fcx_parms.f_bottom = true ;
		if ( !fcx_parms.findReverse() )
		{
			fcx_parms.f_searched = true ;
			return ;
		}
		dl     = edl ;
		offset = 0   ;
	}

	scol = max( fcx_parms.f_scol, LeftBnd ) - 1 ;
	if ( fcx_parms.f_ecol == 0 )
	{
		ecol = ( RightBnd > 0 ) ? RightBnd - 1 : ecol = 0 ;
	}
	else
	{
		ecol = ( RightBnd > 0 ) ? min( fcx_parms.f_ecol, RightBnd ) - 1 : fcx_parms.f_ecol - 1 ;
	}

	optFindPhrase = true  ;
	found         = false ;

	while ( dl != 0 && dl >= sdl && dl <= edl )
	{
		if ( (fcx_parms.f_excl == 'X' && !data[ dl ]->il_excl) ||
		     (fcx_parms.f_excl == 'N' &&  data[ dl ]->il_excl) )
		{
			dl = ( fcx_parms.findReverse() ) ? getPrevFileLine( dl ) : getNextFileLine( dl ) ;
			offset = 0 ;
			continue   ;
		}

		if ( offset == 0 )
		{
			if ( dl == sdl )
			{
				fcx_parms.f_top = true ;
			}
			if ( dl == edl )
			{
				fcx_parms.f_bottom = true ;
			}
		}

		if ( fcx_parms.f_dir == 'P' )
		{
			c1 = 0 ;
			c2 = (offset == 0) ? data[ dl ]->get_idata_len() - 1 : offset - 2 ;
			offset = 0 ;
		}
		else
		{
			c1 = offset ;
			c2 = data[ dl ]->get_idata_len() - 1 ;
		}

		if ( scol > c1 )             { c1 = scol ; }
		if ( ecol > 0 && ecol < c2 ) { c2 = ecol ; }

		fcx_parms.f_searched = true ;
		if ( c1 > c2 || offset > c2 )
		{
			dl = ( fcx_parms.findReverse() ) ? getPrevFileLine( dl ) : getNextFileLine( dl ) ;
			offset = 0 ;
			continue   ;
		}

		fline = true  ;
		found = false ;
		if ( fcx_parms.f_regreq )
		{
			itss = data[ dl ]->get_idata_begin() ;
			std::advance( itss, c1 ) ;
			if ( fcx_parms.f_dir == 'P' )
			{
				itse = data[ dl ]->get_idata_end() ;
			}
			else
			{
				itse = itss ;
				std::advance( itse, c2-c1+1 ) ;
			}
			if ( fcx_parms.f_dir == 'P' || fcx_parms.f_dir == 'L' || fcx_parms.f_dir == 'A' )
			{
				while ( regex_search( itss, itse, results, regexp ) )
				{
					if ( fcx_parms.f_ocol && itss != results[ 0 ].first )
					{
						break ;
					}
					for ( p2 = c1 ; itss != results[ 0 ].first ; itss++ ) { p2++ ; }
					if ( fcx_parms.f_dir == 'P' && p2 > c2 )
					{
						break ;
					}
					found = true ;
					p1    = p2   ;
					c1    = p2+1 ;
					if ( !pcmd.isSeek() )
					{
						data.at( dl )->il_excl = fcx_parms.f_exclude ;
					}
					fcx_parms.setrstring( results[ 0 ] ) ;
					if ( fline )
					{
						fline = false ;
						fcx_parms.f_lines++ ;
					}
					fcx_parms.f_occurs++ ;
					if ( fcx_parms.f_dir == 'A' && fcx_parms.f_URID == 0 )
					{
						fcx_parms.f_URID   = data.at( dl )->il_URID ;
						fcx_parms.f_dl     = dl ;
						fcx_parms.f_offset = p1 ;
					}
					if ( fcx_parms.f_ocol ) { break ; }
					itss = results[ 0 ].first ;
					if ( itss == itse ) { break ; }
					itss++ ;
				}
				if ( fcx_parms.f_dir == 'A' ) { found = false ; }
			}
			else
			{
				if ( boost::regex_search( itss, itse, results, regexp ) )
				{
					if ( fcx_parms.f_ocol && itss != results[ 0 ].first ) {}
					else
					{
						found = true ;
						if ( !pcmd.isSeek() )
						{
							data.at( dl )->il_excl = fcx_parms.f_exclude ;
						}
						fcx_parms.setrstring( results[ 0 ] ) ;
						for ( p1 = c1 ; itss  != results[ 0 ].first ; itss++ ) { p1++ ; }
					}
				}
			}
			if ( found && fcx_parms.f_mtch == 'S' ) { p1++ ; }
			if ( found && fcx_parms.f_URID == 0 )
			{
				fcx_parms.f_URID   = data.at( dl )->il_URID ;
				fcx_parms.f_dl     = dl ;
				fcx_parms.f_offset = p1 ;
			}
		}
		else
		{
			while ( true )
			{
				found = false ;
				if ( fcx_parms.findReverse() )
				{
					if ( fcx_parms.f_asis )
					{
						p1 = data[ dl ]->get_idata_ptr()->rfind( fcx_parms.f_string, c2 ) ;
					}
					else
					{
						p1 = upper( data[ dl ]->get_idata()).rfind( fcx_parms.f_string, c2 ) ;
					}
				}
				else
				{
					if ( fcx_parms.f_asis )
					{
						p1 = data[ dl ]->get_idata_ptr()->find( fcx_parms.f_string, c1 )  ;
					}
					else
					{
						p1 = upper( data[ dl ]->get_idata()).find( fcx_parms.f_string, c1 ) ;
					}
				}
				if ( p1 != string::npos )
				{
					if ( fcx_parms.f_ocol )
					{
						if ( p1 == fcx_parms.f_scol-1 ) { found = true ; }
					}
					else if ( fcx_parms.findReverse() )
					{
						if ( p1 >= c1 )
						{
							if ( ecol == 0 || (p1 + fcx_parms.f_ssize - 1) <= ecol )
							{
								found = true ;
							}
						}
						c2 = p1 - 1 ;
					}
					else
					{
						if ( p1 + fcx_parms.f_ssize - 1 <= c2 ) { found = true ; }
						c1 = p1 + 1 ;
					}
				}
				if ( found )
				{
					if ( fcx_parms.f_URID == 0 )
					{
						fcx_parms.f_URID   = data.at( dl )->il_URID ;
						fcx_parms.f_dl     = dl ;
						fcx_parms.f_offset = p1 ;
					}
					if ( !pcmd.isSeek() )
					{
						data.at( dl )->il_excl = fcx_parms.f_exclude ;
					}
					if ( fcx_parms.f_rstring == "" && !fcx_parms.f_asis )
					{
						fcx_parms.f_rstring = data[ dl ]->get_idata_ptr()->substr( p1, fcx_parms.f_ssize ) ;
					}
				}
				if ( fcx_parms.f_dir != 'A' || !found ) { break ; }
				fcx_parms.f_occurs++ ;
				if ( fline )
				{
					fline = false ;
					fcx_parms.f_lines++ ;
				}
			}
		}
		if ( fcx_parms.f_dir == 'A' )
		{
			dl = getNextFileLine( dl ) ;
		}
		else if ( fcx_parms.findReverse() )
		{
			fcx_parms.f_dir = 'P' ;
			if ( found ) { break ; }
			dl = getPrevFileLine( dl ) ;
		}
		else
		{
			fcx_parms.f_dir = 'N' ;
			if ( found ) { break ; }
			dl = getNextFileLine( dl ) ;
		}
		offset = 0 ;
	}

	fcx_parms.f_success = found ;
	if ( fcx_parms.f_dir == 'A' && fcx_parms.f_occurs == 0 )
	{
		fcx_parms.f_dir = 'N' ;
	}
}


void PEDIT01::actionChange()
{
	//  If updating the same line multiple times with the same Level (eg. change ALL command)
	//  update idata stack in-place.

	//  If a regex has been used as the find argument, use fcx_rstring instead of fcx_string.
	//  If the inserted string is longer, remove some spaces at the end of the insert to compensate.

	int i ;
	int j ;
	int l ;
	int d ;
	int osz ;

	string temp ;

	boost::regex regexp ;
	boost::smatch results ;

	l    = getLine( fcx_parms.f_URID ) ;
	temp = data.at( l )->get_idata()   ;
	osz  = temp.size() ;

	if ( fcx_parms.f_cpic )
	{
		for ( i = 0 ; i < fcx_parms.f_cstring.size() ; i++ )
		{
			if ( fcx_parms.f_cstring[ i ] == '=' )
			{
				continue ;
			}
			else if ( fcx_parms.f_cstring[ i ] == '<' )
			{
				ilower( temp[ fcx_parms.f_offset + i ] ) ;
			}
			else if ( fcx_parms.f_cstring[ i ] == '>' )
			{
				iupper( temp[ fcx_parms.f_offset + i ] ) ;
			}
			else
			{
				temp[ fcx_parms.f_offset + i ] = fcx_parms.f_cstring[ i ] ;
			}
		}
		if ( data.at( l )->get_idata_level() == Level )
		{
			data.at( l )->put_idata( temp ) ;
			data.at( l )->setChngStatus() ;
		}
		else
		{
			if ( data.at( l )->put_idata( temp, Level ) )
			{
				fileChanged = true ;
			}
			data.at( l )->setChngStatus() ;
		}
		return ;
	}

	if ( fcx_parms.f_mtch == 'P' )
	{
		fcx_parms.f_rstring.pop_back() ;
		temp.replace( fcx_parms.f_offset, fcx_parms.f_rstring.size(), fcx_parms.f_cstring ) ;
	}
	else if ( fcx_parms.f_mtch == 'S' )
	{
		fcx_parms.f_rstring.erase( 0, 1 ) ;
		temp.replace( fcx_parms.f_offset, fcx_parms.f_rstring.size(), fcx_parms.f_cstring ) ;
	}
	else if ( fcx_parms.f_regreq )
	{
		if ( fcx_parms.f_asis )
		{
			regexp.assign( fcx_parms.f_string ) ;
		}
		else
		{
			regexp.assign( fcx_parms.f_string, boost::regex_constants::icase ) ;
		}
		temp = regex_replace( temp, regexp, fcx_parms.f_cstring, boost::format_first_only ) ;
	}
	else
	{
		temp.replace( fcx_parms.f_offset, fcx_parms.f_ssize, fcx_parms.f_cstring ) ;
	}

	d = temp.size() - osz ;
	i = temp.find( ' ', fcx_parms.f_offset + fcx_parms.f_cstring.size() ) ;
	if ( d > 0 && i != string::npos )
	{
		j = temp.find_first_not_of( ' ', i ) ;
		if ( j != string::npos && (j - i) > 1 )
		{
			temp.erase( i, min( d, (j - i - 1) ) ) ;
		}
	}

	if ( data.at( l )->get_idata_level() == Level )
	{
		data.at( l )->put_idata( temp ) ;
		data.at( l )->setChngStatus()   ;
	}
	else
	{
		if ( data.at( l )->put_idata( temp, Level ) )
		{
			fileChanged = true ;
		}
		data.at( l )->setChngStatus() ;
	}
}


bool PEDIT01::setCommandRange( string s, cmd_range& t )
{
	// Extract the command range ( start/end labels and start/end columns or on column ) from
	// a command and return data in t.
	// If there is a start label, but not end label, set to .ZLAST
	// c_vlab true if labels can be entered on the command
	// c_vcol true if columns can be entered on the command

	int j ;
	int k ;

	string w ;

	t.cmd_range_clear() ;

	while ( true )
	{
		w = word( s, 1 ) ;
		if ( w == "" ) { break ; }
		if ( datatype( w, 'W' ) )
		{
			if ( !t.c_vcol ) { pcmd.set_msg( "PEDT013T" ) ; return false ; }
			if ( t.c_scol != 0 && t.c_ecol != 0 ) { pcmd.set_msg( "PEDT019" ) ; return false ; }
			j = ds2d( w ) ;
			if ( j < 1 || j > 65535 ) { pcmd.set_msg( "PEDT011J" ) ; return false ; }
			t.c_scol == 0 ? t.c_scol = j : t.c_ecol = j ;
		}
		else if ( w.front() == '.' )
		{
			if ( !t.c_vlab ) { pcmd.set_msg( "PEDT013S" ) ; return false ; }
			if ( t.c_slab != "" && t.c_elab != "" )  { pcmd.set_msg( "PEDT011A" ) ; return false ; }
			t.c_slab == "" ? t.c_slab = w : t.c_elab = w ;
		}
		else { pcmd.set_msg( "PEDT011" ) ; return false ; }
		idelword( s, 1, 1 ) ;
	}

	if ( t.c_scol != 0 && t.c_ecol == 0 )
	{
		t.c_ocol = true ;
	}
	else if ( t.c_scol > t.c_ecol )
	{
		swap( t.c_scol, t.c_ecol ) ;
	}

	if ( t.c_scol == 0 ) { t.c_scol = 1 ; }

	if ( t.c_slab == ".ZLAST" && t.c_elab == "" )
	{
		pcmd.set_msg( "PEDT013R" ) ;
		return false ;
	}

	if ( t.c_slab != "" )
	{
		if ( t.c_elab == "" )       { t.c_elab = ".ZLAST"                       ; }
		if ( t.c_slab == t.c_elab ) { pcmd.set_msg( "PEDT011G" ) ; return false ; }
		j = getLabelIndex( t.c_slab ) ;
		if ( j < 0 ) { pcmd.set_msg( "PEDT011F" ) ; return false ; }
		t.c_sidx = j ;
		k = getLabelIndex( t.c_elab ) ;
		if ( k < 0 ) { pcmd.set_msg( "PEDT011F" ) ; return false ; }
		t.c_eidx = k ;
		if ( j > k )
		{
			t.c_slab.swap( t.c_elab )  ;
			swap( t.c_sidx, t.c_eidx ) ;
		}
	}
	return true ;
}


void PEDIT01::moveColumn( int p )
{
	// reposition startCol if the find/change string is outside ZAREA

	int o ;

	o = p - startCol + 1 ;
	if ( o < 0 )
	{
		if ( p < ZAREAW ) { startCol = 1      ; }
		else              { startCol = p - 13 ; }
		if ( startCol < 0 ) { startCol = 1    ; }
		rebuildZAREA = true ;
	}
	else if ( o > ZDATAW )
	{
		startCol     = p - 13 ;
		if ( startCol < 0 ) { startCol = 1 ; }
		rebuildZAREA = true ;
	}
}


void PEDIT01::clearCursor()
{
	// cursorPlaceUsing: 1 Use URID to place cursor
	//                   2 Use screen row to place cursor

	// cursorPlaceType   1 First char of the line command area
	//                   2 First char of data area
	//                   3 First non-blank char of data area (after startCol)
	//                   4 Use position in cursorPlaceOff as start of data line
	//                   5 Use position in cursorPlaceOff as start of ZDATAW
	// cursorPlaceOff    Offset from start of the data line (adjust for line command and startCol later)

	cursorPlaceHome = false ;
	cursorPlaceURID = 0     ;
	cursorPlaceRow  = 0     ;
}


void PEDIT01::placeCursor( int URID, int pt, int offset )
{
	if ( URID == 0 )
	{
		cursorPlaceHome = true ;
		return ;
	}
	cursorPlaceHome  = false  ;
	cursorPlaceUsing = 1      ;
	cursorPlaceURID  = URID   ;
	cursorPlaceRow   = 0      ;
	cursorPlaceType  = pt     ;
	cursorPlaceOff   = offset ;
}


void PEDIT01::placeCursor( uint Row, int pt, int offset )
{
	if ( Row > ZAREAD || s2data.at( Row-1 ).ipos_URID == 0 )
	{
		cursorPlaceHome = true ;
		return ;
	}
	cursorPlaceHome  = false  ;
	cursorPlaceUsing = 2      ;
	cursorPlaceURID  = 0      ;
	cursorPlaceRow   = Row    ;
	cursorPlaceType  = pt     ;
	cursorPlaceOff   = offset ;
}


void PEDIT01::storeCursor( int URID, int offset )
{
	// Store current cursor position using PlaceType=4 (position in cursorPlaceOff)

	cursorPlaceUsing = 1      ;
	if ( URID == 0 ) { cursorPlaceHome = true ; }
	else             { cursorPlaceURID = URID ; cursorPlaceURIDO = URID ; }
	cursorPlaceRow   = 0      ;
	cursorPlaceType  = 4      ;
	cursorPlaceOff   = offset ;
	cursorPlaceOffO  = offset ;
}


void PEDIT01::restoreCursor()
{
	// Restore current cursor position using PlaceType=4 (position in cursorPlaceOff) and
	// data stored by storeCursor()

	if ( cursorPlaceURIDO == 0 ) { cursorPlaceHome = true ; return ; }
	cursorPlaceUsing = 1      ;
	cursorPlaceHome  = false  ;
	cursorPlaceRow   = 0      ;
	cursorPlaceType  = 4      ;
	cursorPlaceOff   = cursorPlaceOffO  ;
	cursorPlaceURID  = cursorPlaceURIDO ;
}


void PEDIT01::positionCursor()
{
	// Move cursor to its set position
	// Highlight cursor phrase if option set
	// Highlight FIND phrase if option set

	int i  ;
	int j  ;
	int p  ;
	int dl ;
	int o  ;
	int screenLine ;

	const string delim = "\x01\x02\x20" ;

	if ( cursorPlaceHome ) { CURFLD = "ZCMD" ; CURPOS = 1 ; return ; }

	screenLine = -1 ;

	switch ( cursorPlaceUsing )
	{
		case 1:
			dl = getLine( cursorPlaceURID ) ;
			if ( data.at( dl )->il_excl )
			{
				dl = getFirstEX( dl ) ;
				cursorPlaceURID = data.at( dl )->il_URID ;
			}
			for ( i = 0 ; i < ZAREAD ; i++ )
			{
				if ( s2data.at( i ).ipos_URID == cursorPlaceURID )
				{
					screenLine = i ;
					break ;
				}
			}
			break ;

		case 2:
			screenLine = cursorPlaceRow - 1 ;
	}

	if ( screenLine == -1 ) { CURFLD = "ZCMD" ; CURPOS = 1 ; return ; }

	switch ( cursorPlaceType )
	{
		case 1:
			CURFLD = "ZAREA" ;
			CURPOS = ZAREAW * screenLine + 2 ;
			break ;

		case 2:
			CURFLD = "ZAREA" ;
			CURPOS = ZAREAW * screenLine + 1 + CLINESZ ;
			break ;

		case 3:
			CURFLD = "ZAREA" ;
			dl = s2data.at( screenLine ).ipos_dl ;
			p  = data.at( dl )->get_idata_ptr()->find_first_not_of( ' ', startCol-1 ) ;
			p  = (p != string::npos) ? p + CLINESZ + 2 - startCol : CLINESZ + 1 ;
			CURPOS = ZAREAW * screenLine + p ;
			break ;

		case 4:
			o = cursorPlaceOff - startCol + CLINESZ + 2 ;
			if ( o < 1 || o > ZAREAW )
			{
				CURFLD = "ZAREA" ;
				CURPOS = ZAREAW * screenLine + CLINESZ + 1 ;
			}
			else
			{
				CURFLD = "ZAREA" ;
				CURPOS = ZAREAW * screenLine + o ;
			}
			break ;

		case 5:
			CURFLD = "ZAREA" ;
			CURPOS = ZAREAW * screenLine + CLINESZ + 1 + cursorPlaceOff ;
	}

	ZSHADOW.replace( ZAREAW*screenLine, slr.size()-1, slr.size()-1, N_RED ) ;
	rebuildShadow = true ;

	if ( profCsrPhrase && ((CURPOS-1) % ZAREAW) >= CLINESZ )
	{
		dl = s2data.at( screenLine ).ipos_dl ;
		if ( data.at( dl )->isValidFile() )
		{
			i = ZAREA.find_first_of( delim, CURPOS-1 ) ;
			j = ZAREA.find_last_of( delim, CURPOS-1 )  ;
			if ( i == string::npos ) { i = ZAREA.size() ; }
			if ( j == string::npos ) { j = 0            ; }
			ZSHADOW.replace( j, i-j, i-j, B_WHITE ) ;
		}
	}
}


bool PEDIT01::formLineCmd( const string& cmd, string& lcc, int& rept)
{
	// Split line command into the command (string) and repeat value (int), if allowed, and return in lcc and retp
	// A 'rept' of -1 means it has not been entered.  Assume rept of 0 means it has not been entered.

	int    j ;
	string t ;

	t.assign( 6, ' ' )   ;
	lcc.assign( 6, ' ' ) ;

	for ( j = 0 ; j < cmd.size() ; j++ )
	{
		if ( isdigit( cmd[ j ] ) ) { t[ j ]   = cmd[ j ] ; }
		else                       { lcc[ j ] = cmd[ j ] ; }
	}

	trim( t )   ;
	trim( lcc ) ;

	if ( aliasLCMDS.count( lcc ) > 0 )
	{
		lcc = aliasLCMDS[ lcc ] ;
	}

	if ( t != "" )
	{
		if ( datatype( t, 'W' ) ) { rept = ds2d( t )                          ; }
		else                      { pcmd.set_msg( "PEDT012G" ) ; return false ; }
	}
	else { rept = -1 ; }

	if ( rept == 0 ) { rept = -1 ; }

	if ( LineCMDS.count( lcc ) == 0 )
	{
		pcmd.set_msg( "PEDT012" ) ;
		return false  ;
	}

	if ( rept > 0 && ReptOK.count( lcc ) == 0 )
	{
		pcmd.set_msg( "PEDT012A" ) ;
		return false  ;
	}

	return true ;
}


uint PEDIT01::getLine( int URID )
{
	// Return the data line index dl, for a given URID ( URID must exist )

	uint dl ;

	vector<iline *>::iterator it ;

	for ( dl = 0, it = data.begin() ; it != data.end() ; it++, dl++ )
	{
		if ( (*it)->il_URID == URID ) { break ; }
	}
	return dl ;
}


int PEDIT01::getFileLine( uint dl )
{
	// Return the file line that corresponts to data line index dl in the data vector
	// First file line is 0

	int fl ;

	vector<iline *>::iterator it ;

	for ( fl = 0, it = data.begin() ; it != data.end() && dl > 0 ; it++, dl-- )
	{
		if ( (*it)->isValidFile()) { fl++ ; }
	}
	return fl ;
}


uint PEDIT01::getDataLine( int fl )
{
	// Return the data vector line that corresponts to line fl in the file
	// If not found, return bottom-of-data line.  First file line is 1

	uint dl ;
	int j   ;

	j = 0 ;
	if ( fl == 0 ) { return data.size() - 1 ; }

	for ( dl = 0 ; dl < data.size() ; dl++ )
	{
		if ( data.at( dl )->isValidFile() ) { j++ ; }
		if ( fl == j ) { break ; }
	}
	if ( dl == data.size() ) { dl-- ; }
	return dl ;
}


uint PEDIT01::getPrevDataLine( uint l )
{
	// Return the previous non-deleted data vector line that corresponts to the line before l.

	if ( l == 0 ) { return 0 ; }

	for ( l-- ; l > 0 ; l-- )
	{
		if ( !data.at( l )->il_deleted ) { break ; }
	}
	return l ;
}


uint PEDIT01::getNextDataLine( uint l )
{
	// Return the next non-deleted data vector index that corresponts to the line after l.

	for ( l++ ; l < data.size() ; l++ )
	{
		if ( !data.at( l )->il_deleted ) { break ; }
	}
	return l ;
}


vector<iline * >::iterator PEDIT01::getNextDataLine( vector<iline * >::iterator it )
{
	// Return the next valid (ie. non-deleted) data line iterator after iterator it

	for ( it++ ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_deleted ) { break ; }
	}
	return it ;
}


uint PEDIT01::getNextFileLine( uint l )
{
	// Return the next non-deleted data vector line that corresponts to the line after l.
	// *File lines only*.  Return 0 if end of data or bottom of data reached

	if ( data.at( l )->is_bod() ) { return 0 ; }

	for ( l++ ; l < data.size() ; l++ )
	{
		if ( data.at( l )->isValidFile() ) { return l ; }
		if ( data.at( l )->is_bod() )      { return 0 ; }
	}
	return l ;
}


vector<iline * >::iterator PEDIT01::getNextFileLine( vector<iline * >::iterator it )
{
	// Return the next non-deleted data vector line iterator that corresponts to the line after iterator it.
	// *File lines only*.  Return data.end() if end of data or bottom of data reached

	for ( it++ ; it != data.end() ; it++ )
	{
		if ( (*it)->is_bod() )      { break     ; }
		if ( (*it)->isValidFile() ) { return it ; }
	}
	return data.end() ;
}


uint PEDIT01::getValidFileLinePrev( uint l )
{
	// Return a valid (ie. non-deleted) file data line on or before line l
	// *File lines only*.  Return 0 if top of data reached

	for ( ; l > 0 ; l-- )
	{
		if ( data.at( l )->isValidFile() ) { return l ; }
	}
	return 0 ;
}


vector<iline * >::iterator PEDIT01::getValidDataLineNext( vector<iline * >::iterator it )
{
	// Return a valid (ie. non-deleted) data vector iterator on or after iterator it

	for ( ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_deleted ) { break ; }
	}
	return it ;
}


uint PEDIT01::getValidFileLineNext( uint l )
{
	// Return a valid (ie. non-deleted) file data line on or after line l
	// *File lines only*.  Return 0 if end of data reached

	for ( ; l < data.size() - 1 ; l++ )
	{
		if ( data.at( l )->isValidFile() ) { return l ; }
		if ( data.at( l )->is_bod() )      { return 0 ; }
	}
	return 0 ;
}


uint PEDIT01::getPrevFileLine( uint l )
{
	// Return the previous non-deleted data vector index that corresponts to the file line before l.
	// *File lines only*

	if ( l == 0 ) { return 0 ; }

	for ( l-- ; l > 0 ; l-- )
	{
		if ( data.at( l )->isValidFile() ) { break ; }
	}
	return l ;
}


vector<iline *>::iterator PEDIT01::getPrevFileLine( vector<iline * >::iterator it )
{
	// Return the previous non-deleted data vector iterator that corresponts to the file line before iterator it.
	// *File lines only*.  If not found, return data.end()

	if ( it == data.begin() ) { return data.end() ; }

	for ( it-- ; it != data.begin() ; it-- )
	{
		if ( (*it)->is_tod() )      { break     ; }
		if ( (*it)->isValidFile() ) { return it ; }
	}
	return data.end() ;
}


vector<iline *>::iterator PEDIT01::getLineItr( uint dl )
{
	// Return the iterator for a data index

	auto it = data.begin() ;
	std::advance( it, dl ) ;
	return it ;
}


vector<iline *>::iterator PEDIT01::getLineItr( int URID )
{
	// Return the iterator for a URID.  URID must exist.

	for ( auto it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_URID == URID ) { return it ; }
	}
	llog( "E", "Logic error.  URID "<< URID <<" not found" <<endl ) ;
	abend() ;
}


vector<iline *>::iterator PEDIT01::getLineItr( int URID, vector<iline * >::iterator it )
{
	// Return the iterator for a URID, on or after iterator 'it' (eg. for finding eURID after sURID)
	// URID must come on or after the passed iterator position.

	for ( ; it != data.end() ; it++ )
	{
		if ( (*it)->il_URID == URID ) { return it ; }
	}
	llog( "E", "Logic error.  URID "<< URID <<" not found after iterator" <<endl ) ;
	abend() ;
}


bool PEDIT01::getLabelItr( const string& label, vector<iline * >::iterator& it, int& posn )
{
	// Return false for .ZFIRST, and .ZLAST if the data has no valid file lines

	int lvl ;

	vector<iline *>::iterator itt ;

	if ( findword( label, ".ZFIRST .ZF .ZLAST .ZL" ) )
	{
		if ( any_of( data.begin(), data.end(),
			[](iline *& a)
			{
				return a->isValidFile() ;
			} ) )
		{
			if ( label == ".ZFIRST" || label == ".ZF" )
			{
				for ( posn = 0, it = data.begin() ; it != data.end() ; it++, posn++ )
				{
					if ( (*it)->isValidFile() ) { return true ; }
				}
			}
			else
			{
				for ( posn = 0, it = data.begin() ; it != data.end() ; it++, posn++ )
				{
					if ( (*it)->is_bod() )      { it  = itt ; return true ; }
					if ( (*it)->isValidFile() ) { itt = it                ; }
				}
			}
		}
		else
		{
			return false ;
		}
	}

	for ( lvl = nestLevel ; lvl >= 0 ; lvl-- )
	{
		for ( posn = 0, it = data.begin() ; it != data.end() ; it++, posn++ )
		{
			if ( (*it)->il_deleted ) { continue ; }
			if ( (*it)->compareLabel( label, lvl ) ) { return true ; }
		}
	}
	return false ;
}


int PEDIT01::getLabelIndex( const string& label )
{
	// Return index of the data vector corresponding to the label

	// -1 if label not found
	// -2 for .ZFIRST, and .ZLAST if the data has no valid file lines

	int posn  ;
	int lvl   ;

	vector<iline *>::iterator it ;

	if ( findword( label, ".ZFIRST .ZF .ZLAST .ZL" ) )
	{
		if ( any_of( data.begin(), data.end(),
			[](iline *& a )
			{
				return a->isValidFile() ;
			} ) )
		{
			if ( label == ".ZFIRST" || label == ".ZF" )
			{
				for ( posn = 0, it = data.begin() ; it != data.end() ; it++, posn++ )
				{
					if ( (*it)->isValidFile() ) { return posn ; }
				}
			}
			else
			{
				for ( posn = data.size() - 1 ; ; posn-- )
				{
					if ( data.at( posn )->isValidFile() ) { return posn ; }
				}
			}
		}
		else
		{
			return -2 ;
		}
	}

	for ( lvl = nestLevel ; lvl >= 0 ; lvl-- )
	{
		for ( posn = 0, it = data.begin() ; it != data.end() ; it++, posn++ )
		{
			if ( (*it)->il_deleted ) { continue ; }
			if ( (*it)->compareLabel( label, lvl ) ) { return posn ; }
		}
	}
	return -1 ;
}


int PEDIT01::getLabelLine( const string& s )
{
	// Return index of the data vector for the line pointer (label or linenum)
	// Linenum is the file line, starting at 1

	// return  0  linenum is 0
	// return -1  label is not assigned to a line
	// return -2  linenum outside of data (bottom-of-data returned)
	// return -3  label does not exist (.ZFIRST, .ZLAST, .ZCSR)
	// return -4  line pointer is invalid

	int p ;
	int i ;

	if ( datatype( s, 'W' ) || s == ".ZCSR" )
	{
		if ( s == ".ZCSR" )
		{
			if ( any_of( data.begin(), data.end(),
				[](iline *& a )
				{
					return a->isValidFile() ;
				} ) )
			{
				i = mRow ;
			}
			else
			{
				return -2 ;
			}
		}
		else
		{
			i = ds2d( s ) ;
		}

		if ( i == 0 )
		{
			miBlock.seterror( "PEDM011S" ) ;
			return 0 ;
		}
		p = getDataLine( i ) ;
		if ( p == data.size() - 1 )
		{
			miBlock.seterror( "PEDM011S" ) ;
			return -2 ;
		}
	}
	else
	{
		if ( !checkLabel2( s, 1 ) )
		{
			miBlock.seterror( "PEDM012F" ) ;
			return -4 ;
		}
		p = getLabelIndex( s ) ;
		if ( p == -1 )
		{
			miBlock.seterror( "PEDM011Q" ) ;
			return -1 ;
		}
		else if ( p == -2 )
		{
			miBlock.seterror( "PEDM011W" ) ;
			return -3 ;
		}
	}
	return p ;
}


int PEDIT01::getExBlockSize( uint dl )
{
	// Return the number of lines in an exluded block given any data index within that block
	// This does not included logically deleted lines

	uint i       ;
	uint exlines ;

	for ( i = dl ; i > 0 ; i-- )
	{
		if ( data.at( i )->il_deleted ) { continue ; }
		if ( data.at( i )->il_excl )    { continue ; }
		break ;
	}
	i = getNextDataLine( i ) ;

	exlines = 0 ;
	for ( ; i < data.size() ; i++ )
	{
		if (  data.at( i )->il_deleted ) { continue ; }
		if ( !data.at( i )->il_excl )    { break    ; }
		exlines++ ;
	}
	return exlines ;
}


int PEDIT01::getDataBlockSize( uint dl )
{
	// Return the number of lines in an exluded block given any data index within that block
	// This does include logically deleted lines

	uint i       ;
	uint bklines ;

	for ( i = dl ; i > 0 ; i-- )
	{
		if ( data.at( i )->il_deleted ||
			   data.at( i )->il_excl )    { continue ; }
		break ;
	}
	i = getNextDataLine( i ) ;

	bklines = 0 ;
	for ( ; i < data.size() ; i++ )
	{
		if ( !data.at( i )->il_excl &&
		     !data.at( i )->il_deleted ) { break ; }
		bklines++ ;
	}
	return bklines ;
}


uint PEDIT01::getFirstEX( uint dl )
{
	// Return the data index of the first excluded line in a block

	int i ;

	for ( ; dl > 0 ; dl-- )
	{
		if ( data.at( dl )->il_deleted ) { continue ; }
		if ( data.at( dl )->il_excl )    { i = dl   ; }
		else                             { break    ; }
	}
	return i ;
}


int PEDIT01::getLastEX( vector<iline * >::iterator it )
{
	// Return the URID of the last excluded line in a block given the iterator
	// ('it' always points to a non-deleted, excluded line)

	int URID ;

	for ( ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_excl && !(*it)->il_deleted ) { break ; }
		URID = (*it)->il_URID ;
	}
	return URID ;
}


uint PEDIT01::getLastEX( uint dl )
{
	// Return the data index of the last excluded line in a block, given the data index

	int i ;

	for ( ; dl < data.size() ; dl++ )
	{
		if (  data.at( dl )->il_deleted ) { continue ; }
		if (  data.at( dl )->il_excl )    { i = dl   ; }
		else                              { break    ; }
	}
	return i ;
}


int PEDIT01::getNextSpecial( int sidx, int eidx, char dir, char t )
{
	// Return the next data vector line after l with flag il_????
	// Supported: C changed   ( status chng   )
	//            K command   ( il_lcc not blank )
	//            E error     ( status error  )
	//            I info      ( type info     )
	//            L label     ( label not blank )
	//            M message   ( type msg      )
	//            N note      ( type note     )
	//            U undo/redo ( status undo or redo )
	//            X excluded  ( flag il_excl  )
	//            S any special line
	//            (col,prof,tabs,mask,bnds,msg,info or note)
	//            T marked text
	//

	int l ;

	bool found ;

	if ( dir == ' ') { dir = 'N' ; }
	switch ( dir )
	{
		case 'N': l = topLine + 1 ;
				if ( l < sidx || l > eidx ) { l = sidx ; }
				break       ;
		case 'P': l = topLine - 1 ;
				if ( l < sidx || l > eidx ) { l = eidx ; }
				break       ;
		case 'F': l = sidx    ;
				break       ;
		case 'L': l = eidx    ;
				break       ;
	}

	found = false ;
	while ( true )
	{
		if ( !data.at( l )->il_deleted )
		{
			switch ( t )
			{
				case 'C': if ( data.at( l )->is_chng() ) { found = true ; }
						break ;
				case 'K': if ( data.at( l )->il_lcc != "" ) { found = true ; }
						break ;
				case 'E': if ( data.at( l )->is_error() ) { found = true ; }
						break ;
				case 'I': if ( data.at( l )->is_info() ) { found = true ; }
						break ;
				case 'L': if ( data.at( l )->hasLabel( nestLevel ) ) { found = true ; }
						break ;
				case 'M': if ( data.at( l )->is_msg() ) { found = true ; }
						break ;
				case 'N': if ( data.at( l )->is_note() ) { found = true ; }
						break ;
				case 'U': if ( data.at( l )->is_undo() ||
					       data.at( l )->is_redo()  ) { found = true ; }
						break ;
				case 'X': if ( data.at( l )->il_excl  ) { found = true ; }
						break ;
				case 'S': if ( data.at( l )->isSpecial() ) { found = true ; }
						break ;
				case 'T': if ( data.at( l )->il_mark  ) { found = true ; }
						break ;
			}
		}
		if ( found ) { break ; }
		if ( dir == 'N' || dir == 'F' )
		{
			l++ ;
			if ( l > eidx ) { break ; }
		}
		else
		{
			l-- ;
			if ( l < sidx ) { break ; }
		}
	}
	if ( !found )
	{
		pcmd.set_msg( "PEDT014V", 4 ) ;
		return topLine   ;
	}
	return l ;
}


int PEDIT01::getLastURID( vector<iline * >::iterator it, int Rpt )
{
	// Get the last URID for a repeat value, given an iterator and the repeat value

	// Excluded blocks always count as 1 for the repeat value

	int i  ;
	int URID ;

	bool exBlock ;
	bool lastEX  ;

	exBlock = false ;
	lastEX  = false ;
	URID    = (*it)->il_URID ;

	if ( Rpt == -1 ) { Rpt = 1 ; }

	for ( i = 0 ; i < Rpt ; i++, it++ )
	{
		if ( (*it)->is_bod() )   { break          ; }
		if ( (*it)->il_deleted ) { i-- ; continue ; }
		if ( (*it)->il_excl )
		{
			exBlock = true  ;
			lastEX  = false ;
		}
		else
		{
			if ( exBlock ) { lastEX = true  ; }
			else           { lastEX = false ; }
			exBlock = false ;
		}
		if      ( exBlock ) { i-- ; }
		else if ( lastEX  )
		{
			i++ ;
			if ( i == Rpt ) { break ; }
		}
		URID = (*it)->il_URID ;
	}
	return URID ;
}


void PEDIT01::moveTopline( int URID )
{
	// Change topLine so that the URID will be on the screen after a rebuild.
	// Always position topLine if profPosFcx is set

	if ( profPosFcx || !URIDOnScreen( URID, topLine ) )
	{
		topLine = getLine( fcx_parms.f_URID ) ;
		topLine = getPrevDataLine( topLine ) ;
	}
	return ;
}


bool PEDIT01::URIDOnScreen( int URID, int top )
{
	// Return true if the URID would appear on the screen on a rebuild starting at topLine=top.
	// URID may be deleted for UNDO/REDO. (URID found after topLine and n <= ZAREAD)
	// An excluded block counts as 1 unless excluded lines are hidden in which case the block is ignored.

	int i ;
	int j ;
	int n ;

	j = data.size() ;
	for ( n = 1, i = top ; i < j ; i++ )
	{
		if ( data.at( i )->il_URID == URID ) { break    ; }
		if ( data.at( i )->il_deleted      ) { continue ; }
		if ( data.at( i )->il_excl )
		{
			if ( !hideExcl ) { n++ ; }
			for ( ; i < j ; i++ )
			{
				if ( data.at( i )->il_deleted ) { continue ; }
				if ( !data.at( i )->il_excl   ) { break    ; }
			}
			if ( data.at( i )->il_URID == URID ) { break ; }
		}
		if ( profHex || data.at( i )->il_hex ) { n += 4 ; }
		else                                   { n++    ; }
		if ( n > ZAREAD ) { return false ; }
	}
	return ( i != j && n <= ZAREAD ) ;
}


string PEDIT01::overlay1( string s1, string s2, bool& success )
{
	// Overlay s2 with s1 where there are spaces in s2
	// If dest byte is non-blank and would be overlayed with a different non-blank char, return success=false
	// (and do not overlay the char) so that lines are not deleted on a M/MM

	int i  ;
	int l  ;

	string s ;

	l = max( s1.size(), s2.size() ) ;
	s = string( l, ' ' ) ;
	s1.resize( l, ' ' )  ;
	s2.resize( l, ' ' )  ;

	for ( i = 0 ; i < l ; i++ )
	{
		if ( s2[ i ] != ' ' )
		{
			s[ i ] = s2[ i ] ;
			if ( s1[ i ] != ' ' && s1[ i ] != s2[ i ] )
			{
				success = false ;
			}
		}
		else
		{
			s[ i ] = s1[ i ] ;
		}
	}
	return s ;
}


string PEDIT01::overlay2( string s1, string s2 )
{
	// Edit macro overlay

	// Overlay s2 with s1 where there are non-blanks in s1

	int i  ;
	int l  ;

	l = max( s1.size(), s2.size() ) ;
	s2.resize( l, ' ' ) ;

	for ( i = 0 ; i < s1.size() ; i++ )
	{
		if ( s1[ i ] != ' ' )
		{
			s2[ i ] = s1[ i ] ;
		}
	}
	return s2 ;
}


string PEDIT01::templat( string s1, string s2 )
{
	// Edit macro template

	// format < (colvar) literal >
	// format < col (datavar) >

	// s1 is the template applied to string s2

	int p1 ;
	int p2 ;
	int l  ;

	char qt  ;
	string w ;
	string v ;

	s1 = s1.substr( 1, s1.size() - 2 ) ;
	trim( s1 ) ;
	while ( true )
	{
		if ( s1.front() == '(' )
		{
			if ( !getVariables( 1, s1, v ) ) { return "" ; }
			miBlock.macAppl->vcopy( v, w, MOVE ) ;
		}
		else
		{
			w = word( s1, 1 )    ;
			idelword( s1, 1, 1 ) ;
		}
		if ( !datatype( w, 'W' ) )
		{
			miBlock.seterror( "PEDM013B", 20 ) ;
			return "" ;
		}
		p1 = ds2d( w ) - 1 ;
		if ( trim( s1 ) == "" )
		{
			miBlock.seterror( "PEDM012Z", 20 ) ;
			return "" ;
		}
		if ( s1.front() == '(' )
		{
			if ( !getVariables( 1, s1, v ) ) { return "" ; }
			miBlock.macAppl->vcopy( v, w, MOVE ) ;
		}
		else if ( s1.front() == '"' || s1.front() == '\'' )
		{
			qt = s1.front()   ;
			s1.erase( 0 , 1 ) ;
			p2 = s1.find( qt ) ;
			if ( p2 == string::npos )
			{
				miBlock.seterror( "PEDM012W", 20 ) ;
				return "" ;
			}
			w = s1.substr( 0, p2 ) ;
			s1.erase( 0 , p2+1 ) ;
		}
		else
		{
			w = word( s1, 1 )    ;
			idelword( s1, 1, 1 ) ;
		}
		l = p1 + w.size() ;
		if ( s2.size() < l ) { s2.resize( l, ' ' ) ; }
		s2.replace( p1, w.size(), w ) ;
		if ( trim( s1 ) == "" ) { break ; }
	}
	return s2 ;
}


void PEDIT01::copyPrefix( ipline& d, iline *& s )
{
	// Don't copy file line status (changed, error, undo, redo and marked )

	d.ip_type   = s->il_type   ;
	d.ip_excl   = s->il_excl   ;
	d.ip_hex    = s->il_hex    ;
	d.ip_profln = s->il_profln ;
	d.ip_label  = s->getLabel();
}


void PEDIT01::copyPrefix( iline *& d, ipline& s, bool l )
{
	// Don't copy file line status (changed, error, undo, redo and marked )
	// Copy label back for a move request

	d->il_type   = s.ip_type   ;
	d->il_excl   = s.ip_excl   ;
	d->il_hex    = s.ip_hex    ;
	d->il_profln = s.ip_profln ;
	if ( l && s.ip_label != "" )
	{
		d->setLabel( s.ip_label ) ;
	}
}


void PEDIT01::addSpecial( char t, int p, vector<string>& s )
{
	int i ;

	vector<iline * >::iterator it ;
	iline * p_iline ;

	it = data.begin() ;
	if ( p == 0 ) { p = 1 ; }
	std::advance( it, p ) ;
	for ( i = 0 ; i < s.size() ; i++ )
	{
		p_iline = new iline( taskid() ) ;
		switch ( t )
		{
			case 'I': p_iline->il_type = LN_INFO ; break ;
			case 'M': p_iline->il_type = LN_MSG  ; break ;
			case 'N': p_iline->il_type = LN_NOTE ; break ;
			case 'P': p_iline->il_profln = i  ;
				  p_iline->il_type = LN_PROF ; break ;
		}
		p_iline->put_idata( s.at( i ) ) ;
		it = data.insert( it, p_iline ) ;
		it++ ;
	}
	rebuildZAREA = true ;
}


void PEDIT01::addSpecial( char t, int p, const string& s )
{
	vector<iline * >::iterator it ;
	iline * p_iline ;

	it = data.begin()  ;
	if ( p == 0 ) { p = 1 ; }
	std::advance( it, p ) ;
	p_iline = new iline( taskid() ) ;
	switch ( t )
	{
		case 'I': p_iline->il_type = LN_INFO ; break ;
		case 'M': p_iline->il_type = LN_MSG  ; break ;
		case 'N': p_iline->il_type = LN_NOTE ; break ;
	}
	p_iline->put_idata( s, Level )  ;
	it = data.insert( it, p_iline ) ;
	rebuildZAREA = true ;
}


void PEDIT01::addSpecial( char t, vector<iline * >::iterator it, const string& s )
{
	iline * p_iline ;

	it++ ;
	p_iline = new iline( taskid() ) ;
	switch ( t )
	{
		case 'I': p_iline->il_type = LN_INFO ; break ;
		case 'M': p_iline->il_type = LN_MSG  ; break ;
		case 'N': p_iline->il_type = LN_NOTE ; break ;
	}
	p_iline->put_idata( s, Level )  ;
	it = data.insert( it, p_iline ) ;
	rebuildZAREA = true ;
}


void PEDIT01::buildProfLines( vector<string>& Prof )
{
	string t ;

	Prof.clear() ;

	t  = "...."  ;
	t += ZEDPROF ;
	t += "....UNDO "+ OnOff[ profUndoOn ] ;
	if ( profUndoOn && profUndoKeep ) { t += " KEEP" ; }
	t += "....AUTOSAVE "+ OnOff[ profASave ] ;
	if ( !profASave ) { profSaveP ? t += " PROMPT" : t += " NOPROMPT" ; }
	t += "....NUM OFF" ;
	t += "....CAPS "+OnOff[ profCaps ] ;
	Prof.push_back( left( t, ZDATAW, '.') ) ;

	t  = "....HEX " ;
	if ( profHex ) { profVert ? t += "ON VERT" : t += "ON DATA" ; }
	else           { t += "OFF" ; }
	t += "....NULLS " ;
	if ( profNulls ) { t += "ON"  ; profNullA ? t += " ALL" : t += " STD" ; }
	else             { t += "OFF" ; }
	t += "....XTABS "+ OnOff[ profXTabs ] ;
	t += "....XTAB SIZE "+ d2ds(profXTabz) ;
	if ( XTabz > 0 && profXTabz != XTabz ) { t += " (Pending change "+ d2ds(XTabz) + ")" ; }
	Prof.push_back( left( t, ZDATAW, '.' ) ) ;

	t  = "....TABS "+ OnOff[ profTabs  ] ;
	if ( profTabs ) { profATabs ? t += " ALL" : t += " STD" ; }
	t += "....RECOVER "+ OnOff[ profRecover ]+" PATH "+ recoverLoc ;
	Prof.push_back( left( t, ZDATAW, '.' ) ) ;

	t  = "....HILIGHT " ;
	if ( profHilight )
	{
		(profLang == "AUTO") ? t += detLang : t += profLang ;
		if ( profIfLogic )
		{
			(profDoLogic) ? t += " LOGIC" : t += " IFLOGIC" ;
		}
		else if ( profDoLogic ) { t += " DOLOGIC" ; }
	}
	else
	{
		t += "OFF" ;
	}
	if ( profParen ) { t += " PAREN" ; }
	profLock ? t += "....PROFILE LOCK" : t += "....PROFILE UNLOCK" ;
	t  += "....IMACRO "+ profIMAC ;
	Prof.push_back( left( t, ZDATAW, '.' ) ) ;
}


void PEDIT01::updateProfLines( vector<string>& Prof )
{
	vector<iline * >::iterator it ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->is_prof() ) { continue ; }
		(*it)->put_idata( Prof[ (*it)->il_profln ] ) ;
		rebuildZAREA = true ;
	}
}


void PEDIT01::cleanupRedoStacks()
{
	// If there are entries on any redo stack (Redo_data flag) but the Global redo level is 0,
	// remove redo data from all lines as this data is no longer required.

	auto it = data.begin() ;
	if ( (*it)->is_Redo_data() && (*it)->get_Global_Redo_level() == 0 )
	{
		for_each( data.begin(), data.end(),
			[](iline *& a)
			{
				a->remove_redo_idata() ;
			} ) ;
		(*it)->reset_Redo_data() ;

	}
}


void PEDIT01::removeRecoveryData()
{
	// Delete all logically deleted lines and remove entries from the data vector
	// Flatten all remaining data
	// Clear the global Undo/Redo/File stacks

	// Make a copy of the data vector using copy_if that contains only records to be deleted as
	// iterator is invalidated after an erase()

	// Reposition topLine as lines before may have been removed or topLine itself, deleted

	int topURID ;

	vector<iline * >::iterator it      ;
	vector<iline * >::iterator new_end ;
	vector<iline * >tdata              ;

	if ( data.at( topLine )->il_deleted )
	{
		topLine = getNextDataLine( topLine ) ;
	}
	topURID = data.at( topLine )->il_URID ;

	copy_if( data.begin(), data.end(), back_inserter( tdata ),
		[](iline *& a)
		{
			return a->il_deleted ;
		} ) ;

	new_end = remove_if( data.begin(), data.end(),
		[](iline *& a)
		{
			return a->il_deleted ;
		} ) ;
	data.erase( new_end, data.end() ) ;

	for_each( tdata.begin(), tdata.end(),
		[](iline *& a)
		{
			delete a ;
		} ) ;
	for_each( data.begin(), data.end(),
		[](iline *& a)
		{
			a->flatten_idata() ;
		} ) ;

	it = data.begin() ;
	(*it)->clear_Global_Undo() ;
	(*it)->clear_Global_Redo() ;
	(*it)->clear_Global_File_level() ;
	(*it)->reset_Redo_data()      ;
	topLine = getLine( topURID )  ;
	pcmd.set_msg( "PEDT012H", 4 ) ;
}


void PEDIT01::cleanupData()
{
	// Delete all logically deleted lines for non-file data and remove entries from the data vector
	// (Make a copy of the data vector using copy_if that contains only records to be deleted as iterator is invalidated after an erase() )

	// Remove the redo idata for each line
	// Reset Global_Undo stack (set same as Global_File_level stack) as now there are only file UNDO entries
	// Clear Global_Redo stack
	// Only UNDO of file data valid after this has been done

	// Reposition topLine as lines before may have been removed or topLine itself, deleted

	int topURID ;

	vector<iline * >::iterator it      ;
	vector<iline * >::iterator new_end ;
	vector<iline * >tdata              ;

	if ( data.at( topLine )->il_deleted )
	{
		topLine = getNextDataLine( topLine ) ;
	}
	topURID = data.at( topLine )->il_URID ;

	copy_if( data.begin(), data.end(), back_inserter( tdata ),
		[](iline *& a)
		{
			return ( a->il_deleted && !a->is_file() ) ;
		} ) ;

	new_end = remove_if( data.begin(), data.end(),
		[](iline *& a)
		{
			return ( a->il_deleted && !a->is_file() ) ;
		} ) ;
	data.erase( new_end, data.end() ) ;

	for_each( tdata.begin(), tdata.end(),
		[](iline *& a)
		{
			delete a ;
		} ) ;
	for_each( data.begin(), data.end(),
		[](iline *& a)
		{
			a->remove_redo_idata() ;
		} ) ;

	it = data.begin()            ;
	(*it)->reset_Global_Undo()   ;
	(*it)->clear_Global_Redo()   ;
	(*it)->reset_Redo_data()     ;
	topLine = getLine( topURID ) ;
}


void PEDIT01::removeProfLines()
{
	// Delete PROFILE lines in Data
	// Reposition topLine as lines before may have been removed or topLine itself, deleted

	int topURID ;
	vector<iline * >::iterator new_end ;
	vector<iline * >tdata              ;

	while ( data.at( topLine )->is_prof() )
	{
		topLine = getNextDataLine( topLine ) ;
	}
	topURID = data.at( topLine )->il_URID ;

	copy_if( data.begin(), data.end(), back_inserter( tdata ),
		[](iline *& a)
		{
			return a->is_prof() ;
		} ) ;

	new_end = remove_if( data.begin(), data.end(),
		[](iline *& a)
		{
			return a->is_prof() ;
		} ) ;
	data.erase( new_end, data.end() ) ;

	for_each( tdata.begin(), tdata.end(),
		[](iline *& a)
		{
			delete a ;
		} ) ;
	topLine = getLine( topURID ) ;
}


void PEDIT01::removeSpecialLines( vector<iline * >::iterator its, vector<iline * >::iterator ite )
{
	// Logically delete all temporary lines from the Data vector (can be undone)
	// (col,prof,tabs,mask,bnds,msg,info and note)

	int lvl ;

	lvl = ++Level ;
	for_each( its, ite,
		[ &lvl ](iline *& a)
		{
			if ( a->isSpecial() ) { a->set_deleted( lvl ) ; }
		} ) ;
}


bool PEDIT01::getTabLocation( int& pos )
{
	pos = tabsLine.find( '-', aCol+startCol-CLINESZ-1 ) ;
	if ( pos == string::npos )
	{
		pos = tabsLine.find( '-', startCol-1 ) ;
		if ( pos == string::npos )
		{
			return false ;
		}
		return true ;
	}
	return true  ;
}


bool PEDIT01::copyToClipboard( vector<ipline>& vip )
{
	// Copy only data lines in vip (from copy/move) to lspf table clipTable

	// cutReplace - clear clipBoard before copy, else append at end of current contents
	// An error will be issued if the clipboard is locked

	int i   ;
	int t   ;
	int CRP ;
	int pos ;
	int sz  ;

	string uprof    ;

	string zedcname ;
	string zedcstat ;
	string zedcdesc ;
	string zedctext ;

	const string vlist = "ZEDCNAME ZEDCSTAT ZEDCDESC ZEDCTEXT" ;

	vdefine( vlist, &zedcname, &zedcstat, &zedcdesc, &zedctext ) ;
	vdefine( "CRP", &CRP ) ;
	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( clipTable, WRITE, uprof ) ;
	if ( RC == 8 )
	{
		tbcreate( clipTable, "", "(ZEDCNAME,ZEDCTEXT)", WRITE, NOREPLACE, uprof ) ;
		if ( RC > 0 ) { uabend( "PEDT015W", "TBCREATE" ) ; }
	}

	tbvclear( clipTable ) ;
	zedcname = clipBoard  ;

	tbtop( clipTable )  ;
	tbsarg( clipTable ) ;
	tbscan( clipTable ) ;
	if ( RC == 0 )
	{
		if ( zedcstat == "RO" )
		{
			tbclose( clipTable ) ;
			vdelete( vlist ) ;
			vdelete( "CRP" ) ;
			vreplace( "ZEDCLIP", zedcname ) ;
			pcmd.set_msg( "PEDT015V", 8 ) ;
			return false ;
		}
		if ( cutReplace )
		{
			clearClipboard( zedcname ) ;
		}
		else
		{
			pos = 0 ;
			tbtop( clipTable ) ;
			while ( true )
			{
				tbscan( clipTable, "", "", "", "", "NOREAD", "CRP" ) ;
				if ( RC == 8 ) { break ; }
				pos = CRP ;
			}
			if ( pos > 0 )
			{
				tbtop( clipTable ) ;
				tbskip( clipTable, pos, "", "", "", "NOREAD" ) ;
			}
		}
	}

	tbvclear( clipTable ) ;
	zedcname = clipBoard  ;

	sz = vip.size()    ;
	for ( t = 0, i = 0 ; i < sz ; i++ )
	{
		if ( !vip[ i ].is_file() ) { continue ; }
		zedctext = vip[ i ].ip_data ;
		tbadd( clipTable, "", "", min( 32767, sz ) ) ;
		if ( RC > 0 ) { uabend( "PEDT015W", "TBADD" ) ; }
		t++ ;
	}

	tbclose( clipTable ) ;

	pcmd.set_msg( "PEDT013C", 4 )  ;
	vdelete( vlist ) ;
	vdelete( "CRP" ) ;
	vreplace( "ZEDLNES", d2ds( t ) )  ;
	vreplace( "ZEDCNAME", clipBoard ) ;
	return true ;
}


void PEDIT01::getClipboard( vector<ipline>& vip )
{
	// Get lines from clipBoard and put them in vector vip
	// pasteKeep - don't clear clipBoard after paste (A locked clipboard will always be kept)

	string uprof    ;
	string zedcname ;
	string zedctext ;
	ipline t        ;

	vip.clear() ;

	vdefine( "ZEDCNAME ZEDCTEXT", &zedcname, &zedctext ) ;
	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( clipTable, WRITE, uprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbvclear( clipTable ) ;
	zedcname = clipBoard  ;
	tbsarg( clipTable, "", "NEXT", "(ZEDCNAME,EQ)" ) ;

	tbtop( clipTable ) ;
	while ( true )
	{
		tbscan( clipTable ) ;
		if ( RC == 8 ) { break ; }
		t.ip_type = LN_FILE  ;
		t.ip_data = zedctext ;
		vip.push_back( t ) ;
	}

	if ( !pasteKeep ) { clearClipboard( zedcname ) ; }

	tbclose( clipTable ) ;
	vdelete( "ZEDCNAME ZEDCTEXT" ) ;
}


void PEDIT01::clearClipboard( string clip )
{
	// Pass 'clip' by value as this has been vdefined as zedcname so will be cleared by a tbvclear()
	// if passed by reference.  Don't clear clipboard if is has been locked (ZEDCSTAT = RO)

	string t ;

	tbvclear( clipTable ) ;

	verase( "ZEDCSTAT" ) ;
	vreplace( "ZEDCNAME", clip ) ;
	tbsarg( clipTable, "", "NEXT" ) ;

	tbtop( clipTable ) ;
	tbscan( clipTable ) ;
	if ( RC == 0 )
	{
		vcopy( "ZEDCSTAT", t ) ;
		if ( t == "RO" )
		{
			tbtop( clipTable ) ;
			return ;
		}
	}
	tbtop( clipTable ) ;
	while ( true )
	{
		tbscan( clipTable ) ;
		if ( RC == 8 ) { break  ; }
		tbdelete( clipTable )   ;
		if ( RC  > 0 ) { break  ; }
	}
	tbtop( clipTable ) ;
}


void PEDIT01::manageClipboard()
{
	int i ;

	bool rebuild ;

	string suf  ;
	string desc ;
	string lc   ;
	string stat ;
	string name ;

	vector<string> descr ;

	manageClipboard_create( descr ) ;
	rebuild = false ;

	addpop( "", 4, 5 ) ;
	while ( true )
	{
		display( "PEDIT017" ) ;
		if ( RC == 8 ) { break ; }
		for ( i = 0 ; i < descr.size() && i < 11 ; i++ )
		{
			suf = d2ds( i + 1 ) ;
			vcopy( "CLPLC"+ suf, lc,   MOVE  ) ;
			vcopy( "CLPDS"+ suf, desc, MOVE  ) ;
			vcopy( "CLPST"+ suf, stat, MOVE  ) ;
			vcopy( "CLPNM"+ suf, name, MOVE  ) ;
			vreplace( "ZEDCLIP", name ) ;
			if ( descr[ i ] != desc )
			{
				manageClipboard_descr( name, desc ) ;
				descr[ i ] = desc ;
			}
			if ( lc == "B" )
			{
				manageClipboard_browse( name ) ;
				vreplace( "CLPLC"+ d2ds( i + 1 ), "" ) ;
			}
			else if ( lc == "O" )
			{
				manageClipboard_toggle( i, name ) ;
				rebuild = true ;
			}
			else if ( lc == "E" )
			{
				manageClipboard_edit( name, desc ) ;
				rebuild = true ;
			}
			else if ( lc == "R" )
			{
				manageClipboard_rename( name, desc ) ;
				rebuild = true ;
			}
			else if ( lc == "D" || lc == "C" )
			{
				manageClipboard_delete( name) ;
				rebuild = true ;
			}
		}
		if ( rebuild )
		{
			manageClipboard_create( descr ) ;
			rebuild = false ;
		}
	}
	rempop() ;
}


void PEDIT01::manageClipboard_create( vector<string>& descr )
{
	int i ;
	int l ;

	string t ;

	string suf      ;
	string uprof    ;
	string zedcname ;
	string zedcstat ;
	string zedcdesc ;
	string zedctext ;

	descr.clear() ;

	const string vlist = "ZEDCNAME ZEDCSTAT ZEDCDESC ZEDCTEXT" ;

	vdefine( vlist, &zedcname, &zedcstat, &zedcdesc, &zedctext ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( clipTable, NOWRITE, uprof ) ;
	if ( RC > 0 )
	{
		vdelete( vlist ) ;
		pcmd.set_msg( "PEDT014Y" ) ;
		return ;
	}

	t = "" ;
	i = 0 ;
	l = 0 ;

	tbtop( clipTable ) ;
	while ( i < 11 )
	{
		tbskip( clipTable, 1 ) ;
		if ( RC > 0 ) { break ; }
		if ( t != zedcname )
		{
			if ( l > 0 )
			{
				vreplace( "CLPLN"+ d2ds( i + 1 ), d2ds( l ) ) ;
				i++ ;
			}
			t   = zedcname ;
			l   = 0 ;
			suf = d2ds( i + 1 ) ;
			vreplace( "CLPLC"+ suf, ""       ) ;
			vreplace( "CLPNM"+ suf, zedcname ) ;
			vreplace( "CLPDS"+ suf, zedcdesc ) ;
			vreplace( "CLPST"+ suf, zedcstat ) ;
			descr.push_back( zedcdesc ) ;
			if ( zedcname == "DEFAULT" && zedcdesc == "" )
			{
				vreplace( "CLPDS"+ suf, "Default clipboard" ) ;
			}
		}
		l++ ;
	}

	tbend( clipTable ) ;
	if ( l > 0 )
	{
		vreplace( "CLPLN"+ d2ds( i + 1 ), d2ds( l ) ) ;
	}

	for ( i++ ; i < 11 ; i++ )
	{
		suf = d2ds( i + 1 ) ;
		vreplace( "CLPLC"+ suf, "" ) ;
		vreplace( "CLPNM"+ suf, "" ) ;
		vreplace( "CLPLN"+ suf, "" ) ;
		vreplace( "CLPDS"+ suf, "" ) ;
		vreplace( "CLPST"+ suf, "" ) ;
	}

	vdelete( vlist ) ;
}


void PEDIT01::manageClipboard_descr( const string& name, const string& desc )
{
	string uprof ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( clipTable, WRITE, uprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbvclear( clipTable ) ;

	vreplace( "ZEDCNAME", name ) ;
	vreplace( "ZEDCSTAT", "UP" ) ;

	tbsarg( clipTable ) ;
	tbscan( clipTable ) ;
	if ( RC > 0 )
	{
		tbclose( clipTable ) ;
		return ;
	}

	vreplace( "ZEDCDESC", desc ) ;

	tbput( clipTable, "(ZEDCDESC,ZEDCSTAT)" ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBPUT" ) ; }

	tbclose( clipTable ) ;
}


void PEDIT01::manageClipboard_browse( const string& name )
{
	string tname ;

	vector<ipline> vip ;

	tname = createTempName() ;

	clipBoard = name    ;
	pasteKeep = true    ;
	getClipboard( vip ) ;

	std::ofstream fout( tname.c_str() ) ;
	for ( int i = 0 ; i < vip.size() ; i++ )
	{
		fout << vip[ i ].ip_data << endl ;
	}
	fout.close() ;

	vreplace( "ZBRALT", "CLIPBOARD:"+name ) ;
	vput( "ZBRALT", SHARED ) ;

	browse( tname ) ;
	remove( tname ) ;
	verase( "ZBRALT", SHARED ) ;
}


void PEDIT01::manageClipboard_edit( const string& name, const string& desc )
{
	string inLine ;
	string tname  ;

	vector<ipline> vip ;

	ipline ip ;

	tname = createTempName() ;

	clipBoard = name    ;
	pasteKeep = true    ;
	getClipboard( vip ) ;

	std::ofstream fout( tname.c_str() ) ;
	for ( int i = 0 ; i < vip.size() ; i++ )
	{
		fout << vip[ i ].ip_data << endl ;
	}
	fout.close() ;

	vreplace( "ZEDALT", "CLIPBOARD:"+name ) ;
	vput( "ZEDALT", SHARED ) ;

	edit( tname ) ;
	if ( ZRC == 0 && ZRSN == 4 )
	{
		vip.clear() ;
		std::ifstream fin( tname.c_str() ) ;
		while ( getline( fin, inLine ) )
		{
			ip.ip_type = LN_FILE ;
			ip.ip_data = inLine  ;
			vip.push_back( ip )  ;
		}
		fin.close() ;

		cutReplace = true ;
		copyToClipboard( vip ) ;
		manageClipboard_descr( name, desc ) ;
	}
	remove( tname ) ;
	verase( "ZEDALT", SHARED ) ;
}


void PEDIT01::manageClipboard_toggle( int i, const string& name )
{
	string uprof ;
	string t     ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( clipTable, WRITE, uprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbvclear( clipTable ) ;

	vreplace( "ZEDCNAME", name ) ;
	vreplace( "ZEDCSTAT", ""   ) ;
	vreplace( "ZEDCDESC", ""   ) ;

	tbsarg( clipTable ) ;
	tbscan( clipTable ) ;
	if ( RC > 0 )
	{
		tbclose( clipTable ) ;
		return ;
	}

	vcopy( "ZEDCSTAT", t ) ;
	t = ( t == "RO" ) ? "UP" : "RO" ;
	vreplace( "ZEDCSTAT", t ) ;
	vreplace( "CLPST"+ d2ds( i+1 ), t ) ;

	tbput( clipTable, "(ZEDCDESC,ZEDCSTAT)" ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBPUT" ) ; }

	tbclose( clipTable ) ;
}


void PEDIT01::manageClipboard_rename( const string& name, const string& desc )
{
	string newname ;
	string newdesc ;

	string uprof ;

	vector<ipline> vip ;

	vreplace( "OLDNAME", name ) ;
	vreplace( "NEWNAME", name ) ;
	vreplace( "NEWDESC", desc ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	addpop( "", 4, 5 ) ;
	display( "PEDIT018" ) ;
	if ( RC == 0 )
	{
		vcopy( "NEWNAME", newname, MOVE ) ;
		vcopy( "NEWDESC", newdesc, MOVE ) ;
		clipBoard = name    ;
		pasteKeep = false   ;
		getClipboard( vip ) ;
		clipBoard = newname ;
		copyToClipboard( vip ) ;
		manageClipboard_descr( newname, newdesc ) ;
	}
	rempop() ;
}


void PEDIT01::manageClipboard_delete( const string& name )
{
	string uprof ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( clipTable, WRITE, uprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	clearClipboard( name ) ;

	tbclose( clipTable ) ;
}


void PEDIT01::cleanup_custom()
{
	// Called if there is an abnormal termination in the program
	// Try writing out the data to a file so we can reload later (if file has been changed)
	// Release storage and clear the data vector

	string f ;
	vector<iline * >::iterator it ;

	llog( "E", "Control given to EDIT cleanup procedure due to an abnormal event" <<endl ) ;
//      std::ofstream fout( "/tmp/editorsession", ios::binary ) ;
//      fout.write((char*) &data, sizeof data ) ;
//      fout.close() ;

	if ( data.size() == 0 )
	{
		llog( "I", "No data found." <<endl ) ;
		return ;
	}

	if ( abendComplete )
	{
		llog( "W", "Cleanup routine called a second time.  Ignoring..." <<endl ) ;
		releaseDynamicStorage() ;
		return ;
	}

	if ( !profRecover )
	{
		llog( "W", "File not saved as RECOVER is set off" <<endl ) ;
		releaseDynamicStorage() ;
		EditList.erase( ZFILE ) ;
		ZRC  = 0 ;
		ZRSN = 0 ;
		return   ;
	}

	if ( !fileChanged )
	{
		llog( "I", "File not saved as no changes made during edit session or since last save" <<endl ) ;
		releaseDynamicStorage() ;
		EditList.erase( ZFILE ) ;
		ZRC  = 0 ;
		ZRSN = 0 ;
		return   ;
	}

	f = ZFILE + ".##abend" ;
	std::ofstream fout( f.c_str() ) ;

	if ( !fout.is_open() )
	{
		ZRESULT = "Open for output Error" ;
		releaseDynamicStorage() ;
		RC = 16 ;
		return  ;
	}

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->isValidFile() )
		{
			fout << (*it)->get_idata() <<endl ;
		}
	}

	fout.close() ;
	llog( "I", "File saved to "<< f <<endl ) ;
	ZRC  = 0 ;
	ZRSN = 8 ;
	releaseDynamicStorage() ;
	EditList.erase( ZFILE ) ;
	Global_efind_parms = fcx_parms ;
	vreplace( "ZEDABRC", f ) ;
	vput( "ZEDABRC", PROFILE ) ;
	abendComplete = true ;
	llog( "I", "Edit cleanup complete" <<endl ) ;
}


void PEDIT01::releaseDynamicStorage()
{
	for_each( data.begin(), data.end(),
		[](iline *& a)
		{
			delete a ;
		} ) ;
	data.clear() ;
}


void PEDIT01::copyFileData( vector<string>& tdata, int sidx, int eidx )
{
	// Copy valid file entries between sidx and eidx to vector tdata.  Pad all records with spaces
	// to the longest (used in sort so all records should be the same length)

	int i ;
	int l = 0 ;

	for ( i = sidx ; i <= eidx ; i++ )
	{
		if ( data.at( i )->isValidFile() )
		{
			l = max(l, data.at( i )->get_idata_len() ) ;
			tdata.push_back( data.at( i )->get_idata() ) ;
		}
	}

	for ( auto it = tdata.begin() ; it != tdata.end() ; it++ )
	{
		it->resize( l, ' ' ) ;
	}
}


void PEDIT01::compareFiles( const string& s )
{
	//  Simple compare routine using output from the diff command

	//  Compare edit version of file with entered file name.  Differences displayed in browse or
	//  with INFO lines ( .Onnnn signify lines added, INFO lines ====== are lines deleted )

	int i   ;
	int o   ;
	int d1  ;
	int pos ;
	int lab ;

	bool rl ;

	string cmd ;
	string t1  ;
	string dparms ;
	string result ;
	string file   ;
	string v_list ;
	string tname1 ;
	string tname2 ;
	string inLine ;

	string CFILE   ;
	string ECPBRDF ;
	string ECPICAS ;
	string ECPIREF ;
	string ECPIBLK ;
	string ECPITBE ;

	string * pt ;

	char buffer[256] ;

	vector<iline * >::iterator it ;
	vector<string>Changes ;

	v_list = "CFILE ECPBRDF ECPICAS ECPIREF ECPIBLK ECPITBE" ;

	vcopy( "ZUSER", zuser, MOVE ) ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	vdefine( v_list, &CFILE, &ECPBRDF, &ECPICAS, &ECPIREF, &ECPIBLK, &ECPITBE) ;
	vget( v_list, PROFILE ) ;

	if ( s == "" )
	{
		pcmd.clear_msg() ;
		display( "PEDIT013", pcmd.get_msg(), "ZCMD3" ) ;
		if ( RC == 8 ) { vdelete( v_list ) ; return ; }
	}
	else if ( s == "*" )
	{
		CFILE = ZFILE ;
	}
	else if ( s.front() != '/' )
	{
		CFILE = ZFILE.substr( 0, ZFILE.find_last_of( '/' )+1 ) + s ;
	}
	else
	{
		CFILE = s ;
	}

	if ( !is_regular_file( CFILE ) )
	{
		pcmd.set_msg( "PEDT013O" )  ;
		vdelete( v_list ) ;
		return ;
	}

	dparms = "" ;
	if ( ECPBRDF == "/" ) { dparms  = " -y " ; }
	if ( ECPICAS == "/" ) { dparms += " -i " ; }
	if ( ECPIREF == "/" ) { dparms += " -b " ; }
	if ( ECPIBLK == "/" ) { dparms += " -B " ; }
	if ( ECPITBE == "/" ) { dparms += " -E " ; }

	tname1 = createTempName() ;
	tname2 = createTempName() ;

	std::ofstream fout( tname1.c_str() ) ;
	std::ofstream of ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->isValidFile() ) { continue ; }
		pt = (*it)->get_idata_ptr() ;
		(*it)->set_idata_trim() ;
		if ( profXTabs )
		{
			t1 = "" ;
			for ( i = 0 ; i < pt->size() ; i++ )
			{
				if ( (i % profXTabz == 0)         &&
				     pt->size() > (i+profXTabz-1) &&
				     pt->compare( i, profXTabz, string( profXTabz, ' ' ) ) == 0 )
				{
					t1.push_back( '\t' ) ;
					i = i + 7 ;
				}
				else if ( pt->at( i ) != ' ' ) { break ; }
				else                           { t1.push_back( ' ' ) ; }

			}
			if ( i < pt->size() )
			{
				t1 += pt->substr( i ) ;
			}
			fout << t1 <<endl ;
		}
		else
		{
			fout << *pt <<endl ;
		}
	}
	fout.close() ;

	cmd = "diff " + dparms + CFILE + " " + tname1 + " 2>&1" ;
	of.open( tname2 ) ;
	FILE* pipe{popen( cmd.c_str(), "r" ) } ;
	while( fgets( buffer, sizeof( buffer ), pipe ) != nullptr )
	{
		file   = buffer ;
		result = file.substr(0, file.size() - 1 ) ;
		of << result <<endl ;
	}
	pclose( pipe ) ;
	of.close()     ;
	remove( tname1 ) ;

	if ( ECPBRDF == "/" )
	{
		browse( tname2 ) ;
		if ( ZRC == 4 ) { pcmd.set_msg( "PEDT013P", 4 ) ; }
		remove( tname2 )  ;
		vdelete( v_list ) ;
		return            ;
	}

	lab = 1 ;
	rl  = false ;
	Changes.clear() ;
	std::ifstream fin( tname2.c_str() ) ;
	while ( getline( fin, inLine ) )
	{
		rl = true ;
		if ( isdigit( inLine.front() ) )
		{
			o = 0 ;
			if ( Changes.size() > 0 )
			{
				addSpecial( 'I', getDataLine( d1 ), Changes ) ;
				Changes.clear() ;
			}
			pos = inLine.find_first_of( "acd" ) ;
			t1  = inLine.substr( pos+1 )  ;
			pos = t1.find_first_of( ',' ) ;
			if ( pos == string::npos ) { d1 = ds2d( t1 ) ; }
			else                       { d1 = ds2d( t1.substr( 0, pos ) ) ; }
		}
		else if ( inLine.front() == '<' )
		{
			Changes.push_back ( removeTabs( inLine.substr( 2 ) ) ) ;
		}
		else if ( inLine.front() == '>' )
		{
			data.at(( getDataLine( d1+o ) ))->setLabel( ".O" + d2ds( lab, 4 ) ) ;
			lab++ ;
			o++   ;
		}
	}
	fin.close() ;
	if ( Changes.size() > 0 )
	{
		addSpecial( 'I', getDataLine( d1 ), Changes ) ;
	}
	if ( !rl ) { pcmd.set_msg( "PEDT013P", 4 ) ; }
	rebuildZAREA = true ;
	remove( tname2 )  ;
	vdelete( v_list ) ;
}


void PEDIT01::createFile( uint sidx, uint eidx )
{
	//  Create file with contents from sidx to eidx

	int i ;
	int j ;

	string t1 ;

	string * pt ;

	std::ofstream fout( creFile.c_str() ) ;
	if ( !fout.is_open() ) { vreplace( "ZSTR1", creFile ) ; pcmd.set_msg( "PEDT014S" ) ; return ; }

	for ( i = sidx ; i <= eidx ; i++ )
	{
		if ( !data.at( i )->isValidFile() ) { continue ; }
		pt = data.at( i )->get_idata_ptr() ;
		if ( !optPreserve ) { data.at( i )->set_idata_trim() ; }
		if ( profXTabs )
		{
			t1 = "" ;
			for ( j = 0 ; j < pt->size() ; j++ )
			{
				if ( j % profXTabz == 0           &&
				     pt->size() > (j+profXTabz-1) &&
				     pt->compare( j, profXTabz, string( profXTabz, ' ' ) ) == 0 )
				{
					t1.push_back( '\t' ) ;
					j = j + 7 ;
				}
				else if ( pt->at( j ) != ' ' ) { break ; }
				else                           { t1.push_back( ' ' ) ; }

			}
			if ( j < pt->size() )
			{
				t1 += pt->substr( j ) ;
			}
			fout << t1 <<endl ;
		}
		else
		{
			fout << *pt <<endl ;
		}
	}
	fout.close() ;
	if ( fout.fail() ) { pcmd.set_msg( "PEDT011Q" ) ; return ; }
	pcmd.set_msg( "PEDT014T", 4 ) ;
}


string PEDIT01::removeTabs( string s )
{
	int j   ;
	int pos ;

	pos = s.find( '\t' ) ;
	while ( pos != string::npos )
	{
		j = profXTabz - (pos % profXTabz ) ;
		s.replace( pos, 1,  j, ' ' )  ;
		pos = s.find( '\t', pos + 1 ) ;
	}
	return s ;
}


string PEDIT01::getMaskLine()
{
	if ( maskLine == "" )
	{
		return "" ;
	}
	else if ( profCaps )
	{
		return upper( maskLine ) ;
	}
	return maskLine ;
}


string PEDIT01::rshiftCols( int n, const string * p_s )
{
	string s = *p_s ;

	if ( LeftBnd > s.size() ) { return s ; }
	s.insert( LeftBnd-1, n, ' ' ) ;

	if ( RightBnd == 0 || RightBnd > s.size() ) { return s ; }
	return s.erase( RightBnd, n ) ;
}


string PEDIT01::lshiftCols( int n, const string * p_s )
{
	string s = *p_s ;

	if ( RightBnd > 0 && RightBnd < s.size() )
	{
		s.insert( RightBnd, n, ' ' ) ;
	}

	if ( LeftBnd > s.size() ) { return s ; }
	return s.erase( LeftBnd-1, n ) ;
}


bool PEDIT01::rshiftData( int n, const string * s, string& t )
{
	//  > Right shifting rules:
	//  1) scanning starts at left column
	//  2) first blank char is found
	//  3) the next non-blank char is found
	//  4) the next double blank char is found
	//  5) data from 3) to 4) is shifted 1 col to the right
	//  The above 5 steps are repeated until request is satisfied
	//
	//  Without   1) losing data
	//            2) shift beyond bound
	//            3) deleting single blanks
	//            4) deleting blanks within apostrophes
	//  else ==ERR>

	int i  ;
	int c1 ;
	int p1 ;
	int p2 ;
	int p3 ;

	t = *s ;
	if ( LeftBnd > t.size() ) { return true ; }
	for ( i = 0 ; i < n ; i++ )
	{
		(RightBnd > 0 && RightBnd < t.size()) ? c1 = RightBnd-1 : c1 = t.size()-1 ;
		p1 = t.find( ' ', LeftBnd-1 ) ;
		if ( p1 == string::npos || p1 > c1 ) { return false ; }
		p2 = t.find_first_not_of( ' ', p1 ) ;
		if ( p2 == string::npos || p2 > c1 ) { return true  ; }
		p3 = t.find( "  ", p2 ) ;
		if ( p3 != string::npos && p3 < c1 )
		{
			t.insert( p2, 1, ' ' ) ;
			t.erase( p3+1, 1 ) ;
			continue ;
		}
		if ( RightBnd == 0 || RightBnd > t.size() )
		{
			t.insert( p2, 1, ' ' ) ;
			continue ;
		}
		if ( RightBnd < t.size() && t.at( RightBnd-1 ) == ' ' )
		{
			t.insert( p2, 1, ' ' ) ;
			t.erase( RightBnd, 1 ) ;
		}
		else { return false ; }
	}
	return true ;
}


bool PEDIT01::lshiftData( int n, const string * s, string& t )
{
	//  < Left shifting rules:
	//  1) scanning starts at left column
	//  2) first blank char is found
	//  3) the next non-blank char is found
	//  4) the next double blank char is found
	//  5) data from 3) to 4) is shifted 1 col to the left
	//  The above 5 steps are repeated until request is satisfied
	//  Without   1) losing data
	//            2) shift beyond bound
	//            3) deleting single blanks
	//            4) deleting blanks within apostrophes
	//  else ==ERR>

	int i  ;
	int c1 ;
	int p1 ;
	int p2 ;
	int p3 ;

	t = *s ;
	if ( LeftBnd > t.size() ) { return true ; }
	for ( i = 0 ; i < n ; i++ )
	{
		if ( t.size() < 3 ) { return false ; }
		(RightBnd > 0 && RightBnd < t.size()) ? c1 = RightBnd-1 : c1 = t.size()-1 ;
		p1 = t.find( ' ', LeftBnd-1 ) ;
		if ( p1 == string::npos || p1 > c1 ) { return false ; }
		p2 = t.find_first_not_of( ' ', p1 ) ;
		if ( p2 == string::npos || p2 > c1 ) { return true  ; }
		if ( (p2-p1) < 2 ) { return false ; }
		p3 = t.find( "  ", p2 ) ;
		t.erase( p1, 1 ) ;
		if ( p3 == string::npos || p3 > c1 )
		{
			if ( RightBnd > 0 && RightBnd-1 < t.size() )
			{
				t.insert( RightBnd-1, 1, ' ' ) ;
			}
		}
		else
		{
			t.insert( p3, 1, ' ' ) ;
		}
	}
	return true ;
}


bool PEDIT01::textSplitData( const string& s, string& t1, string& t2 )
{
	int p ;
	int k ;

	p = aCol + startCol - CLINESZ - 2 ;
	k = s.find_first_not_of( ' ' ) ;

	if ( (RightBnd > 0 && p >= RightBnd ) || p < LeftBnd-1 || p >= s.size() )
	{
		t1 = s  ;
		t2 = "" ;
		return false ;
	}
	t1 = s.substr( 0, p ) ;
	if ( RightBnd > 0 )
	{
		t1 = s ;
		t1 = t1.replace( p, RightBnd-p, RightBnd-p, ' ' ) ;
		t2 = s.substr( p, RightBnd-p ) ;
		t2.insert( 0, LeftBnd-1, ' ' ) ;
	}
	else
	{
		t1 = s.substr( 0, p ) ;
		t2 = strip( s.substr( p ), 'L' ) ;
		t2.insert( 0, k, ' ' ) ;
	}
	return true ;
}


void PEDIT01::setChangedMsg()
{
	STR  = setFoundString() ;
	TYPE = typList[ fcx_parms.f_mtch ] ;
	OCC  = d2ds( fcx_parms.f_ch_occs ) ;
	pcmd.set_msg( fcx_parms.f_chngall ? "PEDT013K" : "PEDT013L", 0 ) ;
}


void PEDIT01::setFoundMsg()
{
	STR   = setFoundString() ;
	TYPE  = typList[ fcx_parms.f_mtch ] ;
	OCC   = d2ds( fcx_parms.f_occurs ) ;
	LINES = d2ds( fcx_parms.f_lines  ) ;
	pcmd.set_msg( fcx_parms.f_dir == 'A' ? "PEDT013I" : "PEDT013G", 0 ) ;
}


void PEDIT01::setExcludedMsg()
{
	STR   = setFoundString() ;
	TYPE  = typList[ fcx_parms.f_mtch ] ;
	OCC   = d2ds( fcx_parms.f_occurs ) ;
	LINES = d2ds( fcx_parms.f_lines ) ;
	pcmd.set_msg( "PEDT011L", 0 ) ;
}


void PEDIT01::setNotFoundMsg()
{
	// dir = N and not from top    - Bottom of Data/Range reached
	// dir = P and not from bottom - Top of Data/Range reached
	// dir = F/A/L/N-top/P-bottom  - Not found (in range)

	STR = setFoundString() ;

	if ( !fcx_parms.f_searched && ( fcx_parms.f_top || fcx_parms.f_bottom ) )
	{
		if      ( fcx_parms.f_excl == 'X' ) { pcmd.set_msg( "PEDT016B", 4 ) ; }
		else if ( fcx_parms.f_excl == 'N' ) { pcmd.set_msg( "PEDT016C", 4 ) ; }
		else                                { pcmd.set_msg( "PEDT016D", 4 ) ; }
	}
	else if ( fcx_parms.f_dir == 'N' && !fcx_parms.f_top )
	{
		if ( fcx_parms.f_slab != "" )
		{
			vreplace( "ZVAL1", fcx_parms.f_slab ) ;
			vreplace( "ZVAL2", fcx_parms.f_elab ) ;
			pcmd.set_msg( "PEDT016E", 4 ) ;
		}
		else
		{
			pcmd.set_msg( "PEDT013V", 4 ) ;
		}
	}
	else if ( fcx_parms.f_dir == 'P' && !fcx_parms.f_bottom )
	{
		if ( fcx_parms.f_slab != "" )
		{
			vreplace( "ZVAL1", fcx_parms.f_slab ) ;
			vreplace( "ZVAL2", fcx_parms.f_elab ) ;
			pcmd.set_msg( "PEDT016F", 4 ) ;
		}
		else
		{
			pcmd.set_msg( "PEDT013W", 4 ) ;
		}
	}
	else
	{
		TYPE = typList[ fcx_parms.f_mtch ] ;
		if ( fcx_parms.f_slab != "" )
		{
			vreplace( "ZVAL1", fcx_parms.f_slab ) ;
			vreplace( "ZVAL2", fcx_parms.f_elab ) ;
			pcmd.set_msg( "PEDT016G", 4 ) ;
		}
		else
		{
			pcmd.set_msg( "PEDT013H", 4 ) ;
		}
	}
}


string PEDIT01::setFoundString()
{
	// Called to set the found/notfound string in find/change/exclude messages

	string s ;

	if ( fcx_parms.f_rstring == "" )
	{
		s = fcx_parms.f_ostring ;
		if ( fcx_parms.f_hex )
		{
			return "X'" + s + "'" ;
		}
	}
	else
	{
		s = fcx_parms.f_rstring ;
		if ( fcx_parms.f_mtch == 'P' )
		{
			s.pop_back() ;
		}
		else if ( fcx_parms.f_mtch == 'S' )
		{
			s.erase( 0, 1 ) ;
		}
		if ( fcx_parms.f_hex )
		{
			return "X'" + cs2xs( s ) + "'" ;
		}
	}


	if ( any_of( s.begin(), s.end(),
		[]( char c )
		{
			return ( !isprint( c ) ) ;
		} ) )
	{
		s = "X'" + cs2xs( s ) + "'" ;
	}
	else
	{
		s = "'" + s + "'" ;
	}
	return s ;
}


bool PEDIT01::checkLabel1( const string& lab, int nestlvl )
{
	// Return true if line label has a valid user format
	// For level 0 (visible), max chars is 5.  Higher than level 0, max chars is 8.

	int i ;
	int l ;

	l = lab.size() ;

	if ( nestlvl == 0 )
	{
		if ( l < 2 || l > 6  ) { return false ; }
	}
	else
	{
		if ( l < 2 || l > 9  ) { return false ; }
	}
	if ( lab[ 1 ] == 'Z' ) { return false ; }

	if ( !isupper( lab[ 1 ] ) ) { return false ; }

	for ( i = 2 ; i < l ; i++ )
	{
		if ( !isdigit( lab[ i ] ) &&
		     !isupper( lab[ i ] ) ) { return false ; }
	}
	return true ;
}


bool PEDIT01::checkLabel2( const string& lab, int nestlvl )
{
	// Return true if line label has a valid format
	// For level 0 (visible), max chars is 5.  Higher than level 0, max chars is 8.

	int i ;
	int l ;

	l = lab.size() ;

	if ( nestlvl == 0 )
	{
		if ( l < 2 || l > 6  ) { return false ; }
	}
	else
	{
		if ( l < 2 || l > 9  ) { return false ; }
	}

	if ( lab.front() != '.' || !isupper( lab[ 1 ] ) ) { return false ; }

	for ( i = 2 ; i < l ; i++ )
	{
		if ( !isdigit( lab[ i ] ) &&
		     !isupper( lab[ i ] ) ) { return false ; }
	}
	return true ;
}


void PEDIT01::getEditProfile( const string& prof )
{
	// Get edit profile, and set ZEDPROF.  If it does not exist, call createEditProfile() to create

	// ZEDPFLAG :
	//      [0]:  Profile locked if '1'
	//      [1]:  Autosave On if '1'
	//      [2]:  Nulls on if '1'
	//      [3]:  Caps On if '1'
	//      [4]:  Hex On if '1'
	//      [5]:  File Tabs (if '1', convert from spaces -> tabs on save)
	//      [6]:  Recover (if '1', create a backup on edit entry)
	//      [7]:  SETUNDO On if '1'
	//      [8]:  Hilight On if '1'
	//      [9]:  Tabs On if '1'
	//      [10]: Tabs On All if '1' (else STD)
	//      [11]: Autosave PROMPT/NOPROMPT
	//      [12]: Nulls STD if '0', ALL if '1'
	//      [13]: Hilight If logic if '1'
	//      [14]: Hilight Do logic if '1'
	//      [15]: Hilight Parenthesis if '1'
	//      [16]: Hex vert if '1'
	//      [17]: Cut Append if '1'
	//      [18]: Paste Delete if '1'
	//      [19]: Position find/c/x to target line
	//      [20]: Hilite find phrase
	//      [21]: Hilite cursor phrase
	//      [22]: Undo keep

	// If RECOV is OFF, set saveLevel = -1 to de-activate this function

	string uprof    ;
	string tabName  ;
	string flds     ;
	string v_list   ;
	string v_list1  ;
	string v_list2  ;

	tabName = zapplid + "EDIT" ;

	flds    = "(ZEDPFLAG,ZEDPMASK,ZEDPBNDL,ZEDPBNDR,ZEDPTABC,ZEDPTABS,ZEDPTABZ,ZEDPRCLC," \
		  "ZEDPHLLG,ZEDPIMAC,ZEDPFLG2,ZEDPFLG3)" ;

	v_list1 = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;
	v_list2 = "ZEDPTYPE ZEDPHLLG ZEDPIMAC ZEDPFLG2 ZEDPFLG3" ;
	v_list  = v_list1 + " " + v_list2 ;

	vdefine( v_list1, &ZEDPFLAG, &ZEDPMASK, &ZEDPBNDL, &ZEDPBNDR, &ZEDPTABC, &ZEDPTABS, &ZEDPTABZ, &ZEDPRCLC ) ;
	vdefine( v_list2, &ZEDPTYPE, &ZEDPHLLG, &ZEDPIMAC, &ZEDPFLG2, &ZEDPFLG3 ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( tabName, NOWRITE, uprof, SHARE ) ;
	if ( RC == 8 )
	{
		tbcreate( tabName, "ZEDPTYPE", flds, WRITE, NOREPLACE, uprof ) ;
		if ( RC > 0 ) { uabend( "PEDT015W", "TBCREATE" ) ; }
	}

	tbvclear( tabName ) ;
	ZEDPTYPE = prof     ;

	tbget( tabName ) ;
	if ( RC == 8 )
	{
		tbclose( tabName ) ;
		tbopen( tabName, WRITE, uprof ) ;
		if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }
		createEditProfile( tabName, prof ) ;
		pcmd.set_msg( "PEDT014D", 4 ) ;
	}

	tbclose( tabName ) ;

	ZEDPFLAG.resize( 32, '0' ) ;

	profLock        = ( ZEDPFLAG[0]  == '1' ) ;
	profASave       = ( ZEDPFLAG[1]  == '1' ) ;
	profNulls       = ( ZEDPFLAG[2]  == '1' ) ;
	profCaps        = ( ZEDPFLAG[3]  == '1' ) ;
	profHex         = ( ZEDPFLAG[4]  == '1' ) ;
	profXTabs       = ( ZEDPFLAG[5]  == '1' ) ;
	profRecover     = ( ZEDPFLAG[6]  == '1' ) ;
	profUndoOn      = ( ZEDPFLAG[7]  == '1' ) ;
	profHilight     = ( ZEDPFLAG[8]  == '1' ) ;
	profTabs        = ( ZEDPFLAG[9]  == '1' ) ;
	profATabs       = ( ZEDPFLAG[10] == '1' ) ;
	profSaveP       = ( ZEDPFLAG[11] == '1' ) ;
	profNullA       = ( ZEDPFLAG[12] == '1' ) ;
	profIfLogic     = ( ZEDPFLAG[13] == '1' ) ;
	profDoLogic     = ( ZEDPFLAG[14] == '1' ) ;
	profParen       = ( ZEDPFLAG[15] == '1' ) ;
	profVert        = ( ZEDPFLAG[16] == '1' ) ;
	profCutReplace  = ( ZEDPFLAG[17] == '1' ) ;
	profPasteKeep   = ( ZEDPFLAG[18] == '1' ) ;
	profPosFcx      = ( ZEDPFLAG[19] == '1' ) ;
	profFindPhrase  = ( ZEDPFLAG[20] == '1' ) ;
	profCsrPhrase   = ( ZEDPFLAG[21] == '1' ) ;
	profUndoKeep    = ( ZEDPFLAG[22] == '1' ) ;
	maskLine        = ZEDPMASK         ;
	tabsLine        = ZEDPTABS         ;
	tabsChar        = ZEDPTABC.front() ;
	LeftBnd         = ds2d( ZEDPBNDL ) ;
	RightBnd        = ds2d( ZEDPBNDR ) ;
	profXTabz       = ds2d( ZEDPTABZ ) ;
	recoverLoc      = ZEDPRCLC         ;
	profLang        = ZEDPHLLG         ;
	profIMAC        = ZEDPIMAC         ;
	vdelete( v_list ) ;

	if ( profHilight && profLang == "AUTO" )
	{
		detLang = determineLang() ;
	}
	else
	{
		detLang = profLang ;
	}

	ZEDPROF = prof ;
	vput( "ZEDPROF", PROFILE ) ;

	if ( !profUndoOn ) { saveLevel = -1 ; }
}


void PEDIT01::saveEditProfile( const string& prof )
{
	// Retrieve the edit profile, apply changes and save.  Call createEditProfile() if it does not exist.

	// ZEDPFLAG :
	//      [0]: Profile locked    [8]:  Hilight On                        [16]: Hex Vert
	//      [1]: Autosave          [9]:  Tabs On                           [17]: Cut Replace
	//      [2]: Nulls             [10]: Tabs On All (else STD)            [18]: Paste Keep
	//      [3]: Caps              [11]: Autosave PROMPT                   [19]: Position find
	//      [4]: Hex               [12]: Nulls are STD if off, ALL if on   [20]: Find phrase
	//      [5]: File Tabs         [13]: Hilight If logic on               [21]: Cursor phrase
	//      [6]: Recover On        [14]: Hilight Do logic on               [22]: Undo keep
	//      [7]: SETUNDO On        [15]: Hilight Parenthesis on

	string uprof    ;
	string tabName  ;
	string v_list   ;
	string v_list1  ;
	string v_list2  ;

	tabName = zapplid + "EDIT" ;

	v_list1 = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;
	v_list2 = "ZEDPTYPE ZEDPHLLG ZEDPIMAC ZEDPFLG2 ZEDPFLG3" ;
	v_list  = v_list1 + " " + v_list2 ;

	vdefine( v_list1, &ZEDPFLAG, &ZEDPMASK, &ZEDPBNDL, &ZEDPBNDR, &ZEDPTABC, &ZEDPTABS, &ZEDPTABZ, &ZEDPRCLC ) ;
	vdefine( v_list2, &ZEDPTYPE, &ZEDPHLLG, &ZEDPIMAC, &ZEDPFLG2, &ZEDPFLG3 ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( tabName, WRITE, uprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbvclear( tabName ) ;

	ZEDPTYPE = prof ;

	tbget( tabName ) ;
	if ( RC == 8 ) { createEditProfile( tabName, prof ) ; }

	ZEDPFLAG.resize( 32, '0' ) ;
	ZEDPFLAG[0] = ZeroOne[ profLock ] ;

	if ( ZEDPIMAC == "" ) { ZEDPIMAC = "NONE"             ; }
	if ( ZEDPFLG2 == "" ) { ZEDPFLG2 = "0000000000000000" ; }
	if ( ZEDPFLG3 == "" ) { ZEDPFLG3 = "0000000000000000" ; }

	if ( !profLock )
	{
		ZEDPFLAG[1]  = ZeroOne[ profASave   ] ;
		ZEDPFLAG[2]  = ZeroOne[ profNulls   ] ;
		ZEDPFLAG[3]  = ZeroOne[ profCaps    ] ;
		ZEDPFLAG[4]  = ZeroOne[ profHex     ] ;
		ZEDPFLAG[5]  = ZeroOne[ profXTabs   ] ;
		ZEDPFLAG[6]  = ZeroOne[ profRecover ] ;
		ZEDPFLAG[7]  = ZeroOne[ profUndoOn  ] ;
		ZEDPFLAG[8]  = ZeroOne[ profHilight ] ;
		ZEDPFLAG[9]  = ZeroOne[ profTabs    ] ;
		ZEDPFLAG[10] = ZeroOne[ profATabs   ] ;
		ZEDPFLAG[11] = ZeroOne[ profSaveP   ] ;
		ZEDPFLAG[12] = ZeroOne[ profNullA   ] ;
		ZEDPFLAG[13] = ZeroOne[ profIfLogic ] ;
		ZEDPFLAG[14] = ZeroOne[ profDoLogic ] ;
		ZEDPFLAG[15] = ZeroOne[ profParen   ] ;
		ZEDPFLAG[16] = ZeroOne[ profVert    ] ;
		ZEDPFLAG[17] = ZeroOne[ profCutReplace ] ;
		ZEDPFLAG[18] = ZeroOne[ profPasteKeep  ] ;
		ZEDPFLAG[19] = ZeroOne[ profPosFcx     ] ;
		ZEDPFLAG[20] = ZeroOne[ profFindPhrase ] ;
		ZEDPFLAG[21] = ZeroOne[ profCsrPhrase  ] ;
		ZEDPFLAG[22] = ZeroOne[ profUndoKeep   ] ;
		ZEDPMASK     = maskLine ;
		ZEDPTABS     = tabsLine ;
		ZEDPTABC     = tabsChar ;
		ZEDPBNDL     = d2ds( LeftBnd )   ;
		ZEDPBNDR     = d2ds( RightBnd )  ;
		if ( XTabz == 0 ) { ZEDPTABZ = d2ds( profXTabz ) ; }
		else              { ZEDPTABZ = d2ds( XTabz )     ; }
		ZEDPRCLC     = recoverLoc        ;
		ZEDPHLLG     = profLang          ;
		ZEDPIMAC     = profIMAC          ;
	}
	tbmod( tabName ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBMOD" ) ; }

	tbsort( tabName, "(ZEDPTYPE,C,A)" ) ;

	tbclose( tabName ) ;

	vdelete( v_list ) ;
}


void PEDIT01::delEditProfile( const string& prof )
{
	// Delete an edit profile.  'ALL' deletes all profiles and then resets DEFAULT.
	// If the one being deleted is in use, switch the session to DEFAULT

	string uprof   ;
	string tabName ;
	string v_list  ;
	string v_list1 ;
	string v_list2 ;

	tabName = zapplid + "EDIT" ;

	v_list1 = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;
	v_list2 = "ZEDPTYPE ZEDPHLLG ZEDPIMAC ZEDPFLG2 ZEDPFLG3" ;
	v_list  = v_list1 + " " + v_list2 ;

	vdefine( v_list1, &ZEDPFLAG, &ZEDPMASK, &ZEDPBNDL, &ZEDPBNDR, &ZEDPTABC, &ZEDPTABS, &ZEDPTABZ, &ZEDPRCLC ) ;
	vdefine( v_list2, &ZEDPTYPE, &ZEDPHLLG, &ZEDPIMAC, &ZEDPFLG2, &ZEDPFLG3 ) ;
	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( tabName, WRITE, uprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	if ( prof == "ALL" )
	{
		while ( true )
		{
			tbtop( tabName ) ;
			if ( RC > 0 ) { break ; }
			tbskip( tabName ) ;
			if ( RC > 0 ) { break ; }
			tbdelete( tabName ) ;
			if ( RC > 0 ) { uabend( "PEDT015W", "TBDELETE" ) ; }
		}
		pcmd.set_msg( "PEDT014L", 4 ) ;
	}
	else
	{
		tbvclear( tabName ) ;
		ZEDPTYPE = prof     ;
		tbdelete( tabName ) ;
		if ( RC > 0 )
		{
			tbclose( tabName ) ;
			pcmd.set_msg( "PEDT014J" )  ;
			vdelete( v_list )  ;
			return             ;
		}
		pcmd.set_msg( "PEDT014M", 4 ) ;
		if ( prof != "DEFAULT" && prof != ZEDPROF )
		{
			tbclose( tabName ) ;
			vdelete( v_list )  ;
			return ;
		}
	}
	tbclose( tabName ) ;

	getEditProfile( "DEFAULT" ) ;

	vdelete( v_list ) ;
}


void PEDIT01::resetEditProfile()
{
	// Resest DEFAULT edit profile to its default values

	string uprof   ;
	string tabName ;
	string oProf   ;
	string v_list  ;
	string v_list1 ;
	string v_list2 ;

	tabName = zapplid + "EDIT" ;

	v_list1 = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;
	v_list2 = "ZEDPTYPE ZEDPHLLG ZEDPIMAC ZEDPFLG2 ZEDPFLG3" ;
	v_list  = v_list1 + " " + v_list2 ;

	vdefine( v_list1, &ZEDPFLAG, &ZEDPMASK, &ZEDPBNDL, &ZEDPBNDR, &ZEDPTABC, &ZEDPTABS, &ZEDPTABZ, &ZEDPRCLC ) ;
	vdefine( v_list2, &ZEDPTYPE, &ZEDPHLLG, &ZEDPIMAC, &ZEDPFLG2, &ZEDPFLG3 ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( tabName, WRITE, uprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbvclear( tabName )  ;

	ZEDPTYPE = "DEFAULT" ;
	tbdelete( tabName )  ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBDELETE" ) ; }

	tbclose( tabName ) ;

	oProf = ZEDPROF ;

	getEditProfile( "DEFAULT" ) ;
	pcmd.set_msg( "PEDT014K", 4 ) ;

	ZEDPROF = oProf   ;
	vput( "ZEDPROF", PROFILE ) ;
	vdelete( v_list ) ;
}


void PEDIT01::createEditProfile( const string& tabName, const string& prof )
{
	// Create a default entry for edit profile 'prof' in table 'tabName'
	// Called when table is already open for update, and the table variables have been vdefined

	// Defaults: AUTOSAVE OFF PROMPT, RECOVER ON, UNDO ON, HILIGHT ON AUTO/LOGIC/PAREN, HEX VERT[16]
	//           Hilight cursor phrase[21]

	tbvclear( tabName ) ;
	ZEDPTYPE = prof     ;

	ZEDPFLAG = "00000011100101111000010000000000" ;
	ZEDPMASK = ""     ;
	ZEDPBNDL = "1"    ;
	ZEDPBNDR = "0"    ;
	ZEDPTABC = " "    ;
	ZEDPTABS = ""     ;
	ZEDPTABZ = "8"    ;
	ZEDPRCLC = zhome + "/" ;
	ZEDPHLLG = "AUTO" ;
	ZEDPIMAC = "NONE" ;
	ZEDPFLG2 = "0000000000000000" ;
	ZEDPFLG3 = "0000000000000000" ;

	tbadd( tabName )  ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBADD" ) ; }
}


string PEDIT01::getColumnLine( int s )
{
	// Create column line.  If start position, s, not specified, use startCol and data width, else
	// use a width of right screen position + 256

	int i ;
	int j ;
	int k ;
	int l ;

	string t = "" ;

	if ( s == 0 )
	{
		s = startCol ;
		l = ZDATAW   ;
	}
	else
	{
		l = startCol + ZDATAW + 256 ;
	}

	i = s % 10 ;
	j = s / 10 ;

	if ( i > 0 )
	{
		j++ ;
		t = substr( "----+----", i, 10-i ) ;
	}

	for ( k = 0 ; k <= l/10 ; k++, j++ )
	{
		t += d2ds( j%10 ) + "----+----" ;
	}
	return t.substr( 0, l ) ;
}


void PEDIT01::getLineData( vector<iline *>::iterator it )
{
	// Get the data for macro command MASKLINE, TABSLINE, LINE(_BEFORE/_AFTER) = data

	if ( miBlock.var1 != "" ) { return ; }

	miBlock.value = mergeLine( miBlock.value, it ) ;
	return ;

}


string PEDIT01::mergeLine( const string& s, vector<iline *>::iterator it )
{
	// Expand the last parameter passed and call ourselves again for the rest until
	// no more parameters, then merge or overlay the results on return.

	int  p1 ;
	char qt ;

	bool quote  ;
	bool templt ;

	string t1  ;
	string t2  ;
	string r1  ;
	string r2  ;

	string vw1 ;
	string vw2 ;
	string vw3 ;

	string::const_iterator its ;
	string::const_iterator itt ;

	if ( miBlock.fatal ) { return "" ; }

	quote  = false   ;
	templt = false   ;
	itt    = s.end() ;
	for ( quote = false, its = s.begin() ; its != s.end() ; its++ )
	{
		if ( !quote && ((*its) == '"' || (*its) == '\'' )) { qt = (*its) ; quote = true ; continue ; }
		if (  quote &&  (*its) == qt )  { quote = false ; continue ; }
		if ( !quote &&  (*its) == '+' ) { itt = its                ; }
	}

	if ( itt != s.end() )
	{
		t1.assign( s.begin(), itt ) ;
		itt++ ;
		t2.assign( itt, s.end()   ) ;
	}
	else
	{
		t1 = "" ;
		t2 = s  ;
	}

	trim( t2 ) ;
	vw1 = upper( word( t2, 1 ) ) ;
	vw2 = upper( word( t2, 2 ) ) ;
	vw3 = upper( word( t2, 3 ) ) ;

	if ( vw1 == "LINE" )
	{
		if ( vw2 != "" )
		{
			if ( vw3 != "" )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				return "" ;
			}
			p1 = getLabelLine( iupper( vw2 ) ) ;
			if ( p1 < 1 ) { return "" ; }
			r1 = data.at( p1 )->get_idata() ;
		}
		else
		{
			if ( it == data.end() )
			{
				miBlock.seterror( "PEDM013A", "LINE keyword missing lptr parameter", 20 ) ;
				return "" ;
			}
			r1 = (*it)->get_idata() ;
		}
	}
	else if ( t2.size() > 1 && ( t2.front() == '"' || t2.front() == '\'' ) )
	{
		if ( t2.front() != t2.back() )
		{
			miBlock.seterror( "PEDM012W", 20 ) ;
			return "" ;
		}
		r1 = t2.substr( 1, t2.size() - 2 )  ;
	}
	else if ( t2.size() > 1 && t2.front() == '(' )
	{
		if ( !getVariables( 1, t2, vw1 ) ) { return "" ; }
		if ( trim( t2 ) != "" )
		{
			miBlock.seterror( "PEDM011H", 20 ) ;
			return "" ;
		}
		miBlock.macAppl->vcopy( vw1, r1, MOVE ) ;
	}
	else if ( t2.size() > 1 && t2.front() == '<' )
	{
		if ( t2.back() != '>' )
		{
			miBlock.seterror( "PEDM012Y", 20 ) ;
			return "" ;
		}
		templt = true ;
		r1     = t2   ;
	}
	else if ( vw1 == "MASKLINE" )
	{
		r1 = maskLine ;
	}
	else if ( vw1 == "TABSLINE" )
	{
		r1 = tabsLine ;
	}
	else
	{
		if ( words( t2 ) > 1 )
		{
			miBlock.seterror( "PEDM011P", 20 ) ;
			return "" ;
		}
		r1 = t2 ;
	}

	if ( t1 == "" )
	{
		return r1 ;
	}
	else
	{
		r2 = mergeLine( t1, it ) ;
		if ( templt )
		{
			r1 = templat( r1, r2 ) ;
		}
		else
		{
			r1 = overlay2( r1, r2 ) ;
		}
		return r1 ;
	}
}


bool PEDIT01::getVariables( int n, string& s, string& vars )
{
	// Get variable of the form '( var var var )'
	// and remove from string 's'.  Variables go in 'vars' and the max allowed is n

	int p1 ;
	int ws ;

	string w ;

	p1 = s.find( ')', 1 ) ;
	if ( p1 == string::npos )
	{
		miBlock.seterror( "PEDM011F" ) ;
		return false ;
	}
	vars = s.substr( 1, p1-1 ) ;
	s.erase( 0, p1+1 )         ;

	if ( trim( vars ) == "" )
	{
		miBlock.seterror( "PEDM011G" ) ;
		return false ;
	}
	ws = words( vars ) ;
	if ( ws > n )
	{
		miBlock.seterror( "PEDM011H" ) ;
		return false ;
	}
	for ( int i = 1 ; i <= ws ; i++ )
	{
		w = word( vars, i ) ;
		if ( !isvalidName( w ) )
		{
			miBlock.seterror( "PEDM011D", w, 20 ) ;
			return false ;
		}
	}
	return true ;
}


string PEDIT01::createTempName()
{
	path temp ;

	vcopy( "ZUSER", zuser, MOVE ) ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;
	temp = temp_directory_path() / unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;

	return temp.native() ;
}


string PEDIT01::determineLang()
{
	// Try to determine the language, first from the extension, then the directory containing the source,
	// then the contents of the file.
	// Limit the scan to the first 100 lines of code

	// This routine also determines the profile name on a PROFILE USE TYPE command

	// Returned language must exist in eHilight (hiRoutine function map) or an exception will occur

	int p ;
	int i ;

	string s ;
	string w ;

	string * t ;

	vector<iline * >::iterator it ;

	p = ZFILE.find_last_of( '.' ) ;
	if ( p != string::npos )
	{
		s = ZFILE.substr( p+1 ) ;
		if ( findword( s, "c cpp h hpp" ) ) { return "CPP"  ; }
		if ( findword( s, "rex rexx rx" ) ) { return "REXX" ; }
	}

	if ( ZFILE.find( "/rexx/" ) != string::npos ) { return "REXX"  ; }
	if ( ZFILE.find( "/tmp/"  ) != string::npos ) { return "OTHER" ; }

	for ( i = 0, it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->isValidFile() ) { continue ; }
		t = (*it)->get_idata_ptr() ;
		if ( t->size()  == 0   ) { continue       ; }
		if ( t->at( 0 ) == '*' ) { return "ASM"   ; }
		if ( t->at( 0 ) == ')' ) { return "PANEL" ; }
		w = word( *t, 1 ) ;
		if ( findword( w, "TITLE CSECT DSECT MACRO START COPY" ) )
		{
			return "ASM"  ;
		}
		if ( i == 0 && t->find( "rexx" ) != string::npos )
		{
			return "REXX" ;
		}
		if ( w == "/*" ) { return "CPP" ; }
		if ( ++i > 100 ) { break        ; }
	}
	return "DEFAULT" ;
}


void PEDIT01::run_macro( bool imacro )
{
	// Call PEDRXM1 for REXX macros.
	// Set the initial values for mRow and mCol, macro CURSOR position
	// Process screen changes and line commands if no PROCESS has been done when last macro terminates.

	// Return from the macro is in miBlock.exitRC
	// 0    Normal completion, command line blanked and cursor as set by the macro
	// 1    Cursor placed on command line, and command line is blanked
	// 4,8  Treated as 0
	// 12+  Error.  Cursor placed on the command line and edit macro remains
	// 20
	// 24   Nesting level of 255 exceeded
	// 28

	uint dl ;

	miBlock.clear() ;

	ApplUserData[ taskid() ] = (void *)&miBlock ;
	miBlock.editAppl         = this ;
	miBlock.etaskid          = taskid() ;
	miBlock.imacro           = imacro   ;

	miBlock.setMacro( word( pcmd.get_cmd(), 1 ) )  ;
	if ( !miBlock.getMacroFileName( ZORXPATH ) )
	{
		pcmd.set_msg( miBlock.RC > 8 ? "PEDM012Q" : "PEDT015A" ) ;
		return ;
	}

	if ( CURFLD != "ZAREA" )
	{
		mRow = 1 ;
		mCol = 0 ;
	}
	else if ( getLabelIndex( ".ZFIRST" ) == -2 )
	{
		mRow = 0 ;
		mCol = 0 ;
	}
	else if ( aURID > 0 )
	{
		dl = getLine( aURID ) ;
		if ( !data.at( dl )->isValidFile() )
		{
			dl = getNextFileLine( dl ) ;
			if ( dl == 0 )
			{
				dl   = getLabelIndex( ".ZLAST" ) ;
				mRow = getFileLine( dl ) + 1     ;
				mCol = data.at( dl )->get_idata_len() + 1 ;
			}
			else
			{
				mRow = getFileLine( dl ) + 1 ;
				mCol = 0 ;
			}
		}
		else
		{
			mRow = getFileLine( dl ) + 1 ;
			mCol = aCol > 8 ? aCol + startCol - 9 : 0 ;
		}
	}
	else
	{
		dl   = getLabelIndex( ".ZLAST" ) ;
		mRow = getFileLine( dl ) + 1     ;
		mCol = data.at( dl )->get_idata_len() + 1 ;
	}

	macroRunning = true ;
	control( "ERRORS", "RETURN" ) ;
	select( "PGM(PEDRXM1) PARM("+d2ds( taskid() ) +") NEWPOOL" ) ;
	if ( RC > 8 )
	{
		pcmd.set_msg( "PEDT015B" ) ;
	}
	control( "ERRORS", "CANCEL" ) ;

	if ( miBlock.mfound && !miBlock.processed && !miBlock.imacro )
	{
		miBlock.m_cmd = EM_PROCESS ;
		actionService() ;
	}

	if ( miBlock.fatal )
	{
		pcmd.cond_reset() ;
		pcmd.setRC( 20 ) ;
		placeCursor( 0, 0 ) ;
		vreplace( "ZSTR1", miBlock.emacro ) ;
	}
	else if ( miBlock.getExitRC() == 1 )
	{
		placeCursor( 0, 0 ) ;
	}
	else if ( miBlock.setcursor )
	{
		aRow = getDataLine( mRow ) ;
		aCol = mCol + 9 + startCol ;
		placeCursor( data.at( aRow )->il_URID, 4, mCol ) ;
	}
	if ( miBlock.getExitRC() < 12 )
	{
		pcmd.cond_reset() ;
	}
	else
	{
		if ( !miBlock.msgset() )
		{
			pcmd.set_msg( "PEDT014X", miBlock.getExitRC() ) ;
		}
		else
		{
			pcmd.set_msg( miBlock.msgid, miBlock.getExitRC() ) ;
		}
		vreplace( "ZSTR1", miBlock.emacro ) ;
		vreplace( "ZSTR2", d2ds( miBlock.getExitRC() ) ) ;
		placeCursor( 0, 0 ) ;
	}
	ApplUserData[ taskid() ] = NULL ;
	nestLevel = 0   ;
	miBlock.clear() ;
	macroRunning = false ;
	rebuildZAREA = true  ;
}


void PEDIT01::isredit( const string& s )
{
	string s1 = s ;

	if ( miBlock.scanOn() )
	{
		s1 = miBlock.macAppl->sub_vars( s1 ) ;
	}

	miBlock.parseStatement( s1, defNames ) ;

	if ( miBlock.fatal || miBlock.runmacro )
	{
		return ;
	}

	miBlock.query ? querySetting() : actionService() ;
}


void PEDIT01::querySetting()
{
	// Retrieve setting in miBlock.keyword and store in variable names in miBlock.var1 and var2.
	// These variables are in the macro application function pool, not the editor function pool so use
	// the macAppl pointer in the mib when invoking variable services.

	int l1  ;
	int l2  ;
	int p1  ;
	int lvl ;
	int dl  ;

	string kw2 ;
	string t1  ;
	string lab ;

	kw2 = word( miBlock.kphrase, 2 ) ;
	pApplication * macAppl = miBlock.macAppl ;

	switch ( miBlock.m_cmd )
	{
	case EM_AUTOSAVE:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, profASave ? "ON" : "OFF" ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, profSaveP ? "PROMPT" : "NOPROMPT" ) ;
			}
			break ;

	case EM_CAPS:
			macAppl->vreplace( miBlock.var1, profCaps ? "ON" : "OFF" ) ;
			break ;

	case EM_CHANGE_COUNTS:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( fcx_parms.f_ch_occs, 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( fcx_parms.f_ch_errs, 8 ) ) ;
			}
			break ;

	case EM_CURSOR:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( mRow, 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( mCol, 5 ) ) ;
			}
			break ;

	case EM_DATA_CHANGED:
			macAppl->vreplace( miBlock.var1, fileChanged ? "YES" : "NO" ) ;
			break ;

	case EM_DATASET:
			macAppl->vreplace( miBlock.var1, ZFILE ) ;
			break ;

	case EM_DISPLAY_COLS:
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( startCol, 5 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( startCol+ZDATAW-1, 5 ) ) ;
			}
			break ;

	case EM_DISPLAY_LINES:
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
			if ( getLabelIndex( ".ZFIRST" ) == -2 )
			{
				miBlock.setRC( 8 ) ;
				break ;
			}
			l1 = -1 ;
			for ( int i = 0 ; i < ZAREAD ; i++ )
			{
				if ( s2data.at( i ).ipos_hex > 0 || s2data.at( i ).ipos_div ) { continue ; }
				dl = s2data.at( i ).ipos_dl ;
				if ( data.at( dl )->isValidFile() )
				{
					if ( l1 == -1 ) { l1 = dl ; }
					l2 = dl ;
				}
			}
			if ( l1 == -1 )
			{
				miBlock.setRC( 4 ) ;
				break ;
			}
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( getFileLine( l1 ) + 1, 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( getFileLine( l2 ) + 1, 8 ) ) ;
			}
			break ;

	case EM_FIND_COUNTS:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( fcx_parms.f_fd_occs, 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( fcx_parms.f_fd_lnes, 8 ) ) ;
			}
			break ;

	case EM_EXCLUDE_COUNTS:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( fcx_parms.f_ex_occs, 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( fcx_parms.f_ex_lnes, 8 ) ) ;
			}
			break ;

	case EM_HEX:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, profHex ? "ON" : "OFF" ) ;
			}
			if ( miBlock.var2 != "" )
			{
				if ( profHex )
				{
					macAppl->vreplace( miBlock.var2, profVert ? "VERT" : "DATA" ) ;
				}
				else
				{
					macAppl->vreplace( miBlock.var2, "" ) ;
				}
			}
			break ;

	case EM_LABEL:
			p1 = getLabelLine( iupper( kw2 ) ) ;
			if ( p1 < 1 )
			{
				if ( miBlock.var1 != "" ) { macAppl->vreplace( miBlock.var1, "" ) ; }
				if ( miBlock.var2 != "" ) { macAppl->vreplace( miBlock.var2, "" ) ; }
				break ;
			}
			data.at( p1 )->getLabelInfo( nestLevel, lab, lvl ) ;
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, lab ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( lvl, 3 ) ) ;
			}
			if ( lab == "" ) { miBlock.setRC( 4 ) ; }
			break ;

	case EM_LINE:
			p1 = getLabelLine( iupper( kw2 ) ) ;
			if ( p1 < 1 ) { break ; }
			macAppl->vreplace( miBlock.var1, data.at( p1 )->get_idata() ) ;
			break ;

	case EM_LINENUM:
			if ( !checkLabel2( kw2, miBlock.nestlvl ) )
			{
				miBlock.seterror( "PEDM011R" ) ;
				return ;
			}
			if ( kw2 == ".ZCSR" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( mRow, 8 ) ) ;
				break ;
			}
			p1 = getLabelIndex( kw2 ) ;
			if ( p1 == -1 )
			{
				miBlock.setRC( 8 ) ;
				macAppl->vreplace( miBlock.var1, "00000000" ) ;
				return ;
			}
			else if ( p1 == -2 )
			{
				miBlock.setRC( 4 ) ;
				macAppl->vreplace( miBlock.var1, "00000000" ) ;
				return ;
			}
			p1 = getFileLine( p1 ) + 1 ;
			macAppl->vreplace( miBlock.var1, d2ds( p1, 8 ) ) ;
			break ;

	case EM_IMACRO:
			macAppl->vreplace( miBlock.var1, profIMAC ) ;
			break ;

	case EM_MACRO_LEVEL:
			macAppl->vreplace( miBlock.var1, d2ds( miBlock.nestlvl, 3 ) ) ;
			break ;

	case EM_MASKLINE:
			macAppl->vreplace( miBlock.var1, maskLine ) ;
			break ;

	case EM_NULLS:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, profNulls ? "ON" : "OFF" ) ;
			}
			if ( miBlock.var2 != "" )
			{
				if ( profNulls )
				{
					macAppl->vreplace( miBlock.var2, profNullA ? "ALL" : "STD" ) ;
				}
				else
				{
					macAppl->vreplace( miBlock.var2, "" ) ;
				}
			}
			break ;

	case EM_PROFILE:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, ZEDPROF ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, profLock ? "YES" : "NO" ) ;
			}
			break ;

	case EM_SCAN:
			macAppl->vreplace( miBlock.var1, miBlock.scanOn() ? "ON" : "OFF" ) ;
			break ;

	case EM_SEEK_COUNTS:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( fcx_parms.f_sk_occs, 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( fcx_parms.f_sk_lnes, 8 ) ) ;
			}
			break ;

	case EM_TABSLINE:
			macAppl->vreplace( miBlock.var1, tabsLine ) ;
			break ;

	case EM_XSTATUS:
			p1 = getLabelLine( iupper( kw2 ) ) ;
			if ( p1 < 1 ) { break ; }
			macAppl->vreplace( miBlock.var1, data.at( p1 )->il_excl ? "X" : "NX" ) ;
			break ;

	case EM_LINE_AFTER:
	case EM_LINE_BEFORE:
			miBlock.seterror( "PEDM011U" ) ;
			break ;
	default:
			miBlock.seterror( "PEDM011A", 12 ) ;
	}
}


void PEDIT01::actionService()
{
	// Action a service from a command macro.  Details of the service are in the macro interface block.

	// These variables are in the macro application function pool, not the editor function pool so use
	// the macAppl pointer in the mib when invoking variable services.

	// For normal primary commands (macro command syntax and assignment syntax), just create the expected
	// CMD and check the resulting cmdBlock object.

	// For macro command syntax only commands (eg FIND, CHANGE, SEEK..), command must be in the EMCmds map

	int p1    ;
	int n     ;
	int tURID ;

	uint dl   ;

	lcmd cmd  ;

	string t   ;
	string vw1 ;
	string vw2 ;
	string vw3 ;
	string kw1 ;
	string kw2 ;
	string kw3 ;
	string opt ;

	iline * p_iline ;
	vector<iline * >::iterator it ;

	pApplication * macAppl = miBlock.macAppl ;

	vw1 = word( miBlock.value, 1 ) ;
	vw2 = word( miBlock.value, 2 ) ;

	kw1 = word( miBlock.keyopts, 1 ) ;
	kw2 = word( miBlock.keyopts, 2 ) ;
	kw3 = word( miBlock.keyopts, 3 ) ;

	switch ( miBlock.m_cmd )
	{
	case EM_CANCEL:
			if ( words( miBlock.sttment ) > 1 )
			{
				miBlock.seterror( "PEDM011P" ) ;
				break ;
			}
			miBlock.eended = true ;
			termEdit       = true ;
			break ;

	case EM_CURSOR:
			if ( miBlock.var1 != "" )
			{
				macAppl->vcopy( miBlock.var1, vw1, MOVE ) ;
			}
			if ( datatype( vw1, 'W' ) )
			{
				mRow = ds2d( vw1 ) ;
				p1   = getLabelLine( ".ZLAST" ) ;
				if ( p1 == 0 || mRow > p1 )
				{
					mRow = p1 ;
				}
			}
			else
			{
				if ( !checkLabel2( vw1, miBlock.nestlvl ) )
				{
					miBlock.seterror( "PEDM011R" ) ;
					break ;
				}
				p1 = getLabelLine( vw1 ) ;
				if ( p1 < 0 )
				{
					break ;
				}
				else if ( p1 == 0 )
				{
					mRow = 0 ;
				}
				else
				{
					mRow = getFileLine( p1 ) + 1 ;
				}
			}
			if ( miBlock.var2 != "" || datatype( vw2, 'W' ) )
			{
				if ( miBlock.var2 != "" )
				{
					macAppl->vcopy( miBlock.var2, vw2, MOVE ) ;
				}
				mCol = ds2d( vw2 ) ;
				if ( mCol > 1 && mRow > 0 )
				{
					dl = getDataLine( mRow ) ;
					if ( mCol > data.at( dl )->get_idata_len() )
					{
						if ( dl != getLabelIndex( ".ZLAST" ) )
						{
							mCol = 0 ;
							mRow++   ;
						}
						else
						{
							miBlock.seterror( "PEDM013C" ) ;
							break ;
						}
					}
				}
			}
			else if ( vw2 != "" )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
			}
			miBlock.setcursor = true ;
			break ;

	case EM_END:
			if ( words( miBlock.sttment ) > 1 )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			if ( termOK() )
			{
				miBlock.eended = true ;
				termEdit       = true ;
			}
			break ;

	case EM_LABEL:
			if ( vw2 != "" )
			{
				if ( !datatype( vw2, 'W' ) )
				{
					miBlock.seterror( "PEDM011O", "Label level parameter must be numeric" ) ;
					break ;
				}
				n = ds2d( vw2 ) ;
				if ( n < 0 || n > 255 )
				{
					miBlock.seterror( "PEDM012E", 20 ) ;
					break ;
				}
				if ( n > miBlock.nestlvl ) { n = miBlock.nestlvl ; }
			}
			else
			{
				n = miBlock.nestlvl ;
			}
			p1 = getLabelLine( iupper( kw1 ) ) ;
			if ( p1 < 1 ) { break ; }
			if ( !checkLabel1( vw1, n ) )
			{
				miBlock.seterror( "PEDT014", 20 ) ;
				break ;
			}
			if ( data.at( p1 )->setLabel( vw1, n ) > 0 )
			{
				miBlock.setRC( 8 ) ;
			}
			tURID = data.at( p1 )->il_URID ;
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if ( (*it)->il_URID == tURID )     { continue ; }
				if ( (*it)->clearLabel( vw1, n ) ) { break    ; }
			}
			rebuildZAREA = true ;
			break ;

	case EM_LINE:
			p1 = getLabelLine( iupper( kw1 ) ) ;
			if ( p1 < 1 ) { break ; }
			dl = p1 ;
			it = getLineItr( dl ) ;
			getLineData( it )     ;
			if ( miBlock.fatal ) { break ; }
			if ( miBlock.var1 != "" )
			{
				macAppl->vcopy( miBlock.var1, miBlock.value, MOVE ) ;
				if ( macAppl->RC == 8 )
				{
					miBlock.setRC( 8 ) ;
					break ;
				}
			}
			Level++ ;
			if ( (*it)->put_idata( miBlock.value, Level ) )
			{
				fileChanged = true ;
			}
			rebuildZAREA = true ;
			break ;

	case EM_LINE_AFTER:
	case EM_LINE_BEFORE:
			p1 = getLabelLine( iupper( kw1 ) ) ;
			if ( p1 == 0 && miBlock.m_cmd == EM_LINE_AFTER )
			{
				miBlock.setRC( 0 ) ;
			}
			else if ( p1 < 1 )
			{
				break ;
			}
			dl  = p1 ;
			it  = getLineItr( dl ) ;
			opt = word( miBlock.value, 1 ) ;
			if ( findword( opt, "DATALINE INFOLINE MSGLINE NOTELINE" ) )
			{
				idelword( miBlock.value, 1, 1 ) ;
			}
			else
			{
				opt = "DATALINE" ;
			}
			getLineData( it )     ;
			if ( miBlock.fatal ) { break ; }
			if ( miBlock.m_cmd == EM_LINE_AFTER )
			{
				it = getNextDataLine( it ) ;
			}
			if ( miBlock.var1 != "" )
			{
				macAppl->vcopy( miBlock.var1, miBlock.value, MOVE ) ;
				if ( macAppl->RC == 8 )
				{
					miBlock.setRC( 8 ) ;
					break ;
				}
			}
			Level++ ;
			p_iline = new iline( taskid() ) ;
			if      ( opt == "DATALINE" ) { p_iline->il_type = LN_FILE ; fileChanged = true ; }
			else if ( opt == "INFOLINE" ) { p_iline->il_type = LN_INFO ; }
			else if ( opt == "MSGLINE"  ) { p_iline->il_type = LN_MSG  ; }
			else                          { p_iline->il_type = LN_NOTE ; }
			p_iline->put_idata( miBlock.value, Level ) ;
			it = data.insert( it, p_iline ) ;
			rebuildZAREA = true ;
			break ;

	case EM_INSERT:
			vw2 = word( miBlock.sttment, 2 ) ;
			vw3 = word( miBlock.sttment, 3 ) ;
			if ( miBlock.get_sttment_words() > 3 ||
			     vw2 == ""                       ||
			   ( vw3 != "" && !datatype( vw3, 'W' ) ) )
			{
				miBlock.seterror( "PEDM011P" ) ;
				break ;
			}
			p1 = getLabelLine( iupper( vw2 ) ) ;
			if ( p1 < 0 )  { break              ; }
			if ( p1 == 0 ) { miBlock.setRC( 0 ) ; }
			cmd.lcmd_CMD   = LC_I ;
			cmd.lcmd_sURID = data.at( p1 )->il_URID ;
			if ( vw3 == "" ) { cmd.lcmd_Rpt = 1           ; }
			else             { cmd.lcmd_Rpt = ds2d( vw3 ) ; }
			lcmds.clear()          ;
			lcmds.push_back( cmd ) ;
			actionLineCommand( lcmds.begin() ) ;
			break ;

	case EM_MACRO:
			for ( int ws = words( miBlock.keyopts ), i = 1 ; i <= ws ; i++ )
			{
				t = word( miBlock.keyopts, i ) ;
				if ( i < ws ) { macAppl->vreplace( t, word( miBlock.parms, i ) )    ; }
				else          { macAppl->vreplace( t, subword( miBlock.parms, i ) ) ; }
			}
			if ( !miBlock.process  ) { break ; }
	case EM_PROCESS:
			if ( miBlock.processed || miBlock.imacro ) { break ; }
			miBlock.processed = true ;
			getZAREAchanges() ;
			updateData()      ;
			if ( pcmd.error() )  { break ; }

			actionZVERB()   ;
			setLineLabels() ;
			if ( pcmd.error() )  { break ; }

			processNewInserts()  ;
			actionLineCommands() ;
			if ( pcmd.error() )  { break ; }
			break ;

	case EM_MASKLINE:
			if ( miBlock.var1 != "" )
			{
				macAppl->vcopy( miBlock.var1, maskLine, MOVE ) ;
			}
			else
			{
				getLineData( data.end() ) ;
				if ( miBlock.fatal ) { break ; }
				maskLine = miBlock.value ;
			}
			break ;

	case EM_RCHANGE:
			if ( !fcx_parms.f_cset ) { miBlock.seterror( "PEDT013N", 20 ) ; break ; }
			dl = getDataLine( mRow ) ;
			if ( fcx_parms.f_URID == data.at( dl )->il_URID &&
			     fcx_parms.f_success                        &&
			     fcx_parms.f_offset == mCol - 1 )
			{
				actionChange() ;
				mCol = fcx_parms.f_offset + fcx_parms.f_cstring.size() ;
				miBlock.setcursor = true ;
			}
			else
			{
				actionFind() ;
				if ( fcx_parms.f_error ) { miBlock.seterror( pcmd ) ; break ; }
				if ( fcx_parms.f_success )
				{
					actionChange() ;
					mRow = getFileLine( fcx_parms.f_dl ) + 1 ;
					mCol = fcx_parms.f_offset + fcx_parms.f_cstring.size() ;
					miBlock.setcursor = true ;
				}
			}
			if ( !fcx_parms.f_success )
			{
				miBlock.setRC( 4 ) ;
			}
			break ;

	case EM_RFIND:
			if ( !fcx_parms.f_fset ) { miBlock.seterror( "PEDT013M", 20 ) ; break ; }
			actionFind() ;
			if ( fcx_parms.f_error ) { miBlock.seterror( pcmd ) ; break ; }
			if ( fcx_parms.f_success )
			{
				mRow = getFileLine( fcx_parms.f_dl ) + 1 ;
				mCol = fcx_parms.f_offset + 1 ;
				miBlock.setcursor = true ;
			}
			else
			{
				miBlock.setRC( 4 ) ;
			}
			break ;

	case EM_SCAN:
			if ( miBlock.keyopts == "ON" || miBlock.keyopts == "" )
			{
				miBlock.scan = true ;
			}
			else if ( miBlock.keyopts == "OFF" )
			{
				miBlock.scan = false ;
			}
			else
			{
				miBlock.seterror( "PEDM011N" ) ;
			}
			break ;

	case EM_SHIFT:
			vw1 = word( miBlock.sttment, 2 ) ;
			vw2 = word( miBlock.sttment, 3 ) ;
			vw3 = word( miBlock.sttment, 4 ) ;
			if ( miBlock.get_sttment_words() > 4 ||
			     !findword( vw1, "( ) < > [ ]" ) ||
			     vw2 == ""                       ||
			   ( vw3 != "" && !datatype( vw3, 'W' ) ) )
			{
				miBlock.seterror( "PEDM011P" ) ;
				break ;
			}
			p1 = getLabelLine( iupper( vw2 ) ) ;
			if ( p1 < 1 ) { break ; }
			cmd.lcmd_CMD   = LineCMDS[ vw1 ] ;
			cmd.lcmd_sURID = data.at( p1 )->il_URID ;
			cmd.lcmd_eURID = data.at( p1 )->il_URID ;
			if ( vw3 == "" ) { cmd.lcmd_Rpt = findword( vw1, "[ ]" ) ? 1 : 2 ; }
			else             { cmd.lcmd_Rpt = ds2d( vw3 )                    ; }
			lcmds.clear()          ;
			lcmds.push_back( cmd ) ;
			actionLineCommand( lcmds.begin() ) ;
			break ;

	case EM_TABSLINE:
			if ( miBlock.var1 != "" )
			{
				macAppl->vcopy( miBlock.var1, tabsLine, MOVE ) ;
			}
			else
			{
				getLineData( data.end() ) ;
				if ( miBlock.fatal ) { break ; }
				tabsLine = miBlock.value ;
			}
			break ;

	case EM_XSTATUS:
			if ( vw1 != "X" && vw1 != "NX" )
			{
				miBlock.seterror( "PEDM011P" ) ;
				break ;
			}
			p1 = getLabelLine( iupper( kw1 ) ) ;
			if ( p1 < 1 ) { break ; }
			data.at( p1 )->il_excl = ( vw1 == "X" ) ;
			rebuildZAREA = true ;
			break ;

	case EM_LOCATE:
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
	case EM_AUTOSAVE:
	case EM_CAPS:
	case EM_CHANGE:
	case EM_DEFINE:
	case EM_DELETE:
	case EM_EXCLUDE:
	case EM_FIND:
	case EM_FLIP:
	case EM_HEX:
	case EM_HIDE:
	case EM_IMACRO:
	case EM_NULLS:
	case EM_PROFILE:
	case EM_RESET:
	case EM_SAVE:
	case EM_SEEK:
			if (  miBlock.assign ) { pcmd.set_cmd( miBlock.keyword + " " + miBlock.value, defNames ) ; }
			else                   { pcmd.set_cmd( miBlock.sttment, defNames )                       ; }
			if ( !pcmd.error() )
			{
				actionPrimCommand1() ;
				if ( !pcmd.error() && !pcmd.isActioned() )
				{
					actionPrimCommand2() ;
					if ( !pcmd.isActioned() )
					{
						macAppl->vreplace( "ZSTR1", pcmd.get_cmd() ) ;
						macAppl->vreplace( "ZSTR2", miBlock.emacro ) ;
						miBlock.seterror( "PEDM012D" ) ;
						break ;
					}
				}
			}
			miBlock.seterror( pcmd ) ;
			pcmd.cond_reset() ;
			break ;

	case EM_CHANGE_COUNTS:
	case EM_DATA_CHANGED:
	case EM_DATASET:
	case EM_DISPLAY_COLS:
	case EM_DISPLAY_LINES:
	case EM_EXCLUDE_COUNTS:
	case EM_FIND_COUNTS:
	case EM_LINENUM:
	case EM_MACRO_LEVEL:
	case EM_SEEK_COUNTS:
			miBlock.seterror( "PEDM011T" ) ;
			break ;
	default:
			miBlock.seterror( "PEDM011A", 12 ) ;
	}
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PEDIT01 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
