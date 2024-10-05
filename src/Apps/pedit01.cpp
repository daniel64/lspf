/*  Compile with ::                                                                                   */
/* g++  -shared -fPIC -std=c++11 -Wl,-soname,libpedit01.so -o libpedit01.so  pedit01.cpp              */
/* g++  -S -fverbose-asm -shared -fPIC -std=c++11 -Wl,-soname,libpedit01.so -o libpedit01.asm  pedit01.cpp              */
/* g++  -shared -g -fPIC -std=c++11 -Wl,-soname,libpedit01.so -o libpedit01.so pedit01.cpp */
/* g++ -g3 -fsanitize=address -shared -fPIC -std=c++11 -Wl,-soname,libpedit01.so -o libpedit01.so pedit01.cpp */
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

/*********************************************************************************************/
/*                                                                                           */
/* PDF-like editor program                                                                   */
/*                                                                                           */
/* Current status:                                                                           */
/* Most functions work okay                                                                  */
/* Also supports VIEW mode                                                                   */
/* REXX and C++ edit macro support including line command macros                             */
/*                                                                                           */
/*                                                                                           */
/* ZRC/ZRSN exit codes (RC=ZRC in the calling program)                                       */
/*   0/0  Okay - Data saved                                                                  */
/*   0/4  Okay - Data saved but then CANCEL entered when leaving EDIT.                       */
/*                                                                                           */
/*   4/0  Okay - Data not saved.  No changes made to data.                                   */
/*   4/4  Okay - Data not saved.  Browse substituted.                                        */
/*                                                                                           */
/*  14/0  File in use                                                                        */
/*                                                                                           */
/*  20/0  Severe error                                                                       */
/*  20/4  Open for output error                                                              */
/*  20/8  RECLEN too small                                                                   */
/*                                                                                           */
/* ZRESULT contains the relevant message id for RC 0 and 14 (and some RC=20)                 */
/*                                                                                           */
/*********************************************************************************************/

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <boost/thread/thread.hpp>
#include <boost/scope_exit.hpp>

#include <vector>
#include <queue>

#include "../lspfall.h"
#include "ehilight.cpp"
#include "pedit01.h"


using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

#define CLINESZ    8
#define profUndoOn iline::setUNDO[ taskid() ]

#define DATAIN1    0x01
#define DATAIN2    0x02
#define DATAOUT1   0x03
#define DATAOUT2   0x04
#define USERMOD    0x10
#define DATAMOD    0x11

#define MAXLEN     65535

#define CLRTABLE   "EDITCLRS"
#define DEFBACKLOC "/isr"

map<int, edit_find>pedit01::Global_efind_parms ;

LSPF_APP_MAKER( pedit01 )


pedit01::pedit01()
{
	STANDARD_HEADER( "PDF-like editor for lspf", "2.0.0" )

	rebuildZAREA  = true  ;
	rebuildShadow = false ;
	zchanged      = false ;
	macroRunning  = false ;
	cutActive     = false ;
	copyActive    = false ;
	moveActive    = false ;
	pasteActive   = false ;
	colsOn        = false ;
	hideExcl      = false ;
	abendComplete = false ;
	scrollData    = false ;
	termEdit      = false ;
	ichgwarn      = false ;
	LeftBndSet    = false ;
	RightBndSet   = false ;
	explProfile   = true  ;
	XTabz         = 0     ;
	lnumSize      = 0     ;
	lnumSize1     = 0     ;
	lnumSize2     = 0     ;
	lnumS2pos     = 0     ;
	lnummod       = "00"  ;
	modLine       = nullptr ;
	ptopLine      = nullptr ;
	nestLevel     = 0     ;
	aRow          = 0     ;
	aCol          = 0     ;
	aOff          = 0     ;
	zdest         = nullptr ;
	zfrange       = nullptr ;
	zlrange       = nullptr ;
	tflow1        = 0     ;
	tflow2        = 0     ;
	misADDR       = nullptr ;
	zlvline       = 0     ;
	zrange_cmd    = ""    ;
	recovSusp     = false ;
	canBackup     = false ;
	bThread       = nullptr ;
	edrec_done    = false ;
	spfedit       = "SPFEDIT" ;
	recvStatus    = RECV_STOPPED ;

	langSpecials[ "ALL"     ] = "" ;
	langSpecials[ "ASM"     ] = "+-*/=<>&|:,#" ;
	langSpecials[ "BASH"    ] = "=<>+-*[]/&|:,;%\\" ;
	langSpecials[ "COBOL"   ] = "." ;
	langSpecials[ "CPP"     ] = "+-*/=<>&,|:!;%#[]\\" ;
	langSpecials[ "DEFAULT" ] = "" ;
	langSpecials[ "JCL"     ] = ",|<>&=" ;
	langSpecials[ "OTHER"   ] = "+-*/=<>&|:" ;
	langSpecials[ "PANEL"   ] = "=,&.<>!*|" ;
	langSpecials[ "REXX"    ] = "=<>+-*[]/&|:,;%\\" ;
	langSpecials[ "RUST"    ] = "+-*/%^!&|<>=@_.;:,#$?~[]" ;
	langSpecials[ "SKEL"    ] = "&?!<|>" ;
	langSpecials[ "TOML"    ] = ".=" ;

	vdefine( "ZCMD ZVERB ZCURFLD ZPFKEY", &zcmd, &zverb, &zcurfld, &zpfkey ) ;
	vdefine( "ZAREA ZSHADOW ZFILE ZVMODE", &zarea, &zshadow, &zfile, &zvmode ) ;
	vdefine( "ZSCROLLN ZAREAW ZOLEN", &zscrolln, &zareaw, &zolen ) ;
	vdefine( "ZAREAD ZLVLINE ZCURPOS ZSCREEND ZSCREENW", &zaread, &zlvline, &zcurpos, &zscreend, &zscreenw ) ;
	vdefine( "ZSCROLLA ZCOL1 ZCOL2", &zscrolla, &zcol1, &zcol2 ) ;
	vdefine( "ZAPPLID ZEDPROF ZEDPROFT ZHOME", &zapplid, &zedprof, &zedproft, &zhome ) ;
	vdefine( "ZUPROF TYPE STR STR2 OCC LINES", &zuprof, &type, &str, &str2, &occ, &lines ) ;
	vdefine( "ZEDOCLR ZEDFCLR ZEDFHLT", &zedoclr, &zedfclr, &zedfhlt ) ;
	vdefine( "ZEDPCLR ZEDPHLT ZEDLCTAB FSCROLL", &zedpclr, &zedphlt, &zedlctab, &fscroll ) ;
	vdefine( "ZEDCLRD ZEDCLRC ZEDCLRK ZEDCLRQ ZEDCLRV ZEDCLRS", &zedclrd, &zedclrc, &zedclrk, &zedclrq, &zedclrv, &zedclrs ) ;
	vdefine( "ZEDHID ZEDHIC ZEDHIK ZEDHIQ ZEDHIV ZEDHIS", &zedhid, &zedhic, &zedhik, &zedhiq, &zedhiv, &zedhis ) ;
}


void pedit01::application()
{
	TRACE_FUNCTION() ;

	string listid ;

	edit_parms* e_parms = static_cast<edit_parms*>( get_options() ) ;

	if ( !e_parms ) { return ; }

	if ( ( e_parms->edit_confirm  != "YES" &&
	       e_parms->edit_confirm  != "NO"  &&
	       e_parms->edit_confirm  != ""    ) ||
	     ( e_parms->edit_chgwarn  != "YES" &&
	       e_parms->edit_chgwarn  != "NO"  &&
	       e_parms->edit_chgwarn  != ""    ) ||
	     ( e_parms->edit_preserve != "PRESERVE" &&
	       e_parms->edit_preserve != ""    ) )
	{
		llog( "E", "Invalid parameter passed to edit..." << endl ) ;
		llog( "E", "CONFIRM : " << e_parms->edit_confirm << endl ) ;
		llog( "E", "CHGWARN : " << e_parms->edit_chgwarn << endl ) ;
		llog( "E", "PRESERVE: " << e_parms->edit_preserve << endl ) ;
		uabend( "PEDT015" ) ;
	}

	zfile    = e_parms->edit_file  ;
	panel    = ( e_parms->edit_panel == "" ) ? "PEDIT012" : e_parms->edit_panel ;
	edimac   = e_parms->edit_macro ;
	edprof   = e_parms->edit_profile ;
	reclen   = e_parms->edit_reclen ;
	edlmac   = e_parms->edit_lcmds ;
	zedbfile = e_parms->edit_bfile ;
	zedvmode = e_parms->edit_viewmode ;
	zedcwarn = ( e_parms->edit_chgwarn == "YES" || e_parms->edit_chgwarn == "" ) ;
	edmparm  = e_parms->edit_parm ;
	dataid1  = e_parms->edit_dataid ;
	editRecovery  = e_parms->edit_recovery ;
	optConfCancel = ( e_parms->edit_confirm  == "YES" || e_parms->edit_confirm == "" ) ;
	optPreserve   = ( e_parms->edit_preserve == "PRESERVE" ) ;

	if ( zfile == "" )
	{
		llog( "E", "No file name passed to edit" << endl ) ;
		uabend( "PEDT015" ) ;
	}

	try
	{
		if ( exists( zfile ) && is_directory( zfile ) )
		{
			vdefine( "LID", &listid ) ;
			lmdinit( "LID", zfile ) ;
			lmddisp( listid ) ;
			lmdfree( listid ) ;
			vdelete( "LID" ) ;
			return ;
		}
	}
	catch (...)
	{
		llog( "E", "Error accessing entry " << zfile << endl ) ;
		uabend( "PEDT015" ) ;
	}

	set_dialogue_var( "ZEDXPROF", edprof ) ;

	initialise() ;

	Edit() ;

	ZRC  = XRC ;
	ZRSN = XRSN ;
}


void pedit01::initialise()
{
	TRACE_FUNCTION() ;

	uint i ;

	string t ;

	vector<ipos>::iterator it ;

	control( "ABENDRTN", static_cast<void (pApplication::*)()>(&pedit01::cleanup_custom) ) ;

	pquery( panel, "ZAREA", "", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )
	{
		uabend( "PEDT015W", "PQUERY" ) ;
	}

	zdataw = zareaw - CLINESZ ;
	zasize = zareaw * zaread  ;

	sdg.assign( zdataw, N_GREEN )  ;
	sdy.assign( zdataw, N_YELLOW ) ;
	sdw.assign( zdataw, N_WHITE )  ;
	sdr.assign( zdataw, N_RED )    ;
	sdb.assign( zdataw, N_BLUE )   ;

	slg.assign( CLINESZ, ( zedvmode ) ? N_BLUE : N_GREEN ) ;
	slr.assign( CLINESZ, N_RED )   ;
	slb.assign( CLINESZ, N_BLUE )  ;
	slw.assign( CLINESZ, N_WHITE ) ;

	slgu.assign( CLINESZ, ( zedvmode ) ? U_BLUE : U_GREEN ) ;
	slru.assign( CLINESZ, U_RED ) ;

	div.assign( zareaw-1, '-' ) ;

	vget( "ZEDPROF ZEDPROFT ZUPROF  ZEDOCLR", PROFILE ) ;
	vget( "ZEDFCLR ZEDFHLT  ZEDPCLR ZEDPHLT", PROFILE ) ;
	vget( "ZEDLCTAB", PROFILE ) ;
	vget( "ZAPPLID ZHOME ZSCREEND ZSCREENW", SHARED ) ;

	if ( zedoclr == "" ) { zedoclr  = "YELLOW"  ; }
	if ( zedfclr == "" ) { zedfclr  = "WHITE"   ; }
	if ( zedfhlt == "" ) { zedfhlt  = "REVERSE" ; }
	if ( zedpclr == "" ) { zedpclr  = "WHITE"   ; }
	if ( edlmac  != "" ) { zedlctab = edlmac    ; }
	if ( zedprof == "" ) { zedprof  = "DEFAULT" ; }

	clipBoard = "DEFAULT"  ;
	clipTable = "EDITCLIP" ;

	get_shared_var( "ZUSER", zuser ) ;
	get_shared_var( "ZSCREEN", zscreen ) ;
	get_shared_var( "ZSCRNUM", zscrnum ) ;

	optNoConvTabs = ( get_shared_var( "ZEDTABSS" ) == "YES" ) ;

	get_profile_var( "ZETRGPOS", t ) ;
	targetLine = ( t != "" && isnumeric( t ) ) ? ds2d( t ) : 2 ;

	zvmode = ( zedvmode ) ? "VIEW" : "EDIT" ;

	s2data.reserve( zaread ) ;

	for ( uint i = 0 ; i < zaread ; ++i )
	{
		s2data.push_back( ipos( zdataw ) ) ;
	}

	for ( it = s2data.begin(), i = 0 ; it != s2data.end() ; ++it, ++i )
	{
		t = d2ds( i, 3 ) ;
		vdefine( "ZOVRV" + t, &it->ipos_ovalue ) ;
		vdefine( "ZOVRS" + t, &it->ipos_oshadow ) ;
		vdefine( "ZOVRU" + t, &it->ipos_oupdate ) ;
	}
}


void pedit01::Edit()
{
	TRACE_FUNCTION() ;

	bool lcmd_error ;

	string zedalt ;

	string cmdStack = "" ;
	string omsg     = "" ;

	string inAttrs = { DATAIN1, DATAIN2, USERMOD, DATAMOD } ;

	iline* dl ;

	XRC   = 4 ;
	XRSN  = 0 ;
	level = 0 ;

	pcmd.clear() ;
	defNames.clear() ;

	auto it = Global_efind_parms.find( ds2d( zscrnum ) ) ;
	if ( it != Global_efind_parms.end() )
	{
		fcx_parms = it->second ;
	}

	if ( edprof != "" ) { zedprof = edprof ; }

	readFile() ;
	if ( XRC > 11 )
	{
		return ;
	}
	else if ( XRC == 4 && XRSN == 4 )
	{
		vreplace( "ZSTR1", "Record length greater than 65,535 bytes found" ) ;
		setmsg( "PEDT013P" ) ;
		control( "ERRORS", "RETURN" ) ;
		browse( zfile ) ;
		control( "ERRORS", "CANCEL" ) ;
		return ;
	}

	if ( zedlctab != "" )
	{
		readLineCommandTable() ;
	}

	zedalt = zfile ;
	vget( "ZEDALT", SHARED ) ;
	if ( RC == 0 )
	{
		vcopy( "ZEDALT", zedalt, MOVE ) ;
		verase( "ZEDALT", SHARED ) ;
	}

	curfld = "ZCMD" ;
	curpos = 1 ;

	hlight.hl_clear() ;
	miBlock.clear() ;

	cursor.home() ;

	load_language_colours() ;

	run_imacro( EDSWMAC ) ;
	if ( XRC > 11 ) { return ; }

	run_imacro( get_profile_var( "ZUSERMAC" ) ) ;
	if ( XRC > 11 ) { return ; }

	run_imacro( ( edimac != "" ) ? edimac : zedpimac, ( edimac != "" ) ) ;
	if ( XRC > 11 ) { return ; }

	if ( profRecovery )
	{
		startRecoveryTask() ;
	}

	set_language_fvars( detLang ) ;

	while ( !termEdit )
	{
		if ( lnumSize > 0 && iline::file_inserts[ taskid() ] )
		{
			add_line_numbers() ;
			iline::file_inserts[ taskid() ] = false ;
		}

		if ( !edrec_done && recvStatus == RECV_RUNNING && profUndoOn && iline::get_Global_File_level( taskid() ) != saveLevel )
		{
			addEditRecovery() ;
		}

		if ( is_term_resized( zscreend+2, zscreenw ) )
		{
			term_resize() ;
		}

		if ( rebuildZAREA ) { fill_dynamic_area() ; }
		else if ( rebuildShadow ) { zshadow = cshadow ; }

		rebuildShadow = false ;

		protNonDisplayChars() ;

		vreplace( "ZEDALT", zedalt ) ;

		zcol1 = d2ds( startCol, 5 ) ;
		zcol2 = d2ds( startCol+zdataw-1, 5 ) ;

		positionCursor() ;

		if ( profNulls ) { addNulls() ; }

		if ( optFindPhrase && profFindPhrase && fcx_parms.f_fset )
		{
			hiliteFindPhrase() ;
		}

		zcmd = pcmd.condget_cmd() ;
		if ( zcmd != "" && pcmd.error() && cursor.not_fixed() )
		{
			curfld = "ZCMD" ;
			curpos = 1 ;
		}
		pcmd.reset() ;

		if ( zedvmode && zedcwarn && !ichgwarn && iline::data_Changed( taskid() ) )
		{
			ichgwarn = true ;
			pcmd.set_msg( "PEDT016P", 12 ) ;
		}

		if ( profRecovery && recovSusp )
		{
			pcmd.set_msg( "PEDT017T", 8 ) ;
		}

		zolen = get_overflow_size() ;

		if ( cmdStack != "" )
		{
			if ( pcmd.ok() )
			{
				zcmd = cmdStack ;
				omsg = pcmd.get_msg() ;
			}
			else
			{
				omsg = "" ;
			}
			cmdStack = "" ;
		}
		else
		{
			set_msg_variables() ;
			canBackup = true ;
			cond_recov.notify_all() ;
			display( panel, ( omsg == "" ) ? pcmd.get_msg() : omsg, curfld, curpos ) ;
			if ( is_defName( zverb ) )
			{
				cmdStack = zverb ;
			}
			else if ( RC == 8 )
			{
				termEdit = true ;
				cmdStack = "" ;
			}
			omsg = "" ;
			canBackup = false ;
		}

		vget( "ZSCROLLA ZSCROLLN ZPFKEY", SHARED ) ;

		pcmd.clear_msg() ;

		zchanged     = false ;
		pfkeyPressed = ( zpfkey != "PF00" ) ;
		cutActive    = false ;
		copyActive   = false ;
		moveActive   = false ;
		pasteActive  = false ;
		scrollData   = false ;
		level        = iline::get_Global_Undo_level( taskid() ) + 1 ;

		if ( zcurfld == "ZAREA" )
		{
			curfld = zcurfld ;
			curpos = zcurpos ;
			aRow   = ( (curpos-1) / zareaw + 1) ;
			aCol   = ( (curpos-1) % zareaw + 1) ;
			aADDR  = s2data.at( aRow-1 ).ipos_addr ;
			aLCMD  = ( aCol > 0 && aCol < 8 ) ;
			aDATA  = ( aCol > 8 ) ;
			aOff   = ( aDATA ) ? startCol + aCol - CLINESZ - 2 : startCol - 1 ;
		}
		if ( zcurfld != "ZAREA" || aRow > zlvline )
		{
			curfld = "ZCMD" ;
			curpos = 1 ;
			aRow   = 0 ;
			aCol   = 0 ;
			aOff   = 0 ;
			aADDR  = nullptr ;
			aLCMD  = false ;
			aDATA  = false ;
		}

		cursor.store( aADDR, aOff ) ;

		if ( profTabs && tabsChar == ' ' && aADDR && aDATA )
		{
			if ( onHardwareTab( aOff ) && inAttrs.find_first_of( zarea[ curpos - 1 ] ) != string::npos )
			{
				noAttrTabs.insert( aADDR ) ;
				cursor.fix() ;
			}
		}

		getZAREAchanges() ;

		if ( zverb == "CANCEL" )
		{
			if ( confirmCancel() ) { XRSN = 4 ; break ; }
		}

		if ( zcmd != "" && !pfkeyPressed && findword( zverb, "UP DOWN LEFT RIGHT" ) )
		{
			pcmd.set_cmd( zverb, zcmd ) ;
			pcmd.set_msg( "PEDT011", 12 ) ;
			termEdit = false ;
			continue ;
		}

		pcmd.set_cmd( zcmd, defNames, zverb ) ;
		if ( pcmd.error() || pcmd.deactive() )
		{
			termEdit = false ;
			continue ;
		}
		else if ( pcmd.cmdf_equals( "END" ) )
		{
			termEdit = true ;
		}
		else if ( pcmd.cmdf_equals( "CANCEL" ) )
		{
			XRSN     = 4 ;
			termEdit = true ;
			if ( confirmCancel() ) { break ; }
		}

		if ( zverb == "" && ( pcmd.cmdf_equals( "RFIND", "RCHANGE" ) ) )
		{
			zverb = pcmd.get_cmdf() ;
		}

		setLineLabels() ;
		if ( pcmd.error() )
		{
			termEdit     = false ;
			rebuildZAREA = false ;
			continue ;
		}

		if ( pcmd.is_macro() )
		{
			run_macro( pcmd.get_cmd1() ) ;
			if ( termEdit && pcmd.ok() )
			{
				break ;
			}
			termEdit = false ;
			continue ;
		}

		actionPrimCommand1() ;
		if ( pcmd.is_actioned() )
		{
			if ( pcmd.error() )
			{
				termEdit     = false ;
				rebuildZAREA = false ;
				continue ;
			}
			pcmd.cond_reset() ;
		}

		updateData() ;
		if ( pcmd.error() )
		{
			termEdit     = false ;
			rebuildZAREA = false ;
			continue ;
		}

		auto it2 = scrollList.find( zverb ) ;
		if ( it2 != scrollList.end() )
		{
			scrollData = true ;
		}

		processNewInserts() ;

		if ( pcmd.not_actioned() )
		{
			actionPrimCommand2() ;
			if ( pcmd.error() && pcmd.isLine_cmd() )
			{
				pcmd.cond_reset() ;
				continue ;
			}
			if ( pcmd.is_actioned() )
			{
				if ( pcmd.error() ) { termEdit = false ; continue ; }
				pcmd.cond_reset() ;
			}
		}

		actionLineCommands() ;
		if ( pcmd.error() )
		{
			termEdit = false ;
		}
		lcmd_error = pcmd.error() ;

		auto it1 = zverbList.find( zverb ) ;
		if ( it1 != zverbList.end() )
		{
			(this->*(it1->second))() ;
			if ( pcmd.msgset() ) { continue ; }
		}

		if ( pcmd.not_actioned() )
		{
			actionPrimCommand3() ;
			if ( pcmd.is_actioned() )
			{
				if ( pcmd.error() && not lcmd_error ) { termEdit = false ; continue ; }
				pcmd.cond_reset() ;
			}
		}

		if ( it2 != scrollList.end() )
		{
			(this->*(it2->second))() ;
			cmdStack = "" ;
		}

		if ( termEdit && pcmd.ok() )
		{
			if ( !termOK() ) { termEdit = false ; pcmd.setRC( 12 ) ; }
			else             { continue         ; }
		}
		else
		{
			termEdit = false ;
		}

		if ( aRow == zlvline && it2 == scrollList.end() )
		{
			dl = s2data.at( aRow - 1 ).ipos_addr ;
			if ( dl && !dl->is_bod() )
			{
				moveDownLines() ;
			}
		}

		if ( cursor.not_fixed() )
		{
			moveCursorEnter() ;
		}

		cleanupRedoStacks() ;
	}

	cleanup() ;
}


void pedit01::cleanup()
{
	//
	// Do any necessary termination processing before ending the edit session.
	//
	// 1) Stop background recovery task and delete thread.
	// 2) Remove entry from the EDRT.
	// 3) Save profile.
	// 4) Release the file enqueue.
	// 5) Erase any static data for this taskid.
	//

	TRACE_FUNCTION() ;

	int i ;

	if ( recvStatus == RECV_RUNNING )
	{
		recvStatus = RECV_STOPPING ;
		cond_recov.notify_all() ;
	}

	removeEditRecovery() ;

	saveEditProfile( zedprof ) ;

	deq( spfedit, zfile ) ;

	auto it1 = iline::setUNDO.find( taskid() ) ;
	if ( it1 != iline::setUNDO.end() )
	{
		iline::setUNDO.erase( it1 ) ;
	}

	auto it2 = iline::Redo_data.find( taskid() ) ;
	if ( it2 != iline::Redo_data.end() )
	{
		iline::Redo_data.erase( it2 ) ;
	}

	auto it3 = iline::Redo_other.find( taskid() ) ;
	if ( it3 != iline::Redo_other.end() )
	{
		iline::Redo_other.erase( it3 ) ;
	}

	auto it4 = iline::File_save.find( taskid() ) ;
	if ( it4 != iline::File_save.end() )
	{
		iline::File_save.erase( it4 ) ;
	}

	auto it5 = iline::Global_Undo.find( taskid() ) ;
	if ( it5 != iline::Global_Undo.end() )
	{
		iline::Global_Undo.erase( it5 ) ;
	}

	auto it6 = iline::Global_Redo.find( taskid() ) ;
	if ( it6 != iline::Global_Redo.end() )
	{
		iline::Global_Redo.erase( it6 ) ;
	}

	auto it7 = iline::Global_File_level.find( taskid() ) ;
	if ( it7 != iline::Global_File_level.end() )
	{
		iline::Global_File_level.erase( it7 ) ;
	}

	auto it8 = iline::Global_status.find( taskid() ) ;
	if ( it8 != iline::Global_status.end() )
	{
		iline::Global_status.erase( it8 ) ;
	}

	auto it9 = iline::Global_status_redo.find( taskid() ) ;
	if ( it9 != iline::Global_status_redo.end() )
	{
		iline::Global_status_redo.erase( it9 ) ;
	}

	auto it10 = iline::src_file.find( taskid() ) ;
	if ( it10 != iline::src_file.end() )
	{
		iline::src_file.erase( it10 ) ;
	}

	auto it11 = iline::dst_file.find( taskid() ) ;
	if ( it11 != iline::dst_file.end() )
	{
		iline::dst_file.erase( it11 ) ;
	}

	auto it12 = iline::file_changed.find( taskid() ) ;
	if ( it12 != iline::file_changed.end() )
	{
		iline::file_changed.erase( it12 ) ;
	}

	auto it13 = iline::file_inserts.find( taskid() ) ;
	if ( it13 != iline::file_inserts.end() )
	{
		iline::file_inserts.erase( it13 ) ;
	}

	auto it14 = iline::changed_icond.find( taskid() ) ;
	if ( it14 != iline::changed_icond.end() )
	{
		iline::changed_icond.erase( it14 ) ;
	}

	auto it15 = iline::changed_ilabel.find( taskid() ) ;
	if ( it15 != iline::changed_ilabel.end() )
	{
		iline::changed_ilabel.erase( it15 ) ;
	}

	auto it16 = iline::changed_xclud.find( taskid() ) ;
	if ( it16 != iline::changed_xclud.end() )
	{
		iline::changed_xclud.erase( it16 ) ;
	}

	i = 0 ;
	while ( recvStatus != RECV_STOPPED && ++i < 1000 )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
	}

	delete bThread ;
}


void pedit01::run_imacro( const string& imac,
			  bool editserv )
{
	//
	// Run the initial macro or command.
	//

	TRACE_FUNCTION() ;

	if ( imac == "" || upper( imac ) == "NONE" )
	{
		return ;
	}

	pcmd.set_cmd( imac, defNames ) ;
	if ( pcmd.error() )
	{
		XRC     = 20 ;
		XRSN    = 0  ;
		ZRESULT = pcmd.get_msg() ;
		return ;
	}

	if ( pcmd.is_macro() )
	{
		run_macro( pcmd.get_cmd(),
			   true,
			   editserv ) ;
		if ( pcmd.error() )
		{
			termEdit = false ;
		}
	}
	else
	{
		actionPrimCommand2() ;
		if ( pcmd.ok() && pcmd.not_actioned() )
		{
			actionPrimCommand3() ;
			if ( pcmd.not_actioned() )
			{
				pcmd.set_msg( "PEDM012M", pcmd.get_cmd(), 20 ) ;
			}
		}
		pcmd.cond_reset() ;
	}
}


void pedit01::addEditRecovery()
{
	//
	// Add a record to the Edit Recovery Table.
	// File being edited is zfile and saved to bfile.
	//
	// zzdtfile - file being edited (zfile).
	// zzdbfile - backup destination (bfile - set during error recovery).
	// zzdopts  - byte 0 - confirm cancel.
	//          - byte 1 - preserve trailing spaces.
	//

	TRACE_FUNCTION() ;

	string tabName = zapplid + "EDRT" ;
	string vlist   = "ZEDSTAT ZEDTFILE ZEDBFILE ZEDMODE ZEDOPTS ZEDRECFM ZEDLRECL" ;

	string zzdstat  ;
	string zzdtfile ;
	string zzdbfile ;
	string zzdmode  ;
	string zzdopts  ;
	string zzdrecfm ;
	string zzdlrecl ;

	edrec( "INIT" ) ;

	vdefine( vlist, &zzdstat, &zzdtfile, &zzdbfile, &zzdmode, &zzdopts, &zzdrecfm, &zzdlrecl ) ;

	tbopen( tabName, WRITE, zuprof ) ;
	if ( RC > 0 )
	{
		recovSusp = true ;
		llog( "E", "TBOPEN failed. RC="<< RC <<" RECOVERY suspended."<< endl ) ;
		vdelete( vlist ) ;
		return ;
	}

	while ( true )
	{
		tbskip( tabName ) ;
		if ( RC > 0 )
		{
			recovSusp = true ;
			llog( "E", "TBSKIP failed. RC="<< RC <<" RECOVERY suspended."<< endl ) ;
			vdelete( vlist ) ;
			return ;
		}
		if ( zzdstat == "0" )
		{
			vget( "ZEDUSER", SHARED ) ;
			zzdstat  = "1"   ;
			zzdtfile = zfile ;
			zzdbfile = bfile ;
			zzdmode  = ( zedvmode ) ? "V" : "E" ;
			zzdopts  = ( optConfCancel ) ? "1" : "0" ;
			zzdopts  = zzdopts + ( ( optPreserve ) ? "1" : "0" ) ;
			zzdrecfm = ( reclen == 0 ) ? "U" : "F" ;
			zzdlrecl = d2ds( reclen ) ;
			tbput( tabName ) ;
			enq( spfedit, bfile ) ;
			break ;
		}
	}

	tbclose( tabName, "", zuprof ) ;

	vdelete( vlist ) ;

	edrec_done = true ;
}


void pedit01::removeEditRecovery()
{
	//
	// Remove a record from the Edit Recovery Table if Edit shuts down normally or
	// the file has been saved.
	//
	// File being edited is zfile and saved to bfile.
	//
	// zzdtfile - file being edited (zfile).
	// zzdbfile - backup destination (bfile).
	//

	TRACE_FUNCTION() ;

	string tabName = zapplid + "EDRT" ;
	string vlist   = "ZEDSTAT ZEDTFILE ZEDBFILE" ;

	string zzdstat  ;
	string zzdtfile ;
	string zzdbfile ;

	vdefine( vlist, &zzdstat, &zzdtfile, &zzdbfile ) ;

	tbopen( tabName, WRITE, zuprof ) ;
	if ( RC > 0 )
	{
		llog( "E", "TBOPEN failed.  RC="<<RC<<endl ) ;
		vdelete( vlist ) ;
		return ;
	}

	tbskip( tabName ) ;
	while ( RC == 0 )
	{
		if ( zzdstat == "1" && zzdtfile == zfile && zzdbfile == bfile )
		{
			tbvclear( tabName ) ;
			zzdstat = "0" ;
			tbput( tabName ) ;
			deq( spfedit, bfile ) ;
			break ;
		}
		tbskip( tabName ) ;
	}

	tbclose( tabName, "", zuprof ) ;

	vdelete( vlist ) ;

	remove( bfile ) ;

	edrec_done = false ;
}


bool pedit01::termOK()
{
	TRACE_FUNCTION() ;

	if ( !fileChanged ) { return true ; }

	if ( zedvmode )
	{
		return viewWarning2() ;
	}

	if ( !profAutoSave && pcmd.cmdf_not_equals( "SAVE" ) )
	{
		if ( profSaveP )
		{
			pcmd.set_msg( "PEDT011O", 12 ) ;
			return false ;
		}
		else if ( optConfCancel )
		{
			termEdit = true ;
			return confirmCancel() ;
		}
		return true ;
	}

	if ( saveFile() )
	{
		ZRESULT = "PEDT011P" ;
		return true ;
	}

	return false ;
}


bool pedit01::confirmCancel()
{
	TRACE_FUNCTION() ;

	if ( !zchanged && ( !optConfCancel || !fileChanged || macroRunning ) ) { return termEdit ; }

	addpop( "", 2, 4 ) ;

	display( "PEDIT016" ) ;
	if ( RC == 8 ) { termEdit = false ; }

	rempop() ;

	return termEdit ;
}


bool pedit01::confirmMove( const string& str )
{
	TRACE_FUNCTION() ;

	bool retval = false ;

	if ( ( !optConfCancel || macroRunning ) ) { return true ; }

	addpop( "", 2, 4 ) ;

	set_dialogue_var( "ZSTR1", str ) ;

	display( "PEDIT01G" ) ;
	if ( RC == 0 )
	{
		retval = true ;
	}
	else
	{
		pcmd.set_msg( "PEDT014O", "MOVE request has not been confirmed.", 4 ) ;
	}

	rempop() ;

	return retval ;
}


bool pedit01::confirmReplace( const string& str )
{
	TRACE_FUNCTION() ;

	bool retval = false ;

	if ( ( !optConfCancel || macroRunning ) ) { return true ; }

	addpop( "", 2, 4 ) ;

	set_dialogue_var( "ZSTR1", str ) ;

	display( "PEDIT01H" ) ;
	if ( RC == 0 )
	{
		retval = true ;
	}
	else
	{
		pcmd.set_msg( "PEDT014O", "REPLACE request has not been confirmed.", 4 ) ;
	}

	rempop() ;

	return retval ;
}


void pedit01::viewWarning1()
{
	TRACE_FUNCTION() ;

	addpop( "", 2, 4 ) ;

	display( "PEDIT01C", "", "ZCMD1" ) ;

	rempop() ;

	termEdit = false ;
}


bool pedit01::viewWarning2()
{
	TRACE_FUNCTION() ;

	bool retval = false ;

	if ( macroRunning ) { return true ; }

	cursor.home() ;

	addpop( "", 2, 4 ) ;

	display( "PEDIT01D", "", "ZCMD1" ) ;
	if ( RC == 0 ) { retval = true ; }

	rempop() ;

	return retval ;
}


void pedit01::set_msg_variables()
{
	//
	// Set message dialogue variables in the Editor function pool.
	//

	TRACE_FUNCTION() ;

	vreplace( "ZMVAL1", pcmd.get_msg_val1() ) ;
	vreplace( "ZMVAL2", pcmd.get_msg_val2() ) ;
	vreplace( "ZMVAL3", pcmd.get_msg_val3() ) ;
}


void pedit01::readFile()
{
	//
	// Read file into the data container.
	//
	// If automatic profile selection used, read the first 100 lines to
	// determine the language then clear the container.
	//
	// Automatic LRECL/RECFM will be used (when RECLEN = 0) if one of the following:
	// 1)  All records have the same length, be at least 20 bytes, be a multiple of 10,
	//     not contain tabs, and have at least 5 records in the file.  If an edit profile
	//     does not exist for this combination, one will be created.
	// 2)  If there is already an edit profile for this combination, the
	//     file must contain at least 2 records, all be the same length and have no tabs.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int p ;

	int ver1 = 0 ;
	int ver2 = 0 ;

	int lnum1 = -1 ;
	int lnum2 ;
	int lnum3 = -1 ;
	int lnum4 ;

	string t ;
	string lnum ;
	string infile ;

	size_t pos ;
	size_t maxrecl = 0 ;

	bool found ;

	bool invChars   = false ;
	bool lowrOnRead = false ;

	bool numSTD1    = ( reclen == 0 || reclen > 9 ) ;
	bool numSTD2    = ( reclen == 0 || reclen > 9 ) ;
	bool numCBL     = ( reclen == 0 || reclen > 9 ) ;
	bool gmaxrecl   = ( reclen > 0 ) ;
	bool checklrcl  = ( reclen == 0 ) ;

	bool empty      = true ;
	bool caret      = true ;

	size_t lrecl = 0 ;

	string inLine ;

	const string tod = " Top of Data **********" ;
	const string bod = " Bottom of Data **********" ;

	boost::system::error_code ec ;

	RC = 0 ;

	loadEditProfile( zedprof ) ;

	if ( editRecovery )
	{
		infile      = zedbfile ;
		fileChanged = true ;
	}
	else
	{
		infile      = zfile ;
		fileChanged = false ;
	}

	if ( !zedvmode )
	{
		enq( spfedit, zfile ) ;
		if ( RC == 8 )
		{
			set_dialogue_var( "ZMVAL1", zfile ) ;
			ZRESULT = "PEDT014A" ;
			XRC     = 14 ;
			XRSN    = 0 ;
			return ;
		}
		else if ( RC > 0 )
		{
			return ;
		}
	}

	topLine    = nullptr ;
	startCol   = 1 ;
	tabsOnRead = false ;
	lrecl      = 0 ;

	fscroll = ( infile.size() < ( zareaw - 32 ) ) ? "NO" : "YES" ;

	try
	{
		found = exists( infile ) ;
	}
	catch ( const filesystem_error& ex )
	{
		set_dialogue_var( "ZVAL1", ex.what() ) ;
		ZRESULT = "PSYS012C" ;
		XRC     = 20 ;
		XRSN    = 4 ;
		return ;
	}

	if ( !found )
	{
		data.push_back( new iline( taskid(), LN_TOD ), centre( tod, zareaw, '*' ) ) ;
		for ( i = 0 ; i < zaread-2 ; ++i )
		{
			data.push_back( new iline( taskid(), LN_ISRT ) ) ;
		}
		data.push_back( new iline( taskid(), LN_BOD ), centre( bod, zareaw, '*' ) ) ;
		iline::init_Globals( taskid(), zedvmode ) ;
		if ( profNum )
		{
			set_num_parameters() ;
		}
		topLine   = data.top() ;
		saveLevel = 0 ;
		store_status( SS_ALL ) ;
		set_zhivars() ;
		return ;
	}

	std::ifstream fin ;
	try
	{
		fin.open( infile.c_str() ) ;
	}
	catch ( const filesystem_error& ex )
	{
		set_dialogue_var( "ZVAL1", ex.what() ) ;
		pcmd.set_msg( "PSYS012C", 12 ) ;
		ZRESULT = "PSYS012C" ;
		XRC     = 20 ;
		XRSN    = 4 ;
		return ;
	}

	if ( !fin.is_open() )
	{
		ZRESULT = "PSYS011E" ;
		XRC     = 20 ;
		XRSN    = 4 ;
		return ;
	}

	if ( !zedvmode && access( infile.c_str(), W_OK ) != 0 )
	{
		pcmd.set_msg( "PEDT016Z", 12 ) ;
	}

	if ( zedproft == "Y" && edprof == "" )
	{
		i = 0 ;
		while ( getline( fin, inLine ) && ( ++i < 100 ) )
		{
			if ( inLine.size() > MAXLEN )
			{
				fin.close() ;
				XRC     = 4 ;
				XRSN    = 4 ;
				return ;
			}
			data.push_back( new iline( taskid(), LN_FILE ), ( optNoConvTabs ) ? inLine : convertiTabs( inLine ) ) ;
		}
		t = determineLang() ;
		explProfile = false ;
		data.clear() ;
		if ( t != "DEFAULT" )
		{
			zedprof = t ;
		}
		loadEditProfile( zedprof ) ;
		fin.close() ;
		fin.open( infile.c_str() ) ;
	}

	data.push_back( new iline( taskid(), LN_TOD ), centre( tod, zareaw, '*' ) ) ;

	topLine = data.top() ;

	while ( getline( fin, inLine ) )
	{
		empty = false ;
		if ( inLine.size() > MAXLEN )
		{
			fin.close() ;
			XRC     = 4 ;
			XRSN    = 4 ;
			return ;
		}
		pos = inLine.find( '\t' ) ;
		while ( pos != string::npos )
		{
			tabsOnRead = true ;
			checklrcl  = false ;
			lrecl      = 0 ;
			if ( optNoConvTabs ) { break ; }
			j   = profXTabz - ( pos % profXTabz ) ;
			inLine.replace( pos, 1,  j, ' ' )  ;
			pos = inLine.find( '\t', pos + 1 ) ;
		}
		if ( caret && ( inLine.size() == 0 || inLine.back() != 0x0D ) )
		{
			caret = false ;
		}
		if ( gmaxrecl )
		{
			maxrecl = max( maxrecl, inLine.size() ) ;
		}
		if ( checklrcl && !caret )
		{
			if ( data.size() == 1 )
			{
				lrecl = inLine.size() ;
			}
			else if ( lrecl != inLine.size() )
			{
				checklrcl = false ;
				lrecl     = 0 ;
			}
		}
		if ( numSTD1 )
		{
			if ( inLine.size() < 10 )
			{
				numSTD1 = false ;
			}
			else
			{
				lnum = inLine.substr( 0, 8 ) ;
				if ( isnumeric( lnum ) )
				{
					lnum2 = ds2d( lnum.substr( 0, 6 ) ) ;
					if ( lnum1 != -1 && lnum1 >= lnum2 )
					{
						numSTD1 = false ;
					}
					else
					{
						lnum1 = lnum2 ;
						ver1  = max( ver1, ds2d( lnum.substr( 6, 2 ) ) ) ;
					}
				}
				else
				{
					numSTD1 = false ;
				}
			}
		}
		if ( reclen > 0 )
		{
			inLine.resize( reclen, ' ' ) ;
		}
		if ( numSTD2 )
		{
			if ( inLine.size() < 10 || ( reclen == 0 && lrecl < 10 ) )
			{
				numSTD2 = false ;
			}
			else if ( reclen > 9 || lrecl > 9 )
			{
				lnum = inLine.substr( inLine.size() - 8, 8 ) ;
				if ( isnumeric( lnum ) )
				{
					lnum4 = ds2d( lnum.substr( 0, 6 ) ) ;
					if ( lnum3 != -1 && lnum3 >= lnum4 )
					{
						numSTD2 = false ;
					}
					else
					{
						lnum3 = lnum4 ;
						ver2  = max( ver2, ds2d( lnum.substr( 6, 2 ) ) ) ;
					}
				}
				else
				{
					numSTD2 = false ;
				}
			}
		}
		if ( !numSTD1 && numCBL )
		{
			if ( inLine.size() < 6 )
			{
				numCBL = false ;
			}
			else
			{
				lnum = inLine.substr( 0, 6 ) ;
				if ( isnumeric( lnum ) )
				{
					lnum2 = ds2d( lnum ) ;
					if ( lnum1 != -1 && lnum1 >= lnum2 )
					{
						numCBL = false ;
					}
					lnum1 = lnum2 ;
				}
				else
				{
					numCBL = false ;
				}
			}
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
		data.push_back( new iline( taskid(), LN_FILE ), inLine ) ;
	}

	if ( gmaxrecl && maxrecl > reclen )
	{
		set_dialogue_var( "ZVAL1", d2ds( reclen ) ) ;
		set_dialogue_var( "ZVAL2", d2ds( maxrecl ) ) ;
		ZRESULT = "PEDT018A" ;
		XRC     = 20 ;
		XRSN    = 8 ;
		return ;
	}

	if ( lrecl > 0 )
	{
		if ( data.size() > 5 && lrecl > 19 && ( lrecl % 10 ) == 0 )
		{
			reclen = lrecl ;
			loadEditProfile( zedprof ) ;
		}
		else if ( data.size() > 2 && existsEditProfile( zedprof, "F", d2ds( lrecl ) ) )
		{
			reclen = lrecl ;
			loadEditProfile( zedprof ) ;
		}
	}


	if ( caret && !empty )
	{
		pcmd.set_msg( "PEDT017A", 4 ) ;
		invChars = false ;
		for ( auto ln : data )
		{
			ln.remove_back() ;
			if ( !invChars )
			{
				if ( ln.has_invChars() )
				{
					invChars = true ;
				}
			}
		}
	}

	if ( tabsOnRead )
	{
		pcmd.set_msg_cond( ( optNoConvTabs ) ? "PEDT014N" : "PEDT014W", 4 ) ;
	}

	if ( numSTD1 ) { numCBL = false ; }

	if ( data.size() == 1 )
	{
		for ( i = 0 ; i < zaread-2 ; ++i )
		{
			data.push_back( new iline( taskid(), LN_ISRT ) ) ;
		}
		data.push_back( new iline( taskid(), LN_BOD ), centre( bod, zareaw, '*' ) ) ;
		iline::init_Globals( taskid(), zedvmode ) ;
		if ( profNum )
		{
			set_num_parameters() ;
		}
		saveLevel = 0 ;
		fin.close()   ;
		store_status( SS_ALL ) ;
		set_zhivars() ;
		return ;
	}
	fin.close() ;

	data.push_back( new iline( taskid(), LN_BOD ), centre( bod, zareaw, '*' ) ) ;

	if ( profNum )
	{
		if ( reclen > 0 && reclen < 10 )
		{
			profNum    = false ;
			profNumCBL = false ;
			profNumSTD = false ;
			numSTD1    = false ;
			numSTD2    = false ;
			numCBL     = false ;
		}
	}

	issue_cautions( tabsOnRead, lowrOnRead, invChars, numCBL, numSTD1, numSTD2 ) ;

	if ( profNum )
	{
		set_num_parameters() ;
	}

	if ( lnumSize1 > 0 )
	{
		if ( !profNumDisp )
		{
			startCol = lnumSize1 + 1 ;
		}
	}

	set_default_bounds() ;

	if ( profNumSTD )
	{
		lnummod = ( numSTD1 ) ?
			  ( ver1 < 98 ) ? d2ds( ++ver1, 2 ) : "99" :
			  ( ver2 < 98 ) ? d2ds( ++ver2, 2 ) : "99" ;
	}

	iline::clear_Global_Undo( taskid() ) ;
	iline::clear_Global_Redo( taskid() ) ;
	iline::clear_Global_File_level( taskid() ) ;
	iline::reset_Redo_data( taskid() ) ;

	if ( profRecovery && !editRecovery )
	{
		p = zfile.find_last_of( '/' ) + 1 ;
		iline::init_Globals( taskid(),
				     zedvmode,
				     zfile,
				     backupLoc + zfile.substr( p ) +
				     "-" + get_shared_var( "ZJ4DATE" ) +
				     "-" + get_shared_var( "ZTIMEL" ) ) ;
	}
	else
	{
		iline::init_Globals( taskid(),
				     zedvmode ) ;
	}

	saveLevel = 0 ;

	detLang = ( profLang != "AUTO" ) ? profLang : determineLang() ;

	store_status( SS_ALL ) ;
	set_zhivars() ;
}


bool pedit01::saveFile()
{
	//
	// Save file to disk.
	//
	// Remove UNDO/REDO data if not SETUNDO KEEP.
	// Remove entry in the EDRT for edit recovery.
	//

	TRACE_FUNCTION() ;

	uint i ;

	string* pt ;

	string t1 ;
	string f = zfile ;
	string spaces = string( profXTabz, ' ' ) ;

	std::ofstream fout( f.c_str() ) ;

	if ( !fout.is_open() )
	{
		pcmd.set_msg( "PEDT011Q", 12 ) ;
		return false ;
	}

	if ( profNum && profAutoNum )
	{
		renum_data() ;
	}

	for ( auto& ln : data )
	{
		if ( ln.not_valid_file() ) { continue ; }
		pt = ln.get_idata_ptr() ;
		if ( !optPreserve && reclen == 0 ) { ln.set_idata_trim() ; }
		if ( profXTabs )
		{
			t1 = "" ;
			for ( i = 0 ; i < pt->size() ; ++i )
			{
				if ( ( i % profXTabz == 0 )         &&
				     pt->size() > ( i+profXTabz-1 ) &&
				     pt->compare( i, profXTabz, spaces ) == 0 )
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
			if ( reclen > 0 ) { t1.resize( reclen, ' ' ) ; }
			fout << t1 <<endl ;
		}
		else
		{
			if ( reclen > 0 ) { pt->resize( reclen, ' ' ) ; }
			fout << *pt <<endl ;
		}
	}

	fout.close() ;
	if ( fout.fail() )
	{
		pcmd.set_msg( "PEDT011Q", 12 ) ;
		return false ;
	}

	if ( profUndoOn )
	{
		if ( !profUndoKeep )
		{
			removeRecoveryData() ;
		}
		saveLevel = iline::get_Global_File_level( taskid() ) ;
	}

	if ( profRecovery )
	{
		removeEditRecovery() ;
	}

	fileChanged = false ;

	pcmd.set_msg( "PEDT011P", 4, 0 ) ;
	XRC = 0 ;

	return true ;
}


void pedit01::issue_cautions( bool tabsOnRead,
			      bool lowrOnRead,
			      bool invChars,
			      bool numCBL,
			      bool numSTD1,
			      bool numSTD2 )
{
	//
	// Issue cautions for attributes automatically changed from the profile values.
	//

	TRACE_FUNCTION() ;

	caution caut ;

	vector<caution> Cautions ;

	vector<string> Msgs ;

	if ( tabsOnRead )
	{
		if ( reclen > 0 )
		{
			caut.type    = CA_XTABS_ON ;
			caut.message = "-CAUTION- Profile changed to XTABS OFF (from XTABS ON) because the"  ;
			Cautions.push_back( caut ) ;
			caut.message = "          data contains tabs but RECLEN specified" ;
			Cautions.push_back( caut ) ;
			profXTabs = false ;
		}
		else if ( !optNoConvTabs && !profXTabs )
		{
			caut.type    = CA_XTABS_OFF ;
			caut.message = "-CAUTION- Profile changed to XTABS ON (from XTABS OFF) because the"  ;
			Cautions.push_back( caut ) ;
			caut.message = "          data contains tabs" ;
			Cautions.push_back( caut ) ;
			profXTabs = true ;
		}
		else if ( optNoConvTabs && profXTabs )
		{
			caut.type    = CA_XTABS_ON ;
			caut.message = "-CAUTION- Profile changed to XTABS OFF (from XTABS ON) because the"  ;
			Cautions.push_back( caut ) ;
			caut.message = "          data contains tabs but option is not to convert to spaces"  ;
			Cautions.push_back( caut ) ;
			profXTabs = false ;
		}
	}
	else
	{
		if ( profXTabs )
		{
			caut.type    = CA_XTABS_ON ;
			caut.message = "-CAUTION- Profile changed to XTABS OFF (from XTABS ON) because the"  ;
			Cautions.push_back( caut ) ;
			caut.message = "          data does not contain tabs" ;
			Cautions.push_back( caut ) ;
			profXTabs = false ;
		}
	}

	if ( invChars )
	{
		Msgs.push_back( "-CAUTION- Data contains invalid (non-display) characters.  Use command" )  ;
		Msgs.push_back( "          ===> FIND P'.' to position cursor to these" ) ;
	}

	if ( lowrOnRead )
	{
		if ( profCaps )
		{
			caut.type    = CA_CAPS_ON ;
			caut.message = "-CAUTION- Profile changed to CAPS OFF (from CAPS ON) because the" ;
			Cautions.push_back( caut ) ;
			caut.message = "          data contains lower case characters" ;
			Cautions.push_back( caut ) ;
			profCaps = false ;
		}
	}
	else
	{
		if ( !profCaps )
		{
			caut.type    = CA_CAPS_OFF ;
			caut.message = "-CAUTION- Profile changed to CAPS ON (from CAPS OFF) because the" ;
			Cautions.push_back( caut ) ;
			caut.message = "          data does not contain lower case characters" ;
			Cautions.push_back( caut ) ;
			profCaps = true ;
		}
	}

	if ( !profNum )
	{
		if ( numCBL && numSTD2 )
		{
			caut.type    = CA_NUM_OFF ;
			caut.message = "-CAUTION- Profile changed to NUMBER ON COBOL STD (from NUMBER OFF) because the" ;
			Cautions.push_back( caut ) ;
			caut.message = "          data has valid line COBOL and standard numbers" ;
			Cautions.push_back( caut ) ;
		}
		else if ( numSTD1 || numSTD2 )
		{
			caut.type    = CA_NUM_OFF ;
			caut.message = "-CAUTION- Profile changed to NUMBER ON STD (from NUMBER OFF) because the" ;
			Cautions.push_back( caut ) ;
			caut.message = "          data has valid line standard numbers" ;
			Cautions.push_back( caut ) ;
		}
		else if ( numCBL )
		{
			caut.type    = CA_NUM_OFF ;
			caut.message = "-CAUTION- Profile changed to NUMBER ON COBOL (from NUMBER OFF) because the"  ;
			Cautions.push_back( caut ) ;
			caut.message = "          data has valid COBOL numbers" ;
			Cautions.push_back( caut ) ;
		}
	}
	else
	{
		if ( profNumCBL && numCBL && ( numSTD1 || numSTD2 ) )
		{
			caut.type    = CA_NUM_CBL_ON ;
			caut.message = "-CAUTION- Profile changed to NUMBER ON STD COBOL (from NUMBER ON COBOL) because the" ;
			Cautions.push_back( caut ) ;
			caut.message = "          data has valid standard and COBOL numbers" ;
			Cautions.push_back( caut ) ;
		}
		else if ( profNumCBL && ( numSTD1 || numSTD2 ) )
		{
			caut.type    = CA_NUM_CBL_ON ;
			caut.message = "-CAUTION- Profile changed to NUMBER ON STD (from NUMBER ON COBOL) because the" ;
			Cautions.push_back( caut ) ;
			caut.message = "          data has valid standard numbers" ;
			Cautions.push_back( caut ) ;
		}
		else if ( profNumSTD && numCBL && reclen == 0 )
		{
			caut.type    = CA_NUM_STD_ON ;
			caut.message = "-CAUTION- Profile changed to NUMBER ON COBOL (from NUMBER ON STD) because the" ;
			Cautions.push_back( caut ) ;
			caut.message = "          data has valid COBOL line numbers" ;
			Cautions.push_back( caut ) ;
		}
		else if ( !numSTD1 && !numSTD2 && !numCBL  )
		{
			caut.type    = CA_NUM_ON ;
			caut.message = "-CAUTION- Profile changed to NUMBER OFF (from NUMBER ON) because the" ;
			Cautions.push_back( caut ) ;
			caut.message = "          data does not have valid standard or COBOL numbers" ;
			Cautions.push_back( caut ) ;
		}
	}

	profNum    = ( numSTD1 || numSTD2 || numCBL ) ;
	profNumSTD = ( numSTD1 || numSTD2 ) ;
	profNumCBL = ( numCBL ) ;

	if ( !profUndoOn )
	{
		caut.type    = CA_SETU_OFF ;
		caut.message = "-Warning- UNDO/REDO commands are not available until you change" ;
		Cautions.push_back( caut ) ;
		caut.message = "          your edit profile using the command SETUNDO ON" ;
		Cautions.push_back( caut ) ;
	}

	addSpecial( LN_MSG, topLine, Msgs ) ;
	addSpecial( topLine, Cautions ) ;
}


void pedit01::fill_dynamic_area()
{
	//
	// Fill the dynamic area, ZAREA, from the data container.
	//

	TRACE_FUNCTION() ;

	int i  ;
	int j  ;
	int ln ;
	int sl ;
	int elines ;
	int fl = -1 ;

	uint l  ;

	int dataw = get_datawidth() ;

	Data::Iterator it = nullptr ;

	ipos ip( zdataw ) ;

	ipos* pip ;

	string t1 ;
	string t2 ;
	string t3 ;
	string t4 ;
	string lcc ;

	string* pt1 ;

	const string din1( 1, DATAIN1 ) ;
	const string din2( 1, DATAIN2 ) ;

	const string dout1( 1, DATAOUT1 ) ;
	const string dout2( 1, DATAOUT2 ) ;

	const string spaces7( 7, ' ' ) ;

	char fillchar = ( reclen == 0 ) ? ' ' : DATAOUT1 ;

	for ( i = 0 ; i < zaread ; ++i ) { s2data[ i ].clear( zdataw ) ; }

	iline* dl ;

	zarea.reserve( zasize ) ;
	zshadow.reserve( zasize ) ;

	t1.reserve( zdataw )   ;
	t2.reserve( 2*zdataw ) ;
	t3.reserve( 2*zdataw ) ;
	t4.reserve( 2*zdataw ) ;

	if ( colsOn )
	{
		zarea   = dout1 + "=COLS> " + getColumnLine() ;
		zshadow = string( zareaw, N_WHITE ) ;
		sl = 1 ;
	}
	else
	{
		zarea   = "" ;
		zshadow = "" ;
		sl = 0 ;
	}

	for ( it = topLine ; it != data.end() && zarea.size() < zasize ; ++it )
	{
		if ( it->il_deleted ) { continue ; }
		if ( it->is_file() )
		{
			fl = ( fl == -1 ) ? getFileLine( it ) : fl + 1 ;
		}
		if ( hideExcl && it->is_excluded() )
		{
			if ( sl > 0 )
			{
				if ( s2data.at( sl - 1 ).ipos_div() )
				{
					i = ( sl - 4 ) * zareaw ;
				}
				else
				{
					i = ( sl - 1 ) * zareaw ;
				}
				if ( zshadow[ i ] == slg.front() )
				{
					zshadow.replace( i, 8, slgu ) ;
				}
				else
				{
					zshadow.replace( i, 8, slru ) ;
				}
			}
			if ( it == topLine )
			{
				it = getLastEX( it ) ;
				topLine = itr2ptr( it ) ;
			}
			else
			{
				it = getLastEX( it ) ;
			}
			fl = -1 ;
			continue ;
		}
		ip.clear( zdataw ) ;
		if ( it->is_excluded() )
		{
			elines          = getExclBlockSize( it ) ;
			ip.ipos_addr    = itr2ptr( it ) ;
			s2data.at( sl ) = ip ;
			t1 = copies( "-  ", ( zareaw - 30 )/3 - 2 ) + d2ds( elines ) + " Line(s) Not Displayed" ;
			t2 = ( it->il_lcc == "" ) ? "- - - " : left( it->il_lcc, 6 ) ;
			zarea   += din1 + t2 + dout1 + substr( t1, 1, zareaw-8 ) ;
			zshadow += slr + sdw ;
			if ( zarea.size() >= zasize ) { break ; }
			++sl ;
			if ( it == topLine )
			{
				it      = getFirstEX( it ) ;
				topLine = itr2ptr( it ) ;
			}
			it += ( getDataBlockSize( it ) - 1 ) ;
			fl = -1 ;
			continue ;
		}
		if ( it->il_lcc != "" )
		{
			lcc      = left( it->il_lcc, 6 ) ;
			zshadow += slr ;
		}
		else if ( it->has_label() )
		{
			lcc      = left( it->get_label(), 6 ) ;
			zshadow += slr ;
		}
		else if ( it->get_condition() != LS_NONE )
		{
			switch ( it->get_condition() )
			{
			case LS_CHNG:
				lcc = "==CHG>" ; zshadow += slr ;
				break ;

			case LS_ERROR:
				lcc = "==ERR>" ; zshadow += slr ;
				break ;
			}
		}
		else if ( it->il_status != LS_NONE )
		{
			switch ( it->il_status )
			{
			case LS_UNDO:
				lcc = "=UNDO>" ; zshadow += slr ;
				break ;

			case LS_REDO:
				lcc = "=REDO>" ; zshadow += slr ;
				break ;
			}
		}
		else
		{
			switch ( it->il_type )
			{
			case LN_FILE:
				lcc = ( profNum ) ? it->get_linenum_str( lnumSize1, lnumSize2, lnumS2pos ) : d2ds( fl, 6 ) ;
				zshadow += slg ;
				break ;

			case LN_NOTE:
				lcc = "=NOTE=" ; zshadow += slr ;
				break ;

			case LN_INFO:
				lcc = "======" ; zshadow += slr ;
				break ;

			case LN_MSG:
				lcc = "==MSG>" ; zshadow += slr ;
				break ;

			case LN_COL:
				lcc = "=COLS>" ; zshadow += slr ;
				break ;

			case LN_PROF:
				lcc = "=PROF>" ; zshadow += slr ;
				break ;

			case LN_TABS:
				lcc = "=TABS>" ; zshadow += slr ;
				break ;

			case LN_MASK:
				lcc = "=MASK>" ; zshadow += slr ;
				break ;

			case LN_BNDS:
				lcc = "=BNDS>" ; zshadow += slr ;
				break ;

			case LN_ISRT:
				lcc = "''''''" ; zshadow += slr ;
				break ;

			case LN_TOD:
			case LN_BOD:
				lcc = "******" ; zshadow += slr ;
				break ;
			}
		}
		if ( it->is_file() || it->is_isrt() )
		{
			t1 = it->get_idata() ;
			ln = it->get_idata_len() - startCol + 1 ;
			if ( ln < 0 )
			{
				ln = 0 ;
				s2data.at( sl ).clear_value() ;
			}
			else if ( ln > zdataw )
			{
				ln = zdataw ;
				t2 = t1.substr( startCol - 1 ) ;
				s2data.at( sl ).set_ovalue( zdataw, lnumSize2, t2, ( it->marked() ) ? N_WHITE : N_GREEN ) ;
			}
			else
			{
				s2data.at( sl ).clear_value() ;
			}
			if ( !profNulls )
			{
				t1 = substr( t1, startCol, zdataw, fillchar ) ;
			}
			else
			{
				if ( t1.size() > ( startCol - 1 ) )
				{
					t1.erase( 0, startCol-1 ) ;
					if ( !profNullA && t1.size() > 0 && t1.size() < dataw && t1.back() != ' ' )
					{
						++ln ;
					}
					t1.resize( zdataw, fillchar ) ;
				}
				else
				{
					t1 = string( zdataw, fillchar ) ;
				}
				ip.ipos_lchar = ln ;
			}
			if ( it->il_hex || profHex )
			{
				zarea += din1 + lcc + din1 + t1 ;
				if ( it->marked() ) { zshadow += sdw ; }
				else                { zshadow += sdy ; }
				ip.ipos_addr = itr2ptr( it ) ;
				s2data.at( sl ) = ip ;
				if ( zarea.size() >= zasize ) { break ; }
				if ( profVert )
				{
					t3 = cs2xs1( t1, 0, ln ) ;
					t4 = cs2xs2( t1, 0, ln ) ;
					t3.resize( dataw, '2' ) ;
					t4.resize( dataw, '0' ) ;
				}
				else
				{
					t3  = cs2xs( t1.substr( 0, ln ) ) ;
					t3 += copies( "20", ( dataw*2-t3.size() )/2 ) ;
					t4  = t3.substr( dataw ) ;
					t3.erase( dataw ) ;
				}
				if ( dataw < zdataw )
				{
					t3.resize( zdataw, DATAOUT1 ) ;
					t4.resize( zdataw, DATAOUT1 ) ;
				}
				zarea   += spaces7 + din1 + t3 ;
				zshadow += slw + sdg ;
				ip.ipos_hex = 1 ;
				s2data.at( ++sl ) = ip ;
				if ( zarea.size() >= zasize ) { break ; }
				zarea   += spaces7 + din1 + t4 ;
				zshadow += slw + sdg ;
				ip.ipos_hex = 2 ;
				s2data.at( ++sl ) = ip ;
				if ( zarea.size() >= zasize ) { break ; }
				zarea   += dout1 + div ;
				zshadow += slw   + sdw ;
				ip.ipos_hex = 3 ;
				s2data.at( ++sl ) = ip ;
				if ( zarea.size() >= zasize ) { break ; }
				++sl ;
			}
			else
			{
				zarea += din1 + lcc + din1 + t1 ;
				if ( it->marked() ) { zshadow += sdw ; }
				else                { zshadow += sdg ; }
				ip.ipos_addr    = itr2ptr( it ) ;
				s2data.at( sl ) = ip ;
				++sl ;
			}
		}
		else
		{
			switch ( it->il_type )
			{
			case LN_COL:
				zarea   += din1 + lcc + dout1 + getColumnLine() ;
				zshadow += sdw ;
				break ;

			case LN_BNDS:
				t1 = string( LeftBnd - 1, ' ' ) + "<" ;
				if ( RightBnd > 0 )
				{
					t1 += string( RightBnd - t1.size() - 1, ' ' ) + ">" ;
				}
				zarea   += din1 + lcc + din1 + substr( t1, startCol, zdataw ) ;
				zshadow += sdr ;
				break ;

			case LN_MASK:
				it->put_idata( getMaskLine1() ) ;
				zarea   += din1 + lcc + din1 + substr( it->get_idata(), startCol, zdataw ) ;
				zshadow += sdr ;
				break ;

			case LN_PROF:
				l   = zshadow.size()  ;
				j   = profLang.size() ;
				pt1 = it->get_idata_ptr() ;
				zarea   += din1 + lcc + dout1 + substr( *pt1, 1, zdataw ) ;
				zshadow += sdw ;
				if ( profHilight && profLang != "AUTO" && it->il_profln == 4 )
				{
					zshadow.replace( pt1->find( "HILITE " )+l+7, j, j, N_RED ) ;
				}
				if ( it->il_profln == 0 && explProfile )
				{
					j = zedprof.size() ;
					zshadow.replace( l+4, j, j, N_RED ) ;
				}
				break ;

			case LN_TABS:
				it->put_idata( tabsLine ) ;
				zarea   += din1 + lcc + din1 + substr( it->get_idata(), startCol, zdataw ) ;
				zshadow += sdr ;
				break ;

			case LN_TOD:
			case LN_BOD:
				zarea   += din2 + lcc + dout2 + substr( it->get_idata(), 1, zdataw ) ;
				zshadow += sdb ;
				break ;

			case LN_MSG:
				zarea   += din1 + lcc + dout1 + substr( it->get_idata(), 1, zdataw ) ;
				zshadow += sdw ;
				break ;

			case LN_NOTE:
				zarea   += din1 + lcc + dout1 + substr( it->get_idata(), 1, zdataw ) ;
				zshadow += sdb ;
				break ;

			default:
				zarea   += din1 + lcc + dout1 + substr( it->get_idata(), startCol, zdataw, fillchar ) ;
				zshadow += sdw ;
			}
			ip.ipos_addr    = itr2ptr( it ) ;
			s2data.at( sl ) = ip ;
			++sl ;
		}
	}

	zarea.resize( zasize, fillchar ) ;
	zshadow.resize( zasize, N_GREEN ) ;

	pip = &s2data.at( zaread - 1 ) ;
	if ( hideExcl && pip->ipos_addr )
	{
		dl = getNextDataLine( pip->ipos_addr ) ;
		if ( dl && dl->is_excluded() )
		{
			i = ( zaread - pip->ipos_hex - 1 ) * zareaw ;
			if ( zshadow[ i ] == slg.front() )
			{
				zshadow.replace( i, 8, slgu ) ;
			}
			else
			{
				zshadow.replace( i, 8, slru ) ;
			}
		}
	}

	if ( profHilight && !hlight.hl_abend )
	{
		fill_hilight_shadow() ;
	}

	carea   = zarea ;
	cshadow = zshadow ;
	rebuildZAREA = false ;
}


void pedit01::protNonDisplayChars()
{
	//
	// Protect non-display characters in ZAREA by replacing with datain attribute and store original in XAREA.
	// lchar is the last line position to replace.  After this we may have nulls in ZAREA due to the NULLS command.
	//
	// If hardware tabbing is in use, add datain attribute at the tab positions.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int k ;
	int l ;
	int m ;

	const char din1( DATAIN1 ) ;

	bool skip ;

	iline* dl ;

	marea = string( zasize, ' ' ) ;
	xarea = string( zasize, ' ' ) ;

	for ( i = 0 ; i < zaread ; ++i )
	{
		const ipos& tpos = s2data[ i ] ;
		if ( tpos.ipos_hex_line() ) { continue ; }
		dl = tpos.ipos_addr ;
		if ( !dl || dl->not_valid_file() )
		{
			continue ;
		}
		k = i * zareaw + 8 ;
		l = tpos.ipos_lchar ;
		skip = false ;
		for ( j = 0 ; j < l ; ++j, ++k )
		{
			if ( !isprint( zarea[ k ] ) )
			{
				marea[ k ] = '*' ;
				xarea[ k ] = zarea[ k ] ;
				zarea[ k ] = din1 ;
			}
			else if ( !skip && profTabs && tabsChar == ' ' )
			{
				if ( noAttrTabs.find( dl ) != noAttrTabs.end() )
				{
					skip = true ;
				}
				else
				{
					m = j + startCol - 1 ;
					if ( tabsLine.size() > m  && tabsLine[ m ] == '*' )
					{
						if ( profATabs || zarea[ k ] == ' ' || zarea[ k ] == 0x00 )
						{
							marea[ k ] = '*' ;
							xarea[ k ] = zarea[ k ] ;
							zarea[ k ] = din1 ;
						}
					}
				}
			}
		}
	}

	noAttrTabs.clear() ;

}


void pedit01::addNulls()
{
	//
	// Convert trailing spaces to nulls, or from the cursor position (lines above cursor for hex display).
	// Ignore protected bytes.
	//

	TRACE_FUNCTION() ;

	int i ;

	int row ;
	int col ;

	int csr   = curpos - 1 ;
	int dataw = get_datawidth() ;

	size_t lblnk ;
	size_t zbegin ;
	size_t zend ;

	iline* dl ;
	iline* pdl = nullptr ;

	ipos* pip ;

	if ( curfld == "ZAREA" )
	{
		row = ( csr / zareaw ) + 1 ;
		pip = &s2data.at( row - 1 ) ;
		if ( pip->ipos_hex_top() )
		{
			--row ;
			if ( profVert )
			{
				csr -= zareaw ;
			}
			else
			{
				col = ( csr % zareaw ) + 1 ;
				csr = ( row - 1 ) * zareaw + ( col / 2 ) + 5 ;
			}
		}
		else if ( pip->ipos_hex_bottom() )
		{
			row -= 2 ;
			col = ( csr % zareaw ) + 1 ;
			csr = ( row - 1 ) * zareaw + ( dataw / 2 ) + ( col / 2 ) + 5 ;
		}
		dl  = pip->ipos_addr ;
		pip = &s2data.at( row - 1 ) ;
		if ( dl && ( dl->is_valid_file() || dl->is_isrt() ) && dl->is_not_excluded() && !pip->ipos_div() )
		{
			zbegin = ( row - 1 ) * zareaw + 8 ;
			zend   = row * zareaw - ( zdataw - dataw ) - 1 ;
			lblnk  = zend + 1 ;
			for ( uint m = zend ; m >= zbegin ; --m )
			{
				if ( marea[ m ] == '*' ) { continue ; }
				if ( zarea[ m ] != ' ' ) { break    ; }
				lblnk = m ;
			}
			if ( !profNullA )
			{
				++lblnk ;
			}
			for ( uint m = zend ; m > csr && m >= lblnk ; --m )
			{
				if ( marea[ m ] == '*' ) { continue ; }
				zarea[ m ] = 0x00 ;
			}
			for ( uint m = csr ; m >= lblnk ; --m )
			{
				if ( marea[ m ] == '*' ) { continue ; }
				zarea[ m ] = 0x20 ;
			}
			pdl = dl ;
		}
	}

	i = 0 ;
	for ( auto it = s2data.begin() ; it != s2data.end() ; ++it, ++i )
	{
		if ( !it->ipos_addr ||
		    ( it->ipos_addr->not_valid_file() && !it->ipos_addr->is_isrt() ) ||
		      it->ipos_hex_line() ||
		      it->ipos_addr->is_excluded() ||
		      pdl == it->ipos_addr )
		{
			continue ;
		}
		zbegin = i * zareaw + 8 ;
		zend   = ( i + 1 ) * zareaw - ( zdataw - dataw ) - 1 ;
		lblnk  = zend + 1 ;
		for ( uint m = zend ; m >= zbegin ; --m )
		{
			if ( marea[ m ] == '*' ) { continue ; }
			if ( zarea[ m ] != ' ' ) { break    ; }
			lblnk = m ;
		}
		if ( !profNullA )
		{
			++lblnk ;
		}
		if ( !profNullA && lblnk == zbegin + 1 )
		{
			for ( uint m = zbegin ; m <= zend ; ++m )
			{
				if ( marea[ m ] == '*' ) { continue ; }
				zarea[ m ] = 0x20 ;
			}
		}
		else
		{
			for ( uint m = lblnk ; m <= zend ; ++m )
			{
				if ( marea[ m ] == '*' ) { continue ; }
				zarea[ m ] = 0x00 ;
			}
		}
	}
}


void pedit01::fill_hilight_shadow()
{
	//
	// Build il_Shadow starting at the first invalid shadow line in data, backing up
	// to the line after the position where there are no open brackets/comments.
	//
	// Stop when bottom of ZAREA reached.  Invalidate the shadow line after the last line
	// on the screen to continue building after a scroll.
	//
	// il_vShadow - true if there is a valid shadow line for this data line (stored in il_Shadow).
	// il_wShadow - true if no open brackets, comments, if or do statements (optional), quotes or
	//              a continuation at the end of the file line for this shadow line.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;

	Data::Iterator dl = nullptr ;
	Data::Iterator ll = nullptr ;
	Data::Iterator  w = nullptr ;

	iline* dlx ;

	Data::Iterator it = nullptr ;

	const string blue = string( zdataw, N_BLUE ) ;

	string::const_iterator it1 ;
	string::const_iterator it2 ;

	hlight.hl_language  = detLang ;
	hlight.hl_lnumS2pos = lnumS2pos ;
	hlight.hl_lnumSize1 = lnumSize1 ;
	hlight.hl_lnumSize2 = lnumSize2 ;

	for ( i = zaread-1 ; i >= 0 ; --i )
	{
		ll = s2data[ i ].ipos_addr ;
		if ( ll != NULL ) { break ; }
	}

	if ( ll != data.bottom() ) { ++ll ; }

	w = data.begin() ;
	for ( it = data.begin() ; it != ll ; ++it )
	{
		if ( it->not_valid_file() ) { continue ; }
		if ( !it->il_vShadow ) { break  ; }
		if (  it->il_wShadow ) { w = it ; }
	}
	dl = it ;

	if ( data.lt( dl, ll ) )
	{
		hlight.hl_oBrac1   = 0 ;
		hlight.hl_oBrac2   = 0 ;
		hlight.hl_oIf      = 0 ;
		hlight.hl_oDo      = 0 ;
		hlight.hl_oDO      = 0 ;
		hlight.hl_oDOT     = 0 ;
		hlight.hl_oSEL     = 0 ;
		hlight.hl_ifLogic  = profIfLogic ;
		hlight.hl_doLogic  = profDoLogic ;
		hlight.hl_Paren    = profParen   ;
		hlight.hl_oComment = false ;
		hlight.hl_mismatch = false ;
		it = w ;
		for ( ++it ; it != ll ; ++it )
		{
			if ( it->not_valid_file() ) { continue ; }
			addHilight( lg,
				    hlight,
				    it->get_idata(),
				    it->il_Shadow ) ;
			if ( hlight.hl_abend )
			{
				pcmd.set_msg( "PEDT013E", 12 ) ;
				return ;
			}
			it->il_vShadow = true ;
			it->il_wShadow = ( hlight.hl_oBrac1 == 0 &&
					   hlight.hl_oBrac2 == 0 &&
					   hlight.hl_oIf    == 0 &&
					   hlight.hl_oDo    == 0 &&
					   hlight.hl_oDO    == 0 &&
					   hlight.hl_oDOT   == 0 &&
					   hlight.hl_oSEL   == 0 &&
					  !hlight.hl_continue    &&
					  !hlight.hl_oQuote      &&
					  !hlight.hl_oComment ) ;
			if ( !misADDR && hlight.hl_mismatch )
			{
				misADDR = itr2ptr( it ) ;
			}
		}
		it = getFileLineNext( it ) ;
		if ( it != data.end() )
		{
			it->il_vShadow = false ;
			it->il_wShadow = false ;
		}
	}

	for ( i = 0 ; i < zaread ; ++i )
	{
		ipos& tpos = s2data[ i ] ;
		if ( tpos.ipos_hex_line() ) { continue ; }
		dlx = tpos.ipos_addr ;
		if ( !dlx                   ||
		      dlx->not_valid_file() ||
		      dlx->marked()         ||
		      dlx->is_excluded() )  { continue ; }
		if ( dlx->specialLabel() )
		{
			zshadow.replace( (zareaw*i + CLINESZ), zdataw, blue ) ;
		}
		else if ( dlx->il_Shadow.size() >= startCol )
		{
			it1 = zshadow.begin() + ( zareaw*i + CLINESZ ) ;
			it2 = dlx->il_Shadow.begin() + ( startCol - 1 ) ;
			j   = dlx->il_Shadow.size() - ( startCol - 1 ) ;
			if ( j > zdataw )
			{
				tpos.set_shadow( dlx->il_Shadow.substr( startCol - 1 + zdataw ) ) ;
				j = zdataw ;
			}
			zshadow.replace( it1, it1+j, it2, it2+j ) ;
		}
	}
}


void pedit01::clr_hilight_shadow()
{
	TRACE_FUNCTION() ;

	for_each( data.begin(), data.end(),
		[](iline& a)
		{
			a.il_vShadow = false ;
			a.il_wShadow = false ;
		} ) ;

	hlight.hl_clear() ;
	misADDR = nullptr ;
}


void pedit01::getZAREAchanges()
{
	//
	// Put back non-display characters stored in XAREA (replaced by datain attribute).  Set touched/changed byte.
	//
	// Algorithm for getting the line command:
	//    Remove leading digits and spaces (these are ignored).
	//    Find last changed character.
	//    If cursor is on the field and after this point, use cursor positon to truncate field
	//    (characters after cursor position are ignored if not changed).
	//    Else use this changed character position to truncate field (trailing unchanged characters are ignored).
	//    If cursor not on a labelled field containing spaces, first word is the command, second the multiplier
	//    (remove non-numeric data from the multiplier).
	//
	// Strip off leading "- - - " for excluded lines.
	// Strip off leading "''''''" for new insert lines.
	// Strip off leading "*" for top/bottom of data lines.
	// Strip off label characters if cursor still on the line command starting from the cursor, or command has
	// been entered after the label (label+space+lcc).
	//

	TRACE_FUNCTION() ;

	int i  ;
	int l  ;
	int sp ;
	int ep ;

	bool touched ;
	bool changed ;

	uint j ;
	uint off ;

	iline* dlx ;

	string t1 ;
	string t2 ;
	string lcc ;
	string label ;

	const char duserMod( USERMOD ) ;
	const char ddataMod( DATAMOD ) ;

	for ( i = 0 ; i < zaread ; ++i )
	{
		off     = i * zareaw + 7 ;
		touched = false ;
		changed = false ;
		for ( l = off + 1, j = 0 ; j < zdataw ; ++j, ++l )
		{
			if ( marea[ l ] == '*' )
			{
				if      ( zarea[ l ] == duserMod ) { touched = true ; }
				else if ( zarea[ l ] == ddataMod ) { changed = true ; }
				zarea[ l ] = xarea[ l ] ;
			}
			else if ( zarea[ l ] == 0x00 )
			{
				zarea[ l ] = 0x20 ;
			}
		}
		if ( changed )
		{
			zarea[ off ] = ddataMod ;
			zchanged     = true ;
		}
		else if ( touched )
		{
			zarea[ off ] = duserMod ;
		}
	}

	lChanged.clear() ;
	dTouched.clear() ;
	dChanged.clear() ;

	for ( i = 0 ; i < zaread ; ++i )
	{
		lChanged.push_back( false ) ;
		dTouched.push_back( false ) ;
		dChanged.push_back( false ) ;
		dlx = s2data[ i ].ipos_addr ;
		if ( !dlx ) { continue ; }
		off = i * zareaw ;
		if ( zarea[ off ] == ddataMod )
		{
			lChanged[ i ] = true ;
			zchanged      = true ;
			lcc   = "" ;
			label = ( dlx->is_excluded() ) ? "" : dlx->get_label() ;
			if ( profTabs && tabsChar == ' ' && zarea.compare( ( off + 1 ), 6, "      " ) == 0 )
			{
				noAttrTabs.insert( dlx ) ;
			}
			if ( dlx->il_lcc == "" && label == "" )
			{
				for ( j = ( off + 1 ) ; j < ( off + 6 ) ; ++j )
				{
					if ( zarea[ j ] != ' ' && !isdigit( zarea[ j ] ) )
					{
						break ;
					}
					zarea[ j ] = ' ' ;
				}
				sp = j - off ;
				if ( aLCMD && aRow == ( i + 1 ) )
				{
					for ( j = ( off + 6 ) ; j > off ; --j )
					{
						if ( j < ( aCol + off - 1 ) || ( zarea[ j ] != carea[ j ] ) )
						{
							break ;
						}
					}

				}
				else
				{
					for ( j = ( off + 6 ) ; j > off ; --j )
					{
						if ( zarea[ j ] != carea[ j ] ) { break ; }
					}

				}
				ep = j - off + 1 ;
				if ( sp < ep )
				{
					lcc = zarea.substr( ( sp + off ), ( ep - sp ) ) ;
				}
			}
			else if ( aLCMD && aRow == ( i + 1 ) && carea[ off + 1 ] == '.' &&
								zarea[ off + 1 ] != '.' )
			{
				for ( j = ( off + 6 ) ; j > off ; --j )
				{
					if ( j < ( aCol + off - 1 ) || ( zarea[ j ] != carea[ j ] ) )
					{
						break ;
					}
				}
				lcc = zarea.substr( ( off + 1 ), ( j - off ) ) ;
			}
			else
			{
				lcc = zarea.substr( ( off + 1 ), 6 ) ;
			}
			trim( lcc ) ;
			if ( label != "" && lcc.find( ' ' ) != string::npos )
			{
				if ( label == word( lcc, 1 ) )
				{
					lcc = subword( lcc, 2 ) ;
				}
				else
				{
					t1 = word( lcc, 1 ) ;
					t2 = subword( lcc, 2 ) ;
					for ( j = 0 ; j < t2.size() ; ++j )
					{
						if ( !isdigit( t2[ j ] ) )
						{
							t2[ j ] = ' ' ;
						}
					}
					trim( t2 ) ;
					if ( t2 == "" || isnumeric( t2 ) )
					{
						lcc = t1 + " " + t2 ;
					}
				}
			}
			trim( iupper( lcc ) ) ;
			if ( lcc == "" )
			{
				dlx->resetFileStatus() ;
			}
			else if ( dlx->is_excluded() )
			{
				lcc.erase( 0, lcc.find_first_not_of( " -" ) ) ;
			}
			else if ( dlx->is_tod_or_bod() )
			{
				lcc.erase( 0, lcc.find_first_not_of( " *" ) ) ;
			}
			else if ( dlx->is_isrt() )
			{
				lcc.erase( 0, lcc.find_first_not_of( " '" ) ) ;
			}
			else if ( !dlx->is_file() )
			{
				lcc.erase( 0, lcc.find_first_not_of( " =" ) ) ;
			}
			if ( isnumeric( lcc ) ) { lcc = "" ; }
			if ( lcc == "" && dlx->il_lcc == "" )
			{
				dlx->clearLabel( level ) ;
			}
			else
			{
				dlx->il_lcc = lcc ;
				dlx->resetFileStatus() ;
				zarea.replace( off+1, 6, left( lcc, 6 ) ) ;
				cshadow.replace( off, 8, slr ) ;
			}
			rebuildZAREA = true ;
		}
		if ( zarea[ off + 7 ] == duserMod )
		{
			dTouched[ i ] = true ;
		}
		else if ( zarea[ off + 7 ] == ddataMod )
		{
			dChanged[ i ] = true ;
			zchanged      = true ;
		}
	}
}


void pedit01::updateData()
{
	//
	// Update the data vector with changes made to ZAREA.
	//
	// First, process any changed hex display lines and update the character data with any changes.
	// If only one of the hex lines is visible, use the low 4-bits from the character data (for HEX VERT).
	//
	// Update shadow variable (0x00 until EOD, then 0xFF) and convert nulls to spaces in ZAREA for any
	// changes past the old EOD (old EOD is max of ipos_lchar,shadow0xFF).  This is in case NULLS is ON.
	//
	// In the shadow variable, 0xFE indicates a character delete, 0xFF indicates nulls->spaces conversion.
	//

	TRACE_FUNCTION() ;

	int d ;
	int o ;
	int p ;
	int oldValue1 ;
	int oldValue2 ;

	int dataw = get_datawidth() ;

	size_t p1  ;
	size_t p2  ;
	size_t k   ;
	size_t m   ;
	size_t n   ;
	size_t l   ;
	size_t ln  ;
	size_t l1  ;
	size_t l1e ;
	size_t l1h ;
	size_t l2  ;
	size_t l3  ;

	uint i ;
	uint j ;

	string s  ;
	string t  ;
	string t1 ;
	string t2 ;

	iline* dlx ;

	set<int> isrt_set ;

	for ( i = 0 ; i < zaread ; ++i )
	{
		const ipos& tpos = s2data[ i ] ;
		if ( !dChanged[ i ] || !tpos.ipos_hex_line() )
		{
			continue ;
		}
		ln  = i - tpos.ipos_hex ;
		l1  = ln * zareaw + CLINESZ ;
		l1h = l1 + dataw/2 - 1 ;
		l1e = l1 + dataw - 1  ;
		l2  = l1 + zareaw     ;
		l3  = l2 + zareaw     ;
		m   = 0               ;
		dChanged[ ln ] = true ;
		if ( profVert )
		{
			if ( tpos.ipos_hex_top() )
			{
				dChanged[ i+1 ] = false ;
			}
			for ( j = l2, k = l1 ; k <= l1e ; ++j, ++k )
			{
				s = string( 1, zarea[ j ] ) ;
				t = string( 1, carea[ j ] ) ;
				if ( s == " " ) { s = t ; }
				if ( j+zareaw < zasize )
				{
					s.push_back( zarea[ j+zareaw ] ) ;
					t.push_back( carea[ j+zareaw ] ) ;
					if ( s[ 1 ] == ' ' ) { s[ 1 ] = t[ 1 ] ; }
				}
				else
				{
					s.push_back( c2xs( zarea[ k ] )[ 1 ] ) ;
					t.push_back( c2xs( carea[ k ] )[ 1 ] ) ;
				}
				if ( !ishex( s ) )
				{
					iupper( zarea, l2, l2+zdataw-1 ) ;
					iupper( zarea, l3, l3+zdataw-1 ) ;
					pcmd.set_msg( "PEDT011K", 12 ) ;
					cursor.set( ishex( s.front() ) ? i+2 : i+1, CSR_OFF_LCMD, k-l1 ) ;
					return ;
				}
				if ( ( ( k-l1 <= s2data[ ln ].ipos_lchar ) && s == t ) ||
				     ( ( k-l1 >  s2data[ ln ].ipos_lchar ) && s == t && s != "00" ) ) { continue ; }
				zarea[ k ] = xs2cs( s )[ 0 ] ;
				isrt_set.insert( k ) ;
				m = k - l1 ;
			}
		}
		else
		{
			if ( tpos.ipos_hex_top() )
			{
				l = l2  ;
				j = l2  ;
				k = l1  ;
				n = l1h ;
			}
			else
			{
				l = l3    ;
				j = l3    ;
				k = l1h+1 ;
				n = l1e   ;
			}
			for ( ; k <= n ; j += 2, ++k )
			{
				s = zarea.substr( j, 2 ) ;
				t = carea.substr( j, 2 ) ;
				if ( s[ 0 ] == ' ' ) { s[ 0 ] = t[ 0 ] ; }
				if ( s[ 1 ] == ' ' ) { s[ 1 ] = t[ 1 ] ; }
				if ( !ishex( s ) )
				{
					iupper( zarea, l2, l2+zdataw-1 ) ;
					iupper( zarea, l3, l3+zdataw-1 ) ;
					pcmd.set_msg( "PEDT011K", 12 ) ;
					cursor.set( i+1, CSR_OFF_LCMD, ( ishex( s.front() ) ) ? j-l+1 : j-l ) ;
					return ;
				}
				if ( ( ( k-l1 <= s2data[ ln ].ipos_lchar ) && s == t ) ||
				     ( ( k-l1 >  s2data[ ln ].ipos_lchar ) && s == t && s != "00" ) ) { continue ; }
				zarea[ k ] = xs2cs( s )[ 0 ] ;
				isrt_set.insert( k ) ;
				m = k - l1 ;
			}
		}
		n = zshadow.find_last_not_of( 0xFF, l1e ) ;
		if ( n == string::npos || n < l1 || ( n - l1 + 1 == zdataw ) )
		{
			n = s2data[ ln ].ipos_lchar ;
		}
		else
		{
			n = n - l1 + 1 ;
		}
		if ( m >= n )
		{
			for ( j = n ; j <= m ; ++j )
			{
				if ( isrt_set.count( l1+j ) == 0 )
				{
					zarea[ l1+j ] = 0x20 ;
				}
			}
			n = m + 1 ;
		}
		zshadow.replace( l1, n, n, 0x00 ) ;
		zshadow.replace( l1+n, zdataw-n, zdataw-n, 0xFF ) ;
		rebuildZAREA = true ;
	}

	for ( i = 0 ; i < zaread ; ++i )
	{
		const ipos& tpos = s2data[ i ] ;
		dlx = tpos.ipos_addr ;
		if ( ( !dlx || tpos.ipos_hex_line() ) || ( !dTouched[ i ] && !dChanged[ i ] && tpos.ipos_oupdate == "N" ) )
		{
			continue ;
		}
		if ( dTouched[ i ] && tpos.ipos_oupdate == "N" )
		{
			if ( profCaps )
			{
				if ( dlx->set_idata_upper( level, ID_TFTS ) )
				{
					fileChanged  = true ;
					rebuildZAREA = true ;
				}
			}
			if ( !optPreserve && reclen == 0 && not dlx->is_isrt() )
			{
				if ( dlx->set_idata_trim( level ) )
				{
					fileChanged  = true ;
					rebuildZAREA = true ;
				}
			}
		}
		else if ( dlx->is_bnds() )
		{
			oldValue1 = LeftBnd ;
			oldValue2 = RightBnd ;
			t = zarea.substr( CLINESZ+(i*zareaw), zdataw ) ;
			p = LeftBnd - startCol ;
			if ( p >= 0 && p < zdataw && t[ p ] == ' ' ) { LeftBnd  = 1 ; }
			p = RightBnd - startCol ;
			if ( p >= 0 && p < zdataw && t[ p ] == ' ' ) { RightBnd = 0 ; }
			if ( t.find( '<' ) == string::npos ) { LeftBndSet = false ; }
			p1 = 0 ;
			while ( true )
			{
				p1 = t.find( '<', p1 ) ;
				if ( p1 == string::npos ) { break ; }
				if ( p1 != LeftBnd - startCol )
				{
					LeftBnd = startCol + p1 ;
					break ;
				}
				++p1 ;
			}
			p1 = 0 ;
			while ( true )
			{
				p1 = t.find( '>', p1 ) ;
				if ( p1 == string::npos ) { RightBndSet = false ; break ; }
				if ( p1 != RightBnd - startCol )
				{
					RightBnd = startCol + p1 ;
					break ;
				}
				++p1 ;
			}
			if ( RightBnd <= LeftBnd ) { RightBnd = 0 ; }
			if ( profNum )
			{
				j = lnumSize1 + 1 ;
				if ( LeftBnd < j )
				{
					if ( t.find( '<' ) != string::npos )
					{
						pcmd.set_msg( "PEDT016U", 4 ) ;
					}
					LeftBnd    = j ;
					oldValue1  = j ;
					LeftBndSet = false ;
					if ( RightBnd == j ) { RightBnd = 0 ; }
				}
				if ( RightBnd > 0 && RightBnd < j )
				{
					pcmd.set_msg( "PEDT016V", 4 ) ;
					RightBnd    = 0 ;
					RightBndSet = false ;
				}
				if ( lnumS2pos > 0 )
				{
					if ( RightBnd > lnumS2pos )
					{
						pcmd.set_msg( "PEDT016V", 4 ) ;
						RightBnd = lnumS2pos ;
					}
					else if ( RightBnd == 0 )
					{
						RightBnd = lnumS2pos ;
					}
				}
			}
			if ( reclen > 0 )
			{
				if ( RightBnd > reclen )
				{
					pcmd.set_msg( "PEDT016V", 4 ) ;
					RightBnd = reclen ;
				}
				else if ( RightBnd == 0 )
				{
					RightBnd = reclen ;
				}
			}
			if ( RightBnd > 0 && LeftBnd >= RightBnd )
			{
				LeftBnd = ( oldValue1 > RightBnd ) ? lnumSize1 + 1 : oldValue1 ;
			}
			if ( oldValue1 != LeftBnd  ) { LeftBndSet  = true ; }
			if ( oldValue2 != RightBnd ) { RightBndSet = true ; }
		}
		else
		{
			modLine = dlx ;
			dlx->resetFileStatus() ;
			t = dlx->get_idata() ;
			o = ( i * zareaw ) + CLINESZ ;
			s = zshadow.substr( o, zdataw ) ;
			if ( reclen >= startCol && ( reclen + startCol < zdataw ) )
			{
				s.resize( ( reclen - startCol + 1 ) ) ;
			}
			k = s.find_last_not_of( "\xFE\xFF" ) ;
			d = ( k == string::npos ) ? countc( s, 0xFE ) : countc( s.substr( k ), 0xFE ) ;
			s = zarea.substr( o, zdataw ) ;
			if ( profNum       &&
			     lnumSize1 > 0 &&
			     ( dlx->is_valid_file() || dlx->is_isrt() ) &&
			     startCol < ( lnumSize1 + 1 ) )
			{
				t1 = s.substr( 0, ( lnumSize1 - startCol + 1 ) ) ;
				t2 = carea.substr( o, ( lnumSize1 - startCol + 1 ) ) ;
				if ( t1 != t2 )
				{
					pcmd.set_msg( "PEDT016W", 4 ) ;
				}
				s.replace( 0, t2.size(), t2 ) ;
			}
			if ( profNum       &&
			     lnumSize2 > 0 &&
			     ( dlx->is_valid_file() || dlx->is_isrt() ) &&
			     lnumS2pos < ( zdataw + startCol - 1 ) )
			{
				p2 = min( ( zdataw + startCol - lnumS2pos - 1 ), lnumSize2 ) ;
				t1 = s.substr( ( lnumS2pos - startCol + 1 ), p2 ) ;
				t2 = carea.substr( ( o + lnumS2pos - startCol + 1 ), p2 ) ;
				if ( t1 != t2 )
				{
					pcmd.set_msg( "PEDT016W", 4 ) ;
				}
				s.replace( ( lnumS2pos - startCol + 1 ), t2.size(), t2 ) ;
			}
			if ( t.size() < startCol && k == string::npos ) { continue ; }
			t = updateLine( tpos, dlx, s, t, k, d, o ) ;
			if ( dlx->is_mask() )
			{
				maskLine = t ;
				dlx->put_idata( maskLine ) ;
			}
			else if ( dlx->is_tabs() )
			{
				for ( j = 0 ; j < t.size() ; ++j )
				{
					char& c = t[ j ] ;
					if ( c == '_' )
					{
						c = '-' ;
					}
					else if ( c != ' ' && c != '*' && c != '-' )
					{
						c = ' ' ;
					}
				}
				if ( reclen > 0 )
				{
					t.resize( reclen, ' ' ) ;
				}
				tabsLine = t ;
				dlx->put_idata( tabsLine ) ;
			}
			else if ( dlx->is_isrt() )
			{
				dlx->put_idata( t ) ;
			}
			else if ( dlx->put_idata( t, level, ID_OWRITE ) )
			{
				dlx->update_lnummod( lnumSize1, lnumS2pos, lnummod ) ;
				fileChanged = true ;
			}
		}
		rebuildZAREA = true ;
	}
}


string pedit01::updateLine( const ipos& tpos,
			    iline* dlx,
			    string seg,
			    string rec,
			    size_t k,
			    int d,
			    int off )
{
	//
	// Update the data record from the ZDATA segment and overflow variables.
	//
	// If logical tabbing is in effect, process any entered tab characters.
	//
	// tpos - s2data for line being processed.
	// dlx  - iline pointer
	// rec  - line data
	// seg  - ZAREA segment
	// k    - last position in shadow not FE/FF (last byte of real data).
	// d    - count of shadow FE (character deletes)
	// off  - ZAREA offset.
	//

	TRACE_FUNCTION() ;

	int i ;

	string t1 ;
	string t2 ;
	string t3 ;

	size_t p1 ;
	size_t pl ;

	bool tabs = false ;

	vector<size_t> v_tabs ;

	if ( profTabs && tabsChar != ' ' && ( dlx->is_valid_file() || dlx->is_isrt() ) && seg.find( tabsChar ) != string::npos )
	{
		t1 = carea.substr( off, zdataw ) ;
		t2 = string( zdataw, ' ' ) ;
		for ( uint i = 0 ; i < t1.size() ; ++i )
		{
			if ( seg[ i ] != t1[ i ] )
			{
				t2[ i ] = seg[ i ] ;
			}
		}
		p1 = t2.find( tabsChar ) ;
		if ( p1 != string::npos )
		{
			if ( aADDR == dlx && aDATA && p1 <= ( aCol - CLINESZ - 1 ) )
			{
				t3 = seg.substr( p1, ( aCol - CLINESZ - 1 - p1 ) ) ;
				seg.replace( p1, t3.size(), t1.substr( p1 ) ) ;
				trim_left( t3 ) ;
			}
			else
			{
				t3 = t2.substr( p1 ) ;
				seg.replace( p1, t3.size(), t1.substr( p1 ) ) ;
				trim( t3 ) ;
			}
			pl   = p1 + startCol - 1 ;
			tabs = true ;
		}
	}

	if ( rec.size() <= ( startCol + zdataw - 1 ) && tpos.ipos_ovalue == "" )
	{
		if ( lnumSize2 == 0 )
		{
			if ( k == string::npos )
			{
				seg = ( lnumSize1 > 0 && lnumSize1 > ( startCol - 1 ) ) ? seg.substr( 0, ( lnumSize1 - startCol + 1 ) ) : "" ;
			}
			else
			{
				seg.erase( k+d+1 ) ;
			}
		}
		rec = substr( rec, 1, startCol-1 ) + seg ;
		if ( !optPreserve && reclen == 0 )
		{
			trim_right( rec ) ;
		}
	}
	else
	{
		seg += tpos.ipos_ovalue ;
		if ( lnumSize2 > 0 )
		{
			i = rec.size() - startCol - zdataw + 1 ;
			if ( i >= lnumSize2 )
			{
				seg.resize( ( zdataw + i - lnumSize2 ), ' ' ) ;
				seg += rec.substr( lnumS2pos ) ;
			}
			else if ( i > 0 )
			{
				seg += rec.substr( lnumS2pos + ( lnumSize2 - i ) ) ;
			}
		}
		rec = substr( rec, 1, startCol - 1 ) + seg ;
	}

	if ( reclen > 0 )
	{
		rec.resize( reclen, ' ' ) ;
	}

	if ( tabs )
	{
		getLogicalTabLocations( v_tabs ) ;
		if ( v_tabs.empty() )
		{
			pcmd.set_msg( "PEDT018P", 4 ) ;
		}
		else
		{
			applyTabs( dlx, v_tabs, rec, t3, max( pl, lnumSize1 + 1 ) ) ;
		}
	}

	return ( profCaps ) ? upper( rec ) : rec ;
}


void pedit01::applyTabs( iline* dlx,
			 vector<size_t>& v_tabs,
			 string& ltline,
			 const string& tabsl,
			 size_t pl,
			 size_t p2 )
{
	//
	// Apply logical tabs in tabsl to ltline.  If more tabs than tab positions then insert new line and
	// call ourselves again for the rest.
	//

	TRACE_FUNCTION() ;

	int i = 0 ;

	size_t p1 ;
	size_t p3 ;

	bool loop = true ;
	bool more = true ;

	string t1 ;

	while ( pl > 0 && i < v_tabs.size() )
	{
		p3 = v_tabs[ i ] ;
		if ( p3 > pl ) { break ; }
		++i ;
	}

	while ( loop && i < v_tabs.size() )
	{
		p3 = v_tabs[ i ] ;
		++i ;
		if ( pl > p3 )
		{
			p3 = pl ;
			if ( ltline.size() >= p3 )
			{
				ltline[ p3-1 ] = ' ' ;
			}
		}
		p1 = tabsl.find( tabsChar, p2 ) ;
		if ( p1 == string::npos )
		{
			more = false ;
			break ;
		}
		p2 = tabsl.find( tabsChar, p1 + 1 ) ;
		if ( p2 == string::npos )
		{
			t1   = tabsl.substr( p1 + 1 ) ;
			loop = false ;
			more = false ;
		}
		else
		{
			t1 = tabsl.substr( p1 + 1, p2 - p1 - 1 ) ;
		}
		if ( lnumS2pos > 0 && ( p3 + t1.size() ) > lnumS2pos )
		{
			t1.resize( lnumS2pos - p3, ' ' ) ;
			pcmd.set_msg( "PEDT016W", 4 ) ;
		}
		if ( ltline.size() < ( p3 + t1.size() ) )
		{
			ltline.resize( p3 + t1.size(), ' ' ) ;
		}
		ltline.replace( p3, t1.size(), t1 ) ;
		pl = p3 + t1.size() + 1 ;
	}

	if ( more )
	{
		string newrec = getMaskLine2() ;
		applyTabs( dlx, v_tabs, newrec, tabsl, 0, p2 ) ;
		if ( profCaps )
		{
			iupper( newrec ) ;
		}
		iincr( dlx ) ;
		data.insert( dlx, new iline( taskid(), LN_FILE, lnumSize ), newrec, level, ( ID_ISRT | ID_OWRITE ) ) ;
	}
}


void pedit01::processNewInserts()
{
	//
	// Remove new insert lines that have not been changed/touched unless line command
	// has been entered or data is being scrolled.
	//
	// Add a new line below inserted lines when changed/touched and cursor is still on the line.
	//

	TRACE_FUNCTION() ;

	uint vis ;

	iline* dl ;

	for ( int i = zaread-1 ; i >= 0 ; --i )
	{
		ipos& tpos = s2data[ i ] ;
		dl = tpos.ipos_addr ;
		if ( !dl || !dl->is_isrt() || tpos.ipos_hex_line() ) { continue ; }
		if ( !dTouched[ i ] && !dChanged[ i ] )
		{
			if ( !lChanged[ i ] && !scrollData && dl->il_lcc == "" )
			{
				tpos.set_null() ;
				dl = getDataLineNext( delete_line( dl, true ) ) ;
				if ( dl == aADDR )
				{
					moveCursorLine( dl ) ;
				}
				rebuildZAREA = true ;
			}
			else
			{
				moveCursorLine( dl ) ;
			}
		}
		else
		{
			dl->convert_to_file( level, lnumSize ) ;
			if ( dl == aADDR && !aLCMD )
			{
				vis = countVisibleLines( dl ) ;
				if ( vis < 2 )
				{
					moveDownLines( ( 2 - vis ) ) ;
				}
				iincr( dl ) ;
				dl = data.insert( dl, new iline( taskid(), LN_ISRT, lnumSize ), getMaskLine2() ) ;
				moveCursorLine( dl ) ;
			}
			fileChanged  = true ;
			rebuildZAREA = true ;
		}
	}
}


void pedit01::actionPrimCommand1()
{
	//
	// Action primary command.
	// These commands are executed after finding data changes but before updating the data.
	//

	TRACE_FUNCTION() ;

	bool found = false ;

	if ( pcmd.get_cmd_words() == 0 ) { return ; }

	string w1 = upper( word( pcmd.get_cmd(), 2 ) ) ;

	switch ( pcmd.p_cmd )
	{
	case PC_REDO:
			pcmd.actioned() ;
			if ( w1 == "" )
			{
				action_REDO() ;
			}
			else if ( w1 == "ALL" )
			{
				while ( action_REDO() )
				{
					found = true ;
				}
				if ( found )
				{
					pcmd.set_msg( "PEDT016X", 4 ) ;
				}
			}
			else
			{
				pcmd.set_msg( "PEDT011", 12 ) ;
			}
			break ;

	case PC_UNDO:
			pcmd.actioned() ;
			if ( w1 == "" )
			{
				action_UNDO() ;
			}
			else if ( w1 == "ALL" )
			{
				while ( action_UNDO() )
				{
					found = true ;
				}
				if ( found )
				{
					pcmd.set_msg( "PEDT016Y", 4 ) ;
				}
			}
			else
			{
				pcmd.set_msg( "PEDT011", 12 ) ;
			}
			break ;
	}
}


void pedit01::actionPrimCommand2()
{
	//
	// Action primary command.
	// These commands are executed before line command processing.
	//

	TRACE_FUNCTION() ;

	uint i ;
	uint j ;
	uint ws ;

	int rc ;

	string cmd ;
	string fname ;
	string lab1 ;
	string lab2 ;
	string wall ;
	string w ;
	string w1 ;
	string w2 ;
	string w3 ;
	string w4 ;
	string t ;

	iline* sidx ;
	iline* eidx ;

	queue<string> rlist ;

	Data::Iterator it  = nullptr ;
	Data::Iterator its = nullptr ;
	Data::Iterator ite = nullptr ;

	vector<ipline> vip ;

	cmd = pcmd.get_cmd() ;
	ws  = pcmd.get_cmd_words() ;

	if ( ws == 0 ) { return ; }

	if ( pcmd.isLine_cmd() )
	{
		pcmd.actioned() ;
		if ( !aADDR )
		{
			pcmd.set_msg( "PEDT013X", 12 ) ;
			return ;
		}
		if ( aADDR->il_lcc != "" )
		{
			pcmd.set_msg( "PEDT013Y", 12 ) ;
			return ;
		}
		aADDR->il_lcc = cmd ;
		rebuildZAREA = true ;
		pcmd.reset() ;
		return ;
	}

	w1 = pcmd.get_cmdf() ;
	w2 = upper( word( cmd, 2 ) ) ;
	w3 = upper( word( cmd, 3 ) ) ;
	w4 = upper( word( cmd, 4 ) ) ;

	switch ( pcmd.p_cmd )
	{
	case PC_BROWSE:
			pcmd.actioned() ;
			control( "ERRORS", "RETURN" ) ;
			w2 = word( cmd, 2 ) ;
			if ( w2 == "" )
			{
				select( "PGM(PPSP01A) PARM(BROWSEE) SCRNAME(BROWSE) SUSPEND" ) ;
			}
			else
			{
				if ( w2.front() != '/' )
				{
					w2 = zfile.substr( 0, zfile.find_last_of( '/' ) + 1 ) + w2 ;
				}
				browse( w2 ) ;
				if ( RC > 0 && isvalidName( ZRESULT ) )
				{
					pcmd.set_msg( ZRESULT, 12 ) ;
				}
			}
			control( "ERRORS", "CANCEL" ) ;
			break ;

	case PC_COPY:
			if ( pcmd.get_cmd_words() > 1 )
			{
				fname = expandFileName( subword( cmd, 2 ) ) ;
				if ( !exists( fname ) )
				{
					pcmd.set_msg( "PEDT015K", fname, "COPY", 12 ) ;
					pcmd.keep_cmd() ;
					pcmd.actioned() ;
					break ;
				}
				pcmd.set_userdata( fname ) ;
			}
			copyActive = true ;
			break ;

	case PC_CREATE:
			extdata.reset() ;
			cmd = subword( cmd, 2 ) ;
			if ( extract_labels( cmd, sidx, eidx ) )
			{
				if ( cmd == "" )
				{
					cmd = displayPanel( "PEDIT01J", "ZFILE2", "PEDT014R", true, false ) ;
					if ( RC == 8 ) { break ; }
				}
				else
				{
					cmd = expandFileName( cmd ) ;
					if ( pcmd.error() ) { break ; }
				}
				create_file( cmd, sidx, eidx ) ;
			}
			else
			{
				if ( pcmd.error() ) { break ; }
				t = expandFileName( cmd ) ;
				if ( pcmd.error() ) { break ; }
				extdata.set_create( t ) ;
			}
			break ;

	case PC_CUT:
			if ( w2 == "DISPLAY" )
			{
				manageClipboard() ;
			}
			else
			{
				wall = upper( subword( cmd, 2 ) ) ;
				rc   = extract_lptr( wall, its, ite, true, false ) ;
				if ( rc > 1 )
				{
					pcmd.set_msg( "PEDT018H", 12 ) ;
					break ;
				}
				Cut t( pcmd, profCutReplace, wall, its, ite ) ;
				if ( pcmd.error() ) { break ; }
				if ( wall != "" )
				{
					if ( words( wall ) == 1 )
					{
						clipBoard = trim( wall ) ;
					}
					else
					{
						pcmd.set_msg( "PEDT013A", 12 ) ;
					}
				}
				if ( pcmd.ok() )
				{
					if ( t.norange() )
					{
						cut       = t ;
						cutActive = true ;
					}
					else
					{
						create_copy( t, vip ) ;
						clipBoard  = t.clipBoard() ;
						cutReplace = t.repl() ;
						copyToClipboard( vip ) ;
					}
				}
			}
			break ;

	case PC_EDIT:
			pcmd.actioned() ;
			control( "ERRORS", "RETURN" ) ;
			w2 = word( cmd, 2 ) ;
			if ( w2 == "" )
			{
				select( "PGM(PPSP01A) PARM(EDITEE) SCRNAME(EDIT) SUSPEND" ) ;
			}
			else
			{
				if ( w2.front() != '/' )
				{
					w2 = zfile.substr( 0, zfile.find_last_of( '/' ) + 1 ) + w2 ;
				}
				edit( w2 ) ;
				if ( ZRESULT != "" && ( RC == 0 || RC == 4 ) )
				{
					setmsg( ZRESULT ) ;
				}
				else if ( RC > 11 && isvalidName( ZRESULT ) )
				{
					pcmd.set_msg( ZRESULT, 12 ) ;
				}
			}
			control( "ERRORS", "CANCEL" ) ;
			break ;

	case PC_EDITSET:
			pcmd.actioned() ;
			vreplace( "ZPOSFCX", ( profPosFcx     ) ? "/" : ""  ) ;
			vreplace( "ZCUTDEF", ( profCutReplace ) ? "2" : "1" ) ;
			vreplace( "ZPSTDEF", ( profPasteKeep  ) ? "2" : "1" ) ;
			vreplace( "ZETRGPOS", d2ds( targetLine ) ) ;
			vget( "ZUSERMAC", PROFILE ) ;
			addpop( "", 5, 5 ) ;
			while ( true )
			{
				display( "PEDIT019" ) ;
				if ( RC == 0 ) { continue ; }
				vget( "ZVERB", SHARED ) ;
				if ( zverb != "CANCEL" )
				{
					vcopy( "ZPOSFCX", t, MOVE ) ;
					profPosFcx     = ( t == "/" ) ;
					vcopy( "ZCUTDEF", t, MOVE ) ;
					profCutReplace = ( t == "2" ) ;
					vcopy( "ZPSTDEF", t, MOVE ) ;
					profPasteKeep  = ( t == "2" ) ;
					vcopy( "ZETRGPOS", t, MOVE ) ;
					targetLine = ds2d( t ) ;
					vput( "ZUSERMAC ZETRGPOS", PROFILE ) ;
				}
				break ;
			}
			rempop() ;
			break ;

	case PC_MOVE:
			if ( pcmd.get_cmd_words() > 1 )
			{
				string fname = expandFileName( subword( cmd, 2 ) ) ;
				if ( !exists( fname ) )
				{
					pcmd.set_msg( "PEDT015K", fname, "MOVE", 12 ) ;
					pcmd.keep_cmd() ;
					pcmd.actioned() ;
					break ;
				}
				pcmd.set_userdata( fname ) ;
			}
			moveActive = true ;
			break ;

	case PC_PASTE:
			pasteActive = true ;
			pasteKeep   = profPasteKeep ;
			clipBoard   = "DEFAULT" ;
			wall        = upper( subword( cmd, 2 ) ) ;

			parseString2( wall, "DEFAULT" ) ;

			if ( parseString2( wall, "KEEP" ) )
			{
				pasteKeep = true ;
			}
			else if ( parseString2( wall, "DELETE" ) )
			{
				pasteKeep = false ;
			}

			if ( wall != "" )
			{
				if ( words( wall ) == 1 ) { clipBoard = wall ; }
				else                      { pcmd.set_msg( "PEDT013B", 12 ) ; }
			}
			break ;

	case PC_REPLACE:
			extdata.reset() ;
			cmd = subword( cmd, 2 ) ;
			if ( extract_labels( cmd, sidx, eidx ) )
			{
				if ( cmd == "" )
				{
					cmd = displayPanel( "PEDIT01I", "ZFILE2", "", false ) ;
					if ( RC == 8 ) { break ; }
				}
				else
				{
					cmd = expandFileName( cmd ) ;
					if ( pcmd.error() ) { break ; }
				}
				if ( exists( cmd ) && !confirmReplace( cmd ) )
				{
					break ;
				}
				create_file( cmd, sidx, eidx ) ;
			}
			else
			{
				if ( pcmd.error() ) { break ; }
				t = expandFileName( cmd ) ;
				if ( pcmd.error() ) { break ; }
				extdata.set_replace( t ) ;
			}
			break ;

	case PC_RESET:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			wall = upper( cmd ) ;
			for ( j = 0, i = 2 ; i <= ws ; ++i )
			{
				w = word( wall, i ) ;
				if ( w.front() == '.' )
				{
					( ++j == 1 ) ? lab1 = w : lab2 = w ;
				}
				else
				{
					w = get_truename( w ) ;
					if ( !findword( w, "ALL CHA CMD ERR FIND LAB H UNDO PROFILE REDO SPE T X" ) )
					{
						     pcmd.set_msg( "PEDT011", 20 ) ;
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
				its = getLabelIterator( lab1, rc ) ;
				if ( rc != 0 )
				{
					pcmd.set_msg( "PEDT011F", 20 ) ;
					break ;
				}
				ite = getLabelIterator( lab2, rc ) ;
				if ( rc != 0 )
				{
					pcmd.set_msg( "PEDT011F", 20 ) ;
					break ;
				}
				if ( data.gt( its, ite ) )
				{
					swap( its, ite ) ;
				}
				++ite ;
			}
			else
			{
				pcmd.set_msg( "PEDT014U", 20 ) ;
				break ;
			}
			if ( rlist.empty() )
			{
				removeSpecialLines( its, ite ) ;
				for_each( its, ite,
					[ this ](iline& a)
					{
						a.resetFilePrefix( level ) ;
						a.clearLcc() ;
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
				rlist.pop() ;
				if ( w == "ALL" )
				{
					removeSpecialLines( its, ite ) ;
					for_each( its, ite,
						[ this ](iline& a)
						{
							a.resetFilePrefix( level ) ;
							a.clearLcc() ;
							a.clearLabel( level ) ;
						} ) ;
					hideExcl = false ;
				}
				else if ( w == "CMD" )
				{
					for_each( its, ite,
						[](iline& a)
						{
							a.clearLcc() ;
						} ) ;
				}
				else if ( w == "CHA" )
				{
					for_each( its, ite,
						[](iline& a)
						{
							a.clrChngStatus() ;
						} ) ;
				}
				else if ( w == "ERR" )
				{
					for_each( its, ite,
						[](iline& a)
						{
							a.clrErrorStatus() ;
						} ) ;
				}
				else if ( w == "LAB" )
				{
					for_each( its, ite,
						[ this ](iline& a)
						{
							a.clearLabel( level ) ;
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
						[](iline& a)
						{
							a.resetMarked() ;
						} ) ;
				}
				else if ( w == "UNDO" )
				{
					for_each( its, ite,
						[](iline& a)
						{
							a.clrUndoStatus() ;
						} ) ;
				}
				else if ( w == "PROFILE" )
				{
					removeProfLines() ;
				}
				else if ( w == "REDO" )
				{
					for_each( its, ite,
						[](iline& a)
						{
							a.clrRedoStatus() ;
						} ) ;
				}
				else if ( w == "SPE" )
				{
					removeSpecialLines( its, ite ) ;
				}
				else if ( w == "X" )
				{
					for_each( its, ite,
						[ this ](iline& a)
						{
							a.set_unexcluded( level ) ;
						} ) ;
				}
			}
			rebuildZAREA = true ;
			break ;

	case PC_VIEW:
			pcmd.actioned() ;
			control( "ERRORS", "RETURN" ) ;
			w2 = word( cmd, 2 ) ;
			if ( w2 == "" )
			{
				select( "PGM(PPSP01A) PARM(BROWSEE) SCRNAME(VIEW) SUSPEND" ) ;
			}
			else
			{
				if ( w2.front() != '/' )
				{
					w2 = zfile.substr( 0, zfile.find_last_of( '/' ) + 1 ) + w2 ;
				}
				view( w2 ) ;
				if ( ZRESULT != "" && ( RC == 0 || RC == 4 ) )
				{
					setmsg( ZRESULT ) ;
				}
				else if ( RC > 11 && isvalidName( ZRESULT ) )
				{
					pcmd.set_msg( ZRESULT, 12 ) ;
				}
			}
			control( "ERRORS", "CANCEL" ) ;
			break ;

	default:
			break ;
	}
}


void pedit01::actionPrimCommand3()
{
	//
	// Action primary command.
	// These commands are executed after line command processing (even if there is an error).
	//

	TRACE_FUNCTION() ;

	int rc ;
	int rc1 ;
	int rc2 ;

	int ii ;
	int ws ;

	int tCol ;
	int tLb ;
	int tRb ;

	uint i ;
	uint j ;

	char dir ;

	string cmd ;
	string t1 ;

	string w  ;
	string w1 ;
	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string wl ;

	string wall ;
	string lab1 ;
	string lab2 ;

	bool set_cursor ;
	bool save_changes ;

	bool tLSet ;
	bool tRSet ;

	bool hi_paren  = false ;
	bool hi_cursor = false ;
	bool hi_find   = false ;

	bool show_tabs = false ;
	bool show_mask = false ;
	bool show_bnds = false ;

	iline* tTop ;

	defName dn  ;
	D_PARMS dp  ;

	vector<string> Prof  ;
	vector<string> Msgs  ;

	vector<caution> Cautions ;

	Data::Iterator it  = nullptr ;
	Data::Iterator it1 = nullptr ;
	Data::Iterator it2 = nullptr ;
	Data::Iterator its = nullptr ;
	Data::Iterator ite = nullptr ;

	map<string,char>::iterator itl ;

	map<string,stack<defName>>::iterator ita ;
	map<string,stack<defName>>::iterator itb ;

	const string valid1 = "COBOL NOCOBOL STANDARD NOSTD DISPLAY NODISPL" ;
	const string valid2 = "COBOL STANDARD DISPLAY" ;

	set<string> klist ;

	if ( pcmd.is_macro() ) { return ; }

	cmd = pcmd.get_cmd() ;
	ws  = pcmd.get_cmd_words() ;

	if ( ws == 0 ) { return ; }

	w1 = pcmd.get_cmdf() ;
	w2 = upper( word( cmd, 2 ) ) ;
	w3 = upper( word( cmd, 3 ) ) ;
	w4 = upper( word( cmd, 4 ) ) ;
	w5 = upper( word( cmd, 5 ) ) ;

	switch ( pcmd.p_cmd )
	{
	case PC_AUTONUM:
			//
			// Automatically renumber the data when saving.
			//
			// MACRO return codes (Command and assignment syntax):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "ON" || w2 == "" )
			{
				profAutoNum = true ;
			}
			else if ( w2 == "OFF" )
			{
				profAutoNum = false ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			store_status( SS_AUTONUM ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_AUTOSAVE:
			//
			// Automatically save the data when END pressed.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 OFF NOPROMPT specified.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "ON" || w2 == "" )
			{
				if ( ws > 2 )
				{
					pcmd.set_msg( "PEDT015D", 20 ) ;
					break ;
				}
				profAutoSave = true ;
			}
			else
			{
				if ( w2 == "OFF" ) { w2 = w3 ; }
				else if ( ws > 2 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
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
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				profAutoSave = false ;
			}
			store_status( SS_AUTOSAVE ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_BACKUP:
			//
			// Create a backup on first data change.  Set path.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "OFF" )
			{
				if ( ws > 2 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				profBackup = false ;
			}
			else
			{
				if ( ws > 4 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				if ( w2 == "ON" )
				{
					w2 = w3 ;
					w3 = word( cmd, 4 ) ;
				}
				else if ( ws > 3 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				else
				{
					w3 = word( cmd, 3 ) ;
				}
				if ( w2 == "" )
				{
					profBackup = true ;
				}
				else if ( w2 == "PATH" )
				{
					if ( w3 == "" )
					{
						pcmd.set_msg( "PEDT011", 20 ) ;
						break ;
					}
					try
					{
						if ( w3.front() == '~' )
						{
							w3 = zhome + w3.substr( 1 ) ;
						}
						if ( exists( w3 ) && is_directory( w3 ) )
						{
							backupLoc = w3 ;
							if ( backupLoc.back() != '/' ) { backupLoc += '/' ; }
						}
						else
						{
							pcmd.set_msg( "PEDT017S", 8 ) ;
							set_zedimsgs() ;
							break ;
						}
					}
					catch ( const filesystem_error& ex )
					{
						vreplace( "ZVAL1", ex.what() ) ;
						pcmd.set_msg( "PSYS012C", 20 ) ;
						break ;
					}
					profBackup = true ;
				}
				else
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_BOUNDS:
			//
			// Set the left and right data boundries and save in the edit profile.
			//
			// MACRO return codes (Command and assignment syntax):
			// RC =  0 Normal completion.
			// RC =  4 Righ boundary greater than default.  Default used.
			// RC = 12 Invalid boundaries specified.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			tLSet = LeftBndSet ;
			tRSet = RightBndSet ;
			if( w2 == "*" )
			{
				tLb = LeftBnd ;
			}
			else if ( w2 == "" )
			{
				tLb   = lnumSize1 + 1 ;
				tLSet = false ;
			}
			else if ( isnumeric( w2 ) )
			{
				tLb = ds2d( w2 ) ;
				if ( tLb == 0 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				tLSet = true ;
				if ( macroRunning && lnumSize1 > 0 )
				{
					tLb += lnumSize1 ;
				}
			}
			else
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			if ( w3 == "*" )
			{
				tRb = RightBnd ;
			}
			else if ( w3 == "" )
			{
				tRb   = ( lnumS2pos > 0 ) ? lnumS2pos :
					( reclen > 0 )    ? reclen : 0 ;
				tRSet = false ;
			}
			else if ( isnumeric( w3 ) )
			{
				tRb = ds2d( w3 ) ;
				if ( macroRunning && lnumSize1 > 0 && tRb > 0 )
				{
					tRb += lnumSize1 ;
				}
				tRSet = true ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			if ( tRb > 0 && tLb >= tRb )
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			if ( profNum )
			{
				if ( tLb < ( lnumSize1 + 1 ) )
				{
					if ( w2 != "" )
					{
						pcmd.set_msg( "PEDT016U", 4 ) ;
						set_zedimsgs() ;
					}
					tLb   = lnumSize1 + 1 ;
					tLSet = false ;
					if ( tRb == ( lnumSize1 + 1 ) ) { tRb = 0 ; }
				}
				if ( tRb > 0 && tRb <= lnumSize1 )
				{
					pcmd.set_msg( "PEDT016V", 4 ) ;
					set_zedimsgs() ;
					tRb = 0 ;
				}
				if ( lnumS2pos > 0 )
				{
					if ( tRb > lnumS2pos )
					{
						pcmd.set_msg( "PEDT016V", 4 ) ;
						set_zedimsgs() ;
						tRb   = lnumS2pos ;
						tRSet = false ;
					}
					else if ( tRb == 0 )
					{
						tRb   = lnumS2pos ;
						tRSet = false ;
					}
				}
			}
			if ( reclen > 0 )
			{
				if ( tRb > reclen )
				{
					pcmd.set_msg( "PEDT016V", 4 ) ;
					set_zedimsgs() ;
					tRb   = reclen ;
					tRSet = false ;
				}
				else if ( tRb == 0 )
				{
					tRb   = reclen ;
					tRSet = false ;
				}
			}
			LeftBndSet   = tLSet ;
			RightBndSet  = tRSet ;
			LeftBnd      = tLb ;
			RightBnd     = tRb ;
			rebuildZAREA = true ;
			break ;

	case PC_CAPS:
			//
			// Set caps mode.
			//
			// MACRO return codes (Command syntax and assignment syntax):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "ON" || ws == 1 )
			{
				profCaps = true  ;
				removeSpecial( CA_CAPS_ON ) ;
			}
			else if ( w2 == "OFF" )
			{
				profCaps = false ;
				removeSpecial( CA_CAPS_OFF ) ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			buildProfLines( Prof ) ;
			updateProfLines( Prof ) ;
			store_status( SS_CAPS ) ;
			break ;

	case PC_CHANGE:
			//
			// Change a string to another.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 String not found.
			// RC =  8 Change error.  String2 longer than string1 and substitution
			//         was not performed on at least one occation.
			// RC = 12 Inconsistent parameters.  String to be found does not fit between
			//         specified columns.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( !setFindChangeExcl( 'C' ) ) { break ; }
			tTop = topLine  ;
			tCol = startCol ;
			set_cursor = false ;
			fcx_parms.set_change_counts( 0, 0 ) ;
			while ( true )
			{
				actionFind() ;
				if ( fcx_parms.f_error || !fcx_parms.f_success )
				{
					break ;
				}
				if ( !set_cursor )
				{
					setCursorFindChange() ;
					tTop = topLine ;
					tCol = startCol ;
				}
				actionChange() ;
				if ( fcx_parms.f_error )
				{
					fcx_parms.f_incr_change_errors() ;
					setChangedError() ;
					set_zedimsgs() ;
				}
				else
				{
					fcx_parms.f_incr_change_occurs() ;
					if ( !set_cursor )
					{
						setCursorChange() ;
						set_cursor = true ;
					}
				}
				if ( !fcx_parms.f_chngall )
				{
					break ;
				}
				startCol = 1 ;
				topLine  = fcx_parms.f_ADDR ;
				setCursorChange() ;
				if ( macroRunning )
				{
					mRow = fcx_parms.f_ADDR ;
					mCol = fcx_parms.f_offset + fcx_parms.f_cstring.size() + 1 ;
				}
				else
				{
					aCol = fcx_parms.f_offset + fcx_parms.f_cstring.size() + CLINESZ ;
				}
			}
			topLine  = tTop ;
			startCol = tCol ;
			if ( fcx_parms.f_ch_errs == 0 )
			{
				( fcx_parms.f_ch_occs > 0 ) ? setChangedMsg() : setNotFoundMsg() ;
				set_zedimsgs() ;
			}
			rebuildZAREA = true ;
			break ;

	case PC_COLUMN:
			//
			// Display a non-scrolling column indicator at the top of the data area.
			//
			pcmd.actioned() ;
			if      ( w2 == "ON"  ) { colsOn = true    ; }
			else if ( w2 == "OFF" ) { colsOn = false   ; }
			else if ( w2 == ""    ) { colsOn = !colsOn ; }
			else                    { pcmd.set_msg( "PEDT011", 12 ) ; break ; }
			rebuildZAREA = true ;
			break ;

	case PC_COMPARE:
			//
			// Compare data with an external source.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion
			// RC =  8 File not found or error opening file
			// RC = 12 Invalid parameters
			// RC = 20 Severe error
			//
			pcmd.actioned() ;
			compare_files( subword( cmd, 2 ) ) ;
			break ;

	case PC_DEFINE:
			//
			// Define a name (alias, macros, etc).
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  8 Reset attempted for a name not defined, or
			//         DEFINE name ALIAS name2 requested and name2 is a NOP
			// RC = 12 DEFINE requested for a name not currently defined.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
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
					pcmd.set_msg( "PEDT015P", 12 ) ;
					break ;
				}
				else if ( ita->second.top().alias )
				{
					itb = defNames.find( ita->second.top().name ) ;
					if ( itb != defNames.end() && itb->second.top().disabled )
					{
						pcmd.set_msg( "PEDT015P", 12 ) ;
						break ;
					}
				}
			}
			dp = DefParms[ w3 ] ;
			switch ( dp )
			{
			case DF_MACRO:
				if ( w4 != "CMD" && w4 != "PGM" && w4 != "" )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				if ( w4 == "CMD" || w4 == "" )
				{
					dn.cmd = true ;
				}
				else
				{
					dn.pgm = true ;
					iupper( w2 ) ;
				}
				dn.exp = true ;
				defNames[ w2 ].push( dn ) ;
				break ;

			case DF_PGM:
				if ( w4 != "" && w4 != "MACRO" )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				iupper( w2 ) ;
				dn.pgm = true ;
				dn.exp = true ;
				defNames[ w2 ].push( dn ) ;
				break ;

			case DF_CMD:
				if ( w4 != "" && w4 != "MACRO" )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				dn.cmd = true ;
				dn.exp = true ;
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
						set_zedimsgs() ;
						break ;
					}
					else if ( ita->second.top().alias )
					{
						w4 = ita->second.top().name ;
					}
				}
				if ( PrimCMDS.count( w4 ) > 0 )
				{
					w4 = PrimCMDS[ w4 ].truename ;
				}
				itb = defNames.find( w4 ) ;
				if ( itb != defNames.end() && itb->second.top().deactive() )
				{
					pcmd.set_msg( "PEDT015U", 8 ) ;
					set_zedimsgs() ;
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
						pcmd.set_msg( "PEDT015Q", 12 ) ;
						break ;
					}
					( dp == DF_NOP ) ? dn.nop = true : dn.disabled = true ;
					defNames[ w2 ].push( dn ) ;
				}
				else if ( ita->second.top().alias )
				{
					( dp == DF_NOP ) ? dn.nop = true : dn.disabled = true ;
					defNames[ ita->second.top().name ].push( dn ) ;
				}
				else
				{
					( dp == DF_NOP ) ? dn.nop = true : dn.disabled = true ;
					defNames[ w2 ].push( dn ) ;
				}
				break ;

			case DF_RESET:
				if ( ita == defNames.end() )
				{
					pcmd.set_msg( "PEDT015T", 8 ) ;
					set_zedimsgs() ;
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
			//
			// Delete lines from the data being edited.
			//
			{
				int n ;

				Del d( upper( subword( cmd, 2 ) ) ) ;
				if ( d.del_error != "" )
				{
					pcmd.set_msg( d.del_error, d.del_val1, d.del_RC ) ;
					break ;
				}

				check_delete( d ) ;
				if ( d.del_error != "" )
				{
					pcmd.set_msg( d.del_error, d.del_val1, d.del_RC ) ;
					break ;
				}

				if ( !d.del_validCSR )
				{
					type = ( d.del_X )  ? "excluded " :
					       ( d.del_NX ) ? "non-excluded " : "" ;
					pcmd.set_msg( "PEDT011M", 4 ) ;
					break ;
				}

				for ( n = 0, it = d.del_its ; it != d.del_ite && !it->is_bod() ; )
				{
					if ( it->il_deleted || it->is_tod() )
					{
						++it ;
						continue ;
					}
					if ( ( !d.del_X  && !d.del_NX ) ||
					     (  d.del_X  && it->is_excluded() ) ||
					     (  d.del_NX && it->is_not_excluded() ) )
					{
						if ( it->is_valid_file() ) { fileChanged = true ; }
						it = delete_line( it ) ;
						++n ;
					}
					else
					{
						++it ;
					}
				}

				if ( n == 0 )
				{
					type = ( d.del_X )  ? "excluded " :
					       ( d.del_NX ) ? "non-excluded " : "" ;
					pcmd.set_msg( "PEDT011M", 4 ) ;
				}
				else
				{
					lines = d2ds( n ) ;
					pcmd.set_msg( "PEDT011N", 0 ) ;
					rebuildZAREA = true ;
				}
			}
			break ;

	case PC_EXCLUDE:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 String not found.
			// RC =  8 Lines not excluded.
			// RC = 12 Inconsistent parameters.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "ALL" && ws == 2 )
			{
				for_each( data.begin(), data.end(),
					[ this ](iline& a)
					{
						if ( !a.is_tod_or_bod() && !a.il_deleted )
						{
							a.set_excluded( level ) ;
						}
					} ) ;
				topLine      = data.top() ;
				rebuildZAREA = true ;
				break ;
			}
			if ( !setFindChangeExcl( 'X' ) ) { break ; }
			fcx_parms.f_set_nx() ;
			actionFind() ;
			if ( fcx_parms.f_error ) { break ; }
			set_zedimsgs() ;
			if ( fcx_parms.f_occurs > 0 && fcx_parms.f_all() )
			{
				setCursorFind() ;
				fcx_parms.set_exclude_counts() ;
				setExcludedMsg() ;
			}
			else if ( fcx_parms.f_success )
			{
				setCursorFind() ;
				fcx_parms.set_exclude_counts( 1, 1 ) ;
				setExcludedMsg() ;
			}
			else
			{
				fcx_parms.set_exclude_counts( 0, 0 ) ;
				setNotFoundMsg() ;
			}
			set_zedimsgs() ;
			rebuildZAREA = true ;
			break ;

	case PC_FIND:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 String not found.
			// RC = 12 Syntax error.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( !setFindChangeExcl( 'F' ) ) { break ; }
			actionFind() ;
			if ( fcx_parms.f_error || pcmd.msgset() ) { break ; }
			if ( fcx_parms.f_occurs > 0 && fcx_parms.f_all() )
			{
				if ( pcmd.is_seek() )
				{
					fcx_parms.set_seek_counts() ;
				}
				else
				{
					fcx_parms.set_find_counts() ;
				}
				setCursorFind() ;
				setFoundMsg() ;
				fcx_parms.f_set_next() ;
				fcx_parms.f_success = true ;
			}
			else if ( fcx_parms.f_success )
			{
				setCursorFind() ;
				setFoundMsg() ;
				if ( pcmd.is_seek() )
				{
					fcx_parms.set_seek_counts( 1, 1 ) ;
				}
				else
				{
					fcx_parms.set_find_counts( 1, 1 ) ;
				}
			}
			else
			{
				setNotFoundMsg() ;
				if ( pcmd.is_seek() )
				{
					fcx_parms.set_seek_counts( 0, 0 ) ;
				}
				else
				{
					fcx_parms.set_find_counts( 0, 0 ) ;
				}
			}
			set_zedimsgs() ;
			rebuildZAREA = true ;
			break ;

	case PC_FLIP:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			wall = upper( cmd ) ;
			for ( j = 0, i = 2 ; i <= ws ; ++i )
			{
				w = word( wall, i ) ;
				if ( w.front() != '.' )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					return ;
				}
				( ++j == 1 ) ? lab1 = w : lab2 = w ;
			}
			if ( j == 0 )
			{
				its = data.begin() ;
				ite = data.end()   ;
			}
			else if ( j == 1 )
			{
				its = getLabelIterator( lab1, rc ) ;
				if ( rc != 0 )
				{
					pcmd.set_msg( "PEDT011F", 20 ) ;
					return ;
				}
				ite = its ;
				++ite ;
			}
			else if ( j == 2 )
			{
				its = getLabelIterator( lab1, rc ) ;
				if ( rc != 0 )
				{
					pcmd.set_msg( "PEDT011F", 20 ) ;
					break ;
				}
				ite = getLabelIterator( lab2, rc ) ;
				if ( rc != 0 )
				{
					pcmd.set_msg( "PEDT011F", 20 ) ;
					break ;
				}
				if ( data.gt( its, ite ) )
				{
					swap( its, ite ) ;
				}
				++ite ;
			}
			for_each( its, ite,
				[ this ](iline& a)
				{
					if ( !a.is_tod_or_bod() && !a.il_deleted )
					{
						a.toggle_excluded( level ) ;
					}
				} ) ;
			rebuildZAREA = true ;
			break ;

	case PC_HEX:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "OFF" )
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT015D", 20 ) ; break ; }
				profHex  = false ;
				profVert = true  ;
				for_each( data.begin(), data.end(),
					[](iline& a)
					{
						a.il_hex = false ;
					} ) ;
				rebuildZAREA = true ;
			}
			else
			{
				if ( w2 == "ON" )  { w2 = w3 ; }
				else if ( ws > 2 ) { pcmd.set_msg( "PEDT015D", 20 ) ; break ; }
				if ( w2 == "VERT" || w2 == "" )
				{
					profVert = true ;
				}
				else if ( w2 == "DATA" )
				{
					profVert = false ;
				}
				else { pcmd.set_msg( "PEDT011", 20 ) ; break ; }
				profHex      = true ;
				rebuildZAREA = true ;
			}
			store_status( SS_HEX ) ;
			buildProfLines( Prof ) ;
			updateProfLines( Prof ) ;
			break ;

	case PC_HIDE:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 HIDE X not supported.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			w2 = get_truename( w2 ) ;
			if ( w2 != "X" )
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			hideExcl     = true ;
			rebuildZAREA = true ;
			break ;

	case PC_HILITE:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  8 LOGIC or SEARCH not supported in the current environment.
			// RC = 12 HILITE dialogue invalid from an edit macro.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "" )
			{
				if ( macroRunning )
				{
					pcmd.set_msg( "PEDM012T", 12 ) ;
					break ;
				}
				vreplace( "ZPROFLG", profLang ) ;
				vreplace( "ZPROFHI", YesNo[ profHilight ] ) ;
				vreplace( "ZPROFIF", YesNo[ profIfLogic ] ) ;
				vreplace( "ZPROFDO", YesNo[ profDoLogic ] ) ;
				vreplace( "ZPARMTC", profParen      ? "/" : ""  ) ;
				vreplace( "ZHLFIND", profFindPhrase ? "/" : ""  ) ;
				vreplace( "ZHLCURS", profCsrPhrase  ? "/" : ""  ) ;
				save_changes = false ;
				addpop( "", 5, 5 ) ;
				while ( true )
				{
					display( "PEDIT01A" ) ;
					rc1 = RC ;
					vcopy( "ZCMD1", t1 ) ;
					if ( t1 == "EDCCLR" )
					{
						vcopy( "ZCCPAN", t1 ) ;
						addpop( "", -1, -2 ) ;
						while ( true )
						{
							display( t1 ) ;
							if ( RC > 0 ) { break ; }
						}
						rempop() ;
						continue ;
					}
					else if ( word( t1, 1 ) == "LANG" )
					{
						save_changes |= set_language_colours( word( t1, 2 ) ) ;
					}
					if ( rc1 == 0 ) { continue ; }
					vget( "ZVERB", SHARED ) ;
					if ( zverb != "CANCEL" )
					{
						vcopy( "ZPROFLG", profLang, MOVE ) ;
						vcopy( "ZPROFHI", t1, MOVE )  ;
						profHilight = ( t1 == "YES" ) ;
						vcopy( "ZPROFIF", t1, MOVE )  ;
						profIfLogic = ( t1 == "YES" ) ;
						vcopy( "ZPROFDO", t1, MOVE )  ;
						profDoLogic = ( t1 == "YES" ) ;
						vcopy( "ZPARMTC", t1, MOVE )  ;
						profParen   = ( t1 == "/" )   ;
						vcopy( "ZHLFIND", t1, MOVE )  ;
						profFindPhrase = ( t1 == "/" ) ;
						vcopy( "ZHLCURS", t1, MOVE )   ;
						profCsrPhrase = ( t1 == "/" )  ;
						optFindPhrase = profFindPhrase ;
						buildProfLines( Prof )  ;
						updateProfLines( Prof ) ;
						detLang = ( profLang != "AUTO" ) ? profLang : determineLang() ;
						set_language_fvars( detLang ) ;
						clr_hilight_shadow() ;
						rebuildZAREA   = true ;
					}
					store_status( SS_HILITE ) ;
					break ;
				}
				if ( save_changes )
				{
					save_language_colours() ;
				}
				rempop() ;
				set_zhivars() ;
				break ;
			}
			if ( w2 == "RESET" )
			{
				if ( ws > 2 )
				{
					pcmd.set_msg( "PEDT011", 12, 20 ) ;
					break ;
				}
				profIfLogic  = false  ;
				profDoLogic  = false  ;
				profLang     = "AUTO" ;
				detLang      = determineLang() ;
				set_language_fvars( detLang ) ;
				rebuildZAREA = true     ;
				buildProfLines( Prof )  ;
				updateProfLines( Prof ) ;
				store_status( SS_HILITE ) ;
				set_zhivars() ;
				break ;
			}
			iupper( cmd ) ;
			hi_paren  = parseString2( cmd, "PAREN" ) ;
			hi_cursor = parseString2( cmd, "CURSOR" ) ;
			hi_find   = parseString2( cmd, "FIND" ) ;
			ws = words( cmd )   ;
			w2 = word( cmd, 2 ) ;
			w3 = word( cmd, 3 ) ;
			if ( aliasNames.find( w2 ) != aliasNames.end() )
			{
				w2 = aliasNames[ w2 ] ;
			}
			if ( findword( w2, "ON OFF LOGIC NOLOGIC IFLOGIC DOLOGIC SEARCH" ) )
			{
				if ( ws > 3 )
				{
					pcmd.set_msg( "PEDT011", 12, 20 ) ;
					break ;
				}
				if ( w3 != "" && w3 != "AUTO" && !addHilight( w3 ) )
				{
					pcmd.set_msg( "PEDT013Q", 12, 20 ) ;
					break ;
				}
			}
			else
			{
				if ( ws > 2 )
				{
					pcmd.set_msg( "PEDT011", 12, 20 ) ;
					break ;
				}
				if ( w2 != "" && w2 != "AUTO" && !addHilight( w2 ) )
				{
					pcmd.set_msg( "PEDT013Q", 12, 20 ) ;
					break ;
				}
				w3 = w2   ;
				w2 = "ON" ;
			}
			if ( w2 == "OFF" )
			{
				profHilight  = false ;
				if ( hi_paren  ) { profParen      = !profParen      ; }
				if ( hi_cursor ) { profCsrPhrase  = !profCsrPhrase  ; }
				if ( hi_find   ) { profFindPhrase = !profFindPhrase ; }
				rebuildZAREA = true  ;
				buildProfLines( Prof )  ;
				updateProfLines( Prof ) ;
				store_status( SS_HILITE ) ;
				set_zhivars() ;
				break ;
			}
			if ( w2 == "SEARCH" )
			{
				if ( profHilight && !hlight.hl_abend )
				{
					clr_hilight_shadow() ;
					fill_hilight_shadow() ;
					if ( misADDR )
					{
						topLine = misADDR ;
						rebuildZAREA = true ;
					}
				}
				else
				{
					pcmd.setRC( 8 ) ;
				}
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
			set_language_fvars( detLang ) ;
			clr_hilight_shadow() ;
			if ( hi_paren  ) { profParen      = !profParen      ; }
			if ( hi_cursor ) { profCsrPhrase  = !profCsrPhrase  ; }
			if ( hi_find   ) { profFindPhrase = !profFindPhrase ; }
			profHilight  = true  ;
			rebuildZAREA = true  ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			store_status( SS_HILITE ) ;
			set_zhivars() ;
			break ;

	case PC_LEVEL:
			if ( !datatype( w2, 'W' ) )
			{
				pcmd.set_msg( "PEDT018F", 12 ) ;
				break ;
			}
			i = ds2d( w2 ) ;
			if ( i > 99 )
			{
				pcmd.set_msg( "PEDT018F", 12 ) ;
				break ;
			}
			if ( !profStats )
			{
				pcmd.set_msg( "PEDT018G", 4 ) ;
				break ;
			}
			lnummod = d2ds( i, 2 ) ;
			break ;

	case PC_LOCATE:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Line not located.
			// RC =  8 Empty file.
			// RC = 20 Severe error (including label does not exist).
			//
			pcmd.actioned() ;
			if ( macroRunning && !getFileLineZFIRST() )
			{
				pcmd.setRC( 8 ) ;
				break ;
			}
			tTop = topLine ;
			if ( ws == 2 )
			{
				if ( w2.front() == '.' )
				{
					it = getLabelIterator( w2, rc1 ) ;
					if ( rc1 < 0 )
					{
						pcmd.set_msg( "PEDT011F", 4 ) ;
						break ;
					}
					it->set_unexcluded( level ) ;
					topLine      = itr2ptr( it ) ;
					ptopLine     = tTop ;
					rebuildZAREA = true ;
					break ;
				}
				else if ( isnumeric( w2 ) )
				{
					if ( profNum )
					{
						if ( profNumSTD )
						{
							if ( w2.size() > 8 )
							{
								pcmd.set_msg( "PEDT011", 20 ) ;
								break ;
							}
							topLine  = locateSTDlinenum( tTop, w2 ) ;
							ptopLine = tTop ;
							topLine->set_unexcluded( level ) ;
							rebuildZAREA = true ;
						}
						else
						{
							if ( w2.size() > 6 )
							{
								pcmd.set_msg( "PEDT011", 20 ) ;
								break ;
							}
							topLine  = locateCBLlinenum( tTop, w2 ) ;
							ptopLine = tTop ;
							topLine->set_unexcluded( level ) ;
							rebuildZAREA = true ;
						}
						break ;
					}
					else if ( w2.size() <= 8 )
					{
						if ( w2 == "0" )
						{
							topLine = data.top() ;
						}
						else
						{
							topLine = itr2ptr( getDataLine( ds2d( w2 ) ) ) ;
							topLine->set_unexcluded( level ) ;
						}
						ptopLine     = tTop ;
						rebuildZAREA = true ;
						break ;
					}
				}
				else if ( w2 == "*" )
				{
					if ( ptopLine && ptopLine != data.bottom() )
					{
						topLine  = ptopLine ;
						ptopLine = tTop ;
						topLine->set_unexcluded( level ) ;
						rebuildZAREA = true ;
					}
					else
					{
						pcmd.set_msg( "PEDT014V", 4 ) ;
					}
					break ;
				}
				else if ( w2 == "MOD" )
				{
					if ( modLine )
					{
						moveTopline( modLine, true ) ;
						ptopLine = tTop ;
						modLine->set_unexcluded( level ) ;
						rebuildZAREA = true ;
					}
					else
					{
						pcmd.set_msg( "PEDT014V", 4 ) ;
					}
					break ;
				}
			}
			dir = ' ' ;
			w1  = ""  ;
			for ( j = 0, i = 2 ; i <= ws ; ++i )
			{
				w = upper( word( cmd, i ) ) ;
				if ( w.front() == '.' )
				{
					( ++j == 1 ) ? lab1 = w : lab2 = w ;
				}
				else
				{
					if ( findword( w, "FIRST LAST NEXT PREV" ) )
					{
						if ( dir != ' ' )
						{
							pcmd.set_msg( "PEDT011", 20 ) ;
							break ;
						}
						dir = w.front() ;
					}
					else if ( w1 != "" )
					{
						pcmd.set_msg( "PEDT011", 20 ) ;
						break ;
					}
					else
					{
						w1 = w ;
					}
				}
			}
			if ( pcmd.error() ) { break ; }
			if ( j == 0 )
			{
				it1 = data.top() ;
				it2 = data.bottom() ;
			}
			else if ( j == 2 )
			{
				it1 = getLabelIterator( lab1, rc1 ) ;
				it2 = getLabelIterator( lab2, rc2 ) ;
				if ( rc1 < 0 || rc2 < 0 )
				{
					pcmd.set_msg( "PEDT011F", 20 ) ;
					break ;
				}
				if ( data.gt( it1, it2 ) )
				{
					swap( it1, it2 ) ;
				}
			}
			else
			{
				pcmd.set_msg( "PEDT014U", 20 ) ;
				break ;
			}
			itl = locList.find( get_truename( w1 ) ) ;
			if ( itl != locList.end() )
			{
				topLine = getNextSpecial( tTop, it1, it2, dir, itl->second ) ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			if ( w1 != "X" && w1 != "CMD" )
			{
				topLine->set_unexcluded( level ) ;
			}
			rebuildZAREA = true ;
			ptopLine     = tTop ;
			break ;

	case PC_IMACRO:
			//
			// Set the Initial Macro.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 IMACRO set not accepted - profile is locked.
			// RC = 12 Invalid name specified.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( profLock )
			{
				pcmd.set_msg( "PEDT015H", 4 ) ;
			}
			else if ( !isvalidName( w2 ) )
			{
				pcmd.set_msg( "PEDT011", 12 ) ;
			}
			else
			{
				profIMACRO = w2 ;
				buildProfLines( Prof )  ;
				updateProfLines( Prof ) ;
				store_status( SS_IMACRO ) ;
			}
			break ;

	case PC_NONUMBER:
			//
			// Turn off number mode without removing line numbers.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			profNum    = false ;
			profNumSTD = false ;
			profNumCBL = false ;
			update_status() ;
			if ( !LeftBndSet )
			{
				LeftBnd = 1 ;
			}
			if ( !RightBndSet )
			{
				RightBnd = ( reclen == 0 ) ? 0 : reclen ;
			}
			if ( startCol == ( lnumSize1 + 1 ) )
			{
				startCol = 1 ;
			}
			removeSpecial( CA_NUM_ON, CA_NUM_CBL_ON, CA_NUM_STD_ON ) ;
			lnumSize1 = 0 ;
			lnumSize2 = 0 ;
			lnumS2pos = 0 ;
			store_status( SS_NUMBER ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			rebuildZAREA = true ;
			break ;

	case PC_NOTES:
			//
			// Allow notes.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if      ( w2 == "ON" || ws == 1 ) { profNotes = true  ; }
			else if ( w2 == "OFF" )           { profNotes = false ; }
			else { pcmd.set_msg( "PEDT011", 20 ) ; break ; }
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			store_status( SS_NOTES ) ;
			break ;

	case PC_NULLS:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "OFF" )
			{
				if ( ws > 2 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				profNullA = false ;
				profNulls = false ;
			}
			else
			{
				if ( w2 == "ON" )
				{
					w2 = w3  ;
				}
				else if ( ws > 2 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				if ( w2 == "STD" || w2 == "" )
				{
					profNulls = true  ;
					profNullA = false ;
				}
				else if ( w2 == "ALL" )
				{
					profNulls = true ;
					profNullA = true ;
				}
				else
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
			}
			store_status( SS_NULLS ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			rebuildZAREA = true ;
			break ;

	case PC_NUMBER:
			//
			// Turn on number mode.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 != "OFF" )
			{
				if ( reclen > 0 && reclen < 10 )
				{
					pcmd.set_msg( "PEDT018E", 8 ) ;
					break ;
				}
				if ( w2 != "ON" )
				{
					if ( ws > 4 )
					{
						pcmd.set_msg( "PEDT011", 20 ) ;
						break ;
					}
					w5 = w4 ;
					w4 = w3 ;
					w3 = w2 ;
					++ws ;
				}
				if ( ws == 2 )
				{
					profNum     = true  ;
					profNumDisp = false ;
					if ( !profNumSTD && !profNumCBL )
					{
						if ( ( ( profLang == "AUTO" ) ? detLang : profLang ) == "COBOL" )
						{
							profNumCBL = true ;
						}
						else
						{
							profNumSTD = true ;
						}
					}
				}
				else
				{
					check_number_parm( valid1, klist, w3 ) ;
					if ( pcmd.error() ) { break ; }
					check_number_parm( valid1, klist, w4 ) ;
					if ( pcmd.error() ) { break ; }
					check_number_parm( valid1, klist, w5 ) ;
					if ( pcmd.error() ) { break ; }
					if ( klist.find( "DISPLAY" ) != klist.end() &&
					     klist.find( "NODISPL" ) != klist.end() )
					{
						pcmd.set_msg( "PEDT016R", "DISPLAY", "NODISPL", 12 ) ;
						break ;
					}
					if ( klist.find( "STANDARD" ) != klist.end() &&
					     klist.find( "NOSTD" )    != klist.end() )
					{
						pcmd.set_msg( "PEDT016R", "STANDARD", "NOSTD", 12 ) ;
						break ;
					}
					if ( klist.find( "COBOL" )   != klist.end() &&
					     klist.find( "NOCOBOL" ) != klist.end() )
					{
						pcmd.set_msg( "PEDT016R", "COBOL", "NOCOBOL", 12 ) ;
						break ;
					}
					if ( klist.find( "COBOL" )    != klist.end() &&
					     klist.find( "STANDARD" ) != klist.end() &&
					     reclen == 0 )
					{
						pcmd.set_msg( "PEDT016R", "COBOL", "STANDARD", 12 ) ;
						break ;
					}
					if ( klist.find( "COBOL" ) != klist.end() )
					{
						profNum     = true  ;
						profNumCBL  = true  ;
						profNumDisp = false ;
						if ( reclen == 0 )
						{
							profNumSTD = false ;
						}
						set_num_parameters() ;
					}
					if ( klist.find( "STANDARD" ) != klist.end() )
					{
						profNum     = true  ;
						profNumSTD  = true  ;
						profNumDisp = false ;
						if ( reclen == 0 )
						{
							profNumCBL = false ;
						}
						set_num_parameters() ;
					}
					if ( klist.find( "NOCOBOL" ) != klist.end() )
					{
						if ( profNumCBL )
						{
							profNum    = profNumSTD ;
							profNumCBL = false ;
							set_num_parameters() ;
						}
						profNumDisp = false ;
					}
					if ( klist.find( "NOSTD" ) != klist.end() )
					{
						if ( profNumSTD )
						{
							profNum    = profNumCBL ;
							profNumSTD = false ;
							set_num_parameters() ;
						}
						profNumDisp = false ;
					}
					if ( profNum )
					{
						if ( klist.find( "DISPLAY" ) != klist.end() )
						{
							profNumDisp = true ;
						}
						else if ( klist.find( "NODISPL" ) != klist.end() )
						{
							profNumDisp = false ;
						}
					}
				}
				if ( profNum )
				{
					update_status() ;
					set_num_parameters() ;
					add_missing_line_numbers() ;
					if ( !LeftBndSet || LeftBnd <= lnumSize1 )
					{
						LeftBnd = lnumSize1 + 1 ;
					}
					if ( lnumSize2 > 0 && ( !RightBndSet || RightBnd > lnumS2pos ) )
					{
						RightBnd = lnumS2pos ;
					}
					if ( !profNumDisp && startCol <= lnumSize1 )
					{
						startCol = lnumSize1 + 1 ;
					}
					else if ( profNumDisp && startCol <= ( lnumSize1 + 1 ) )
					{
						startCol = 1 ;
					}
				}
				else
				{
					update_status() ;
					if ( !LeftBndSet )
					{
						LeftBnd = 1 ;
					}
					if ( !RightBndSet )
					{
						RightBnd = ( reclen == 0 ) ? 0 : reclen ;
					}
				}
				removeSpecial( ( profNum )    ? CA_NUM_ON     : CA_NUM_OFF,
					       ( profNumCBL ) ? CA_NUM_CBL_ON : CA_NUM_CBL_OFF,
					       ( profNumSTD ) ? CA_NUM_STD_ON : CA_NUM_STD_OFF ) ;
			}
			else if ( ws == 2 )
			{
				update_status() ;
				profNum   = false ;
				lnumSize1 = 0 ;
				lnumSize2 = 0 ;
				lnumS2pos = 0 ;
				if ( !LeftBndSet )
				{
					LeftBnd = 1 ;
				}
				if ( !RightBndSet )
				{
					RightBnd = ( reclen == 0 ) ? 0 : reclen ;
				}
				removeSpecial( CA_NUM_OFF ) ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			store_status( SS_NUMBER ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			rebuildZAREA = true ;
			break ;

	case PC_PRESERVE:
			pcmd.actioned() ;
			if ( w2 == "ON" || w2 == "" )
			{
				optPreserve = true ;
			}
			else if ( w2 == "OFF" )
			{
				optPreserve = false ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 12 ) ;
				break ;
			}
			break ;

	case PC_PROFILE:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			ii = -1 ;
			if ( ws == 3 && datatype( w2, 'W' ) )
			{
				wl = word( cmd, 2 ) ;
				swap( w2, w3 ) ;
			}
			else
			{
				wl = word( cmd, ws ) ;
			}
			if ( ( ws == 2 || ws == 3 ) &&
			     wl.size() < 9     &&
			     isnumeric( wl )   &&
			     !findword( w2, "LOCK UNLOCK USE DEL DELETE RESET" ) )
			{
				--ws ;
				ii = ds2d( wl ) ;
			}
			if ( ws == 3 && w2 == "USE" && w3 == "TYPE" )
			{
				swapEditProfile( determineLang(), Cautions ) ;
				zedproft    = "Y" ;
				explProfile = false ;
				vput( "ZEDPROFT", PROFILE ) ;
				clr_hilight_shadow()       ;
				store_status( SS_PROFILE ) ;
				set_zhivars() ;
			}
			else if ( ws > 2 )
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			else if ( w2 == "LOCK" )
			{
				saveEditProfile( zedprof ) ;
				profLock = true ;
				store_status( SS_PROFILE ) ;
			}
			else if ( w2 == "UNLOCK" )
			{
				profLock = false ;
				store_status( SS_PROFILE ) ;
			}
			else if ( w2 == "RESET" )
			{
				resetEditProfile() ;
			}
			else if ( isvalidName( w2 ) )
			{
				swapEditProfile( w2, Cautions ) ;
				explProfile = true ;
				zedproft    = "N" ;
				vput( "ZEDPROFT", PROFILE) ;
				clr_hilight_shadow() ;
				store_status( SS_PROFILE ) ;
			}
			else if ( ws == 2 )
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				break ;
			}
			removeProfLines() ;
			buildProfLines( Prof, ii ) ;
			if ( topLine == data.bottom() ) { idecr( topLine ) ; }
			it = addSpecial( LN_PROF, topLine, Prof ) ;
			if ( ii > 5 || ( show_tabs = ( ii == -1 && tabsLine != "" ) ) )
			{
				it = data.insert( ++it, new iline( taskid(), LN_TABS ), tabsLine, level ) ;
				it->il_prof = true ;
			}
			if ( ii > 6 || ( show_mask = ( ii == -1 && strip( maskLine ) != "" ) ) )
			{
				it = data.insert( ++it, new iline( taskid(), LN_MASK ), getMaskLine1(), level ) ;
				it->il_prof = true ;
			}
			if ( ii > 7 || ( show_bnds = ( ii == -1 && ( LeftBndSet || RightBndSet ) ) ) )
			{
				it = data.insert( ++it, new iline( taskid(), LN_BNDS ), "", level ) ;
				it->il_prof = true ;
			}
			if ( ii > 8 || show_tabs || show_mask || show_bnds )
			{
				it = data.insert( ++it, new iline( taskid(), LN_COL ), "", level ) ;
				it->il_prof = true ;
			}

			it = addSpecial( LN_MSG, it, Msgs ) ;
			addSpecial( it, Cautions ) ;

			rebuildZAREA = true ;
			if ( topLine != data.begin() ) { iincr( topLine ) ; }
			break ;

	case PC_RECOVERY:
			//
			// Turn on recovery.  Starts background recovery task.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "OFF" )
			{
				if ( ws > 2 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				profRecovery = false ;
				recovSusp    = false ;
				if ( recvStatus == RECV_RUNNING )
				{
					recvStatus = RECV_STOPPING ;
					cond_recov.notify_all() ;
					removeEditRecovery() ;
					while ( recvStatus != RECV_STOPPED )
					{
						boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
					}
					delete bThread ;
					bThread = nullptr ;
				}
			}
			else
			{
				if ( ws > 4 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				if ( w2 == "ON" )
				{
					w2 = w3 ;
					w3 = word( cmd, 4 ) ;
				}
				else if ( ws > 3 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				else
				{
					w3 = word( cmd, 3 ) ;
				}
				if ( w2 == "" )
				{
					profRecovery = true ;
				}
				else
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
			}
			if ( profRecovery && recvStatus != RECV_RUNNING )
			{
				recovSusp = false ;
				startRecoveryTask() ;
			}
			buildProfLines( Prof ) ;
			updateProfLines( Prof ) ;
			break ;

	case PC_RENUM:
			//
			// Renum line numbers (STD and/or COBOL)
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 != "OFF" && reclen > 0 && reclen < 10 )
			{
				pcmd.set_msg( "PEDT018E", 20 ) ;
				break ;
			}
			if ( w2 != "ON" )
			{
				if ( ws > 3 )
				{
					pcmd.set_msg( "PEDT011", 20 ) ;
					break ;
				}
				w4 = w3 ;
				w3 = w2 ;
				++ws ;
			}
			if ( ws == 2 )
			{
				profNum     = true  ;
				profNumDisp = false ;
				if ( !profNumSTD && !profNumCBL )
				{
					if ( detLang == "COBOL" )
					{
						profNumCBL = true ;
					}
					else
					{
						profNumSTD = true ;
					}
				}
			}
			else
			{
				check_number_parm( valid2, klist, w3 ) ;
				if ( pcmd.error() ) { break ; }
				check_number_parm( valid2, klist, w4 ) ;
				if ( pcmd.error() ) { break ; }
				check_number_parm( valid2, klist, w5 ) ;
				if ( pcmd.error() ) { break ; }
				if ( klist.find( "COBOL" )    != klist.end() &&
				     klist.find( "STANDARD" ) != klist.end() &&
				     reclen == 0 )
				{
					pcmd.set_msg( "PEDT016R", "COBOL", "STANDARD", 20 ) ;
					break ;
				}
				if ( klist.find( "COBOL" ) != klist.end() )
				{
					profNum     = true  ;
					profNumCBL  = true  ;
					profNumDisp = false ;
					if ( reclen == 0 )
					{
						profNumSTD = false ;
					}
					if ( klist.find( "STANDARD" ) == klist.end() )
					{
						profNumSTD = false ;
					}
				}
				if ( klist.find( "STANDARD" ) != klist.end() )
				{
					profNum     = true  ;
					profNumSTD  = true  ;
					profNumDisp = false ;
					if ( reclen == 0 )
					{
						profNumCBL = false ;
					}
					if ( klist.find( "COBOL" ) == klist.end() )
					{
						profNumCBL = false ;
					}
				}
				if ( profNum )
				{
					if ( klist.find( "DISPLAY" ) != klist.end() )
					{
						profNumDisp = true ;
					}
				}
			}
			renum_data() ;
			update_status() ;
			if ( !profNumDisp && startCol <= lnumSize1 )
			{
				startCol = lnumSize1 + 1 ;
			}
			else if ( profNumDisp && startCol <= ( lnumSize1 + 1 ) )
			{
				startCol = 1 ;
			}
			if ( !LeftBndSet || LeftBnd <= lnumSize1 )
			{
				LeftBnd = lnumSize1 + 1 ;
			}
			if ( lnumSize2 > 0 && ( !RightBndSet || RightBnd > lnumS2pos ) )
			{
				RightBnd = lnumS2pos ;
			}
			removeSpecial( ( profNum )    ? CA_NUM_ON     : CA_NUM_OFF,
				       ( profNumCBL ) ? CA_NUM_CBL_ON : CA_NUM_CBL_OFF,
				       ( profNumSTD ) ? CA_NUM_STD_ON : CA_NUM_STD_OFF ) ;
			store_status( SS_NUMBER ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			rebuildZAREA = true ;
			break ;

	case PC_SAVE:
			//
			// Save data changes.
			//
			if ( zedvmode )
			{
				  viewWarning1() ;
				  pcmd.cond_reset() ;
				  break ;
			}
			if ( !saveFile() ) { break                         ; }
			if ( termEdit )    { ZRESULT = "PEDT011P"          ; }
			else               { pcmd.set_msg( "PEDT011P", 4 ) ; }
			break ;

	case PC_SETUNDO:
			pcmd.actioned() ;
			if ( w2 == "ON" || w2 == "KEEP" )
			{
				profUndoOn   = true ;
				profUndoKeep = ( w2 == "KEEP" ) ;
				if ( !fileChanged )
				{
					saveLevel = iline::get_Global_File_level( taskid() ) ;
				}
				removeSpecial( CA_SETU_OFF ) ;
			}
			else if ( w2 == "OFF" )
			{
				profUndoOn   = false ;
				pcmd.set_msg( "PEDT011U", 12 ) ;
				removeRecoveryData() ;
				saveLevel    = -1    ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 12 ) ;
				break ;
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_SORT:
			pcmd.actioned() ;
			sort_data( cmd ) ;
			break ;

	case PC_STATS:
			//
			// Turn on STATS.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "ON" || ws == 1 )
			{
				profStats = true ;
			}
			else if ( w2 == "OFF" )
			{
				profStats = false ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 12 ) ;
				break ;
			}
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			store_status( SS_STATS ) ;
			break ;

	case PC_TABS:
			pcmd.actioned() ;
			if ( w2 == "OFF" )
			{
				if ( ws > 2 ) { pcmd.set_msg( "PEDT011", 12 ) ; break ; }
				profTabs  = false ;
				profATabs = false ;
				tabsChar  = ' ' ;
			}
			else
			{
				if ( ws > 3 )
				{
					pcmd.set_msg( "PEDT011", 12 ) ;
					break ;
				}
				if ( w2 == "ON" )  { w2 = w3                               ; }
				else if ( ws > 2 ) { pcmd.set_msg( "PEDT011", 12 ) ; break ; }
				if ( w2 == "STD" || w2 == "" )
				{
					profTabs  = true  ;
					profATabs = false ;
					tabsChar  = ' ' ;
				}
				else if ( w2 == "ALL" )
				{
					profTabs  = true ;
					profATabs = true ;
					tabsChar  = ' ' ;
				}
				else if ( w2.size() == 1 )
				{
					profTabs  = true ;
					profATabs = false ;
					tabsChar = w2.front() ;
				}
				else if ( w2.size() == 3 && w2[ 0 ] == w2[ 2 ] && ( w2[ 0 ] == '"' || w2[ 0 ] == '\'' ) )
				{
					profTabs  = true ;
					profATabs = false ;
					tabsChar = w2[ 1 ] ;
				}
				else
				{
					pcmd.set_msg( "PEDT011", 12 ) ;
					break ;
				}
			}
			store_status( SS_TABS ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;

	case PC_UNNUMBER:
			//
			// Turn off number mode and remove line numbers.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Number mode not on.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( !profNum )
			{
				pcmd.set_msg( "PEDT016T", 12 ) ;
				break ;
			}
			unnum_data() ;
			update_status() ;
			profNum    = false ;
			profNumSTD = false ;
			profNumCBL = false ;
			if ( !LeftBndSet )
			{
				LeftBnd = 1 ;
			}
			if ( !RightBndSet )
			{
				RightBnd = ( reclen == 0 ) ? 0 : reclen ;
			}
			removeSpecial( CA_NUM_OFF ) ;
			lnumSize1 = 0 ;
			lnumSize2 = 0 ;
			lnumS2pos = 0 ;
			startCol  = 1 ;
			store_status( SS_NUMBER ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			rebuildZAREA = true ;
			break ;

	case PC_XTABS:
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Tabs set on when no tabs found in file.
			//         or off when tabs where found in file.
			// RC =  8 Tab size changed.  Change is PENDING.
			// RC = 12 Invalid parameter.
			// RC = 20 Severe error.
			//
			pcmd.actioned() ;
			if ( w2 == "ON" )
			{
				if ( reclen > 0 )
				{
					pcmd.set_msg( "PEDT018D", 8 ) ;
				}
				else
				{
					profXTabs = true ;
					if ( !tabsOnRead )
					{
						pcmd.set_msg( "PEDT017G", 4 ) ;
					}
					removeSpecial( CA_XTABS_ON ) ;
				}
			}
			else if ( w2 == "OFF" )
			{
				profXTabs = false ;
				if ( tabsOnRead )
				{
					pcmd.set_msg( "PEDT017H", 4 ) ;
				}
				removeSpecial( CA_XTABS_OFF ) ;
			}
			else if ( isnumeric( w2 ) && w2.size() < 3 )
			{
				XTabz = ds2d( w2 ) ;
				if ( XTabz == 0 ) { XTabz = 1 ; }
				if ( profXTabz == XTabz )
				{
					buildProfLines( Prof )  ;
					updateProfLines( Prof ) ;
					break ;
				}
				pcmd.set_msg( "PEDT017I", 8 ) ;
				if ( topLine == data.bottom() ) { idecr( topLine ) ; }
				addSpecial( LN_MSG, topLine, Msgs ) ;
			}
			else
			{
				pcmd.set_msg( "PEDT011", 12 ) ;
				break ;
			}
			store_status( SS_XTABS ) ;
			buildProfLines( Prof )  ;
			updateProfLines( Prof ) ;
			break ;
	}
}


void pedit01::actionLineCommands()
{
	//
	// For each line in the command vector, action the line command.
	// If there have been any delete commands (including moves), clear all shadow lines to force a rebuild.
	//

	TRACE_FUNCTION() ;

	bool csrPlaced1 = false ;
	bool csrPlaced2 = false ;

	if ( !storeLineCommands() )
	{
		return ;
	}

	if ( lcmds.size() == 0 ) { return ; }

	cursor.clear() ;

	for_each( data.begin(), data.end(),
		[](iline& a)
		{
			a.clearLcc() ;
		} ) ;

	for ( auto& lc : lcmds )
	{
		actionLineCommand( lc, csrPlaced1, csrPlaced2 ) ;
		lc.lcmd_procd = true ;
	}

	if ( any_of( lcmds.begin(), lcmds.end(),
		[](lcmd& a)
		{
			return a.lcmd_cmdstr.front() == 'M' || a.lcmd_cmdstr.front() == 'D' ;
		} ) )
	{
		clr_hilight_shadow() ;
	}

	lcmds.clear() ;
}


void pedit01::actionLineCommand( lcmd& lc,
				 bool& csrPlaced1,
				 bool& csrPlaced2 )
{
	//
	// Action the line command.
	// For copy/move/repeat preserve flags: file, note, prof, col, excl, hex.
	//
	// Treat an excluded block as though it were just one line for the purposes of the Rpt value.
	//

	TRACE_FUNCTION() ;

	int i ;
	int l ;
	int j ;
	int k ;

	int defCol ;

	uint vis ;

	uint32_t status ;

	size_t p1 ;

	bool overlayOK ;
	bool splitOK   ;
	bool shiftOK   ;

	string tmp1   ;
	string tmp2   ;
	string inLine ;
	string fname  ;

	std::ifstream fin ;

	vector<ipline> vip ;
	ipline ip ;

	vector<string> vst ;

	iline* p_iline ;
	iline* t_iline ;

	vector<string> tdata ;

	vector<ipline>::iterator new_end ;

	Data::Iterator it  = nullptr ;
	Data::Iterator its = nullptr ;
	Data::Iterator ite = nullptr ;

	map<string, lmac>::iterator itx ;

	if ( lc.lcmd_cmd != LC_TFLOW && lc.lcmd_Rpt == -1 )
	{
		lc.lcmd_Rpt = ( findword( lc.lcmd_cmdstr, "( (( )) ) < << >> >" ) ) ? 2 : 1 ;
	}

	switch ( lc.lcmd_cmd )
	{
	case LC_BOUNDS:
		it = data.insert( lc.lcmd_sADDR, new iline( taskid(), LN_BNDS ), "", level ) ;
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( itr2ptr( it ), CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		if ( lc.lcmd_sADDR == topLine )
		{
			topLine = itr2ptr( it ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_COLS:
		it = data.insert( lc.lcmd_sADDR, new iline( taskid(), LN_COL ), "", level ) ;
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		if ( lc.lcmd_sADDR == topLine )
		{
			topLine = itr2ptr( it ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_C:
	case LC_M:
	case LC_CC:
	case LC_MM:
		status = ( lc.lcmd_cmdstr.front() == 'M' ) ? ID_MOVEC : ID_COPYC ;
		vip.clear() ;
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		overlayOK = true ;
		if ( lc.lcmd_cut )
		{
			cut.iterators( its, ++ite ) ;
			create_copy( cut, vip ) ;
			clipBoard  = cut.clipBoard() ;
			cutReplace = cut.repl() ;
		}
		else
		{
			for ( ++ite ; its != ite ; ++its )
			{
				if ( its->il_deleted ) { continue ; }
				copyPrefix( ip, its ) ;
				ip.ip_data = its->get_idata() ;
				vip.push_back( ip ) ;
			}
		}
		if ( lc.lcmd_cut )
		{
			if ( !copyToClipboard( vip ) ) { break ; }
		}
		else if ( lc.extdata.is_create() )
		{
			if ( lc.extdata.name() == "" )
			{
				fname = displayPanel( "PEDIT01J", "ZFILE2", "PEDT014R", true, false ) ;
				if ( RC == 8 ) { break ; }
			}
			else
			{
				fname = lc.extdata.name() ;
			}
			if ( exists( fname ) )
			{
				pcmd.set_msg( "PEDT014R", 12 ) ;
				break ;
			}
			create_file( fname, lc.lcmd_sADDR, lc.lcmd_eADDR ) ;
		}
		else if ( lc.extdata.is_replace() )
		{
			if ( lc.extdata.name() == "" )
			{
				fname = displayPanel( "PEDIT01I", "ZFILE2", "", false ) ;
				if ( RC == 8 ) { break ; }
			}
			else
			{
				fname = lc.extdata.name() ;
			}
			if ( exists( fname ) && !confirmReplace( fname ) )
			{
				break ;
			}
			create_file( fname, lc.lcmd_sADDR, lc.lcmd_eADDR ) ;
		}
		else if ( lc.lcmd_ABOW == 'O' )
		{
			new_end = remove_if( vip.begin(), vip.end(),
				[](const ipline& a)
				{
					return !a.is_file() ;
				} ) ;
			vip.erase( new_end, vip.end() ) ;
			if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_dADDR ) )
			{
				cursor.set( lc.lcmd_dADDR, CSR_FC_LCMD ) ;
			}
			j   = 0 ;
			k   = 0 ;
			its = lc.lcmd_dADDR ;
			ite = lc.lcmd_lADDR ;
			for ( ++ite ; its != ite ; ++its )
			{
				if ( its->not_valid_file() ) { continue ; }
				tmp1 = overlay1( vip[ j ].ip_data, its->get_idata(), overlayOK ) ;
				its->put_idata( tmp1, level, ID_CHNGO ) ;
				++j ;
				if ( j == vip.size() ) { j = 0 ; ++k ; }
				fileChanged = true ;
			}
			if ( k == 0 && j < vip.size() )
			{
				pcmd.set_msg( "PEDT012C", 12 ) ;
				for ( ; j < vip.size() ; ++j )
				{
					idecr( lc.lcmd_eADDR ) ;
				}
			}
		}
		else
		{
			ite = lc.lcmd_dADDR ;
			if ( lc.lcmd_ABOW == 'B' ) { --ite ; }
			if ( lc.lcmd_ABRpt == -1 ) { lc.lcmd_ABRpt = 1 ; }
			for ( i = 0 ; i < lc.lcmd_ABRpt ; ++i )
			{
				for ( j = 0 ; j < vip.size() ; ++j )
				{
					ite = data.insert( ++ite, new iline( taskid() ) ) ;
					const ipline& ipl = vip[ j ] ;
					if ( lc.lcmd_cmd == LC_M || lc.lcmd_cmd == LC_MM )
					{
						copyPrefix( ite, ipl, true ) ;
					}
					else
					{
						copyPrefix( ite, ipl ) ;
					}
					ite->put_idata( ipl.ip_data, level, ( ipl.ip_status | status ) ) ;
					if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == ite ) )
					{
						cursor.set( itr2ptr( ite ), CSR_FC_LCMD ) ;
						csrPlaced2 = true ;
					}
					if ( ite->is_file() ) { fileChanged = true ; }
				}
			}
		}
		if ( overlayOK && ( lc.lcmd_cmd == LC_M || lc.lcmd_cmd == LC_MM ) )
		{
			its = lc.lcmd_sADDR ;
			ite = lc.lcmd_eADDR ;
			for ( ++ite ; its != ite ; )
			{
				if ( its->is_file() ) { fileChanged = true ; }
				its = delete_line( its ) ;
			}
		}
		if ( !overlayOK && lc.lcmd_cmdstr.front() == 'M' )
		{
			pcmd.set_msg( "PEDT012D", 12 ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_D:
	case LC_DD:
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		++ite ;
		for ( it = its ; it != ite ; )
		{
			if ( it->is_file() ) { fileChanged = true ; }
			it = delete_line( it ) ;
		}
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			cursor.set( getDataLineNext( ite ), CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_F:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		for ( j = 0 ; j < lc.lcmd_Rpt ; ++j, ++its )
		{
			if ( its->is_bod() )          { break          ; }
			if ( its->il_deleted )        { --j ; continue ; }
			if ( its->is_not_excluded() ) { break          ; }
			its->set_unexcluded( level ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_HX:
	case LC_HXX:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted ) { continue ; }
			its->il_hex = !its->il_hex ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_I:
		its = lc.lcmd_sADDR ;
		vis = countVisibleLines( its ) ;
		if ( vis == 0 )
		{
			vis = 1 ;
			moveDownLines( vis ) ;
		}
		for ( j = 0 ; j < lc.lcmd_Rpt && j < vis ; ++j )
		{
			its = data.insert( ++its, new iline( taskid(), LN_ISRT, lnumSize ), getMaskLine2() ) ;
			if ( !csrPlaced1 )
			{
				moveCursorLine( its ) ;
				its->set_idata_minsize( get_minsize() ) ;
				csrPlaced1 = true ;
			}
		}
		rebuildZAREA = true ;
		break ;

	case LC_L:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		it = getLastEX( lc.lcmd_sADDR ) ;
		for ( j = 0 ; j < lc.lcmd_Rpt ; ++j, --it )
		{
			if ( it->is_tod() )          { break          ; }
			if ( it->il_deleted )        { --j ; continue ; }
			if ( it->is_not_excluded() ) { break          ; }
			it->set_unexcluded( level ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_LC:
	case LC_LCC:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			if ( its->set_idata_lower( level, LeftBnd, RightBnd, ID_TFTS ) )
			{
				fileChanged = true ;
			}
		}
		rebuildZAREA = true ;
		break ;

	case LC_LMAC:
		itx = lmacs.find( lc.lcmd_cmdstr ) ;
		lc.lcmd_lcname = itx->second.lcname ;

		set_dialogue_var( "ZLMACENT", itx->second.lcname ) ;

		run_macro( itx->second.lcmac,
			   false,
			   false,
			   itx->second.lcpgm,
			   lc.lcmd_cmdstr ) ;
		break ;

	case LC_MASK:
		it = data.insert( lc.lcmd_sADDR, new iline( taskid(), LN_MASK ), getMaskLine2(), level ) ;
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( itr2ptr( it ), CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		if ( lc.lcmd_sADDR == topLine )
		{
			topLine = itr2ptr( it ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_MD:
	case LC_MDD:
		its     = lc.lcmd_sADDR ;
		ite     = lc.lcmd_eADDR ;
		t_iline = nullptr ;
		for ( ++ite, it = its ; it != ite && !it->is_bod() ; )
		{
			if ( !it->il_deleted && !it->is_valid_file() )
			{
				if ( it->is_col() )
				{
					tmp1 = getColumnLine( 1 ) ;
				}
				else if ( it->is_bnds() )
				{
					tmp1 = getMaskLine2() ;
				}
				else
				{
					tmp1 = it->get_idata() ;
				}
				p_iline = data.insert( it, new iline( taskid(), LN_FILE, lnumSize ), tmp1, level, ID_TE ) ;
				if ( !t_iline )
				{
					t_iline = p_iline ;
				}
				it = delete_line( it ) ;
				fileChanged  = true ;
				rebuildZAREA = true ;
			}
			else
			{
				++it ;
			}
		}
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			cursor.set( t_iline, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		break ;

	case LC_MN:
	case LC_MNN:
		its     = lc.lcmd_sADDR ;
		ite     = lc.lcmd_eADDR ;
		t_iline = nullptr ;
		for ( ++ite, it = its ; it != ite && !it->is_bod() ; )
		{
			if ( it->is_valid_file() )
			{
				p_iline = data.insert( it, new iline( taskid(), LN_NOTE ), it->get_idata(), level ) ;
				if ( !t_iline ) { t_iline = p_iline ; }
				it  = delete_line( it ) ;
				it  = p_iline ;
				ite = lc.lcmd_eADDR ;
				if ( ++it == ite ) { break ; }
				ite += 2 ;
				fileChanged  = true ;
				rebuildZAREA = true ;
			}
			else
			{
				++it ;
			}
		}
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			cursor.set( ( t_iline ) ? t_iline : lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		break ;

	case LC_NOP:
		break ;

	case LC_R:
		it   = lc.lcmd_sADDR ;
		tmp1 = it->get_idata() ;
		copyPrefix( ip, it ) ;
		if ( it->is_file() ) { fileChanged = true ; }
		for ( j = 0 ; j < lc.lcmd_Rpt ; ++j )
		{
			p_iline = data.insert( ++it, new iline( taskid() ) ) ;
			copyPrefix( p_iline, ip ) ;
			p_iline->put_idata( tmp1, level, ( ip.ip_status | ID_REPTC ) ) ;
			if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
			{
				cursor.set( p_iline, CSR_FC_LCMD ) ;
				csrPlaced2 = true ;
			}
			it = p_iline ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_RR:
		vip.clear() ;
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( it = its ; it != data.end() && !it->is_bod() ; ++it )
		{
			if ( it->il_deleted ) { continue           ; }
			if ( it->is_file()  ) { fileChanged = true ; }
			copyPrefix( ip, it ) ;
			if ( it == its )
			{
				if ( ip.is_isrt() )
				{
					cursor.set( itr2ptr( it ), CSR_FC_DATA ) ;
					csrPlaced2 = true ;
				}
			}
			ip.ip_data = it->get_idata() ;
			vip.push_back( ip ) ;
			if ( it == ite ) { break ; }
		}
		for ( j = 0 ; j < lc.lcmd_Rpt ; ++j )
		{
			for ( k = 0 ; k < vip.size() ; ++k )
			{
				p_iline = data.insert( ++ite, new iline( taskid() ) ) ;
				const ipline& ipl = vip[ k ] ;
				copyPrefix( p_iline, ipl ) ;
				p_iline->put_idata( ipl.ip_data, level, ( ipl.ip_status | ID_REPTC ) ) ;
				ite = p_iline ;
			}
		}
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( getNextDataLine( lc.lcmd_sADDR ), getNextDataLine( lc.lcmd_eADDR ), CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( getNextDataLine( lc.lcmd_eADDR ), getNextDataLine( lc.lcmd_sADDR ), CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_S:
	case LC_SI:
		j = MAXLEN ;
		l = 0 ;
		its = lc.lcmd_sADDR ;
		if ( its->is_not_excluded() ) { break ; }
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == its ) )
		{
			cursor.set( aADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		ite = lc.lcmd_eADDR ;
		for ( ++ite, it = its ; it != ite ; ++it )
		{
			if ( it->il_deleted ) { continue ; }
			k = it->get_idata_ptr()->find_first_not_of( ' ' ) ;
			if ( k != string::npos ) { j = min( j, k ) ; }
		}
		for ( it = its ; it != ite ; ++it )
		{
			if ( it->il_deleted ) { continue ; }
			k = it->get_idata_ptr()->find_first_not_of( ' ' ) ;
			if ( k == j || k == string::npos )
			{
				it->set_unexcluded( level ) ;
				if ( lc.lcmd_cmd == LC_S )
				{
					if ( ++l == lc.lcmd_Rpt ) { break ; }
				}
			}
		}
		break ;

	case LC_TABS:
		p_iline = data.insert( lc.lcmd_sADDR, new iline( taskid(), LN_TABS ), tabsLine, level ) ;
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( p_iline, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		if ( lc.lcmd_sADDR == topLine )
		{
			topLine = p_iline ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_TFLOW:
		its    = lc.lcmd_sADDR ;
		defCol = ( RightBnd == 0 ) ? ( zdataw + startCol - 1 ) : RightBnd ;
		if ( lc.lcmd_Rpt == -1 )
		{
			lc.lcmd_Rpt = defCol ;
		}
		else if ( RightBnd > 0 && lc.lcmd_Rpt > RightBnd )
		{
			lc.lcmd_Rpt = RightBnd ;
		}
		if ( lc.lcmd_Rpt <= LeftBnd )
		{
			lc.lcmd_Rpt = defCol ;
		}
		for ( k = 0, it = its ; it != data.end() ; ++it )
		{
			if ( it->is_valid_file() )
			{
				if ( tdata.size() == 0 )
				{
					k = it->get_idata_ptr()->find_first_not_of( ' ', LeftBnd-1 ) ;
					if ( k == string::npos || ( RightBnd > 0 && k >= RightBnd ) )
					{
						break ;
					}
					if ( lc.lcmd_Rpt <= ( k + 1 ) )
					{
						lc.lcmd_Rpt = defCol ;
					}
					l = k ;
				}
				else if ( tdata.size() == 1 )
				{
					k = it->get_idata_ptr()->find_first_not_of( ' ', LeftBnd-1 ) ;
					if ( k == string::npos )
					{
						k = l ;
						break ;
					}
				}
				else if ( k != it->get_idata_ptr()->find_first_not_of( ' ', LeftBnd-1 ) )
				{
					break ;
				}
				tdata.push_back( it->get_idata() ) ;
			}
		}
		tflow1 = tdata.size() ;
		tflow2 = tflow1 ;
		reflowData( tdata, lc.lcmd_Rpt, l-LeftBnd+1, k-LeftBnd+1 ) ;
		if ( tdata.size() == 0 )
		{
			if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
			{
				cursor.set( itr2ptr( its ), CSR_FC_LCMD ) ;
				csrPlaced2 = true ;
			}
			break ;
		}
		else
		{
			tflow2 = tdata.size() ;
		}
		l  = 0   ;
		it = its ;
		t_iline = nullptr ;
		for ( auto itf = tdata.begin() ; itf != tdata.end() ; ++itf, ++it, ++l )
		{
			if ( l < tflow1 )
			{
				while ( it->not_valid_file() )
				{
					++it ;
				}
				it->put_idata( *itf, level, ID_TFTS ) ;
			}
			else
			{
				p_iline = data.insert( it, new iline( taskid(), LN_FILE, lnumSize ), *itf, level, ID_TE ) ;
				if ( !t_iline )
				{
					t_iline = p_iline ;
				}
				it = p_iline ;
			}
		}
		for ( ; l < tflow1 && it != data.end() ; )
		{
			if ( it->is_valid_file() )
			{
				it = delete_line( it ) ;
				++l ;
			}
			else
			{
				++it ;
			}
		}
		if ( !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( itr2ptr( its ), ( aLCMD ) ? CSR_FC_LCMD : CSR_FNB_DATA ) ;
			csrPlaced2 = true ;
		}
		fileChanged = true ;
		break ;

	case LC_TJ:
	case LC_TJJ:
		its  = lc.lcmd_sADDR ;
		ite  = lc.lcmd_eADDR ;
		tmp1 = its->get_idata( lnumSize1, lnumS2pos ) ;
		its  = delete_line( its ) ;
		for ( ++ite ; its != ite ; )
		{
			if ( its->not_valid_file() ) { ++its ; continue ; }
			tmp1 = trim_right( tmp1 ) ;
			if ( tmp1 == "" )
			{
				tmp1 = strip( its->get_idata( lnumSize1, lnumS2pos ) ) ;
			}
			else if ( tmp1.back() == '.' )
			{
				tmp1 += "  " + strip( its->get_idata( lnumSize1, lnumS2pos ) ) ;
			}
			else
			{
				tmp1 += " " + strip( its->get_idata( lnumSize1, lnumS2pos ) ) ;
			}
			if ( reclen > 0 && tmp1.size() > ( reclen - lnumSize ) )
			{
				pcmd.set_msg( "PEDT014L", 12 ) ;
				break ;
			}
			its = delete_line( its ) ;
		}
		if ( lnumSize1 > 0 )
		{
			tmp1 = string( lnumSize1, ' ' ) + tmp1 ;
		}
		if ( reclen > 0 )
		{
			tmp1.resize( reclen, ' ' ) ;
		}
		if ( tmp1 != "" )
		{
			p_iline = data.insert( its, new iline( taskid(), LN_FILE, lnumSize ), tmp1, level, ID_TFTS ) ;
			k = p_iline->get_idata_ptr()->find_first_not_of( ' ' ) ;
			if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
			{
				cursor.set( p_iline, CSR_OFF_DATA, k ) ;
				csrPlaced2 = true ;
			}
			fileChanged = true ;
		}
		break ;

	case LC_TR:
	case LC_TRR:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			if ( its->set_idata_trim( level, ID_TFTS ) )
			{
				fileChanged = true ;
			}
		}
		rebuildZAREA = true ;
		break ;

	case LC_TS:
		it      = lc.lcmd_sADDR ;
		splitOK = textSplitData( *it->get_idata_ptr(), tmp1, tmp2 ) ;
		if ( aADDR == lc.lcmd_sADDR && aCol > CLINESZ )
		{
			if ( it->is_valid_file() )
			{
				if ( it->put_idata( tmp1, level, ID_TFTS ) )
				{
					fileChanged = true ;
				}
				it = data.insert( ++it, new iline( taskid(), LN_ISRT, lnumSize ), getMaskLine2(), ID_TE ) ;
				if ( splitOK )
				{
					data.insert( ++it, new iline( taskid(), LN_FILE, lnumSize ), tmp2, level, ID_TE ) ;
				}
			}
		}
		else
		{
			data.insert( ++it, new iline( taskid(), LN_ISRT, lnumSize ), getMaskLine2(), ID_TFTS ) ;
		}
		cursor.fix() ;
		rebuildZAREA = true ;
		break ;

	case LC_T:
	case LC_TT:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			its->toggleMarked() ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_TX:
	case LC_TXX:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted  ) { continue ; }
			its->toggle_excluded( level ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_UC:
	case LC_UCC:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			if ( its->set_idata_upper( level, LeftBnd, RightBnd, ID_TFTS ) )
			{
				fileChanged = true ;
			}
		}
		rebuildZAREA = true ;
		break ;

	case LC_XI:
		its = lc.lcmd_sADDR ;
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( itr2ptr( its ), CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		j = its->get_idata_ptr()->find_first_not_of( ' ' ) ;
		its->set_excluded( level ) ;
		while ( j != string::npos )
		{
			++its ;
			if (  its->is_bod() )   { break    ; }
			if (  its->il_deleted ) { continue ; }
			if ( !its->is_file() )  { its->set_excluded( level ) ; continue ; }
			k = its->get_idata_ptr()->find_first_not_of( ' ' ) ;
			if ( k != string::npos && k < j ) { break ; }
			its->set_excluded( level ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_X:
	case LC_XX:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted ) { continue ; }
			its->set_excluded( level ) ;
		}
		rebuildZAREA = true ;
		break ;

	case LC_SRTC:
		its = lc.lcmd_sADDR ;
		k = its->get_idata_ptr()->find_first_not_of( ' ' ) ;
		lc.lcmd_Rpt = (lc.lcmd_Rpt-1) * profXTabz + (profXTabz - k % profXTabz) ;
	case LC_SRC:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		if ( its->is_special() ) { break ; }
		its->put_idata( rshiftCols( lc.lcmd_Rpt, its->get_idata_ptr()), level, ID_CSHIFT ) ;
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SRCC:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			its->put_idata( rshiftCols( lc.lcmd_Rpt, its->get_idata_ptr()), level, ID_CSHIFT ) ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SRTCC:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			k = its->get_idata_ptr()->find_first_not_of( ' ' ) ;
			l = (lc.lcmd_Rpt-1) * profXTabz + (profXTabz - k % profXTabz) ;
			its->put_idata( rshiftCols( l, its->get_idata_ptr()), level, ID_CSHIFT ) ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SLTC:
		its = lc.lcmd_sADDR ;
		k = its->get_idata_ptr()->find_first_not_of( ' ', LeftBnd-1 ) ;
		if ( k % profXTabz == 0 ) { lc.lcmd_Rpt = lc.lcmd_Rpt * profXTabz ; }
		else                      { lc.lcmd_Rpt = (lc.lcmd_Rpt-1) * profXTabz + (k % profXTabz) ; }
		lc.lcmd_Rpt = min( lc.lcmd_Rpt, k ) ;
	case LC_SLC:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		if ( its->is_special() ) { break ; }
		its->put_idata( lshiftCols( lc.lcmd_Rpt, its->get_idata_ptr()), level, ID_CSHIFT ) ;
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SLCC:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			its->put_idata( lshiftCols( lc.lcmd_Rpt, its->get_idata_ptr()), level, ID_CSHIFT ) ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SLTCC:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			k = its->get_idata_ptr()->find_first_not_of( ' ', LeftBnd-1 ) ;
			if ( k % profXTabz == 0 ) { l = lc.lcmd_Rpt * profXTabz ; }
			else                      { l = (lc.lcmd_Rpt-1) * profXTabz + (k % profXTabz) ; }
			l = min( l, k ) ;
			its->put_idata( lshiftCols( l, its->get_idata_ptr()), level, ID_CSHIFT ) ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_SRD:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		if ( its->is_special() ) { break ; }
		shiftOK = rshiftData( lc.lcmd_Rpt, its->get_idata_ptr(), tmp1 ) ;
		if ( its->put_idata( tmp1, level, ID_DSHIFT ) )
		{
			fileChanged = true ;
		}
		if ( !shiftOK ) { its->setErrorStatus( level ) ; pcmd.set_msg( "PEDT014B", 12 ) ; }
		rebuildZAREA = true ;
		break ;

	case LC_SRDD:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			shiftOK = rshiftData( lc.lcmd_Rpt, its->get_idata_ptr(), tmp1 ) ;
			if ( its->put_idata( tmp1, level, ID_DSHIFT ) )
			{
				fileChanged = true ;
			}
			if ( !shiftOK ) { its->setErrorStatus( level ) ; pcmd.set_msg( "PEDT014B", 12 ) ; }
		}
		rebuildZAREA = true ;
		break ;

	case LC_SLD:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR ) )
		{
			cursor.set( lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		if ( its->is_special() ) { break ; }
		shiftOK = lshiftData( lc.lcmd_Rpt, its->get_idata_ptr(), tmp1 ) ;
		if ( its->put_idata( tmp1, level, ID_DSHIFT ) )
		{
			fileChanged = true ;
		}
		if ( !shiftOK ) { its->setErrorStatus( level ) ; pcmd.set_msg( "PEDT014B", 12 ) ; }
		rebuildZAREA = true ;
		break ;

	case LC_SLDD:
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == lc.lcmd_sADDR
							   || aADDR == lc.lcmd_eADDR ) )
		{
			if ( aADDR != lc.lcmd_eADDR )
			{
				cursor.set( lc.lcmd_sADDR, lc.lcmd_eADDR, CSR_FC_LCMD ) ;
			}
			else
			{
				cursor.set( lc.lcmd_eADDR, lc.lcmd_sADDR, CSR_FC_LCMD ) ;
			}
			csrPlaced2 = true ;
		}
		its = lc.lcmd_sADDR ;
		ite = lc.lcmd_eADDR ;
		for ( ++ite ; its != ite ; ++its )
		{
			if ( its->il_deleted || its->is_special() ) { continue ; }
			shiftOK = lshiftData( lc.lcmd_Rpt, its->get_idata_ptr(), tmp1 ) ;
			if ( its->put_idata( tmp1, level, ID_DSHIFT ) )
			{
				fileChanged = true ;
			}
			if ( !shiftOK ) { its->setErrorStatus( level ) ; pcmd.set_msg( "PEDT014B", 12 ) ; }
		}
		rebuildZAREA = true ;
		break ;

	case LC_COPY:
	case LC_MOVE:
		i = 0  ;
		j = -1 ;
		k = -1 ;
		if ( pcmd.get_cmd_words() == 1 )
		{
			if ( lc.lcmd_cmd == LC_COPY )
			{
				fname = displayPanel( "PEDIT015", "ZFILE2", "PEDT011R" ) ;
				if ( RC == 8 ) { break ; }
				vcopy( "LINE1", tmp1, MOVE ) ;
				if ( tmp1 != "" ) { j = ds2d( tmp1 ) ; }
				vcopy( "LINE2", tmp1, MOVE ) ;
				if ( tmp1 != "" ) { k = ds2d( tmp1 ) ; }
			}
			else
			{
				fname = displayPanel( "PEDIT01K", "ZFILE2", "PEDT011R" ) ;
				if ( RC == 8 ) { break ; }
			}
		}
		else
		{
			fname = pcmd.get_userdata() ;
		}
		if ( lc.lcmd_cmd == LC_MOVE && !confirmMove( fname ) )
		{
			break ;
		}
		fin.open( fname.c_str() ) ;
		if ( !fin.is_open() )
		{
			pcmd.set_msg( "PEDT014S", fname, 20 ) ;
			break ;
		}
		vst.clear() ;
		while ( getline( fin, inLine ) )
		{
			if ( lc.lcmd_cmd == LC_COPY )
			{
				if ( ++i < j ) { continue ; }
				if ( k > -1 && i > k ) { break ; }
			}
			p1 = inLine.find( '\t' ) ;
			while ( p1 != string::npos && !optNoConvTabs )
			{
				j  = profXTabz - (p1 % profXTabz ) ;
				inLine.replace( p1, 1,  j, ' ' ) ;
				p1 = inLine.find( '\t', p1 + 1 ) ;
			}
			vst.push_back( inLine ) ;
		}
		fin.close() ;
		if ( vst.size() == 0 )
		{
			pcmd.set_msg( "PEDT015L", 8 ) ;
			break ;
		}
		its = lc.lcmd_sADDR ;
		if ( lc.lcmd_ABOW == 'B' )
		{
			--its ;
		}
		its = addSpecial( LN_INFO, its, left( "== Start of inserted file ", zdataw, '=' ) ) ;
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == its ) )
		{
			cursor.set( aADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		for ( j = 0 ; j < vst.size() ; ++j )
		{
			its = data.insert( ++its, new iline( taskid(), LN_FILE, lnumSize ), vst[ j ], level, ID_COPYRM ) ;
		}
		addSpecial( LN_INFO, its, left( "== End of inserted file ", zdataw, '=' ) ) ;
		pcmd.set_msg( "PEDT015J", d2ds( vst.size() ), fname, 0 ) ;
		if ( lc.lcmd_cmd == LC_MOVE )
		{
			try
			{
				remove( fname ) ;
			}
			catch ( boost::filesystem::filesystem_error &e )
			{
				pcmd.set_msg( "PEDT018Q", fname, e.what(), 12 ) ;
			}
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;

	case LC_PASTE:
		getClipboard( vip ) ;
		if ( vip.size() == 0 )
		{
			pcmd.set_msg( "PEDT015L", 8 ) ;
			break ;
		}
		vreplace( "ZEDLNES", d2ds( vip.size() ) ) ;
		vreplace( "CLIPNAME", clipBoard ) ;
		pcmd.set_msg( "PEDT013D", 0 ) ;
		its = lc.lcmd_sADDR ;
		if ( lc.lcmd_ABOW == 'B' )
		{
			--its ;
		}
		for ( j = 0 ; j < vip.size() ; ++j )
		{
			if ( reclen > 0 )
			{
				string& str = vip[ j ].ip_data ;
				if ( str.size() > reclen )
				{
					pcmd.set_msg( "PEDT018C", 0 ) ;
				}
				str.resize( reclen, ' ' ) ;
			}
			its = data.insert( ++its, new iline( taskid(), LN_FILE, lnumSize ), vip[ j ].ip_data, level, ID_COPYRM ) ;
		}
		if ( aLCMD && !csrPlaced1 && ( !csrPlaced2 || aADDR == its ) )
		{
			cursor.set( aADDR, CSR_FC_LCMD ) ;
			csrPlaced2 = true ;
		}
		rebuildZAREA = true ;
		fileChanged  = true ;
		break ;
	}
}


void pedit01::action_RCHANGE()
{
	//
	// Change string starting at cursor position.
	//

	TRACE_FUNCTION() ;

	string w1 = upper( word( pcmd.get_parms(), 1 ) ) ;

	fcx_parms.set_change_counts( 0, 0 ) ;

	if ( w1 != "" && !pfkeyPressed )
	{
		pcmd.set_zverb_cmd() ;
		pcmd.set_msg( "PEDT011", 12 ) ;
		return ;
	}

	if ( findword( w1, "C CHA CHG CHANGE" ) )
	{
		pcmd.pop_parm1() ;
		if ( !setFindChangeExcl( 'C' ) ) { return ; }
	}
	else if ( !fcx_parms.f_cset )
	{
		pcmd.set_msg( "PEDT013N", 12 ) ;
		return ;
	}
	else if ( findword( w1, "F FIND" ) )
	{
		pcmd.pop_parm1() ;
		if ( !setFindChangeExcl( 'F', true ) ) { return ; }
	}
	else if ( w1 != "" )
	{
		pcmd.set_cmd( pcmd.get_parms() ) ;
		pcmd.set_msg( "PEDT011", 12 ) ;
		return ;
	}

	( fcx_parms.f_reverse() ) ? ++aCol : --aCol ;

	actionFind() ;
	if ( fcx_parms.f_error ) { return ; }

	if ( !fcx_parms.f_success )
	{
		setNotFoundMsg() ;
		fcx_parms.f_ADDR = nullptr ;
		rebuildZAREA = true ;
		return ;
	}

	actionChange() ;
	if ( fcx_parms.f_error )
	{
		fcx_parms.f_ch_errs = 1 ;
		setChangedError() ;
		rebuildZAREA = true ;
		return ;
	}

	fcx_parms.f_ch_occs = 1 ;

	setCursorChange() ;

	moveTopline( fcx_parms.f_ADDR ) ;

	setChangedMsg() ;

	fcx_parms.f_ADDR = nullptr ;

	rebuildZAREA = true ;
}


void pedit01::action_RFIND()
{
	//
	// Action REPEAT FIND or EXCLUDE.
	//

	TRACE_FUNCTION() ;

	string w1 = upper( word( pcmd.get_parms(), 1 ) ) ;

	if ( w1 != "" && !pfkeyPressed )
	{
		pcmd.set_zverb_cmd() ;
		pcmd.set_msg( "PEDT011", 12 ) ;
		return ;
	}

	if ( findword( w1, "C CHA CHG CHANGE" ) )
	{
		pcmd.pop_parm1() ;
		if ( !setFindChangeExcl( 'C' ) ) { return ; }
	}
	else if ( w1 == "F" || w1 == "FIND" )
	{
		pcmd.pop_parm1() ;
		if ( !setFindChangeExcl( 'F' ) ) { return ; }
	}
	else if ( w1 != "" )
	{
		pcmd.set_cmd( pcmd.get_parms() ) ;
		pcmd.set_msg( "PEDT011", 12 ) ;
		return ;
	}
	else if ( !fcx_parms.f_fset )
	{
		pcmd.set_msg( "PEDT013M", 12 ) ;
		return ;
	}

	if ( fcx_parms.f_reverse() && aCol > 0 && fcx_parms.f_suffix() )
	{
		--aCol ;
	}

	if ( fcx_parms.f_exclude && aADDR && aADDR->is_excluded() )
	{
		aCol = ( fcx_parms.f_reverse() ) ? 1 : aADDR->get_idata_len() ;
	}

	actionFind() ;
	if ( fcx_parms.f_error ) { return ; }

	if ( fcx_parms.f_success )
	{
		if ( fcx_parms.f_exclude )
		{
			fcx_parms.set_exclude_counts( 1, 1 ) ;
		}
		else
		{
			fcx_parms.set_find_counts( 1, 1 ) ;
		}
		setCursorFind() ;
		setFoundMsg() ;
	}
	else
	{
		if ( fcx_parms.f_exclude )
		{
			fcx_parms.set_exclude_counts( 0, 0 ) ;
		}
		else
		{
			fcx_parms.set_find_counts( 0, 0 ) ;
		}
		setNotFoundMsg() ;
	}

	rebuildZAREA = true ;
}


void pedit01::action_ScrollUp()
{
	TRACE_FUNCTION() ;

	int t ;

	Data::Iterator it = nullptr ;

	rebuildZAREA = true ;

	cursor.fix() ;

	if ( zscrolla == "MAX" )
	{
		it = data.top() ;
	}
	else
	{
		for ( it = topLine, t = 0 ; it != data.top() ; --it )
		{
			if ( it->il_deleted ) { continue ; }
			if ( it->is_excluded() )
			{
				it = getFirstEX( it ) ;
				if ( hideExcl ) { it = getPrevDataLine( it ) ; }
				if ( it == data.begin() ) { break ; }
			}
			if ( it->is_valid_file()     &&
			   ( it->il_hex || profHex ) &&
			    !isnumeric( zscrolla ) )
			{
				t += 4 ;
			}
			else
			{
				++t ;
			}
			if ( t > zscrolln ) { break ; }
		}
		if ( it->is_excluded() )
		{
			it = getFirstEX( it ) ;
		}
	}

	topLine = itr2ptr( it ) ;
}


void pedit01::action_ScrollDown()
{
	TRACE_FUNCTION() ;

	int t ;

	rebuildZAREA = true ;

	cursor.fix() ;

	Data::Iterator it = nullptr ;

	if ( zscrolla == "MAX" )
	{
		t = ( colsOn ) ? 1 : 0 ;
		for ( it = data.bottom() ; it != data.begin() ; --it )
		{
			if ( it->il_deleted ) { continue ; }
			if ( it->is_excluded() )
			{
				it = getFirstEX( it ) ;
				if ( hideExcl ) { it = getPrevDataLine( it ) ; }
				if ( it == data.begin() ) { break ; }
			}
			if ( it->is_valid_file() && ( it->il_hex || profHex ) )
			{
				t += 4 ;
			}
			else
			{
				++t ;
			}
			if ( t > zlvline ) { break ; }
			topLine = itr2ptr( it ) ;
		}
		if ( it == data.begin() && t <= zlvline )
		{
			topLine = data.top() ;
		}
	}
	else
	{
		t = ( colsOn && zscrolla == "CSR" ) ? 1 : 0 ;
		for ( it = topLine ; it != data.bottom() ; ++it )
		{
			if ( it->il_deleted ) { continue ; }
			if ( it->is_excluded() )
			{
				it = getLastEX( it ) ;
				if ( hideExcl ) { it = getNextDataLine( it ) ; }
				if ( it == data.end() || it == data.bottom() ) { break ; }
			}
			if ( it->is_valid_file() && ( it->il_hex || profHex ) && !isnumeric( zscrolla ) )
			{
				t += 4 ;
			}
			else
			{
				++t ;
			}
			if ( t > zscrolln ) { break ; }
		}
		topLine = itr2ptr( it ) ;
	}

	if ( topLine->is_excluded() )
	{
		topLine = itr2ptr( getFirstEX( topLine ) ) ;
	}
}


void pedit01::action_ScrollLeft()
{
	TRACE_FUNCTION() ;

	rebuildZAREA = true ;

	cursor.fix() ;

	if ( zscrolla == "MAX" )
	{
		startCol = 1 ;
	}
	else
	{
		if ( LeftBnd  > 1 &&
		     LeftBnd  < startCol &&
		     zscrolln > startCol - LeftBnd )
		{
			startCol = LeftBnd ;
		}
		else
		{
			startCol = startCol - zscrolln ;
		}
		if ( startCol < 1 ) { startCol = 1 ; }
	}
}


void pedit01::action_ScrollRight()
{
	TRACE_FUNCTION() ;

	rebuildZAREA = true ;

	cursor.fix() ;

	if ( zscrolla == "MAX" )
	{
		startCol = ( reclen > 0 ) ? reclen - zdataw + 1 : maxCol() - zdataw + 1 ;
	}
	else if ( zscrolla == "CSR" )
	{
		if ( zscrolln == CLINESZ )
		{
			startCol += zdataw ;
		}
		else
		{
			startCol += zscrolln - CLINESZ ;
		}
	}
	else if ( !LeftBndSet &&
		   LeftBnd  > startCol &&
		   zscrolln > ( LeftBnd - startCol ) )
	{
		startCol = LeftBnd ;
	}
	else if ( RightBnd > 0 &&
		  RightBnd > ( startCol + zdataw - 1 ) &&
		  zscrolln > ( RightBnd - startCol - zdataw ) )
	{
		startCol = RightBnd - zdataw + 1 ;
	}
	else
	{
		startCol += zscrolln ;
	}

	if ( startCol < 1 )
	{
		startCol = LeftBnd ;
	}
	else if ( !LeftBndSet && LeftBnd == startCol )
	{
		noop ;
	}
	else if ( reclen > 0 && ( startCol + zdataw ) > reclen )
	{
		startCol = ( reclen <= zdataw ) ?
			   ( startCol > LeftBnd ) ? LeftBnd : startCol : ( reclen - zdataw + 1 ) ;
	}
	else if ( startCol + zdataw > MAXLEN )
	{
		startCol = MAXLEN - zdataw ;
	}
}


void pedit01::moveCursorEnter()
{
	//
	// If cursor has not been positioned, move to first word on the same line if on the line command area,
	// next line down if on the data area, next tab position, or home position if not on data.
	//

	TRACE_FUNCTION() ;

	int row ;
	int dataw = get_datawidth() ;

	size_t p1 ;
	size_t len ;

	bool nline ;

	iline* dl ;

	ipos* pip ;

	cursor.clear() ;

	if ( rebuildZAREA )
	{
		fill_dynamic_area() ;
		aRow = getRow( aADDR ) ;
	}

	if ( aRow == 0 )
	{
		return ;
	}

	pip = &s2data.at( aRow - 1 ) ;

	if ( !pip->ipos_addr )
	{
		cursor.home() ;
		return ;
	}

	if ( pip->ipos_hex_line() )
	{
		if ( aLCMD )
		{
			cursor.set( ( aRow - pip->ipos_hex ), CSR_FC_LCMD ) ;
			return ;
		}
		row = aRow ;
		p1  = aCol - CLINESZ - 1 ;
		if ( p1 < dataw )
		{
			if ( profVert )
			{
				if ( pip->ipos_hex_bottom() ) { --row ; }
			}
			else
			{
				p1 &= ~1 ;
			}
		}
		else
		{
			p1 = dataw - 1 ;
		}
		cursor.set( row, CSR_OFF_LCMD, p1 ) ;
		return ;
	}

	dl = aADDR ;

	if ( ( dl->il_hex || profHex ) && !pip->ipos_div() )
	{
		if ( aLCMD )
		{
			cursor.set( aRow, CSR_FC_LCMD ) ;
			return ;
		}
		row = aRow + 1 ;
		p1  = aCol - CLINESZ - 1 ;
		if ( !profVert )
		{
			if ( p1 < dataw )
			{
				p1 *= 2 ;
				if ( p1 >= dataw )
				{
					p1 -= dataw ;
					++row ;
				}
			}
			else
			{
				p1 = dataw - 1 ;
			}
		}
		cursor.set( row, CSR_OFF_LCMD, p1 ) ;
		return ;
	}

	if ( pip->ipos_div() )
	{
		if ( aRow < zlvline )
		{
			cursor.set( ( aRow + 1 ), CSR_FC_LCMD ) ;
		}
		return ;
	}

	if ( getSoftwareTabLocation( p1, len, nline ) )
	{
		if ( nline )
		{
			dl = getNextDataLine( dl ) ;
			moveCursorLine( ( aRow + 1 ), dl, p1, len ) ;
		}
		else
		{
			moveCursorLine( aRow, dl, p1, len ) ;
		}
	}
	else if ( aRow == zlvline )
	{
		if ( dl->is_bod() )
		{
			cursor.home() ;
			return ;
		}
		if ( aLCMD )
		{
			moveCursorLine( aRow-1, dl ) ;
		}
		else
		{
			cursor.set( aRow, CSR_FNB_DATA ) ;
		}
		rebuildZAREA = true ;
	}
	else if ( aLCMD )
	{
		moveCursorLine( aRow, dl ) ;
	}
	else
	{
		dl = s2data.at( aRow ).ipos_addr ;
		moveCursorLine( ( aRow + 1 ), dl ) ;
	}
}


int pedit01::maxCol()
{
	//
	// Get the maximum column length of any line on the screen.
	//

	TRACE_FUNCTION() ;

	int mcol = 0 ;

	iline* dl ;

	for ( int i = 0 ; i < zlvline ; ++i )
	{
		dl = s2data[ i ].ipos_addr ;
		if ( dl && dl->is_valid_file() )
		{
			mcol = max( mcol, dl->get_idata_len() ) ;
		}
	}

	return mcol ;
}


void pedit01::moveCursorLine( int row,
			      iline* dl )
{
	//
	// Starting at the beginning of a line, move cursor to the correct position on that line:
	// either first non-blank character, or, if blank, the position of the first
	// non-blank character on the line above or, if blank also, the line below.
	//

	TRACE_FUNCTION() ;

	iline* dx ;

	if ( !dl )
	{
		cursor.home() ;
		return ;
	}

	if ( !moveCursorNB( row, dl ) )
	{
		dx = getPrevFileLine( dl ) ;
		if ( !moveCursorNB( row, dx ) )
		{
			dx = getNextFileLine( dl ) ;
			if ( !moveCursorNB( row, dx ) )
			{
				cursor.set( row, CSR_FC_DATA ) ;
			}
		}
	}
}


void pedit01::moveCursorLine( int row,
			      iline* dl,
			      size_t p1,
			      size_t len )
{
	//
	// Starting at the beginning of a line, move cursor to the correct position on that line:
	// either first non-blank character of the tab field, or, if blank, the position of the first
	// non-blank character in the tab field on the line above or, if blank also, the line below.
	//

	TRACE_FUNCTION() ;

	iline* dx ;

	if ( !dl )
	{
		cursor.home() ;
		return ;
	}

	if ( !moveCursorNB( row, dl, p1, len ) )
	{
		dx = getPrevFileLine( dl ) ;
		if ( !moveCursorNB( row, dx, p1, len ) )
		{
			dx = getNextFileLine( dl ) ;
			if ( !moveCursorNB( row, dx, p1, len ) )
			{
				cursor.set( row, CSR_OFF_DATA, p1 ) ;
			}
		}
	}
}


void pedit01::moveCursorLine( Data::Iterator it )
{
	//
	// Starting at the beginning of a line, move cursor to the correct position on that line:
	// either first non-blank character, or, if blank, the position of the first
	// non-blank character on the line above or, if blank also, the line below.
	//

	TRACE_FUNCTION() ;

	if ( it == NULL )
	{
		cursor.home() ;
		return ;
	}

	iline* dl = itr2ptr( it ) ;
	iline* dx ;

	if ( !moveCursorNB( dl, dl ) )
	{
		dx = getPrevFileLine( dl ) ;
		if ( !moveCursorNB( dl, dx ) )
		{
			dx = getNextFileLine( dl ) ;
			if ( !moveCursorNB( dl, dx ) )
			{
				cursor.set( dl, CSR_FC_DATA ) ;
			}
		}
	}
}


bool pedit01::moveCursorNB( int row,
			    iline* dl )
{
	//
	// Move cursor to the first non-blank char on the screen (ignoring the number areas), or return false if all blank or invalid.
	// For special lines, just place cursor on first byte of the screen data.
	//

	TRACE_FUNCTION() ;

	size_t p1 ;
	size_t st ;

	if ( !dl )
	{
		return false ;
	}

	if ( dl->is_special() )
	{
		cursor.set( row, CSR_FC_DATA ) ;
		return true ;
	}

	st = ( lnumSize1 > 0 && startCol <= lnumSize1 ) ? lnumSize1 : startCol - 1 ;

	p1 = dl->get_idata_ptr()->find_first_not_of( ' ', st ) ;
	if ( p1 == string::npos || p1 >= ( startCol + zdataw - 1 ) || ( lnumS2pos > 0 && p1 >= lnumS2pos ) )
	{
		return false ;
	}

	cursor.set( row, CSR_OFF_DATA, p1 ) ;

	return true ;
}


bool pedit01::moveCursorNB( int row,
			    iline* dl,
			    size_t p1,
			    size_t len )
{
	//
	// Move cursor to the first non-blank char in the software tab field, or return false if all blank.
	// Ignore tab field if it is entirely before the left margin.
	// For special lines, just place cursor on first byte of the software tab field.
	//

	TRACE_FUNCTION() ;

	size_t p2 ;

	if ( !dl )
	{
		return false ;
	}

	if ( dl->is_special() )
	{
		cursor.set( row, CSR_OFF_DATA, p1 ) ;
		return true ;
	}

	p2 = dl->get_idata_ptr()->find_first_not_of( ' ', p1 ) ;
	if ( p2 == string::npos || p2 >= ( startCol + zdataw - 1 ) || p2 >= ( p1 + len ) )
	{
		return false ;
	}

	cursor.set( row, CSR_OFF_DATA, p2 ) ;

	return true ;
}


bool pedit01::moveCursorNB( iline* dl,
			    iline* dx )
{
	//
	// Move cursor to the first non-blank char on the screen (ignoring the number areas), or return false if all blank or invalid.
	// For special lines, just place cursor on first byte of the screen data.
	//

	TRACE_FUNCTION() ;

	size_t p1 ;
	size_t st ;

	if ( !dx )
	{
		return false ;
	}

	if ( dx->is_special() )
	{
		cursor.set( dl, CSR_FC_DATA ) ;
		return true ;
	}

	st = ( lnumSize1 > 0 && startCol <= lnumSize1 ) ? lnumSize1 : startCol - 1 ;

	p1 = dx->get_idata_ptr()->find_first_not_of( ' ', st ) ;
	if ( p1 == string::npos || p1 >= ( startCol + zdataw - 1 ) || ( lnumS2pos > 0 && p1 >= lnumS2pos ) )
	{
		return false ;
	}

	cursor.set( dl, CSR_OFF_DATA, p1 ) ;

	return true ;
}


bool pedit01::getSoftwareTabLocation( size_t& pos,
				      size_t& len,
				      bool& nline )
{
	//
	// Get the software tab field location and length starting from the cursor position
	// adjusting for left hand start column position and line numbers.
	//
	// If none found, start from the beginning of the line.
	//
	// Don't search for tabs before the left hand side or in the line number areas.
	//

	TRACE_FUNCTION() ;

	string tline = tabsLine ;

	if ( lnumS2pos > 0 && tabsLine.size() > lnumS2pos )
	{
		tline.resize( lnumS2pos ) ;
	}

	size_t st = ( lnumSize1 >= startCol ) ? lnumSize1 : startCol - 1 ;

	size_t c1 = ( aDATA ) ? ( aCol + st - CLINESZ - 1 ) : st ;

	size_t pl ;

	if ( tline.find( '-', st ) == string::npos )
	{
		return false ;
	}

	pos   = string::npos ;
	nline = false ;

	if ( tline.size() > c1 )
	{
		if ( tline[ c1 ] == '-' )
		{
			if ( aDATA )
			{
				pos = tline.find_first_not_of( '-', c1 ) ;
				if ( pos != string::npos )
				{
					pos = tline.find( '-', pos ) ;
				}
			}
			else
			{
				pos = c1 ;
			}
		}
		else
		{
			pos = tline.find( '-', c1 ) ;
		}
	}

	if ( pos == string::npos )
	{
		pos   = tline.find( '-', st ) ;
		nline = true ;
	}

	pl  = tline.find_first_not_of( '-', pos ) ;
	pos = tline.find_last_not_of( '-', pos ) ;

	pos = ( pos == string::npos ) ? 0 : ( pos + 1 ) ;

	pos = max( pos, st ) ;

	len = ( pl == string::npos ) ? tline.size() - pos : pl - pos ;

	return true ;
}


void pedit01::getLogicalTabLocations( vector<size_t>& tabs )
{
	//
	// Get the logical tab locations.
	// For hardware tabs, this is the space or hyphen after the tab position.
	// Ignore tabs in the line number areas or beyond reclen.
	//
	// Assume first position is a tab location, unless it contains a hardware tab.
	//

	TRACE_FUNCTION() ;

	if ( tabsLine.size() > lnumSize1 && tabsLine[ lnumSize1 ] != '*' )
	{
		tabs.push_back( lnumSize1 ) ;
	}

	size_t pos = tabsLine.find( '*', lnumSize1 ) ;

	while ( pos != string::npos )
	{
		pos = tabsLine.find_first_of( " -", pos + 1 ) ;
		if ( pos == string::npos )
		{
			if ( reclen > 0 && tabsLine.size() >= reclen )
			{
				break ;
			}
			tabs.push_back( tabsLine.size() ) ;
			break ;
		}
		else if ( ( reclen > 0 && pos >= reclen ) || ( lnumS2pos > 0 && pos > lnumS2pos ) )
		{
			break ;
		}
		tabs.push_back( pos ) ;
		pos = tabsLine.find( '*', pos + 1 ) ;
	}
}


bool pedit01::onHardwareTab( size_t p )
{
	//
	// Return true if 'p' is on a hardware tab position.
	//

	TRACE_FUNCTION() ;

	return ( tabsLine.size() > p && tabsLine[ p ] == '*' ) ;
}


void pedit01::sort_data( const string& cmd )
{
	//
	// Sort data between columns and ranges.
	//
	// Macro return codes:
	//  0  Normal completion.
	//  4  Lines were already in sort order.
	//  8  No records to sort.
	// 16  Not enough storage to sort.
	// 20  Severe error.
	//

	TRACE_FUNCTION() ;

	int i ;
	int ws ;
	int rc ;

	size_t s1 ;
	size_t s2 ;
	size_t ln ;

	bool sorted ;
	bool only_x ;
	bool only_nx ;

	string wall ;

	string w1 ;
	string w2 ;
	string w3 ;
	string t1 ;
	string t2 ;

	vector<line_data> tdata ;

	vector<bool>srt_asc ;

	vector<size_t>col_a ;
	vector<size_t>col_b ;
	vector<size_t>col_l ;

	Data::Iterator it  = nullptr ;
	Data::Iterator its = nullptr ;
	Data::Iterator ite = nullptr ;

	wall = strip( upper( subword( cmd, 2 ) ) ) ;

	rc = extract_lptr( wall, its, ite, false ) ;
	if ( rc == 1 )
	{
		pcmd.set_msg( "PEDT018I", 8 ) ;
		return ;
	}
	else if ( rc > 1 )
	{
		pcmd.set_msg( "PEDT018J", 20 ) ;
		return ;
	}

	only_x  = parseString2( wall, "X" ) ;
	only_nx = parseString2( wall, "NX" ) ;

	if ( wall == "" )
	{
		srt_asc.push_back( true ) ;
		col_a.push_back( LeftBnd - 1 ) ;
		col_b.push_back( ( RightBnd > 0 ) ? RightBnd - 1 : string::npos ) ;
	}

	while ( wall != "" )
	{
		ws = words( wall ) ;
		if ( ws <= 3 )
		{
			if ( parseString2( wall, "D" ) )
			{
				srt_asc.push_back( false ) ;
			}
			else if ( parseString2( wall, "A" ) )
			{
				srt_asc.push_back( true ) ;
			}
			else
			{
				srt_asc.push_back( true ) ;
			}
			if ( words( wall ) > 2 )
			{
				pcmd.set_msg( "PEDT018M", 20 ) ;
				return ;
			}
			w2 = word( wall, 1 ) ;
			w3 = word( wall, 2 ) ;
			if ( w2 != "" && ( !datatype( w2, 'W' ) || ds2d( w2 ) < 1 ) )
			{
				pcmd.set_msg( "PEDT018L", 20 ) ;
				return ;
			}
			if ( w3 != "" && ( !datatype( w3, 'W' ) || ds2d( w3 ) < 1 ) )
			{
				pcmd.set_msg( "PEDT018L", 20 ) ;
				return ;
			}
			s1   = ( w2 != "" ) ? ds2d( w2 ) : LeftBnd ;
			s2   = ( w3 != "" ) ? ds2d( w3 ) :
			       ( RightBnd > 0 ) ? RightBnd : string::npos ;
			wall = "" ;
		}
		else if ( allNumeric( wall ) )
		{
			w1 = word( wall, 1 ) ;
			w2 = word( wall, 2 ) ;
			s1   = ds2d( w1 ) ;
			s2   = ( w2 != "" ) ? ds2d( w2 ) :
			       ( RightBnd > 0 ) ? RightBnd : string::npos ;
			srt_asc.push_back( true ) ;
			idelword( wall, 1, 2 ) ;
			trim( wall ) ;
		}
		else
		{
			if ( ws % 3 != 0 )
			{
				pcmd.set_msg( "PEDT018N", 20 ) ;
				return ;
			}
			w1 = word( wall, 1 ) ;
			w2 = word( wall, 2 ) ;
			w3 = word( wall, 3 ) ;
			if ( datatype( w1, 'W' ) && datatype( w2, 'W' ) && !datatype( w3, 'W' ) )
			{
				swap( w1, w2 ) ;
				swap( w1, w3 ) ;
			}
			if ( w1 == "D" )
			{
				srt_asc.push_back( false ) ;
			}
			else if ( w1 == "A" )
			{
				srt_asc.push_back( true ) ;
			}
			else
			{
				pcmd.set_msg( "PEDT018M", 20 ) ;
				return ;
			}
			if ( !datatype( w2, 'W' ) || !datatype( w3, 'W' ) )
			{
				pcmd.set_msg( "PEDT018L", 20 ) ;
				return ;
			}
			s1 = ds2d( w2 ) ;
			s2 = ds2d( w3 ) ;
			idelword( wall, 1, 3 ) ;
			trim( wall ) ;
		}
		if ( s1 > s2 )
		{
			swap( s1, s2 ) ;
		}
		if ( LeftBnd > 1 && ( ( s1 > 0 && s1 < LeftBnd ) || ( s2 > 0 && s2 < LeftBnd ) ) )
		{
			pcmd.set_msg( "PEDT014F", d2ds( LeftBnd ), 12 ) ;
			return ;
		}
		if ( RightBnd > 1 && ( ( s1 > 0 && s1 > RightBnd ) || ( s2 > 0 && s2 > RightBnd ) ) )
		{
			pcmd.set_msg( "PEDT014G", d2ds( RightBnd ), 12 ) ;
			return ;
		}
		col_a.push_back( s1 - 1 ) ;
		col_b.push_back( ( s2 == string::npos ) ? string::npos : s2 - 1 ) ;
	}

	copyFileData( tdata, only_x, only_nx, its, ++ite, ln ) ;

	if ( tdata.empty() )
	{
		pcmd.set_msg( "PEDT018I", 8 ) ;
		return ;
	}

	replace( col_b.begin(), col_b.end(), string::npos, ln ) ;

	for ( i = 0 ; i < col_a.size() ; ++i )
	{
		col_l.push_back( col_b[ i ] - col_a[ i ] + 1 ) ;
	}

	if ( col_a.size() > 1 )
	{
		t1.reserve( ln ) ;
		for ( i = 0 ; i < col_a.size() ; ++i )
		{
			s1 = col_a[ i ] ;
			s2 = col_b[ i ] ;
			ln = col_l[ i ] ;
			if ( t1.size() <= s2 )
			{
				t1.resize( s2+1, ' ' ) ;
			}
			if ( t1.compare( s1, ln, string( ln, ' ' ) ) != 0 )
			{
				pcmd.set_msg( "PEDT018O", d2ds( s1+1 ), 20 ) ;
				return ;
			}
			t1.replace( s1, ln, ln, 'X' ) ;
		}
	}

	stable_sort( tdata.begin(), tdata.end(),
		[ &srt_asc, &col_a, &col_l ]( const line_data& a, const line_data& b )
		{
			for ( size_t i = 0 ; i < srt_asc.size() ; ++i )
			{
				size_t ca = col_a[ i ] ;
				size_t cl = col_l[ i ] ;
				if ( a.line_str.substr( ca, cl ) != b.line_str.substr( ca, cl ) )
				{
					if ( srt_asc[ i ] )
					{
						return a.line_str.substr( ca, cl ) < b.line_str.substr( ca, cl ) ;
					}
					else
					{
						return a.line_str.substr( ca, cl ) > b.line_str.substr( ca, cl ) ;
					}
				}
			}
			return false ;
		} ) ;

	sorted = true ;
	it     = its ;

	for ( i = 0 ; i < tdata.size() ; ++it )
	{
		if ( it->is_valid_file() &&
		    ( (  !only_x  && !only_nx ) ||
		      ( ( only_x  && it->is_excluded() ) ) ||
		      ( ( only_nx && it->is_not_excluded() ) ) ) )
		{
			line_data& ld = tdata.at( i ) ;
			t1 = ld.get_string() ;
			t2 = it->get_idata() ;
			if ( RightBnd > 0 )
			{
				t1 = substr( t2, 1, LeftBnd-1 ) +
				     substr( t1, LeftBnd, ( RightBnd - LeftBnd + 1 ) ) +
				     substr( t2, RightBnd+1 ) ;
				trim_right( t1 ) ;
			}
			else if ( LeftBnd > 1 )
			{
				t1 = substr( t2, 1, LeftBnd-1 ) +
				     substr( t1, LeftBnd ) ;
			}
			if ( it->put_idata( t1, level, ID_CHNGO ) )
			{
				fileChanged = true  ;
				sorted      = false ;
			}
			if ( ld.line_type != LS_NONE )
			{
				it->set_condition( ld.line_type, level ) ;
			}
			if ( ld.line_label != "" )
			{
				for ( auto itl = data.begin() ; itl != data.end() ; ++itl )
				{
					if ( itl->clearLabel( ld.line_label, level ) ) { break ; }
				}
				it->setLabel( ld.line_label, level ) ;
			}
			( ld.line_excl ) ? it->set_excluded( level ) : it->set_unexcluded( level ) ;
			++i ;
		}
	}

	if ( sorted )
	{
		pcmd.set_msg( "PEDT014E", 4 ) ;
	}
	else
	{
		rebuildZAREA = true ;
	}
}


int pedit01::get_datawidth()
{
	//
	// Return the width of the data on the screen.
	//

	TRACE_FUNCTION() ;

	int i = zdataw ;

	if ( reclen > 0 )
	{
		i = reclen - startCol + 1 ;
		if      ( i < 0 )      { i = 0  ; }
		else if ( i > zdataw ) { i = zdataw ; }
	}

	return i ;
}


int pedit01::get_minsize()
{
	//
	// Return the minimum size for a new insert record.
	//

	TRACE_FUNCTION() ;

	return ( reclen > 0 ) ? reclen : max( size_t( startCol - 1 ), lnumSize1 ) ;
}


int pedit01::get_overflow_size()
{
	//
	// Return the overflow size for a record.
	//

	TRACE_FUNCTION() ;

	return ( reclen > 0 ) ? max( 0, int( reclen - ( zdataw + startCol - 1 + lnumSize2 ) ) ) :
				max( 0, int( MAXLEN - ( zdataw + startCol - 1 ) ) ) ;
}


void pedit01::setLineLabels()
{
	//
	// Process line labels before any commands.  This allows us to enter labels at the same time as a command
	// containing those labels.
	//

	TRACE_FUNCTION() ;

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->il_lcc.size() > 0 && it->il_lcc.front() == '.' )
		{
			if ( !checkLabel1( it->il_lcc ) )
			{
				pcmd.set_msg( "PEDT014", 12 ) ;
				cursor.set( itr2ptr( it ), CSR_FC_LCMD ) ;
				break ;
			}
			if ( !it->is_file() || it->is_excluded() )
			{
				pcmd.set_msg( "PEDT011E", 12 ) ;
				cursor.set( itr2ptr( it ), CSR_FC_LCMD ) ;
				it->il_lcc = "" ;
				break ;
			}
			for ( auto itl = data.begin() ; itl != data.end() ; ++itl )
			{
				if ( it != itl && itl->clearLabel( it->il_lcc, level ) ) { break ; }
			}
			if ( aLCMD )
			{
				cursor.set( itr2ptr( it ), CSR_FNB_DATA ) ;
			}
			it->setLabel( it->il_lcc, level ) ;
			it->clearLcc() ;
			rebuildZAREA = true ;
		}
	}
}


const string& pedit01::get_truename( const string& w )
{
	TRACE_FUNCTION() ;

	auto it = aliasNames.find( w ) ;
	return ( it == aliasNames.end() ) ? w : it->second ;
}


string pedit01::genNextLabel( string lab )
{
	//
	// Generate the next label in the sequence.
	// .OAAAA -> .OAAAB
	// .OAAAZ -> .OAABA
	//

	TRACE_FUNCTION() ;

	char* ch ;

	const string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;

	for ( int i = 5 ; i > 1 ; --i )
	{
		ch = &lab[ i ] ;
		if ( *ch == 'Z' )
		{
			*ch = 'A' ;
		}
		else
		{
			*ch = alpha[ alpha.find( *ch ) + 1 ] ;
			break ;
		}
	}

	return lab ;
}


void pedit01::set_zhivars()
{
	//
	// Set the following variables in the SHARED pool:
	//   ZHIAUTO
	//   ZHILANG
	//   ZHICOLOR
	//   ZHIPAREN
	//   ZHIFIND
	//   ZHICURSR
	//

	TRACE_FUNCTION() ;

	string zhicolor ;

	zhicolor = ( profHilight ) ?
		   ( profIfLogic ) ?
		   ( profDoLogic ) ? "LOGIC"   : "IFLOGIC" :
		   ( profDoLogic ) ? "DOLOGIC" : "ON"
				   : "OFF" ;

	set_shared_var( "ZHIAUTO", ( ( profLang == "AUTO" ) ? "ON" : "OFF" ) ) ;
	set_shared_var( "ZHILANG", detLang ) ;
	set_shared_var( "ZHICOLOR", zhicolor ) ;
	set_shared_var( "ZHIPAREN", OnOff[ profParen ] ) ;
	set_shared_var( "ZHIFIND", OnOff[ profFindPhrase ] ) ;
	set_shared_var( "ZHICURSR", OnOff[ profCsrPhrase ] ) ;
}


int pedit01::set_zdest()
{
	//
	// Get the A or B line commands for a PROCESS DEST macro statement
	// and set zdest to the relevant file line.  Default to last line.
	//
	//   RETURN:
	//    0  Normal completion.
	//    8  DEST expected but none specified - defaults set.
	//   16  Incomplete or conflicting commands entered.
	//   20  Severe error.
	//
	// Return codes from rc will not produce a fatal error.
	//

	TRACE_FUNCTION() ;

	int rept ;

	int rc = 0 ;

	string lcc ;

	zdest = nullptr ;

	Data::Iterator it = nullptr ;
	Data::Iterator ix = nullptr ;

	for ( it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->il_deleted || it->il_lcc == "" ) { continue ; }
		if ( !formLineCmd2( it->il_lcc, lcc, rept ) )
		{
			if ( !pcmd.msgset() )
			{
				pcmd.set_msg( "PEDT011Y", 20 ) ;
				miBlock.seterror( pcmd ) ;
				pcmd.cond_reset() ;
			}
			return 20 ;
		}
		if ( lcc != "A" && lcc != "B" )
		{
			continue ;
		}
		if ( it->not_valid_file() )
		{
			miBlock.seterror( "PEDM013H", 16 ) ;
			return 20 ;
		}
		if ( zdest )
		{
			miBlock.seterror( "PEDM013I", lcc, 16 ) ;
			return 20 ;
		}
		ix    = it ;
		zdest = itr2ptr( it ) ;
		if ( lcc == "B" )
		{
			zdest = getPrevDataLine( zdest ) ;
		}
	}

	if ( ix != nullptr )
	{
		ix->clearLcc() ;
	}
	else
	{
		zdest = getFileLineZLAST() ;
		rc    = ( zdest ) ? 8 : 20 ;
	}

	return rc ;
}


int pedit01::set_zranges( const string& r1,
			  const string& r2 )
{
	//
	// Get the PROCESS RANGE commands.
	//
	// Set ZFRANGE to the line of the start of the RANGE command (or the first file line if not entered).
	// Set ZLRANGE to the line of the end of the RANGE command (or the last file line if not entered).
	//
	// Range can be a block command by doubling the last character (must be 5 chars or less).
	//
	//   RETURN:
	//    0  Normal completion.
	//    4  RANGE expected but none specified - defaults set.
	//   16  Incomplete or conflicting commands entered.
	//   20  Severe error.
	//
	// Return codes from rc will not produce a fatal error.
	//

	TRACE_FUNCTION() ;

	int rept ;

	int rc = 0 ;

	bool single = false ;
	bool block  = false ;

	string lcc ;
	string lcb ;

	string rr1 ;
	string rr2 ;

	if ( r1 == "" )
	{
		miBlock.seterror( "PEDM013G", 20 ) ;
		return 20 ;
	}

	if ( r1.size() > 6 || r2.size() > 6 )
	{
		miBlock.seterror( "PEDM013J", 20 ) ;
		return 20 ;
	}

	if ( r1.size() <= 5 )
	{
		rr1 = r1 ;
		rr1.push_back( r1.back() ) ;
	}

	if ( r2.size() > 0 && r2.size() <= 5 )
	{
		rr2 = r2 ;
		rr2.push_back( r2.back() ) ;
	}

	Data::Iterator it = nullptr ;
	Data::Iterator ix = nullptr ;
	Data::Iterator iy = nullptr ;
	Data::Iterator iz = nullptr ;

	zrange_cmd = "" ;
	zfrange    = nullptr ;
	zlrange    = nullptr ;

	for ( it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->il_deleted || it->il_lcc == "" ) { continue ; }
		if ( !formLineCmd2( it->il_lcc, lcc, rept ) )
		{
			if ( !pcmd.msgset() )
			{
				pcmd.set_msg( "PEDT011Y", 20 ) ;
				miBlock.seterror( pcmd ) ;
				pcmd.cond_reset() ;
			}
			return 20 ;
		}
		if ( ( lcc == r1 ) || ( r2 != "" && lcc == r2 ) )
		{
			if ( it->not_valid_file() )
			{
				miBlock.seterror( "PEDM013H", 16 ) ;
				return 20 ;
			}
			if ( block || zfrange )
			{
				miBlock.seterror( "PEDM013I", lcc, 16 ) ;
				return 20 ;
			}
			zrange_cmd = lcc ;
			for ( iz = it ; iz != data.end() && rept > 1 ; --rept )
			{
				iz = getNextFileLine( iz ) ;
			}
			if ( iz == data.end() )
			{
				iz = getFileLineZLAST() ;
			}
			zfrange = itr2ptr( it ) ;
			zlrange = itr2ptr( iz ) ;
			single  = true ;
			ix      = it ;
		}
		else if ( ( rr1 != "" && lcc == rr1 ) || ( rr2 != "" && lcc == rr2 ) )
		{
			if ( it->not_valid_file() )
			{
				miBlock.seterror( "PEDM013H", 16 ) ;
				return 20 ;
			}
			if ( single )
			{
				miBlock.seterror( "PEDM013I", lcc, 16 ) ;
				return 20 ;
			}
			if ( rept > 1 )
			{
				miBlock.seterror( "PEDT012A", 16 ) ;
				return 20 ;
			}
			if ( block )
			{
				if ( lcc != lcb || zlrange )
				{
					miBlock.seterror( "PEDM013I", lcc, 16 ) ;
					return 20 ;
				}
				zlrange = itr2ptr( it ) ;
				iy      = it ;
			}
			else
			{
				zfrange = itr2ptr( it ) ;
				block   = true ;
				lcb     = lcc ;
				ix      = it ;
			}
			zrange_cmd = ( lcc == rr1 ) ? r1 : r2 ;
		}
	}

	if ( zfrange && !zlrange )
	{
		miBlock.seterror( "PEDM013K", lcb, 16 ) ;
		return 20 ;
	}

	if ( !zfrange )
	{
		zfrange = getFileLineZFIRST() ;
		zlrange = getFileLineZLAST() ;
		rc      = ( zfrange ) ? 4 : 20 ;
	}
	else
	{
		if ( ix != nullptr )
		{
			ix->clearLcc() ;
		}
		if ( iy != nullptr )
		{
			iy->clearLcc() ;
		}
	}

	return rc ;
}


bool pedit01::storeLineCommands()
{
	//
	// For each line in the data vector, check the line commands entered and build the line command vector.
	// Treat commands in XBlock that are on an exluded block, as a block command (eg R, <, >, etc)
	//
	// Special processing for TJ on non-excluded lines, as TJn implies TJ(n+1).
	//
	// cmd   - command block.
	// tcmd  - temporary command block for storing cmd when complete, and a non-abo command entered
	//         ie. Block/single commands without A/B/O bewteen block/single commands with A/B/O.
	// abo   - contains a, b or o M/C positions, including OO block command, and xK commands.
	// tabo  - temporary abo command block used during AK, BK, OK and OOK operations (pushed to tabos stack).
	// tabos - command block stack for storing tabo for AK, BK, OK and OOK operations.
	//
	// Check for overlapping commands (eg. Cn, Dn, Mn, On ... don't conflict with the next command).
	//
	// Translate a swap (W/WW) into two move commands.
	//

	TRACE_FUNCTION() ;

	lcmd cmd  ;
	lcmd tcmd ;
	lcmd abo  ;
	lcmd tabo ;

	stack<lcmd> tabos ;

	int rept  ;
	int adist = 1 ;
	int rdist = -1 ;

	string lcc ;

	bool cmd_inuse    = false ;
	bool abo_inuse    = false ;
	bool cmd_complete = false ;
	bool abo_complete = false ;
	bool abo_block    = false ;
	bool abo_k        = false ;

	Data::Iterator it = nullptr ;

	lcmds.clear() ;

	for ( it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->il_deleted    ) { continue ; }
		++adist ;
		if ( it->il_lcc == ""  ) { continue ; }
		if ( rdist != -1 && rdist >= adist )
		{
			pcmd.set_msg( "PEDT014C", 12 ) ;
			break ;
		}
		rdist = -1 ;
		if ( !formLineCmd1( it->il_lcc, lcc, rept ) )
		{
			if ( !pcmd.msgset() )
			{
				pcmd.set_msg( "PEDT011Y", 12 ) ;
			}
			break ;
		}
		if ( ( it->is_tod() && todCmds.count( lcc ) == 0 ) ||
		     ( it->is_bod() && bodCmds.count( lcc ) == 0 ) )
		{
			pcmd.set_msg( "PEDT013", 12 ) ;
			break ;
		}
		if ( abowList.count( lcc ) > 0 )
		{
			if ( cmd_inuse && !cmd_complete )
			{
				pcmd.set_msg( "PEDT011W", 8 ) ;
				cursor.set( aADDR, CSR_LC_OR_FNB_DATA, 0, aLCMD ) ;
				return false ;
			}
			if ( abokLCMDS.count( lcc ) > 0 )
			{
				abo_k = true ;
				lcc   = abokLCMDS[ lcc ] ;
			}
			else
			{
				abo_k = false ;
			}
			if ( not abo_inuse )
			{
				if ( cmd_inuse && !cmd_complete )
				{
					pcmd.set_msg( "PEDT011Y", 12 ) ;
					break ;
				}
				abo.lcmd_ABOW = lcc.front() ;
				if ( lcc == "A" && it->is_excluded() )
				{
					abo.lcmd_sADDR = itr2ptr( getLastEX( it ) ) ;
				}
				else
				{
					abo.lcmd_sADDR = itr2ptr( it ) ;
				}
				abo.lcmd_Rpt   = rept ;
				abo.lcmd_ABRpt = rept ;
				abo_inuse      = true ;
				if ( owBlock.count( lcc) > 0 )
				{
					abo_block = true ;
				}
				else
				{
					if ( lcc == "O" || lcc == "W" )
					{
						abo.lcmd_eADDR = getLastADDR( it, rept ) ;
					}
					if ( abo_k )
					{
						tabos.push( abo ) ;
						abo_inuse = false ;
						abo.lcmd_clear()  ;
					}
					else
					{
						abo_complete = true ;
					}
				}
			}
			else
			{
				if ( owBlock.count( lcc ) == 0 ||
				     !abo_block                ||
				      abo.lcmd_eADDR           ||
				     lcc.front() != abo.lcmd_ABOW )
				{
					pcmd.set_msg( "PEDT011Y", 12 ) ;
					break ;
				}
				if ( it->is_excluded() )
				{
					abo.lcmd_eADDR = itr2ptr( getLastEX( it ) ) ;
				}
				else
				{
					abo.lcmd_eADDR = itr2ptr( it ) ;
				}
				if ( abo_k ) { tabos.push( abo ) ; abo_inuse = false ; abo.lcmd_clear() ; }
				else         { abo_complete = true ; }
			}
		}
		else if ( blkCmds.count( lcc ) > 0 )
		{
			if ( cmd_inuse && !cmd_complete && cmd.lcmd_cmd != lineCmds.at( lcc ) )
			{
				pcmd.set_msg( "PEDT011Y", 12 ) ;
				break ;
			}
			if ( !cmd_inuse )
			{
				cmd.lcmd_cmd    = lineCmds.at( lcc ) ;
				cmd.lcmd_cmdstr = lcc ;
				cmd.lcmd_sADDR  = itr2ptr( it ) ;
				cmd.lcmd_Rpt    = rept ;
				cmd_inuse       = true ;
			}
			else
			{
				if ( cmd.lcmd_Rpt != -1 && rept != -1 && cmd.lcmd_Rpt != rept )
				{
					pcmd.set_msg( "PEDT012B", 12 ) ;
					break ;
				}
				auto itx = lmacs.find( lcc ) ;
				if ( itx == lmacs.end() && cmd_complete && abokReq.count( lcc) > 0 )
				{
					pcmd.set_msg( "PEDT011Y", 12 ) ;
					break ;
				}
				if ( cmd_complete )
				{
					tcmd = cmd ;
					cmd.lcmd_clear() ;
					cmd.lcmd_cmd    = lineCmds.at( lcc ) ;
					cmd.lcmd_cmdstr = lcc ;
					cmd.lcmd_sADDR  = itr2ptr( it ) ;
					cmd.lcmd_Rpt    = rept  ;
					cmd_inuse       = true  ;
					cmd_complete    = false ;
				}
				else
				{
					if ( cmd.lcmd_Rpt == -1 ) { cmd.lcmd_Rpt = rept ; }
					if ( it->is_excluded() )
					{
						cmd.lcmd_eADDR = itr2ptr( getLastEX( it ) ) ;
					}
					else
					{
						cmd.lcmd_eADDR = itr2ptr( it ) ;
					}
					if ( abokReq.count( lcc ) > 0 )
					{
						cmd_complete = true ;
					}
					else
					{
						lcmds.push_back( cmd ) ;
						if ( tcmd.lcmd_sADDR )
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
			if ( checkDst.count( lcc ) > 0 ) { adist = 1 ; rdist = rept ; }
			else                             { rdist = -1               ; }
			auto itx = lmacs.find( lcc ) ;
			if ( ( cmd_inuse && !cmd_complete ) ||
			     ( itx == lmacs.end() && cmd_complete && abokReq.count( lcc ) > 0 ) )
			{
				pcmd.set_msg( "PEDT011Y", 12 ) ;
				break ;
			}
			if ( cmd_complete )
			{
				tcmd = cmd ;
				cmd.lcmd_clear() ;
			}
			if ( lcc == "TJ" && it->is_not_excluded() )
			{
				if ( rept == -1 ) { rept = 1 ; }
				++rept ;
			}
			cmd.lcmd_sADDR = itr2ptr( it ) ;
			if ( lcc == "I" && it->is_excluded() )
			{
				cmd.lcmd_sADDR = getLastADDR( it, 1 ) ;
			}
			cmd.lcmd_eADDR  = getLastADDR( it, rept ) ;
			cmd.lcmd_Rpt    = rept ;
			cmd.lcmd_cmdstr = lcc  ;
			if ( it->is_excluded() )
			{
				if ( XBlock.count( lcc ) > 0 )
				{
					cmd.lcmd_cmdstr += lcc ;
					if ( aliasLCMDS.count( cmd.lcmd_cmdstr ) > 0 )
					{
						cmd.lcmd_cmdstr = aliasLCMDS[ cmd.lcmd_cmdstr ] ;
					}
					cmd.lcmd_eADDR = itr2ptr( getLastEX( it ) ) ;
				}
			}
			cmd.lcmd_cmd = lineCmds.at( cmd.lcmd_cmdstr ) ;
			if ( abokReq.count( lcc ) > 0 )
			{
				cmd_complete = true ;
				cmd_inuse    = true ;
			}
			else
			{
				lcmds.push_back( cmd ) ;
				cmd.lcmd_clear()       ;
				if ( tcmd.lcmd_sADDR ) { cmd = tcmd ; tcmd.lcmd_clear() ; }
				else                   { cmd_complete = false           ; }
			}
		}
		if ( cmd_complete && abo_complete )
		{
			if ( lmacs.count( cmd.lcmd_cmdstr ) > 0 )
			{
				if ( !tabos.empty() )
				{
					pcmd.set_msg( "PEDT016J", 12 ) ;
					break ;
				}
				else if ( abo.lcmd_ABRpt > 1 )
				{
					pcmd.set_msg( "PEDT012A", 12 ) ;
					break ;
				}
			}
			while ( !tabos.empty() )
			{
				tabo = cmd ;
				tabo.lcmd_ABOW   = tabos.top().lcmd_ABOW  ;
				tabo.lcmd_dADDR  = tabos.top().lcmd_sADDR ;
				tabo.lcmd_lADDR  = tabos.top().lcmd_eADDR ;
				tabo.lcmd_ABRpt  = tabos.top().lcmd_ABRpt ;
				tabo.lcmd_cmd    = LC_C ;
				tabo.lcmd_cmdstr = "C"  ;
				lcmds.push_back( tabo ) ;
				tabos.pop()             ;
			}
			cmd.lcmd_ABOW  = abo.lcmd_ABOW  ;
			cmd.lcmd_dADDR = abo.lcmd_sADDR ;
			cmd.lcmd_lADDR = abo.lcmd_eADDR ;
			cmd.lcmd_ABRpt = abo.lcmd_ABRpt ;
			if ( cmd.lcmd_ABOW == 'W' )
			{
				if ( cmd.lcmd_cmdstr.front() != 'M' )
				{
					pcmd.set_msg( "PEDT014H", 12 ) ;
					break ;
				}
				cmd.lcmd_ABOW  = 'B'   ;
				cmd.lcmd_ABRpt = 1     ;
				lcmds.push_back( cmd ) ;
				cmd.lcmd_dADDR = cmd.lcmd_sADDR ;
				cmd.lcmd_lADDR = cmd.lcmd_eADDR ;
				cmd.lcmd_sADDR = abo.lcmd_sADDR ;
				cmd.lcmd_eADDR = abo.lcmd_eADDR ;
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
		else if ( extdata.is_create() && cmd_complete )
		{
			cmd.set( extdata ) ;
			lcmds.push_back( cmd ) ;
			cmd.lcmd_clear()     ;
			cmd_inuse    = false ;
			cmd_complete = false ;
			extdata.reset() ;
		}
		else if ( extdata.is_replace() && cmd_complete )
		{
			cmd.set( extdata ) ;
			lcmds.push_back( cmd ) ;
			cmd.lcmd_clear()     ;
			cmd_inuse    = false ;
			cmd_complete = false ;
			extdata.reset() ;
		}
		else if ( pasteActive && abo_complete )
		{
			abo.lcmd_cmd = LC_PASTE ;
			lcmds.push_back( abo )  ;
			abo.lcmd_clear()     ;
			abo_inuse    = false ;
			abo_complete = false ;
			abo_block    = false ;
			pasteActive  = false ;
		}
		else if ( copyActive && abo_complete )
		{
			abo.lcmd_cmd = LC_COPY ;
			lcmds.push_back( abo ) ;
			abo.lcmd_clear()     ;
			abo_inuse    = false ;
			abo_complete = false ;
			abo_block    = false ;
			copyActive   = false ;
		}
		else if ( moveActive && abo_complete )
		{
			abo.lcmd_cmd = LC_MOVE ;
			lcmds.push_back( abo ) ;
			abo.lcmd_clear()     ;
			abo_inuse    = false ;
			abo_complete = false ;
			abo_block    = false ;
			moveActive   = false ;
		}
	}

	if ( it != data.end() )
	{
		cursor.set( itr2ptr( it ), CSR_FC_LCMD ) ;
		return false ;
	}

	if ( cmd_inuse )
	{
		if ( cmd_complete )
		{
			if ( lmacs.count( lcc ) == 0 )
			{
				pcmd.set_msg( "PEDT014P", 8 ) ;
				return false ;
			}
			else
			{
				lcmds.push_back( cmd ) ;
			}
		}
		else
		{
			pcmd.set_msg( "PEDT011W", 8 ) ;
			return false ;
		}
	}

	if ( abo_inuse || !tabos.empty() )
	{
		pcmd.set_msg( "PEDT014Q", 8 ) ;
		cursor.set( aADDR, CSR_LC_OR_FNB_DATA, 0, aLCMD ) ;
		return false ;
	}

	if ( cutActive )
	{
		cmd.lcmd_cmd   = LC_C  ;
		cmd.lcmd_cut   = true  ;
		cmd.lcmd_sADDR = data.top() ;
		cmd.lcmd_eADDR = data.bottom() ;
		cutActive      = false ;
		lcmds.push_back( cmd ) ;
	}

	if ( extdata.is_create() )
	{
		cmd.set( extdata ) ;
		cmd.lcmd_cmd    = LC_C  ;
		cmd.lcmd_sADDR  = data.top() ;
		cmd.lcmd_eADDR  = data.bottom() ;
		lcmds.push_back( cmd )  ;
		extdata.reset() ;
	}

	if ( extdata.is_replace() )
	{
		cmd.set( extdata ) ;
		cmd.lcmd_cmd    = LC_C  ;
		cmd.lcmd_sADDR  = data.top() ;
		cmd.lcmd_eADDR  = data.bottom() ;
		lcmds.push_back( cmd )  ;
		extdata.reset() ;
	}

	if ( pasteActive )
	{
		if ( !any_of( data.begin(), data.end(),
			[](iline& a)
			{
				return a.is_valid_file() ;
			} ) )
		{
			abo.lcmd_sADDR = data.bottom() ;
			abo.lcmd_ABOW  = 'B'      ;
			abo.lcmd_cmd   = LC_PASTE ;
			lcmds.push_back( abo ) ;
			pasteActive  = false   ;
		}
		else
		{
			pcmd.set_msg( "PEDT013F", 8 ) ;
			pcmd.keep_cmd() ;
			return false ;
		}
	}

	if ( copyActive || moveActive )
	{
		if ( !any_of( data.begin(), data.end(),
			[](iline& a)
			{
				return !a.il_deleted && a.is_file() ;
			} ) )
		{
			abo.lcmd_sADDR = data.bottom() ;
			abo.lcmd_ABOW  = 'B' ;
			abo.lcmd_cmd   = ( copyActive ) ? LC_COPY : LC_MOVE ;
			lcmds.push_back( abo ) ;
			copyActive     = false ;
			moveActive     = false ;
		}
		else
		{
			pcmd.set_msg( "PEDT015I", ( copyActive ) ? "COPY" : "MOVE", 8 ) ;
			pcmd.keep_cmd() ;
			return false ;
		}
	}

	return true ;
}


bool pedit01::action_UNDO()
{
	//
	// Get the current data level from Global_Undo and call undo_idata() for all records
	// that match that level.  Move level from Global_Undo to Global_Redo.
	//
	// If any file lines have been undone, remove the top Global_File_level entry and
	// reset fileChanged indicator.
	//
	// Move the top line to the first undone change or top-of-data, if the line is on the first page.
	//

	TRACE_FUNCTION() ;

	int lvl ;

	iline* tADDR = 0 ;

	bool isFile = false ;
	bool isProf = false ;
	bool undo   = false ;

	SS_TYPE sstype ;

	vector<string> Prof ;

	if ( !profUndoOn )
	{
		pcmd.set_msg( "PEDT011U", 12 ) ;
		return false ;
	}

	if ( zchanged )
	{
		pcmd.set_msg( "PEDT016K", 12 ) ;
		cursor.home() ;
		return false ;
	}

	lvl = iline::get_Global_Undo_level( taskid() ) ;
	if ( lvl < 1 )
	{
		pcmd.set_msg( "PEDT017", 4 ) ;
		return false ;
	}

	iline::move_Global_Undo2Redo( taskid() ) ;

	while ( iline::get_status_level( taskid() ) == lvl )
	{
		sstype = iline::status_get_type( taskid() ) ;
		iline::status_undo( taskid() ) ;
		restore_status( sstype ) ;
		undo = true ;
	}

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->get_idata_level() == lvl )
		{
			it->undo_idata() ;
			it->setUndoStatus() ;
			if ( it->is_file() ) { isFile = true ; }
			if ( !tADDR )
			{
				tADDR = itr2ptr( it ) ;
			}
			if ( it->is_prof() )
			{
				isProf = true ;
			}
			rebuildZAREA = true ;
			undo         = true ;
		}
		if ( it->get_icond_level() == lvl )
		{
			it->undo_icond() ;
			if ( !tADDR )
			{
				tADDR = itr2ptr( it ) ;
			}
			rebuildZAREA = true ;
			undo         = true ;
			iline::reset_changed_icond( taskid() ) ;
		}
		if ( it->get_ilabel_level() == lvl )
		{
			it->undo_ilabel() ;
			if ( !tADDR )
			{
				tADDR = itr2ptr( it ) ;
			}
			rebuildZAREA = true ;
			undo         = true ;
			iline::reset_changed_ilabel( taskid() ) ;
		}
		if ( it->get_iexcl_level() == lvl )
		{
			it->undo_iexcl() ;
			if ( !tADDR )
			{
				tADDR = itr2ptr( it ) ;
			}
			rebuildZAREA = true ;
			undo         = true ;
			iline::reset_changed_xclud( taskid() ) ;
		}
	}

	level = iline::get_Global_Undo_level( taskid() ) + 1 ;

	if ( isFile )
	{
		iline::remove_Global_File_level( taskid() ) ;
		fileChanged = ( iline::get_Global_File_level( taskid() ) != saveLevel ) ;
	}

	clr_hilight_shadow() ;

	if ( isProf )
	{
		buildProfLines( Prof ) ;
		updateProfLines( Prof ) ;
	}

	if ( tADDR && !ADDROnScreen( tADDR, topLine ) )
	{
		topLine = ADDROnScreen( tADDR, 0 ) ? data.top() : tADDR ;
	}

	if ( !undo )
	{
		undo = action_UNDO() ;
	}

	return undo ;
}


bool pedit01::action_REDO()
{
	//
	// Get the current data level from Global_Redo and call redo_idata() for all records
	// that match that level.  Move level from Global_Redo to Global_Undo.
	//
	// If any file lines have been redone, update the Global_File_level from the Global_Undo stack and
	// reset fileChanged indicator.
	//
	// Move the top line to the first redone change or top-of-data, if the line is on the first page.
	//

	TRACE_FUNCTION() ;

	int  lvl ;

	iline* tADDR = nullptr ;

	bool isFile = false ;
	bool isProf = false ;
	bool redo   = false ;

	SS_TYPE sstype ;

	vector<string> Prof ;

	if ( !profUndoOn )
	{
		pcmd.set_msg( "PEDT011U", 12 ) ;
		return false ;
	}

	if ( zchanged )
	{
		pcmd.set_msg( "PEDT016K", 12 ) ;
		cursor.home() ;
		return false ;
	}

	lvl = iline::get_Global_Redo_level( taskid() ) ;
	if ( lvl < 1 )
	{
		pcmd.set_msg( "PEDT018", 4 ) ;
		return false ;
	}

	iline::move_Global_Redo2Undo( taskid() ) ;

	while ( iline::get_status_redo_level( taskid() ) == lvl )
	{
		iline::status_redo( taskid() ) ;
		sstype = iline::status_get_type( taskid() ) ;
		restore_status( sstype ) ;
		redo = true ;
	}

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->get_idata_Redo_level() == lvl )
		{
			it->redo_idata() ;
			it->setRedoStatus() ;
			if ( it->is_file() ) { isFile = true ; }
			if ( !tADDR )
			{
				tADDR = itr2ptr( it ) ;
			}
			if ( it->is_prof() )
			{
				isProf = true ;
			}
			rebuildZAREA = true ;
			redo         = true ;
		}
		if ( it->get_icond_Redo_level() == lvl )
		{
			it->redo_icond() ;
			if ( !tADDR )
			{
				tADDR = itr2ptr( it ) ;
			}
			rebuildZAREA = true ;
			redo         = true ;
			iline::reset_changed_icond( taskid() ) ;
		}
		if ( it->get_ilabel_Redo_level() == lvl )
		{
			it->redo_ilabel() ;
			if ( !tADDR )
			{
				tADDR = itr2ptr( it ) ;
			}
			rebuildZAREA = true ;
			redo         = true ;
			iline::reset_changed_ilabel( taskid() ) ;
		}
		if ( it->get_iexcl_Redo_level() == lvl )
		{
			it->redo_iexcl() ;
			if ( !tADDR )
			{
				tADDR = itr2ptr( it ) ;
			}
			rebuildZAREA = true ;
			redo         = true ;
			iline::reset_changed_xclud( taskid() ) ;
		}
	}

	level = iline::get_Global_Undo_level( taskid() ) + 1 ;

	if ( isFile )
	{
		iline::set_Global_File_level( taskid() ) ;
		fileChanged = ( iline::get_Global_File_level( taskid() ) != saveLevel ) ;
	}

	clr_hilight_shadow() ;

	if ( isProf )
	{
		buildProfLines( Prof ) ;
		updateProfLines( Prof ) ;
	}

	if ( tADDR && !ADDROnScreen( tADDR, topLine ) )
	{
		topLine = ADDROnScreen( tADDR, 0 ) ? data.top() : tADDR ;
	}

	if ( !redo )
	{
		redo = action_REDO() ;
	}

	return redo ;
}


void pedit01::store_status( SS_TYPE type )
{
	//
	// Store the profile status so we can do UNDO/REDO on profile parameters.
	//

	TRACE_FUNCTION() ;

	profile prof ;

	load_profile( prof ) ;

	iline::status_put( taskid(), type, prof, LeftBnd, RightBnd, LeftBndSet, RightBndSet ) ;
}


void pedit01::update_status()
{
	//
	// Called on a NUMBER change.
	//
	// Normally bounds changes are not undo/redo'able but we need to store them
	// when the NUMBER status changes so they will be correct on an UNDO so
	// update the istatus object before stacking the new NUMBER status.
	//

	TRACE_FUNCTION() ;

	iline::status_update( taskid(), LeftBnd, RightBnd, LeftBndSet, RightBndSet ) ;
}


void pedit01::restore_status( SS_TYPE type )
{
	//
	// Restore the profile status for UNDO/REDO processing.
	//

	TRACE_FUNCTION() ;

	vector<string> Prof ;

	const profile& p = iline::status_get_profile( taskid() ) ;

	switch ( type )
	{
	case SS_NUMBER:
		profNum     = p.profNum ;
		profNumSTD  = p.profNumSTD ;
		profNumCBL  = p.profNumCBL ;
		LeftBnd     = iline::status_get_LeftBnd( taskid() ) ;
		RightBnd    = iline::status_get_RightBnd( taskid() ) ;
		LeftBndSet  = iline::status_get_LeftBndSet( taskid() ) ;
		RightBndSet = iline::status_get_RightBndSet( taskid() ) ;
		set_num_parameters() ;
		if ( !profNumDisp )
		{
			startCol = lnumSize1 + 1 ;
		}
		break ;

	case SS_AUTONUM:
		profAutoNum = p.profAutoNum ;
		break ;

	case SS_AUTOSAVE:
		profAutoSave = p.profAutoSave ;
		profSaveP    = p.profSaveP ;
		break ;

	case SS_CAPS:
		profCaps = p.profCaps ;
		break ;

	case SS_HEX:
		profHex  = p.profHex  ;
		profVert = p.profVert ;
		rebuildZAREA = true ;
		break ;

	case SS_HILITE:
		profLang       = p.profLang ;
		profHilight    = p.profHilight ;
		profIfLogic    = p.profIfLogic ;
		profDoLogic    = p.profDoLogic ;
		profParen      = p.profParen ;
		profFindPhrase = p.profFindPhrase ;
		profCsrPhrase  = p.profCsrPhrase  ;
		rebuildZAREA   = true ;
		break ;

	case SS_IMACRO:
		profIMACRO = p.profIMACRO ;
		break ;

	case SS_NOTES:
		profNotes = p.profNotes ;
		break ;

	case SS_PROFILE:
		if ( zedprof != p.profName )
		{
			loadEditProfile( p.profName ) ;
		}
		else
		{
			profLock = p.profLock ;
		}
		rebuildZAREA = true ;
		break ;

	case SS_NULLS:
		profNullA = p.profNullA ;
		profNulls = p.profNulls ;
		rebuildZAREA = true ;
		break ;

	case SS_TABS:
		profTabs  = p.profTabs  ;
		profATabs = p.profATabs ;
		break ;

	case SS_STATS:
		profStats = p.profStats ;
		break ;

	case SS_XTABS:
		profXTabs = p.profXTabs ;
		break ;

	}

	buildProfLines( Prof ) ;
	updateProfLines( Prof ) ;
}


void pedit01::load_profile( profile& prof )
{
	TRACE_FUNCTION() ;

	prof.profName       = zedprof        ;
	prof.profAutoSave   = profAutoSave   ;
	prof.profSaveP      = profSaveP      ;
	prof.profNullA      = profNullA      ;
	prof.profNulls      = profNulls      ;
	prof.profLock       = profLock       ;
	prof.profCaps       = profCaps       ;
	prof.profHex        = profHex        ;
	prof.profTabs       = profTabs       ;
	prof.profATabs      = profATabs      ;
	prof.profXTabs      = profXTabs      ;
	prof.profXTabz      = profXTabz      ;
	prof.profRecovery   = profRecovery   ;
	prof.profBackup     = profBackup     ;
	prof.profHilight    = profHilight    ;
	prof.profIfLogic    = profIfLogic    ;
	prof.profDoLogic    = profDoLogic    ;
	prof.profLogic      = profLogic      ;
	prof.profParen      = profParen      ;
	prof.profCutReplace = profCutReplace ;
	prof.profPasteKeep  = profPasteKeep  ;
	prof.profPosFcx     = profPosFcx     ;
	prof.profLang       = profLang       ;
	prof.profIMACRO     = profIMACRO     ;
	prof.profVert       = profVert       ;
	prof.profFindPhrase = profFindPhrase ;
	prof.profCsrPhrase  = profCsrPhrase  ;
	prof.profUndoKeep   = profUndoKeep   ;
	prof.profNum        = profNum        ;
	prof.profNumSTD     = profNumSTD     ;
	prof.profNumCBL     = profNumCBL     ;
	prof.profNumDisp    = profNumDisp    ;
	prof.profAutoNum    = profAutoNum    ;
	prof.profNotes      = profNotes      ;
	prof.profStats      = profStats      ;
}


bool pedit01::setFindChangeExcl( char ftyp,
				 bool keep )
{
	//
	// Set the edit_find structure with the find/change/exclude command entered.
	//
	// BUG: Doesn't work well with some PICTURE strings specifying WORD/PREFIX/SUFFIX.
	//      Generated regex depends on whether the chars are alphanumeric or not for WORD/PREFIX/SUFFIX
	//      but this cannot be determined at this stage for PICTURE strings with =, - and x'AC'
	//      starting/ending characters.  Need a cleverer regex.
	//

	TRACE_FUNCTION() ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	bool t_dir  ;
	bool t_excl ;
	bool t_mtch ;
	bool t_str1 ;
	bool t_str2 ;
	bool alnum  ;
	bool alnum1 ;
	bool alnum2 ;

	char c      ;

	string cmd  ;
	string r1   ;
	string r2   ;
	string w1   ;
	string pic  ;
	string pre1 ;
	string pre2 ;
	string suf1 ;
	string rcmd1 ;
	string rcmd2 ;

	edit_find t ;
	cmd_range r ;

	const string f_keywexcl  = "X NX" ;
	const string f_keywdir   = "NEXT PREV FIRST LAST ALL" ;
	const string f_keywmtch  = "CHARS PRE PREFIX SUF SUFFIX WORD" ;
	const string pic_chars   = "=@#$¬.-<>" ;
	const string regex_chars = "\\^$.|?*+()[{" ;

	t_dir  = false ;
	t_excl = false ;
	t_mtch = false ;
	t_str1 = false ;
	t_str2 = false ;

	r.c_vlab = true ;
	r.c_vcol = true ;

	str = fcxList[ ftyp ] ;
	pcmd.clear_msg() ;
	cmd = pcmd.get_parms() ;
	trim( cmd ) ;
	if ( cmd == "" )
	{
		set_macro_var( "STR", str ) ;
		pcmd.set_msg( "PEDT013U", 20 ) ;
		return false ;
	}

	rcmd1 = "" ;
	rcmd2 = "" ;
	r1    = "" ;
	r2    = "" ;

	while ( cmd.size() > 0 )
	{
		w1 = upper( word( cmd, 1 ) ) ;
		p1 = w1.find_first_of( "'\"" ) ;
		if ( p1 != string::npos )
		{
			p2 = cmd.find( w1[ p1 ], p1+1 ) ;
			if ( p2 == string::npos )
			{
				pcmd.set_msg( "PEDT011H", 20 ) ;
				return false ;
			}
			p3   = cmd.find( ' ', p2 ) ;
			pre1 = cmd.substr( 0, p1 ) ;
			pre2 = cmd.substr( p2+1, p3-p2-1 ) ;
			trim( pre2 ) ;
			if ( pre1 != "" && pre2 != "" )
			{
				pcmd.set_msg( "PEDT011I", 20 ) ; return false ;
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
				else                     { pcmd.set_msg( "PEDT011I", 20 ) ; return false ; }
				t.f_string = cmd.substr( (p1+1), (p2-p1-1) ) ;
				if ( t.f_text ) { iupper( t.f_string ) ; }
			}
			else if ( !t_str2 )
			{
				if ( ftyp != 'C' ) { pcmd.set_msg( "PEDT011B", 20 ) ; return false ; }
				t_str2 = true ;
				if      ( pre1 == ""   ) { t.f_ctext = true ; }
				else if ( pre1 == "T"  ) { t.f_ctext = true ; }
				else if ( pre1 == "C"  ) { t.f_casis = true ; }
				else if ( pre1 == "X"  ) { t.f_chex  = true ; }
				else if ( pre1 == "P"  ) { t.f_cpic  = true ; }
				else                     { pcmd.set_msg( "PEDT011I", 20 ) ; return false ; }
				t.f_cstring = cmd.substr( (p1+1), (p2-p1-1) ) ;
				if ( profCaps && t.f_ctext ) { iupper( t.f_cstring ) ; }
			}
			else
			{
				pcmd.set_msg( "PEDT011", 20 ) ;
				return false ;
			}
			cmd.erase( 0, p3 ) ;
			trim_left( cmd )   ;
			continue ;
		}
		if ( findword( w1, f_keywdir ) )
		{
			if ( t_dir )
			{
				pcmd.set_msg( "PEDT013J", str, 20 ) ;
				return false ;
			}
			t.f_set_dir( w1 ) ;
			t_dir  = true ;
			rcmd2 += " " + w1 ;
		}
		else if ( findword( get_truename( w1 ), f_keywexcl ) )
		{
			if ( t_excl || ftyp == 'X' )
			{
				pcmd.set_msg( "PEDT013J", str, 20 ) ;
				return false ;
			}
			t_excl = true ;
			t.f_set_excl( get_truename( w1 ).front() ) ;
			rcmd2 += " " + w1 ;
		}
		else if ( findword( w1, f_keywmtch ) )
		{
			if ( t_mtch )
			{
				pcmd.set_msg( "PEDT013J", str, 20 ) ;
				return false ;
			}
			if ( t.f_reg )
			{
				pcmd.set_msg( "PEDT016D", 20 ) ;
				return false ;
			}
			t.f_set_match( w1 ) ;
			t_mtch = true ;
			rcmd2 += " " + w1 ;
		}
		else if ( isnumeric( w1 ) )
		{
			r1 += " " + w1 ;
		}
		else if ( w1.front() == '.' )
		{
			r2 += " " + w1 ;
		}
		else if ( !t_str1 )
		{
			t_str1 = true ;
			if ( w1 == "*" )
			{
				if ( fcx_parms.f_string == "" )
				{
					pcmd.set_msg( "PEDT011C", 20 ) ;
					return false ;
				}
				t.f_string = fcx_parms.f_ostring ;
				t.f_regreq = fcx_parms.f_regreq ;
				t.f_text   = fcx_parms.f_text ;
				t.f_asis   = fcx_parms.f_asis ;
				t.f_hex    = fcx_parms.f_hex  ;
				t.f_pic    = fcx_parms.f_pic  ;
			}
			else
			{
				t.f_text   = true ;
				t.f_string = w1   ;
			}
		}
		else if ( !t_str2 )
		{
			if ( ftyp != 'C' ) { pcmd.set_msg( "PEDT011B", 20 ) ; return false ; }
			t.f_cstring = ( profCaps ) ? w1 : word( cmd, 1 ) ;
			t.f_ctext   = true ;
			t_str2      = true ;
		}
		else
		{
			pcmd.set_msg( "PEDT013J", str, 20 ) ;
			return false ;
		}
		idelword( cmd, 1, 1 ) ;
		trim( cmd ) ;
	}

	if ( ftyp == 'C' && !t_str2 )
	{
		pcmd.set_msg( "PEDT011D", 20 ) ;
		return false ;
	}

	rcmd1 = strip( r2 + " " + r1 ) ;

	if ( t.f_string == "" )
	{
		if ( rcmd1 == "" )
		{
			if ( words( rcmd2 ) > 1 )
			{
				pcmd.set_msg( "PEDT011D", 20 ) ;
				return false ;
			}
			t.f_string = strip( rcmd2 ) ;
			t.f_set_next() ;
			t.f_set_chars() ;
			t.f_set_excl( 'A' ) ;
		}
		else if ( words( r1 ) > 2 )
		{
			pcmd.set_msg( "PEDT019", 20 ) ;
			return false ;
		}
		else if ( words( r2 ) > 2 )
		{
			pcmd.set_msg( "PEDT011A", 20 ) ;
			return false ;
		}
		else if ( words( r1 ) == 2 && words( r2 ) == 1 )
		{
			t.f_string = strip( r2 ) ;
			rcmd1 = r1 ;
		}
		else if ( words( r1 ) == 1 && words( r2 ) == 2 )
		{
			t.f_string = strip( r1 ) ;
			rcmd1 = r2 ;
		}
		else if ( words( r2 ) == 2 )
		{
			pcmd.set_msg( "PEDT016M", 20 ) ;
			return false ;
		}
		else
		{
			t.f_string = word( rcmd1, 1 ) ;
			idelword( rcmd1, 1, 1 ) ;
		}
	}

	t.f_ostring = t.f_string ;

	if ( t.f_reg ) { t.f_regreq = true ; }

	if ( !extract_range( rcmd1, r ) )
	{
		return false ;
	}

	t.f_slab = r.c_slab ;
	t.f_elab = r.c_elab ;
	t.f_scol = r.c_scol ;
	t.f_ecol = r.c_ecol ;
	t.f_ocol = r.c_ocol ;

	if ( !t.f_ocol && !t.f_reg && t.f_scol > 0 && t.f_string.size() > ( t.f_ecol - t.f_scol + 1 ) )
	{
		pcmd.set_msg( "PEDT016L", t.f_string, 20 ) ;
		return false ;
	}

	if ( t.f_hex )
	{
		if ( !ishex( t.f_string ) )
		{
			pcmd.set_msg( "PEDT011K", 20 ) ;
			return false ;
		}
		t.f_string = xs2cs( t.f_string ) ;
		t.f_asis   = true     ;
		iupper( t.f_ostring ) ;
	}

	if ( t.f_chex )
	{
		if ( !ishex( t.f_cstring ) )
		{
			pcmd.set_msg( "PEDT011K", 20 ) ;
			return false ;
		}
		t.f_cstring = xs2cs( t.f_cstring ) ;
		t.f_casis   = true ;
	}

	if ( t.f_cpic )
	{
		//
		// =  don't change character.
		// <  change to lower case alphabetic.
		// >  change to upper case alphabetic.
		//
		if ( t.f_reg )
		{
			pcmd.set_msg( "PEDT016A", 20 ) ;
			return false ;
		}
		for ( int i = 0 ; i < t.f_cstring.size() ; ++i )
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
					pcmd.set_msg( "PEDT015X", 20 ) ;
					return false ;
			}
		}
		if ( t.f_cstring.size() != t.f_string.size() )
		{
			pcmd.set_msg( "PEDT015Y", 20 ) ;
			return false ;
		}
	}

	//
	// For picture strings:
	//
	// =  any character                   .  invalid characters (dot, x'2E')
	// @  alphabetic characters           -  non-numeric characters
	// #  numeric characters              <  lower case alphabetics
	// $  special characters              >  upper case alphabetics
	// ¬  non-blank characters (x'AC')
	//

	if ( t.f_pic || t.f_prefix() || t.f_suffix() || t.f_word() )
	{
		t.f_regreq = true ;
		pic   = "" ;
		alnum = false ;
		for ( int i = 0 ; i < t.f_string.size() ; ++i )
		{
			c = t.f_string[ i ] ;
			if ( i == 0 || i == t.f_string.size() - 1 )
			{
				alnum = isalnum( c ) ;
			}
			if ( t.f_pic && pic_chars.find( c ) != string::npos )
			{
				switch ( c )
				{
					case '=':
						alnum = true ;
						pic += "." ;
						break ;

					case '@':
						alnum = true ;
						pic += "[[:alpha:]]" ;
						break ;

					case '#':
						alnum = true ;
						pic += "[[:digit:]]" ;
						break ;

					case '$':
						alnum = false ;
						pic += "[^[:blank:]^[:alpha:]^[:digit:]]" ;
						break ;

					case '¬':
						alnum = false ;
						pic += "[^[:blank:]]" ;
						break ;

					case '.':
						alnum = false ;
						pic += "[^[:print:]]" ;
						break ;

					case '-':
						alnum = true ;
						pic += "[^[:digit:]]" ;
						break ;

					case '<':
						alnum = true ;
						pic += "(?-i)[a-z](?i)" ;
						break ;

					case '>':
						alnum = true ;
						pic += "(?-i)[A-Z](?i)" ;
						break ;

				}
			}
			else if ( regex_chars.find( c ) != string::npos )
			{
				pic.push_back( '\\' ) ;
				pic.push_back( c ) ;
			}
			else
			{
				pic.push_back( c ) ;
			}
			if ( i == 0 )
			{
				alnum1 = alnum ;
			}
			if ( i == t.f_string.size() - 1 )
			{
				alnum2 = alnum ;
			}
		}
		t.f_string = pic ;
	}

	switch ( t.f_mtch )
	{
		case 'P':
			pre1 = ( alnum1 ) ? "\\b" : "(^|\\B)" ;
			t.f_string = pre1 + t.f_string + "\\w" ;
			break ;

		case 'S':
			suf1 = ( alnum2 ) ? "\\b" : "($|\\B)" ;
			t.f_string = "\\w" + t.f_string + suf1 ;
			break ;

		case 'W':
			pre1 = ( alnum1 ) ? "\\b" : "(^|\\B)" ;
			suf1 = ( alnum2 ) ? "\\b" : "($|\\B)" ;
			t.f_string = pre1 + t.f_string + suf1 ;
	}

	t.f_fset = true ;
	if ( ftyp == 'C')
	{
		t.f_cset = true ;
		if ( t.f_all() )
		{
			t.f_set_first() ;
			t.f_chngall = true ;
		}
	}
	else if ( ftyp == 'X')
	{
		t.f_exclude = true ;
	}
	else if ( keep && ftyp == 'F' && fcx_parms.f_cset )
	{
		t.f_cset    = true ;
		t.f_cstring = fcx_parms.f_cstring ;
		t.f_ctext   = fcx_parms.f_ctext ;
		t.f_casis   = fcx_parms.f_casis ;
		t.f_chex    = fcx_parms.f_chex ;
		t.f_cpic    = fcx_parms.f_cpic ;
		t.f_creg    = fcx_parms.f_creg ;
	}
	t.f_ssize = t.f_string.size() ;

	if ( t.f_regreq )
	{
		try
		{
			if ( t.f_asis )
			{
				t.f_regexp.assign( t.f_string ) ;
			}
			else
			{
				t.f_regexp.assign( t.f_string, boost::regex_constants::icase ) ;
			}
		}
		catch ( boost::regex_error& e )
		{
			pcmd.set_msg( "PEDT014X", 20 ) ;
			return false ;
		}
	}

	fcx_parms = t ;

	if ( t.f_all() )
	{
		t.f_set_first() ;
	}

	Global_efind_parms[ ds2d( zscrnum ) ] = t ;

	return true ;
}


void pedit01::hiliteFindPhrase()
{
	//
	// Hilite FIND phrase between normal limits (bounds, range) between first and last line on screen.
	// c1,c2 are the column limits of the find, set by bnds, col, line length and initial cursor position.
	//

	TRACE_FUNCTION() ;

	int i ;
	int rc ;
	int ln ;
	int s1 ;

	int c1 ;
	int c2 ;

	int scol ;
	int ecol ;

	iline* dl ;

	size_t p1 ;

	string ss ;

	bool found ;

	Data::Iterator it  = nullptr ;
	Data::Iterator its = nullptr ;
	Data::Iterator ite = nullptr ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	boost::smatch results ;

	if ( fcx_parms.f_slab != "" )
	{
		its = getLabelIterator( fcx_parms.f_slab, rc ) ;
		if ( rc != 0 )
		{
			return ;
		}
		ite = getLabelIterator( fcx_parms.f_elab, rc ) ;
		if ( rc != 0 )
		{
			return ;
		}
	}
	else
	{
		its = topLine ;
		ite = data.bottom() ;
	}

	scol = LeftBnd - 1 ;
	ecol = ( RightBnd > 1 ) ? RightBnd - 1 : 0 ;

	for ( i = 0 ; i < zlvline ; ++i )
	{
		ipos& tpos = s2data[ i ] ;
		dl = tpos.ipos_addr ;
		if ( !dl                  ||
		     tpos.ipos_hex_line() ||
		     data.lt( dl, its )   ||
		     dl->not_valid_file() ||
		     dl->is_excluded() )  { continue ; }

		if ( data.gt( dl, ite ) ) { break ; }

		c1 = 0 ;
		c2 = dl->get_idata_len() - 1 ;

		if ( scol > c1 )             { c1 = scol ; }
		if ( ecol > 0 && ecol < c2 ) { c2 = ecol ; }

		if ( c1 > c2 ) { continue ; }

		if ( fcx_parms.f_regreq )
		{
			itss = dl->get_idata_begin() + c1 ;
			itse = itss + ( c2 - c1 + 1 ) ;
			while ( regex_search( itss, itse, results, fcx_parms.f_regexp ) )
			{
				if ( fcx_parms.f_ocol && itss != results[ 0 ].first )
				{
					break ;
				}
				for ( p1 = c1 ; itss != results[ 0 ].first ; ++itss ) { ++p1 ; }
				c1    = p1 + 1 ;
				itss  = results[ 0 ].first ;
				if ( itss == itse ) { break ; }
				++itss ;
				ss = results[ 0 ] ;
				ln = ss.size()    ;
				s1 = p1 - startCol + 1 ;
				if ( fcx_parms.f_suffix() ) { ++s1 ; --ln ; }
				if ( s1 < 0 )
				{
					ln = ln + s1 ;
					if ( ln <= 0 ) { continue ; }
					s1 = 0 ;
				}
				zshadow.replace( (zareaw*i + CLINESZ + s1 ), ln, ln, F_PHRASE ) ;
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
					p1 = dl->get_idata_ptr()->find( fcx_parms.f_string, c1 )  ;
				}
				else
				{
					p1 = upper( dl->get_idata() ).find( fcx_parms.f_string, c1 ) ;
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
				zshadow.replace( (zareaw*i + CLINESZ + s1 ), ln, string( ln, F_PHRASE ) ) ;
				if ( fcx_parms.f_ocol ) { break ; }
			}
		}
	}
}


void pedit01::actionFind()
{
	//
	// c1,c2 are the column limits of the find, set by bnds, col, line length, number mode and initial cursor position.
	//
	// If running under an edit macro, use mRow/mCol instead of aRow/aCol.
	//

	TRACE_FUNCTION() ;

	int rc   ;

	int c1   ;
	int c2   ;

	int Col  ;
	int scol ;
	int ecol ;

	int offset ;

	iline*  dl ;
	iline* sdl ;
	iline* edl ;

	Data::Iterator it = nullptr ;

	bool found ;

	Col = ( macroRunning ) ? mCol : max( 0, aCol - CLINESZ ) ;

	if ( Col == 0 && fcx_parms.f_prev() && data.le( topLine, getFileLineZFIRST() ) )
	{
		fcx_parms.f_set_last() ;
	}

	if ( !fcx_parms.f_success && ( fcx_parms.f_next() || fcx_parms.f_prev() ) )
	{
		if ( fcx_parms.f_pADDR    == aADDR &&
		     fcx_parms.f_pCol     == ( Col + startCol ) &&
		     fcx_parms.f_ptopLine == topLine )
		{
			( fcx_parms.f_next() ) ? fcx_parms.f_set_first() : fcx_parms.f_set_last() ;
		}
	}

	fcx_parms.f_pADDR    = aADDR ;
	fcx_parms.f_pCol     = Col + startCol ;
	fcx_parms.f_ptopLine = topLine ;

	fcx_parms.reset() ;

	if ( !getFileLineZFIRST() )
	{
		fcx_parms.f_top = true ;
		return ;
	}

	sdl = data.top() ;
	edl = data.bottom() ;

	if ( fcx_parms.f_first() || fcx_parms.f_all() || fcx_parms.f_last() )
	{
		offset = 0 ;
	}
	else if ( fcx_parms.f_prev() && fcx_parms.f_prefix() )
	{
		offset = startCol + Col - 2 ;
	}
	else
	{
		offset = startCol + Col - 1 ;
	}

	if ( fcx_parms.f_first() || fcx_parms.f_all() )
	{
		dl = data.top() ;
	}
	else if ( fcx_parms.f_last() )
	{
		dl = data.bottom() ;
	}
	else if ( macroRunning )
	{
		dl = mRow ;
	}
	else if ( aRow == 0 )
	{
		dl = topLine ;
		if ( fcx_parms.f_prev() && dl != data.top() )
		{
			idecr( dl ) ;
		}
		if ( fcx_parms.f_next() && dl->is_bod() )
		{
			fcx_parms.f_searched = true ;
			return ;
		}
	}
	else
	{
		dl = s2data.at( aRow-1 ).ipos_addr ;
		if ( fcx_parms.f_next() && ( !dl || dl->is_bod() ) )
		{
			fcx_parms.f_searched = true ;
			return ;
		}
		if ( fcx_parms.f_prev() && ( !dl || dl->is_tod() ) )
		{
			dl = data.bottom() ;
		}
	}

	if ( fcx_parms.f_slab != "" )
	{
		sdl = getLabelAddress( fcx_parms.f_slab, rc ) ;
		if ( rc != 0 )
		{
			pcmd.set_msg( "PEDT011F", 20 ) ;
			fcx_parms.f_error = true ;
			return ;
		}
		edl = getLabelAddress( fcx_parms.f_elab, rc ) ;
		if ( rc != 0 )
		{
			pcmd.set_msg( "PEDT011F", 20 ) ;
			fcx_parms.f_error = true ;
			return ;
		}
		if ( data.gt( sdl, edl ) )
		{
			fcx_parms.f_slab.swap( fcx_parms.f_elab )  ;
			swap( sdl, edl ) ;
		}
	}

	sdl = getFileLineNext( sdl ) ;
	edl = getFileLinePrev( edl ) ;

	if ( !sdl ) { sdl = data.top()    ; }
	if ( !edl ) { edl = data.bottom() ; }

	if ( data.lt( dl, sdl ) )
	{
		fcx_parms.f_top = true ;
		if ( fcx_parms.f_reverse() )
		{
			fcx_parms.f_searched = true ;
			return ;
		}
		dl     = sdl ;
		offset = 0 ;
	}
	else if ( data.gt( dl, edl ) )
	{
		fcx_parms.f_bottom = true ;
		if ( !fcx_parms.f_reverse() )
		{
			fcx_parms.f_searched = true ;
			return ;
		}
		dl     = edl ;
		offset = 0 ;
	}

	if ( offset == 0 && fcx_parms.f_prev() )
	{
		dl = getPrevFileLine( dl ) ;
	}

	if ( fcx_parms.f_ocol )
	{
		scol = fcx_parms.f_scol - 1 ;
		ecol = scol ;
	}
	else
	{
		scol = max( fcx_parms.f_scol, LeftBnd ) - 1 ;
		if ( RightBnd > 0 )
		{
			ecol = ( fcx_parms.f_ecol > 0 ) ? min( fcx_parms.f_ecol, RightBnd ) - 1 : RightBnd - 1 ;
		}
		else
		{
			ecol = ( fcx_parms.f_ecol > 0 ) ? fcx_parms.f_ecol - 1 : 0 ;
		}
	}

	fcx_parms.f_acol = scol + 1 ;
	fcx_parms.f_bcol = ecol + 1 ;

	optFindPhrase = true  ;
	found         = false ;

	idecr( sdl ) ;
	iincr( edl ) ;

	while ( dl && dl != sdl && dl != edl )
	{
		if ( dl->get_idata_len() == 0 || ( fcx_parms.f_x() && dl->is_not_excluded() ) || ( fcx_parms.f_nx() && dl->is_excluded() ) )
		{
			dl = ( fcx_parms.f_reverse() ) ? getPrevFileLine( dl ) : getNextFileLine( dl ) ;
			offset = 0 ;
			continue ;
		}
		if ( fcx_parms.f_prev() )
		{
			c1 = 0 ;
			c2 = ( offset == 0 ) ? dl->get_idata_len() - 1 : offset - 2 ;
			offset = 0 ;
		}
		else
		{
			c1 = offset ;
			c2 = dl->get_idata_len() - 1 ;
		}

		if ( scol > c1 )             { c1 = scol ; }
		if ( ecol > 0 && ecol < c2 ) { c2 = ecol ; }

		fcx_parms.f_searched = true ;
		if ( ( c2 == -1 && fcx_parms.f_word() ) || ( c2 > -1 && ( c1 > c2 || offset > c2 ) ) || ( fcx_parms.f_ocol && fcx_parms.f_scol <= c1 ) )
		{
			dl = ( fcx_parms.f_reverse() ) ? getPrevFileLine( dl ) : getNextFileLine( dl ) ;
			offset = 0 ;
			continue ;
		}

		if ( fcx_parms.f_regreq )
		{
			found = actionFind_regex( dl, c1, c2 ) ;
		}
		else
		{
			found = actionFind_nonregex( dl, c1, c2, ecol ) ;
		}

		if ( fcx_parms.f_all() )
		{
			dl = getNextFileLine( dl ) ;
		}
		else if ( fcx_parms.f_reverse() )
		{
			fcx_parms.f_set_prev() ;
			if ( found ) { break ; }
			dl = getPrevFileLine( dl ) ;
		}
		else
		{
			fcx_parms.f_set_next() ;
			if ( found ) { break ; }
			dl = getNextFileLine( dl ) ;
		}
		offset = 0 ;
	}

	fcx_parms.f_success = found ;

	if ( fcx_parms.f_all() && fcx_parms.f_occurs == 0 )
	{
		fcx_parms.f_set_next() ;
	}
}


bool pedit01::actionFind_regex( iline* dl,
				int c1,
				int c2 )
{
	//
	// Perform find for line dl using a regex.
	//
	// :: For suffix, decrement start of range (c1) in case suffix starts on c1.
	// :: For prefix, increment end of range (c2) in case prefix ends on c2.
	//    otherwise regex_search() will not produce a match.
	//

	TRACE_FUNCTION() ;

	bool found = false ;
	bool first = true ;

	size_t p1 ;
	size_t p2 ;

	int rc ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	boost::smatch results ;

	if ( fcx_parms.f_prefix() )
	{
		if ( ( c2 + 1 ) < dl->get_idata_len() )
		{
			++c2 ;
		}
	}
	else if ( fcx_parms.f_suffix() && c1 > 0 )
	{
		--c1 ;
	}

	itss = dl->get_idata_begin() + c1 ;

	if ( fcx_parms.f_prev() || fcx_parms.f_ocol )
	{
		itse = dl->get_idata_end() ;
	}
	else
	{
		itse = itss + ( c2 - c1 + 1 ) ;
	}

	if ( fcx_parms.f_prev() || fcx_parms.f_last() || fcx_parms.f_all() )
	{
		while ( regex_search( itss, itse, results, fcx_parms.f_regexp ) )
		{
			if ( fcx_parms.f_word_prefix_suffix() )
			{
				rc = actionFind_regex_wps( dl, c1, itss, itse, results[ 0 ].first, results[ 0 ].second ) ;
				if ( rc == 1 )
				{
					++itss ;
					continue ;
				}
				else if ( rc == 2 )
				{
					break ;
				}
			}
			if ( fcx_parms.f_ocol && itss != results[ 0 ].first )
			{
				break ;
			}
			for ( p2 = c1 ; itss != results[ 0 ].first ; ++itss ) { ++p2 ; }
			if ( fcx_parms.f_prev() && p2 > c2 )
			{
				break ;
			}
			found = true ;
			p1    = p2   ;
			c1    = p2+1 ;
			if ( !pcmd.is_seek() )
			{
				dl->set_excluded( fcx_parms.f_exclude, level ) ;
			}
			fcx_parms.set_rstring( results[ 0 ] ) ;
			if ( first )
			{
				fcx_parms.f_incr_lines() ;
				first = false ;
			}
			fcx_parms.f_incr_occurs() ;
			if ( fcx_parms.f_all() && !fcx_parms.f_ADDR )
			{
				if ( fcx_parms.f_suffix() ) { ++p1 ; }
				fcx_parms.f_ADDR   = dl ;
				fcx_parms.f_offset = p1 ;
			}
			if ( fcx_parms.f_ocol ) { break ; }
			itss = results[ 0 ].first ;
			if ( itss == itse ) { break ; }
			++itss ;
		}
		if ( fcx_parms.f_all() ) { found = false ; }
	}
	else
	{
		while ( boost::regex_search( itss, itse, results, fcx_parms.f_regexp ) )
		{
			if ( fcx_parms.f_word_prefix_suffix() )
			{
				rc = actionFind_regex_wps( dl, c1, itss, itse, results[ 0 ].first, results[ 0 ].second ) ;
				if ( rc == 1 )
				{
					++itss ;
					continue ;
				}
				else if ( rc == 2 )
				{
					break ;
				}
			}
			if ( !fcx_parms.f_ocol || itss == results[ 0 ].first )
			{
				found = true ;
				if ( !pcmd.is_seek() )
				{
					dl->set_excluded( fcx_parms.f_exclude, level ) ;
				}
				fcx_parms.set_rstring( results[ 0 ] ) ;
				for ( p1 = c1 ; itss != results[ 0 ].first ; ++itss ) { ++p1 ; }
			}
			break ;
		}
	}

	if ( found && fcx_parms.f_suffix() ) { ++p1 ; }

	if ( found && !fcx_parms.f_ADDR )
	{
		fcx_parms.f_ADDR   = dl ;
		fcx_parms.f_offset = p1 ;
	}

	return found ;
}


int pedit01::actionFind_regex_wps( iline* dl,
				   int c1,
				   string::const_iterator itss,
				   string::const_iterator itse,
				   string::const_iterator itrf,
				   string::const_iterator itrs )
{
	//
	// A match for word/prefix/suffix may give a false positive if the
	// match is on a boundary so we need to check further.
	// :: word/prefix at start of range must have non-word character before start of range.
	// :: word/suffix at end of range must have non-word character after end of range.
	//
	//   RETURN:
	//    0 - No action
	//    1 - Increment itss and continue
	//    2 - Break
	//

	TRACE_FUNCTION() ;

	const string wchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_" ;

	if ( fcx_parms.f_word_prefix() && c1 > 0 && itss == itrf )
	{
		if ( wchars.find( *(itss - 1) ) != string::npos )
		{
			return 1 ;
		}
	}

	if ( fcx_parms.f_word_suffix() && itse == itrs )
	{
		if ( itse != dl->get_idata_end() && wchars.find( *itse ) != string::npos )
		{
			return 2 ;
		}
	}

	return 0 ;
}


bool pedit01::actionFind_nonregex( iline* dl,
				   int c1,
				   int c2,
				   int ecol )
{
	//
	// Perform find for line dl.
	//

	TRACE_FUNCTION() ;

	bool found ;
	bool first = true ;

	string  temp ;
	string* strptr ;

	size_t p1 ;

	if ( fcx_parms.f_asis )
	{
		strptr = dl->get_idata_ptr() ;
	}
	else
	{
		temp   = upper( dl->get_idata() ) ;
		strptr = &temp ;
	}

	if ( fcx_parms.f_ocol )
	{
		found = ( strptr->compare( fcx_parms.f_scol-1, fcx_parms.f_string.size(), fcx_parms.f_string ) == 0 ) ;
		if ( found )
		{
			if ( !fcx_parms.f_ADDR )
			{
				fcx_parms.f_ADDR   = dl ;
				fcx_parms.f_offset = fcx_parms.f_scol - 1 ;
			}
			if ( !pcmd.is_seek() )
			{
				dl->set_excluded( fcx_parms.f_exclude, level ) ;
			}
			if ( !fcx_parms.f_asis )
			{
				fcx_parms.set_rstring( temp.substr( fcx_parms.f_scol - 1, fcx_parms.f_ssize ) ) ;
			}
			fcx_parms.f_incr_occurs() ;
			fcx_parms.f_incr_lines() ;
		}
		return found ;
	}

	while ( true )
	{
		found  = false ;
		p1     = ( fcx_parms.f_reverse() ) ? strptr->rfind( fcx_parms.f_string, c2 ) : strptr->find( fcx_parms.f_string, c1 ) ;
		if ( p1 != string::npos )
		{
			if ( fcx_parms.f_reverse() )
			{
				if ( p1 >= c1 )
				{
					if ( ecol == 0 || ( p1 + fcx_parms.f_ssize - 1 ) <= ecol )
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
			if ( !fcx_parms.f_ADDR )
			{
				fcx_parms.f_ADDR   = dl ;
				fcx_parms.f_offset = p1 ;
			}
			if ( !pcmd.is_seek() )
			{
				dl->set_excluded( fcx_parms.f_exclude, level ) ;
			}
			if ( !fcx_parms.f_asis )
			{
				fcx_parms.set_rstring( dl->get_idata_ptr()->substr( p1, fcx_parms.f_ssize ) ) ;
			}
		}
		if ( !found || !fcx_parms.f_all() )
		{
			break ;
		}
		fcx_parms.f_incr_occurs() ;
		if ( first )
		{
			fcx_parms.f_incr_lines() ;
			first = false ;
		}
	}

	return found ;
}


void pedit01::actionChange()
{
	//
	// If a regex has been used as the find argument, use fcx_rstring instead of fcx_string.
	//
	// If the inserted string is longer, remove some spaces at the end of the insert to compensate.
	// If the inserted string is shorter and more there is more 1 blank after the found string, maintain column position.
	//
	// If change string is text and CAPS OFF, try to keep the capitalisation for PREFIX and SUFFIX.
	//
	// If error occurs, set f_rc = 8 for the MACRO return code.
	//

	TRACE_FUNCTION() ;

	int d ;
	int osz ;
	int offset = fcx_parms.f_offset - lnumSize1 ;

	size_t i ;
	size_t j ;

	bool alllower = false ;
	bool allupper = false ;

	string temp ;
	string temp1 ;

	iline* dl ;

	dl   = fcx_parms.f_ADDR ;
	temp = dl->get_idata( lnumSize1, lnumS2pos ) ;
	osz  = temp.size() ;

	if ( fcx_parms.f_cpic )
	{
		for ( i = 0 ; i < fcx_parms.f_cstring.size() ; ++i )
		{
			if ( fcx_parms.f_cstring[ i ] == '=' )
			{
				continue ;
			}
			else if ( fcx_parms.f_cstring[ i ] == '<' )
			{
				ilower( temp[ offset + i ] ) ;
			}
			else if ( fcx_parms.f_cstring[ i ] == '>' )
			{
				iupper( temp[ offset + i ] ) ;
			}
			else
			{
				temp[ offset + i ] = fcx_parms.f_cstring[ i ] ;
			}
		}
		if ( dl->put_idata( temp, level, ID_CHNGO ) )
		{
			fileChanged = true ;
		}
		dl->setChngStatus( level ) ;
		return ;
	}

	if ( !profCaps && fcx_parms.f_ctext && ( fcx_parms.f_prefix() || fcx_parms.f_suffix() ) )
	{
		alllower = lalpha( temp, offset ) ;
		if ( !alllower )
		{
			allupper = ualpha( temp, offset ) ;
		}
	}

	if ( fcx_parms.f_prefix() )
	{
		temp.replace( offset, fcx_parms.f_rstring.size()-1, ( alllower ) ? lower( fcx_parms.f_cstring ) :
								    ( allupper ) ? upper( fcx_parms.f_cstring ) :
											  fcx_parms.f_cstring ) ;
	}
	else if ( fcx_parms.f_suffix() )
	{
		temp.replace( offset, fcx_parms.f_rstring.size()-1, ( alllower ) ? lower( fcx_parms.f_cstring ) :
								    ( allupper ) ? upper( fcx_parms.f_cstring ) :
											  fcx_parms.f_cstring ) ;
	}
	else if ( fcx_parms.f_regreq )
	{
		auto its = temp.begin() + offset ;
		auto ite = temp.end() ;
		regex_replace( back_inserter( temp1 ), its, ite, fcx_parms.f_regexp, fcx_parms.f_cstring, boost::format_first_only ) ;
		temp.replace( its, ite, temp1 ) ;
	}
	else
	{
		temp.replace( offset, fcx_parms.f_ssize, fcx_parms.f_cstring ) ;
	}

	if ( reclen > 0 )
	{
		i = temp.find_last_not_of( ' ' ) ;
		if ( i != string::npos && ( i + 1 ) > ( reclen - lnumSize ) )
		{
			dl->setErrorStatus( level ) ;
			fcx_parms.f_rc    = 8 ;
			fcx_parms.f_error = true ;
			return ;
		}
	}

	d = temp.size() - osz ;
	i = temp.find( ' ', offset + fcx_parms.f_cstring.size() ) ;
	if ( d > 0 )
	{
		if( i != string::npos )
		{
			j = temp.find_first_not_of( ' ', i ) ;
			if ( j != string::npos && ( j - i ) > 1 )
			{
				temp.erase( i, min( d, int( j - i - 1 ) ) ) ;
			}
		}
	}
	else if ( d < 0 && i != string::npos )
	{
		j = temp.find_first_not_of( ' ', i ) ;
		if ( j != string::npos && ( j - i ) > 1 )
		{
			temp.insert( i, string( abs( d ), ' ' ) ) ;
		}
	}

	if ( reclen > 0 )
	{
		temp.resize( ( reclen - lnumSize ), ' ' ) ;
		d = 0 ;
	}

	if ( d > 0 ) { fcx_parms.f_chnincr = d ; }

	if ( dl->put_idata( lnumSize1, lnumS2pos, temp, level, ID_CHNGO ) )
	{
		fileChanged = true ;
		dl->update_lnummod( lnumSize1, lnumS2pos, lnummod ) ;
	}

	dl->setChngStatus( level ) ;
}


bool pedit01::extract_range( string s,
			     cmd_range& t )
{
	//
	// Extract the command range ( start/end labels and start/end columns or on column ) from
	// a command and return data in t.
	//
	// c_vlab true if labels can be entered on the command.
	// c_vcol true if columns can be entered on the command.
	//

	TRACE_FUNCTION() ;

	int j ;

	int rc ;

	string w ;

	t.cmd_range_clear() ;

	w = word( s, 1 ) ;
	while ( w != "" )
	{
		if ( isnumeric( w ) )
		{
			if ( !t.c_vcol )
			{
				pcmd.set_msg( "PEDT013T", 20 ) ;
				return false ;
			}
			if ( t.c_scol != 0 && t.c_ecol != 0 )
			{
				pcmd.set_msg( "PEDT019", 20 ) ;
				return false ;
			}
			j = ds2d( w ) ;
			if ( j < 1 || j > MAXLEN )
			{
				pcmd.set_msg( "PEDT011J", 20 ) ;
				return false ;
			}
			( t.c_scol == 0 ) ? t.c_scol = j : t.c_ecol = j ;
		}
		else if ( w.front() == '.' )
		{
			if ( !t.c_vlab )
			{
				pcmd.set_msg( "PEDT013S", 20 ) ;
				return false ;
			}
			if ( t.c_slab != "" && t.c_elab != "" )
			{
				pcmd.set_msg( "PEDT011A", 20 ) ;
				return false ;
			}
			( t.c_slab == "" ) ? t.c_slab = w : t.c_elab = w ;
		}
		else
		{
			pcmd.set_msg( "PEDT011", 20 ) ;
			return false ;
		}
		idelword( s, 1, 1 ) ;
		w = word( s, 1 ) ;
	}

	if ( t.c_scol != 0 && t.c_ecol == 0 )
	{
		t.c_ocol = true ;
	}
	else if ( t.c_scol > t.c_ecol )
	{
		swap( t.c_scol, t.c_ecol ) ;
	}

	if ( t.c_slab == ".ZLAST" && t.c_elab == "" )
	{
		pcmd.set_msg( "PEDT013R", 20 ) ;
		return false ;
	}

	if ( t.c_slab != "" )
	{
		if ( t.c_elab == "" )
		{
			pcmd.set_msg( "PEDT016", 20 ) ;
			return false ;
		}
		if ( t.c_slab == t.c_elab )
		{
			pcmd.set_msg( "PEDT011G", 20 ) ;
			return false ;
		}
		t.c_sidx = getLabelAddress( t.c_slab, rc ) ;
		if ( rc < 0 )
		{
			pcmd.set_msg( "PEDT011F", 20 ) ;
			return false ;
		}
		t.c_eidx = getLabelAddress( t.c_elab, rc ) ;
		if ( rc < 0 )
		{
			pcmd.set_msg( "PEDT011F", 20 ) ;
			return false ;
		}
		if ( data.gt( t.c_sidx, t.c_eidx ) )
		{
			t.c_slab.swap( t.c_elab )  ;
			swap( t.c_sidx, t.c_eidx ) ;
		}
	}

	return true ;
}


bool pedit01::extract_labels( string& s,
			      iline*& sidx,
			      iline*& eidx )
{
	//
	// Extract the command labels and return them in l1, l2.
	// String s must have at least two words for the entered labels.  Labels are removed.
	//
	// For MACRO commands CREATE and REPLACE.
	//

	TRACE_FUNCTION() ;

	int rc ;
	int ws = words( s ) ;

	sidx = nullptr ;
	eidx = nullptr ;

	if ( ws < 2 )
	{
		return false ;
	}

	string l1 = upper( word( s, ws ) ) ;
	string l2 = upper( word( s, ws-1 ) ) ;

	if ( l1.front() != '.' || l2.front() != '.' )
	{
		return false ;
	}

	idelword( s, ws-1 ) ;

	if ( l1 == l2 )
	{
		pcmd.set_msg( "PEDT011G", 20 ) ;
		return false ;
	}

	sidx = getLabelAddress( l1, rc ) ;
	if ( rc < 0 )
	{
		pcmd.set_msg( "PEDT011F", 20 ) ;
		return false ;
	}

	eidx = getLabelAddress( l2, rc ) ;
	if ( rc < 0 )
	{
		pcmd.set_msg( "PEDT011F", 20 ) ;
		return false ;
	}

	if ( data.gt( sidx, eidx ) )
	{
		swap( sidx, eidx ) ;
	}

	return true ;
}


int pedit01::extract_lptr( string& s,
			   Data::Iterator& it1,
			   Data::Iterator& it2,
			   bool i_lnum,
			   bool set_defs )
{
	//
	// Extract the lineptr's for a macro command and return the iterators.
	// If none entered, set defaults if requested:
	//  it1 = .ZFIRST
	//  it2 = .ZLAST
	//
	// lineptr can be a label or linenum.
	// Remove lptr's from string.
	//
	// if i_lnum set, also extact linenum entries (default).
	// if set_defs set, set defaults to first line/last line (default).
	//
	//   RETURN:
	//    0  Normal completion
	//    1  No file lines
	//    2  Error
	//

	TRACE_FUNCTION() ;

	it1 = nullptr ;
	it2 = nullptr ;

	int rc ;
	int ln ;

	int ws = words( s ) ;

	string w ;

	for ( int i = ws ; i > 0 ; --i )
	{
		w = word( s, i ) ;
		if ( w.front() == '.' )
		{
			if ( it1 != nullptr && it2 != nullptr )
			{
				return 2 ;
			}
			auto it = getLabelIterator( upper( w ), rc ) ;
			if ( rc != 0 )
			{
				return 2 ;
			}
			( it1 == nullptr ) ? it1 = it : it2 = it ;
			idelword( s, i, 1 ) ;
		}
		else if ( i_lnum && datatype( w, 'W' ) )
		{
			if ( it1 != nullptr && it2 != nullptr )
			{
				return 2 ;
			}
			ln = ds2d( w ) ;
			auto it = getDataLine( ln ) ;
			if ( it == data.bottom() )
			{
				return 2 ;
			}
			( it1 == nullptr ) ? it1 = it : it2 = it ;
			idelword( s, i, 1 ) ;
		}
	}

	if ( it1 != nullptr && it2 == nullptr )
	{
		return 2 ;
	}
	else if ( it1 == nullptr && it2 == nullptr )
	{
		if ( set_defs )
		{
			it1 = getFileLineZFIRST() ;
			if ( it1 == nullptr ) { return 1 ; }
			it2 = getFileLineZLAST() ;
			if ( it2 == nullptr ) { return 2 ; }
		}
	}
	else if ( data.gt( it1, it2 ) )
	{
		swap( it1, it2 ) ;
	}

	return 0 ;
}


void pedit01::moveColumn( int diff )
{
	//
	// Reposition startCol if any part of the find string is outside ZAREA.
	// diff is used for change commands where the string has increased in size.
	//

	TRACE_FUNCTION() ;

	int o ;
	int p ;

	if ( fcx_parms.f_regreq )
	{
		p = fcx_parms.f_offset + fcx_parms.f_rstring.size() ;
	}
	else
	{
		p = fcx_parms.f_offset + fcx_parms.f_ssize ;
	}

	o = p + diff - startCol + 1 ;

	if ( o < 0 )
	{
		startCol     = ( p < zdataw ) ? 1 : fcx_parms.f_offset - 13 ;
		rebuildZAREA = true ;
	}
	else if ( o > zdataw )
	{
		startCol     = fcx_parms.f_offset - 13 ;
		rebuildZAREA = true ;
	}
	else if ( fcx_parms.f_reverse() && ( fcx_parms.f_offset - startCol + 1 ) < 0 )
	{
		startCol     = ( fcx_parms.f_offset <= zdataw ) ? 1 : startCol - ( zdataw / 2 ) ;
		rebuildZAREA = true ;
	}

	startCol = max( 1, startCol ) ;
}


void pedit01::positionCursor()
{
	//
	// Move cursor to its set position.
	//
	// Highlight cursor phrase if option set.
	// Highlight FIND phrase if option set.
	//

	TRACE_FUNCTION() ;

	int i ;
	int o ;
	int screenLine ;

	iline* dl ;
	iline* ldl = nullptr ;

	size_t p  ;
	size_t p1 ;
	size_t p2 ;

	const string delim = "\x01\x02\x20" ;

	Cursorp* cursorx ;

	while ( true )
	{
		cursorx = cursor.get() ;
		if ( !cursorx )
		{
			cursorx = cursor.def() ;
			if ( !cursorx ) { break ; }
		}
		if ( cursorx->placeUsing == 1 )
		{
			if ( cursorx->getAddr() )
			{
				if ( data.ge( cursorx->getAddr(), topLine ) )
				{
					for ( i = ( zlvline - 1 ) ; i >= 0 && !ldl ; --i )
					{
						ldl = s2data.at( i ).ipos_addr ;
					}
					if ( ldl && data.le( cursorx->getAddr(), ldl ) )
					{
						break ;
					}
				}
			}
		}
		else if ( cursorx->placeRow <= zlvline )
		{
			break ;
		}
		if ( cursor.isdef( cursorx ) )
		{
			cursorx = nullptr ;
			break ;
		}
		cursor.pop_front() ;
	}

	if ( !cursorx || cursorx->placeHome )
	{
		curfld = "ZCMD" ;
		curpos = 1 ;
		return ;
	}

	screenLine = -1 ;

	switch ( cursorx->placeUsing )
	{
		case 1:
			dl = cursorx->placeADDR ;
			if ( dl->il_deleted )
			{
				cursorx->placeADDR = getNextDataLine( dl ) ;
			}
			if ( dl->is_excluded() )
			{
				cursorx->placeADDR = itr2ptr( getFirstEX( dl ) ) ;
			}
			for ( i = 0 ; i < zlvline ; ++i )
			{
				if ( s2data.at( i ).ipos_addr == cursorx->placeADDR )
				{
					screenLine = i ;
					break ;
				}
			}
			break ;

		case 2:
			screenLine = cursorx->placeRow - 1 ;
	}

	if ( screenLine == -1 )
	{
		curfld = "ZCMD" ;
		curpos = 1 ;
		return ;
	}

	switch ( cursorx->placeType )
	{
		case CSR_FC_LCMD:
			curfld = "ZAREA" ;
			curpos = zareaw * screenLine + 2 ;
			break ;

		case CSR_FC_DATA:
			curfld = "ZAREA" ;
			p1     = 1 ;
			if ( lnumSize1 > 0 && startCol <= lnumSize1 )
			{
				p1 = lnumSize1 - startCol + 2 ;
			}
			curpos = zareaw * screenLine + CLINESZ + p1 ;
			break ;

		case CSR_FNB_DATA:
			curfld = "ZAREA" ;
			dl = s2data.at( screenLine ).ipos_addr ;
			p  = dl->get_idata_ptr()->find_first_not_of( ' ', max( lnumSize1, size_t( startCol-1 ) ) ) ;
			if ( lnumS2pos > 0 && p >= lnumS2pos )
			{
				if ( lnumSize1 > 0 && startCol <= lnumSize1 )
				{
					p = CLINESZ + 2 + lnumSize1 - startCol ;
				}
				else
				{
					p = CLINESZ + 1 ;
				}
			}
			else
			{
				p = ( p != string::npos ) ? ( p + CLINESZ + 2 - startCol ) : CLINESZ + 1 ;
			}
			curpos = zareaw * screenLine + p ;
			break ;

		case CSR_OFF_DATA:
			o = cursorx->placeOff - startCol + CLINESZ + 2 ;
			if ( o < ( CLINESZ + 2 ) || o > zareaw )
			{
				curfld = "ZAREA" ;
				curpos = zareaw * screenLine + CLINESZ + 1 ;
			}
			else
			{
				curfld = "ZAREA" ;
				curpos = zareaw * screenLine + o ;
			}
			break ;

		case CSR_OFF_LCMD:
			curfld = "ZAREA" ;
			curpos = zareaw * screenLine + CLINESZ + 1 + cursorx->placeOff ;
	}

	if ( zshadow[ zareaw*screenLine ] == slgu.front() )
	{
		zshadow.replace( zareaw*screenLine, slru.size()-1, slru.size()-1, U_RED ) ;
	}
	else if ( zshadow[ zareaw*screenLine ] == slg.front() )
	{
		zshadow.replace( zareaw*screenLine, slr.size()-1, slr.size()-1, N_RED ) ;
	}

	rebuildShadow = true ;

	if ( profCsrPhrase && ( (curpos - 1) % zareaw ) >= CLINESZ )
	{
		dl = s2data.at( screenLine ).ipos_addr ;
		if ( dl->is_valid_file() )
		{
			p1 = zarea.find_last_of( delim, curpos-1 )  ;
			p2 = zarea.find_first_of( delim, curpos-1 ) ;
			if ( p2 == string::npos ) { p2 = zarea.size() - 1 ; }
			if ( p1 == string::npos ) { p1 = 0 ; }
			if ( p2 > p1 )
			{
				++p1 ;
				zshadow.replace( p1, p2-p1, p2-p1, C_PHRASE ) ;
			}
		}
	}
}


bool pedit01::formLineCmd1( const string& cmd,
			    string& lcc,
			    int& rept )
{
	//
	// Split line command into the command (string) and repeat value (int), if allowed, and return in lcc and rept.
	//
	// A 'rept' of -1 means it has not been entered.
	// Assume rept of 0 means it has not been entered.
	//

	TRACE_FUNCTION() ;

	string t ;

	t.assign( cmd.size(), ' ' ) ;
	lcc.assign( cmd.size(), ' ' ) ;

	for ( int j = 0 ; j < cmd.size() ; ++j )
	{
		if ( isdigit( cmd[ j ] ) ) { t[ j ]   = cmd[ j ] ; }
		else                       { lcc[ j ] = cmd[ j ] ; }
	}

	trim( t ) ;
	trim( lcc ) ;

	if ( aliasLCMDS.count( lcc ) > 0 )
	{
		lcc = aliasLCMDS[ lcc ] ;
	}

	if ( t != "" )
	{
		if ( isnumeric( t ) ) { rept = ds2d( t ) ; }
		else                  { pcmd.set_msg( "PEDT012G", 12 ) ; return false ; }
	}
	else
	{
		rept = -1 ;
	}

	if ( rept == 0 ) { rept = -1 ; }

	if ( lineCmds.count( lcc ) == 0 )
	{
		pcmd.set_msg( "PEDT012", 12 ) ;
		return false ;
	}

	if ( rept > 0 && reptOk.count( lcc ) == 0 )
	{
		pcmd.set_msg( "PEDT012A", 12 ) ;
		return false ;
	}

	return true ;
}


bool pedit01::formLineCmd2( const string& cmd,
			    string& lcc,
			    int& rept )
{
	//
	// Split line command into the command (string) and repeat value (int), and return in lcc and rept.
	//
	// This is a stripped down version for PROCESS DEST RANGE processing.
	//

	TRACE_FUNCTION() ;

	string t ;

	rept = 1 ;

	t.assign( cmd.size(), ' ' ) ;
	lcc.assign( cmd.size(), ' ' ) ;

	for ( int j = 0 ; j < cmd.size() ; ++j )
	{
		if ( isdigit( cmd[ j ] ) ) { t[ j ]   = cmd[ j ] ; }
		else                       { lcc[ j ] = cmd[ j ] ; }
	}

	trim( t ) ;
	trim( lcc ) ;

	if ( t != "" )
	{
		if ( isnumeric( t ) ) { rept = ds2d( t ) ; }
		else                  { pcmd.set_msg( "PEDT012G", 12 ) ; return false ; }
	}

	return true ;
}


int pedit01::getFileLine( Data::Iterator it1 )
{
	//
	// Return the file line that corresponts to data line iterator it1 in the data chain.
	// Iterator can be on or after the file line.
	//
	// First file line is 1.
	// Return 0 if none.
	//

	TRACE_FUNCTION() ;

	int f = 0 ;

	for ( auto it2 = data.begin() ; it2 != data.end() ; ++it2 )
	{
		if ( it2->is_valid_file() ) { ++f ; }
		if ( it2 == it1 ) { break ; }
	}

	return f ;
}


Data::Iterator pedit01::getDataLine( int fl )
{
	//
	// Return the iterator that corresponts to line fl in the file.
	// If not found, return bottom-of-data line.
	//
	// First file line is 1.
	//

	TRACE_FUNCTION() ;

	int j ;

	Data::Iterator it = nullptr ;

	if ( fl == 0 ) { return data.top() ; }

	for ( j = 0, it = data.begin() ; it != data.bottom() ; ++it )
	{
		if ( it->is_valid_file() )
		{
			if ( ++j == fl ) { break ; }
		}
	}

	return it ;
}


Data::Iterator pedit01::getNextFileLine( Data::Iterator it )
{
	//
	// Return the next non-deleted data vector line iterator that corresponts to the line after iterator it.
	// *File lines only*.  Return data.end() if end of data or bottom of data reached.
	//

	TRACE_FUNCTION() ;

	for ( ++it ; it != data.end() ; ++it )
	{
		if ( it->is_valid_file() ) { return it ; }
	}

	return data.end() ;
}


iline* pedit01::getNextFileLine( iline* l )
{
	//
	// Return the next non-deleted data vector line iterator that corresponts to the line after address l.
	// *File lines only*.  Return NULL if end of data or bottom of data reached.
	//

	TRACE_FUNCTION() ;

	if ( l == data.bottom() ) { return nullptr ; }

	for ( iincr( l ) ; l != data.bottom() ; iincr( l ) )
	{
		if ( l->is_valid_file() ) { return l ; }
	}

	return nullptr ;
}


iline* pedit01::getFileLinePrev( iline* l )
{
	//
	// Return a valid (ie. non-deleted) file data line on or before line l.
	// *File lines only*.  Return NULL if top of data reached.
	//

	TRACE_FUNCTION() ;

	for ( ; l != data.begin() ; idecr( l ) )
	{
		if ( l->is_valid_file() ) { return l ; }
	}

	return nullptr ;
}


Data::Iterator pedit01::getFileLineNext( Data::Iterator it )
{
	//
	// Return a valid (ie. non-deleted) file data line on or after iterator it.
	// *File lines only*.  Return data.end() if end of data reached.
	//

	TRACE_FUNCTION() ;

	for ( ; it != data.end() ; ++it )
	{
		if ( it->is_valid_file() ) { return it ; }
	}

	return data.end() ;
}


iline* pedit01::getFileLineNext( iline* l )
{
	//
	// Return a valid (ie. non-deleted) file data line on or after line l.
	// *File lines only*.  Return NULL if end of data reached.
	//

	TRACE_FUNCTION() ;

	for ( ; l != data.end() ; iincr( l ) )
	{
		if ( l->is_valid_file() ) { return l ; }
	}

	return nullptr ;
}


iline* pedit01::getPrevFileLine( iline* l )
{
	//
	// Return the previous non-deleted data vector iterator that corresponts to the file line before l.
	// *File lines only*.  If not found, return NULL.
	//

	TRACE_FUNCTION() ;

	if ( l == data.top() ) { return nullptr ; }

	for ( idecr( l ) ; l != data.top() ; idecr( l ) )
	{
		if ( l->is_valid_file() ) { return l ; }
	}

	return nullptr ;
}


iline* pedit01::getFileLineZFIRST()
{
	//
	// Return the address of the first file line.
	//
	// First file line is 1.
	// Return NULL if none.
	//

	TRACE_FUNCTION() ;

	iline* l ;

	for ( l = data.top() ; l != data.bottom() ; iincr( l ) )
	{
		if ( l->is_valid_file() ) { return l ; }
	}

	return nullptr ;
}


iline* pedit01::getFileLineZLAST()
{
	//
	// Return the address of the last file line in a file.
	//
	// First file line is 1.
	// Return NULL if none.
	//

	TRACE_FUNCTION() ;

	iline* l ;

	for ( l = data.bottom() ; l != data.top() ; idecr( l ) )
	{
		if ( l->is_valid_file() ) { return l ; }
	}

	return nullptr ;
}


Data::Iterator pedit01::getPrevDataLine( Data::Iterator it )
{
	//
	// Return the previous non-deleted data vector line that corresponts to the line before it.
	//

	TRACE_FUNCTION() ;

	if ( it == data.begin() ) { return data.begin() ; }

	for ( --it ; it != data.begin() ; --it )
	{
		if ( !it->il_deleted ) { break ; }
	}

	return it ;
}


iline* pedit01::getPrevDataLine( iline* l )
{
	//
	// Return the previous non-deleted data vector line that corresponts to the line before address l.
	//

	TRACE_FUNCTION() ;

	if ( l == data.top() ) { return data.top() ; }

	for ( idecr( l ) ; l != data.top() ; idecr( l ) )
	{
		if ( !l->il_deleted ) { break ; }
	}

	return l ;
}


Data::Iterator pedit01::getNextDataLine( Data::Iterator it )
{
	//
	// Return the next valid (ie. non-deleted) data line iterator after iterator it.
	//

	TRACE_FUNCTION() ;

	for ( ++it ; it != data.end() ; ++it )
	{
		if ( !it->il_deleted ) { break ; }
	}

	return it ;
}


iline* pedit01::getNextDataLine( iline* l )
{
	//
	// Return the next valid (ie. non-deleted) data line address after address l.
	// Return NULL if already at bottom.
	//

	TRACE_FUNCTION() ;


	if ( l == data.bottom() ) { return nullptr ; }

	for ( iincr( l ) ; l != data.bottom() ; iincr( l ) )
	{
		if ( !l->il_deleted ) { break ; }
	}

	return l ;
}


iline* pedit01::getDataLineNext( Data::Iterator it )
{
	//
	// Return a valid (ie. non-deleted) data line address on or after iterator it.
	//

	TRACE_FUNCTION() ;

	for ( ; it != data.end() ; ++it )
	{
		if ( !it->il_deleted ) { break ; }
	}

	return itr2ptr( it ) ;
}


int pedit01::isValidiline( iline* l )
{
	//
	// Check if l is a valid address.
	//
	// Return Code: 0 - Is a valid line.
	//              1 - Is a logically deleted line.
	//              2 - Address is not valid.
	//

	TRACE_FUNCTION() ;

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( l == it ) { return ( it->il_deleted ) ? 1 : 0 ; }
	}

	return 2 ;
}


int pedit01::getLinePtrIterator( const string& lptr,
				 Data::Iterator& it )
{
	//
	// Get iterator of the passed line pointer.
	//
	// An lptr can be a label or linenum.
	// Linenum is the file line, starting at 1.
	//
	//   RETURN:
	//    1  success.
	//    0  Error set - linenum is 0 or no file lines in data.
	//   -1  Error set - label is not assigned to a line.
	//   -2  Error set - linenum outside of data.
	//   -3  Error set - line pointer is an invalid format.
	//

	TRACE_FUNCTION() ;

	int i ;

	int rc = 1 ;

	it = nullptr ;

	if ( none_of( data.begin(), data.end(),
		[](iline& a )
		{
			return a.is_valid_file() ;
		} ) )
	{
		miBlock.seterror( "PEDM011W", 12 ) ;
		return 0 ;
	}

	if ( lptr == ".ZCSR" )
	{
		it = mRow ;
	}
	else if ( isnumeric( lptr ) )
	{
		i = ds2d( lptr ) ;
		if ( i == 0 )
		{
			miBlock.seterror( "PEDM011W", 12 ) ;
			return 0 ;
		}
		it = getDataLine( i ) ;
		if ( it == data.bottom() )
		{
			miBlock.seterror( "PEDM011S", 12 ) ;
			return -2 ;
		}
	}
	else
	{
		if ( !checkLabel2( lptr, 1 ) )
		{
			miBlock.seterror( "PEDM012F", 20 ) ;
			return -3 ;
		}
		it = getLabelIterator( lptr, rc ) ;
		if ( rc == -1 )
		{
			miBlock.seterror( "PEDM011Q", lptr, 12 ) ;
			return -1 ;
		}
		else if ( rc == -2 )
		{
			miBlock.seterror( "PEDM011W", 12 ) ;
			return 0 ;
		}
		else
		{
			rc = 1 ;
		}
	}

	return rc ;
}


iline* pedit01::getLabelAddress( const string& label,
				 int& rc )
{
	TRACE_FUNCTION() ;

	Data::Iterator it = getLabelIterator( label, rc ) ;

	return ( rc == 0 ) ? itr2ptr( it ) : nullptr ;
}


Data::Iterator pedit01::getLabelIterator( const string& label,
					  int& rc )
{
	//
	// Return the iterator of the data vector corresponding to the label.
	//
	// rc  0  Success.
	// rc -1  Label is not assigned to a line (or .ZCSR not on data).
	// rc -2  For .ZCSR, .ZFIRST, and .ZLAST if the data has no valid file lines.
	//
	// NOTE:
	// zdest, zfrange and zlrange contain the file line address.
	//

	TRACE_FUNCTION() ;

	int lvl ;

	Data::Iterator it = nullptr ;

	rc = 0 ;

	if ( label == ".ZCSR" )
	{
		if ( any_of( data.begin(), data.end(),
			[](iline& a )
			{
				return a.is_valid_file() ;
			} ) )
		{
			if ( macroRunning )
			{
				if ( !mRow )
				{
					rc = -1 ;
				}
				return mRow ;
			}
			else if ( !aADDR )
			{
				rc = -1 ;
			}
			return aADDR ;
		}
		else
		{
			rc = -2 ;
			return nullptr ;
		}
	}
	else if ( label == ".ZDEST" )
	{
		if ( !zdest )
		{
			rc = -1 ;
		}
		return zdest ;
	}
	else if ( label == ".ZFRANGE" )
	{
		if ( !zfrange )
		{
			rc = -1 ;
		}
		return zfrange ;
	}
	else if ( label == ".ZLRANGE" )
	{
		if ( !zlrange )
		{
			rc = -1 ;
		}
		return zlrange ;
	}

	if ( findword( label, ".ZFIRST .ZF .ZLAST .ZL" ) )
	{
		if ( any_of( data.begin(), data.end(),
			[](iline& a )
			{
				return a.is_valid_file() ;
			} ) )
		{
			if ( label == ".ZFIRST" || label == ".ZF" )
			{
				for ( it = data.begin() ; it != data.end() ; ++it )
				{
					if ( it->is_valid_file() ) { return it ; }
				}
				rc = -2 ;
				return nullptr ;
			}
			else
			{
				for ( it = data.bottom() ; it != data.begin() ; --it )
				{
					if ( it->is_valid_file() ) { return it ; }
				}
				rc = -2 ;
				return nullptr ;
			}
		}
		else
		{
			rc = -2 ;
			return nullptr ;
		}
	}

	for ( lvl = nestLevel ; lvl >= 0 ; --lvl )
	{
		for ( it = data.begin() ; it != data.end() ; ++it )
		{
			if ( it->is_valid_file() && it->compareLabel( label, lvl ) ) { return it ; }
		}
	}

	rc = -1 ;

	return nullptr ;
}


iline* pedit01::getFileLineZLABEL( const string& zlab )
{
	//
	// Return the address of the special label, zlab
	// May be null if not valid (eg No file lines, cursor not on data, etc...).
	//

	TRACE_FUNCTION() ;

	if ( zlab == ".ZFIRST" || zlab == ".ZF" )
	{
		return getFileLineZFIRST() ;
	}
	else if ( zlab == ".ZLAST" || zlab == ".ZL" )
	{
		return getFileLineZLAST() ;
	}
	else if ( zlab == ".ZCSR" )
	{
		return ( macroRunning ) ? mRow : aADDR ;
	}
	else if ( zlab == ".ZDEST" )
	{
		return zdest ;
	}
	else if ( zlab == ".ZFRANGE" )
	{
		return zfrange ;
	}
	else if ( zlab == ".ZLRANGE" )
	{
		return zlrange ;
	}

	return nullptr ;
}


int pedit01::getExclBlockSize( Data::Iterator it )
{
	//
	// Return the number of lines in an exluded block given any iterator within that block.
	// This does not included logically deleted lines.
	//

	TRACE_FUNCTION() ;

	uint exlines = 0 ;

	for ( ; it != data.begin() ; --it )
	{
		if ( it->il_deleted || it->is_excluded() ) { continue ; }
		break ;
	}
	it = getNextDataLine( it ) ;

	for ( ; it != data.end() ; ++it )
	{
		if ( it->il_deleted )        { continue ; }
		if ( it->is_not_excluded() ) { break    ; }
		++exlines ;
	}

	return exlines ;
}


int pedit01::getDataBlockSize( Data::Iterator it )
{
	//
	// Return the number of lines in an exluded block where the iterator is the first excluded line.
	// This does include logically deleted lines.
	//

	TRACE_FUNCTION() ;

	int bklines = 0 ;

	for ( ; it != data.end() ; ++it )
	{
		if ( it->is_not_excluded() && !it->il_deleted ) { break ; }
		++bklines ;
	}

	return bklines ;
}


Data::Iterator pedit01::getFirstEX( Data::Iterator it )
{
	//
	// Return the iterator of the first excluded line in a block.
	//

	TRACE_FUNCTION() ;

	Data::Iterator iy = it ;

	for ( ; it != data.begin() ; --it )
	{
		if ( it->il_deleted )    { continue ; }
		if ( it->is_excluded() ) { iy = it  ; }
		else                     { break    ; }
	}

	return iy ;
}


Data::Iterator pedit01::getLastEX( Data::Iterator it )
{
	//
	// Return the iterator of the last excluded line in a block given the iterator
	// ('it' always points to a non-deleted, excluded line).
	//

	TRACE_FUNCTION() ;

	Data::Iterator iy = it ;

	for ( ; it != data.end() ; ++it )
	{
		if ( it->is_not_excluded() && !it->il_deleted ) { break ; }
		iy = it ;
	}

	return iy ;
}


iline* pedit01::getNextSpecial( iline* tTop,
				Data::Iterator sidx,
				Data::Iterator eidx,
				char dir,
				char t )
{
	//
	// Return the next data vector line after topLine/sidx/eidx with flag il_????.
	//
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

	TRACE_FUNCTION() ;

	Data::Iterator it = nullptr ;

	bool found = false ;

	switch ( dir )
	{
		case ' ': dir = 'N' ;
		case 'N': it = getNextDataLine( topLine ) ;
			  if ( data.lt( it, sidx ) || data.gt( it, eidx ) )
			  {
				  it = sidx ;
			  }
			  break ;

		case 'P': it = getPrevDataLine( topLine ) ;
			  if ( data.lt( it, sidx ) || data.gt( it, eidx ) )
			  {
				  it = eidx ;
			  }
			  break ;

		case 'F': it = sidx ;
			  break ;

		case 'L': it = eidx ;
			  break ;
	}

	while ( true )
	{
		if ( !it->il_deleted )
		{
			switch ( t )
			{
				case 'C': if ( it->is_chng() ) { found = true ; }
					  break ;

				case 'K': if ( it->il_lcc != "" ) { found = true ; }
					  break ;

				case 'E': if ( it->is_error() ) { found = true ; }
					  break ;

				case 'I': if ( it->is_info() ) { found = true ; }
					  break ;

				case 'L': if ( it->has_label( nestLevel ) ) { found = true ; }
					  break ;

				case 'M': if ( it->is_msg() ) { found = true ; }
					  break ;

				case 'N': if ( it->is_note() ) { found = true ; }
					  break ;

				case 'U': if ( it->is_undo() ||
					       it->is_redo() ) { found = true ; }
					  break ;

				case 'X': if ( it->is_excluded() ) { found = true ; }
					  break ;

				case 'S': if ( it->is_special() ) { found = true ; }
					  break ;

				case 'T': if ( it->marked() ) { found = true ; }
					  break ;
			}
		}
		if ( found )
		{
			break ;
		}
		if ( dir == 'N' || dir == 'F' )
		{
			if ( it == eidx ) { break ; }
			it = getNextDataLine( it ) ;
		}
		else
		{
			if ( it == sidx ) { break ; }
			it = getPrevDataLine( it ) ;
		}
	}

	if ( !found )
	{
		pcmd.set_msg( "PEDT014V", 4 ) ;
		return tTop ;
	}

	return itr2ptr( it ) ;
}


iline* pedit01::locateCBLlinenum( iline* tTop,
				  const string& l )
{
	//
	// Return the line for COBOL line number l.
	//

	TRACE_FUNCTION() ;

	int lnum1 = ds2d( l ) ;
	int lnum2 ;

	iline* dl = nullptr ;

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->not_valid_file() )
		{
			continue ;
		}
		lnum2 = it->get_linenum( lnumSize1, lnumSize2, lnumS2pos ) ;
		if ( lnum2 > lnum1 )
		{
			break ;
		}
		dl = itr2ptr( it ) ;
	}

	if ( !dl )
	{
		dl = tTop ;
	}

	return dl ;
}


iline* pedit01::locateSTDlinenum( iline* tTop,
				  const string& l )
{
	//
	// Return the line for STANDARD line number l.
	//
	// If the entered number is 7 or 8, include the modification level.
	//

	TRACE_FUNCTION() ;

	int lnum1 = ds2d( l ) ;
	int lnum2 ;

	bool inc_level = ( l.size() > 6 ) ;

	iline* dl = nullptr ;

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->not_valid_file() )
		{
			continue ;
		}
		if ( inc_level )
		{
			lnum2 = it->get_linenum( lnumSize2, lnumS2pos ) ;
		}
		else
		{
			lnum2 = it->get_linenum( lnumSize1, lnumSize2, lnumS2pos ) ;
		}
		if ( lnum2 > lnum1 )
		{
			break ;
		}
		dl = itr2ptr( it ) ;
	}

	if ( !dl )
	{
		dl = tTop ;
	}

	return dl ;
}


iline* pedit01::getLastADDR( Data::Iterator it,
			     int Rpt )
{
	//
	// Get the last 'addr' for a repeat value, given an iterator and the repeat value.
	//
	// Excluded blocks always count as 1 for the repeat value.
	//

	TRACE_FUNCTION() ;

	Data::Iterator itr = it ;

	bool lastEX ;
	bool exBlock = false ;

	if ( Rpt == -1 ) { Rpt = 1 ; }

	for ( int i = 0 ; i < Rpt ; ++it )
	{
		if ( it->is_bod() )   { break    ; }
		if ( it->il_deleted ) { continue ; }
		if ( it->is_excluded() )
		{
			exBlock = true ;
		}
		else
		{
			lastEX  = exBlock ;
			exBlock = false ;
			if ( lastEX && ++i == Rpt ) { break ; }
			++i ;
		}
		itr = it ;
	}

	return itr2ptr( itr ) ;
}


void pedit01::moveTopline( iline* addr,
			   bool force_move )
{
	//
	// Change topLine so that the 'addr' will be on the screen at position targetLine after a rebuild.
	// Always position topLine if profPosFcx or force_move is set.
	//

	TRACE_FUNCTION() ;

	int t ;
	int maxDepth = ( colsOn ) ? zlvline - 1 : zlvline ;

	bool isHex ;

	if ( force_move || profPosFcx || !ADDROnScreen( addr, topLine ) )
	{
		topLine = addr ;
		isHex   = ( topLine->is_valid_file() && ( topLine->il_hex || profHex ) ) ;
		for ( t = 0 ; !topLine->is_tod() ; idecr( topLine ) )
		{
			if ( topLine->is_excluded() )
			{
				topLine = itr2ptr( getFirstEX( topLine ) ) ;
				if ( hideExcl ) { topLine = itr2ptr( getPrevDataLine( topLine ) ) ; }
				if ( topLine->is_tod() ) { break ; }
			}
			if ( topLine->is_valid_file() && ( topLine->il_hex || profHex ) )
			{
				t += 4 ;
				if ( !isHex && t > maxDepth )
				{
					topLine = getNextDataLine( topLine ) ;
					break ;
				}
			}
			else
			{
				++t ;
			}
			if ( t >= targetLine || t >= maxDepth ) { break ; }
		}
		if ( topLine->is_excluded() )
		{
			topLine = itr2ptr( getFirstEX( topLine ) ) ;
		}
	}
}


bool pedit01::ADDROnScreen( iline* addr,
			    iline* top )
{
	//
	// Return true if the 'addr' would appear on the screen on a rebuild starting at topLine=top.
	// 'addr' may be deleted for UNDO/REDO. ('addr' found after topLine and n <= maxDepth).
	//
	// An excluded block counts as 1 unless excluded lines are hidden in which case the block is ignored.
	//

	TRACE_FUNCTION() ;

	Data::Iterator it = nullptr ;

	int n ;
	int maxDepth = ( colsOn ) ? zlvline - 1 : zlvline ;

	if ( !top ) { top = topLine ; }

	for ( n = 1, it = top ; it != data.end() && n <= maxDepth && it != addr ; ++it )
	{
		if ( it->il_deleted     ) { continue ; }
		if ( it->is_excluded() )
		{
			if ( !hideExcl ) { ++n ; }
			for ( ; it != data.end() ; ++it )
			{
				if ( it->il_deleted )        { continue ; }
				if ( it->is_not_excluded() ) { break    ; }
			}
			if ( it == addr ) { break ; }
		}
		if ( it->is_valid_file() && ( profHex || it->il_hex ) )
		{
			n += 4 ;
		}
		else
		{
			++n ;
		}
	}

	return ( it != data.end() && n <= maxDepth ) ;
}


int pedit01::getRow( iline* addr )
{
	//
	// Return the screen row (starting at 1) if the 'addr' is on the screen, else 0.
	//

	TRACE_FUNCTION() ;

	int i ;

	for ( i = 0 ; i < s2data.size() ; ++i )
	{
		if ( addr == s2data[ i ].ipos_addr ) { break ; }
	}

	return ( i == s2data.size() ) ? 0 : i + 1 ;
}


uint pedit01::countVisibleLines( Data::Iterator it )
{
	//
	// Count the number of visible data lines on the screen from iterator to the bottom of the screen.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;

	iline* addr = itr2ptr( it ) ;

	iline* dl ;

	for ( i = 0 ; i < zlvline ; ++i )
	{
		if ( s2data[ i ].ipos_addr == addr ) { break ; }
	}

	for ( j = 0 ; i < zlvline ; ++i )
	{
		dl = s2data[ i ].ipos_addr ;
		if ( !dl || ( !dl->il_deleted && addr != dl ) )
		{
			++j ;
			addr = dl ;
		}
	}

	return j ;
}


void pedit01::moveDownLines( int l )
{
	//
	// Move topLine down 'l' lines.
	//

	TRACE_FUNCTION() ;

	Data::Iterator it = topLine ;

	for ( int i = 0 ; it != data.end() ; ++it )
	{
		if ( it->il_deleted ) { continue ; }
		if ( it->is_excluded() )
		{
			it = getLastEX( it ) ;
			if ( hideExcl ) { it = getNextDataLine( it ) ; }
			if ( it == data.end() ) { break ; }
		}
		if ( ++i > l ) { break ; }
	}

	if ( it->is_excluded() )
	{
		it = getFirstEX( it ) ;
	}

	topLine      = itr2ptr( it ) ;
	rebuildZAREA = true ;
}


string pedit01::overlay1( string s1,
			  string s2,
			  bool& success )
{
	//
	// Overlay s2 with s1 where there are spaces in s2.
	//
	// If dest byte is non-blank and would be overlayed with a different non-blank char, return success=false
	// (and do not overlay the char) so that lines are not deleted on a M/MM.
	//

	TRACE_FUNCTION() ;

	size_t l = max( s1.size(), s2.size() ) ;

	string s = string( l, ' ' ) ;

	s1.resize( l, ' ' ) ;
	s2.resize( l, ' ' ) ;

	for ( int i = 0 ; i < l ; ++i )
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


string pedit01::overlay2( const string& s1,
			  string s2 )
{
	//
	// Edit macro overlay.
	//
	// Overlay s2 with s1 where there are non-blanks in s1.
	//

	TRACE_FUNCTION() ;

	s2.resize( max( s1.size(), s2.size() ), ' ' ) ;

	for ( int i = 0 ; i < s1.size() ; ++i )
	{
		if ( s1[ i ] != ' ' )
		{
			s2[ i ] = s1[ i ] ;
		}
	}

	return s2 ;
}


string pedit01::templat( string s1,
			 string s2 )
{
	//
	// Edit macro template.
	//
	// format < col,literal >
	// format < col,(datavar) >
	// format <(var),literal >
	// format <(var),(datavar) >
	//
	// s1 is the template applied to string s2.
	// Parameters can be separated by a ',' or a space.
	//

	TRACE_FUNCTION() ;

	int l ;
	int p1 ;

	size_t p2 ;

	bool quote ;

	char qt ;

	string w ;

	string::iterator it ;

	s1 = s1.substr( 1, s1.size() - 2 ) ;
	trim( s1 ) ;

	for ( quote = false, it = s1.begin() ; it != s1.end() ; ++it )
	{
		if ( !quote && ( *it == '"' || *it == '\'' ) )
		{
			quote = true ;
			qt    = *it ;
			continue ;
		}
		if ( quote )
		{
			if ( *it == qt ) { quote = false ; }
		}
		else if ( *it == ',' )
		{
			*it = ' ' ;
		}
	}

	while ( s1 != "" )
	{
		w = word( s1, 1 ) ;
		idelword( s1, 1, 1 ) ;
		if ( trim( s1 ) == "" )
		{
			miBlock.seterror( "PEDM012Z", 20 ) ;
			return "" ;
		}
		if ( !isnumeric( w ) )
		{
			miBlock.seterror( "PEDM013B", w, 20 ) ;
			return "" ;
		}
		p1 = ds2d( w ) ;
		if ( p1 == 0 )
		{
			miBlock.seterror( "PEDM013B", w, 20 ) ;
			return "" ;
		}
		if ( s1.front() == '"' || s1.front() == '\'' )
		{
			p2 = s1.find( s1.front(), 1 ) ;
			if ( p2 == string::npos )
			{
				miBlock.seterror( "PEDM012W", 20 ) ;
				return "" ;
			}
			w = s1.substr( 1, p2-1 ) ;
			s1.erase( 0, p2+1 ) ;
			if ( s1 != "" && s1.front() != ' ' )
			{
				miBlock.seterror( "PEDM012W", 20 ) ;
				return "" ;
			}
		}
		else
		{
			w = word( s1, 1 ) ;
			idelword( s1, 1, 1 ) ;
		}
		trim( s1 ) ;
		l = p1 + w.size() - 1 ;
		if ( s2.size() < l ) { s2.resize( l, ' ' ) ; }
		s2.replace( p1-1, w.size(), w ) ;
	}

	return s2 ;
}


void pedit01::copyPrefix( ipline& d,
			  Data::Iterator it )
{
	//
	// Don't copy file line status (changed, error, undo, redo and marked).
	// Keep the update part of the line id_status (bits 8 - 32).
	//

	TRACE_FUNCTION() ;

	uint32_t s = it->get_idstatus() ;

	d.ip_type   = it->il_type  ;
	d.ip_excl   = it->is_excluded() ;
	d.ip_hex    = it->il_hex   ;
	d.ip_prof   = it->il_prof  ;
	d.ip_profln = it->il_profln ;
	d.ip_label  = it->get_label() ;
	d.ip_status = ( ( s << 7 ) >> 7 ) ;
}


void pedit01::copyPrefix( Data::Iterator it,
			  const ipline& s,
			  bool l )
{
	//
	// Don't copy file line status (changed, error, undo, redo and marked).
	// Copy label back for a move request.
	//

	TRACE_FUNCTION() ;

	it->il_type   = s.ip_type   ;
	it->set_excluded( s.ip_excl, level ) ;
	it->il_hex    = s.ip_hex    ;
	it->il_prof   = s.ip_prof   ;
	it->il_profln = s.ip_profln ;
	it->set_file_insert( lnumSize ) ;

	if ( l && s.ip_label != "" )
	{
		it->setLabel( s.ip_label, level ) ;
	}
}


Data::Iterator pedit01::addSpecial( LN_TYPE t,
				    Data::Iterator it,
				    vector<string>& s )
{
	//
	// Add special lines after iterator it.
	// Return the new iterator in case it has been invalidated by the insert().
	//

	TRACE_FUNCTION() ;

	for ( int i = 0 ; i < s.size() ; ++i )
	{
		it = data.insert( ++it, new iline( taskid() ) ) ;
		switch ( t )
		{
			case LN_PROF:
				    it->il_profln = i ;
				    it->il_prof   = true ;
			case LN_INFO:
			case LN_MSG:
			case LN_NOTE:
				    it->il_type = t ; break ;
			default:
				    llog( "E", "Invalid line type passed to addSpecial()"<<endl );
				    return it ;
		}
		it->put_idata( s.at( i ), level ) ;
	}
	rebuildZAREA = true ;

	return it ;
}


Data::Iterator pedit01::addSpecial( LN_TYPE t,
				    Data::Iterator it,
				    const string& s )
{
	//
	// Add special line after iterator it.
	// Return the new iterator in case it has been invalidated by the insert().
	//

	TRACE_FUNCTION() ;

	it = data.insert( ++it, new iline( taskid() ) ) ;

	switch ( t )
	{
		case LN_INFO:
		case LN_MSG:
		case LN_NOTE:
			    it->il_type = t ; break ;
		default:
			    llog( "E", "Invalid line type passed to addSpecial()"<<endl );
			    return it ;
	}

	it->put_idata( s, level )  ;
	rebuildZAREA = true ;

	return it ;
}


void pedit01::addSpecial( Data::Iterator it,
			  vector<caution>& s )
{
	//
	// Add caution lines after iterator it.
	//

	TRACE_FUNCTION() ;

	for ( int i = 0 ; i < s.size() ; ++i )
	{
		it = data.insert( ++it, new iline( taskid(), s[ i ].type ) ) ;
		it->put_idata( s[ i ].message, level ) ;
	}

	rebuildZAREA = true ;
}


void pedit01::removeSpecial( CA_TYPE catype1 )
{
	//
	// Delete caution lines for a specific type.
	//

	TRACE_FUNCTION() ;

	for ( auto it = data.begin() ; it != data.end() ; )
	{
		if ( it->is_caution( catype1 ) )
		{
			it = delete_line( it ) ;
		}
		else
		{
			++it ;
		}
	}

	rebuildZAREA = true ;
}


void pedit01::removeSpecial( CA_TYPE catype1,
			     CA_TYPE catype2 )
{
	//
	// Delete caution lines for specific types.
	//

	TRACE_FUNCTION() ;

	for ( auto it = data.begin() ; it != data.end() ; )
	{
		if ( it->is_caution( catype1 ) || it->is_caution( catype2 ) )
		{
			it = delete_line( it ) ;
		}
		else
		{
			++it ;
		}
	}

	rebuildZAREA = true ;
}


void pedit01::removeSpecial( CA_TYPE catype1,
			     CA_TYPE catype2,
			     CA_TYPE catype3 )
{
	//
	// Delete caution lines for specific types.
	//

	TRACE_FUNCTION() ;

	for ( auto it = data.begin() ; it != data.end() ; )
	{
		if ( it->is_caution( catype1 ) || it->is_caution( catype2 ) || it->is_caution( catype3 ) )
		{
			it = delete_line( it ) ;
		}
		else
		{
			++it ;
		}
	}

	rebuildZAREA = true ;
}


void pedit01::buildProfLines( vector<string>& Prof,
			      int i )
{
	TRACE_FUNCTION() ;

	string bloc = backupLoc ;

	string t ;

	Prof.clear() ;

	if ( i == 0  ) { return ; }
	if ( i == -1 ) { i = 5  ; }

	if ( backupLoc.compare( 0, zhome.size(), zhome ) == 0 )
	{
		bloc.erase( 0, zhome.size() ) ;
		bloc = "~" + bloc ;
	}

	t  = "...."  ;
	t += zedprof ;
	if ( reclen > 0 )
	{
		t += " (FIXED - " + d2ds( reclen ) + ")" ;
	}
	t += "....RECOVERY "+ ( ( profRecovery && ( recovSusp || !profUndoOn ) ) ? "SUSP" : OnOff[ profRecovery ] ) ;
	t += "....BACKUP "+ OnOff[ profBackup ]+" PATH "+ bloc ;
	t += "....NUMBER " ;
	if ( profNum )
	{
		t += ( profNumDisp ) ? "DISPLAY" : "ON" ;
		if ( profNumSTD  ) { t += " STD"   ; }
		if ( profNumCBL  ) { t += " COBOL" ; }
	}
	else
	{
		t += "OFF" ;
	}

	Prof.push_back( left( t, zdataw, '.') ) ;
	if ( i == Prof.size() ) { return ; }

	t  = "....CAPS "+ OnOff[ profCaps ] ;
	t += "....HEX " ;
	t += ( profHex ) ? ( profVert ) ? "ON VERT" : "ON DATA" : "OFF" ;
	t += "....NULLS " ;
	t += ( profNulls ) ? ( profNullA ) ? "ON ALL" : "ON STD" : "OFF" ;
	t += "....SETUNDO "+ OnOff[ profUndoOn ] ;
	if ( profUndoOn && profUndoKeep ) { t += " KEEP" ; }
	t += "....TABS "+ OnOff[ profTabs  ] ;
	if ( profTabs ) { ( tabsChar == ' ' ) ? t += ( profATabs ) ? " ALL" : " STD" : t += " " + string( 1, tabsChar ) ; }

	Prof.push_back( left( t, zdataw, '.' ) ) ;
	if ( i == Prof.size() ) { return ; }

	t  = "....AUTOSAVE "+ OnOff[ profAutoSave ] ;
	if ( !profAutoSave ) { t += ( profSaveP ) ? " PROMPT" : " NOPROMPT" ; }
	t += "....AUTONUM "+ OnOff[ profAutoNum ] ;
	t += "....XTABS "+ OnOff[ profXTabs ] ;
	if ( profXTabs ) { t += " " + d2ds(profXTabz) ; }
	if ( XTabz > 0 && profXTabz != XTabz ) { t += " (PENDING "+ d2ds(XTabz) + ")" ; }

	Prof.push_back( left( t, zdataw, '.' ) ) ;
	if ( i == Prof.size() ) { return ; }

	t  = ( profLock ) ? "....PROFILE LOCK" : "....PROFILE UNLOCK" ;
	t += "....IMACRO "+ profIMACRO ;
	t += "....NOTE "+ OnOff[ profNotes ] ;
	t += "....STATS "+ OnOff[ profStats ] ;

	Prof.push_back( left( t, zdataw, '.' ) ) ;
	if ( i == Prof.size() ) { return ; }

	t = "....HILITE " ;
	if ( profHilight )
	{
		t += ( profLang == "AUTO" ) ? detLang : profLang ;
		if ( profIfLogic )
		{
			t += ( profDoLogic ) ? " LOGIC" : " IFLOGIC" ;
		}
		else if ( profDoLogic )
		{
			t += " DOLOGIC" ;
		}
	}
	else
	{
		t += "OFF" ;
	}
	if ( profParen      ) { t += " PAREN"  ; }
	if ( profCsrPhrase  ) { t += " CURSOR" ; }
	if ( profFindPhrase ) { t += " FIND"   ; }

	Prof.push_back( left( t, zdataw, '.' ) ) ;
}


void pedit01::updateProfLines( vector<string>& Prof )
{
	TRACE_FUNCTION() ;

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->is_prof_prof() && Prof.size() > it->il_profln )
		{
			it->put_idata( Prof[ it->il_profln ] ) ;
			rebuildZAREA = true ;
		}
	}
}


void pedit01::cleanupRedoStacks()
{
	//
	// If there are entries on any redo data stack (Redo_data flag) but the Global redo level is 0,
	// remove redo data from all lines as this data is no longer required.
	//
	// If there are entries on any redo non-data stack (Redo_other flag) and there is only redo data
	// for labels or exclude lines, if either have since changed then clear both.
	//

	TRACE_FUNCTION() ;

	if ( iline::has_Redo_data( taskid() ) && iline::get_Global_Redo_level( taskid() ) == 0 )
	{
		for_each( data.begin(), data.end(),
			[](iline& a)
			{
				a.remove_redo_idata() ;
				a.remove_redo_label() ;
				a.remove_redo_excl()  ;
			} ) ;
		iline::remove_redo_status( taskid() ) ;
		iline::reset_Redo_data( taskid() ) ;
		iline::reset_Redo_other( taskid() ) ;
		iline::reset_changed_icond( taskid() ) ;
		iline::reset_changed_ilabel( taskid() ) ;
		iline::reset_changed_xclud( taskid() ) ;
	}
	else if ( iline::has_Redo_other( taskid() ) &&
		( iline::has_changed_ilabel( taskid() ) || iline::has_changed_xclud( taskid() ) || iline::has_changed_icond( taskid() ) ) )
	{
		for_each( data.begin(), data.end(),
			[](iline& a)
			{
				a.remove_redo_label() ;
				a.remove_redo_excl() ;
			} ) ;
		iline::reset_changed_icond( taskid() ) ;
		iline::reset_changed_ilabel( taskid() ) ;
		iline::reset_changed_xclud( taskid() ) ;
		iline::reset_Redo_other( taskid() ) ;
	}
}


void pedit01::removeRecoveryData()
{
	//
	// Delete all logically deleted lines and remove entries from the data container.
	// Flatten all remaining data, labels and excl and set idata level to 0.
	// Clear the global Undo/Redo/File stacks.
	//

	TRACE_FUNCTION() ;

	level = 0 ;

	for ( auto it = data.begin() ; it != data.end() ; )
	{
		if ( it->il_deleted )
		{
			it = delete_line( it ) ;
		}
		else
		{
			it->flatten_idata() ;
			it->flatten_label() ;
			it->flatten_excl() ;
			++it ;
		}
	}

	iline::clear_status( taskid() ) ;
	store_status( SS_ALL ) ;

	iline::clear_Global_Undo( taskid() ) ;
	iline::clear_Global_Redo( taskid() ) ;
	iline::clear_Global_File_level( taskid() ) ;

	pcmd.set_msg( "PEDT012H", 4 ) ;
}


void pedit01::removeProfLines()
{
	//
	// Delete PROFILE lines in Data.
	//

	TRACE_FUNCTION() ;

	for ( auto it = data.begin() ; it != data.end() ; )
	{
		if ( it->is_prof() )
		{
			it = delete_line( it ) ;
		}
		else
		{
			++it ;
		}
	}
}


void pedit01::removeSpecialLines( Data::Iterator its,
				  Data::Iterator ite )
{
	//
	// Delete all temporary lines from the Data vector (can be undone).
	// (col,prof,tabs,mask,bnds,msg,info and note).
	//

	TRACE_FUNCTION() ;

	for ( auto it = its ; it != ite ; )
	{
		if ( it->is_special() )
		{
			it = delete_line( it ) ;
		}
		else
		{
			++it ;
		}
	}
}


void pedit01::create_copy( Cut& cut,
			   vector<ipline>& vip )
{
	//
	// Create a copy of the data in vip using Cut object criteria.
	//

	TRACE_FUNCTION() ;

	for ( auto it = cut.begin() ; it != cut.end() ; ++it )
	{
		if ( it->not_valid_file() || ( cut.marked() && !it->marked() ) )
		{
			continue ;
		}
		if ( cut.x() )
		{
			if ( it->is_excluded() )
			{
				vip.push_back( ipline( LN_FILE, it->get_idata() ) ) ;
			}
		}
		else if ( cut.nx() )
		{
			if ( it->is_not_excluded() )
			{
				vip.push_back( ipline( LN_FILE, it->get_idata() ) ) ;
			}
		}
		else
		{
			vip.push_back( ipline( LN_FILE, it->get_idata() ) ) ;
		}
	}
}


bool pedit01::copyToClipboard( vector<ipline>& vip )
{
	//
	// Copy only data lines in vip (from copy/move) to lspf table clipTable.
	//
	// cutReplace - clear clipBoard before copy, else append at end of current contents.
	// A message will be issued if the clipboard is locked (Error when running from a macro).
	//

	TRACE_FUNCTION() ;

	int i ;
	int t ;
	int sz ;

	bool newclip = true ;

	string zedcname ;
	string zedcstat ;
	string zedcdesc ;
	string zedctext ;

	const string vlist = "ZEDCNAME ZEDCSTAT ZEDCDESC ZEDCTEXT" ;

	vdefine( vlist, &zedcname, &zedcstat, &zedcdesc, &zedctext ) ;

	tbopen( clipTable, WRITE, zuprof ) ;
	if ( RC == 8 )
	{
		tbcreate( clipTable,
			  "",
			  "(ZEDCNAME,ZEDCTEXT)",
			  WRITE,
			  NOREPLACE,
			  zuprof ) ;
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
			tbend( clipTable ) ;
			vdelete( vlist ) ;
			vreplace( "ZEDCLIP", zedcname ) ;
			pcmd.set_msg( "PEDT015V", clipBoard, 8, 12 ) ;
			return false ;
		}
		if ( cutReplace )
		{
			clearClipboard( zedcname ) ;
		}
		else
		{
			tbbottom( clipTable ) ;
			tbscan( clipTable, "", "", "", "PREVIOUS", "NOREAD" ) ;
			newclip = false ;
		}
	}

	tbvclear( clipTable ) ;
	zedcname = clipBoard  ;

	sz = vip.size() ;
	for ( t = 0, i = 0 ; i < sz ; ++i )
	{
		if ( vip[ i ].is_file() )
		{
			zedctext = vip[ i ].ip_data ;
			if ( t == 0 && newclip )
			{
				zedcdesc = ( clipBoard == "DEFAULT" ) ? "Default clipboard" : "" ;
				zedcstat = "UP" ;
				tbadd( clipTable, "(ZEDCDESC,ZEDCSTAT)", "", min( 32767, sz ) ) ;
			}
			else
			{
				tbadd( clipTable, "", "", min( 32767, sz ) ) ;
			}
			if ( RC > 0 ) { uabend( "PEDT015W", "TBADD" ) ; }
			++t ;
		}
	}

	tbclose( clipTable, "", zuprof ) ;

	pcmd.set_msg( "PEDT013C", 4, 0 ) ;
	vdelete( vlist ) ;
	vreplace( "ZEDLNES", d2ds( t ) )  ;
	vreplace( "ZEDCNAME", clipBoard ) ;

	return true ;
}


void pedit01::getClipboard( vector<ipline>& vip )
{
	//
	// Get lines from clipBoard and put them in vector vip.
	// pasteKeep - don't clear clipBoard after paste (a locked clipboard will always be kept).
	//

	TRACE_FUNCTION() ;

	string zedcname ;

	ipline ip( LN_FILE ) ;

	vip.clear() ;

	vdefine( "ZEDCNAME ZEDCTEXT", &zedcname, &ip.ip_data ) ;

	tbopen( clipTable, WRITE, zuprof ) ;
	if ( RC > 0 )
	{
		uabend( "PEDT015W", "TBOPEN" ) ;
	}

	tbvclear( clipTable ) ;

	zedcname = clipBoard  ;
	tbsarg( clipTable, "", "NEXT", "(ZEDCNAME,EQ)" ) ;

	tbtop( clipTable ) ;
	while ( RC == 0 )
	{
		tbscan( clipTable ) ;
		if ( RC == 0 )
		{
			vip.push_back( ip ) ;
		}
	}

	if ( !pasteKeep )
	{
		clearClipboard( zedcname ) ;
	}

	tbclose( clipTable, "", zuprof ) ;
	vdelete( "ZEDCNAME ZEDCTEXT" ) ;
}


void pedit01::clearClipboard( string clip )
{
	//
	// Pass 'clip' by value as this has been vdefined as zedcname so will be cleared by a tbvclear()
	// if passed by reference.  Don't clear clipboard if is has been locked (ZEDCSTAT = RO).
	//

	TRACE_FUNCTION() ;

	string t ;

	tbvclear( clipTable ) ;

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
		tbdelete( clipTable ) ;
	}

	while ( RC == 0 )
	{
		tbscan( clipTable, "", "", "", "", "NOREAD" ) ;
		if ( RC == 0 )
		{
			tbdelete( clipTable ) ;
		}
	}

	tbtop( clipTable ) ;
}


void pedit01::manageClipboard()
{
	TRACE_FUNCTION() ;

	int i ;

	bool rebuild = false ;

	string suf  ;
	string desc ;
	string lc   ;
	string stat ;
	string name ;

	vector<string> descr ;

	manageClipboard_create( descr ) ;
	if ( pcmd.msgset() ) { return ; }

	addpop( "", 4, 5 ) ;
	while ( true )
	{
		display( "PEDIT017" ) ;
		if ( RC == 8 ) { break ; }
		for ( i = 0 ; i < descr.size() && i < 11 ; ++i )
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
				pcmd.clear_msg() ;
				rebuild = true ;
			}
			else if ( lc == "R" )
			{
				manageClipboard_rename( name, desc ) ;
				rebuild = true ;
			}
			else if ( lc == "D" || lc == "C" )
			{
				manageClipboard_delete( name ) ;
				rebuild = true ;
			}
		}
		if ( rebuild )
		{
			manageClipboard_create( descr ) ;
			if ( pcmd.error() ) { rempop() ; return ; }
			rebuild = false ;
		}
	}
	rempop() ;
}


void pedit01::manageClipboard_create( vector<string>& descr )
{
	TRACE_FUNCTION() ;

	int i ;
	int l ;

	string t ;
	string suf ;

	string zedcname ;
	string zedcstat ;
	string zedcdesc ;
	string zedctext ;

	descr.clear() ;

	const string vlist = "ZEDCNAME ZEDCSTAT ZEDCDESC ZEDCTEXT" ;

	vdefine( vlist, &zedcname, &zedcstat, &zedcdesc, &zedctext ) ;

	tbopen( clipTable, NOWRITE, zuprof ) ;
	if ( RC > 0 )
	{
		vdelete( vlist ) ;
		pcmd.set_msg( "PEDT014Y", 12 ) ;
		return ;
	}

	t = "" ;
	i = 0  ;
	l = 0  ;

	tbtop( clipTable ) ;
	while ( i < 11 )
	{
		tbskip( clipTable ) ;
		if ( RC > 0 ) { break ; }
		if ( t != zedcname )
		{
			if ( l > 0 )
			{
				vreplace( "CLPLN"+ d2ds( i + 1 ), d2ds( l ) ) ;
				++i ;
			}
			t   = zedcname ;
			l   = 0 ;
			suf = d2ds( i + 1 ) ;
			vreplace( "CLPLC"+ suf, ""       ) ;
			vreplace( "CLPNM"+ suf, zedcname ) ;
			vreplace( "CLPDS"+ suf, zedcdesc ) ;
			vreplace( "CLPST"+ suf, zedcstat ) ;
			descr.push_back( zedcdesc ) ;
		}
		++l ;
	}

	tbend( clipTable ) ;
	if ( t == "" )
	{
		vdelete( vlist ) ;
		pcmd.set_msg( "PEDT014Y", 12 ) ;
		return ;
	}

	if ( l > 0 )
	{
		vreplace( "CLPLN"+ d2ds( i + 1 ), d2ds( l ) ) ;
	}

	for ( ++i ; i < 11 ; ++i )
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


void pedit01::manageClipboard_descr( const string& name,
				     const string& desc )
{
	TRACE_FUNCTION() ;

	tbopen( clipTable, WRITE, zuprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbvclear( clipTable ) ;

	vreplace( "ZEDCNAME", name ) ;
	vreplace( "ZEDCSTAT", "UP" ) ;

	tbsarg( clipTable, "ZEDCSTAT" ) ;
	tbscan( clipTable ) ;
	if ( RC > 0 )
	{
		tbend( clipTable ) ;
		return ;
	}

	vreplace( "ZEDCDESC", desc ) ;

	tbput( clipTable, "(ZEDCDESC,ZEDCSTAT)" ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBPUT" ) ; }

	tbclose( clipTable, "", zuprof ) ;
}


void pedit01::manageClipboard_browse( const string& name )
{
	TRACE_FUNCTION() ;

	string tname = createTempName() ;

	vector<ipline> vip ;

	clipBoard = name  ;
	pasteKeep = true  ;
	getClipboard( vip ) ;

	std::ofstream fout( tname.c_str() ) ;
	for ( int i = 0 ; i < vip.size() ; ++i )
	{
		fout << vip[ i ].ip_data << endl ;
	}
	fout.close() ;

	set_dialogue_var( "ZBRALT", "CLIPBOARD:"+name ) ;

	control( "ERRORS", "RETURN" ) ;
	browse( tname ) ;
	control( "ERRORS", "CANCEL" ) ;
	remove( tname ) ;
	verase( "ZBRALT", SHARED ) ;
}


void pedit01::manageClipboard_edit( const string& name,
				    const string& desc )
{
	TRACE_FUNCTION() ;

	string tname = createTempName() ;

	vector<ipline> vip ;

	ipline ip( LN_FILE ) ;

	clipBoard = name    ;
	pasteKeep = true    ;
	getClipboard( vip ) ;

	std::ofstream fout( tname.c_str() ) ;
	for ( int i = 0 ; i < vip.size() ; ++i )
	{
		fout << vip[ i ].ip_data << endl ;
	}
	fout.close() ;

	set_dialogue_var( "ZEDALT", "CLIPBOARD:"+name ) ;

	edit( tname ) ;
	if ( RC == 0 )
	{
		vip.clear() ;
		std::ifstream fin( tname.c_str() ) ;
		while ( getline( fin, ip.ip_data ) )
		{
			vip.push_back( ip ) ;
		}
		fin.close() ;

		cutReplace = true ;
		copyToClipboard( vip ) ;
		manageClipboard_descr( name, desc ) ;
	}

	remove( tname ) ;
	verase( "ZEDALT", SHARED ) ;
}


void pedit01::manageClipboard_toggle( int i,
				      const string& name )
{
	TRACE_FUNCTION() ;

	string t ;

	tbopen( clipTable, WRITE, zuprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbvclear( clipTable ) ;

	vreplace( "ZEDCNAME", name ) ;
	vreplace( "ZEDCSTAT", "*"  ) ;

	tbsarg( clipTable, "ZEDCSTAT", "NEXT", "(ZEDCSTAT,EQ)" ) ;
	tbscan( clipTable ) ;
	if ( RC > 0 )
	{
		tbend( clipTable ) ;
		return ;
	}

	vcopy( "ZEDCSTAT", t ) ;
	t = ( t == "RO" ) ? "UP" : "RO" ;
	vreplace( "ZEDCSTAT", t ) ;
	vreplace( "CLPST"+ d2ds( i+1 ), t ) ;

	tbput( clipTable, "(ZEDCDESC,ZEDCSTAT)" ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBPUT" ) ; }

	tbclose( clipTable, "", zuprof ) ;
}


void pedit01::manageClipboard_rename( const string& name,
				      const string& desc )
{
	TRACE_FUNCTION() ;

	string newname ;
	string newdesc ;

	vector<ipline> vip ;

	vreplace( "OLDNAME", name ) ;
	vreplace( "NEWNAME", name ) ;
	vreplace( "NEWDESC", desc ) ;

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


void pedit01::manageClipboard_delete( const string& name )
{
	TRACE_FUNCTION() ;

	tbopen( clipTable, WRITE, zuprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	clearClipboard( name ) ;

	tbclose( clipTable, "", zuprof ) ;
}


void pedit01::cleanup_custom()
{
	//
	// Called if there is an abnormal termination in the program.
	// Give the recovery subtask another chance to create a recovery file
	// if necessary, and then shut it down.
	//
	// Release storage and clear the data vector.
	//
	// Note: By default, cleanup_custom runs with CONTROL ERRORS RETURN.
	//

	TRACE_FUNCTION() ;

	llog( "E", "Control given to EDIT cleanup procedure due to an abnormal event" <<endl ) ;

	if ( data.size() == 0 )
	{
		llog( "I", "No data found." <<endl ) ;
		deq( spfedit, zfile ) ;
		stopRecoveryTask() ;
		return ;
	}

	if ( abendComplete )
	{
		llog( "W", "Cleanup routine called a second time.  Ignoring..." <<endl ) ;
		deq( spfedit, zfile ) ;
		stopRecoveryTask() ;
		return ;
	}

	if ( !profRecovery || recovSusp )
	{
		llog( "W", "Recovery file not saved as RECOVERY is set off or suspended." <<endl ) ;
		deq( spfedit, zfile ) ;
		stopRecoveryTask() ;
		return ;
	}

	if ( recvStatus != RECV_RUNNING )
	{
		llog( "W", "Recovery subtask is not active during abnormal termination." <<endl ) ;
		deq( spfedit, zfile ) ;
		return ;
	}

	if ( !fileChanged )
	{
		llog( "I", "Recovery file not saved as no changes made during edit session or since last save." <<endl ) ;
		deq( spfedit, zfile ) ;
		stopRecoveryTask() ;
		return ;
	}

	canBackup = true ;
	cond_recov.notify_all() ;
	boost::this_thread::sleep_for( boost::chrono::milliseconds( 100 ) ) ;

	stopRecoveryTask() ;

	deq( spfedit, zfile ) ;

	abendComplete = true ;

	llog( "I", "Edit cleanup complete." << endl ) ;
}


void pedit01::copyFileData( vector<line_data>& tdata,
			    bool only_x,
			    bool only_nx,
			    Data::Iterator its,
			    Data::Iterator ite,
			    size_t& l )
{
	//
	// Copy valid file entries between its and ite to vector tdata.  Pad all records with 0x00
	// to the longest (used in sort so all records should be the same length).
	//

	TRACE_FUNCTION() ;

	l = 0 ;

	line_data t ;

	for ( ; its != ite ; ++its )
	{
		if ( its->is_valid_file() )
		{
			if ( ( !only_x  && !only_nx ) ||
			     (  only_x  && its->is_excluded() ) ||
			     (  only_nx && its->is_not_excluded() ) )
			{
				t.set_string( *its ) ;
				l = max( l, t.get_string_len() ) ;
				its->set_condition( LS_NONE, level ) ;
				tdata.push_back( t ) ;
			}
		}
	}

	for ( auto it = tdata.begin() ; it != tdata.end() ; ++it )
	{
		it->resize( l ) ;
	}
}


void pedit01::reflowData( vector<string>& tdata1,
			  int tf_col,
			  int ind1,
			  int ind2 )
{
	//
	// Reflow text in tdata1.
	//
	// tf_col - text flow column
	// ind1   - first line indentation
	// ind2   - next line indentation
	//

	TRACE_FUNCTION() ;

	int di  ;
	int gap ;
	int ind ;
	int tlen1 ;
	int tlen2 ;
	int tlen  ;

	if ( tdata1.empty() ) { return ; }

	bool boundary1 = ( LeftBnd  > 1 && RightBnd == 0 ) ;
	bool boundary2 = ( RightBnd > 0 ) ;

	size_t p1 ;

	string t1 ;
	string t2 ;
	string t3 ;

	vector<string> tdata2 = tdata1 ;

	tdata1.clear() ;

	tlen1 = tf_col - LeftBnd + 1 - ind1 ;
	tlen2 = tf_col - LeftBnd + 1 - ind2 ;
	gap   = RightBnd - LeftBnd + 1 ;

	if ( tlen1 < 0 ) { tlen1 = 0 ; }
	if ( tlen2 < 0 ) { tlen2 = 0 ; }

	t1   = "" ;
	di   = 0  ;
	tlen = tlen1 ;
	ind  = ind1 ;

	for ( auto it = tdata2.begin() ; it != tdata2.end() ; ++it )
	{
		if ( t1.size() > 0 )
		{
			if ( t1.back() == '.' ) { t1.push_back( ' ' ) ; }
			t1.push_back( ' ' ) ;
		}
		if ( boundary1 )
		{
			t1 += strip( it->substr( LeftBnd-1 ) ) ;
		}
		else if ( boundary2 )
		{
			t1 += strip( it->substr( LeftBnd-1, gap ) ) ;
		}
		else
		{
			t1 += strip( *it ) ;
		}
		while ( t1.size() > tlen )
		{
			p1 = t1.find_last_of( ' ', tlen ) ;
			if ( p1 != string::npos )
			{
				t2 = t1.substr( 0, p1+1 ) ;
				t1.erase( 0, p1+1 ) ;
			}
			else
			{
				t2 = word( t1, 1 ) ;
				idelword( t1, 1, 1 ) ;
			}
			trim( t1 ) ;
			if ( boundary1 )
			{
				t3 = ( di < tdata2.size() ) ? tdata2[ di ] : "" ;
				t3.resize( LeftBnd-1, ' ' ) ;
				t2 = t3 + string( ind, ' ' ) + t2 ;
			}
			else if ( boundary2 )
			{
				t3 = ( di < tdata2.size() ) ? tdata2[ di ] : "" ;
				t2 = left( t3, LeftBnd-1 ) +
				     string( ind, ' ' ) + left( t2, gap-ind ) +
				     substr( t3, RightBnd+1 ) ;
			}
			else
			{
				t2 = string( ind, ' ' ) + t2 ;
			}
			if ( reclen > 0 )
			{
				t2.resize( reclen, ' ' ) ;
			}
			else
			{
				trim_right( t2 ) ;
			}
			tdata1.push_back( t2 ) ;
			tlen = tlen2 ;
			ind  = ind2  ;
			++di ;
		}
	}

	trim( t1 ) ;
	if ( t1.size() > 0 )
	{
		if ( boundary1 )
		{
			t3 = ( di < tdata2.size() ) ? tdata2[ di ] : "" ;
			t3.resize( LeftBnd-1, ' ' ) ;
			t1 = t3 + string( ind, ' ' ) + t1 ;
		}
		else if ( boundary2 )
		{
			t3 = ( di < tdata2.size() ) ? tdata2[ di ] : "" ;
			t1 = left( t3, LeftBnd-1 ) +
			     string( ind, ' ' ) + left( t1, gap-ind ) +
			     substr( t3, RightBnd+1 ) ;
		}
		else
		{
			t1 = string( ind, ' ' ) + t1 ;
		}
		if ( reclen > 0 )
		{
			t1.resize( reclen, ' ' ) ;
		}
		else
		{
			trim_right( t1 ) ;
		}
		tdata1.push_back( t1 ) ;
		++di ;
	}


	if ( boundary1 || boundary2 )
	{
		while ( di < tdata2.size() )
		{
			t3 = tdata2[ di ] ;
			if ( boundary1 )
			{
				t1 = t3.substr( 0, LeftBnd-1 ) ;
			}
			else
			{
				t1 = left( t3, LeftBnd-1 ) + string( gap, ' ' ) + substr( t3, RightBnd+1 ) ;
			}
			if ( reclen > 0 )
			{
				t1.resize( reclen, ' ' ) ;
			}
			else
			{
				trim_right( t1 ) ;
			}
			tdata1.push_back( t1 ) ;
			++di ;
		}
		while ( is_line_blank( tdata1.back() ) )
		{
			tdata1.pop_back() ;
		}
	}

	if ( tdata1 == tdata2 )
	{
		tdata1.clear() ;
	}
}


bool pedit01::is_line_blank( const string& s )
{
	//
	// Return true if line is blank, ignoring the number areas.
	//

	TRACE_FUNCTION() ;

	if ( lnumSize1 == 0 )
	{
		if ( lnumS2pos == 0 )
		{
			return ( strip( s ) == "" ) ;
		}
		else
		{
			return ( strip( s.substr( 0, lnumS2pos ) ) == "" ) ;
		}
	}
	else if ( lnumS2pos == 0 )
	{
		return ( strip( s.substr( lnumSize1 ) ) == "" ) ;
	}
	else
	{
		return ( strip( s.substr( lnumSize1, lnumS2pos - lnumSize1 ) ) == "" ) ;
	}

	return false ;
}


void pedit01::compare_files( string s )
{
	//
	// Simple compare routine using output from the diff command.
	// Compare edit version of file with entered file name.
	//
	// Differences displayed in browse or with INFO lines.
	//    Special labels .Onnnn signify lines added to the edit file.
	//    INFO lines ====== are lines deleted from the edit file.
	//
	// All INFO lines and special labels are cleared first.
	//

	TRACE_FUNCTION() ;

	Data::Iterator top = nullptr ;
	Data::Iterator it  = nullptr ;

	int rc ;

	uint s1 ;
	uint s2 ;
	uint d1 ;
	uint d2 ;

	uint add = 0 ;
	uint del = 0 ;

	size_t i ;
	size_t pos ;

	char ltp ;

	bool exclude = false ;

	string t1 ;
	string t2 ;
	string cmd ;
	string file ;
	string vlist ;
	string label ;
	string dparms ;
	string tname1 ;
	string tname2 ;
	string inLine ;

	string cfile ;

	string ecpbrdf ;
	string ecpicas ;
	string ecpiref ;
	string ecpiblk ;
	string ecpitbe ;
	string ecpnxno ;
	string ecplpre ;

	string* pt ;

	string spaces = string( profXTabz, ' ' ) ;

	char buffer[ 2048 ] ;

	vector<string>changes ;

	vlist = "CFILE ECPBRDF ECPICAS ECPIREF ECPIBLK ECPITBE ECPNXNO ECPLPRE" ;

	vdefine( vlist, &cfile, &ecpbrdf, &ecpicas, &ecpiref, &ecpiblk, &ecpitbe, &ecpnxno, &ecplpre ) ;
	vget( vlist, PROFILE ) ;

	exclude = parseString2( s, "X EXCLUDE" ) ;

	if ( exclude && s == "" )
	{
		for_each( data.begin(), data.end(),
			[ this ](iline& a)
			{
				a.set_unexcluded( level ) ;
			} ) ;
		compare_exclude( ecpnxno ) ;
		rebuildZAREA = true ;
		vdelete( vlist ) ;
		return ;
	}

	if ( s == "" || s == "-" )
	{
		if ( macroRunning )
		{
			pcmd.set_msg( "PEDM012J", 12 ) ;
			vdelete( vlist ) ;
			return ;
		}
		pcmd.clear_msg() ;
		addpop( "", 2, 4 ) ;
		while ( true )
		{
			display( "PEDIT013", pcmd.get_msg() ) ;
			if ( RC > 0 ) { break ; }
		}
		rempop() ;
		vdelete( vlist ) ;
		return ;
	}
	else if ( s == "*" || upper( s ) == "SESSION" )
	{
		cfile = zfile ;
	}
	else if ( s == "/" )
	{
		if ( macroRunning )
		{
			pcmd.set_msg( "PEDM012J", 12 ) ;
			vdelete( vlist ) ;
			return ;
		}
		cfile = displayPanel( "PEDIT01M", "CFILE", "PEDT011R" ) ;
		if ( RC == 8 )
		{
			vdelete( vlist ) ;
			return ;
		}
	}
	else
	{
		cfile = expandFileName( s ) ;
	}

	try
	{
		if ( !is_regular_file( cfile ) )
		{
			pcmd.set_msg( "PEDT013O", cfile, 8 ) ;
			vdelete( vlist ) ;
			return ;
		}
	}
	catch ( const filesystem_error& ex )
	{
		vreplace( "ZVAL1", ex.what() ) ;
		pcmd.set_msg( "PSYS012C", 20 ) ;
		vdelete( vlist ) ;
		return ;
	}

	if ( ecpbrdf == "/" && !macroRunning ) { dparms = " -y " ; }

	if ( ecpicas == "/" ) { dparms += " -i " ; }
	if ( ecpiref == "/" ) { dparms += " -b " ; }
	if ( ecpiblk == "/" ) { dparms += " -B " ; }
	if ( ecpitbe == "/" ) { dparms += " -E " ; }

	tname1 = createTempName() ;
	tname2 = createTempName() ;

	std::ofstream fout( tname1.c_str() ) ;
	std::ofstream of ;

	for ( it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->not_valid_file() ) { continue ; }
		pt = it->get_idata_ptr() ;
		if ( profXTabs )
		{
			t1 = "" ;
			for ( i = 0 ; i < pt->size() ; ++i )
			{
				if ( (i % profXTabz == 0)         &&
				     pt->size() > (i+profXTabz-1) &&
				     pt->compare( i, profXTabz, spaces ) == 0 )
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

	cmd = "diff " + dparms + tname1 + " '" + cfile + "' 2>&1" ;
	FILE* pipe{ popen( cmd.c_str(), "r" ) } ;
	if ( !pipe )
	{
		llog( "E", "POPEN failed for command "<< cmd <<endl ) ;
		pcmd.set_msg( "PEDT017L", 20 ) ;
		remove( tname1 ) ;
		remove( tname2 ) ;
		vdelete( vlist ) ;
		return ;
	}

	of.open( tname2 ) ;
	while ( fgets( buffer, sizeof( buffer ), pipe ) )
	{
		file = buffer ;
		if ( file != "" && file.back() == 0x0a )
		{
			file.pop_back() ;
		}
		of << file << endl ;
	}

	rc = WEXITSTATUS( pclose( pipe ) ) ;

	of.close() ;
	remove( tname1 ) ;

	if ( rc == 0 )
	{
		pcmd.set_msg( "PEDT017J", 4, 0 ) ;
		remove( tname2 ) ;
		vdelete( vlist ) ;
		return ;
	}
	else if ( rc > 1 )
	{
		pcmd.set_msg( "PEDT017M", 20 ) ;
		std::ifstream fin( tname2.c_str() ) ;
		while ( getline( fin, inLine ) )
		{
			llog( "E", inLine << endl) ;
		}
		fin.close() ;
		remove( tname2 ) ;
		vdelete( vlist ) ;
		return ;
	}

	if ( ecpbrdf == "/" && !macroRunning )
	{
		set_dialogue_var( "ZBRALT", "COMPARE: NEW - OLD" ) ;
		control( "ERRORS", "RETURN" ) ;
		browse( tname2 ) ;
		if ( RC == 12 )
		{
			pcmd.set_msg( "PEDT017J", 4 ) ;
		}
		control( "ERRORS", "CANCEL" ) ;
		verase( "ZBRALT", SHARED ) ;
		remove( tname2 ) ;
		vdelete( vlist ) ;
		return ;
	}

	for ( auto it = data.begin() ; it != data.end() ; )
	{
		it->clearSpecialLabel( level ) ;
		it->set_unexcluded( level ) ;
		if ( it->is_special() )
		{
			it = delete_line( it ) ;
		}
		else
		{
			++it ;
		}
	}

	label = "." + ( ( ecplpre == "" ) ? "O" : ecplpre ) + "AAAA" ;

	std::ifstream fin( tname2.c_str() ) ;
	while ( getline( fin, inLine ) )
	{
		if ( inLine == "\\ No newline at end of file" ) { continue ; }
		pos = inLine.find_first_of( "acd" ) ;
		ltp = inLine[ pos ] ;
		t1  = inLine.substr( 0, pos ) ;
		t2  = inLine.substr( pos + 1 ) ;
		pos = t1.find( ',' ) ;
		s1  = ds2d( t1.substr( 0, pos ) ) ;
		s2  = ( pos == string::npos ) ? s1 : ds2d( t1.substr( pos + 1 ) ) ;
		pos = t2.find( ',' ) ;
		d1  = ds2d( t2.substr( 0, pos ) ) ;
		d2  = ( pos == string::npos ) ? d1 : ds2d( t2.substr( pos + 1 ) ) ;
		if ( ltp == 'a' )
		{
			changes.clear() ;
			for ( uint i = d1 ; i <= d2 ; ++i )
			{
				getline( fin, inLine ) ;
				changes.push_back( convertTabs( inLine.substr( 2 ) ) ) ;
				++del ;
			}
			auto it = getDataLine( s1+1 ) - 1 ;
			if ( top == nullptr ) { top = it ; }
			addSpecial( LN_INFO, it, changes ) ;
		}
		else if ( ltp == 'd' )
		{
			if ( top == nullptr ) { top = getDataLine( s1 ) ; }
			for ( uint i = s1 ; i <= s2 ; ++i )
			{
				getline( fin, inLine ) ;
				getDataLine( i )->setLabel( label, level ) ;
				label = genNextLabel( label ) ;
				++add ;
			}
		}
		else if ( ltp == 'c' )
		{
			for ( uint i = s1 ; i <= s2 ; ++i )
			{
				getline( fin, inLine ) ;
				getDataLine( i )->setLabel( label, level ) ;
				label = genNextLabel( label ) ;
				++add ;
			}
			getline( fin, inLine ) ;
			changes.clear() ;
			for ( uint i = d1 ; i <= d2 ; ++i )
			{
				getline( fin, inLine ) ;
				changes.push_back( convertTabs( inLine.substr( 2 ) ) ) ;
				++del ;
			}
			auto it = getDataLine( s1 ) ;
			if ( top == nullptr ) { top = it ; }
			addSpecial( LN_INFO, --it, changes ) ;
		}
	}
	fin.close() ;
	remove( tname2 ) ;

	pcmd.set_msg( "PEDT017K", d2ds( add ), d2ds( del ), 4, 0 ) ;

	topLine = itr2ptr( getPrevDataLine( top ) ) ;

	if ( exclude )
	{
		compare_exclude( ecpnxno ) ;
	}

	vdelete( vlist ) ;
	rebuildZAREA = true ;
}


void pedit01::compare_exclude( const string& nxno )
{
	//
	// Exclude lines except labelled and info lines keeping nxno lines
	// around these, unexcluded.
	//

	TRACE_FUNCTION() ;

	bool excl ;

	int i = ds2d( nxno ) ;
	int j ;

	Data::Iterator it2 = nullptr ;

	for ( auto it1 = data.begin() ; it1 != data.end() ; ++it1 )
	{
		if ( it1->is_valid_file() || it1->is_info() )
		{
			if ( !it1->has_label() && !it1->is_info() )
			{
				excl = true ;
				for ( it2 = it1 + 1, j = i ; it2 != data.end() && j > 0 ; ++it2, --j )
				{
					if ( it2->is_bod() )
					{
						break ;
					}
					else if ( it2->has_label() || it2->is_info() )
					{
						excl = false ;
						break ;
					}
					if ( it2->not_valid_file() )
					{
						++j ;
					}
				}
				if ( excl )
				{
					it1->set_excluded( level ) ;
				}
			}
			else
			{
				for ( j = i ; it1 != data.end() && j >= 0 ; ++it1, --j )
				{
					if ( it1->has_label() || it1->is_info() )
					{
						j = i ;
					}
					if ( it1->not_valid_file() && !it1->is_info() )
					{
						++j ;
					}
				}
				if ( it1 == data.end() ) { break ; }
				--it1 ;
			}
		}
	}
}


string pedit01::displayPanel( const string& pname,
			      const string& fname,
			      const string& msgid,
			      bool must_check,
			      bool must_exist )
{
	//
	// Display panel pname and expand entered file name.
	// For COMPARE, MOVE, COPY, CREATE and REPLACE dialogues.
	//

	TRACE_FUNCTION() ;

	string lname ;
	string zcmd ;
	string msg ;

	vreplace( "ZCMD", "" ) ;

	const string vlist = fname + " ZCMD" ;

	vdefine( vlist, &lname, &zcmd ) ;

	vget( fname, PROFILE ) ;

	while ( true )
	{
		display( pname, msg ) ;
		if ( RC == 8 )
		{
			pcmd.set_msg( "PEDT014O", 4 ) ;
			vdelete( vlist ) ;
			RC = 8 ;
			return "" ;
		}
		msg   = "" ;
		lname = expandFileName( lname ) ;
		vput( fname, PROFILE ) ;
		try
		{
			if ( is_directory( lname ) )
			{
				select( "PGM(PFLST0A) PARM(EXPAND2 " + lname + ") NESTED SUSPEND" ) ;
				if ( ZRC == 0 )
				{
					lname = ZRESULT ;
					vput( fname, PROFILE ) ;
				}
				continue ;
			}
		}
		catch ( const filesystem_error& ex )
		{
			vreplace( "ZVAL1", ex.what() ) ;
			setmsg( "PSYS012C" ) ;
			continue ;
		}
		if ( !must_check )
		{
			break ;
		}
		try
		{
			if ( must_exist )
			{
				if ( is_regular_file( lname ) ) { break ; }
			}
			else
			{
				if ( !exists( lname ) ) { break ; }
			}
		}
		catch ( const filesystem_error& ex )
		{
			vreplace( "ZVAL1", ex.what() ) ;
			setmsg( "PSYS012C" ) ;
			continue ;
		}
		vreplace( "ZMVAL1", lname ) ;
		msg = msgid ;
	}

	vdelete( vlist ) ;

	return lname ;
}


void pedit01::create_file( const string& fname,
			   Data::Iterator it1,
			   Data::Iterator it2 )
{
	//
	// Create file with contents from it1 to it2.
	//

	TRACE_FUNCTION() ;

	create_file( fname, itr2ptr( it1 ), itr2ptr( it2 ) ) ;
}


void pedit01::create_file( const string& fname,
			   iline* addr1,
			   iline* addr2 )
{
	//
	// Create file 'fname' with contents from addr1 to addr2.
	//

	TRACE_FUNCTION() ;

	int j ;

	bool replace = exists( fname ) ;

	string t1 ;
	string spaces = string( profXTabz, ' ' ) ;

	string* pt ;

	std::ofstream fout( fname.c_str() ) ;
	if ( !fout.is_open() )
	{
		pcmd.set_msg( "PEDT014S", fname, 20 ) ;
		return ;
	}

	Data::Iterator its = addr1 ;
	Data::Iterator ite = addr2 ;

	for ( ++ite ; its != ite ; ++its )
	{
		if ( its->not_valid_file() ) { continue ; }
		pt = its->get_idata_ptr() ;
		if ( !optPreserve && reclen == 0 ) { its->set_idata_trim() ; }
		if ( profXTabs )
		{
			t1 = "" ;
			for ( j = 0 ; j < pt->size() ; ++j )
			{
				if ( j % profXTabz == 0           &&
				     pt->size() > (j+profXTabz-1) &&
				     pt->compare( j, profXTabz, spaces ) == 0 )
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
	if ( fout.fail() )
	{
		pcmd.set_msg( "PEDT011Q", 12 ) ;
	}
	else
	{
		pcmd.set_msg( "PEDT014T", ( replace ) ? "replaced" : "created", fname, 4, 0 ) ;
	}
}


void pedit01::copyIn( const string& fname,
		      Data::Iterator it,
		      int l1,
		      int l2 )
{
	//
	// Copy in file after iterator using range l1 - l2.
	// Used by MACRO functions only.
	//
	//   Return code:
	//    0 Normal completion
	//    8 End of data before last record found (l2).
	//   12 Invalid line pointer l1.
	//

	TRACE_FUNCTION() ;

	int i = 0 ;
	int j ;

	string inLine ;

	vector<string> vst ;

	if ( l2 < l1 )
	{
		swap( l1, l2 ) ;
	}

	std::ifstream fin( fname.c_str() ) ;
	if ( !fin.is_open() )
	{
		miBlock.seterror( "PEDT014S", fname, 20 ) ;
		return;
	}

	vst.clear() ;
	while ( getline( fin, inLine ) )
	{
		++i ;
		if ( l1 > -1 && i < l1 ) { continue ; }
		if ( l2 > -1 && i > l2 ) { break    ; }
		vst.push_back( convertiTabs( inLine ) ) ;
	}
	fin.close() ;

	if ( l1 > 0 && i < l1 )
	{
		miBlock.seterror( "PEDM011U", d2ds( l1, 8 ), 16 ) ;
		return;
	}

	if ( l2 > 0 && i < l2 )
	{
		miBlock.setRC( 8 ) ;
	}

	for ( j = 0 ; j < vst.size() ; ++j )
	{
		it = data.insert( ++it, new iline( taskid(), LN_FILE, lnumSize ), vst[ j ], level, ID_COPYRM ) ;
	}
}


string& pedit01::convertiTabs( string& s )
{
	//
	// Convert tabs to spaces in-place
	//

	TRACE_FUNCTION() ;

	int j ;

	size_t pos ;

	pos = s.find( '\t' ) ;
	while ( pos != string::npos )
	{
		j = profXTabz - ( pos % profXTabz ) ;
		s.replace( pos, 1,  j, ' ' )  ;
		pos = s.find( '\t', pos + 1 ) ;
	}

	return s ;
}


string pedit01::convertTabs( string s )
{
	//
	// Convert tabs to spaces.
	//

	TRACE_FUNCTION() ;

	int j ;

	size_t pos ;

	pos = s.find( '\t' ) ;
	while ( pos != string::npos )
	{
		j = profXTabz - ( pos % profXTabz ) ;
		s.replace( pos, 1,  j, ' ' )  ;
		pos = s.find( '\t', pos + 1 ) ;
	}

	return s ;
}


string pedit01::getMaskLine1()
{
	//
	// Get the mask line.
	//

	TRACE_FUNCTION() ;

	return ( profCaps ) ? upper( maskLine ) : maskLine ;
}


string pedit01::getMaskLine2()
{
	//
	// Get the mask line, replacing the number areas with spaces.
	//

	TRACE_FUNCTION() ;

	string t = maskLine ;

	size_t len ;

	if ( lnumSize1 > 0 )
	{
		len = min( t.size(), size_t( lnumSize1 ) ) ;
		t.replace( 0, len, len, ' ' ) ;
	}

	if ( reclen > 0 )
	{
		t.resize( reclen, ' ' ) ;
		if ( lnumSize2 > 0 )
		{
			t.replace( lnumS2pos, lnumSize2, lnumSize2, ' ' ) ;
		}
	}

	return ( profCaps ) ? upper( t ) : t ;
}


string pedit01::rshiftCols( int n,
			    const string* p_s )
{
	//
	// ) Right shift.
	//

	TRACE_FUNCTION() ;

	string s = *p_s ;

	if ( LeftBnd > s.size() ) { return s ; }

	s.insert( LeftBnd-1, n, ' ' ) ;

	if ( RightBnd == 0 || RightBnd > s.size() )
	{
		if ( reclen > 0 && reclen < s.size() )
		{
			s.erase( reclen ) ;
		}
		return s ;
	}

	s.erase( RightBnd, n ) ;

	return s ;
}


string pedit01::lshiftCols( int n,
			    const string* p_s )
{
	//
	// ( Left shift.
	//

	TRACE_FUNCTION() ;

	string s = *p_s ;


	if ( RightBnd > 0 && RightBnd < s.size() )
	{
		s.insert( RightBnd, n, ' ' ) ;
	}

	if ( LeftBnd > s.size() ) { return s ; }

	return s.erase( LeftBnd-1, n ) ;
}


bool pedit01::rshiftData( int n,
			  const string* s,
			  string& t )
{
	//
	// > Right shifting rules:
	// 1) scanning starts at left column.
	// 2) first blank char is found.
	// 3) the next non-blank char is found.
	// 4) the next double blank char is found.
	// 5) data from 3) to 4) is shifted 1 col to the right.
	// The above 5 steps are repeated until request is satisfied.
	//
	// Without   1) losing data.
	//           2) shift beyond bound.
	//           3) deleting single blanks.
	//           4) deleting blanks within apostrophes.
	//
	// else ==ERR>
	//

	TRACE_FUNCTION() ;

	size_t c1 ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	t = *s ;
	if ( LeftBnd > t.size() ) { return true ; }

	for ( int i = 0 ; i < n ; ++i )
	{
		if ( reclen > 0 )
		{
			t.resize( reclen, ' ' ) ;
		}
		if ( RightBnd > 0 && RightBnd <= t.size() &&  t[ RightBnd - 1 ] != ' ' ) { return false ; }
		c1 = ( RightBnd > 0 && RightBnd < t.size() ) ? RightBnd-1 : t.size()-1 ;
		p1 = findnq( t, ' ', LeftBnd-1 ) ;
		if ( p1 == string::npos || p1 > c1 ) { return false ; }
		p2 = t.find_first_not_of( ' ', p1 ) ;
		if ( p2 == string::npos || p2 > c1 ) { return true  ; }
		p3 = findnq( t, "  ", p2 ) ;
		if ( reclen > 0 && p3 == string::npos ) { return false ; }
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
		if ( RightBnd <= t.size() && t.at( RightBnd-1 ) == ' ' )
		{
			t.insert( p2, 1, ' ' ) ;
			t.erase( RightBnd-1, 1 ) ;
		}
		else
		{
			return false ;
		}
	}

	return true ;
}


bool pedit01::lshiftData( int n,
			  const string* s,
			  string& t )
{
	//
	// < Left shifting rules:
	// 1) scanning starts at left column.
	// 2) first blank char is found.
	// 3) the next non-blank char is found.
	// 4) the next double blank char is found.
	// 5) data from 3) to 4) is shifted 1 col to the left.
	// The above 5 steps are repeated until request is satisfied.
	//
	// Without   1) losing data.
	//           2) shift beyond bound.
	//           3) deleting single blanks.
	//           4) deleting blanks within apostrophes.
	//
	// else ==ERR>
	//

	TRACE_FUNCTION() ;

	size_t c1 ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	t = *s ;
	if ( LeftBnd > t.size() ) { return true ; }

	for ( int i = 0 ; i < n ; ++i )
	{
		if ( reclen > 0 )
		{
			t.resize( reclen, ' ' ) ;
		}
		if ( t.size() < 3 ) { return false ; }
		c1 = ( RightBnd > 0 && RightBnd < t.size() ) ? RightBnd-1 : t.size()-1 ;
		p1 = findnq( t, ' ', LeftBnd-1 ) ;
		if ( p1 == string::npos || p1 > c1 ) { return false ; }
		p2 = t.find_first_not_of( ' ', p1 ) ;
		if ( p2 == string::npos || p2 > c1 ) { return true  ; }
		if ( ( p2 - p1 ) < 2 ) { return false ; }
		p3 = findnq( t, "  ", p2 ) ;
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


bool pedit01::textSplitData( const string& s,
			     string& t1,
			     string& t2 )
{
	TRACE_FUNCTION() ;

	int k ;
	int p = aOff ;

	if ( ( RightBnd > 0 && p >= RightBnd ) || p < LeftBnd-1 || p >= s.size() )
	{
		t1 = s ;
		t2 = ( reclen > 0 ) ? string( reclen, ' ' ) : "" ;
		return false ;
	}

	k = s.find_first_not_of( ' ', LeftBnd - 1 ) ;
	k = ( k == string::npos ) ? LeftBnd - 1 : k ;

	if ( RightBnd > 0 )
	{
		t1 = s ;
		t1.replace( p, RightBnd-p, RightBnd-p, ' ' ) ;
		t2 = string( k, ' ' ) + strip( s.substr( p, RightBnd-p ), 'L' ) ;
	}
	else
	{
		t1 = s.substr( 0, p ) ;
		t2 = string( k, ' ' ) + strip( s.substr( p ), 'L' ) ;
	}

	if ( reclen > 0 )
	{
		t1.resize( reclen, ' ' ) ;
		t2.resize( reclen, ' ' ) ;
	}

	return true ;
}


void pedit01::setFoundMsg()
{
	TRACE_FUNCTION() ;

	if ( fcx_parms.f_all() )
	{
		type  = ( fcx_parms.f_pic ) ? "PIC" :
			( fcx_parms.f_reg ) ? "REGEX" : fcx_parms.f_mtch_str() ;
		str   = ( fcx_parms.f_pic || fcx_parms.f_reg ) ? "'" + fcx_parms.f_ostring + "'" : fcx_parms.setFoundString() ;
		occ   = d2ds( fcx_parms.f_occurs ) ;
		lines = d2ds( fcx_parms.f_lines  ) ;
		pcmd.set_msg( "PEDT013I", fcx_parms.getColumnsString(), 0 ) ;
	}
	else
	{
		str  = fcx_parms.setFoundString() ;
		type = fcx_parms.f_mtch_str() ;
		pcmd.set_msg( "PEDT013G", fcx_parms.getColumnsString(), fcx_parms.getRangeString(), 0 ) ;
	}
}


void pedit01::setNotFoundMsg()
{
	//
	// dir = N and not from top    - Bottom of Data/Range reached.
	// dir = P and not from bottom - Top of Data/Range reached.
	// dir = F/A/L/N-top/P-bottom  - Not found (in range).
	//

	TRACE_FUNCTION() ;

	str  = fcx_parms.setFoundString() ;
	type = ( fcx_parms.f_pic ) ? "PIC" :
	       ( fcx_parms.f_reg ) ? "REGEX" : fcx_parms.f_mtch_str() ;

	if ( !macroRunning )
	{
		if ( aDATA )
		{
			cursor.fix() ;
		}
		else
		{
			cursor.set( aADDR, CSR_FC_LCMD ) ;
		}
	}

	if ( !fcx_parms.f_searched && ( fcx_parms.f_top || fcx_parms.f_bottom ) )
	{
		pcmd.set_msg( "PEDT016B", fcx_parms.getXNXString(), fcx_parms.getColumnsString(), fcx_parms.getRangeString(), 4 ) ;
	}
	else if ( fcx_parms.f_next() && !fcx_parms.f_top )
	{
		if ( fcx_parms.f_slab != "" )
		{
			pcmd.set_msg( "PEDT016E", fcx_parms.f_slab, fcx_parms.f_elab, 4 ) ;
		}
		else
		{
			pcmd.set_msg( "PEDT013V", 4 ) ;
		}
	}
	else if ( fcx_parms.f_prev() && !fcx_parms.f_bottom )
	{
		if ( fcx_parms.f_slab != "" )
		{
			pcmd.set_msg( "PEDT016F", fcx_parms.f_slab, fcx_parms.f_elab, 4 ) ;
		}
		else
		{
			pcmd.set_msg( "PEDT013W", 4 ) ;
		}
	}
	else
	{
		pcmd.set_msg( "PEDT016G", fcx_parms.getXNXString(), fcx_parms.getColumnsString(), fcx_parms.getRangeString(), 4 ) ;
		fcx_parms.f_ptopLine = nullptr ;
	}
}


void pedit01::setExcludedMsg()
{
	TRACE_FUNCTION() ;

	str   = fcx_parms.setFoundString() ;
	type  = fcx_parms.f_mtch_str() ;
	occ   = d2ds( fcx_parms.f_ex_occs ) ;
	lines = d2ds( fcx_parms.f_ex_lnes ) ;

	pcmd.set_msg( "PEDT011L", 0 ) ;

}


void pedit01::setChangedMsg()
{
	TRACE_FUNCTION() ;

	if ( fcx_parms.f_chngall )
	{
		type = ( fcx_parms.f_pic ) ? "PIC" :
		       ( fcx_parms.f_reg ) ? "REGEX" : fcx_parms.f_mtch_str() ;
		str  = ( fcx_parms.f_pic || fcx_parms.f_reg ) ? "'" + fcx_parms.f_ostring + "'" : fcx_parms.setFoundString() ;
		occ  = d2ds( fcx_parms.f_ch_occs ) ;
		pcmd.set_msg( "PEDT013K", 0 ) ;
		fcx_parms.f_chngall = false ;
	}
	else
	{
		str  = fcx_parms.setFoundString() ;
		type = fcx_parms.f_mtch_str() ;
		pcmd.set_msg( "PEDT013L", 0 ) ;
	}
}


void pedit01::setChangedError()
{
	TRACE_FUNCTION() ;

	if ( !macroRunning )
	{
		cursor.fix() ;
	}

	str  = fcx_parms.setFoundString() ;
	type = fcx_parms.f_mtch_str() ;
	str2 = fcx_parms.f_cstring ;

	pcmd.set_msg( "PEDT018B", 8 ) ;
}


void pedit01::setCursorFind()
{
	//
	// Set the cursor after a successful FIND operation.
	//

	TRACE_FUNCTION() ;

	moveColumn() ;
	moveTopline( fcx_parms.f_ADDR ) ;

	if ( macroRunning )
	{
		mRow = fcx_parms.f_ADDR ;
		mCol = fcx_parms.f_offset + 1 ;
		miBlock.cursorset = true ;
	}
	else
	{
		cursor.set( fcx_parms.f_ADDR,
			    CSR_OFF_DATA,
			    fcx_parms.f_offset ) ;
	}
}


void pedit01::setCursorFindChange()
{
	//
	// Set the cursor after a successful FIND operation before a change.
	//

	TRACE_FUNCTION() ;

	moveColumn( fcx_parms.f_chnincr + 1 ) ;
	moveTopline( fcx_parms.f_ADDR ) ;

	if ( macroRunning )
	{
		mRow = fcx_parms.f_ADDR ;
		if ( fcx_parms.f_reverse() )
		{
			mCol = ( fcx_parms.f_offset == 0 ) ? 1 : fcx_parms.f_offset ;
		}
		else
		{
			mCol = fcx_parms.f_offset + 1 ;
		}
		miBlock.cursorset = true ;
	}
	else
	{
		if ( fcx_parms.f_reverse() )
		{
			cursor.set( fcx_parms.f_ADDR,
				    CSR_OFF_DATA,
				    ( fcx_parms.f_offset == 0 ) ? 0 : fcx_parms.f_offset - 1 ) ;
		}
		else
		{
			cursor.set( fcx_parms.f_ADDR,
				    CSR_OFF_DATA,
				    fcx_parms.f_offset ) ;
		}
	}
}


void pedit01::setCursorChange()
{
	//
	// Set the cursor after a successful CHANGE operation.
	//

	TRACE_FUNCTION() ;

	moveColumn( fcx_parms.f_chnincr + 1 ) ;

	if ( macroRunning )
	{
		mRow = fcx_parms.f_ADDR ;
		if ( fcx_parms.f_reverse() )
		{
			mCol = ( fcx_parms.f_offset == 0 ) ? 1 : fcx_parms.f_offset ;
		}
		else
		{
			mCol = fcx_parms.f_offset + fcx_parms.f_cstring.size() + 1 ;
		}
		miBlock.cursorset = true ;
	}
	else
	{
		if ( fcx_parms.f_reverse() )
		{
			cursor.set( fcx_parms.f_ADDR,
				    CSR_OFF_DATA,
				    ( fcx_parms.f_offset == 0 ) ? 0 : fcx_parms.f_offset - 1 ) ;
		}
		else
		{
			cursor.set( fcx_parms.f_ADDR,
				    CSR_OFF_DATA,
				    fcx_parms.f_offset + fcx_parms.f_cstring.size() ) ;
		}
	}
}


bool pedit01::checkLabel1( const string& lab,
			   int nestlvl )
{
	//
	// Return true if line label has a valid user format.
	// For level 0 (visible), max chars is 5.  Higher than level 0, max chars is 8.
	//

	TRACE_FUNCTION() ;

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

	if ( lab[ 1 ] == 'Z' || !isupper( lab[ 1 ] ) ) { return false ; }

	for ( i = 2 ; i < l ; ++i )
	{
		if ( !isupper( lab[ i ] ) ) { return false ; }
	}

	return true ;
}


bool pedit01::checkLabel2( const string& lab,
			   int nestlvl )
{
	//
	// Return true if line label has a valid format.
	// For level 0 (visible), max chars is 5.  Higher than level 0, max chars is 8.
	//

	TRACE_FUNCTION() ;

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

	for ( i = 2 ; i < l ; ++i )
	{
		if ( !isupper( lab[ i ] ) ) { return false ; }
	}

	return true ;
}


void pedit01::loadEditProfile( const string& prof )
{
	//
	// Get edit profile, and set ZEDPROF.
	// If it does not exist copy oldprof to create or call createDefaultEditProfile() for defaults if not specified.
	//
	// ZEDPFLAG :
	//      [0]:  Profile locked if '1'
	//      [1]:  Autosave On if '1'
	//      [2]:  Nulls on if '1'
	//      [3]:  Caps On if '1'
	//      [4]:  Hex On if '1'
	//      [5]:  File Tabs (if '1', convert from spaces -> tabs on save)
	//      [6]:  Recovery (if '1', create a recovery file)
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
	//      [23]: Number on
	//      [24]: Number on standard
	//      [25]: Number on COBOL
	//      [26]: Number on display
	//      [27]: Autonum
	//      [28]: Note
	//      [29]: Stats
	//      [30]: Backup (if '1', create a backup on first file change)
	//
	// If RECOV is OFF, set saveLevel = -1 to de-activate this function.
	//

	TRACE_FUNCTION() ;

	string tabName ;
	string flds    ;
	string keys    ;
	string vlist1  ;
	string vlist2  ;

	tabName = zapplid + "EDIT" ;

	flds = "(ZEDPTYPE,ZEDPFLAG,ZEDPMASK,ZEDPBNDL,ZEDPBNDR,ZEDPTABC,ZEDPTABS,ZEDPTABZ,ZEDPRCLC,"
	       "ZEDPHLLG,ZEDPIMAC,ZEDPFLG2,ZEDPFLG3)" ;

	vlist1 = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;
	vlist2 = "ZEDPTYPE ZEDPLRCL ZEDPRCFM ZEDPHLLG ZEDPIMAC ZEDPFLG2 ZEDPFLG3" ;

	vdefine( vlist1, &zedpflag, &zedpmask, &zedpbndl, &zedpbndr, &zedptabc, &zedptabs, &zedptabz, &zedprclc ) ;
	vdefine( vlist2, &zedptype, &zedplrcl, &zedprcfm, &zedphllg, &zedpimac, &zedpflg2, &zedpflg3 ) ;

	tbopen( tabName, NOWRITE, zuprof, SHARE ) ;
	if ( RC == 8 )
	{
		tbcreate( tabName,
			  "",
			  flds,
			  WRITE,
			  NOREPLACE,
			  zuprof ) ;
		if ( RC > 0 ) { uabend( "PEDT015W", "TBCREATE" ) ; }
	}

	tbquery( tabName,
		 "KEYS" ) ;

	vcopy( "KEYS", keys, MOVE ) ;
	if ( keys == "(ZEDPTYPE)" )
	{
		uabend( "PEDT017U", tabName, zuprof ) ;
	}

	tbvclear( tabName ) ;
	zedptype = prof ;
	zedprcfm = file_recfm() ;
	zedplrcl = file_lrecl() ;

	tbsarg( tabName ) ;
	tbscan( tabName ) ;
	if ( RC == 8 )
	{
		tbclose( tabName, "", zuprof ) ;
		tbopen( tabName, WRITE, zuprof ) ;
		if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }
		createEditProfile( tabName, prof, zedprcfm, zedplrcl ) ;
		pcmd.set_msg( "PEDT014D", prof, ( zedprcfm == "F" ) ? " (" + zedprcfm + " - " + zedplrcl + ")" : " ", 4 ) ;
	}

	tbclose( tabName, "", zuprof ) ;

	zedpflag.resize( 32, '0' ) ;

	profLock        = ( zedpflag[0]  == '1' ) ;
	profAutoSave    = ( zedpflag[1]  == '1' ) ;
	profNulls       = ( zedpflag[2]  == '1' ) ;
	profCaps        = ( zedpflag[3]  == '1' ) ;
	profHex         = ( zedpflag[4]  == '1' ) ;
	profXTabs       = ( zedpflag[5]  == '1' ) ;
	profRecovery    = ( zedpflag[6]  == '1' ) ;
	profUndoOn      = ( zedpflag[7]  == '1' ) ;
	profHilight     = ( zedpflag[8]  == '1' ) ;
	profTabs        = ( zedpflag[9]  == '1' ) ;
	profATabs       = ( zedpflag[10] == '1' ) ;
	profSaveP       = ( zedpflag[11] == '1' ) ;
	profNullA       = ( zedpflag[12] == '1' ) ;
	profIfLogic     = ( zedpflag[13] == '1' ) ;
	profDoLogic     = ( zedpflag[14] == '1' ) ;
	profParen       = ( zedpflag[15] == '1' ) ;
	profVert        = ( zedpflag[16] == '1' ) ;
	profCutReplace  = ( zedpflag[17] == '1' ) ;
	profPasteKeep   = ( zedpflag[18] == '1' ) ;
	profPosFcx      = ( zedpflag[19] == '1' ) ;
	profFindPhrase  = ( zedpflag[20] == '1' ) ;
	profCsrPhrase   = ( zedpflag[21] == '1' ) ;
	profUndoKeep    = ( zedpflag[22] == '1' ) ;
	profNum         = ( zedpflag[23] == '1' ) ;
	profNumSTD      = ( zedpflag[24] == '1' ) ;
	profNumCBL      = ( zedpflag[25] == '1' ) ;
	profNumDisp     = ( zedpflag[26] == '1' ) ;
	profAutoNum     = ( zedpflag[27] == '1' ) ;
	profNotes       = ( zedpflag[28] == '1' ) ;
	profStats       = ( zedpflag[29] == '1' ) ;
	profBackup      = ( zedpflag[30] == '1' ) ;
	maskLine        = zedpmask ;
	tabsLine        = zedptabs ;
	tabsChar        = zedptabc.front() ;
	LeftBnd         = ds2d( zedpbndl ) ;
	RightBnd        = ds2d( zedpbndr ) ;
	profXTabz       = ds2d( zedptabz ) ;
	backupLoc       = zedprclc ;
	profLang        = zedphllg ;
	profIMACRO      = zedpimac ;

	vdelete( vlist1, vlist2 ) ;

	detLang = ( profLang != "AUTO" ) ? profLang : determineLang() ;

	zedprof = prof ;
	vput( "ZEDPROF", PROFILE ) ;

	set_zhivars() ;

	if ( !profUndoOn ) { saveLevel = -1 ; }
}


void pedit01::saveEditProfile( const string& prof )
{
	//
	// Retrieve the edit profile, apply changes and save.  Call createDefaultEditProfile() if it does not exist.
	//
	// Delete old edit profiles if more than the EDMAXPRF value in lspf.h.
	//
	// ZEDPFLAG :
	//      [0]:  Profile locked
	//      [1]:  Autosave
	//      [2]:  Nulls
	//      [3]:  Caps
	//      [4]:  Hex
	//      [5]:  File Tabs
	//      [6]:  Recovery On
	//      [7]:  SETUNDO On
	//      [8]:  Hilight On
	//      [9]:  Tabs On
	//      [10]: Tabs On All (else STD)
	//      [11]: Autosave PROMPT
	//      [12]: Nulls are STD if off, ALL if on
	//      [13]: Hilight If logic on
	//      [14]: Hilight Do logic on
	//      [15]: Hilight Parenthesis on
	//      [16]: Hex Vert
	//      [17]: Cut Replace
	//      [18]: Paste Keep
	//      [19]: Position find
	//      [20]: Find phrase
	//      [21]: Cursor phrase
	//      [22]: Undo keep
	//      [23]: Number on
	//      [24]: Number on standard
	//      [25]: Number on COBOL
	//      [26]: Number on display
	//      [27]: Autonum
	//      [28]: Note
	//      [29]: Stats
	//      [30]: Backup
	//

	TRACE_FUNCTION() ;

	string tabName ;

	string vlist1 ;
	string vlist2 ;

	bool newprof = false ;

	tabName = zapplid + "EDIT" ;

	vlist1 = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;
	vlist2 = "ZEDPTYPE ZEDPLRCL ZEDPRCFM ZEDPHLLG ZEDPIMAC ZEDPFLG2 ZEDPFLG3" ;

	vdefine( vlist1, &zedpflag, &zedpmask, &zedpbndl, &zedpbndr, &zedptabc, &zedptabs, &zedptabz, &zedprclc ) ;
	vdefine( vlist2, &zedptype, &zedplrcl, &zedprcfm, &zedphllg, &zedpimac, &zedpflg2, &zedpflg3 ) ;

	tbopen( tabName, WRITE, zuprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbtop( tabName ) ;
	tbskip( tabName, EDMAXPRF+1 ) ;
	while ( RC == 0 )
	{
		if ( zedptype != "ZDEFAULT" )
		{
			tbdelete( tabName ) ;
		}
		tbskip( tabName ) ;
	}

	tbtop( tabName ) ;
	tbvclear( tabName ) ;

	zedptype = prof ;
	zedprcfm = file_recfm() ;
	zedplrcl = file_lrecl() ;

	tbsarg( tabName ) ;
	tbscan( tabName ) ;
	if ( RC == 0 )
	{
		tbdelete( tabName ) ;
	}
	else
	{
		newprof = true ;
	}

	zedpflag.resize( 32, '0' ) ;
	zedpflag[0] = ZeroOne[ profLock ] ;

	if ( zedpimac == "" ) { zedpimac = "NONE"             ; }
	if ( zedpflg2 == "" ) { zedpflg2 = "0000000000000000" ; }
	if ( zedpflg3 == "" ) { zedpflg3 = "0000000000000000" ; }

	if ( !profLock || newprof )
	{
		zedpflag[1]  = ZeroOne[ profAutoSave ] ;
		zedpflag[2]  = ZeroOne[ profNulls    ] ;
		zedpflag[3]  = ZeroOne[ profCaps     ] ;
		zedpflag[4]  = ZeroOne[ profHex      ] ;
		zedpflag[5]  = ZeroOne[ profXTabs    ] ;
		zedpflag[6]  = ZeroOne[ profRecovery ] ;
		zedpflag[7]  = ZeroOne[ profUndoOn   ] ;
		zedpflag[8]  = ZeroOne[ profHilight  ] ;
		zedpflag[9]  = ZeroOne[ profTabs     ] ;
		zedpflag[10] = ZeroOne[ profATabs    ] ;
		zedpflag[11] = ZeroOne[ profSaveP    ] ;
		zedpflag[12] = ZeroOne[ profNullA    ] ;
		zedpflag[13] = ZeroOne[ profIfLogic  ] ;
		zedpflag[14] = ZeroOne[ profDoLogic  ] ;
		zedpflag[15] = ZeroOne[ profParen    ] ;
		zedpflag[16] = ZeroOne[ profVert     ] ;
		zedpflag[17] = ZeroOne[ profCutReplace ] ;
		zedpflag[18] = ZeroOne[ profPasteKeep  ] ;
		zedpflag[19] = ZeroOne[ profPosFcx     ] ;
		zedpflag[20] = ZeroOne[ profFindPhrase ] ;
		zedpflag[21] = ZeroOne[ profCsrPhrase  ] ;
		zedpflag[22] = ZeroOne[ profUndoKeep   ] ;
		zedpflag[23] = ZeroOne[ profNum        ] ;
		zedpflag[24] = ZeroOne[ profNumSTD     ] ;
		zedpflag[25] = ZeroOne[ profNumCBL     ] ;
		zedpflag[26] = ZeroOne[ profNumDisp    ] ;
		zedpflag[27] = ZeroOne[ profAutoNum    ] ;
		zedpflag[28] = ZeroOne[ profNotes      ] ;
		zedpflag[29] = ZeroOne[ profStats      ] ;
		zedpflag[30] = ZeroOne[ profBackup     ] ;
		zedpmask     = maskLine ;
		zedptabs     = tabsLine ;
		zedptabc     = string( 1, tabsChar ) ;
		zedpbndl     = d2ds( LeftBnd )  ;
		zedpbndr     = d2ds( RightBnd ) ;
		zedptabz     = ( XTabz == 0 ) ? d2ds( profXTabz ) : d2ds( XTabz ) ;
		zedprclc     = backupLoc ;
		zedphllg     = profLang ;
		zedpimac     = profIMACRO ;
	}

	tbtop( tabName ) ;
	tbadd( tabName ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBADD" ) ; }

	tbclose( tabName, "", zuprof ) ;

	vdelete( vlist1, vlist2 ) ;
}


void pedit01::createEditProfile( const string& tabName,
				 const string& prof,
				 const string rcfm,
				 const string lrcl )
{
	//
	// Create an edit profile based on one with the same type.
	//
	// If recfm = U copy first recfm = F
	// If recfm = F copy first recfm = F or recfm = U if not found.
	// If neither found, create a default edit profile.
	//
	// Called when table is already open for update, and the table variables have been vdefined.
	//
	// Pass rcfm and lrcl by value as the function is called with VDEFINED variables.
	//

	TRACE_FUNCTION() ;

	tbvclear( tabName ) ;
	zedptype = prof ;
	zedprcfm = "F" ;

	tbsarg( tabName ) ;
	tbscan( tabName ) ;
	if ( RC == 8 )
	{
		if ( rcfm == "F" )
		{
			zedprcfm = "U" ;
			tbsarg( tabName ) ;
			tbscan( tabName ) ;
			if ( RC == 8 )
			{
				createDefaultEditProfile( tabName, prof, rcfm, lrcl ) ;
				return ;
			}
		}
		else
		{
			createDefaultEditProfile( tabName, prof, rcfm, lrcl ) ;
			return ;
		}
	}

	zedprcfm = rcfm ;
	zedplrcl = ( rcfm == "F" ) ? lrcl : "" ;
}


void pedit01::createDefaultEditProfile( const string& tabName,
					const string& prof,
					const string rcfm,
					const string lrcl )
{
	//
	// Create a default entry for edit profile 'prof' in table 'tabName'.
	// Use ZDEFAULT if it exists or system defaults.
	//
	// Called when table is already open for update, and the table variables have been vdefined.
	//
	// Defaults: AUTOSAVE OFF PROMPT, RECOVERY ON, UNDO ON, HILIGHT ON AUTO/LOGIC/PAREN, HEX VERT[16]
	//           Hilight cursor phrase[21], NUM OFF[23], NOTE ON[28]
	//
	// Pass rcfm and lrcl by value as the function is called with VDEFINED variables.
	//

	TRACE_FUNCTION() ;

	tbvclear( tabName ) ;
	zedptype = "ZDEFAULT" ;

	tbsarg( tabName ) ;
	tbscan( tabName ) ;
	if ( RC > 0 )
	{
		zedplrcl = "" ;
		zedprcfm = "U" ;
		zedpflag = "00000011100101111000010000001000" ;
		zedpmask = ""     ;
		zedpbndl = "1"    ;
		zedpbndr = "0"    ;
		zedptabc = " "    ;
		zedptabs = ""     ;
		zedptabz = "8"    ;
		zedprclc = zhome + DEFBACKLOC ;
		zedphllg = "AUTO" ;
		zedpimac = "NONE" ;
		zedpflg2 = "0000000000000000" ;
		zedpflg3 = "0000000000000000" ;
	}

	if ( prof != "ZDEFAULT" )
	{
		zedplrcl = lrcl ;
		zedprcfm = rcfm ;
	}
	zedptype = prof ;
}


void pedit01::swapEditProfile( const string& newprof,
			       vector<caution>& Cautions )
{
	//
	// Switch edit profiles.  Keep some values from the previous profile
	// and add caution messages.
	//
	// Also set the globals in case RECOVERY has been set ON from OFF or
	// vice versa (if set OFF also remove any EDRT entries).
	//

	TRACE_FUNCTION() ;

	size_t p ;

	int  oldXTabz  = profXTabz ;

	bool oldXTabs  = profXTabs ;
	bool oldCaps   = profCaps  ;
	bool oldNum    = profNum   ;
	bool oldNumSTD = profNumSTD ;
	bool oldNumCBL = profNumCBL ;

	caution caut( CA_PSWITCH ) ;
	caution caux( CA_PSWITCH, "          to match the previous profile." ) ;

	saveEditProfile( zedprof ) ;
	loadEditProfile( newprof ) ;

	set_language_fvars( detLang ) ;

	removeSpecial( CA_PSWITCH ) ;

	if ( oldNum && oldNumSTD && oldNumCBL && ( !profNum || !profNumSTD || !profNumCBL ) )
	{
		caut.message = "-CAUTION- Profile changed to NUMBER ON STD COBOL" ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}
	else if ( oldNum && oldNumSTD && ( !profNum || !profNumSTD ) )
	{
		caut.message = "-CAUTION- Profile changed to NUMBER ON STD" ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}
	else if ( oldNum && oldNumCBL && ( !profNum || !profNumCBL ) )
	{
		caut.message = "-CAUTION- Profile changed to NUMBER ON COBOL" ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}
	else if ( !oldNum && profNum )
	{
		caut.message = "-CAUTION- Profile changed to NUMBER OFF" ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}

	if ( !oldCaps && profCaps )
	{
		caut.message = "-CAUTION- Profile changed to CAPS OFF" ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}
	else if ( oldCaps && !profCaps )
	{
		caut.message = "-CAUTION- Profile changed to CAPS ON" ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}

	if ( !oldXTabs && profXTabs )
	{
		caut.message = "-CAUTION- Profile changed to XTABS OFF" ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}
	else if ( oldXTabs && !profXTabs )
	{
		caut.message = "-CAUTION- Profile changed to XTABS ON" ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}

	if ( oldXTabz != profXTabz )
	{
		caut.message = "-CAUTION- Profile changed to XTABS SIZE " + d2ds( oldXTabz ) ;
		Cautions.push_back( caut ) ;
		Cautions.push_back( caux ) ;
	}

	profXTabz  = oldXTabz ;
	profXTabs  = oldXTabs ;

	profCaps   = oldCaps ;

	profNum    = oldNum ;
	profNumSTD = oldNumSTD ;
	profNumCBL = oldNumCBL ;

	set_default_bounds() ;

	if ( profRecovery && !editRecovery )
	{
		p = zfile.find_last_of( '/' ) + 1 ;
		iline::set_Globals( taskid(),
				    zfile,
				    backupLoc + zfile.substr( p ) +
				    "-" + get_shared_var( "ZJ4DATE" ) +
				    "-" + get_shared_var( "ZTIMEL" ) ) ;
	}
}


bool pedit01::existsEditProfile( const string& prof,
				 const string& rcfm,
				 const string& lrcl )
{
	//
	// Check to see if an edit profile exists with the prof/rcfm/lrcl combination.
	//
	// VDEFINE/VDELETE the table fields so the TBVCLEAR does not interfere with
	// the real variables.
	//

	TRACE_FUNCTION() ;

	string tabName ;
	string vlist1  ;
	string vlist2  ;

	string xedptype ;
	string xedplrcl ;
	string xedprcfm ;
	string xedpflag ;
	string xedpmask ;
	string xedpbndl ;
	string xedpbndr ;
	string xedptabc ;
	string xedptabs ;
	string xedptabz ;
	string xedprclc ;
	string xedphllg ;
	string xedpimac ;
	string xedpflg2 ;
	string xedpflg3 ;

	bool result ;

	tabName = zapplid + "EDIT" ;

	vlist1 = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;
	vlist2 = "ZEDPTYPE ZEDPLRCL ZEDPRCFM ZEDPHLLG ZEDPIMAC ZEDPFLG2 ZEDPFLG3" ;

	vdefine( vlist1, &xedpflag, &xedpmask, &xedpbndl, &xedpbndr, &xedptabc, &xedptabs, &xedptabz, &xedprclc ) ;
	vdefine( vlist2, &xedptype, &xedplrcl, &xedprcfm, &xedphllg, &xedpimac, &xedpflg2, &xedpflg3 ) ;

	tbopen( tabName, NOWRITE, zuprof, SHARE ) ;
	if ( RC == 8 )
	{
		vdelete( vlist1, vlist2 ) ;
		return false ;
	}

	tbvclear( tabName ) ;
	xedptype = prof ;
	xedprcfm = rcfm ;
	xedplrcl = lrcl ;

	tbsarg( tabName ) ;
	tbscan( tabName, "", "", "", "", "NOREAD" ) ;
	result = ( RC == 0 ) ;

	tbend( tabName ) ;

	vdelete( vlist1, vlist2 ) ;

	return result ;
}


void pedit01::resetEditProfile()
{
	//
	// Delete ZDEFAULT edit profile.
	//
	// VDEFINE/VDELETE the table fields so the TBVCLEAR does not interfere with
	// the real variables.
	//

	TRACE_FUNCTION() ;

	bool deleted = false ;

	string tabName ;
	string vlist1 ;
	string vlist2 ;

	string xedptype ;
	string xedplrcl ;
	string xedprcfm ;
	string xedpflag ;
	string xedpmask ;
	string xedpbndl ;
	string xedpbndr ;
	string xedptabc ;
	string xedptabs ;
	string xedptabz ;
	string xedprclc ;
	string xedphllg ;
	string xedpimac ;
	string xedpflg2 ;
	string xedpflg3 ;

	vlist1 = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;
	vlist2 = "ZEDPTYPE ZEDPLRCL ZEDPRCFM ZEDPHLLG ZEDPIMAC ZEDPFLG2 ZEDPFLG3" ;

	vdefine( vlist1, &xedpflag, &xedpmask, &xedpbndl, &xedpbndr, &xedptabc, &xedptabs, &xedptabz, &xedprclc ) ;
	vdefine( vlist2, &xedptype, &xedplrcl, &xedprcfm, &xedphllg, &xedpimac, &xedpflg2, &xedpflg3 ) ;

	tabName = zapplid + "EDIT" ;

	tbopen( tabName, WRITE, zuprof ) ;
	if ( RC > 0 ) { uabend( "PEDT015W", "TBOPEN" ) ; }

	tbvclear( tabName ) ;

	xedptype = "ZDEFAULT" ;

	tbsarg( tabName ) ;
	tbscan( tabName, "", "", "", "", "NOREAD" ) ;
	while ( RC == 0 )
	{
		tbdelete( tabName ) ;
		if ( RC > 0 ) { uabend( "PEDT015W", "TBDELETE" ) ; }
		tbscan( tabName, "", "", "", "", "NOREAD" ) ;
		deleted = true ;
	}

	tbclose( tabName, "", zuprof ) ;

	if ( deleted )
	{
		pcmd.set_msg( "PEDT014K", 4 ) ;
	}

	vdelete( vlist1, vlist2 ) ;
}


void pedit01::readLineCommandTable()
{
	TRACE_FUNCTION() ;

	string keys    ;
	string names   ;

	string blkcmd  ;

	string zelcnam ;
	string zelcmac ;
	string zelcpgm ;
	string zelcblk ;
	string zelcmlt ;
	string zelcdst ;

	lmac t ;

	const string vlist1 = "ZELCNAM ZELCMAC ZELCPGM ZELCBLK ZELCMLT ZELCDST" ;

	tbopen( zedlctab,
		NOWRITE,
		"",
		SHARE ) ;
	if ( RC > 0 )
	{
		pcmd.set_msg( "PEDT017B", 12 ) ;
		return ;
	}

	tbquery( zedlctab,
		 "KEYS",
		 "NAMES" ) ;

	vcopy( "KEYS", keys, MOVE ) ;
	vcopy( "NAMES", names, MOVE ) ;

	if ( keys  != "(ZELCNAM)" ||
	     names != "(ZELCMAC ZELCPGM ZELCBLK ZELCMLT ZELCDST)" )
	{
		pcmd.set_msg( "PEDT017C", 12 ) ;
		tbend( zedlctab ) ;
		return ;
	}

	vdefine( vlist1, &zelcnam, &zelcmac, &zelcpgm, &zelcblk, &zelcmlt, &zelcdst ) ;

	tbtop( zedlctab ) ;
	tbskip( zedlctab ) ;
	while ( RC == 0 )
	{
		if ( datatype( zelcnam, 'U' ) && lineCmds.count( zelcnam ) == 0 )
		{
			lineCmds[ zelcnam ] = LC_LMAC ;
			t.lcname = zelcnam ;
			t.lcmac  = zelcmac ;
			t.lcpgm  = ( zelcpgm == "Y" ) ;
			t.lcdst  = ( zelcdst == "Y" ) ;
			lmacs[ zelcnam ] = t ;
			blkCmds.insert( blkcmd ) ;
			if ( zelcmlt == "Y" )
			{
				reptOk.insert( zelcnam )  ;
			}
			if ( zelcdst == "Y" )
			{
				abokReq.insert( zelcnam ) ;
			}
			if ( zelcblk == "Y" && zelcnam.size() < 6 )
			{
				blkcmd = zelcnam + zelcnam.back() ;
				lineCmds[ blkcmd ] = LC_LMAC ;
				blkCmds.insert( blkcmd ) ;
				if ( zelcdst == "Y" )
				{
					abokReq.insert( blkcmd ) ;
				}
				lmacs[ blkcmd ] = t ;
			}
		}
		tbskip( zedlctab ) ;
	}
	tbend( zedlctab ) ;

	vdelete( vlist1 ) ;
}


string pedit01::getColumnLine( int s )
{
	//
	// Create column line.  If start position, s, not specified, use startCol and data width, else
	// use a width of right screen position + 256.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int k ;
	int l ;

	string t = "" ;

	const string ruler = "----+----" ;

	if ( s == 0 )
	{
		s = startCol ;
		l = zdataw   ;
	}
	else
	{
		l = startCol + zdataw + 256 ;
	}

	i = s % 10 ;
	j = s / 10 ;

	if ( i > 0 )
	{
		++j ;
		t = substr( ruler, i, 10-i ) ;
	}

	for ( k = 0 ; k <= l/10 ; ++k, ++j )
	{
		t += d2ds( j%10 ) + ruler ;
	}

	if ( reclen > 0 && ( reclen - startCol ) < zdataw )
	{
		l = reclen - startCol + 1 ;
		t.resize( l, ' ' ) ;
		t.resize( zdataw, ' ' ) ;
	}
	else
	{
		t.resize( l, ' ' ) ;
	}

	return t ;
}


string pedit01::formLineData( Data::Iterator it )
{
	//
	// Create the data for macro command MASKLINE, TABSLINE, LINE (_BEFORE/_AFTER) = data.
	//
	// If just a template is passed, apply it to a blank line.
	//
	// data can be:
	//   o) Simple string
	//   o) Delimited string
	//   o) Variable
	//   o) Template (<col,sring>)
	//   o) Merge format (str1 + str2, operand + str2, str1 + operand)
	//   o) Operand:
	//        LINE
	//          Data from this line used.
	//        LINE linenum/label
	//          Data from this lptr used.
	//        MASKLINE
	//          Data from the mask line used.
	//        TABSLINE
	//          Data from the tabs line used.
	//

	TRACE_FUNCTION() ;

	size_t p1 ;

	p1 = miBlock.value.find_first_not_of( ' ' ) ;
	if ( p1 != string::npos && miBlock.value[ p1 ] == '<' )
	{
		miBlock.value = "'' + " + miBlock.value ;
	}

	return ( profCaps ) ? upper( mergeLine( miBlock.value, it ) ) :
			      mergeLine( miBlock.value, it ) ;
}


string pedit01::mergeLine( const string& s,
			   Data::Iterator it1 )
{
	//
	// Expand the last parameter passed and call ourselves again for the rest until
	// no more parameters, then merge or overlay the results on return.
	//
	// Parameters are connected with a '+'.
	//

	TRACE_FUNCTION() ;

	char qt ;

	bool quote ;
	bool templt = false ;

	string t1 ;
	string t2 ;

	string r1 ;

	string vw1 ;
	string vw2 ;
	string vw3 ;

	Data::Iterator it2 = nullptr ;

	string::const_iterator its ;
	string::const_iterator itt ;

	if ( miBlock.fatal ) { return "" ; }

	for ( quote = false, its = s.begin(), itt = s.end() ; its != s.end() ; ++its )
	{
		if ( !quote && ( *its == '"' || *its == '\'' ) ) { qt = *its ; quote = true ; continue ; }
		if (  quote &&   *its == qt )  { quote = false ; continue ; }
		if ( !quote &&   *its == '+' ) { itt = its                ; }
	}

	if ( itt != s.end() )
	{
		t1.assign( s.begin(), itt ) ;
		t2.assign( ++itt, s.end() ) ;
		trim( t1 ) ;
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
			if ( getLinePtrIterator( vw2, it2 ) != 1 )
			{
				return "" ;
			}
			r1 = it2->get_idata() ;
		}
		else
		{
			if ( it1 == nullptr )
			{
				miBlock.seterror( "PEDM013A", "LINE", 20 ) ;
				return "" ;
			}
			r1 = it1->get_idata() ;
		}
	}
	else if ( t2.size() > 1 && ( t2.front() == '"' || t2.front() == '\'' ) )
	{
		if ( t2.front() != t2.back() )
		{
			miBlock.seterror( "PEDM012W", 20 ) ;
			return "" ;
		}
		r1 = t2.substr( 1, t2.size() - 2 ) ;
	}
	else if ( t2.size() > 1 && t2.front() == '<' )
	{
		if ( t2.back() != '>' )
		{
			miBlock.seterror( "PEDM012Y", 20 ) ;
			return "" ;
		}
		templt = true ;
		r1     = t2 ;
	}
	else if ( vw1 == "MASKLINE" )
	{
		if ( vw2 != "" )
		{
			miBlock.seterror( "PEDM011P", 20 ) ;
			return "" ;
		}
		r1 = maskLine ;
	}
	else if ( vw1 == "TABSLINE" )
	{
		if ( vw2 != "" )
		{
			miBlock.seterror( "PEDM011P", 20 ) ;
			return "" ;
		}
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

	return ( t1 == "" ) ? r1 :
	       ( templt   ) ? templat( r1, mergeLine( t1, it1 ) ) : overlay2( r1, mergeLine( t1, it1 ) ) ;
}


void pedit01::check_number_parm( const string& valid,
				 set<string>& klist,
				 const string& k )
{
	TRACE_FUNCTION() ;

	pair<set<string>::iterator,bool> ret ;

	string t ;

	if ( k != "" )
	{
		t   = get_truename( k ) ;
		ret = klist.insert( t ) ;
		if ( !ret.second )
		{
			pcmd.set_msg( "PEDT016S", 12 ) ;
		}
		else if ( !findword( t, valid ) )
		{
			pcmd.set_msg( "PEDT011", 12 ) ;
		}
	}
}


void pedit01::set_num_parameters()
{
	//
	// Set the left side number length and
	// the right side number length and position
	//

	TRACE_FUNCTION() ;

	lnumSize1 = 0 ;
	lnumSize2 = 0 ;
	lnumS2pos = 0 ;

	if ( profNumCBL )
	{
		lnumSize1 = 6 ;
	}

	if ( profNumSTD )
	{
		if ( reclen > 0 )
		{
			lnumSize2 = 8 ;
			lnumS2pos = reclen - 8 ;
		}
		else
		{
			lnumSize1 = 8 ;
		}
	}

	lnumSize = lnumSize1 + lnumSize2 ;
}


void pedit01::set_default_bounds()
{
	//
	// Set default bounds settings when the NUM parameters change.
	//

	TRACE_FUNCTION() ;

	if ( !profNum && !LeftBndSet )
	{
		LeftBnd = 1 ;
	}

	if ( LeftBnd <= lnumSize1 )
	{
		LeftBnd = lnumSize1 + 1 ;
	}

	if ( reclen > 0 )
	{
		RightBnd = reclen ;
	}

	if ( lnumS2pos > 0 && ( RightBnd == 0 || RightBnd <= LeftBnd || RightBnd > lnumS2pos ) )
	{
		RightBnd = lnumS2pos ;
	}

	if ( reclen > 0 && ( RightBnd > reclen || RightBnd == 0 ) )
	{
		RightBnd = reclen ;
	}
}


void pedit01::unnum_data()
{
	TRACE_FUNCTION() ;

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->is_valid_file() )
		{
			it->remove_linenum( lnumSize1, lnumS2pos, level ) ;
			fileChanged = true ;
		}
	}
}


void pedit01::renum_data()
{
	TRACE_FUNCTION() ;

	int i    = ( data.size() > 10000 ) ? 10 : 100 ;
	int diff = i ;

	set_num_parameters() ;

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->is_valid_file() )
		{
			if ( it->upd_linenum( i, lnumSize1, lnumSize2, lnumS2pos, lnummod, level ) )
			{
				fileChanged = true ;
			}
			i += diff ;
		}
	}

	rebuildZAREA = true ;
}


void pedit01::add_line_numbers()
{
	//
	// Add line numbers to lines that have been inserted.
	// Renumber lines after these if necessary.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int diff ;
	int num  ;
	int lnum ;
	int renum  = 0 ;
	int p_lnum = 0 ;
	int n_lnum = 999999 ;

	Data::Iterator it1 = nullptr ;
	Data::Iterator it2 = nullptr ;

	for ( it1 = data.begin() ; it1 != data.end() ; ++it1 )
	{
		if ( it1->not_valid_file() )
		{
			continue ;
		}
		if ( !it1->il_lnumreq )
		{
			p_lnum = it1->get_linenum( lnumSize1, lnumSize2, lnumS2pos ) ;
			continue ;
		}
		it2 = it1 ;
		for ( num = 1, ++it2 ; it2 != data.end() ; ++it2 )
		{
			if ( it2->is_valid_file() )
			{
				if ( !it2->il_lnumreq )
				{
					n_lnum = it2->get_linenum( lnumSize1, lnumSize2, lnumS2pos ) ;
					break ;
				}
				++num ;
			}
		}
		diff = ( n_lnum - p_lnum ) / ( num + 1 ) ;
		if ( diff >= 100 )
		{
			diff = 100 ;
		}
		else if ( diff >= 10 )
		{
			diff = 10 ;
		}
		else if ( diff >= 1 )
		{
			diff = 1 ;
		}
		else
		{
			diff = 1 ;
			for ( i = p_lnum + num, j = 1 ; it2 != data.end() ; ++it2 )
			{
				if ( it2->is_valid_file() )
				{
					if ( it2->get_linenum( lnumSize1, lnumSize2, lnumS2pos ) > ( i + j ) )
					{
						break ;
					}
					it2->add_linenum( ( i + j ), lnumSize1, lnumSize2, lnumS2pos, lnummod, level ) ;
					++j ;
					++renum ;
				}
			}
		}
		lnum = p_lnum + diff ;
		for ( it2 = it1 ; it2 != data.end() && num > 0 ; ++it2 )
		{
			if ( it2->is_valid_file() )
			{
				it2->add_linenum( lnum, lnumSize1, lnumSize2, lnumS2pos, lnummod, level ) ;
				lnum += diff ;
				--num ;
			}
		}
	}

	if ( renum > 20 )
	{
		pcmd.set_msg( "PEDT017R", 4 ) ;
	}

	rebuildZAREA = true ;
}


void pedit01::add_missing_line_numbers()
{
	//
	// Add missing COBOL or STANDARD line numbers (NUM ON).
	// If STANDARD line numbers added, also reset mod level to 01.
	//

	TRACE_FUNCTION() ;

	bool added = false ;

	size_t pseq1 = 0 ;
	size_t pseq2 = 0 ;

	for ( auto it = data.begin() ; it != data.end() ; ++it )
	{
		if ( it->is_valid_file() )
		{
			if ( it->add_linenum( lnumSize1, lnumSize2, lnumS2pos, pseq1, pseq2, level ) )
			{
				added = true ;
			}
		}
	}

	if ( added )
	{
		fileChanged = true ;
		if ( lnumSize1 == 8 || lnumSize2 == 8 )
		{
			for ( auto it = data.begin() ; it != data.end() ; ++it )
			{
				if ( it->is_valid_file() )
				{
					it->upd_modlevel( lnumS2pos ) ;
				}
			}
			lnummod = "02" ;
		}
	}
}


string pedit01::createTempName()
{
	TRACE_FUNCTION() ;

	path temp = temp_directory_path() / unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;

	return temp.native() ;
}


void pedit01::qstrings( const string& s,
			vector<string>& v,
			int maxw )
{
	//
	// Split a string up into words delimited by spaces, single or double quotes
	// and place into string vector v.
	//
	// If more than maxw words, place rest in the last vector entry.
	//
	// Usage:  MACRO statement parameters.
	//

	TRACE_FUNCTION() ;

	size_t i ;
	size_t j = 0 ;

	char quote ;

	v.clear() ;

	while ( true )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return ; }
		if ( s[ i ] == '\'' || s[ i ] == '"' )
		{
			quote = s[ i ] ;
			++i ;
			j = s.find( quote, i ) ;
			if ( j == string::npos )
			{
				miBlock.seterror( "PSYE033F", 20 ) ;
				return ;
			}
			++j ;
			if ( j < s.size() && s[ j ] != ' ' )
			{
				miBlock.seterror( "PSYE033G", 20 ) ;
				return ;
			}
			if ( v.size() == maxw )
			{
				v.back() = v.back() + " " + s.substr( i-1, j-i+1 ) ;
			}
			else
			{
				v.push_back( s.substr( i-1, j-i+1 ) ) ;
			}
		}
		else
		{
			j = s.find( ' ', i ) ;
			if ( j == string::npos )
			{
				if ( v.size() == maxw )
				{
					v.back() = v.back() + " " + s.substr( i ) ;
				}
				else
				{
					v.push_back( s.substr( i ) ) ;
				}
				return ;
			}
			if ( v.size() == maxw )
			{
				v.back() = v.back() + " " + s.substr( i, j-i ) ;
			}
			else
			{
				v.push_back( s.substr( i, j-i ) ) ;
			}
		}
	}
}


bool pedit01::is_defName( const string& name )
{
	TRACE_FUNCTION() ;

	return ( defNames.count( name ) > 0 ) ;
}


void pedit01::set_dialogue_var( const string& var,
				const string& val )
{
	//
	// Set the dialogue variable in the Editor function pool and the SHARED pool.
	//

	TRACE_FUNCTION() ;

	vreplace( var, val ) ;
	vput( var, SHARED ) ;
}


void pedit01::get_shared_var( const string& var,
			      string& val )
{
	//
	// Get the dialogue variable from the SHARED pool.
	//

	TRACE_FUNCTION() ;

	vget( var, SHARED ) ;
	vcopy( var, val, MOVE ) ;
}


string pedit01::get_shared_var( const string& var )
{
	//
	// Return the dialogue variable from the SHARED pool.
	//

	TRACE_FUNCTION() ;

	string val ;

	vget( var, SHARED ) ;
	vcopy( var, val, MOVE ) ;

	return val ;
}


void pedit01::set_shared_var( const string& var,
			      const string& val )
{
	//
	// Set the dialogue variable in the SHARED pool.
	//

	TRACE_FUNCTION() ;

	string t = val ;

	vdefine( var, &t ) ;
	vput( var, SHARED ) ;
	vdelete( var ) ;
}


void pedit01::get_profile_var( const string& var,
			       string& val )
{
	//
	// Get the dialogue variable from the PROFILE pool.
	//

	TRACE_FUNCTION() ;

	vget( var, PROFILE ) ;
	vcopy( var, val, MOVE ) ;
}


string pedit01::get_profile_var( const string& var )
{
	//
	// Return the dialogue variable from the PROFILE pool.
	//

	TRACE_FUNCTION() ;

	string val ;

	vget( var, PROFILE ) ;
	vcopy( var, val, MOVE ) ;

	return val ;
}


void pedit01::set_profile_var( const string& var,
			       string& val )
{
	//
	// Set the dialogue variable from the PROFILE pool.
	//

	TRACE_FUNCTION() ;

	vreplace( var, val ) ;
	vput( var, PROFILE ) ;
}


void pedit01::set_macro_var( const string& var,
			     const string& val )
{
	//
	// Set the dialogue variable in the macro application function pool only.
	//

	TRACE_FUNCTION() ;

	if ( macroRunning )
	{
		miBlock.macAppl->vreplace( var, val ) ;
	}
}


string pedit01::expandFileName( const string& s )
{
	//
	// Return the expanded file name:
	//  ~  as first character represents the home directory.
	//  +  as first character represents the directory of the file name being edited.
	//  .  as first character represents the current working directory.
	//  .. as first two characters represents the parent directory of the current working directory.
	//  These must be followed by a '/' or the file name is returned unchanged.
	//
	//  Names not starting with / will be treated as having +/ prepended.
	//

	TRACE_FUNCTION() ;

	string ent ;

	if ( s.size() < 3 )
	{
		return s ;
	}

	if ( s.front() == '~' )
	{
		if ( s[ 1 ] != '/' ) { return s ; }
		ent = zhome + s.substr( 1 ) ;
	}
	else if ( s.front() == '+' )
	{
		if ( s[ 1 ] != '/' ) { return s ; }
		ent = zfile.substr( 0, zfile.find_last_of( '/' ) ) + s.substr( 1 ) ;
	}
	else if ( s.compare( 0, 2, ".." ) == 0 )
	{
		if ( s.size() < 4 || s[ 2 ] != '/' ) { return s ; }
		boost::filesystem::path p = boost::filesystem::current_path() ;
		ent = p.native() ;
		ent = ent.substr( 0, ent.find_last_of( '/' ) ) + s.substr( 2 ) ;
	}
	else if ( s.front() == '.' )
	{
		if ( s[ 1 ] != '/' ) { return s ; }
		boost::filesystem::path p = boost::filesystem::current_path() ;
		ent = p.native() + s.substr( 1 ) ;
	}
	else if ( s.front() != '/' )
	{
		ent = zfile.substr( 0, zfile.find_last_of( '/' ) + 1 ) + s ;
	}
	else
	{
		ent = s ;
	}

	istrip( ent ) ;

	return ent ;
}


void pedit01::check_delete( Del& d )
{
	//
	// Check parameters entered on the DELETE primary command/macro statement
	// and set the start/end iterators.
	//

	TRACE_FUNCTION() ;

	int rc ;

	const string zlabels = ".ZCSR .ZFIRST .ZLAST .ZDEST .ZFRANGE .ZLRANGE" ;
	const string mlabels = ".ZDEST .ZFRANGE .ZLRANGE" ;

	if ( macroRunning )
	{
		if ( d.del_ALL && ( !d.del_X && !d.del_NX && d.del_labela == "" && d.del_linenum1 == 0 ) )
		{
			d.seterror( "PEDM012G" ) ;
			return ;
		}
		if ( ( d.del_X || d.del_NX ) && ( !d.del_ALL && d.del_labela == "" && d.del_linenum1 == 0 ) )
		{
			d.seterror( "PEDM012G" ) ;
			return ;
		}
	}
	else
	{
		if ( findword( d.del_labela, mlabels ) || findword( d.del_labelb, mlabels ) )
		{
			d.seterror( "PEDT014" ) ;
			return ;
		}
		if ( d.del_ws < 2 )
		{
			d.seterror( "PEDT015E" ) ;
			return ;
		}
		if ( d.del_linenum1 > 0 )
		{
			d.seterror( "PEDT013J", "DELETE" ) ;
			return ;
		}
		if ( d.del_labela != "" && d.del_labelb == "" )
		{
			d.seterror( "PEDT014U" ) ;
			return ;
		}
		if ( !d.del_ALL && !d.del_X && !d.del_NX )
		{
			d.seterror( "PEDT013J", "DELETE" ) ;
			return ;
		}
		if ( d.del_ALL && ( !d.del_X && !d.del_NX && d.del_labela == "" ) )
		{
			d.seterror( "PEDT013J", "DELETE" ) ;
			return ;
		}
		if ( ( d.del_X || d.del_NX ) && ( !d.del_ALL && d.del_labela == "" ) )
		{
			d.seterror( "PEDT013J", "DELETE" ) ;
			return ;
		}
	}

	if ( d.del_labela != "" )
	{
		if ( findword( d.del_labela, zlabels ) )
		{
			d.del_its = getFileLineZLABEL( d.del_labela ) ;
			if ( d.del_its == nullptr )
			{
				if ( d.del_labela == ".ZCSR" )
				{
					d.del_validCSR = false ;
				}
				else
				{
					d.seterror( "PEDM011Q", d.del_labela ) ;
				}
				return ;
			}
		}
		else if ( checkLabel2( d.del_labela ) )
		{
			d.del_its = getLabelIterator( d.del_labela, rc ) ;
			if ( rc != 0 )
			{
				d.seterror( "PEDT011F" ) ;
				return ;
			}
		}
		else
		{
			d.seterror( "PEDT014" ) ;
			return ;
		}
		if ( d.del_linenum1 > 0 )
		{
			d.del_ite = getDataLine( d.del_linenum1 ) ;
			if ( d.del_ite == data.bottom() )
			{
				d.seterror( "PEDT015G", 12 ) ;
				return ;
			}
			if ( data.gt( d.del_its, d.del_ite ) )
			{
				swap( d.del_its, d.del_ite ) ;
			}
		}
		else if ( d.del_labelb != "" )
		{
			if ( findword( d.del_labelb, zlabels ) )
			{
				d.del_ite = getFileLineZLABEL( d.del_labelb ) ;
				if ( d.del_ite == nullptr )
				{
					if ( d.del_labelb == ".ZCSR" )
					{
						d.del_validCSR = false ;
					}
					else
					{
						d.seterror( "PEDM011Q", d.del_labelb ) ;
					}
					return ;
				}
			}
			else if ( checkLabel2( d.del_labelb ) )
			{
				d.del_ite = getLabelIterator( d.del_labelb, rc ) ;
				if ( rc != 0 )
				{
					d.seterror( "PEDT011F" ) ;
					return ;
				}
			}
			else
			{
				d.seterror( "PEDT014" ) ;
				return ;
			}
			if ( data.gt( d.del_its, d.del_ite ) )
			{
				swap( d.del_its, d.del_ite ) ;
			}
		}
		else
		{
			d.del_ite = d.del_its ;
		}
	}
	else if ( d.del_linenum1 > 0 )
	{
		d.del_its = getDataLine( d.del_linenum1 ) ;
		if ( d.del_its == data.bottom() )
		{
			d.seterror( "PEDT015G", 12 ) ;
			return ;
		}
		if ( d.del_linenum2 > 0 )
		{
			d.del_ite = getDataLine( d.del_linenum2 ) ;
			if ( d.del_ite == data.bottom() )
			{
				d.seterror( "PEDT015G", 12 ) ;
				return ;
			}
		}
		else
		{
			d.del_ite = d.del_its ;
		}
	}
	else
	{
		d.del_its = getFileLineZFIRST() ;
		d.del_ite = getFileLineZLAST() ;
	}

	++d.del_ite ;
}


string pedit01::determineLang()
{
	//
	// Try to determine the language, first from the extension then the contents of the file.
	// Limit the scan to the first 100 lines of code.
	//
	// This routine also determines the profile name on a PROFILE USE TYPE command.
	//
	// Returned language must exist in ehilight (hiRoutine function map) or an exception will occur.
	//

	TRACE_FUNCTION() ;

	int i ;

	size_t p ;

	string s ;
	string w ;

	string* t ;

	bool diffu1 = false ;
	bool diffc1 = false ;

	const string cpp   = "CPP"   ;
	const string rexx  = "REXX"  ;
	const string cobol = "COBOL" ;
	const string assem = "ASM"   ;
	const string jcl   = "JCL"   ;
	const string bash  = "BASH"  ;
	const string rust  = "RUST"  ;
	const string other = "OTHER" ;
	const string panel = "PANEL" ;
	const string skel  = "SKEL"  ;
	const string toml  = "TOML"  ;
	const string diffu = "DIFFU" ;
	const string diffc = "DIFFC" ;
	const string def   = "DEFAULT" ;

	Data::Iterator it = nullptr ;

	p = zfile.find_last_of( '.' ) ;
	if ( p != string::npos )
	{
		s = zfile.substr( p+1 ) ;
		if ( findword( s, "c cpp h hpp" ) ) { return cpp   ; }
		if ( findword( s, "rex rexx rx" ) ) { return rexx  ; }
		if ( findword( s, "cob cobol"   ) ) { return cobol ; }
		if ( findword( s, "asm assem"   ) ) { return assem ; }
		if ( findword( s, "rs"          ) ) { return rust  ; }
		if ( findword( s, "toml ini desktop kdelnk" ) ) { return toml ; }
	}

	for ( i = 0, it = data.begin() ; it != data.end() && i < 100 ; ++it )
	{
		if ( it->not_valid_file() ) { continue ; }
		t = it->get_idata_ptr() ;
		if ( t->size() == 0 ) { continue ; }
		w = word( *t, 1 ) ;
		if ( findword( w, "#!/bin/sh #!/bin/bash" ) ||
		   ( w == "#!" && findword( word( *t, 2 ), "/bin/sh /bin/bash" ) ) )
		{
			return bash ;
		}
		if ( findword( w, "TITLE CSECT DSECT MACRO START COPY" ) )
		{
			return assem ;
		}
		if ( i == 0 && w == "---" )
		{
			diffu1 = true ;
			++i ;
			continue ;
		}
		if ( i == 0 && w == "***" )
		{
			diffc1 = true ;
			++i ;
			continue ;
		}
		if ( i == 1 && diffu1 && w == "+++" ) { return diffu ; }
		if ( i == 1 && diffc1 && w == "---" ) { return diffc ; }
		if ( t->at( 0 ) == '*' ) { return assem ; }
		if ( t->at( 0 ) == '[' ) { return toml  ; }
		if ( t->at( 0 ) == ')' )
		{
			w = word( *t, 1 ) ;
			if ( w == ")PANEL" || w == ")COMMENT" || w == ")ATTR" )
			{
				return panel ;
			}
			return skel ;
		}
		if ( ( t->size() > 23 && upper( subword( t->substr( 6 ), 1, 2 ) ) == "IDENTIFICATION DIVISION." ) ||
		     ( t->size() > 11 && upper( subword( t->substr( 6 ), 1, 2 ) ) == "ID DIVISION." ) )
		{
			return cobol ;
		}
		if ( i == 0 && t->size() > 2 && t->at( 0 ) == '/' )
		{
			if ( t->compare( 1, 2, "/*" ) == 0 ) { return jcl ; }
			if ( t->at( 1 ) == '/' )
			{
				w = word( *t, 2 ) ;
				if ( findword( w, "JOB DD PROC EXEC MSG" ) ) { return jcl ; }
			}
			else if ( t->compare( 1, 9, "*PRIORITY" ) == 0 ) { return jcl ; }
		}
		if ( i == 0 && upper( *t ).find( "REXX" ) != string::npos )
		{
			return rexx ;
		}
		if ( w == "/*" ) { return cpp ; }
		++i ;
	}

	return def ;
}


bool pedit01::set_language_colours( const string& lang )
{
	//
	// Set the language colours in langColours map.
	// If we are changing the current language, also update the function pool variables.
	//
	// If language not found in the langColours map (because we have added a new one later),
	// setup langSpecials for the new language and add to the langColours map.
	//

	TRACE_FUNCTION() ;

	string zcmd ;

	string cd ;
	string cc ;
	string ck ;
	string cq ;
	string cv ;
	string cs ;

	string hd ;
	string hc ;
	string hk ;
	string hq ;
	string hv ;
	string hs ;
	string sp ;

	const string vlist1 = "ZCMD CD CC CK CQ CV CS SP" ;
	const string vlist2 = "HD HC HK HQ HV HS SP" ;

	vdefine( vlist1, &zcmd, &cd, &cc, &ck, &cq, &cv, &cs, &sp ) ;
	vdefine( vlist2, &hd, &hc, &hk, &hq, &hv, &hs, &sp ) ;

	vreplace( "LANG", lang ) ;

	auto it = langColours.find( lang ) ;
	if ( it == langColours.end() )
	{
		auto result = langColours.insert( pair<string, lang_colours>( lang, lang_colours( langSpecials[ lang ] ) ) ) ;
		it = result.first ;
	}

	lang_colours& c = it->second ;

	cd = c.cold ;
	cc = c.colc ;
	ck = c.colk ;
	cq = c.colq ;
	cv = c.colv ;
	cs = c.cols ;

	hd = ( c.hid == "" ) ? "NORMAL" : c.hid ;
	hc = ( c.hic == "" ) ? "NORMAL" : c.hic ;
	hk = ( c.hik == "" ) ? "NORMAL" : c.hik ;
	hq = ( c.hiq == "" ) ? "NORMAL" : c.hiq ;
	hv = ( c.hiv == "" ) ? "NORMAL" : c.hiv ;
	hs = ( c.his == "" ) ? "NORMAL" : c.his ;

	sp = c.specials ;

	addpop() ;
	while ( RC == 0 )
	{
		display( "PEDIT01N" ) ;
		if ( zcmd == "CANCEL" )
		{
			rempop() ;
			vdelete( vlist1, vlist2 ) ;
			return false ;
		}
		else if ( zcmd == "RESET" )
		{
			cd = c.cold ;
			cc = c.colc ;
			ck = c.colk ;
			cq = c.colq ;
			cv = c.colv ;
			cs = c.cols ;
			hd = ( c.hid == "" ) ? "NORMAL" : c.hid ;
			hc = ( c.hic == "" ) ? "NORMAL" : c.hic ;
			hk = ( c.hik == "" ) ? "NORMAL" : c.hik ;
			hq = ( c.hiq == "" ) ? "NORMAL" : c.hiq ;
			hv = ( c.hiv == "" ) ? "NORMAL" : c.hiv ;
			hs = ( c.his == "" ) ? "NORMAL" : c.his ;
			sp = c.specials ;
		}
		else if ( zcmd == "DEFAULTS" )
		{
			cd = "" ;
			cc = "" ;
			ck = "" ;
			cq = "" ;
			cv = "" ;
			cs = "" ;
			hd = "" ;
			hc = "" ;
			hk = "" ;
			hq = "" ;
			hv = "" ;
			hs = "" ;
			sp = langSpecials[ lang ] ;
		}
		zcmd = "" ;
	}

	c.cold = cd ;
	c.colc = cc ;
	c.colk = ck ;
	c.colq = cq ;
	c.colv = cv ;
	c.cols = cs ;

	c.hid = ( hd == "NORMAL" ) ? "" : hd ;
	c.hic = ( hc == "NORMAL" ) ? "" : hc ;
	c.hik = ( hk == "NORMAL" ) ? "" : hk ;
	c.hiq = ( hq == "NORMAL" ) ? "" : hq ;
	c.hiv = ( hv == "NORMAL" ) ? "" : hv ;
	c.his = ( hs == "NORMAL" ) ? "" : hs ;

	c.specials = sp ;

	rempop() ;

	if ( lang == "ALL" )
	{
		for ( auto& l : langColours )
		{
			if ( l.first == "ALL" ) { continue ; }
			l.second << c ;
		}
		set_language_fvars( detLang ) ;
	}
	else if ( lang == detLang )
	{
		set_language_fvars( lang ) ;
	}

	vdelete( vlist1, vlist2 ) ;

	return true ;
}


void pedit01::set_language_fvars( const string& lang )
{
	//
	// Set the language colours in the function pool variables.
	// If it isn't found, create a default for the language and add to the map.
	//

	TRACE_FUNCTION() ;

	auto it = langColours.find( lang ) ;
	if ( it == langColours.end() )
	{
		auto result = langColours.insert( pair<string, lang_colours>( lang, lang_colours( langSpecials[ lang ] ) ) ) ;
		it = result.first ;
	}

	const lang_colours& c = it->second ;

	zedclrd = c.cold ;
	zedclrc = c.colc ;
	zedclrk = c.colk ;
	zedclrq = c.colq ;
	zedclrv = c.colv ;
	zedclrs = c.cols ;

	zedhid  = c.hid ;
	zedhic  = c.hic ;
	zedhik  = c.hik ;
	zedhiq  = c.hiq ;
	zedhiv  = c.hiv ;
	zedhis  = c.his ;

	hlight.hl_specials = c.specials ;
}


void pedit01::load_language_colours()
{
	//
	// Load the language colours into the langColours map.
	// Create defaults if no table found.
	//

	TRACE_FUNCTION() ;

	int i ;
	int ws ;

	string temp ;

	string lang ;
	string cold ;
	string colc ;
	string colk ;
	string colq ;
	string colv ;
	string cols ;

	string hid ;
	string hic ;
	string hik ;
	string hiq ;
	string hiv ;
	string his ;

	string sp ;

	const string vlist1 = "LANG COLD COLC COLK COLQ COLV COLS" ;
	const string vlist2 = "HID HIC HIK HIQ HIV HIS SPECIALS" ;

	vdefine( vlist1, &lang, &cold, &colc, &colk, &colq, &colv, &cols ) ;
	vdefine( vlist2, &hid, &hic, &hik, &hiq, &hiv, &his, &sp ) ;

	tbopen( CLRTABLE, NOWRITE, "ZUPROF" ) ;
	if ( RC == 0 )
	{
		tbskip( CLRTABLE ) ;
		while ( RC == 0 )
		{
			langColours[ lang ] = lang_colours( cold,
							    colc,
							    colk,
							    colq,
							    colv,
							    cols,
							    hid,
							    hic,
							    hik,
							    hiq,
							    hiv,
							    his,
							    sp ) ;
			tbskip( CLRTABLE ) ;
		}
		tbend( CLRTABLE ) ;
	}

	if ( langColours.size() == 0 )
	{
		const string list = "ASM BASH COBOL CPP DEFAULT JCL OTHER PANEL RUST TOML SKEL REXX" ;
		for ( i = 1, ws = words( list ) ; i <= ws ; ++i )
		{
			temp = word( list, i ) ;
			langColours[ temp ] = lang_colours( langSpecials[ temp ] ) ;
		}
	}

	vdelete( vlist1, vlist2 ) ;
}


void pedit01::save_language_colours()
{
	//
	// Save the language colours into the CLRTABLE.
	//

	TRACE_FUNCTION() ;

	string lang ;

	string cold ;
	string colc ;
	string colk ;
	string colq ;
	string colv ;
	string cols ;

	string hid ;
	string hic ;
	string hik ;
	string hiq ;
	string hiv ;
	string his ;

	string sp ;

	const string vlist1 = "LANG COLD COLC COLK COLQ COLV COLS" ;
	const string vlist2 = "HID HIC HIK HIQ HIV HIS SPECIALS" ;

	vdefine( vlist1, &lang, &cold, &colc, &colk, &colq, &colv, &cols ) ;
	vdefine( vlist2, &hid, &hic, &hik, &hiq, &hiv, &his, &sp ) ;

	tbopen( CLRTABLE, WRITE, "ZUPROF" ) ;
	if ( RC > 0 )
	{
		tbcreate( CLRTABLE,
			  "LANG",
			  "(COLD,COLC,COLK,COLQ,COLV,COLS,HID,HIC,HIK,HIQ,HIV,HIS,SPECIALS)",
			  WRITE,
			  NOREPLACE,
			  "ZUPROF" ) ;
		if ( RC > 0 ) { uabend( "PEDT015W", "TBCREATE" ) ; }
	}

	for ( const auto& c : langColours )
	{
		lang = c.first ;
		cold = c.second.cold ;
		colc = c.second.colc ;
		colk = c.second.colk ;
		colq = c.second.colq ;
		colv = c.second.colv ;
		cols = c.second.cols ;
		hid  = c.second.hid ;
		hic  = c.second.hic ;
		hik  = c.second.hik ;
		hiq  = c.second.hiq ;
		hiv  = c.second.hiv ;
		his  = c.second.his ;
		sp   = c.second.specials ;
		tbmod( CLRTABLE ) ;
	}

	tbsort( CLRTABLE, "(LANG,C,A)" ) ;
	tbclose( CLRTABLE, "", "ZUPROF" ) ;

	vdelete( vlist1, vlist2 ) ;
}


void pedit01::run_macro( const string& macro,
			 bool imacro,
			 bool editserv,
			 bool force_pgmmac,
			 const string& lcmd )
{
	//
	// Call PEDRXM1 for REXX macros or SELECT application for c++ macros.
	//
	// Set the initial values for mRow and mCol, macro CURSOR position.
	// Process screen changes and line commands if no PROCESS has been done when last macro terminates.
	//
	// REXX search order:
	//       ALTLIB-set path for EXTERNAL_CALL_PATH.
	//       REXX_PATH env variable (don't add to miBlock.rxpath1 as OOREXX searches this automatically).
	//       PATH      env variable (as above).
	//
	// Return from the macro is in miBlock.exitRC.
	// 0    Normal completion, command line blanked and cursor as set by the macro.
	// 1    Cursor placed on command line, and command line is blanked.
	// 4,8  Treated as 0.
	// 12+  Error.  Cursor placed on the command line and edit macro remains.
	// 20
	// 24   Nesting level of 255 exceeded.
	// 28
	//
	// Note: always keep the command on the command line if prefixed with '&'
	//

	TRACE_FUNCTION() ;

	BOOST_SCOPE_EXIT( &macroRunning, &dataid2, &nestLevel, &zdest, &zfrange, &zlrange, &zrange_cmd, &miBlock, &rebuildZAREA, this_ )
	{
		macroRunning = false ;
		if ( dataid2 != "" )
		{
			this_->lmfree( dataid2 ) ;
			dataid2 = "" ;
		}
		this_->set_userdata( nullptr ) ;
		nestLevel  = 0  ;
		zdest      = nullptr ;
		zfrange    = nullptr ;
		zlrange    = nullptr ;
		zrange_cmd = "" ;
		miBlock.clear() ;
	}
	BOOST_SCOPE_EXIT_END

	bool cmdline ;

	Data::Iterator it = nullptr ;

	miBlock.clear_all() ;

	clear_user_states() ;

	nestLevel  = 0  ;
	zdest      = nullptr ;
	zfrange    = nullptr ;
	zlrange    = nullptr ;
	zrange_cmd = "" ;

	set_userdata( (void *)&miBlock ) ;

	miBlock.editAppl = this ;
	miBlock.etaskid  = taskid() ;
	miBlock.imacro   = imacro   ;
	miBlock.lcmd     = lcmd     ;
	miBlock.editserv = editserv ;

	get_profile_var( "ZLDPATH", miBlock.zldpath ) ;

	miBlock.set_macro( macro, defNames, force_pgmmac ) ;
	miBlock.set_cmd( pcmd ) ;

	if ( miBlock.cmd_macro || !miBlock.pgm_macro )
	{
		miBlock.rxpath1 = sysexec() ;
		miBlock.rxpath2 = mergepaths( miBlock.rxpath1, getenv( "REXX_PATH" ), getenv( "PATH" ) ) ;
		if ( !miBlock.getMacroFileName( miBlock.rxpath2 ) )
		{
			if ( miBlock.cmd_macro )
			{
				pcmd.set_msg( ( miBlock.RC > 8 ) ? "PEDM012Q" : "PEDT015A", 12 ) ;
				cursor.home() ;
				return ;
			}
			miBlock.pgm_macro = true ;
			iupper( miBlock.emacro ) ;
		}
		else
		{
			miBlock.cmd_macro = true ;
		}
	}

	set_macro_cursor() ;

	cmdline = ( curfld != "ZAREA" ) ;

	macroRunning = true ;

	control( "ERRORS", "RETURN" ) ;

	if ( miBlock.cmd_macro )
	{
		select( "PGM(PEDRXM1) PARM("+ d2ds( taskid() ) +") NESTED" ) ;
	}
	else if ( is_pgmmacro( miBlock.emacro ) )
	{
		miBlock.mfound = true ;
		select( "PGM("+ miBlock.emacro +") PARM("+ d2ds( taskid() ) +") NESTED" ) ;
	}
	else
	{
		control( "ERRORS", "CANCEL" ) ;
		return ;
	}

	if ( lnumSize > 0 && iline::file_inserts[ taskid() ] )
	{
		add_line_numbers() ;
		iline::file_inserts[ taskid() ] = false ;
	}

	if ( RC == 20 )
	{
		pcmd.set_msg( ( ZRESULT == "Abended" ) ? "PEDT017F" : "PEDT015B", macro, 20 ) ;
		pcmd.cond_reset() ;
		cursor.home() ;
		miBlock.fatal = true ;
	}

	control( "ERRORS", "CANCEL" ) ;

	clear_user_states() ;

	ZRESULT = "" ;
	if ( miBlock.mfound    &&
	    !miBlock.processed &&
	    !miBlock.imacro    &&
	    !miBlock.lcmacro() &&
	    !miBlock.cancel )
	{
		updateData() ;
		if ( pcmd.error() )
		{
			termEdit = false ;
			return ;
		}
		processNewInserts() ;
		actionLineCommands() ;
		if ( pcmd.error() )
		{
			termEdit = false ;
			return ;
		}
		if ( miBlock.eended && termOK() )
		{
			return ;
		}
	}

	pcmd.setRC( miBlock.exitRC() ) ;

	if ( miBlock.fatal )
	{
		pcmd.setRC( 20 ) ;
		pcmd.set_msg( "PEDT017D",
			      macro,
			      miBlock.exitRC() ) ;
		cursor.home() ;
		pcmd.reset() ;
	}
	else if ( miBlock.exitRC() == 1 )
	{
		cursor.home() ;
	}
	else if ( miBlock.exitRC() >= 12 )
	{
		pcmd.set_msg( "PEDT017E",
			      macro,
			      d2ds( miBlock.exitRC() ),
			      miBlock.exitRC() ) ;
		cursor.home() ;
	}
	else if ( !mRow || ( cmdline && !miBlock.cursorset ) )
	{
		cursor.home() ;
	}
	else if ( miBlock.cursorset )
	{
		if ( mCol == 0 )
		{
			cursor.set( mRow, CSR_FC_LCMD ) ;
		}
		else
		{
			cursor.set( mRow, CSR_OFF_DATA, mCol - 1 ) ;
		}
	}

	if ( miBlock.iskeep() )
	{
		pcmd.set_cmd_keep( macro ) ;
	}

	rebuildZAREA = true ;
}


void pedit01::set_macro_cursor()
{
	//
	// Set the initial macro cursor position (mRow, mCol).
	//
	// Internally, mCol includes the line number in number mode.
	// Only added/removed when externalised.
	//
	// See table 20 ISPF Edit And Edit Macros.
	//

	TRACE_FUNCTION() ;

	if ( !getFileLineZFIRST() )
	{
		mRow = nullptr ;
		mCol = 0 ;
	}
	else if ( curfld != "ZAREA" )
	{
		mRow = topLine ;
		mCol = 0 ;
		if ( mRow->not_valid_file() )
		{
			mRow = getNextFileLine( mRow ) ;
			if ( !mRow )
			{
				mRow = getFileLineZLAST() ;
			}
		}
	}
	else if ( aADDR )
	{
		mRow = aADDR ;
		if ( mRow->is_isrt() )
		{
			mRow = getPrevFileLine( mRow ) ;
			if ( !mRow )
			{
				mRow = getFileLineZFIRST() ;
				mCol = 0 ;
			}
			else
			{
				mCol = ( aDATA ) ? aOff + 1 : 0 ;
			}
		}
		else if ( mRow->not_valid_file() )
		{
			mRow = getNextFileLine( mRow ) ;
			if ( !mRow )
			{
				mRow = getFileLineZLAST() ;
				mCol = mRow->get_idata_len() + 1 ;
			}
			else
			{
				mCol = 0 ;
			}
		}
		else
		{
			mCol = ( aDATA ) ? aOff + 1 : 0 ;
		}
		if ( reclen > 0 && mCol > reclen )
		{
			mCol = 0 ;
		}
	}
	else
	{
		mRow = getFileLineZLAST() ;
		mCol = mRow->get_idata_len() + 1 ;
	}
}


void pedit01::clear_user_states()
{
	TRACE_FUNCTION() ;

	for ( auto it = user_states.begin() ; it != user_states.end() ; ++it )
	{
		delete it->second ;
	}

	user_states.clear() ;
}


bool pedit01::is_pgmmacro( const string& macro )
{
	//
	// Check program being executed as a macro exists, and contains symbol lspf_editmac_v1.
	//

	TRACE_FUNCTION() ;

	locator loc( miBlock.zldpath, "/lib" + macro + ".so" ) ;
	loc.locate() ;
	if ( loc.not_found() )
	{
		pcmd.set_msg( "PEDT017P", 20 ) ;
		cursor.home() ;
		return false ;
	}

	dynloader loader( loc.entry() ) ;

	loader.open() ;
	if ( loader.errors() )
	{
		pcmd.set_msg( "PEDT017Q", 20 ) ;
		cursor.home() ;
		return false ;
	}

	loader.lookup( "lspf_editmac_v1" ) ;
	if ( loader.errors() )
	{
		pcmd.set_msg( "PEDT017Q", 20 ) ;
		cursor.home() ;
		return false ;
	}

	return true ;
}


void pedit01::term_resize()
{
	//
	// Get/set new size of variables after a terminal resize.
	// Adjust Top/Bottom of data.
	//

	TRACE_FUNCTION() ;

	uint i ;

	string t ;

	vector<ipos>::iterator it ;

	const string tod    = " Top of Data **********" ;
	const string bod    = " Bottom of Data **********" ;
	const string vlist1 = "ZSCREEND ZSCREENW" ;

	iline* dl ;

	vget( vlist1, SHARED ) ;

	pquery( panel, "ZAREA", "", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )
	{
		uabend() ;
	}

	zasize = zareaw * zaread ;
	zdataw = zareaw - CLINESZ ;

	sdg.assign( zdataw, N_GREEN ) ;
	sdy.assign( zdataw, N_YELLOW ) ;
	sdw.assign( zdataw, N_WHITE ) ;
	sdr.assign( zdataw, N_RED ) ;
	sdb.assign( zdataw, N_BLUE ) ;

	div.assign( zareaw-1, '-' ) ;

	for ( it = s2data.begin(), i = 0 ; it != s2data.end() ; ++it, ++i )
	{
		t = d2ds( i, 3 ) ;
		vdelete( "ZOVRV" + t, it->ipos_ovalue ) ;
		vdelete( "ZOVRS" + t, it->ipos_oshadow ) ;
		vdelete( "ZOVRU" + t, it->ipos_oupdate ) ;
	}

	s2data.clear() ;
	s2data.reserve( zaread ) ;

	for ( i = 0 ; i < zaread ; ++i )
	{
		s2data.push_back( ipos( zdataw ) ) ;
	}

	for ( it = s2data.begin(), i = 0 ; it != s2data.end() ; ++it, ++i )
	{
		t = d2ds( i, 3 ) ;
		vdefine( "ZOVRV" + t, &it->ipos_ovalue ) ;
		vdefine( "ZOVRS" + t, &it->ipos_oshadow ) ;
		vdefine( "ZOVRU" + t, &it->ipos_oupdate ) ;
	}

	dl = data.top() ;
	dl->put_idata( centre( tod, zareaw, '*' ) ) ;

	dl = data.bottom() ;
	dl->put_idata( centre( bod, zareaw, '*' ) ) ;

	rebuildZAREA = true ;
}


void pedit01::isredit( const string& s )
{
	TRACE_FUNCTION() ;

	string s1 = ( miBlock.scanOn() ) ? miBlock.macAppl->sub_vars( s ) : s ;

	if ( upper( word( s1, 1 ) ) == "ISREDIT" )
	{
		idelword( s1, 1, 1 ) ;
	}

	miBlock.parse( s1, defNames ) ;

	if ( miBlock.fatal || miBlock.runmacro )
	{
		return ;
	}

	miBlock.setRC( 0 ) ;
	pcmd.clear() ;

	clr_zedimsgs() ;

	( miBlock.query ) ? querySetting() : actionService() ;
}


void pedit01::clr_zedimsgs()
{
	TRACE_FUNCTION() ;

	if ( miBlock.messageOn )
	{
		miBlock.macAppl->vreplace( "ZEDMSGNO", "" ) ;
		miBlock.macAppl->vreplace( "ZEDISMSG", "" ) ;
		miBlock.macAppl->vreplace( "ZEDILMSG", "" ) ;
	}
}


void pedit01::set_zedimsgs()
{
	//
	// Set macro application function pool variables:
	//   ZEDMSGNO
	//   ZEDISMSG
	//   ZEDILMSG
	// for the message in pcmd if not an error and MACRO_MSG = ON.
	//

	TRACE_FUNCTION() ;

	string* t ;

	if ( macroRunning && miBlock.messageOn && pcmd.msgset() && pcmd.ok() )
	{
		set_msg_variables() ;
		getmsg( pcmd.get_msg(),
			"ZERRSM",
			"ZERRLM" ) ;
		miBlock.macAppl->vreplace( "ZEDMSGNO", pcmd.get_msg() ) ;
		vcopy( "ZERRSM", t, LOCATE ) ;
		miBlock.macAppl->vreplace( "ZEDISMSG", *t ) ;
		vcopy( "ZERRLM", t, LOCATE ) ;
		miBlock.macAppl->vreplace( "ZEDILMSG", *t ) ;
	}
}


void pedit01::set_zedimsgs( const string& m )
{
	//
	// Set macro application function pool variables:
	//   ZEDMSGNO
	//   ZEDISMSG
	//   ZEDILMSG
	// for message 'm' if MACRO_MSG = ON and running from a macro.
	//

	TRACE_FUNCTION() ;

	string* t ;

	if ( macroRunning && miBlock.messageOn )
	{
		getmsg( m,
			"ZERRSM",
			"ZERRLM" ) ;
		miBlock.macAppl->vreplace( "ZEDMSGNO", m ) ;
		vcopy( "ZERRSM", t, LOCATE ) ;
		miBlock.macAppl->vreplace( "ZEDISMSG", *t ) ;
		vcopy( "ZERRLM", t, LOCATE ) ;
		miBlock.macAppl->vreplace( "ZEDILMSG", *t ) ;
	}
}


void pedit01::startRecoveryTask()
{
	//
	//  Start the background recovery subtask.
	//

	TRACE_FUNCTION() ;

	bThread = new boost::thread( &pedit01::updateRecoveryData, this ) ;

	while ( recvStatus != RECV_RUNNING )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 1 ) ) ;
	}
}


void pedit01::stopRecoveryTask()
{
	//
	//  Stop the background recovery subtask if it is still running.
	//

	TRACE_FUNCTION() ;

	int i = 0 ;

	if ( recvStatus == RECV_STOPPED )
	{
		return ;
	}

	recvStatus = RECV_STOPPING ;
	cond_recov.notify_all() ;

	while ( recvStatus != RECV_STOPPED && ++i < 1000 )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
	}

	delete bThread ;
	bThread = nullptr ;
}


void pedit01::updateRecoveryData()
{
	//
	// This method runs in the background to update the recovery file when RECOVERY is ON
	// so we have a point-of-failure recovery file without affecting online performance.
	//
	// Since there is no locking to protect the data container in a multi-threaded
	// environment, this only runs while the edit screen is being displayed.  This should
	// be enough time for even very large files.
	//
	// Subtask will be started with the RECOVERY ON command and stopped with the RECOVERY OFF command.
	//
	// Don't access anything updated by the display service (namely variable RC) in this procedure.
	//

	TRACE_FUNCTION() ;

	string tfile ;
	string dfile ;

	bool aborted = false ;

	int fileLevel = 0 ;

	path temp = unique_path( backupLoc + zuser + "-" + zscreen + "-isredit-%%%%%.recov" ) ;

	bfile = temp.native() ;
	tfile = bfile + ".swp" ;
	dfile = bfile + ".del" ;

	uint i ;

	string* pt ;

	string t1 ;
	string spaces = string( profXTabz, ' ' ) ;

	std::ofstream fout ;

	boost::mutex mutex ;

	recvStatus = RECV_RUNNING ;

	while ( recvStatus == RECV_RUNNING )
	{
		boost::mutex::scoped_lock lk( mutex ) ;
		cond_recov.wait_for( lk, boost::chrono::milliseconds( 200 ) ) ;
		lk.unlock() ;
		if ( recvStatus == RECV_RUNNING && !recovSusp && canBackup && iline::get_Global_File_level( taskid() ) != fileLevel )
		{
			fout.open( tfile.c_str() ) ;
			if ( !fout.is_open() )
			{
				recovSusp = true ;
				llog( "E", "File "<< tfile <<" cannot be opened.  RECOVERY suspended."<<endl ) ;
				break ;
			}
			for ( auto& ln : data )
			{
				if ( !canBackup )
				{
					aborted = true ;
					break ;
				}
				if ( ln.not_valid_file() ) { continue ; }
				pt = ln.get_idata_ptr() ;
				if ( !optPreserve && reclen == 0 ) { ln.set_idata_trim() ; }
				if ( profXTabs )
				{
					t1 = "" ;
					for ( i = 0 ; i < pt->size() ; ++i )
					{
						if ( ( i % profXTabz == 0 )         &&
						     pt->size() > ( i+profXTabz-1 ) &&
						     pt->compare( i, profXTabz, spaces ) == 0 )
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
			fout.close() ;
			if ( fout.fail() )
			{
				recovSusp = true ;
				llog( "E", "File "<< tfile <<" close error.  RECOVERY suspended."<<endl ) ;
			}
			if ( aborted )
			{
				remove( tfile ) ;
				aborted = false ;
			}
			else
			{
				fileLevel = iline::get_Global_File_level( taskid() ) ;
				if ( exists( bfile ) )
				{
					rename( bfile, dfile ) ;
				}
				rename( tfile, bfile ) ;
				remove( dfile ) ;
			}
		}
	}

	recvStatus = RECV_STOPPED ;
}


void pedit01::querySetting()
{
	//
	// Retrieve setting in miBlock.keyword and store in variable names in miBlock.var1 and var2.
	//
	// These variables are in the macro application function pool, not the editor function pool so use
	// the macAppl pointer in the mib when invoking variable services.
	//

	TRACE_FUNCTION() ;

	int i ;

	int rc ;
	int lvl ;

	string t1  ;
	string lab ;
	string kw1 ;

	iline* dl ;
	iline* adr1 ;
	iline* adr2 ;

	Data::Iterator it = nullptr ;

	kw1 = word( miBlock.keyopts, 1 ) ;

	pApplication* macAppl = miBlock.macAppl ;

	switch ( miBlock.m_cmd )
	{
	case EM_AUTONUM:
			//
			// Query AUTONUM mode.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			macAppl->vreplace( miBlock.var1, OnOff[ profAutoNum ] ) ;
			break ;

	case EM_AUTOSAVE:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, OnOff[ profAutoSave ] ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, ( profSaveP ) ? "PROMPT" : "NOPROMPT" ) ;
			}
			break ;

	case EM_BACKUP:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, OnOff[ profBackup ] ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, backupLoc ) ;
			}
			break ;

	case EM_CAPS:
			macAppl->vreplace( miBlock.var1, OnOff[ profCaps ] ) ;
			break ;

	case EM_BOUNDS:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( LeftBnd - lnumSize1, 5 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, ( RightBnd > 0 ) ? d2ds( RightBnd - lnumSize1, 5 ) : "00000" ) ;
			}
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
			//
			// Query the column and relative line number position.
			//
			// Column number does not include the number when NUM ON.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( getFileLine( mRow ), 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( max( 0, int( mCol - lnumSize1 ) ), 5 ) ) ;
			}
			break ;

	case EM_DATAID:
			//
			// Query the DATAID for the current file being edited.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.  Dataid passed to the editor.
			// RC =  4 Dataid generated and will be freed when macro terminates.
			// RC =  8 A previously generated dataid returned.
			// RC = 20 Severe error.
			//
			if ( dataid1 != "" )
			{
				macAppl->vreplace( miBlock.var1, dataid1 ) ;
			}
			else if ( dataid2 != "" )
			{
				macAppl->vreplace( miBlock.var1, dataid2 ) ;
				miBlock.setRC( 8 ) ;
			}
			else
			{
				lminit( "ZEDATAID", zfile, "", "EXCLU" ) ;
				vcopy( "ZEDATAID", dataid2 ) ;
				macAppl->vreplace( miBlock.var1, dataid2 ) ;
				miBlock.setRC( 4 ) ;
			}
			break ;

	case EM_DATA_CHANGED:
			macAppl->vreplace( miBlock.var1, YesNo[ fileChanged ] ) ;
			break ;

	case EM_DATA_WIDTH:
			//
			// Query the logical data width.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Invalid command format.
			// RC = 20 Severe error.
			//
			i = ( reclen > 0 ) ? reclen - lnumSize : 0 ;
			macAppl->vreplace( miBlock.var1, d2ds( i, 5 ) ) ;
			break ;

	case EM_DATASET:
			macAppl->vreplace( miBlock.var1, zfile ) ;
			break ;

	case EM_DISPLAY_COLS:
			//
			// Retrieve the first and last data columns.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Invalid command format.
			// RC = 20 Severe error.
			//
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
				macAppl->vreplace( miBlock.var2, d2ds( startCol+zdataw-1, 5 ) ) ;
			}
			break ;

	case EM_DISPLAY_LINES:
			//
			// Retrieve the first and last data lines that would appear if the macro ended.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 No visible data lines.
			// RC =  8 No data lines.
			// RC = 12 Invalid command format.
			// RC = 20 Severe error.
			//
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
			if ( !getFileLineZFIRST() )
			{
				miBlock.setRC( 8 ) ;
				break ;
			}
			adr1 = nullptr ;
			fill_dynamic_area() ;
			for ( int i = 0 ; i < zlvline ; ++i )
			{
				const ipos& tpos = s2data[ i ] ;
				if ( tpos.ipos_hex_line() ) { continue ; }
				dl = tpos.ipos_addr ;
				if ( dl && dl->is_valid_file() )
				{
					if ( !adr1 ) { adr1 = dl ; }
					adr2 = dl ;
				}
			}
			if ( !adr1 )
			{
				miBlock.setRC( 4 ) ;
				break ;
			}
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( getFileLine( adr1 ), 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( getFileLine( adr2 ), 8 ) ) ;
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

	case EM_FLOW_COUNTS:
			//
			// Query values from the most recent TFLOW command.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, d2ds( tflow1, 8 ) ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, d2ds( tflow2, 8 ) ) ;
			}
			break ;

	case EM_HEX:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, OnOff[ profHex ] ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, ( profHex ) ? ( profVert ) ? "VERT" : "DATA" : "" ) ;
			}
			break ;

	case EM_LABEL:
			//
			// Retrieve label by relative line number or label.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Line has no label.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			if ( getLinePtrIterator( iupper( kw1 ), it ) != 1 )
			{
				if ( miBlock.var1 != "" ) { macAppl->vreplace( miBlock.var1, "" ) ; }
				if ( miBlock.var2 != "" ) { macAppl->vreplace( miBlock.var2, "" ) ; }
				break ;
			}
			it->getLabelInfo( nestLevel, lab, lvl ) ;
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
			//
			// Retrieve a line by relative line number or label.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Invalid line number or line pointer.
			// RC = 20 Severe error.
			//
			// Data does not include any COBOL or STD line numbers.
			//
			if ( getLinePtrIterator( iupper( kw1 ), it ) == 1 )
			{
				macAppl->vreplace( miBlock.var1, it->get_idata( lnumSize1, lnumS2pos ) ) ;
			}
			break ;

	case EM_LINE_STATUS:
			//
			// Retrieve the source and change information of a line by relative line number or label.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			if ( getLinePtrIterator( iupper( kw1 ), it ) == 1 )
			{
				macAppl->vreplace( miBlock.var1, i2bs( it->get_idstatus() ) ) ;
			}
			break ;

	case EM_LINENUM:
			//
			// Query the relative line number of a specified label.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Line 0 specified or no file lines.
			// RC =  8 Label spcified but not found (variable set to 0).
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			rc = getLinePtrIterator( kw1, it ) ;
			if ( rc == 1 )
			{
				macAppl->vreplace( miBlock.var1, d2ds( getFileLine( it ), 8 ) ) ;
			}
			else if ( rc == 0 )
			{
				miBlock.setRC( 4 ) ;
				macAppl->vreplace( miBlock.var1, "00000000" ) ;
				break ;
			}
			else if ( rc == -1 )
			{
				miBlock.setRC( 8 ) ;
				macAppl->vreplace( miBlock.var1, "00000000" ) ;
				break ;
			}
			break ;

	case EM_LRECL:
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Invalid command format.
			// RC = 20 Severe error.
			//
			macAppl->vreplace( miBlock.var1, right( file_lrecl(), 5, '0' ) ) ;
			break ;

	case EM_IMACRO:
			macAppl->vreplace( miBlock.var1, profIMACRO ) ;
			break ;

	case EM_MACRO_MSG:
			//
			// Query the value of MACRO_MSG
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			macAppl->vreplace( miBlock.var1, OnOff[ miBlock.messageOn ] ) ;
			break ;

	case EM_MASKLINE:
			//
			// Query the MASK line.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			macAppl->vreplace( miBlock.var1, maskLine ) ;
			break ;

	case EM_NOTES:
			macAppl->vreplace( miBlock.var1, OnOff[ profNotes ] ) ;
			break ;

	case EM_NUMBER:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, OnOff[ profNum ] ) ;
			}
			if ( miBlock.var2 != "" )
			{
				t1  = ( profNumSTD  ) ? "STD "    : "NOSTD "   ;
				t1 += ( profNumCBL  ) ? "COBOL "  : "NOCOBOL " ;
				t1 += ( profNumDisp ) ? "DISPLAY" : "NODISPL"  ;
				macAppl->vreplace( miBlock.var2, t1 ) ;
			}
			break ;

	case EM_NULLS:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, OnOff[ profNulls ] ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, ( profNulls ) ? ( profNullA ) ? "ALL" : "STD" : "" ) ;
			}
			break ;

	case EM_PATH:
			//
			// Query the backup path location.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			macAppl->vreplace( miBlock.var1, backupLoc ) ;
			break ;

	case EM_PROFILE:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, zedprof ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, ( profLock ) ? "LOCK" : "UNLOCK" ) ;
			}
			break ;

	case EM_RANGE_CMD:
			//
			// Identify a line command entered from the keyboard.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Line command not set.
			// RC =  8 Line command setting not acceptable.
			// RC = 20 Severe error.
			//
			macAppl->vreplace( miBlock.var1, zrange_cmd ) ;
			if ( zrange_cmd == "" )
			{
				miBlock.setRC( 4 ) ;
			}
			break ;

	case EM_RECFM:
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, file_recfm() ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, "" ) ;
			}
			break ;

	case EM_RECOVERY:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, OnOff[ profRecovery ] ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, "" ) ;
			}
			break ;

	case EM_SCAN:
			macAppl->vreplace( miBlock.var1, OnOff[ miBlock.scanOn() ] ) ;
			break ;

	case EM_SAVE_LENGTH:
			//
			// Query data length
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.  Data saved.
			// RC =  6 Record format is not variable.
			// RC = 16 Error setting variable.
			// RC = 20 Severe error.
			//
			if ( getLinePtrIterator( iupper( kw1 ), it ) != 1 )
			{
				break ;
			}
			macAppl->vreplace( miBlock.var1, d2ds( it->get_idata_len() ) ) ;
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

	case EM_SESSION:
			//
			// Query session type, EDIT or VIEW.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, ( zedvmode ) ? "VIEW" : "EDIT" ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, "****" ) ;
			}
			break ;

	case EM_SETUNDO:
			//
			// Query SETUNDO setting.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			macAppl->vreplace( miBlock.var1, ( profUndoOn ) ? ( profUndoKeep ) ? "KEEP" : "ON" : "OFF" ) ;
			break ;

	case EM_STATS:
			macAppl->vreplace( miBlock.var1, OnOff[ profStats ] ) ;
			break ;

	case EM_TABS:
			//
			// Query TABS mode.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, ( profTabs ) ? "ON" : "OFF" ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, ( profTabs ) ?
								 ( tabsChar == ' ' ) ?
								 ( profATabs ) ? "ALL" : "STD" : string( 1, tabsChar ) : "" ) ;
			}
			break ;

	case EM_TABSLINE:
			//
			// Query the TABSLINE.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			macAppl->vreplace( miBlock.var1, tabsLine ) ;
			break ;

	case EM_USER_STATE:
			//
			// Store user state using a random key.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			{
				string key ;

				profile prof ;
				load_profile( prof ) ;
				user_state* u = new user_state( prof,
								fcx_parms,
								getMaskLine1(),
								tabsLine,
								LeftBnd,
								RightBnd,
								mRow,
								getFileLine( mRow ),
								mCol,
								topLine,
								getFileLine( topLine ),
								miBlock.cursorset ) ;
				key = u->get_key( ++miBlock.ustatekey ) ;
				user_states[ key ] = u ;
				macAppl->vreplace( miBlock.var1, key ) ;
			}
			break ;

	case EM_XSTATUS:
			//
			// Query the exclude status of a data line at linenum or label.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Line number does not exist.
			// RC = 20 Severe error.
			//
			if ( getLinePtrIterator( iupper( kw1 ), it ) != 1 )
			{
				break ;
			}
			macAppl->vreplace( miBlock.var1, ( it->is_excluded() ) ? "X" : "NX" ) ;
			break ;

	case EM_XTABS:
			if ( miBlock.var1 != "" )
			{
				macAppl->vreplace( miBlock.var1, OnOff[ profXTabs ] ) ;
			}
			if ( miBlock.var2 != "" )
			{
				macAppl->vreplace( miBlock.var2, profXTabz ) ;
			}
			break ;

	default:
			miBlock.seterror( "PEDM011A", 12 ) ;
	}
}


void pedit01::actionService()
{
	//
	// Action a service from a command macro.  Details of the service are in the macro interface block.
	//
	// These variables are in the macro application function pool, not the editor function pool so use
	// the macAppl pointer in the mib when invoking variable services.
	//
	// For normal primary commands (macro command syntax and assignment syntax), just create the expected
	// CMD and check the resulting cmdBlock object.
	//

	TRACE_FUNCTION() ;

	int n ;
	int rc ;
	int tCol ;
	int l1 ;
	int l2 ;
	int ws ;

	bool unused = false ;
	bool before = true ;

	bool okay ;

	vector<lcmd>::iterator itc ;

	lcmd cmd  ;

	string t   ;
	string t1  ;
	string head ;
	string tail ;
	string vw1 ;
	string vw2 ;
	string vw3 ;
	string kw1 ;
	string kw2 ;
	string kw3 ;
	string opt ;

	Data::Iterator it  = nullptr ;
	Data::Iterator it1 = nullptr ;
	Data::Iterator it2 = nullptr ;

	pApplication* macAppl = miBlock.macAppl ;

	vw1 = word( miBlock.value, 1 ) ;
	vw2 = word( miBlock.value, 2 ) ;

	kw1 = word( miBlock.keyopts, 1 ) ;
	kw2 = word( miBlock.keyopts, 2 ) ;
	kw3 = word( miBlock.keyopts, 3 ) ;

	iline* addr ;

	vector<string> vstring ;

	vector<ipline> vip ;

	rebuildZAREA = true ;

	cmd_range range ;

	switch ( miBlock.m_cmd )
	{
	case EM_CANCEL:
			//
			// Cancel out of the edit session.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			if ( miBlock.get_sttment_words() > 1 )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			miBlock.eended = true ;
			miBlock.cancel = true ;
			termEdit       = true ;
			break ;

	case EM_COPY:
			//
			// Copy data from a file into the data being edited.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  8 End of data before last record found (l2).
			// RC = 12 Invalid line pointer (linenum or label).  File not found.
			// RC = 16 End of data before first record found (l1).
			// RC = 20 Severe error (syntax error or I/O error).
			//
			l1  = -1 ;
			l2  = -1 ;
			t   = upper( miBlock.value ) ;
			n   = wordpos( "BEFORE", t ) ;
			if ( n == 0 )
			{
				n = wordpos( "AFTER", t ) ;
				if ( n == 0 )
				{
					miBlock.seterror( "PEDM012G", 20 ) ;
					break ;
				}
				before = false ;
			}
			head = strip( subword( miBlock.value, 1, ( n - 1 ) ) ) ;
			tail = upper( subword( miBlock.value, ( n + 1 ) ) ) ;
			ws   = words( tail ) ;
			if ( ws > 3 )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			if ( ws == 0 )
			{
				miBlock.seterror( "PEDM012G", 20 ) ;
				break ;
			}
			vw1 = word( tail, 1 ) ;
			if ( getLinePtrIterator( iupper( vw1 ), it1 ) != 1 )
			{
				break ;
			}
			vw1 = word( tail, 2 ) ;
			if ( vw1 != "" )
			{
				if ( !isnumeric( vw1 ) )
				{
					miBlock.seterror( "PEDM011P", 20 ) ;
					break ;
				}
				l1 = ds2d( vw1 ) ;
			}
			vw1 = word( tail, 3 ) ;
			if ( vw1 != "" )
			{
				if ( !isnumeric( vw1 ) )
				{
					miBlock.seterror( "PEDM011P", 20 ) ;
					break ;
				}
				l2 = ds2d( vw1 ) ;
			}
			t = expandFileName( head ) ;
			if ( !exists( t ) )
			{
				miBlock.seterror( "PEDM014B", t, 12 ) ;
				break ;
			}
			copyIn( t, ( before ) ? --it1 : it1, l1, l2 ) ;
			break ;

	case EM_CREATE:
	case EM_REPLACE:
			//
			// Create/Replace file between ranges.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  8 File exists (CREATE only)
			// RC = 12 Invalid label or relative line number.
			// RC = 20 Severe error.
			//
			t  = miBlock.value ;
			rc = extract_lptr( t, it1, it2 ) ;
			if ( rc > 1 )
			{
				miBlock.seterror( "PEDT018H", 12 ) ;
				break ;
			}
			if ( t == "" )
			{
				miBlock.seterror( "PEDM012G", 20 ) ;
				break ;
			}
			t = expandFileName( t ) ;
			if ( miBlock.m_cmd == EM_CREATE && exists( t ) )
			{
				miBlock.setRC( 8 ) ;
				break ;
			}
			create_file( t, it1, it2 ) ;
			miBlock.seterror( pcmd ) ;
			set_zedimsgs() ;
			pcmd.cond_reset() ;
			break ;

	case EM_CURSOR:
			//
			// Set the column and relative line number position.
			//
			// Column number does not include the number when NUM ON.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Column number beyond data.  Line number incremented, mCol set to 0.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			if ( isnumeric( vw1 ) )
			{
				mRow = itr2ptr( getDataLine( ds2d( vw1 ) ) ) ;
				if ( mRow == data.bottom() )
				{
					miBlock.seterror( "PEDM013C", 12 ) ;
					break ;
				}
			}
			else
			{
				if ( !checkLabel2( vw1, miBlock.nestlvl ) )
				{
					miBlock.seterror( "PEDM011R", 12 ) ;
					break ;
				}
				if ( getLinePtrIterator( vw1, it ) != 1 )
				{
					break ;
				}
				mRow = itr2ptr( it ) ;
			}
			if ( isnumeric( vw2 ) )
			{
				mCol = ds2d( vw2 ) + lnumSize1 ;
				if ( mRow && mCol > mRow->get_idata_len() )
				{
					mRow = getNextFileLine( mRow ) ;
					if ( !mRow )
					{
						miBlock.seterror( "PEDM013C", 12 ) ;
						break ;
					}
					mCol = 0 ;
					miBlock.setRC( 4 ) ;
				}
			}
			else if ( vw2 != "" )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
			}
			miBlock.cursorset = true ;
			break ;

	case EM_CUT:
			//
			// Copy data to clipboard.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Parameter error.
			// RC = 20 Severe error.
			//
			t  = miBlock.value ;
			rc = extract_lptr( t, it1, it2 ) ;
			if ( rc > 1 )
			{
				miBlock.seterror( "PEDT018H", 12 ) ;
			}
			else
			{
				Cut cut( pcmd, profCutReplace, t, it1, ++it2 ) ;
				if ( pcmd.error() )
				{
					miBlock.seterror( pcmd ) ;
					pcmd.cond_reset() ;
					break ;
				}
				create_copy( cut, vip ) ;
				clipBoard  = cut.clipBoard() ;
				cutReplace = cut.repl() ;
				copyToClipboard( vip ) ;
				miBlock.seterror( pcmd ) ;
				set_zedimsgs() ;
				pcmd.cond_reset() ;
			}
			break ;

	case EM_DELETE:
			//
			// Delete lines from the data being edited.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion, lines deleted.
			// RC =  4 No lines deleted.
			// RC =  8 No standard records exist.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			{
				if ( !getFileLineZFIRST() )
				{
					miBlock.setRC( 8 ) ;
					break ;
				}

				Del del( miBlock.value ) ;
				if ( del.del_error != "" )
				{
					miBlock.seterror( del.del_error, del.del_val1, del.del_RC ) ;
					break ;
				}

				check_delete( del ) ;
				if ( del.del_error != "" )
				{
					miBlock.seterror( del.del_error, del.del_val1, del.del_RC ) ;
					break ;
				}

				if ( !del.del_validCSR )
				{
					type = ( del.del_X )  ? "excluded " :
					       ( del.del_NX ) ? "non-excluded " : "" ;
					pcmd.set_msg( "PEDT011M", 4 ) ;
					miBlock.seterror( pcmd ) ;
					set_zedimsgs() ;
					pcmd.cond_reset() ;
					break ;
				}

				for ( n = 0, it = del.del_its ; it != del.del_ite && !it->is_bod() ; )
				{
					if ( it->il_deleted || it->is_tod() )
					{
						++it ;
						continue ;
					}
					if ( ( !del.del_X  && !del.del_NX ) ||
					     (  del.del_X  && it->is_excluded() ) ||
					     (  del.del_NX && it->is_not_excluded() ) )
					{
						if ( it->is_valid_file() ) { fileChanged = true ; }
						it = delete_line( it ) ;
						++n ;
					}
					else
					{
						++it ;
					}
				}

				if ( n == 0 )
				{
					type = ( del.del_X )  ? "excluded " :
					       ( del.del_NX ) ? "non-excluded " : "" ;
					pcmd.set_msg( "PEDT011M", 4 ) ;
					set_zedimsgs() ;
				}
				else
				{
					lines = d2ds( n ) ;
					pcmd.set_msg( "PEDT011N", 0 ) ;
					set_zedimsgs() ;
					rebuildZAREA = true ;
				}
				miBlock.seterror( pcmd ) ;
				set_zedimsgs() ;
				pcmd.cond_reset() ;
			}
			break ;

	case EM_DOWN:
			//
			// Scroll data down from the current position.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  2 No more data down.
			// RC =  4 No visible lines.
			// RC =  8 No data to display.
			// RC = 12 Amount not specified.
			// RC = 20 Severe error.
			//
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
			zscrolla = "" ;
			if ( zlvline == 0 )
			{
				miBlock.setRC( 4 ) ;
				break ;
			}
			if ( !getFileLineZFIRST() )
			{
				miBlock.setRC( 8 ) ;
				break ;
			}
			if ( datatype( miBlock.value, 'W' ) && miBlock.value.size() <= 8 )
			{
				zscrolln = ds2d( miBlock.value ) ;
				action_ScrollDown() ;
			}
			else if ( miBlock.value == "MAX" || miBlock.value == "M" )
			{
				zscrolla = "MAX" ;
				action_ScrollDown() ;
			}
			else if ( miBlock.value == "HALF" || miBlock.value == "H" )
			{
				zscrolln = zlvline / 2 ;
				action_ScrollDown() ;
			}
			else if ( miBlock.value == "PAGE" || miBlock.value == "P" )
			{
				zscrolln = zlvline ;
				action_ScrollDown() ;
			}
			else if ( miBlock.value == "DATA" || miBlock.value == "D" )
			{
				zscrolln = zlvline - 1 ;
				action_ScrollDown() ;
			}
			else if ( miBlock.value == "CURSOR" || miBlock.value == "CSR" )
			{
				if ( !mRow )
				{
					miBlock.setRC( 4 ) ;
					break ;
				}
				topLine = mRow ;
			}
			else if ( miBlock.value == "" )
			{
				miBlock.seterror( "PEDM012G", 12 ) ;
				break ;
			}
			else
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			if ( topLine->is_bod() )
			{
				miBlock.setRC( 2 ) ;
			}
			break ;

	case EM_END:
			//
			// End edit session.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  8 AUTOSAVE in VIEW mode.
			// RC = 12 End not done.  AUTOSAVE OFF PROMPT set.
			// RC = 20 Severe error.
			//
			if ( miBlock.get_sttment_words() > 1 )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			if ( zedvmode )
			{
				if ( fileChanged )
				{
					if ( profAutoSave )
					{
						miBlock.setRC( 8 ) ;
						set_zedimsgs( "PEDM014A" ) ;
					}
					else if ( profSaveP )
					{
						miBlock.setRC_noError( 12 ) ;
					}
					else
					{
						miBlock.eended = true ;
						termEdit       = true ;
					}
				}
				else
				{
					miBlock.eended = true ;
					termEdit       = true ;
				}
			}
			else if ( termOK() )
			{
				miBlock.eended = true ;
				termEdit       = true ;
			}
			else
			{
				miBlock.setRC_noError( 12 ) ;
			}
			break ;

	case EM_LABEL:
			//
			// Set label by relative line number or label.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  8 Label set but existing label at same level, deleted.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			if ( vw2 != "" )
			{
				if ( !isnumeric( vw2 ) )
				{
					miBlock.seterror( "PEDM011O", "Label level parameter must be numeric", 12 ) ;
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
			if ( getLinePtrIterator( iupper( kw1 ), it ) != 1 )
			{
				break ;
			}
			if ( !checkLabel1( vw1, n ) )
			{
				miBlock.seterror( "PEDT014", 20 ) ;
				break ;
			}
			if ( it->setLabel( vw1, level, n ) > 0 )
			{
				miBlock.setRC( 8 ) ;
			}
			for ( it1 = data.begin() ; it1 != data.end() ; ++it1 )
			{
				if ( it != it1 )
				{
					if ( it1->clearLabel( vw1, level, n ) )
					{
						break ;
					}
				}
			}
			break ;

	case EM_LEFT:
			//
			// Scroll data left from the current position.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 No visible lines.
			// RC =  8 No data to display.
			// RC = 12 Amount not specified.
			// RC = 20 Severe error.
			//
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
			zscrolla = "" ;
			if ( zlvline == 0 )
			{
				miBlock.setRC( 4 ) ;
				break ;
			}
			if ( !getFileLineZFIRST() )
			{
				miBlock.setRC( 8 ) ;
				break ;
			}
			if ( datatype( miBlock.value, 'W' ) && miBlock.value.size() <= 8 )
			{
				zscrolln = ds2d( miBlock.value ) ;
				action_ScrollLeft() ;
			}
			else if ( miBlock.value == "MAX" || miBlock.value == "M" )
			{
				zscrolla = "MAX" ;
				action_ScrollLeft() ;
			}
			else if ( miBlock.value == "HALF" || miBlock.value == "H" )
			{
				zscrolln = zdataw / 2 ;
				action_ScrollLeft() ;
			}
			else if ( miBlock.value == "PAGE" || miBlock.value == "P" )
			{
				zscrolln = zdataw ;
				action_ScrollLeft() ;
			}
			else if ( miBlock.value == "DATA" || miBlock.value == "D" )
			{
				zscrolln = zdataw - 1 ;
				action_ScrollLeft() ;
			}
			else if ( miBlock.value == "CURSOR" || miBlock.value == "CSR" )
			{
				if ( mCol == 0 )
				{
					miBlock.setRC( 4 ) ;
					break ;
				}
				zscrolln = zdataw - mCol + startCol - 1 ;
				if ( zscrolln > 0 )
				{
					action_ScrollLeft() ;
				}
				else if ( zscrolln < 0 )
				{
					zscrolln = abs( zscrolln ) ;
					action_ScrollRight() ;
				}
			}
			else if ( miBlock.value == "" )
			{
				miBlock.seterror( "PEDM012G", 12 ) ;
				break ;
			}
			else
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			break ;

	case EM_LINE:
			//
			// Update a line by relative line number or label.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Data truncated.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			// Data does not include any COBOL or STD line numbers.
			//
			if ( getLinePtrIterator( iupper( kw1 ), it ) != 1 )
			{
				break ;
			}
			t = formLineData( it ) ;
			if ( miBlock.fatal )
			{
				break ;
			}
			if ( reclen > 0 )
			{
				if ( reclen < ( t.size() + lnumSize ) )
				{
					miBlock.setRC( 4 ) ;
				}
				t.resize( ( reclen - lnumSize ), ' ' ) ;
			}
			else if ( t.size() > MAXLEN )
			{
				t.resize( MAXLEN ) ;
				miBlock.setRC( 4 ) ;
			}
			if ( it->put_idata( lnumSize1, lnumS2pos, t, level ) )
			{
				fileChanged = true ;
				it->resetFileStatus() ;
			}
			it->update_lnummod( lnumSize1, lnumS2pos, lnummod ) ;
			break ;

	case EM_LINE_AFTER:
	case EM_LINE_BEFORE:
			//
			// Add a line before/after the linenum or label.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Data truncated.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			// Data does not include any COBOL or STD line numbers.
			//
			rc = getLinePtrIterator( iupper( kw1 ), it ) ;
			if ( rc == 0 )
			{
				if ( miBlock.m_cmd == EM_LINE_AFTER )
				{
					it = data.begin() ;
					miBlock.setRC( 0 ) ;
				}
				else
				{
					break ;
				}
			}
			else if ( rc < 0 )
			{
				break ;
			}
			opt = upper( word( miBlock.value, 1 ) ) ;
			if ( findword( opt, "DATALINE INFOLINE MSGLINE NOTELINE" ) )
			{
				idelword( miBlock.value, 1, 1 ) ;
			}
			else
			{
				opt = "DATALINE" ;
			}
			t = formLineData( it ) ;
			if ( miBlock.fatal ) { break ; }
			if ( opt != "DATALINE" )
			{
				if ( reclen > 0 )
				{
					if ( reclen < t.size() )
					{
						miBlock.setRC( 4 ) ;
					}
					t.resize( reclen, ' ' ) ;
				}
				else if ( t.size() > MAXLEN )
				{
					t.resize( MAXLEN ) ;
					miBlock.setRC( 4 ) ;
				}
			}
			if ( miBlock.m_cmd == EM_LINE_AFTER )
			{
				it = getNextDataLine( it ) ;
			}
			if ( opt == "DATALINE" )
			{
				if ( reclen > 0 && reclen < ( t.size() + lnumSize ) )
				{
					miBlock.setRC( 4 ) ;
				}
				if ( lnumSize1 > 0 )
				{
					t = string( lnumSize1, ' ' ) + t ;
				}
				if ( lnumSize2 > 0 )
				{
					t.resize( ( lnumS2pos + lnumSize2 ), ' ' ) ;
				}
				if ( reclen > 0 )
				{
					t.resize( reclen, ' ' ) ;
				}
				else if ( t.size() > MAXLEN )
				{
					t.resize( MAXLEN ) ;
					miBlock.setRC( 4 ) ;
				}
				it = data.insert( it, new iline( taskid(), LN_FILE, lnumSize ) ) ;
				fileChanged = true ;
			}
			else if ( opt == "INFOLINE" )
			{
				it = data.insert( it, new iline( taskid(), LN_INFO ) ) ;
			}
			else if ( opt == "MSGLINE"  )
			{
				it = data.insert( it, new iline( taskid(), LN_MSG ) ) ;
			}
			else
			{
				it = data.insert( it, new iline( taskid(), LN_NOTE ) ) ;
			}
			it->put_idata( t, level ) ;
			break ;

	case EM_INSERT:
			//
			// Insert a number of lines after the linenum or label.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			vw2 = word( miBlock.sttment, 2 ) ;
			vw3 = word( miBlock.sttment, 3 ) ;
			if ( miBlock.get_sttment_words() > 3 ||
			     vw2 == ""                       ||
			   ( vw3 != "" && !isnumeric( vw3 ) ) )
			{
				miBlock.seterror( "PEDM011P", 12 ) ;
				break ;
			}
			if ( getLinePtrIterator( iupper( vw2 ), it ) < 0 )
			{
				break ;
			}
			else if ( it == nullptr )
			{
				it = data.begin() ;
				miBlock.setRC( 0 ) ;
			}
			cmd.lcmd_cmd   = LC_I ;
			cmd.lcmd_sADDR = itr2ptr( it ) ;
			cmd.lcmd_Rpt   = ( vw3 == "" ) ? 1 : ds2d( vw3 ) ;
			actionLineCommand( cmd, unused, unused ) ;
			break ;

	case EM_MACRO:
			//
			// Identify command as a macro
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  8 No parameters are permitted.
			// RC = 12 Syntax error.
			// RC = 20 Severe error.
			//
			if ( miBlock.nestlvl == 1 )
			{
				level = iline::get_Global_Undo_level( taskid() ) + 1 ;
			}
			t = ( miBlock.nestlvl == 1 && miBlock.editserv ) ? edmparm : miBlock.parms ;
			if ( t != "" && miBlock.keyopts == "" )
			{
				miBlock.setRC( 8 ) ;
			}
			else
			{
				int ws = words( miBlock.keyopts ) ;
				if ( ws > 0 )
				{
					qstrings( t, vstring, ws ) ;
					if ( miBlock.fatal )
					{
						break ;
					}
					for ( int i = 1 ; i <= ws ; ++i )
					{
						t1 = word( miBlock.keyopts, i ) ;
						if ( i <= vstring.size() )
						{
							macAppl->vreplace( t1, vstring[ i-1 ] ) ;
						}
						else
						{
							macAppl->vreplace( t1, "" ) ;
						}
					}
				}
			}
			if ( miBlock.process )
			{
				if ( !miBlock.processed && !miBlock.imacro && !miBlock.lcmacro() )
				{
					updateData() ;
					if ( pcmd.error() )
					{
						miBlock.seterror( pcmd ) ;
						pcmd.cond_reset() ;
						break ;
					}
					processNewInserts() ;
					actionLineCommands() ;
					if ( pcmd.error() )
					{
						miBlock.seterror( pcmd ) ;
						pcmd.cond_reset() ;
						break ;
					}
				}
				miBlock.processed = true ;
			}
			break ;

	case EM_MACRO_MSG:
			//
			// Set the value of MACRO_MSG to control setting of lspf messages.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			if ( miBlock.value == "ON" )
			{
				miBlock.messageOn = true ;
			}
			else if ( miBlock.value == "OFF" )
			{
				miBlock.messageOn = false ;
			}
			else
			{
				miBlock.seterror( "PEDM011N", 20 ) ;
			}
			break ;

	case EM_MOVE:
			//
			// Move data from a file into the data being edited.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Invalid line pointer (linenum or label).  File not found.
			// RC = 20 Severe error (syntax error or I/O error).
			//
			t = upper( miBlock.value ) ;
			n = wordpos( "BEFORE", t ) ;
			if ( n == 0 )
			{
				n = wordpos( "AFTER", t ) ;
				if ( n == 0 )
				{
					miBlock.seterror( "PEDM012G", 20 ) ;
					break ;
				}
				before = false ;
			}
			head = strip( subword( miBlock.value, 1, ( n - 1 ) ) ) ;
			tail = upper( subword( miBlock.value, ( n + 1 ) ) ) ;
			ws   = words( tail ) ;
			if ( ws > 1 )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			if ( ws == 0 )
			{
				miBlock.seterror( "PEDM012G", 20 ) ;
				break ;
			}
			if ( getLinePtrIterator( upper( strip( tail ) ), it1 ) != 1 )
			{
				break ;
			}
			t = expandFileName( head ) ;
			if ( !exists( t ) )
			{
				miBlock.seterror( "PEDM014B", t, 12 ) ;
				break ;
			}
			copyIn( t, ( before ) ? --it1 : it1 ) ;
			if ( miBlock.ok() )
			{
				try
				{
					remove( t ) ;
				}
				catch ( boost::filesystem::filesystem_error &e )
				{
					miBlock.seterror( "PEDT018Q", t, e.what(), 12 ) ;
				}
			}
			break ;

	case EM_PROCESS:
			//
			// Process line commands and data changes.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 RANGE expected but none specified - defaults set.
			// RC =  8 DEST expected but none specified - defaults set.
			// RC = 12 RANGE and DEST expected but none specified - defaults set.
			// RC = 16 Incomplete or conflicting commands entered.
			// RC = 20 Severe error.
			//
			// DEST and RANGE function differently for line command macros.
			//
			if ( miBlock.processed )
			{
				miBlock.seterror( "PEDM012K", 20 ) ;
				break ;
			}
			miBlock.processed = true ;
			t = upper( miBlock.sttment ) ;
			if ( word( t, 2 ) == "DEST" )
			{
				idelword( t, 2, 1 ) ;
				if ( miBlock.lcmacro() )
				{
					auto itx = lmacs.find( miBlock.lcmd ) ;
					if ( itx->second.lcdst )
					{
						for ( const auto& lc : lcmds )
						{
							if ( !lc.lcmd_procd && lc.lcmd_lcname == itx->second.lcname )
							{
								if ( lc.lcmd_dADDR )
								{
									zdest = lc.lcmd_dADDR ;
									if ( lc.lcmd_ABOW == 'B' )
									{
										zdest = getPrevDataLine( zdest ) ;
									}
								}
								else
								{
									miBlock.setRC( 8 ) ;
								}
								break ;
							}
						}
					}
				}
				else
				{
					rc = set_zdest() ;
					if ( miBlock.fatal )
					{
						zdest = nullptr ;
						break ;
					}
					miBlock.setRC_noError( rc ) ;
				}
			}
			if ( words( t ) > 4 )
			{
				miBlock.seterror( "PEDM011P", 12 ) ;
				break ;
			}
			vw1 = word( t, 2 ) ;
			vw2 = word( t, 3 ) ;
			vw3 = word( t, 4 ) ;
			if ( vw1 == "RANGE" )
			{
				if ( miBlock.lcmacro() )
				{
					if ( vw2 != "" )
					{
						t1         = subword( t, 3 ) ;
						zrange_cmd = "" ;
						for ( itc = lcmds.begin() ; itc != lcmds.end() && !zfrange ; ++itc )
						{
							if ( !itc->lcmd_procd && wordpos( itc->lcmd_lcname, t1 ) > 0 )
							{
								zrange_cmd = itc->lcmd_lcname ;
								zfrange    = itc->lcmd_sADDR ;
								zlrange    = itc->lcmd_eADDR ;
							}
						}
					}
					else
					{
						miBlock.seterror( "PEDM013G", 20 ) ;
						break ;
					}
					if ( !zfrange )
					{
						miBlock.addRC_noError( 4 ) ;
					}
				}
				else
				{
					rc = set_zranges( vw2, vw3 ) ;
					if ( miBlock.fatal )
					{
						zrange_cmd = "" ;
						zfrange    = nullptr ;
						zlrange    = nullptr ;
						break ;
					}
					miBlock.addRC_noError( rc ) ;
				}
			}
			else if ( vw1 != "" )
			{
				miBlock.seterror( "PEDM013F", 20 ) ;
				break ;
			}
			if ( !miBlock.imacro && !miBlock.lcmacro() )
			{
				updateData() ;
				if ( pcmd.error() )
				{
					miBlock.seterror( pcmd ) ;
					pcmd.cond_reset() ;
					break ;
				}
				processNewInserts() ;
				actionLineCommands() ;
				if ( pcmd.error() )
				{
					miBlock.seterror( pcmd ) ;
					pcmd.cond_reset() ;
					break ;
				}
			}
			break ;

	case EM_MASKLINE:
			//
			// Set the MASK line.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Data truncated.
			// RC =  8 Variable data truncated.
			// RC = 20 Severe error.
			//
			t = formLineData() ;
			if ( miBlock.fatal ) { break ; }
			maskLine = t ;
			if ( reclen > 0 )
			{
				if ( maskLine.size() > reclen )
				{
					miBlock.setRC( 4 ) ;
				}
				maskLine.resize( reclen, ' ' ) ;
			}
			break ;

	case EM_PATH:
			//
			// Set the backup location.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  8 Path does not exist.  Change ignored.
			// RC = 20 Severe error.
			//
			try
			{
				if ( exists( miBlock.value ) && is_directory( miBlock.value ) )
				{
					backupLoc = miBlock.value ;
					if ( backupLoc.back() != '/' ) { backupLoc += '/' ; }
				}
				else
				{
					miBlock.setRC( 8 ) ;
				}
			}
			catch ( const filesystem_error& ex )
			{
				vreplace( "ZVAL1", ex.what() ) ;
				miBlock.seterror( "PSYS012C", 20 ) ;
			}
			break ;

	case EM_RCHANGE:
			//
			// Repeat CHANGE.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 String not found.
			// RC =  8 Change error.  String2 longer than string1 and substitution
			//         was not performed on at least one occation.
			// RC = 12 Syntax error.
			// RC = 20 Severe error.
			//
			if ( !fcx_parms.f_cset )
			{
				miBlock.seterror( "PEDT013N", 20 ) ;
				break ;
			}
			fcx_parms.set_change_counts( 0, 0 ) ;
			actionFind() ;
			if ( fcx_parms.f_error )
			{
				miBlock.seterror( pcmd ) ;
				set_zedimsgs() ;
				pcmd.cond_reset() ;
				break ;
			}
			if ( !fcx_parms.f_success )
			{
				if ( miBlock.messageOn )
				{
					setNotFoundMsg() ;
					set_zedimsgs() ;
				}
				miBlock.setRC( 4 ) ;
				break ;
			}
			setCursorFindChange() ;
			actionChange() ;
			if ( fcx_parms.f_error )
			{
				fcx_parms.f_ch_errs = 1 ;
				setChangedError() ;
				miBlock.seterror( pcmd, fcx_parms.f_rc ) ;
				set_zedimsgs() ;
				pcmd.cond_reset() ;
				rebuildZAREA = true ;
				break ;
			}
			fcx_parms.f_ch_occs = 1 ;
			setCursorChange() ;
			moveTopline( fcx_parms.f_ADDR ) ;
			if ( miBlock.messageOn )
			{
				setChangedMsg() ;
				set_zedimsgs() ;
			}
			break ;

	case EM_RFIND:
			//
			// Repeat FIND or EXCLUDE.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 String not found.
			// RC = 12 Syntax error.
			// RC = 20 Severe error (string not defined).
			//
			if ( !fcx_parms.f_fset )
			{
				miBlock.seterror( "PEDT013M", 20 ) ;
				break ;
			}
			tCol = mCol ;
			if ( fcx_parms.f_reverse() && mCol > 0 && fcx_parms.f_suffix() )
			{
				--mCol ;
			}
			if ( fcx_parms.f_exclude && mRow && mRow->is_excluded() )
			{
				mCol = ( fcx_parms.f_reverse() ) ? 1 : mRow->get_idata_len() ;
			}

			actionFind() ;
			if ( fcx_parms.f_error )
			{
				miBlock.seterror( pcmd ) ;
				set_zedimsgs() ;
				pcmd.cond_reset() ;
				break ;
			}
			if ( pcmd.msgset() )
			{
				set_zedimsgs() ;
			}
			if ( fcx_parms.f_success )
			{
				if ( fcx_parms.f_exclude )
				{
					fcx_parms.set_exclude_counts( 1, 1 ) ;
				}
				else
				{
					fcx_parms.set_find_counts( 1, 1 ) ;
				}
				setCursorFind() ;
				if ( miBlock.messageOn )
				{
					setFoundMsg() ;
					set_zedimsgs() ;
				}
			}
			else
			{
				mCol = tCol ;
				if ( fcx_parms.f_exclude )
				{
					fcx_parms.set_exclude_counts( 0, 0 ) ;
				}
				else
				{
					fcx_parms.set_find_counts( 0, 0 ) ;
				}
				miBlock.setRC( 4 ) ;
				if ( miBlock.messageOn )
				{
					setNotFoundMsg() ;
					set_zedimsgs() ;
				}
			}
			break ;

	case EM_RIGHT:
			//
			// Scroll data right from the current position.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  4 No visible lines.
			// RC =  8 No data to display.
			// RC = 12 Amount not specified.
			// RC = 20 Severe error.
			//
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
			zscrolla = "" ;
			if ( zlvline == 0 )
			{
				miBlock.setRC( 4 ) ;
				break ;
			}
			if ( !getFileLineZFIRST() )
			{
				miBlock.setRC( 8 ) ;
				break ;
			}
			if ( datatype( miBlock.value, 'W' ) && miBlock.value.size() <= 8 )
			{
				zscrolln = ds2d( miBlock.value ) ;
				action_ScrollRight() ;
			}
			else if ( miBlock.value == "MAX" || miBlock.value == "M" )
			{
				zscrolla = "MAX" ;
				action_ScrollRight() ;
			}
			else if ( miBlock.value == "HALF" || miBlock.value == "H" )
			{
				zscrolln = zdataw / 2 ;
				action_ScrollRight() ;
			}
			else if ( miBlock.value == "PAGE" || miBlock.value == "P" )
			{
				zscrolln = zdataw ;
				action_ScrollRight() ;
			}
			else if ( miBlock.value == "DATA" || miBlock.value == "D" )
			{
				zscrolln = zdataw - 1 ;
				action_ScrollRight() ;
			}
			else if ( miBlock.value == "CURSOR" || miBlock.value == "CSR" )
			{
				if ( mCol == 0 )
				{
					miBlock.setRC( 4 ) ;
					break ;
				}
				zscrolln = mCol - startCol ;
				if ( zscrolln > 0 )
				{
					action_ScrollRight() ;
				}
				else if ( zscrolln < 0 )
				{
					zscrolln = abs( zscrolln ) ;
					action_ScrollLeft() ;
				}
			}
			else if ( miBlock.value == "" )
			{
				miBlock.seterror( "PEDM012G", 12 ) ;
				break ;
			}
			else
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			break ;

	case EM_SAVE:
			//
			// Save data.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.  Data saved.
			// RC =  8 Save entered in VIEW mode.
			// RC = 20 Severe error.
			//
			if ( zedvmode )
			{
				miBlock.setRC( 8 ) ;
				set_zedimsgs( "PEDM014A" ) ;
			}
			else if ( saveFile() )
			{
				level = iline::get_Global_Undo_level( taskid() ) + 1 ;
				set_zedimsgs( "PEDT011P" ) ;
			}
			else
			{
				miBlock.seterror( pcmd, 20 ) ;
			}
			break ;

	case EM_SAVE_LENGTH:
			//
			// Set data length
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.  Data saved.
			// RC =  6 Record format is not variable.
			// RC = 16 Error setting variable.
			// RC = 20 Severe error.
			//
			if ( !datatype( vw1, 'W' ) )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			if ( getLinePtrIterator( iupper( kw1 ), it ) != 1 )
			{
				break ;
			}
			it->set_idata_minsize( ds2d( vw1 ) ) ;
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
				miBlock.seterror( "PEDM011N", 12 ) ;
			}
			break ;

	case EM_SHIFT:
			vw1 = word( miBlock.sttment, 2 ) ;
			vw2 = word( miBlock.sttment, 3 ) ;
			vw3 = word( miBlock.sttment, 4 ) ;
			if ( miBlock.get_sttment_words() > 4 ||
			     !findword( vw1, "( ) < > [ ]" ) ||
			     vw2 == ""                       ||
			   ( vw3 != "" && !isnumeric( vw3 ) ) )
			{
				miBlock.seterror( "PEDM011P", 12 ) ;
				break ;
			}
			if ( getLinePtrIterator( iupper( vw2 ), it ) != 1 )
			{
				break ;
			}
			cmd.lcmd_cmd   = lineCmds.at( vw1 ) ;
			cmd.lcmd_sADDR = itr2ptr( it ) ;
			cmd.lcmd_eADDR = cmd.lcmd_sADDR ;
			cmd.lcmd_Rpt   = ( vw3 == "" ) ? ( findword( vw1, "[ ]" ) ) ? 1 : 2 : ds2d( vw3 ) ;
			actionLineCommand( cmd, unused, unused ) ;
			break ;

	case EM_TFLOW:
			//
			// Restructure paragraphs.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC = 12 Invalid line number.
			// RC = 20 Severe error.
			//
			if ( miBlock.get_sttment_words() < 2 )
			{
				miBlock.seterror( "PEDM013A", "TFLOW", 20 ) ;
				break ;
			}
			vw1 = word( miBlock.sttment, 2 ) ;
			vw2 = word( miBlock.sttment, 3 ) ;
			if ( miBlock.get_sttment_words() > 3  ||
			   ( vw2 != "" && ( !isnumeric( vw2 ) || vw2.size() > 9 ) ) )
			{
				miBlock.seterror( "PEDM011P", 12 ) ;
				break ;
			}
			if ( vw2 == "" && RightBnd == 0 )
			{
				miBlock.seterror( "PEDM012U", 20 ) ;
				break ;
			}
			if ( getLinePtrIterator( iupper( vw1 ), it ) != 1 )
			{
				break ;
			}
			cmd.lcmd_cmd   = LC_TFLOW ;
			cmd.lcmd_sADDR = itr2ptr( it ) ;
			cmd.lcmd_Rpt   = ( vw2 == "" ) ? RightBnd : ds2d( vw2 ) ;
			actionLineCommand( cmd, unused, unused ) ;
			break ;

	case EM_TABSLINE:
			//
			// Set the TABSLINE.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  4 Data truncated.
			// RC =  8 Invalid data detected and ignored.
			// RC = 20 Severe error.
			//
			t = formLineData() ;
			if ( miBlock.fatal ) { break ; }
			tabsLine = t ;
			if ( reclen > 0 )
			{
				if ( tabsLine.size() > reclen )
				{
					miBlock.setRC( 4 ) ;
				}
				tabsLine.resize( reclen, ' ' ) ;
			}
			for ( n = 0 ; n < tabsLine.size() ; ++n )
			{
				char& c = tabsLine[ n ] ;
				if ( c == '_' )
				{
					c = '-' ;
				}
				else if ( c != ' ' && c != '*' && c != '-' )
				{
					c = ' ' ;
					miBlock.setRC( 8 ) ;
				}
			}
			break ;

	case EM_UP:
			//
			// Scroll data up from the current position.
			//
			// MACRO return codes (Command syntax only):
			// RC =  0 Normal completion.
			// RC =  2 No more data up.
			// RC =  4 No visible lines.
			// RC =  8 No data to display.
			// RC = 12 Amount not specified.
			// RC = 20 Severe error.
			//
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
			zscrolla = "" ;
			if ( zlvline == 0 )
			{
				miBlock.setRC( 4 ) ;
				break ;
			}
			if ( !getFileLineZFIRST() )
			{
				miBlock.setRC( 8 ) ;
				break ;
			}
			if ( datatype( miBlock.value, 'W' ) && miBlock.value.size() <= 8 )
			{
				zscrolln = ds2d( miBlock.value ) ;
				action_ScrollUp() ;
			}
			else if ( miBlock.value == "MAX" || miBlock.value == "M" )
			{
				zscrolla = "MAX" ;
				action_ScrollUp() ;
			}
			else if ( miBlock.value == "HALF" || miBlock.value == "H" )
			{
				zscrolln = zlvline / 2 ;
				action_ScrollUp() ;
			}
			else if ( miBlock.value == "PAGE" || miBlock.value == "P" )
			{
				zscrolln = zlvline ;
				action_ScrollUp() ;
			}
			else if ( miBlock.value == "DATA" || miBlock.value == "D" )
			{
				zscrolln = zlvline - 1 ;
				action_ScrollUp() ;
			}
			else if ( miBlock.value == "CURSOR" || miBlock.value == "CSR" )
			{
				if ( !mRow )
				{
					miBlock.setRC( 4 ) ;
					break ;
				}
				topLine  = mRow ;
				zscrolln = zlvline - 1 ;
				action_ScrollUp() ;
			}
			else if ( miBlock.value == "" )
			{
				miBlock.seterror( "PEDM012G", 12 ) ;
				break ;
			}
			else
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			if ( topLine->is_tod() )
			{
				miBlock.setRC( 2 ) ;
			}
			break ;

	case EM_USER_STATE:
			//
			// Restore a saved user state.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC = 20 Severe error.
			//
			{
				auto it = user_states.find( miBlock.value ) ;
				if ( it == user_states.end() )
				{
					miBlock.seterror( "PEDM011V", 20 ) ;
					break ;
				}

				user_state* u = it->second ;
				if ( profAutoNum != u->ustate_prof.profAutoNum )
				{
					pcmd.set_cmd( "AUTONUM", OnOff[ u->ustate_prof.profAutoNum ], defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profAutoSave != u->ustate_prof.profAutoSave || profSaveP != u->ustate_prof.profSaveP )
				{
					t = OnOff[ u->ustate_prof.profAutoSave ] +
					  ( ( u->ustate_prof.profAutoSave ) ? "" :
					    ( u->ustate_prof.profSaveP )    ? " PROMPT" : " NOPROMPT" ) ;
					pcmd.set_cmd( "AUTOSAVE", t, defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profCaps != u->ustate_prof.profCaps )
				{
					pcmd.set_cmd( "CAPS", OnOff[ u->ustate_prof.profCaps ], defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profHex != u->ustate_prof.profHex || profVert != u->ustate_prof.profVert )
				{
					t = OnOff[ u->ustate_prof.profHex ] +
					  ( ( u->ustate_prof.profHex && u->ustate_prof.profVert ) ? " VERT" : "" ) ;
					pcmd.set_cmd( "HEX", t, defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profIMACRO != u->ustate_prof.profIMACRO )
				{
					pcmd.set_cmd( "IMACRO", u->ustate_prof.profIMACRO, defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profNotes != u->ustate_prof.profNotes )
				{
					pcmd.set_cmd( "NOTES", OnOff[ u->ustate_prof.profNotes ], defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profNulls != u->ustate_prof.profNulls || profNullA != u->ustate_prof.profNullA )
				{
					t = OnOff[ u->ustate_prof.profNulls ] +
					  ( ( u->ustate_prof.profNulls ) ?
					    ( u->ustate_prof.profNullA ) ? " ALL" : " STD" : "" ) ;
					pcmd.set_cmd( "NULLS", t, defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profNum    != u->ustate_prof.profNum    ||
				     profNumSTD != u->ustate_prof.profNumSTD ||
				     profNumCBL != u->ustate_prof.profNumCBL )
				{
					t = OnOff[ u->ustate_prof.profNum ] +
					  ( ( u->ustate_prof.profNum ) ? ( u->ustate_prof.profNumSTD ) ? " STD" :
									 ( u->ustate_prof.profNumCBL ) ? " COBOL" : "" : "" ) ;
					pcmd.set_cmd( "NUMBER", t, defNames ) ;
					actionPrimCommand3() ;
				}

				if ( zedprof != u->ustate_prof.profName )
				{
					pcmd.set_cmd( "PROFILE", u->ustate_prof.profName, defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profStats != u->ustate_prof.profStats )
				{
					pcmd.set_cmd( "STATS", OnOff[ u->ustate_prof.profStats ], defNames ) ;
					actionPrimCommand3() ;
				}

				if ( profTabs  != u->ustate_prof.profTabs ||
				     profATabs != u->ustate_prof.profATabs )
				{
					t = OnOff[ u->ustate_prof.profTabs ] +
					  ( ( u->ustate_prof.profTabs ) ? ( u->ustate_prof.profATabs ) ? " ALL" : " STD" : "" ) ;
					pcmd.set_cmd( "TABS", t, defNames ) ;
					actionPrimCommand3() ;
				}
				rc   = isValidiline( u->ustate_mRow1 ) ;
				mRow = ( rc == 0 ) ? u->ustate_mRow1 :
				       ( rc == 1 ) ? getNextFileLine( u->ustate_mRow1 ) :
						     itr2ptr( getDataLine( u->ustate_mRow2 ) ) ;
				mCol     = u->ustate_mCol ;
				maskLine = u->ustate_maskline ;
				tabsLine = u->ustate_tabline ;
				LeftBnd  = u->ustate_lbounds ;
				RightBnd = u->ustate_rbounds ;
				rc       = isValidiline( u->ustate_topline1 ) ;
				topLine  = ( rc == 0 ) ? u->ustate_topline1 :
					   ( rc == 1 ) ? getNextDataLine( u->ustate_topline1 ) :
							 itr2ptr( getDataLine( u->ustate_topline2 ) ) ;
				miBlock.cursorset = u->ustate_cursorset ;

				if ( u->ustate_find.f_fset )
				{
					fcx_parms = u->ustate_find ;
					Global_efind_parms[ ds2d( zscrnum ) ] = fcx_parms ;
				}
				else
				{
					fcx_parms = edit_find() ;
				}
			}
			break ;

	case EM_XSTATUS:
			//
			// Set the exclude status of a data line at linenum or label.
			//
			// MACRO return codes (Assignment syntax only):
			// RC =  0 Normal completion.
			// RC =  8 Line command pending.
			// RC = 12 Line number does not exist.
			// RC = 20 Severe error.
			//
			if ( vw1 != "X" && vw1 != "NX" )
			{
				miBlock.seterror( "PEDM011P", 20 ) ;
				break ;
			}
			if ( getLinePtrIterator( iupper( kw1 ), it ) != 1 )
			{
				break ;
			}
			addr = itr2ptr( it ) ;
			okay = true ;
			for ( const auto& lc : lcmds )
			{
				if ( !lc.lcmd_procd )
				{
					if ( lc.lcmd_sADDR == addr ||
					     lc.lcmd_eADDR == addr ||
					     lc.lcmd_dADDR == addr ||
					     lc.lcmd_lADDR == addr )
					{
						miBlock.setRC( 8 ) ;
						okay = false ;
						break ;
					}
				}
			}
			if ( okay )
			{
				it->set_excluded( ( vw1 == "X" ), level ) ;
			}
			break ;

	case EM_LOCATE:
			if ( miBlock.imacro )
			{
				miBlock.seterror( "PEDM013E", 20 ) ;
				break ;
			}
	case EM_AUTONUM:
	case EM_AUTOSAVE:
	case EM_BACKUP:
	case EM_BOUNDS:
	case EM_BUILTIN:
	case EM_CAPS:
	case EM_CHANGE:
	case EM_COMPARE:
	case EM_DEFINE:
	case EM_EXCLUDE:
	case EM_FIND:
	case EM_FLIP:
	case EM_HEX:
	case EM_HIDE:
	case EM_HILITE:
	case EM_IMACRO:
	case EM_NOTES:
	case EM_NULLS:
	case EM_NUMBER:
	case EM_NONUMBER:
	case EM_PROFILE:
	case EM_RECOVERY:
	case EM_RENUM:
	case EM_RESET:
	case EM_SEEK:
	case EM_SETUNDO:
	case EM_SORT:
	case EM_STATS:
	case EM_TABS:
	case EM_UNNUMBER:
	case EM_XTABS:
			pcmd.set_cmd( miBlock.keyword, miBlock.value, defNames ) ;
			if ( pcmd.ok() )
			{
				actionPrimCommand2() ;
				if ( pcmd.ok() && pcmd.not_actioned() )
				{
					actionPrimCommand3() ;
					if ( pcmd.not_actioned() )
					{
						macAppl->vreplace( "ZVAL2", miBlock.emacro ) ;
						miBlock.seterror( "PEDM012D", pcmd.get_cmd(), 12 ) ;
						break ;
					}
				}
			}
			miBlock.seterror( pcmd ) ;
			set_zedimsgs() ;
			pcmd.cond_reset() ;
			break ;

	default:
			miBlock.seterror( "PEDM011A", 12 ) ;
	}
}


Data::Iterator pedit01::delete_line( Data::Iterator it,
				     bool perm )
{
	//
	// Logically delete a line if UNDO is on, or physically delete it and remove from the data container if not.
	//
	// If we are deleting topLine, modLine, aADDR or cursor position, reposition to the next line.
	// (ptopline and misADDR can be set to NULL).
	//
	// If macro running, reposition mRow and set zdest, zfrange, zlrange to NULL.
	//
	// Return an iterator to the entry after the one being deleted.
	//

	TRACE_FUNCTION() ;

	iline* deladdr = itr2ptr( it ) ;

	if ( deladdr == topLine )
	{
		topLine = getNextDataLine( topLine ) ;
	}

	if ( deladdr == modLine )
	{
		modLine = getNextDataLine( modLine ) ;
	}

	if ( cursor.addr( deladdr ) )
	{
		cursor.replace( deladdr, getNextDataLine( deladdr ) ) ;
	}

	if ( deladdr == aADDR )
	{
		aADDR = getNextDataLine( aADDR ) ;
	}

	if ( deladdr == ptopLine )
	{
		ptopLine = nullptr ;
	}

	if ( deladdr == misADDR )
	{
		misADDR = nullptr ;
	}

	if ( macroRunning )
	{
		if ( mRow == deladdr )
		{
			mRow = getNextFileLine( mRow ) ;
			if ( !mRow )
			{
				mRow = getFileLineZLAST() ;
			}
		}
		if ( zdest == deladdr )
		{
			zdest = nullptr ;
		}
		if ( zfrange == deladdr )
		{
			zfrange = nullptr ;
		}
		if ( zlrange == deladdr )
		{
			zlrange = nullptr ;
		}
	}

	assert( deladdr != data.bottom() && deladdr != data.top() ) ;

	if ( profUndoOn && !perm )
	{
		it->set_deleted( level ) ;
		++it ;
	}
	else
	{
		it = data.erase( it ) ;
	}

	return it ;
}


void pedit01::iincr( iline*& l )
{
	//
	// Advance 1 line from line l.
	//

	l = l->il_next ;
}


void pedit01::idecr( iline*& l )
{
	//
	// Back 1 line from line l.
	//

	l = l->il_prev ;
}
