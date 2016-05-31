/*  Compile with ::                                                                                    */
/* g++  -shared -fPIC -std=c++11 -Wl,-soname,libPEDIT01.so -lboost_regex  -o libPEDIT01.so PEDIT01.cpp */
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

/* PDF-like editor program                                                                   */
/*                                                                                           */
/* Current status:                                                                           */
/* Most basic functions work okay                                                            */
/*                                                                                           */
/*                                                                                           */
/* RC/RSN codes returned                                                                     */
/*  0/0   Okay - No saves made                                                               */
/*  0/4   Okay - Data saved                                                                  */
/*  0/8   Okay - Data saved to an alternate name due to an abnormal termination              */
/*  4/0   File is currently being edited.  Edit aborted.                                     */
/*  8/4   Cannot use edit on file.  Browse instead                                           */
/*  8/8   Error saving data                                                                  */

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <vector>
#include "../lspf.h"
#include "../utilities.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "eHilight.cpp"
#include "PEDIT01.h"


using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PEDIT01

#define CLINESZ     8
#define maxURID     iline::maxURID
#define Global_Undo iline::Global_Undo
#define Global_Redo iline::Global_Redo
#define undoON      iline::setUNDO[ taskid() ]
#define Global_File_level iline::Global_File_level

map<int,int> maxURID ;
map<int, bool> iline::setUNDO    ;
map<int, stack<int>>Global_Undo  ;
map<int, stack<int>>Global_Redo  ;
map<int, stack<int>>Global_File_level ;

map<string,bool>PEDIT01::EditList ;


void PEDIT01::application()
{
	log( "I", "Application PEDIT01 starting.  Parms are "+ PARM << endl ) ;

	bool result ;

	string * pt ;

	string panel("") ;
	string rfile     ;

	initialise() ;

	if ( PARM == "ENTRYPNL" )
	{
		vcopy( "ZEDABRC", ZFILE, MOVE ) ;
		if ( ZFILE != "" )
		{
			rfile = ZFILE ;
			ZFILE = ZFILE.erase( ZFILE.size()-6 ) ;
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
				}
			}
		}
		while ( true )
		{
			showEditEntry() ;
			if ( ZRC > 0 ) { break ; }
			vcopy( "SHOWDIR", pt, LOCATE ) ;
			if ( (*pt) == "YES" )
			{
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
		ZFILE = parseString( result, PARM, "FILE()" ) ;
		if ( !result || ZFILE == "" )
		{
			log( "E", "Invalid parameter format passed to PEDIT01" << endl ; )
			abend() ;
			return  ;
		}
		panel = parseString( result, PARM, "PANEL()" ) ;
		if ( !result )
		{
			log( "E", "Invalid parameter format passed to PEDIT01" << endl ; )
			abend() ;
			return  ;
		}
		Edit() ;
	}
	cleanup() ;
}


void PEDIT01::initialise()
{
	control( "ABENDRTN", static_cast<void (pApplication::*)()>(&PEDIT01::cleanup_custom) ) ;

	vdefine( "ZCMD  ZVERB   ZROW1  ZROW2", &ZCMD, &ZVERB, &ZROW1, &ZROW2 ) ;
	vdefine( "ZAREA ZSHADOW ZAREAT ZFILE", &ZAREA, &ZSHADOW, &ZAREAT, &ZFILE ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &ZAREAW, &ZAREAD ) ;
	vdefine( "ZSCROLLA ZCOL1", &ZSCROLLA, &ZCOL1 ) ;
	vdefine( "ZAPPLID  ZEDPROF ZHOME", &ZAPPLID, &ZEDPROF, &ZHOME ) ;
	vdefine( "TYPE STR OCC LINES", &TYPE, &STR, &OCC, &LINES ) ;
	vdefine( "EETABCC EETABSS EESTSPC", &EETABCC, &EETABSS, &EESTSPC ) ;

	vget( "ZEDPROF", PROFILE ) ;
	vget( "ZAPPLID ZHOME", SHARED ) ;

	pquery( "PEDIT012", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 ) { abend() ; }

	if ( ZEDPROF == "" ) { ZEDPROF = "DEFAULT" ; }

	typList[ 'C' ] = "CHARS"  ;
	typList[ 'P' ] = "PREFIX" ;
	typList[ 'S' ] = "SUFFIX" ;
	typList[ 'W' ] = "WORD"   ;

	ZDATAW = ZAREAW - CLINESZ ;
	ZASIZE = ZAREAW * ZAREAD  ;

	sdg.assign( ZDATAW, N_GREEN )   ;
	sdy.assign( ZDATAW, N_YELLOW )  ;
	sdw.assign( ZDATAW, N_WHITE )   ;

	sdr.assign( ZDATAW, N_RED )     ;
	sdb.assign( ZDATAW, B_BLUE )    ;

	slg.assign( CLINESZ, N_GREEN )  ;
	sly.assign( CLINESZ, N_YELLOW ) ;
	slw.assign( CLINESZ, N_WHITE )  ;
	slr.assign( CLINESZ, N_RED )    ;
	slb.assign( CLINESZ, B_BLUE )   ;

	sdrh.assign( ZDATAW,  B_RED )    ;
	sdyh.assign( ZDATAW,  B_YELLOW ) ;

	slrh.assign( CLINESZ, B_RED )    ;
	slyh.assign( CLINESZ, B_YELLOW ) ;

	div.assign( ZAREAW-1, '-' )      ;

	clipboard = "DEFAULT"  ;
	CLIPTABL  = "EDITCLIP" ;

	OnOff[ false ] = "OFF" ;
	OnOff[ true  ] = "ON"  ;

	ZeroOne[ false ] = '0' ;
	ZeroOne[ true  ] = '1' ;

	maxURID[ taskid() ] = 0 ;
	abendRecovery = false ;

	ZAHELP = "HEDIT01" ;
}


void PEDIT01::Edit()
{
	string t      ;
	bool termEdit ;

	RC  = 0  ;
	MSG = "" ;

	getEditProfile( ZEDPROF ) ;
	readFile() ;
	if ( RC > 0 )
	{
		if ( MSG == "" ) { MSG = "PSYS01E" ; }
		setmsg( MSG ) ;
		return ;
	}

	CURFLD  = "ZCMD" ;
	CURPOS  = 1      ;

	cutActive   = false ;
	pasteActive = false ;
	colsOn      = false ;
	placeCursor( 0, 0 ) ;

	while ( true )
	{
		if      ( rebuildZAREA  ) { fill_dynamic_area() ; }
		else if ( rebuildShadow ) { ZSHADOW = CSHADOW   ; }
		rebuildShadow = false ;

		ZROW1 = right( d2ds( topLine ), 8, '0' )    ;
		ZROW2 = right( d2ds( data.size() - 2 ), 8, '0' ) ;
		ZCOL1 = right( d2ds( startCol ), 7, '0' )     ;
		if ( MSG == "" )
		{
			if ( OCMD[ 0 ] == '&' ) { ZCMD = OCMD ; }
			else                    { ZCMD = ""   ; }
		}

		termEdit = false ;
		positionCursor() ;

		display( "PEDIT012", MSG, CURFLD, CURPOS ) ;

		if ( RC  > 8 ) { abend()         ; }
		if ( RC == 8 ) { termEdit = true ; }
		MSG = "" ;
		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		clearCursor() ;
		if ( abbrev( "CANCEL", upper( ZCMD ), 3 ) ) { break ; }

		if ( ZCMD[ 0 ] == '&' ) { OCMD = ZCMD ; ZCMD.erase( 0, 1 ) ; }
		else                    { OCMD = ""   ;                      }

		if ( ZCURFLD == "ZAREA" )
		{
			CURFLD = ZCURFLD ;
			CURPOS = ZCURPOS ;
			aRow   = ((ZCURPOS-1) / ZAREAW + 1)    ;
			aCol   = ((ZCURPOS-1) % ZAREAW + 1)    ;
			aURID  = s2data.at( aRow-1 ).ipos_URID ;
		}
		else
		{
			CURFLD = "ZCMD" ;
			CURPOS = 1      ;
			aRow   = 0      ;
			aCol   = 0      ;
			aURID  = 0      ;
		}
		storeCursor( aURID, 4, aCol-CLINESZ+startCol-2 ) ;

		getZAREAchanges() ;
		updateData()      ;
		actionZVERB()     ;
		processNISRTlines() ;

		actionPrimCommand1() ;
		if ( MSG != "" )
		{
			getmsg( MSG, "", "", "", "", "ZMTYPE" ) ;
			vcopy( "ZMTYPE", t, MOVE ) ;
			if ( t != "NOTIFY" ) { continue ; }
		}

		actionLineCommands() ;
		if ( MSG != "" && MSG != "PEDT01W" )
		{
			getmsg( MSG, "", "", "", "", "ZMTYPE" ) ;
			vcopy( "ZMTYPE", t, MOVE ) ;
			if ( t != "NOTIFY" ) { continue ; }
		}

		actionPrimCommand2() ;
		if ( MSG != "" )
		{
			getmsg( MSG, "", "", "", "", "ZMTYPE" ) ;
			vcopy( "ZMTYPE", t, MOVE ) ;
			if ( t != "NOTIFY" ) { continue ; }
		}

		if ( upper( ZCMD ) == "SAVE" )
		{
			if ( !saveFile() ) { continue ; }
			ZCMD = ""  ;
		}
		if ( termEdit && ( MSG == "" || MSG == "PEDT01P" ) )
		{
			if ( termOK() ) { break    ; }
			else            { continue ; }
		}
	}
	vput( "ZSCROLL", PROFILE ) ;

	for_each( data.begin(), data.end(), [](iline * & a) { delete a ; } ) ;
	data.clear() ;
	saveEditProfile( ZEDPROF ) ;
	EditList.erase( ZFILE ) ;
}


void PEDIT01::showEditEntry()
{
	ZRC = 0  ;
	MSG = "" ;
	display( "PEDIT011", MSG, "ZCMD1" ) ;
	if ( RC  > 8 ) { abend() ; }
	if ( RC == 8 ) { ZRC = 4 ; }
}


void PEDIT01::showEditRecovery()
{
	ZRC = 0  ;
	MSG = "" ;
	display( "PEDIT014", MSG, "ZCMD4" ) ;
	if ( RC  > 8 ) { abend() ; }
	if ( RC == 8 ) { ZRC = 4 ; }
}


bool PEDIT01::termOK()
{
	if ( fileChanged && !profSave && ZCMD != "SAVE" )
	{
		MSG = "PEDT01O" ;
		return false    ;
	}
	if ( fileChanged )
	{
		if ( saveFile() ) { setmsg( "PEDT01P" ) ; }
		else              { return false        ; }
	}
	return true ;
}


void PEDIT01::readFile()
{
	int p   ;
	int j   ;
	int pos ;

	string ZDATE  ;
	string ZTIMEL ;
	string inLine ;
	string fname  ;

	vector<string> Notes ;

	boost::system::error_code ec ;

	iline * p_iline ;

	if ( abendRecovery )
	{
		fname       = ZFILE + ".abend" ;
		fileChanged = true ;
	}
	else
	{
		fname       = ZFILE ;
		fileChanged = false ;
	}

	std::ifstream fin( fname.c_str() ) ;

	if ( EditList.find( ZFILE ) != EditList.end() )
	{
		MSG     = "PEDT013A" ;
		ZRESULT = "File In Use" ;
		RC      = 4 ;
		ZRC     = 4 ;
		ZRSN    = 0 ;
		return      ;
	}

	if ( !exists( fname ) )
	{
	}

	if ( !fin.is_open() )
	{
		ZRESULT = "Open Error" ;
		RC      = 16 ;
		return  ;
	}

	topLine  = 0 ;
	startCol = 1 ;
	maxCol   = 1 ;
	Level    = 0 ;

	rebuildZAREA = true  ;
	tabsOnRead   = false ;

	data.clear() ;
	maxURID[ taskid() ] = 0 ;

	p_iline          = new iline( taskid() ) ;
	p_iline->il_tod  = true   ;
	p_iline->put_idata( centre( " TOP OF DATA ", ZAREAW, '*' ), 0 ) ;
	data.push_back( p_iline ) ;

	while ( getline( fin, inLine ) )
	{
		p_iline = new iline( taskid() ) ;
		pos = inLine.find( '\t' ) ;
		while ( pos != string::npos )
		{
			tabsOnRead = true ;
			j = 8 - (pos % 8 ) ;
			inLine.replace( pos, 1,  j, ' ' )  ;
			pos = inLine.find( '\t', pos + 1 ) ;
		}
		if ( maxCol < inLine.size() ) maxCol = inLine.size() ;
		p_iline->il_file = true   ;
		p_iline->put_idata( inLine, 0 ) ;
		data.push_back( p_iline ) ;
	}
	maxCol++ ;
	p_iline = new iline( taskid() ) ;
	p_iline->il_bod  = true ;
	p_iline->put_idata( centre( " BOTTOM OF DATA ", ZAREAW, '*' ), 0 ) ;
	data.push_back( p_iline )    ;
	fin.close() ;

	Notes.push_back( " WARNING!!!  EDITOR UNDER CONSTRUCTION.  SAVE FILES AT YOUR OWN RISK !!!!" ) ;

	EditList[ ZFILE ] = true ;

	if ( tabsOnRead && !profXTabs )
	{
		Notes.push_back( " WARNING!!!  TABS HAVE BEEN DETECTED AND CONVERTED TO SPACES" ) ;
		Notes.push_back( " WARNING!!!  BUT PROFILE OPTION IS NOT TO CONVERT SPACES TO TABS ON SAVE" )            ;
		Notes.push_back( " WARNING!!!  USE 'XTABS ON' COMMAND TO CHANGE" )                                       ;
	}
	else if ( !tabsOnRead && profXTabs )
	{
		Notes.push_back( " WARNING!!!  TABS HAVE NOT BEEN DETECTED.  PROFILE XTABS OPTION" ) ;
		Notes.push_back( " WARNING!!!  HAS BEEN SET OFF" )                                                       ;
		Notes.push_back( " WARNING!!!  USE 'XTABS ON' COMMAND TO CHANGE" )                                       ;
		profXTabs = false ;
	}

	if ( profRecover && !abendRecovery )
	{
		vcopy( "ZJ4DATE", ZDATE, MOVE ) ;
		vcopy( "ZTIMEL", ZTIMEL, MOVE ) ;
		p = ZFILE.find_last_of( '/' )   ;
		copy_file( ZFILE, recoverLoc + ZFILE.substr( p+1 ) + "-" +ZDATE + "-" + ZTIMEL, ec ) ;
		if ( ec.value() != boost::system::errc::success )
		{
			MSG = "PEDT011F" ;
		}
	}

	addSpecial( 'N', topLine, Notes ) ;
	data[ 0 ]->clear_Global_Redo() ;
	data[ 0 ]->clear_Global_Undo() ;
	data[ 0 ]->clear_Global_File_level() ;
	saveLevel = 0 ;
}


bool PEDIT01::saveFile()
{
	int i     ;

	string t1 ;
	string t2 ;
	string f  ;

	f = ZFILE ;
	vector<iline * >::iterator it ;
	std::ofstream fout( f.c_str() ) ;

	if ( !fout.is_open() )
	{
		ZRESULT = "Open Error" ;
		RC = 16 ;
		return  false ;
	}

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_file || (*it)->il_deleted ) { continue ; }
		t1 = (*it)->get_idata() ;
		t1 = strip( t1, 'T', ' ' ) ;
		if ( profXTabs )
		{
			t2 = "" ;
			for ( i = 0 ; i < t1.size() ; i++ )
			{
				if ( (i % 8 == 0) && t1.size() > i+7 && t1.compare( i, 8, "        " ) == 0 )
				{
					t2.push_back( '\t' ) ;
					i = i + 7 ;
				}
				else if ( t1[ i ] != ' ' ) { break ; }
				else                       { t2.push_back( ' ' ) ; }

			}
			if ( i < t1.size() )
			{
				t2 = t2 + t1.substr( i ) ;
			}
			fout << t2 << endl ;
		}
		else
		{
			fout << t1 << endl ;
		}
	}
	fileChanged = false ;
	fout.close() ;
	if ( fout.fail() ) { MSG = "PEDT01Q" ; ZRC = 8 ; ZRSN = 8 ; return false ; }

	if ( undoON )
	{
		it        = data.begin() ;
		saveLevel = (*it)->get_Global_File_level() ;
	}
	MSG  = "PEDT01P" ;
	ZRC  = 0 ;
	ZRSN = 4 ;
	return true ;
}


void PEDIT01::fill_dynamic_area()
{
	// s2data is only valid while the data vector has not changed since building the screen
	// ie. after fill_dynamic_area until the end of procedure processNISRTlines()

	int i   ;
	int l   ;
	int ln  ;
	int t   ;
	int sl  ;
	int fl  ;
	int elines ;
	int blines ;
	int URID   ;

	uint dl ;

	ipos ip ;

	string t1 ;
	string t2 ;
	string t3 ;
	string t4 ;
	string lcc ;
	string tmp1 ;
	string tmp2 ;

	const string din( 1, 0x01 )  ;
	const string dout( 1, 0x02 ) ;

	t3.assign( ZDATAW, ' ' ) ;
	t4.assign( ZDATAW, ' ' ) ;

	ZAREA   = "" ;
	ZSHADOW = "" ;

	s2data.clear() ;
	sl = 0         ;

	if ( data.at( topLine )->il_deleted ) { topLine = getNextDataLine( topLine ) ; }

	fl = getFileLine( topLine ) ;
	for ( i = 0 ; i < ZAREAD ; i++ ) { s2data[ i ] = ip ; }
	for ( dl = topLine ; dl < data.size() ; dl++ )
	{
		if ( data.at( dl )->il_deleted ) { continue ; }
		if ( data.at( dl )->il_file )    { fl++     ; }
		if ( data.at( dl )->il_excl )
		{
			elines       = getEXBlock( data.at( dl )->il_URID ) ;
			ip.ipos_line = dl ;
			ip.ipos_URID = data.at( dl )->il_URID ;
			s2data.at( sl ) = ip ;
			tmp1 = copies( "-  ", (ZAREAW - 30)/3 - 2 ) + d2ds( elines ) + " Line(s) Not Displayed" ;
			if ( data.at( dl )->il_lcc == "" )
			{
				tmp2 = "- - - " ;
			}
			else
			{
				tmp2 = left( data.at( dl )->il_lcc, 6 ) ;
			}
			ZAREA   = ZAREA + din + tmp2 + dout + substr( tmp1, 1, ZAREAW-8 ) ;
			ZSHADOW = ZSHADOW + slr + sdw ;
			if ( ZAREA.size() >= ZASIZE ) { break ; }
			sl++ ;
			if ( dl == topLine )
			{
				URID    = data.at( topLine )->il_URID ;
				topLine = getFirstEX( topLine ) ;
				blines  = getDataBlock( URID )  ;
				dl      = topLine + blines - 1 ;
			}
			else
			{
				blines = getDataBlock( data.at( dl )->il_URID ) ;
				dl = dl + blines - 1 ;
			}
			fl = getFileLine( dl+1 ) ;
			continue ;
		}
		if ( data.at( dl )->il_lcc == "" )
		{
			if ( data.at( dl )->il_label == "" )
			{
				if      ( data.at( dl )->il_nisrt ) { lcc = "''''''" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_note  ) { lcc = "=NOTE=" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_prof  ) { lcc = "=PROF>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_col   ) { lcc = "=COLS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_bnds  ) { lcc = "=BNDS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_mask  ) { lcc = "=MASK>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_tabs  ) { lcc = "=TABS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_chg   ) { lcc = "==CHG>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_msg   ) { lcc = "==MSG>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_error ) { lcc = "==ERR>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_undo  ) { lcc = "=UNDO>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_redo  ) { lcc = "=REDO>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_info  ) { lcc = "======" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_file  )
				{
					lcc = right( d2ds( fl ), 6, '0' ) ;
					ZSHADOW = ZSHADOW + slg ;
				}
				else { lcc = "******" ; ZSHADOW = ZSHADOW + slr ; }
			}
			else
			{
				lcc     = left( data.at( dl )->il_label, 6 ) ;
				ZSHADOW = ZSHADOW + slr                      ;
			}
		}
		else
		{
			lcc     = left( data.at( dl )->il_lcc, 6 ) ;
			ZSHADOW = ZSHADOW + slr                    ;
		}
		if ( data.at( dl )->il_file )
		{
			if ( data.at( dl )->il_hex || profHex )
			{
				t1 = substr( data.at( dl )->get_idata(), startCol, ZDATAW ) ;
				t2 = cs2xs( t1 ) ;
				ln = data.at( dl )->get_idata().size() - startCol + 1 ;
				ln = t1.size() ;
				if ( ln > 0 )
				{
					i = 0 ;
					for ( l = 0 ; l < (ln * 2) ; l++ )
					{
						t3.replace(i, 1, 1, t2[ l ] ) ;
						l++ ;
						t4.replace(i, 1, 1, t2[ l ] ) ;
						i++ ;
					}
				}
				t1.resize( ZDATAW ) ;
				t3.resize( ZDATAW ) ;
				t4.resize( ZDATAW ) ;
				ZAREA   = ZAREA + din + lcc + din  + t1 ;
				ZAREA   = ZAREA + "       " + dout + t3 ;
				ZAREA   = ZAREA + "       " + dout + t4 ;
				ZAREA   = ZAREA + dout + div  ;
				ZSHADOW = ZSHADOW + sdg       ;
				ZSHADOW = ZSHADOW + slw + sdg ;
				ZSHADOW = ZSHADOW + slw + sdg ;
				ZSHADOW = ZSHADOW + slw + sdw ;
				ip.ipos_line  = dl ;
				ip.ipos_URID  = data.at( dl )->il_URID ;
				s2data.at( sl ) = ip ;
				sl              = sl + 4 ;
			}
			else
			{
				ZAREA = ZAREA + din + lcc + din + substr( data.at( dl )->get_idata(), startCol, ZDATAW ) ;
				if ( data.at( dl )->il_nisrt ) { ZSHADOW = ZSHADOW + sdyh ; }
				else                           { ZSHADOW = ZSHADOW + sdy  ; }
				ip.ipos_line     = dl ;
				ip.ipos_URID     = data.at( dl )->il_URID ;
				s2data.at( sl ) = ip ;
				sl++ ;
			}
		}
		else
		{
			if ( data.at( dl )->il_col )
			{
				tmp1 = substr( "+----+----+----+---", startCol%10+1, 10-startCol%10 ) ;
				if ( startCol%10 == 0 ) { tmp1 = "" ; t = startCol / 10 ; }
				else                    { t = startCol / 10 + 1        ; }
				for ( i = 0 ; i < (ZDATAW/10+1) ; i++ )
				{
					tmp1 = tmp1 + d2ds( t%10 ) + "----+----" ;
					t++ ;
				}
				ZAREA   = ZAREA + din + lcc + dout + substr( tmp1, 1, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdw ;
			}
			else if ( data.at( dl )->il_bnds )
			{
				tmp1 = string( LeftBnd-1, ' ' ) + "<" ;
				if ( RightBnd > 0 ) { tmp1 = tmp1 + string( RightBnd-tmp1.size()-1, ' ' ) + ">" ; }
				ZAREA   = ZAREA + din + lcc + din + substr( tmp1, startCol, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdw ;
			}
			else if ( data.at( dl )->il_mask )
			{
				data.at( dl )->put_idata( maskLine ) ;
				ZAREA   = ZAREA + din + lcc + din + substr( data.at( dl )->get_idata(), startCol, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdr ;
			}
			else if ( data.at( dl )->il_tabs )
			{
				data.at( dl )->put_idata( tabsLine ) ;
				ZAREA   = ZAREA + din + lcc + din + substr( data.at( dl )->get_idata(), startCol, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdr ;
			}
			else
			{
				ZAREA = ZAREA + din + lcc + dout + substr( data.at( dl )->get_idata(), 1, ZDATAW ) ;
				if ( data.at( dl )->il_tod || data.at( dl )->il_bod )
				{
					ZSHADOW = ZSHADOW + sdb ;
				}
				else
				{
					ZSHADOW = ZSHADOW + sdw ;
				}
			}
			ip.ipos_line = dl ;
			ip.ipos_URID = data.at( dl )->il_URID ;
			s2data.at( sl ) = ip ;
			sl++ ;
		}
		if ( ZAREA.size() >= ZASIZE ) break ;
	}

	ZAREA.resize( ZASIZE, ' ' ) ;
	ZSHADOW.resize( ZASIZE, N_GREEN ) ;

	CAREA = ZAREA ;
	if ( profHilight )
	{
		fill_hilight_shadow() ;
	}
	if ( colsOn )
	{
		tmp1 = substr( "+----+----+----+---", startCol%10+1, 10-startCol%10 ) ;
		if ( startCol%10 == 0 ) { tmp1 = "" ; t = startCol / 10 ; }
		else                    { t = startCol / 10 + 1        ; }
		for ( i = 0 ; i < (ZDATAW/10+1) ; i++ )
		{
			tmp1 = tmp1 + d2ds( t%10 ) + "----+----" ;
			t++ ;
		}
		tmp1 = dout + "=COLS> " + tmp1 ;
		ZAREA.replace( 0, ZAREAW, substr( tmp1, 1, ZAREAW ) ) ;
		ZSHADOW.replace( 0, CLINESZ, slr ) ;
		ZSHADOW.replace( 8, ZDATAW, sdw ) ;
	}
	CSHADOW      = ZSHADOW ;
	rebuildZAREA = false   ;
}


void PEDIT01::fill_hilight_shadow()
{
	// Build il_Shadow starting at the first invalid shadow line in data,
	// (backing up to the line after the position where there are no open brackets/comments)
	// until bottom of ZAREA reached (only for non-excluded, data file lines)

	// il_vShadow - true if there is a valid shadow line for this data line (stored in il_Shadow)
	// il_wShadow - true if no open brackets or open comments at the end of the file line for this shadow line

	int i  ;
	int ll ;
	int l  ;
	int w  ;

	uint dl ;

	string ZTEMP ;

	hlight.hl_language = profLang ;

	for ( i = ZAREAD-1 ; i >= 0 ; i-- )
	{
		ll = s2data.at( i ).ipos_line ;
		if ( ll > 0 ) { break ; }
	}

	w = 0 ;
	for ( dl = 1 ; dl <= ll ; dl++ )
	{
		if (  data.at( dl )->il_deleted || !data.at( dl )->il_file ) { continue ; }
		if ( !data.at( dl )->il_vShadow ) { break  ; }
      //        if (  data.at( dl )->il_wShadow ) { w = dl ; }
	}
	for ( dl-- ; dl > 0 ; dl-- )
	{
		if (  data.at( dl )->il_deleted || !data.at( dl )->il_file ) { continue ; }
		if (  data.at( dl )->il_wShadow ) { break ; }
	}
	w = dl + 1 ;
	hlight.hl_oBrac1   = 0     ;
	hlight.hl_oBrac2   = 0     ;
	hlight.hl_oComment = false ;
 //     if ( dl != w && w < data.size()-1 ) { w++ ; }
	for ( dl = w ; dl <= ll ; dl++ )
	{
		if ( data.at( dl )->il_deleted || !data.at( dl )->il_file ) { continue ; }
		data.at( dl )->il_vShadow = true ;
		addHilight( hlight, data.at( dl )->get_idata(), data.at( dl )->il_Shadow ) ;
		data.at( dl )->il_wShadow = ( hlight.hl_oBrac1 == 0 && hlight.hl_oBrac2 == 0 && !hlight.hl_oComment ) ;
	}
	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		l = s2data.at( i ).ipos_line ;
		if ( !data.at( l )->il_file || data.at( l )->il_excl )  { continue ; }
		ZTEMP = data.at( l )->il_Shadow ;
		if ( startCol > 1 ) { ZTEMP.erase( 0, startCol-1 ) ; }
		ZTEMP.resize( ZDATAW, N_GREEN ) ;
		ZSHADOW.replace( (ZAREAW*i + CLINESZ), ZDATAW, ZTEMP ) ;
	}
}


void PEDIT01::getZAREAchanges()
{
	// Algorithm for getting the line command:
	//    Remove leading digits and spaces (these are ignored)
	//    Find last changed character
	//    If cursor is on the field and after this point, use cursor positon to truncate field (characters after cursor position are ignored if not changed)
	//    Else use this changed character position to truncate field (trailing unchanged characters are ignored)

	int i   ;
	int j   ;
	int off ;
	int sp  ;
	int ep  ;

	uint dl ;

	string lcc ;

	const char duserMod(0x3) ;
	const char ddataMod(0x4) ;

	sTouched.clear() ;
	sChanged.clear() ;

	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		dl            = s2data.at( i ).ipos_line ;
		off           = i * ZAREAW  ;
		sTouched[ i ] = false       ;
		sChanged[ i ] = false       ;
		if ( ZAREA[ off ] == ddataMod )
		{
			lcc = "" ;
			if ( data.at( dl )->il_lcc == "" && data.at( dl )->il_label == "" )
			{
				for ( j = off+1 ; j < off+6 ; j++ )
				{
					if ( ZAREA[ j ] == ' ' || isdigit( ZAREA[ j ] ) ) { ZAREA[ j ] = ' ' ; continue ; }
					break ;
				}
				sp = j - off ;
				if ( aRow == (i + 1 ) && ( aCol > 1 && aCol < 9 ) )
				{
					for ( j = (off + 6) ; j > off ; j-- )
					{
						if ( j < (aCol + off -1 ) || ( ZAREA[ j ] != CAREA[ j ] ) )  { break ; }
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
			lcc = strip( upper( lcc ) ) ;
			if ( lcc == "" )
			{
				if ( data.at( dl )->il_chg   ) { data.at( dl )->il_chg   = false ; }
				if ( data.at( dl )->il_error ) { data.at( dl )->il_error = false ; }
				if ( data.at( dl )->il_undo  ) { data.at( dl )->il_undo  = false ; }
				if ( data.at( dl )->il_redo  ) { data.at( dl )->il_redo  = false ; }
			}
			else if ( !data.at( dl )->il_file )
			{
				lcc = strip( lcc, 'L', '=' ) ;
			}
			if ( datatype( lcc, 'W' ) ) { lcc = "" ; }
			if ( lcc == "" )
			{
				if ( data.at( dl )->il_lcc == "" ) { data.at( dl )->il_label = "" ; }
				else                               { data.at( dl )->il_lcc   = "" ; }
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
			rebuildZAREA  = true ;
		}
	}
}


void PEDIT01::updateData()
{
	int i ;
	int j ;
	int p ;

	uint dl  ;
	string t ;

	Level++ ;
	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		if ( s2data.at( i ).ipos_URID == 0 ) { continue ; }
		dl = s2data.at( i ).ipos_line ;
		if ( !data.at( dl )->il_nisrt && !sChanged[ i ] ) { continue ; }
		if (  data.at( dl )->il_nisrt && !sChanged[ i ] && !sTouched[ i ] ) { continue ; }
		if (  data.at( dl )->il_bnds )
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
			t = data.at( dl )->get_idata() ;
			if ( startCol > 1 )
			{
				if ( t.size() > ZDATAW+startCol-1 )
				{
					t = substr( t, 1, startCol-1 ) + ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ) + t.substr( ZDATAW+startCol-1 ) ;
				}
				else
				{
					t = substr( t, 1, startCol-1 ) + strip( ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ), 'T', ' ' ) ;
				}
			}
			else
			{
				if ( t.size() > ZDATAW )
				{
					t = ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ) + t.substr( ZDATAW ) ;
				}
				else
				{
					t = strip( ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ), 'T', ' ' ) ;
				}
			}
			if ( profCaps ) { t = upper( t ) ; }
			if ( data.at( dl )->il_mask )
			{
				data.at( dl )->put_idata( t ) ;
				maskLine = t ;
			}
			else if ( data.at( dl )->il_tabs )
			{
				for ( j = 0 ; j < t.size() ; j++ )
				{
					if ( t[ j ] != ' ' && t[ j ] != '*' && t[ j ] != '-' )
					{
						t[ j ] = ' ' ;
					}
				}
				data.at( dl )->put_idata( t ) ;
				tabsLine = t ;
			}
			else
			{
				if ( data.at( dl )->il_nisrt )
				{
					data.at( dl )->put_idata( t ) ;
					data.at( dl )->set_idata_level( Level ) ;
				}
				else
				{
					data.at( dl )->put_idata( t, Level ) ;
				}
			}
			fileChanged = true ;
		}
		rebuildZAREA = true ;
	}
}


void PEDIT01::processNISRTlines()
{
	// Remove new insert lines that have not been changed/touched
	// Add a new line below inserted lines when changed/touched and cursor is on the line still

	// s2data no longer valid after this routine as it changes the data vector

	int i ;
	int k ;

	uint dl  ;

	iline * p_iline ;

	vector<iline * >::iterator it ;

	for ( i = ZAREAD-1 ; i >= 0 ; i-- )
	{
		if ( s2data.at( i ).ipos_URID == 0 ) { continue ; }
		dl = s2data.at( i ).ipos_line ;
		if ( !data.at( dl )->il_nisrt ) { continue ; }
		if ( !sTouched[ i ] && !sChanged[ i ] )
		{
			it = data.begin() ;
			advance( it, dl ) ;
			delete *it        ;
			it = data.erase( it ) ;
			if ( (*it)->il_deleted ) { it = getNextDataLine( it ) ; }
			placeCursor( (*it)->il_URID, 3 ) ;
		}
		else
		{
			data.at( dl )->il_nisrt = false ;
			if ( data.at( dl )->il_URID == aURID )
			{
				it = getLineItr( data.at( dl )->il_URID ) ;
				k  = (*it)->get_idata().find_first_not_of( ' ', startCol-1 ) ;
				p_iline = new iline( taskid() ) ;
				p_iline->il_file  = true ;
				p_iline->il_nisrt = true ;
				p_iline->put_idata( maskLine )  ;
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


	string w1   ;
	string w2   ;
	string w3   ;
	string w4   ;
	string wall ;

	vector<string> Info  ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator its ;
	vector<iline * >::iterator ite ;

	if ( ZVERB == "RFIND" || ZVERB == "RCHANGE" ) { return ; }

	ws = words( ZCMD )            ;
	w1 = upper( word( ZCMD, 1 ) ) ;
	if ( w1 == "" ) { return ; }

	w2 = upper( word( ZCMD, 2 ) ) ;
	w3 = upper( word( ZCMD, 3 ) ) ;
	w4 = upper( word( ZCMD, 4 ) ) ;

	cutActive   = false ;
	pasteActive = false ;

	if ( w1 == "KILL" )
	{
		string xxxx ;
		xxxx.at(5)  ;
	}
	else if ( w1 == "XABEND" )
	{
		abend() ;
	}
	else if ( w1[ 0 ] == ':' && ZCMD.size() < 8 )
	{
		if ( aURID == 0 ) { MSG = "PEDT012X" ; return ; }
		dl = s2data.at( aRow-1 ).ipos_line ;
		if ( data.at( dl )->il_lcc != "" ) { MSG = "PEDT012Y" ; return ; }
		data.at( dl )->il_lcc = upper( ZCMD.substr( 1 ) ) ;
		ZCMD         = ""   ;
		rebuildZAREA = true ;
		return ;
	}
	else if ( w1 == "DIAG" )
	{
		it = data.begin() ;
		Info.push_back( " Editor Diagnostic Information:" ) ;
		Info.push_back( " Global UNDO stack size: " + d2ds( (*it)->get_Global_Undo_Size() ) ) ;
		Info.push_back( " Global REDO stack size: " + d2ds( (*it)->get_Global_Redo_Size() ) ) ;
		Info.push_back( " Global File stack size: " + d2ds( (*it)->get_Global_File_Size() ) ) ;
		Info.push_back( " Data size . . . . . . : " + d2ds( data.size() ) ) ;
		Info.push_back( " Data Level. . . . . . : " + d2ds( (*it)->get_Global_Undo_level() ) ) ;
		Info.push_back( " File Data Level . . . : " + d2ds( (*it)->get_Global_File_level() ) ) ;
		Info.push_back( " Save Level. . . . . . : " + d2ds( saveLevel ) ) ;
		Info.push_back( " Update Level. . . . . : " + d2ds( Level ) ) ;
		if ( fileChanged )
		{
			Info.push_back( " Data changed since last SAVE" ) ;
		}
		if ( saveLevel != (*it)->get_Global_File_level() )
		{
			Info.push_back( " Data changed since last SAVE (acc Global File level)" ) ;
		}
		Info.push_back( " -End-" ) ;
		addSpecial( 'I', topLine, Info ) ;
		rebuildZAREA = true ;
		ZCMD = ""    ;
		return ;
	}
	else if ( w1 == "SHOW" )
	{
		log( "A", " Dumping contents:: (undo level) " << Level << endl ; )
		log( "A", " ++++++++++++++++++++++++++++++++++++++++" << endl ; )
		it = data.begin() ;
		log( "A", " Global UNDO stack size: " << (*it)->get_Global_Undo_Size() << endl  ; )
		log( "A", " Global REDO stack size: " << (*it)->get_Global_Redo_Size() << endl  ; )
		ZCMD = ""    ;
		return ;
	}
	else if ( w1 == "SHOWALL" )
	{
		uint dl ;
		dl = 0 ;
		log( "A", " Dumping array contents:: (undo level) " << Level << endl ; )
		log( "A", " ++++++++++++++++++++++++" << endl ; )
		it = data.begin() ;
		log( "A", " Global UNDO stack size: " << (*it)->get_Global_Undo_Size() << endl  ; )
		log( "A", " Global REDO stack size: " << (*it)->get_Global_Redo_Size() << endl  ; )
		for ( ; it != data.end() ; it++ )
		{
		   //   if ( !(*it)->il_file ) { dl++ ; continue ; }
			log( "A", " Current  lvl: " << (*it)->get_idata_level() << endl  ; )
			log( "A", " Data.at()   : " << dl << endl  ; )
			log( "A", " Current URID: " << (*it)->il_URID << endl  ; )
			if ( (*it)->il_deleted) { log( "A", " Line is Deleted" << endl ; ) }
			if ( (*it)->il_excl   ) { log( "A", " Line is Excluded" << endl ; ) }
			log( "A", " Line command: " << (*it)->il_lcc << endl ; )
			log( "A", " Line label  : " << (*it)->il_label << endl ; )
			if ( !(*it)->idata_is_empty() )
			{
				log( "A", " Record      : " << (*it)->get_idata() << endl ; )
			}
			log( "A", " +End+Record+++++++++++++++++++++++++++++++++++++" << endl ; )
			dl++ ;
		}
		ZCMD = ""    ;
		return ;
	}

	if ( PrimCMDS.find( w1 ) == PrimCMDS.end() )
	{
	       MSG = "PSYS018" ;
	       return          ;
	}

	switch ( PrimCMDS[ w1 ] )
	{
	case PC_CUT:
			cutActive  = true ;
			cutReplace = true ;
			clipboard  = "DEFAULT" ;
			wall       = upper( subword( ZCMD, 2 ) ) ;

			p1 = wordpos( "DEFAULT", wall ) ;
			if ( p1 > 0 ) { wall = delword( wall, p1, 1 ) ; }

			p1 = wordpos( "APPEND", wall )  ;
			if ( p1 > 0 ) { cutReplace = false ; wall = delword( wall, p1, 1 ) ; }
			else
			{
				p1 = wordpos( "REPLACE", wall )  ;
				if ( p1 > 0 ) { wall = delword( wall, p1, 1 ) ; }
				else
				{
					p1 = wordpos( "R", wall )  ;
					if ( p1 > 0 ) { wall = delword( wall, p1, 1 ) ; }
				}
			}
			if ( wall != "" )
			{
				if ( words( wall ) == 1 ) { clipboard = strip( wall ) ; }
				else                      { MSG = "PEDT012A"          ; }
			}
			break ;

	case PC_PASTE:
			pasteActive = true  ;
			pasteKeep   = false ;
			clipboard   = "DEFAULT" ;
			wall        = upper( subword( ZCMD, 2 ) ) ;

			p1 = wordpos( "DEFAULT", wall ) ;
			if ( p1 > 0 ) { wall = delword( wall, p1, 1 ) ; }

			p1 = wordpos( "KEEP", wall )  ;
			if ( p1 > 0 ) { pasteKeep = true ; wall = delword( wall, p1, 1 ) ; }
			else
			{
				p1 = wordpos( "DELETE", wall )  ;
				if ( p1 > 0 ) { wall = delword( wall, p1, 1 ) ; }
			}

			if ( wall != "" )
			{
				if ( words( wall ) == 1 ) { clipboard = wall ; }
				else                      { MSG = "PEDT012B" ; }
			}
			break ;

	case PC_RESET:
			if ( aliasNames.find( w2 ) != aliasNames.end() )
			{
				w2 = aliasNames[ w2 ] ;
			}
			if ( w2 == "" )
			{
				removeSpecialLines() ;
				for_each( data.begin(), data.end(), [](iline * & a)
					{ a->resetFilePrefix() ; a->clearLcc() ; } ) ;
				icmds.clear() ;
				rebuildZAREA = true ;
			}
			else if ( ( w2 == "ALL" && w3 == "" ) )
			{
				removeSpecialLines() ;
				for_each( data.begin(), data.end(), [](iline * & a)
					{ a->resetFilePrefix() ; a->clearLcc() ; a->il_label = "" ; } ) ;
				icmds.clear()       ;
				rebuildZAREA = true ;
			}
			else if ( ( w2 == "CLEAN" && w3 == "" ) )
			{
				removeSpecialLines() ;
				for_each( data.begin(), data.end(), [](iline * & a)
					{ a->resetFilePrefix() ; a->clearLcc() ; } ) ;
				icmds.clear() ;
				cleanupData() ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "CMD")
			{
				if ( w3 == "" )
				{
					its = data.begin() ;
					ite = data.end()   ;
				}
				else if ( w3[ 0 ] != '.' || w4[ 0 ] != '.' || ws != 4 ) { MSG = "PEDT016" ; return ; }
				else if ( !getLabelItr( w3, its, i ) || !getLabelItr( w4, ite, j ) ) { MSG = "PEDT01F" ; return ; }
				else if ( i > j )
				{
					it  = its ;
					its = ite ;
					ite = it  ;
				}
				for ( it = its ; it != ite ; it++ ) { (*it)->il_lcc = "" ; }
				rebuildZAREA = true ;
			}
			else if ( w2 == "CHA" && w3 == "" )
			{
				for_each( data.begin(), data.end(), [](iline * & a) { a->il_chg = false ; } ) ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "ERR" && w3 == "" )
			{
				for_each( data.begin(), data.end(), [](iline * & a) { a->il_error = false ; } ) ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "LAB" && w3 == "" )
			{
				for_each( data.begin(), data.end(), [](iline * & a) { a->il_label = "" ; } ) ;
				rebuildZAREA = true ;
			}
			else if ( findword( w2, "UNDO REDO UNDOREDO" ) && w3 == "" )
			{
				for_each( data.begin(), data.end(), [](iline * & a)
					{ a->il_undo = false ; a->il_redo = false ; } ) ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "SPE" && w3 == "" )
			{
				removeSpecialLines() ;
				rebuildZAREA = true  ;
			}
			else if ( w2 == "X" )
			{
				if ( w3 == "" )
				{
					its = data.begin() ;
					ite = data.end()   ;
				}
				else if ( w3[ 0 ] != '.' || w4[ 0 ] != '.' || ws != 4 ) { MSG = "PEDT016" ; return ; }
				else if ( !getLabelItr( w3, its, i ) || !getLabelItr( w4, ite, j ) ) { MSG = "PEDT01F" ; return ; }
				else if ( i > j )
				{
					it  = its ;
					its = ite ;
					ite = it  ;
				}
				for ( it = its ; it != ite ; it++ ) { (*it)->il_excl = false ; }
				rebuildZAREA = true ;
			}
			else  { MSG = "PEDT011" ; return ; }
			ZCMD = "" ;
			break ;

	}
}


void PEDIT01::actionPrimCommand2()
{
	// Action primary command.  These commands are executed after line command processing

	int i  ;
	int ws ;
	int p1 ;
	int tCol ;
	int tLb  ;
	int tRb  ;

	uint tTop ;

	char loc_dir ;

	string t1   ;
	string t2   ;
	string w1   ;
	string w2   ;
	string w3   ;
	string w4   ;
	string wall ;
	string ucmd ;

	bool   firstc ;

	c_range r ;

	vector<string> Prof  ;
	vector<string> Info  ;
	vector<string> Msgs  ;
	vector<string> tdata ;

	vector<bool>srt_asc ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator its ;
	vector<iline * >::iterator ite ;

	if ( ZVERB == "RFIND" || ZVERB == "RCHANGE" ) { return ; }

	ws = words( ZCMD )            ;
	w1 = upper( word( ZCMD, 1 ) ) ;
	if ( w1 == "" ) { return ; }

	w2 = upper( word( ZCMD, 2 ) ) ;
	w3 = upper( word( ZCMD, 3 ) ) ;
	w4 = upper( word( ZCMD, 4 ) ) ;

	if ( PrimCMDS.find( w1 ) == PrimCMDS.end() )
	{
	       MSG = "PSYS018" ;
	       return          ;
	}

	switch ( PrimCMDS[ w1 ] )
	{
	case PC_AUTOSAVE:
			if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
			else if ( w2 == "ON" || w2 == "" ) { profSave = true  ; }
			else if ( w2 == "OFF"            ) { profSave = false ; }
			break ;

	case PC_BOUNDS:
			if ( ws > 3 ) { MSG = "PEDT011" ; return ; }
			if      ( w2 == "*" )           { tLb = LeftBnd     ; }
			else if ( w2 == ""  )           { tLb = 1 ; tRb = 0 ; }
			else if ( datatype( w2, 'W' ) ) { tLb = ds2d( w2 )  ; }
			else    { MSG = "PEDT011" ; return ; }
			if      ( w3 == "*" )           { tRb = RightBnd    ; }
			else if ( w3 == ""  )           { tRb = 0           ; }
			else if ( datatype( w3, 'W' ) ) { tRb = ds2d( w3 )  ; }
			else    { MSG = "PEDT011" ; return ; }
			if ( tRb > 0 && tLb >= tRb ) { MSG = "PEDT011" ; return ; }
			LeftBnd  = tLb ;
			RightBnd = tRb ;
			rebuildZAREA = true ;
			break ;

	case PC_CAPS:
			if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
			if      ( w2 == "ON" || ws == 1 ) { profCaps = true  ; }
			else if ( w2 == "OFF" )           { profCaps = false ; }
			else { MSG = "PEDT011" ; }
			ZCMD = "" ;

			break ;
	case PC_CHANGE:
			i = 0 ;
			if ( !setFindChangeExcl( 'C' ) ) { return ; }
			Level++ ;
			tTop   = topLine  ;
			tCol   = startCol ;
			firstc = true     ;
			while ( true )
			{
				actionFind() ;
				if (  find_parms.fcx_error )   { return ; }
				if ( !find_parms.fcx_success ) { break  ; }
				i++          ;
				if ( firstc )
				{
					moveColumn( find_parms.fcx_offset ) ;
					tCol = startCol                     ;
					placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
					if ( !URIDonScreen( find_parms.fcx_URID ) )
					{
						tTop = getLine( find_parms.fcx_URID ) ;
						tTop = getPrevDataLine( tTop ) ;
					}
					firstc = false ;
				}
				actionChange() ;
				if ( !find_parms.fcx_chngall ) { break  ; }
				startCol = 1 ;
				topLine  = getLine( find_parms.fcx_URID ) ;
				aCol     = find_parms.fcx_offset + find_parms.fcx_cstring.size() + CLINESZ ;
			}
			topLine  = tTop ;
			startCol = tCol ;
			if ( i > 0 )
			{
				if ( find_parms.fcx_chngall ) { MSG = "PEDT012K" ; }
				else                          { MSG = "PEDT012L" ; }
			}
			else                                  { MSG = "PEDT012H" ; }
			find_parms.fcx_success = false ;
			TYPE = typList[ find_parms.fcx_mtch ] ;
			if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
			else                                { STR = find_parms.fcx_ostring ; }
			OCC  = d2ds( i )    ;
			ZCMD = ""           ;
			rebuildZAREA = true ;
			break ;

	case PC_COLUMN:
			if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
			if      ( w2 == "ON"  ) { colsOn = true    ; }
			else if ( w2 == "OFF" ) { colsOn = false   ; }
			else if ( w2 == ""    ) { colsOn = !colsOn ; }
			else                    { MSG = "PEDT011" ; return ; }
			rebuildZAREA = true ;
			ZCMD = "" ;
			break ;

	case PC_COMPARE:
			compareFiles( subword( ZCMD, 2 ) ) ;
			ZCMD = "" ;
			break ;

	case PC_DELETE:
			if ( ws != 3 ) { MSG = "PEDT011" ; return ; }
			if ( (w2 == "X"   && w3 == "ALL") ||
			     (w2 == "ALL" && w3 == "X" ) )
			{
				i = 0 ;
				Level++ ;
				for ( it = data.begin() ; it != data.end() ; it++ )
				{
					if (  (*it)->il_deleted ||
					     !(*it)->il_excl     ) { continue ; }
					(*it)->set_deleted( Level ) ;
					i++ ;
				}
				if ( i == 0 ) { Level-- ; }
				else
				{
					rebuildZAREA = true ;
					fileChanged  = true ;
				}
				LINES = d2ds( i ) ;
				MSG   = "PEDT01M" ;
			}
			else if ( (w2 == "NX"  && w3 == "ALL") ||
				  (w2 == "ALL" && w3 == "NX") )
			{
				i = 0 ;
				Level++ ;
				for ( it = data.begin() ; it != data.end() ; it++ )
				{
					if ( (*it)->il_deleted ||
					     (*it)->il_excl    ||
					     (*it)->il_tod )    { continue ; }
					if ( (*it)->il_bod )    { break    ; }
					(*it)->set_deleted( Level ) ;
					i++ ;
				}
				if ( i == 0 ) { Level-- ; }
				else
				{
					rebuildZAREA  = true  ;
					fileChanged   = true  ;
				}
				LINES = d2ds( i ) ;
				MSG   = "PEDT01M" ;
			}
			else { MSG = "PEDT011" ; return ; }
			ZCMD = ""           ;
			rebuildZAREA = true ;
			return              ;
			break ;

	case PC_EXCLUDE:
			if ( w2 == "ALL" && ws == 2 )
			{
				for_each( data.begin(), data.end(), [](iline * & a)
				    { if ( !a->il_bod && !a->il_tod && !a->il_deleted ) { a->il_excl = true ; } } ) ;
				rebuildZAREA = true ;
				ZCMD = ""           ;
				return              ;
			}
			if ( !setFindChangeExcl( 'X' ) ) { return ; }
			find_parms.fcx_excl = 'N' ;
			actionFind() ;
			if ( find_parms.fcx_error ) { return ; }
			if ( find_parms.fcx_success )
			{
				moveColumn( find_parms.fcx_offset ) ;
				placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
				MSG   = "PEDT01L" ;
				TYPE  = typList[ find_parms.fcx_mtch ] ;
				LINES = d2ds( find_parms.fcx_lines  ) ;
				if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
				else                                { STR = find_parms.fcx_ostring ; }
			}
			else if ( find_parms.fcx_occurs > 0 && find_parms.fcx_dir == 'A' )
			{
				moveColumn( find_parms.fcx_offset ) ;
				placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
				MSG   = "PEDT01L" ;
				LINES = d2ds( find_parms.fcx_lines  ) ;
				TYPE  = typList[ find_parms.fcx_mtch ] ;
				if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
				else                                { STR = find_parms.fcx_ostring ; }
			}
			else
			{
				MSG  = "PEDT012H" ;
				TYPE = typList[ find_parms.fcx_mtch ] ;
				if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
				else                                { STR = find_parms.fcx_ostring ; }
			}
			ZCMD         = ""   ;
			rebuildZAREA = true ;
			break ;

	case PC_FIND:
			if ( !setFindChangeExcl( 'F' ) ) { return ; }
			actionFind()        ;
			if ( find_parms.fcx_error ) { return ; }
			if ( MSG != "" ) { return ; }
			if ( find_parms.fcx_success )
			{
				moveColumn( find_parms.fcx_offset ) ;
				placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
				if ( !URIDonScreen( find_parms.fcx_URID ) )
				{
					topLine = getLine( find_parms.fcx_URID ) ;
					topLine = getPrevDataLine( topLine ) ;
				}
				MSG  = "PEDT012G" ;
				TYPE = typList[ find_parms.fcx_mtch ] ;
				OCC  = d2ds( find_parms.fcx_occurs ) ;
				if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
				else                                { STR = find_parms.fcx_ostring ; }
			}
			else if ( find_parms.fcx_occurs > 0 && find_parms.fcx_dir == 'A' )
			{
				moveColumn( find_parms.fcx_offset ) ;
				placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
				if ( !URIDonScreen( find_parms.fcx_URID ) )
				{
					topLine = getLine( find_parms.fcx_URID ) ;
					topLine = getPrevDataLine( topLine ) ;
				}
				find_parms.fcx_dir     = 'N'  ;
				find_parms.fcx_success = true ;
				MSG   = "PEDT012I" ;
				TYPE  = typList[ find_parms.fcx_mtch ] ;
				OCC   = d2ds( find_parms.fcx_occurs ) ;
				LINES = d2ds( find_parms.fcx_lines  ) ;
				if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
				else                                { STR = find_parms.fcx_ostring ; }
			}
			else
			{
				MSG  = "PEDT012H" ;
				TYPE = typList[ find_parms.fcx_mtch ] ;
				if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
				else                                { STR = find_parms.fcx_ostring ; }
			}
			ZCMD = ""           ;
			rebuildZAREA = true ;
			break ;

	case PC_FLIP:
			if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
			for_each( data.begin(), data.end(), [](iline * & a)
				{ if ( !a->il_bod && !a->il_tod && !a->il_deleted ) { a->il_excl = !a->il_excl ; } } ) ;
			ZCMD = ""           ;
			rebuildZAREA = true ;
			break ;

	case PC_HEX:
			if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
			if ( w2 == "ON" || ws == 1 )
			{
				profHex      = true ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "OFF" )
			{
				profHex      = false ;
				for_each( data.begin(), data.end(), [](iline * & a) { a->il_hex = false ; } ) ;
				rebuildZAREA = true  ;
			}
			else { MSG = "PEDT011" ; }
			ZCMD = "" ;
			break ;

	case PC_HILIGHT:
			if ( ws > 3 ) { MSG = "PEDT011" ; return ; }
			if ( w2 == "ON" )
			{
				if ( ws == 3 )
				{
					if ( !addHilight( w3 ) ) { MSG = "PEDT012Q" ; }
					else                     { profLang = w3    ; }
				}
				else { profLang = "AUTO" ; }
				profHilight  = true ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "OFF" )
			{
				profHilight  = false ;
				rebuildZAREA = true ;
			}
			else { MSG = "PEDT011" ; }
			ZCMD = "" ;
			break ;

	case PC_LOCATE:
			if ( aliasNames.find( w2 ) != aliasNames.end() )
			{
				w2 = aliasNames[ w2 ] ;
			}
			loc_dir = 'N' ;
			ucmd = upper( subword( ZCMD, 2 ) ) ;
			p1   = wordpos( "FIRST", ucmd ) ;
			if ( p1 > 0 ) { loc_dir = 'F' ; ucmd = delword( ucmd, p1, 1 ) ; }
			else
			{
				p1 = wordpos( "LAST", ucmd ) ;
				if ( p1 > 0 ) { loc_dir = 'L' ; ucmd = delword( ucmd, p1, 1 ) ; }
				else
				{
					p1 = wordpos( "NEXT", ucmd ) ;
					if ( p1 > 0 ) { loc_dir = 'N' ; ucmd = delword( ucmd, p1, 1 ) ; }
					else
					{
						p1 = wordpos( "PREV", ucmd ) ;
						if ( p1 > 0 ) { loc_dir = 'P' ; ucmd = delword( ucmd, p1, 1 ) ; }
					}
				}
			}
			ws = words( ucmd )   ;
			if ( ws != 1 ) { MSG = "PEDT011" ; return ; }
			w2 = word( ucmd, 1 ) ;
			if ( w2[ 0 ] == '.' )
			{
				if ( getLabelItr( w2, it, p1 ) ) { topLine = p1 ; rebuildZAREA = true ; }
				else                             { MSG = "PEDT01F"                    ; }
			}
			else if ( w2 == "SPE" )
			{
				topLine      = getNextSpecial( loc_dir, 'S' ) ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "CHA" )
			{
				topLine      = getNextSpecial( loc_dir, 'C' ) ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "CMD" )
			{
				topLine      = getNextSpecial( loc_dir, 'K' ) ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "ERR" )
			{
				topLine      = getNextSpecial( loc_dir, 'E' ) ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "X" )
			{
				topLine      = getNextSpecial( loc_dir, 'X' ) ;
				rebuildZAREA = true ;
			}
			else if ( findword( w2, "INFOLINE INFO" ) )
			{
				topLine      = getNextSpecial( loc_dir, 'I' ) ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "LAB" )
			{
				topLine      = getNextSpecial( loc_dir, 'L' ) ;
				rebuildZAREA = true ;
			}
			else if ( findword( w2, "MSGLINE MSG" ) )
			{
				topLine      = getNextSpecial( loc_dir, 'M' ) ;
				rebuildZAREA = true ;
			}
			else if ( findword( w2, "NOTELINE NOTE" ) )
			{
				topLine      = getNextSpecial( loc_dir, 'N' ) ;
				rebuildZAREA = true ;
			}
			else if ( findword( w2, "UNDO REDO UNDOREDO" ) )
			{
				topLine      = getNextSpecial( loc_dir, 'U' ) ;
				rebuildZAREA = true ;
			}
			else if ( w2.size() < 9 && datatype( w2, 'W' ) )
			{
				if ( w2 == "0" )
				{
					topLine      = 0    ;
					rebuildZAREA = true ;
				}
				else
				{
					p1 = ds2d( w2 ) ;
					topLine      = getDataLine( p1 ) ;
					rebuildZAREA = true ;
				}
			}
			else { MSG = "PEDT011" ; }
			ZCMD = "" ;
			break ;

	case PC_NULLS:
			if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
			if ( w2 == "ON" || w2 == "" )
			{
				profNulls    = true ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "OFF" )
			{
				profNulls    = false ;
				rebuildZAREA = true  ;
			}
			else { MSG = "PEDT011" ; }
			ZCMD = "" ;
			break ;

	case PC_PROFILE:
			if ( ws == 1 )
			{
				removeProfLines() ;
				t1  = "...."   ;
				t1 += ZEDPTYPE ;
				t1 += "....UNDO "+ OnOff[ undoON ] ;
				t1 += "....AUTOSAVE "+ OnOff[ profSave ] ;
				t1 += "....NUM OFF" ;
				t1 += "....CAPS "+OnOff[ profCaps ] ;
				Prof.push_back( left( t1, ZDATAW, '.') ) ;

				t1  = "....HEX "+ OnOff[ profHex ] ;
				t1 += "....NULLS "+ OnOff[ profNulls ] ;
				t1 += "....XTABS "+ OnOff[ profXTabs ] ;
				t1 += "....TAB SIZE "+ d2ds(profXTabz) ;
				Prof.push_back( left( t1, ZDATAW, '.' ) ) ;

				t1  = "....SOFTWARE TABS "+ OnOff[ profSTabs ] ;
				t1 += "....HARDWARE TABS "+ OnOff[ profHTabs ] ;
				Prof.push_back( left( t1, ZDATAW, '.' ) ) ;

				t1  = "....RECOVER "+ OnOff[ profRecover ]+" PATH "+ recoverLoc ;
				t1 += "....HILIGHT "+ OnOff[ profHilight ]+" "+profLang ;
				t1 += "....PROFILE LOCK "+OnOff[ profLock ] ;
				Prof.push_back( left( t1, ZDATAW, '.' ) ) ;
				addSpecial( 'P', topLine, Prof ) ;
				rebuildZAREA = true  ;
			}
			else if ( ws != 2 )           { MSG = "PEDT011" ; return ; }
			else if ( w2 == "LOCK"    )   { profLock = true  ; }
			else if ( w2 == "UNLOCK"  )   { profLock = false ; }
			else if ( isvalidName( w2 ) ) { saveEditProfile( ZEDPROF ) ; getEditProfile( w2 ) ; ZEDPROF = w2 ; }
			else                          { MSG = "PEDT011" ; return ; }
			ZCMD = "" ;
			break ;

	case PC_RECOVERY:
			if ( w2 == "PATH" )
			{
				if ( w3 == "" ) { MSG = "PEDT011" ; }
				recoverLoc = subword( ZCMD, 3 ) ;
			}
			else if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
			else if ( w2 == "ON" || ws == 1 )
			{
				profRecover  = true ;
				rebuildZAREA = true ;
			}
			else if ( w2 == "OFF" )
			{
				profRecover  = false ;
				rebuildZAREA = true ;
			}
			else { MSG = "PEDT011" ; }
			ZCMD = "" ;
			break ;

	case PC_REDO:
			if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
			actionREDO() ;
			ZCMD = ""    ;
			break ;

	case PC_SAVE:
			if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
			break ;

	case PC_SETUNDO:
			if      ( ws > 2 )   { MSG = "PEDT011" ; return ; }
			else if ( w2 == "ON" )
			{
				undoON = true ;
				if ( !fileChanged )
				{
					it        = data.begin() ;
					saveLevel = (*it)->get_Global_File_level() ;
				}
			}
			else if ( w2 == "OFF" )
			{
				undoON       = false     ;
				MSG          = "PEDT01U" ;
				removeRecoveryData() ;
				saveLevel    = -1    ;
				rebuildZAREA = true  ;
			}
			else if ( w2 == "CLEAR" )
			{
				removeRecoveryData() ;
				if ( fileChanged || !undoON )
				{
					saveLevel = -1 ;
				}
				else { saveLevel = 0 ; }
				rebuildZAREA = true  ;
			}
			else { MSG = "PEDT011" ; return ; }
			ZCMD = ""    ;
			break ;

	case PC_SORT:
			int nsort ;
			int s1    ;
			int s2    ;
			bool sortOrder ;
			r.c_vlab = true ;
			r.c_vcol = true ;
			wall     = upper( subword( ZCMD, 2 ) ) ;
			nsort    = 1    ;                      // just one sort field supported for now
			i = wordpos( "D", wall ) ;
			if ( i > 0 )
			{
				wall = delword( wall, i, 1 ) ;
				srt_asc.push_back( false )   ;
			}
			else
			{
				i = wordpos( "A", wall ) ;
				if ( i > 0 )
				{
					wall = delword( wall, i, 1 ) ;
					srt_asc.push_back( true )    ;
				}
			}
			srt_asc.push_back( true ) ;
			if ( !setCommandRange( wall, r ) ) { return ; }
			if ( r.c_sidx == -1 ) { r.c_sidx = 1 ; r.c_eidx = data.size() - 2 ; }
			copyFileData( tdata, r.c_sidx, r.c_eidx ) ;
			s1 = r.c_scol-1 ;
			s2 = r.c_ecol-1 ;
			if (  LeftBnd  > 1 &&
			    ((r.c_scol > 0 && r.c_scol < LeftBnd) ||
			     (r.c_ecol > 0 && r.c_ecol < LeftBnd) ) )
			{
				MSG = "PEDT013F" ;
				return           ;
			}
			if (  RightBnd > 1 &&
			    ((r.c_scol > 0 && r.c_scol > RightBnd) ||
			     (r.c_ecol > 0 && r.c_ecol > RightBnd) ) )
			{
				MSG = "PEDT013G" ;
				return           ;
			}
			sort( tdata.begin(), tdata.end(),
				[ &s1, &s2, &srt_asc, &nsort ]( const string& a, const string& b )
				{
					for ( int i = 0 ; i < nsort ; i++ )
					{
						 if ( a.size() < s1 || b.size() < s1 ) { return false ; }
						 if ( srt_asc[ i ] )
						 {
							 return a.compare( s1, s2, b, s1, s2 ) < 0 ;
						 }
						 else
						 {
							 return a.compare( s1, s2, b, s1, s2 ) > 0 ;
						 }
					}
					return false ;
				} ) ;
			Level++ ;
			sortOrder = true ;
			its       = getLineItr( data.at( r.c_sidx )->il_URID ) ;
			for ( i = 0 ; i < tdata.size() ; i++, its++ )
			{
				if ( (*its)->il_deleted ||
				    !(*its)->il_file )  { i-- ; continue ; }
				t1 = tdata.at( i ) ;
				t2 = (*its)->get_idata() ;
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
				t1 = strip( t1, 'T', ' ' ) ;
				if ( t1 == t2 ) { continue ; }
				(*its)->put_idata( t1, Level ) ;
				fileChanged = true  ;
				sortOrder   = false ;
			}
			if ( sortOrder ) { MSG = "PEDT013E"    ; }
			else             { rebuildZAREA = true ; }
			ZCMD = "" ;
			break ;

	case PC_TABS:
			if ( ws > 3 ) { MSG = "PEDT011" ; return ; }
			else if ( w2 == "ON" )
			{
				if ( w3 == "ALL" )
				{
					profSTabs = true ;
					profHTabs = true ;
				}
				else if ( w3 == "STD" || w3 == "" )
				{
					profSTabs = true ;
					profHTabs = true ;
				}
				else if ( w3.size() == 1 )
				{
					tabsChar = w3[ 0 ] ;
				}
				else { MSG = "PEDT011" ; }
			}
			else if ( w2 == "OFF" )
			{
				if ( w3 == "" )
				{
					profSTabs = false ;
					profHTabs = false ;
				}
				else { MSG = "PEDT011" ; }
			}
			else { MSG = "PEDT011" ; }
			ZCMD = "" ;
			break ;

	case PC_UNDO:
			if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
			actionUNDO() ;
			ZCMD = ""    ;
			break ;

	case PC_XTABS:
			if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
			else if ( w2 == "ON" )
			{
				profXTabs = true ;
				if ( !tabsOnRead )
				{
					Msgs.push_back( " TABS HAVE BEEN SET ON WHEN NO TABS WHERE FOUND IN THE FILE" ) ;
					addSpecial( 'M', topLine, Msgs ) ;
				}
			}
			else if ( w2 == "OFF" )
			{
				profXTabs = false ;
				if ( tabsOnRead )
				{
					Msgs.push_back( " TABS HAVE BEEN SET OFF WHEN TABS WHERE FOUND IN THE FILE" ) ;
					addSpecial( 'M', topLine, Msgs ) ;
				}
			}
			else if ( datatype( w2, 'W' ) && w2.size() < 3 )
			{
				profXTabz = ds2d( w2 ) ;
			}
			else { MSG = "PEDT011" ; }
			ZCMD = "" ;
			break ;
	}
}


void PEDIT01::actionLineCommands()
{
	// For each line in the command vector, action the line command

	vector<icmd>::iterator itc ;

	if ( !checkLineCommands() ) { return ; }

	for_each( data.begin(), data.end(), [](iline * & a) { a->clearLcc() ; } ) ;

	for ( itc = icmds.begin() ; itc != icmds.end() ; itc++ )
	{
		actionLineCommand( itc ) ;
	}
	icmds.clear() ;
}


void PEDIT01::actionLineCommand( vector<icmd>::iterator itc )
{
	// Action the line command
	// For copy/move/repeat preserve flags: file, note, prof, col, excl, hex

	int j    ;
	int k    ;
	int size ;

	uint dl  ;

	bool overlayOK ;
	bool splitOK   ;
	bool shiftOK   ;

	string tmp1 ;
	string tmp2 ;

	vector<ipline> vip ;
	ipline  ip  ;

	iline * p_iline  ;

	vector<ipline>::iterator new_end  ;
	vector<iline * >::iterator il_it  ;
	vector<iline * >::iterator il_its ;
	vector<iline * >::iterator il_ite ;

	if ( itc->icmd_Rpt == -1 )
	{
		if ( findword( itc->icmd_CMDSTR, "( (( )) ) < << >> >" ) ) { itc->icmd_Rpt = 2 ; }
		else                                                       { itc->icmd_Rpt = 1 ; }
	}
	switch ( itc->icmd_CMD )
	{
		case LC_BOUNDS:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID )   ;
			p_iline          = new iline( taskid() ) ;
			p_iline->il_bnds = true ;
			p_iline->put_idata( "", Level ) ;
			il_its = data.insert( il_its, p_iline ) ;
			rebuildZAREA = true ;
			break ;

		case LC_COLS:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID )   ;
			p_iline          = new iline( taskid() ) ;
			p_iline->il_col  = true ;
			p_iline->put_idata( "", Level ) ;
			il_its = data.insert( il_its, p_iline ) ;
			rebuildZAREA = true ;
			break ;

		case LC_C:
		case LC_M:
			Level++     ;
			vip.clear() ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_bod )     { break                     ; }
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				copyPrefix( ip, (*il_its) ) ;
				ip.ip_data = (*il_its)->get_idata() ;
				vip.push_back( ip ) ;
				il_its++ ;
			}
			overlayOK = true ;
			if ( itc->icmd_cut )
			{
				copyToClipboard( vip ) ;
			}
			else
			{
				il_ite = getLineItr( itc->icmd_dURID ) ;
				if ( itc->icmd_ABO == 'O' )
				{
					new_end = remove_if( vip.begin(), vip.end(), [](const ipline & a) { return !a.ip_file ; } ) ;
					vip.erase( new_end, vip.end() ) ;
					if ( itc->icmd_OSize == -1 ) { itc->icmd_OSize = 1 ; }
					size = truncateSize( itc->icmd_dURID ) ;
					itc->icmd_OSize = (size > itc->icmd_OSize ) ? itc->icmd_OSize : size ;
					if ( itc->icmd_OSize < vip.size() ) { MSG = "PEDT011C" ; }
					k = 0 ;
					while ( true )
					{
						for ( j = 0 ; j < vip.size() ; j++ )
						{
							if ( !(*il_ite)->il_file    ||
							      (*il_ite)->il_deleted ) { j--; il_ite++ ; continue ; }
							fileChanged = true ;
							(*il_ite)->put_idata( overlay( vip[ j ].ip_data, (*il_ite)->get_idata(), overlayOK ), Level ) ;
							il_ite++ ;
							k++      ;
							if ( k >= itc->icmd_OSize ) { break ; }
						}
						if ( k >= itc->icmd_OSize ) { break ; }
					}
				}
				else
				{
					if ( itc->icmd_ABO == 'B' ) { il_ite = getLineBeforeItr( itc->icmd_dURID ) ; }
					for ( j = 0 ; j < vip.size() ; j++ )
					{
						p_iline  = new iline( taskid() ) ;
						copyPrefix( p_iline, vip[ j ]  ) ;
						p_iline->put_idata( vip[ j ].ip_data, Level ) ;
						il_ite++ ;
						il_ite = data.insert( il_ite, p_iline ) ;
						if ( p_iline->il_file ) { fileChanged = true ; }
					}
				}
			}
			if ( itc->icmd_CMD == LC_M && overlayOK )
			{
				il_its = getLineItr( itc->icmd_sURID ) ;
				for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
				{
					if ( (*il_its)->il_bod )     { break                     ; }
					if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
					(*il_its)->set_deleted( Level ) ;
					il_its++ ;
				}
			}
			if ( !overlayOK && itc->icmd_CMDSTR[ 0 ] == 'M' ) { MSG = "PEDT011D" ; }
			rebuildZAREA = true ;
			break ;

		case LC_CC:
		case LC_MM:
			Level++ ;
			vip.clear() ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue ; }
				copyPrefix( ip, (*il_its) ) ;
				ip.ip_data = (*il_its)->get_idata() ;
				vip.push_back( ip ) ;
			}
			overlayOK = true ;
			if ( itc->icmd_cut )
			{
				copyToClipboard( vip ) ;
			}
			else
			{
				il_ite = getLineItr( itc->icmd_dURID ) ;
				if ( itc->icmd_ABO == 'O' )
				{
					new_end = remove_if( vip.begin(), vip.end(), [](const ipline & a) { return !a.ip_file ; } ) ;
					vip.erase( new_end, vip.end() ) ;
					if ( itc->icmd_OSize == -1 ) { itc->icmd_OSize = 1 ; }
					size = truncateSize( itc->icmd_dURID ) ;
					itc->icmd_OSize = (size > itc->icmd_OSize ) ? itc->icmd_OSize : size ;
					if ( itc->icmd_OSize < vip.size() ) { MSG = "PEDT011C" ; }
					k = 0 ;
					while ( true )
					{
						for ( j = 0 ; j < vip.size() ; j++ )
						{
							if ( !(*il_ite)->il_file   ||
							      (*il_ite)->il_deleted ) { j--; il_ite++ ; continue ; }
							fileChanged = true ;
							(*il_ite)->put_idata( overlay( vip[ j ].ip_data, (*il_ite)->get_idata(), overlayOK ), Level ) ;
							il_ite++ ;
							k++      ;
							if ( k >= itc->icmd_OSize ) { break ; }
						}
						if ( k >= itc->icmd_OSize ) { break ; }
					}
				}
				else
				{
					if ( itc->icmd_ABO == 'B' ) { il_ite = getLineBeforeItr( itc->icmd_dURID ) ; }
					for ( j = 0 ; j < vip.size() ; j++ )
					{
						p_iline  = new iline( taskid() ) ;
						copyPrefix( p_iline, vip[ j ]  ) ;
						p_iline->put_idata( vip[ j ].ip_data, Level ) ;
						il_ite++ ;
						il_ite = data.insert( il_ite, p_iline ) ;
						if ( p_iline->il_file ) { fileChanged = true ; }
					}
				}
			}
			if ( itc->icmd_CMD == LC_MM && overlayOK )
			{
				il_its = getLineItr( itc->icmd_sURID ) ;
				il_ite = getLineItr( itc->icmd_eURID ) ;
				il_ite++ ;
				for ( ; il_its != il_ite ; il_its++ )
				{
					if ( (*il_its)->il_deleted ) { continue ; }
					(*il_its)->set_deleted( Level ) ;
				}
			}
			if ( !overlayOK && itc->icmd_CMDSTR[ 0 ] == 'M' ) { MSG = "PEDT011D" ; }
			rebuildZAREA = true ;
			break ;

		case LC_D:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_bod )     { break                     ; }
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				if ( (*il_its)->il_file    ) { fileChanged = true        ; }
				(*il_its)->set_deleted( Level ) ;
				il_its++ ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_DD:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue           ; }
				if ( (*il_its)->il_file    ) { fileChanged = true ; }
				(*il_its)->set_deleted( Level ) ;

			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_F:
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if (  (*il_its)->il_bod )    { break                      ; }
				if ( !(*il_its)->il_excl )   { break                      ; }
				if (  (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				(*il_its)->il_excl = false ;
				il_its++ ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_HX:
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_bod )     { break                     ; }
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				(*il_its)->il_hex = !(*il_its)->il_hex ;
				il_its++ ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_HXX:
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue ; }
				(*il_its)->il_hex = !(*il_its)->il_hex ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_I:
			il_its = getLineItr( itc->icmd_sURID ) ;
			k = (*il_its)->get_idata().find_first_not_of( ' ', startCol-1 ) ;
			k = (k != string::npos) ? k : 0 ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				p_iline = new iline( taskid() ) ;
				p_iline->il_file  = true ;
				p_iline->il_nisrt = true ;
				p_iline->put_idata( maskLine ) ;
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
				il_its = getValidDataLine( il_its ) ;
				placeCursor( (*il_its)->il_URID, 4, k ) ;
			}
			rebuildZAREA = true ;
			break ;

		case LC_L:
			if ( itc->icmd_eURID == 0 ) { break ; }
			dl = getLine( itc->icmd_eURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if (  data.at( dl )->il_tod  )    { break ; }
				if (  data.at( dl )->il_deleted ) { j-- ; dl = getPrevDataLine( dl ) ; continue ; }
				if ( !data.at( dl )->il_excl )    { break ; }
				data.at( dl )->il_excl = false ;
				dl = getPrevDataLine( dl ) ;
			}
			placeCursor( itc->icmd_sURID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_LC:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_bod )     { break                     ; }
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				(*il_its)->put_idata( lower( (*il_its)->get_idata() ), Level ) ;
				il_its++ ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_LCC:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue ; }
				(*il_its)->put_idata( lower( (*il_its)->get_idata() ), Level ) ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_MASK:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			p_iline          = new iline( taskid() ) ;
			p_iline->il_mask = true ;
			p_iline->put_idata( maskLine, Level ) ;
			il_its = data.insert( il_its, p_iline ) ;
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_MD:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				if ( (*il_its)->il_file )    { il_its++ ; continue       ; }
				if ( (*il_its)->il_bod  )    { break                     ; }
				p_iline = new iline( taskid() ) ;
				p_iline->il_file = true ;
				p_iline->put_idata( (*il_its)->get_idata(), Level ) ;
				(*il_its)->set_deleted( Level ) ;
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
				il_its++ ;
				fileChanged = true ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_MDD:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue ; }
				if ( (*il_its)->il_file )    { continue ; }
				p_iline = new iline( taskid() )        ;
				p_iline->il_file    = true ;
				p_iline->put_idata( (*il_its)->get_idata(), Level ) ;
				(*il_its)->set_deleted( Level ) ;
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
				il_ite = getLineItr( itc->icmd_eURID )  ;
				il_ite++ ;
				fileChanged = true ;
				if ( il_its == il_ite ) { break ; }
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_MN:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_bod     ) { break ; }
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				p_iline = new iline( taskid() ) ;
				p_iline->il_note = true ;
				p_iline->put_idata( (*il_its)->get_idata(), Level ) ;
				(*il_its)->set_deleted( Level ) ;
				if ( (*il_its)->il_file ) { fileChanged = true ; }
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
				il_its++ ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_MNN:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue ; }
				p_iline = new iline( taskid() )        ;
				p_iline->il_note    = true ;
				p_iline->put_idata( (*il_its)->get_idata(), Level ) ;
				(*il_its)->set_deleted( Level ) ;
				if ( (*il_its)->il_file ) { fileChanged = true ; }
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
				il_ite = getLineItr( itc->icmd_eURID )  ;
				il_ite++ ;
				if ( il_its == il_ite ) { break ; }
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_R:
			Level++ ;
			il_its  = getLineItr( itc->icmd_sURID ) ;
			tmp1    = (*il_its)->get_idata() ;
			copyPrefix( ip, (*il_its ) ) ;
			if ( (*il_its)->il_file ) { fileChanged = true ; }
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				p_iline = new iline( taskid() )  ;
				copyPrefix( p_iline, ip ) ;
				p_iline->put_idata( tmp1, Level ) ;
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_RR:
			Level++ ;
			vip.clear() ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			for ( ; ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue           ; }
				if ( (*il_its)->il_file )    { fileChanged = true ; }
				copyPrefix( ip, (*il_its ) ) ;
				ip.ip_data = (*il_its)->get_idata() ;
				vip.push_back( ip ) ;
				if ( il_its == il_ite ) { break ; }
			}
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				for ( k = 0 ; k < vip.size() ; k++ )
				{
					p_iline = new iline( taskid() ) ;
					copyPrefix( p_iline, vip[ k ] ) ;
					p_iline->put_idata( vip[ k ].ip_data, Level ) ;
					il_ite++ ;
					il_ite = data.insert( il_ite, p_iline ) ;
				}
			}
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_TABS:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			p_iline          = new iline( taskid() ) ;
			p_iline->il_tabs = true        ;
			p_iline->put_idata( tabsLine, Level ) ;
			il_its = data.insert( il_its, p_iline ) ;
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_TJ:
			Level++ ;
			dl   = getLine( itc->icmd_sURID )   ;
			tmp1 = data.at( dl )->get_idata()   ;
			data.at( dl )->set_deleted( Level ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				dl = getNextDataLine( dl, 'F' ) ;
				if ( dl == 0 ) { break ; }
				tmp1 = tmp1 + " " + strip( data.at( dl )->get_idata() ) ;
				data.at( dl )->set_deleted( Level ) ;
			}
			p_iline = new iline( taskid() ) ;
			p_iline->il_file = true         ;
			p_iline->put_idata( tmp1, Level ) ;
			il_its  = getLineItr( itc->icmd_sURID ) ;
			il_its++ ;
			il_its  = data.insert( il_its, p_iline ) ;
			placeCursor( p_iline->il_URID, 4, k ) ;
			break ;

		case LC_TS:
			Level++ ;
			il_its  = getLineItr( itc->icmd_sURID ) ;
			splitOK = textSplitData( (*il_its)->get_idata(), tmp1, tmp2 ) ;
			if ( aURID == (*il_its)->il_URID && aCol > CLINESZ )
			{
				if ( (*il_its)->il_file && !(*il_its)->il_deleted )
				{
					(*il_its)->put_idata( tmp1, Level ) ;
					p_iline = new iline( taskid() ) ;
					p_iline->il_file  = true ;
					p_iline->il_nisrt = true ;
					p_iline->put_idata( maskLine )  ;
					il_its++ ;
					il_its  = data.insert( il_its, p_iline ) ;
					if ( splitOK )
					{
						p_iline = new iline( taskid() )  ;
						p_iline->il_file = true ;
						p_iline->put_idata( tmp2, Level ) ;
						il_its++ ;
						il_its = data.insert( il_its, p_iline ) ;
					}
				}
			}
			else
			{
				p_iline = new iline( taskid() ) ;
				p_iline->il_file  = true ;
				p_iline->il_nisrt = true ;
				p_iline->put_idata( maskLine ) ;
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_TX:
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_bod )     { break                     ; }
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				(*il_its)->il_excl = !(*il_its)->il_excl ;
				il_its++ ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_TXX:
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted  ) { continue ; }
				(*il_its)->il_excl = !(*il_its)->il_excl ;
			}
			rebuildZAREA = true ;
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			break ;

		case LC_UC:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				(*il_its)->put_idata( upper( (*il_its)->get_idata() ), Level ) ;
				il_its++ ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_UCC:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue ; }
				(*il_its)->put_idata( upper( (*il_its)->get_idata() ), Level ) ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_X:
			il_its = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_its)->il_bod )     { break                     ; }
				if ( (*il_its)->il_deleted ) { j-- ; il_its++ ; continue ; }
				(*il_its)->il_excl = true ;
				il_its++ ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_XP:
			il_its = getLineItr( itc->icmd_sURID ) ;
			j = (*il_its)->get_idata().find_first_not_of( ' ' ) ;
			(*il_its)->il_excl = true ;
			while ( j != string::npos )
			{
				il_its++ ;
				if (  (*il_its)->il_bod )     { break    ; }
				if (  (*il_its)->il_deleted ) { continue ; }
				if ( !(*il_its)->il_file )    { continue ; }
				k = (*il_its)->get_idata().find_first_not_of( ' ' ) ;
				if ( k != string::npos && k < j ) { break ; }
				(*il_its)->il_excl = true ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			break ;

		case LC_XX:
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted  ) { continue ; }
				(*il_its)->il_excl = true ;
			}
			rebuildZAREA = true ;
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			break ;

		case LC_S:
			j = 65535 ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			if ( !(*il_its)->il_excl ) { break ; }
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( il_it = il_its ; il_it != il_ite ; il_it++ )
			{
				if ( (*il_it)->il_deleted  ) { continue ; }
				k = (*il_it)->get_idata().find_first_not_of( ' ' ) ;
				if ( k != string::npos ) { j = min( j, k ) ; }
			}
			for ( il_it = il_its ; il_it != il_ite ; il_it++ )
			{
				if ( (*il_it)->il_deleted  ) { continue ; }
				k = (*il_it)->get_idata().find_first_not_of( ' ' ) ;
				if ( k == j || k == string::npos )
				{
					(*il_it)->il_excl = false ;
				}
			}
			break ;

		case LC_SRC:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_its = getValidDataLine( il_its ) ;
			(*il_its)->put_idata( rshiftCols( itc->icmd_Rpt, (*il_its)->get_idata()), Level ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_SRCC:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted  ) { continue ; }
				(*il_its)->put_idata( rshiftCols( itc->icmd_Rpt, (*il_its)->get_idata()), Level ) ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_SLC:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_its = getValidDataLine( il_its ) ;
			(*il_its)->put_idata( lshiftCols( itc->icmd_Rpt, (*il_its)->get_idata()), Level ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_SLCC:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue ; }
				(*il_its)->put_idata( lshiftCols( itc->icmd_Rpt, (*il_its)->get_idata()), Level ) ;
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_SRD:
			Level++ ;
			il_its  = getLineItr( itc->icmd_sURID ) ;
			il_its  = getValidDataLine( il_its ) ;
			shiftOK = rshiftData( itc->icmd_Rpt, (*il_its)->get_idata(), tmp1 ) ;
			if ( tmp1 != (*il_its)->get_idata() )
			{
				(*il_its)->put_idata( tmp1, Level ) ;
			}
			if ( !shiftOK ) { (*il_its)->il_error = true ; MSG = "PEDT013B" ; }
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_SRDD:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted  ) { continue ; }
				shiftOK = rshiftData( itc->icmd_Rpt, (*il_its)->get_idata(), tmp1 ) ;
				if ( tmp1 != (*il_its)->get_idata() )
				{
					(*il_its)->put_idata( tmp1, Level ) ;
				}
				if ( !shiftOK ) { (*il_its)->il_error = true ; MSG = "PEDT013B" ; }
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_SLD:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_its = getValidDataLine( il_its ) ;
			shiftOK = lshiftData( itc->icmd_Rpt, (*il_its)->get_idata(), tmp1 ) ;
			if ( tmp1 != (*il_its)->get_idata() )
			{
				(*il_its)->put_idata( tmp1, Level ) ;
			}
			if ( !shiftOK ) { (*il_its)->il_error = true ; MSG = "PEDT013B" ; }
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_SLDD:
			Level++ ;
			il_its = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_its != il_ite ; il_its++ )
			{
				if ( (*il_its)->il_deleted ) { continue ; }
				shiftOK = lshiftData( itc->icmd_Rpt, (*il_its)->get_idata(), tmp1 ) ;
				if ( tmp1 != (*il_its)->get_idata() )
				{
					(*il_its)->put_idata( tmp1, Level ) ;
				}
				if ( !shiftOK ) { (*il_its)->il_error = true ; MSG = "PEDT013B" ; }
			}
			il_its = getValidDataLine( il_its ) ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			break ;

		case LC_PASTE:
			Level++;
			getClipboard( vip ) ;
			il_its  = getLineItr( itc->icmd_sURID ) ;
			if ( itc->icmd_ABO == 'B' ) { il_its = getLineBeforeItr( itc->icmd_sURID ) ; }
			for ( j = 0 ; j < vip.size() ; j++ )
			{
				p_iline          = new iline( taskid() ) ;
				p_iline->il_file = true ;
				p_iline->put_idata( vip[ j ].ip_data, Level ) ;
				il_its++ ;
				il_its = data.insert( il_its, p_iline ) ;
			}
			vreplace( "ZEDLNES", d2ds( vip.size() ) ) ;
			vreplace( "CLIPNAME", clipboard ) ;
			MSG  = "PEDT012D"    ;
			ZCMD = ""            ;
			placeCursor( (*il_its)->il_URID, 1 ) ;
			rebuildZAREA = true  ;
			fileChanged  = true  ;

		default:
		       log( "E", "Invalid line command "<< itc->icmd_CMDSTR<< endl );
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

	if ( ZVERB == "DOWN" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			topLine = (data.size() > ZAREAD) ? data.size() - ZAREAD : 0 ;
		}
		else
		{
			t = 0 ;
			for ( ; topLine < ( data.size() - 1 ) ; topLine++ )
			{
				if ( data.at( topLine )->il_deleted ) { continue ; }
				if ( data.at( topLine )->il_excl )    { topLine = getLastEX( topLine ) ; }
				t++ ;
				if ( t > ZSCROLLN ) break ;
			}
		}
	}
	else if ( ZVERB == "UP" )
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
				if ( data.at( topLine )->il_excl )    { topLine = getFirstEX( topLine ) ; }
				t++ ;
				if ( t > ZSCROLLN ) break ;
			}
		}
	}
	else if ( ZVERB == "LEFT" )
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
	else if ( ZVERB == "RIGHT" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			startCol = maxCol - ZAREAW ;
			if ( startCol < 1 ) { startCol = 1 ; }
		}
		else
		{
			if ( ZSCROLLA == "CSR" )
				startCol = startCol + ZSCROLLN - CLINESZ - 1 ;
			else
				startCol = startCol + ZSCROLLN ;
			if ( startCol < 1 ) { startCol = 1 ; }
		}
	}
	else if ( ZVERB == "RCHANGE" )
	{
		w1 = upper( word( ZCMD, 1 ) ) ;
		if ( findword( w1, "C CHA CHG CHANGE" ) )
		{
			if ( !setFindChangeExcl( 'C' ) ) { return ; }
		}
		else if ( !find_parms.fcx_cset ) { MSG = "PEDT012N"; return ; }
		if ( aURID == find_parms.fcx_URID    &&
			      find_parms.fcx_success &&
			      find_parms.fcx_offset == (aCol + startCol - CLINESZ - 2) )
		{
			actionChange() ;
			moveColumn( find_parms.fcx_offset ) ;
			placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
		}
		else
		{
			actionFind() ;
			if ( find_parms.fcx_error ) { return ; }
			if ( find_parms.fcx_success )
			{
				actionChange() ;
				moveColumn( find_parms.fcx_offset ) ;
				placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
			}
		}
		if ( find_parms.fcx_success )
		{
			moveColumn( find_parms.fcx_offset ) ;
			placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
			if ( !URIDonScreen( find_parms.fcx_URID ) )
			{
				topLine = getLine( find_parms.fcx_URID ) ;
				topLine = getPrevDataLine( topLine ) ;
			}
			MSG  = "PEDT012L" ;
			TYPE = typList[ find_parms.fcx_mtch ] ;
			if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
			else                                { STR = find_parms.fcx_ostring ; }
		}
		else
		{
			TYPE = typList[ find_parms.fcx_mtch ] ;
			if ( find_parms.fcx_rstring != ""  ) { STR = find_parms.fcx_rstring ; }
			else                                 { STR = find_parms.fcx_ostring ; }
			if ( find_parms.fcx_dir     == 'N' ) { MSG = "PEDT012V"             ; }
			else                                 { MSG = "PEDT012W"             ; }
		}
		find_parms.fcx_URID = 0 ;
		ZCMD = ""           ;
		rebuildZAREA = true ;
	}
	else if ( ZVERB == "RFIND" )
	{
		w1 = upper( word( ZCMD, 1 ) ) ;
		if ( findword( w1, "C CHA CHG CHANGE" ) )
		{
			if ( !setFindChangeExcl( 'C' ) ) { return ; }
		}
		else if ( w1 == "F" || w1 == "FIND" )
		{
			if ( !setFindChangeExcl( 'F' ) ) { return ; }
		}
		else if ( !find_parms.fcx_fset ) { MSG = "PEDT012M"; return ; }
		actionFind() ;
		if ( find_parms.fcx_error ) { return ; }
		if ( find_parms.fcx_success )
		{
			moveColumn( find_parms.fcx_offset ) ;
			placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset ) ;
			if ( !URIDonScreen( find_parms.fcx_URID ) )
			{
				topLine = getLine( find_parms.fcx_URID ) ;
				topLine = getPrevDataLine( topLine ) ;
			}
			MSG  = "PEDT012G" ;
			TYPE = typList[ find_parms.fcx_mtch ] ;
			if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
			else                                { STR = find_parms.fcx_ostring ; }
		}
		else
		{
			TYPE = typList[ find_parms.fcx_mtch ] ;
			if ( find_parms.fcx_rstring != ""  ) { STR = find_parms.fcx_rstring ; }
			else                                 { STR = find_parms.fcx_ostring ; }
			if ( find_parms.fcx_dir     == 'N' ) { MSG = "PEDT012V"             ; }
			else                                 { MSG = "PEDT012W"             ; }
		}
		ZCMD = ""           ;
		rebuildZAREA = true ;
	}
	else if ( aRow > 0 && s2data.at( aRow-1 ).ipos_URID == 0 )
	{
		placeCursor( 0, 0 ) ;
	}
	else if ( aRow > 0 && s2data.at( aRow-1 ).ipos_URID > 0 )
	{
		if ( profSTabs && tabsLine != "" && getTabLocation( p1 ) )
		{
			if ( p1 > aCol+startCol-CLINESZ-2 )
			{
				row = aRow ;
				placeCursor( row, 4, p1 ) ;
			}
			else
			{
				t = getNextDataLine( s2data.at( aRow-1 ).ipos_URID ) ;
				placeCursor( t, 4, p1 ) ;
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
				rebuildZAREA = true ;
			}
			else
			{
				row = aRow + 1 ;
				dl  = s2data.at( aRow-1 ).ipos_line ;
				p1  = data.at( dl )->get_idata().find_first_not_of( ' ', startCol-1 ) ;
				dl  = s2data.at( aRow ).ipos_line ;
				if ( data.at( dl )->il_bod || dl == 0 )
				{
					placeCursor( 0, 0 ) ;
					return ;
				}
				p2  = data.at( dl )->get_idata().find_first_not_of( ' ', startCol-1 ) ;
				if ( p2 == string::npos )
				{
					if ( p1 == string::npos ) { placeCursor( row, 2 )     ; }
					else                      { placeCursor( row, 4, p1 ) ; }
				}
				else { placeCursor( row, 4, p2 ) ; }
			}
		}
	}
}


bool PEDIT01::checkLineCommands()
{
	// For each line in the data vector, check the line commands entered and build the command vector
	// Treat single commands on an exluded block as a block command (except XOnly commands, ie F, L and S)

	// cmd   - command block
	// tcmd  - temporary command block for storing cmd when complete, and a non-abo command entered
	//         ie. Block/single commands without A/B/O bewteen block/single commands with A/B/O
	// abo   - contains a, b or o M/C positions, including OO block command, and xK commands
	// tabo  - temporary abo command block used during AK, BK, OK and OOK operations (pushed to tabos stack)
	// tabos - command block stack for storing tabo for AK, BK, OK and OOK operations

	// Check for overlapping commands (eg. Cn, Dn, Mn, On ... don't conflict with the next command)

	icmd cmd  ;
	icmd tcmd ;
	icmd abo  ;
	icmd tabo ;
	stack<icmd> tabos ;

	int rept  ;
	int adist ;
	int rdist ;

	string lcc ;
	string lc2 ;

	bool cmd_inuse    ;
	bool abo_inuse    ;
	bool cmd_complete ;
	bool abo_complete ;
	bool abo_block    ;
	bool abo_k        ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	icmds.clear() ;

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
			MSG = "PEDT013C" ;
			break            ;
		}
		rdist = -1 ;
		if ( (*it)->il_lcc[ 0 ] == '.' )
		{
			if ( !checkLabel( (*it)->il_lcc ) ) { MSG = "PEDT014" ; return false ; }
			if ( !(*it)->il_file )              { MSG = "PEDT01E" ; return false ; }
			for ( itt = data.begin() ; itt != data.end() ; itt++ )
			{
				if ( (*itt)->il_label == (*it)->il_lcc ) { (*itt)->il_label = "" ; break ; }
			}
			(*it)->il_label = (*it)->il_lcc ;
			(*it)->clearLcc()    ;
			rebuildZAREA = true  ;
			continue             ;
		}
		if ( !formLineCmd( (*it)->il_lcc, (*it)->il_excl, lc2, rept ) )
		{
			if ( MSG == "" ) { MSG = "PEDT01Y" ; }
			break ;
		}
		if ( LineCMDS.find( lc2 ) == LineCMDS.end() )
		{
			MSG = "PEDT012" ;
			return false ;
		}
		if ( (*it)->isSpecial() && !findword( lc2, spllcmds ) ) { MSG = "PEDT013" ; break ; }
		if ( (*it)->il_tod && !findword( lc2, todlcmds ) )      { MSG = "PEDT013" ; break ; }
		if ( (*it)->il_bod && !findword( lc2, bodlcmds ) )      { MSG = "PEDT013" ; break ; }
		if ( findword( lc2, ABOList ) )
		{
			if ( abokLCMDS.find( lc2 ) != abokLCMDS.end() )
			{
				abo_k = true ;
				lc2   = abokLCMDS[ lc2 ] ;
			}
			else
			{
				abo_k = false ;
			}
			if ( !abo_inuse )
			{
				if ( cmd_inuse && !cmd_complete ) { MSG = "PEDT01Y" ; break ; }
				abo.icmd_ABO   = lc2[ 0 ]       ;
				if ( lc2 == "A" && (*it)->il_excl )
				{
					abo.icmd_sURID = getLastEX( (*it)->il_URID ) ;
				}
				else
				{
					abo.icmd_sURID = (*it)->il_URID ;
				}
				abo.icmd_Rpt   = rept ;
				abo_inuse      = true ;
				if ( findword( lc2, ABOBlock ) )
				{
					abo_block = true ;
				}
				else
				{
					if ( lc2 == "O" )
					{
						if ( (*it)->il_excl )
						{
							abo.icmd_OSize = getEXBlock( abo.icmd_sURID ) ;
							if ( rept != -1 )
							{
								abo.icmd_OSize += rept - 1 ;
							}
						}
						else
						{
							abo.icmd_OSize = rept ;
						}
					}
					if ( abo_k ) { tabos.push( abo ) ; abo_inuse = false ; abo.icmd_clear() ; }
					else         { abo_complete = true ; }
				}
			}
			else
			{
				if ( !findword( lc2, ABOBlock ) ||
				     !abo_block                 ||
				      abo.icmd_eURID > 0        ||
				      lc2[ 0 ] != abo.icmd_ABO )  { MSG = "PEDT01Y" ; break ; }
				if ( (*it)->il_excl )
				{
					abo.icmd_eURID = getLastEX( (*it)->il_URID ) ;
				}
				else
				{
					abo.icmd_eURID = (*it)->il_URID ;
				}
				abo.icmd_OSize = getRangeSize( abo.icmd_sURID, abo.icmd_eURID ) ;
				if ( abo_k ) { tabos.push( abo ) ; abo_inuse = false ; abo.icmd_clear() ; }
				else         { abo_complete = true ; }
			}
		}
		else if ( findword( lc2, blkcmds ) )
		{
			if ( cmd_inuse && !cmd_complete && cmd.icmd_CMD != LineCMDS[ lc2 ] )
			{
				MSG = "PEDT01Y" ;
				break ;
			}
			if ( !cmd_inuse )
			{
				cmd.icmd_CMD    = LineCMDS[ lc2 ] ;
				cmd.icmd_CMDSTR = lc2 ;
				cmd.icmd_sURID  = (*it)->il_URID  ;
				cmd.icmd_Rpt    = rept ;
				cmd_inuse       = true ;
			}
			else
			{
				if ( (cmd.icmd_Rpt != -1 && rept != -1) && cmd.icmd_Rpt != rept )
				{
					MSG = "PEDT011B" ;
					break ;
				}
				if ( (cmd_complete && findword( lc2, ABOReq ) ) )
				{
					MSG = "PEDT01Y" ;
					break ;
				}
				if ( cmd_complete )
				{
					tcmd = cmd ;
					cmd.icmd_clear() ;
					cmd.icmd_CMD    = LineCMDS[ lc2 ] ;
					cmd.icmd_CMDSTR = lc2 ;
					cmd.icmd_sURID  = (*it)->il_URID  ;
					cmd.icmd_Rpt    = rept  ;
					cmd_inuse       = true  ;
					cmd_complete    = false ;
				}
				else
				{
					if ( cmd.icmd_Rpt == -1 ) { cmd.icmd_Rpt = rept ; }
					if ( (*it)->il_excl )
					{
						cmd.icmd_eURID = getLastEX( (*it)->il_URID ) ;
					}
					else
					{
						cmd.icmd_eURID = (*it)->il_URID ;
					}
					if ( findword( lc2, ABOReq ) )
					{
						cmd_complete = true ;
					}
					else
					{
						icmds.push_back( cmd ) ;
						if ( tcmd.icmd_sURID > 0 )
						{
							cmd = tcmd           ;
							cmd_inuse    = true  ;
							cmd_complete = true  ;
							tcmd.icmd_clear()    ;
						}
						else
						{
							cmd.icmd_clear()     ;
							cmd_inuse    = false ;
							cmd_complete = false ;
						}
					}
				}
			}
		}
		else
		{
			if ( findword( lc2, Chkdist ) ) { adist = 1 ; rdist = rept ; }
			else                            { rdist = -1               ; }
			if ( (cmd_inuse && !cmd_complete) ||
			     (cmd_complete && findword( lc2, ABOReq ) ) )
			{
				MSG = "PEDT01Y" ;
				break ;
			}
			if ( cmd_complete )
			{
				tcmd = cmd ;
				cmd.icmd_clear() ;
			}
			cmd.icmd_sURID  = (*it)->il_URID ;
			cmd.icmd_Rpt    = rept ;
			cmd.icmd_CMDSTR = lc2  ;
			if ( (*it)->il_excl )
			{
				if ( !findword( lc2, XOnly ) )
				{
					cmd.icmd_CMDSTR += lc2 ;
				}
				cmd.icmd_eURID = getLastEX( (*it)->il_URID ) ;
			}
			cmd.icmd_CMD = LineCMDS[ cmd.icmd_CMDSTR ] ;
			if ( findword( lc2, ABOReq ) )
			{
				cmd_complete = true ;
				cmd_inuse    = true ;
			}
			else
			{
				icmds.push_back( cmd ) ;
				cmd.icmd_clear()       ;
				if ( tcmd.icmd_sURID > 0 ) { cmd = tcmd ; tcmd.icmd_clear() ; }
				else                       { cmd_complete = false           ; }
			}
		}
		if ( cmd_complete && abo_complete )
		{
			while ( !tabos.empty() )
			{
				tabo = cmd ;
				tabo.icmd_ABO   = tabos.top().icmd_ABO   ;
				tabo.icmd_dURID = tabos.top().icmd_sURID ;
				tabo.icmd_OSize = tabos.top().icmd_OSize ;
				if ( cmd.icmd_CMD == LC_M )
				{
					tabo.icmd_CMD    = LC_C ;
					tabo.icmd_CMDSTR = "C"  ;
				}
				else if ( cmd.icmd_CMD == LC_MM )
				{
					tabo.icmd_CMD    = LC_CC ;
					tabo.icmd_CMDSTR = "CC"  ;
				}
				icmds.push_back( tabo ) ;
				tabos.pop()              ;
			}
			cmd.icmd_ABO   = abo.icmd_ABO   ;
			cmd.icmd_dURID = abo.icmd_sURID ;
			cmd.icmd_OSize = abo.icmd_OSize ;
			icmds.push_back( cmd ) ;
			cmd.icmd_clear()     ;
			cmd_inuse    = false ;
			cmd_complete = false ;
			abo.icmd_clear()     ;
			abo_inuse    = false ;
			abo_complete = false ;
			abo_block    = false ;
		}
		else if ( cutActive && cmd_complete )
		{
			cmd.icmd_cut = true    ;
			icmds.push_back( cmd ) ;
			cmd.icmd_clear()     ;
			cmd_inuse    = false ;
			cmd_complete = false ;
			cutActive    = false ;
		}
		else if ( pasteActive && abo_complete )
		{
			abo.icmd_CMD = LC_PASTE ;
			icmds.push_back( abo )  ;
			abo.icmd_clear()     ;
			abo_inuse    = false ;
			abo_complete = false ;
			abo_block    = false ;
			pasteActive  = false ;
		}
	}
	if ( it != data.end() )
	{
		placeCursor( (*it)->il_URID, 1 ) ;
		return false ;
	}
	if (  cmd_inuse      ) { MSG = "PEDT01W"  ; return false ; }
	if (  abo_inuse      ) { MSG = "PEDT01Z"  ; return false ; }
	if ( !tabos.empty() )  { MSG = "PEDT01Z"  ; return false ; }
	if (  cutActive      ) { MSG = "PEDT012E" ; return false ; }
	if (  pasteActive    ) { MSG = "PEDT012F" ; return false ; }
	return true ;
}


void PEDIT01::actionUNDO()
{
	// Get the current data level from Global_Undo and call undo_idata() for all records
	// that match that level.  Move level from Global_Undo to Global_Redo

	// If any file lines have been undone, remove the top Global_File_level entry and
	// reset fileChanged indicator

	// If no un-done lines are visible on the screen, move the top line to the line before
	// the first un-done change (currently removed!)

	int  lvl  ;
	uint tTop ;

	bool moveTop ;
	bool isFile  ;

	vector<iline * >::iterator it ;

	if ( !undoON ) { MSG = "PEDT01U" ; return ; }

	it  = data.begin() ;
	lvl = (*it)->get_Global_Undo_level() ;
	if ( lvl < 1 )
	{
		MSG = "PEDT017" ;
		return ;
	}

	moveTop = true  ;
	isFile  = false ;
	tTop    = data.size() - 1 ;
	(*it)->move_Global_Undo2Redo() ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->get_idata_level() == lvl )
		{
			(*it)->undo_idata()    ;
			(*it)->il_undo = true  ;
			(*it)->il_redo = false ;
			if ( (*it)->il_file ) { isFile = true ; }
		 /*     if ( moveTop )
			{
				if ( URIDonScreen( (*it)->il_URID ) ) { moveTop = false ; }
				else
				{
					tTop = min( tTop, getLine( (*it)->il_URID ) ) ;
				}
			} */
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

   //   if ( moveTop ) { topLine = getPrevDataLine( tTop ) ; }
}


void PEDIT01::actionREDO()
{
	// Get the current data level from Global_Redo and call redo_idata() for all records
	// that match that level.  Move level from Global_Redo to Global_Undo

	// If any file lines have been redone, update the Global_File_level from the Global_Undo stack and
	// reset fileChanged indicator

	// If no re-done lines are visible on the screen, move the top line to the line before
	// the first re-done change (currently removed!)

	int  lvl  ;
	uint tTop ;

	bool moveTop ;
	bool isFile  ;

	vector<iline * >::iterator it ;

	if ( !undoON ) { MSG = "PEDT01U" ; return ; }

	it  = data.begin() ;
	lvl = (*it)->get_Global_Redo_level() ;
	if ( lvl < 1 )
	{
		MSG = "PEDT018" ;
		return ;
	}

	moveTop = true ;
	tTop    = data.size() - 1 ;
	(*it)->move_Global_Redo2Undo() ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->get_idata_Redo_level() == lvl )
		{
			(*it)->redo_idata()    ;
			(*it)->il_undo = false ;
			(*it)->il_redo = true  ;
			if ( (*it)->il_file ) { isFile = true ; }
		 /*     if ( moveTop )
			{
				if ( URIDonScreen( (*it)->il_URID ) ) { moveTop = false ; }
				else
				{
					tTop = min( tTop, getLine( (*it)->il_URID ) ) ;
				}
			} */
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

    //  if ( moveTop ) { topLine = getPrevDataLine( tTop ) ; }
}


bool PEDIT01::setFindChangeExcl( char type )
{
	int i        ;
	int p1       ;
	int p2       ;

	char c1      ;
	char c2      ;

	bool f_dir   ;
	bool f_excl  ;
	bool f_mtch  ;
	bool f_str1  ;
	bool f_str2  ;

	string delim ;
	string cmd   ;
	string rcmd  ;
	string w1    ;
	string pic   ;
	e_find t     ;
	c_range r    ;

	const string f_keywdir  = "NEXT PREV FIRST LAST ALL" ;
	const string f_keywexcl = "X NX" ;
	const string f_keywmtch = "CHARS PRE PREFIX SUF SUFFIX WORD" ;

	f_dir  = false ;
	f_excl = false ;
	f_mtch = false ;
	f_str1 = false ;
	f_str2 = false ;

	r.c_vlab = true ;
	r.c_vcol = true ;

	const char quote('\"')  ;
	const char apost('\'')  ;

	MSG  = "" ;
	cmd  = subword( ZCMD, 2 ) + " " ;
	if ( strip( cmd ) == "" ) { MSG = "PEDT012U" ; return false ; }
	rcmd = "" ;

	while ( true )
	{
		if ( cmd.size() == 0 ) { break ; }
		c1 = cmd[ 0 ] ;
		if ( c1 == ' ' ) { cmd.erase( 0, 1 ) ; continue ; }
		if ( cmd.size() > 1 ) { c2 = cmd.at( 1 ) ; }
		else                  { c2 = ' '         ; }
		if ( c1 == quote || c2 == quote || c1 == apost || c2 == apost )
		{
			if ( c1 != quote && c1 != apost ) { delim = c2 ; c1 = cmd[ 0 ] ; p1 = 2 ; }
			else                              { delim = c1 ; c1 = ' '      ; p1 = 1 ; }
			if ( !f_str1 )
			{
				f_str1 = true ;
				p2  = pos( delim, cmd, p1+1 ) ;
				if ( p2 == 0 ) { MSG = "PEDT01H" ; return false ; }
				c1 = toupper( c1 )            ;
				if ( p2 >= cmd.size() ) { c2 = ' ' ; }
				else                    { c2 = toupper( cmd.at( p2 ) ) ; }
				if      ( c1 == ' ' && c2 == ' ' ) { t.fcx_text   = true ; }
				else if ( c1 == 'T' && c2 == ' ' ) { t.fcx_text   = true ; }
				else if ( c1 == 'C' && c2 == ' ' ) { t.fcx_asis   = true ; }
				else if ( c1 == 'X' && c2 == ' ' ) { t.fcx_hex    = true ; }
				else if ( c1 == 'P' && c2 == ' ' ) { t.fcx_pic    = true ; }
				else if ( c1 == 'R' && c2 == ' ' ) { t.fcx_rreg   = true ; }
				else if ( c2 == 'T' && c1 == ' ' ) { t.fcx_text   = true ; }
				else if ( c2 == 'C' && c1 == ' ' ) { t.fcx_asis   = true ; }
				else if ( c2 == 'X' && c1 == ' ' ) { t.fcx_hex    = true ; }
				else if ( c2 == 'P' && c1 == ' ' ) { t.fcx_pic    = true ; }
				else if ( c2 == 'R' && c1 == ' ' ) { t.fcx_rreg   = true ; }
				else                               { MSG = "PEDT01I" ; return false ; }
				if ( cmd.size() > p2+1 && cmd.at ( p2 ) != ' ' && cmd.at( p2+1 ) != ' ' )
				{
					MSG = "PEDT01I" ; return false ;
				}
				if ( t.fcx_text )
				{
					t.fcx_string = upper( substr( cmd, (p1+1), (p2-p1-1) ) ) ;
				}
				else
				{
					t.fcx_string = substr( cmd, (p1+1), (p2-p1-1) ) ;
					t.fcx_asis   = true ;
				}
				cmd = substr( cmd, p2+2 ) ;
			}
			else if ( !f_str2 )
			{
				if ( type != 'C' ) { MSG = "PEDT01B" ; return false ; }
				f_str2 = true ;
				p2  = pos( delim, cmd, p1+1 ) ;
				if ( p2 == 0 ) { MSG = "PEDT01H" ; return false ; }
				c1 = toupper( c1 )            ;
				if ( p2 >= cmd.size() ) { c2 = ' ' ; }
				else                    { c2 = toupper( cmd.at( p2 ) ) ; }
				if ( c1 == ' ' && c2 == ' ' )      { t.fcx_ctext   = true ; }
				else if ( c1 == 'T' && c2 == ' ' ) { t.fcx_ctext   = true ; }
				else if ( c1 == 'C' && c2 == ' ' ) { t.fcx_casis   = true ; }
				else if ( c1 == 'X' && c2 == ' ' ) { t.fcx_chex    = true ; }
				else if ( c1 == 'P' && c2 == ' ' ) { t.fcx_cpic    = true ; }
				else if ( c2 == 'T' && c1 == ' ' ) { t.fcx_ctext   = true ; }
				else if ( c2 == 'C' && c1 == ' ' ) { t.fcx_casis   = true ; }
				else if ( c2 == 'X' && c1 == ' ' ) { t.fcx_chex    = true ; }
				else if ( c2 == 'P' && c1 == ' ' ) { t.fcx_cpic    = true ; }
				else                               { MSG = "PEDT01I" ; return false ; }
				if ( cmd.size() > p2+1 && cmd.at ( p2 ) != ' ' && cmd.at( p2+1 ) != ' ' )
				{
					MSG = "PEDT01I" ; return false ;
				}
				if ( profCaps && t.fcx_ctext )
				{
					t.fcx_cstring = upper( substr( cmd, (p1+1), (p2-p1-1) ) ) ;
				}
				else
				{
					t.fcx_cstring = substr( cmd, (p1+1), (p2-p1-1) ) ;
					t.fcx_casis   = true ;
				}
				cmd = substr( cmd, p2+2 ) ;
			}
			else  { MSG = "PEDT011" ; return false ; }
			continue ;
		}
		w1 = upper( word( cmd, 1 ) ) ;
		if ( wordpos( w1, f_keywdir ) > 0 )
		{
			if ( f_dir ) { MSG = "PEDT012J" ; return false ; }
			f_dir     = true    ;
			t.fcx_dir = w1[ 0 ] ;
		}
		else if ( wordpos( w1, f_keywexcl ) > 0 )
		{
			if ( f_excl || type == 'X' ) { MSG = "PEDT012J" ; return false ; }
			f_excl     = true    ;
			t.fcx_excl = w1[ 0 ] ;
		}
		else if ( wordpos( w1, f_keywmtch ) > 0 )
		{
			if ( f_mtch ) { MSG = "PEDT012J" ; return false ; }
			f_mtch     = true    ;
			t.fcx_mtch = w1[ 0 ] ;
		}
		else if ( datatype( w1, 'W' ) || w1[ 0 ] == '.' )
		{
			rcmd = rcmd + " " + w1 ;
		}
		else
		{
			if ( !f_str1 )
			{
				f_str1 = true ;
				if ( w1 == "*" )
				{
					if ( find_parms.fcx_string == "" ) { MSG = "PEDT01C" ; return false ; }
					t.fcx_string = find_parms.fcx_string ;
					t.fcx_text   = find_parms.fcx_text   ;
					t.fcx_asis   = find_parms.fcx_asis   ;
					t.fcx_hex    = find_parms.fcx_hex    ;
					t.fcx_pic    = find_parms.fcx_pic    ;
				}
				else
				{
					t.fcx_text   = true ;
					t.fcx_string = w1   ;
				}
			}
			else if ( !f_str2 )
			{
				if ( type != 'C' ) { MSG = "PEDT01B" ; return false ; }
				if ( profCaps )
				{
					t.fcx_cstring = w1 ;
				}
				else
				{
					t.fcx_cstring = word( cmd, 1 ) ;
				}
				t.fcx_ctext = true ;
				f_str2      = true ;
			}
			else  { MSG = "PEDT012J" ; return false ; }
		}
		cmd = subword( cmd, 2 ) ;
	}

	if ( t.fcx_string == "" ||
	   ( type == 'C' && t.fcx_cstring == "" && !f_str2 )) { MSG = "PEDT01D" ; return false ; }
	t.fcx_ostring = t.fcx_string ;

	if ( t.fcx_rreg ) { t.fcx_regreq = true ; }

	if ( !setCommandRange( rcmd, r ) ) { return false ; }
	t.fcx_slab = r.c_slab ;
	t.fcx_elab = r.c_elab ;
	t.fcx_scol = r.c_scol ;
	t.fcx_ecol = r.c_ecol ;
	t.fcx_ocol = r.c_ocol ;

	if ( t.fcx_hex )
	{
		if ( !isvalidHex( t.fcx_string ) )  { MSG = "PEDT01K" ; return false ; }
		t.fcx_string = xs2cs( t.fcx_string ) ;
		t.fcx_asis   = true                  ;
	}

	if ( t.fcx_pic )
	{
		pic = "" ;
		// =  any character                   .  invalid characters
		// @  alphabetic characters           -  non-numeric characters
		// #  numeric characters              <  lower case alphabetics
		// $  special characters              >  upper case alphabetics
		// ¬  non-blank characters            *  any number of non-blank characters
		for ( i = 0 ; i < t.fcx_string.size() ; i++ )
		{
			switch ( t.fcx_string[ i ] )
			{
				case '*':
					pic = pic + "[^[:blank:]]*" ;
					break ;
				case '=':
					pic = pic + "." ;
					break ;
				case '@':
					pic = pic + "[[:alpha:]]" ;
					break ;
				case '#':
					pic = pic + "[[:digit:]]" ;
					break ;
				case '$':
					pic = pic + "[^[:blank:]^[:alpha:]^[:digit:]]" ;
					break ;
				case '¬':
					pic = pic + "[^[:blank:]]" ;
					break ;
				case '.':
					pic = pic + "[^[:print:]]" ;
					break ;
				case '-':
					pic = pic + "[^[:digit:]]" ;
					break ;
				case '<':
					pic = pic + "[a-z]" ;
					break ;
				case '>':
					pic = pic + "[A-Z]" ;
					break ;
				default:
					pic = pic + t.fcx_string[ i ] ;
					break ;
			}
		}
		t.fcx_string = pic  ;
		t.fcx_regreq = true ;
	}

	switch ( t.fcx_mtch )
	{
		case 'P':
			t.fcx_string = "[^|^[:word:]](" + t.fcx_string + ")" ;
			t.fcx_regreq = true ;
			break ;
		case 'S':
			t.fcx_string = "(" + t.fcx_string + ")[^[:word:]|$]" ;
			t.fcx_regreq = true ;
			break ;
		case 'W':
			t.fcx_string = "[^|^[:word:]](" + t.fcx_string + ")[^[:word:]|$]" ;
			t.fcx_regreq = true ;
	}

	t.fcx_fset = true ;
	if ( type == 'C')
	{
		t.fcx_cset = true ;
		if ( t.fcx_dir == 'A' )
		{
			t.fcx_dir     = 'F'  ;
			t.fcx_chngall = true ;
		}
	}
	else if ( type == 'X')
	{
		t.fcx_exclude = true ;
	}
	find_parms = t ;
	return true ;
}


bool PEDIT01::setCommandRange( string s, c_range & t )
{
	// Extract the command range ( start/end labels and start/end columns or on column ) from
	// a command and return data in t.
	// If there is a start label, but not end label, set to .ZLAST
	// c_vlab true if labels can be entered on the command
	// c_vcol true if columns can be entered on the command

	int j ;
	int k ;
	int l ;

	string w  ;

	t.c_range_clear() ;

	while ( true )
	{
		w = word( s, 1 ) ;
		if ( w == "" ) { break ; }
		if ( datatype( w, 'W' ) )
		{
			if ( !t.c_vcol ) { MSG = "PEDT012T" ; return false ; }
			if ( t.c_scol != 0 && t.c_ecol != 0 ) { MSG = "PEDT019" ; return false ; }
			j = ds2d( w ) ;
			if ( j < 1 || j > 65535 ) { MSG = "PEDT01J" ; return false ; }
			t.c_scol == 0 ? t.c_scol = j : t.c_ecol = j ;
		}
		else if ( w[ 0 ] == '.' )
		{
			if ( !t.c_vlab ) { MSG = "PEDT012S" ; return false ; }
			if ( t.c_slab != "" && t.c_elab != "" )  { MSG = "PEDT01A" ; return false ; }
			t.c_slab == "" ? t.c_slab = w : t.c_elab = w ;
		}
		else { MSG = "PEDT011" ; return false ; }
		s = delword( s, 1, 1 ) ;
	}
	if ( t.c_scol != 0 && t.c_ecol == 0 ) { t.c_ocol = true ; }
	else if ( t.c_scol > t.c_ecol )
	{
		j          = t.c_scol ;
		t.c_scol = t.c_ecol ;
		t.c_ecol = j ;
	}
	if ( t.c_scol == 0 ) { t.c_scol = 1 ; }

	if ( t.c_slab == ".ZLAST" && t.c_elab == "" )   { MSG = "PEDT012R" ; return false ; }
	if ( t.c_slab != "" )
	{
		if ( t.c_elab == "" )       { t.c_elab = ".ZLAST"             ; }
		if ( t.c_slab == t.c_elab ) { MSG  = "PEDT01G" ; return false ; }
		j =  getLabelLine( t.c_slab ) ;
		if ( j == -1 ) { MSG = "PEDT01F" ; return false ; }
		t.c_sidx = j ;
		k =  getLabelLine( t.c_elab ) ;
		if ( k == -1 ) { MSG = "PEDT01F" ; return false ; }
		t.c_eidx = k ;
		if ( j > k )
		{
			w        = t.c_slab ;
			l        = t.c_sidx ;
			t.c_slab = t.c_elab ;
			t.c_sidx = t.c_eidx ;
			t.c_elab = w        ;
			t.c_eidx = l        ;
		}
	}
	return true ;
}


void PEDIT01::actionFind()
{
	// dl has to be a uint so the corect getNextDataLine routine is called ( int is for URID )
	// c1,c2 are the column limits of the find, set by bnds, col and initial cursor position

	uint dl  ;
	uint sdl ;
	uint edl ;

	int c1 ;
	int c2 ;
	int p1 ;
	int offset ;

	bool found  ;
	bool fline  ;
	bool skip   ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	boost::regex regexp ;
	boost::smatch results ;

	if ( !find_parms.fcx_success && find_parms.fcx_dir == 'N' )
	{
		find_parms.fcx_dir = 'F' ;
	}
	else if ( !find_parms.fcx_success && find_parms.fcx_dir == 'P' )
	{
		find_parms.fcx_dir = 'L' ;
	}

	find_parms.fcx_success = false ;
	find_parms.fcx_error   = false ;
	find_parms.fcx_rstring = "" ;
	find_parms.fcx_occurs  = 0  ;
	find_parms.fcx_URID    = 0  ;
	find_parms.fcx_lines   = 0  ;
	find_parms.fcx_offset  = 0  ;

	offset = 0 ;
	sdl    = 0 ;
	edl    = data.size() - 2 ;

	if ( aCol > 0 )
	{
		offset = aCol - CLINESZ - 1 + startCol ;
	}
	if ( find_parms.fcx_dir == 'F' || find_parms.fcx_dir == 'A' || find_parms.fcx_dir == 'L' )
	{
		offset = 0 ;
	}

	if ( find_parms.fcx_regreq )
	{
		try
		{
			if ( find_parms.fcx_asis )
			{
				regexp.assign( find_parms.fcx_string ) ;
			}
			else
			{
				regexp.assign( find_parms.fcx_string, boost::regex_constants::icase ) ;
			}
		}
		catch  ( boost::regex_error& e )
		{
			find_parms.fcx_error = true ;
			MSG = "PEDT01N" ;
			return          ;
		}
	}

	if      ( find_parms.fcx_dir == 'F' ) { dl = 1 ; }
	else if ( find_parms.fcx_dir == 'A' ) { dl = 1 ; }
	else if ( find_parms.fcx_dir == 'L' ) { dl = data.size()-2 ; }
	else if ( aRow == 0 )                 { dl = topLine       ; }
	else                                  { dl = s2data.at( aRow-1 ).ipos_line ; }

	if ( find_parms.fcx_slab != "" )
	{
		sdl = getLabelLine( find_parms.fcx_slab ) ;
	}
	if ( find_parms.fcx_elab != "" )
	{
		edl = getLabelLine( find_parms.fcx_elab ) ;
		if ( find_parms.fcx_dir == 'L' ) { dl = edl ; }
	}
	dl = max( sdl, dl ) ;
	dl = getValidDataLine( dl, 'F' ) ;
	found = false ;
	while ( true )
	{
		if ( dl < sdl || dl > edl ) { break ; }
		skip = false ;
		c1   = offset;
		c2   = data[ dl ]->get_idata().size() - 1 ;

		if ( (find_parms.fcx_excl == 'X' && !data[ dl ]->il_excl) ||
		     (find_parms.fcx_excl == 'N' &&  data[ dl ]->il_excl) )
		{
			skip = true ;
		}

		if ( find_parms.fcx_scol > c1+1 )                            { c1 = find_parms.fcx_scol - 1 ; }
		if ( find_parms.fcx_ecol > 0 && c2 > find_parms.fcx_ecol-1 ) { c2 = find_parms.fcx_ecol - 1 ; }

		if ( LeftBnd  > c1+1 )                 { c1 = LeftBnd  - 1 ; }
		if ( RightBnd > 0 && RightBnd < c2+1 ) { c2 = RightBnd - 1 ; }

		if ( skip || c1 >= c2 || offset > c2 )
		{
			if ( find_parms.fcx_dir == 'L' || find_parms.fcx_dir == 'P' )
			{
				dl = getPrevDataLine( dl, 'F' ) ;
			}
			else
			{
				dl = getNextDataLine( dl, 'F' ) ;
			}
			if ( dl < 1 || dl > edl ) { break ; }
			offset = 0 ;
			continue ;
		}

		if ( c1 > data[ dl ]->get_idata().size() -1 ) { abend() ; }
		if ( c2 > data[ dl ]->get_idata().size() -1 ) { abend() ; }
		if ( c1 < 0  ) { abend() ; }
		if ( c2 < 0  ) { abend() ; }
		if ( c1 > c2 ) { abend() ; }

		if ( find_parms.fcx_regreq )
		{
			fline  = true  ;
			found  = false ;
			itss   = data[ dl ]->get_idata().begin() ;
			itse   = itss  ;
			advance( itss, c1 ) ;
			advance( itse, c2 ) ;
			if ( find_parms.fcx_ocol )
			{
				if ( regex_search( itss, itss, results, regexp ) )
				{
					find_parms.fcx_rstring = results[ 0 ] ;
					if ( itss == results[0].first )
					{
						found = true ;
						p1    = find_parms.fcx_scol-1  ;
						data.at( dl )->il_excl = find_parms.fcx_exclude ;
					}
				}
			}
			else
			{
				if ( find_parms.fcx_dir == 'P' || find_parms.fcx_dir == 'L' || find_parms.fcx_dir == 'A' )
				{
					while ( regex_search( itss, itse, results, regexp ) )
					{
						found = true ;
						data.at( dl )->il_excl = find_parms.fcx_exclude ;
						find_parms.fcx_rstring = results[ 0 ] ;
						for ( p1 = c1 ; itss != results[0].first ; itss++ ) { p1++ ; }
						c1   = p1 + 1 ;
						itss = results[0].first ;
						itss++ ;
						if ( fline ) { fline = false ; find_parms.fcx_lines++ ; }
						find_parms.fcx_occurs++ ;
						if ( find_parms.fcx_dir == 'A' && find_parms.fcx_URID == 0 )
						{
							find_parms.fcx_URID   = data.at( dl )->il_URID ;
							find_parms.fcx_offset = p1 ;
						}
					}
				}
				else
				{
					if ( boost::regex_search( itss, itse, results, regexp ) )
					{
						found = true ;
						data.at( dl )->il_excl = find_parms.fcx_exclude ;
						find_parms.fcx_rstring = results[ 0 ] ;
						for ( p1 = c1 ; itss  != results[0].first ; itss++ ) { p1++ ; }
					}
				}
			}
			if ( found && find_parms.fcx_URID == 0 )
			{
				find_parms.fcx_URID   = data.at( dl )->il_URID ;
				find_parms.fcx_offset = p1 ;
			}
		}
		else
		{
			fline = true ;
			while ( true )
			{
				found  = false ;
				if ( find_parms.fcx_dir == 'P' || find_parms.fcx_dir == 'L' )
				{
					if ( find_parms.fcx_asis )
					{
						p1 = data[ dl ]->get_idata().rfind( find_parms.fcx_string, c2 ) ;
					}
					else
					{
						p1 = upper( data[ dl ]->get_idata()).rfind( find_parms.fcx_string, c2 ) ;
					}
					c2 = p1 - 1 ;
				}
				else
				{
					if ( find_parms.fcx_asis )
					{
						p1 = data[ dl ]->get_idata().find( find_parms.fcx_string, c1 )         ;
					}
					else
					{
						p1 = upper( data[ dl ]->get_idata()).find( find_parms.fcx_string, c1 ) ;
					}
					c1 = p1 + 1 ;
				}
				if ( p1 != string::npos )
				{
					if ( find_parms.fcx_ocol )
					{
						if ( p1 == find_parms.fcx_scol-1 ) { found = true ; }
					}
					else if ( find_parms.fcx_dir == 'P' || find_parms.fcx_dir == 'L' )
					{
						if ( p1 >= c1 ) { found = true ; }
					}
					else
					{
						if ( p1 <= c2 ) { found = true ; }
					}
				}
				if ( found && find_parms.fcx_URID == 0 )
				{
					find_parms.fcx_URID   = data.at( dl )->il_URID ;
					find_parms.fcx_offset = p1 ;
				}
				if ( found )
				{
					data.at( dl )->il_excl = find_parms.fcx_exclude ;
				}
				if ( find_parms.fcx_dir != 'A' || !found ) { break ; }
				find_parms.fcx_occurs++ ;
				if ( fline ) { fline = false ; find_parms.fcx_lines++ ; }
			}
		}
		if      ( find_parms.fcx_dir == 'A' ) { dl = getNextDataLine( dl, 'F' ) ; }
		else if ( find_parms.fcx_dir == 'F' ) { if ( found ) { find_parms.fcx_dir = 'N' ; break ; } ; dl = getNextDataLine( dl, 'F' ) ; }
		else if ( find_parms.fcx_dir == 'L' ) { if ( found ) { find_parms.fcx_dir = 'P' ; break ; } ; dl = getPrevDataLine( dl, 'F' ) ; }
		else if ( find_parms.fcx_dir == 'N' ) { if ( found ) { break ; } ; dl = getNextDataLine( dl, 'F' ) ; }
		else if ( find_parms.fcx_dir == 'P' ) { if ( found ) { break ; } ; dl = getPrevDataLine( dl, 'F' ) ; }
		if ( dl == 0 || dl < sdl || dl > edl ) { break ; }
		offset = 0 ;
	}
	find_parms.fcx_success = found ;
}


void PEDIT01::actionChange()
{
	//  If updating the same line multiple time with the same Level (eg. change ALL command)
	//  update idata stack in-place

	int l       ;
	string temp ;

	l    = getLine( find_parms.fcx_URID ) ;
	temp = data.at( l )->get_idata()      ;
	temp.replace( find_parms.fcx_offset, find_parms.fcx_string.size(), find_parms.fcx_cstring ) ;
	if ( data.at( l )->get_idata_level() == Level )
	{
		data.at( l )->put_idata( temp ) ;
	}
	else
	{
		data.at( l )->put_idata( temp, Level ) ;
	}
	data.at( l )->il_chg = true ;
	fileChanged = true ;
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
	//                   2 Use Row to place cursor

	// cursorPlaceType   1 First char of the line command area
	//                   2 First char of data area
	//                   3 First non-blank char of data area (after startCol)
	//                   4 Use position in cursorPlaceOff
	// cursorPlaceOff    Offset from start of the data line (adjust for command line and startCol later)

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


void PEDIT01::storeCursor( int URID, int pt, int offset )
{
	cursorPlaceUsing = 1      ;
	if ( URID == 0 ) { cursorPlaceHome  = true ; }
	else             { cursorPlaceURID  = URID ; }
	cursorPlaceRow   = 0      ;
	cursorPlaceType  = pt     ;
	cursorPlaceOff   = offset ;
}


void PEDIT01::positionCursor()
{
	int i  ;
	int p  ;
	int dl ;
	int o  ;
	int screenLine ;

	const char din  = '\01' ;
	const char dout = '\02' ;

	if ( cursorPlaceHome ) { CURFLD = "ZCMD" ; CURPOS = 1 ; return ; }

	screenLine = -1 ;

	switch ( cursorPlaceUsing )
	{
		case 1:
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
			break ;
		default:
			return ;
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
			dl = s2data.at( screenLine ).ipos_line ;
			p  = data.at( dl )->get_idata().find_first_not_of( ' ', startCol-1 ) ;
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
	}
	ZSHADOW.replace( ZAREAW*screenLine+1-1, slr.size(), slr ) ;
	rebuildShadow = true ;
	if ( cursorPlaceType == 1 ) { return ; }

	for ( i = CURPOS-1 ; i < ZASIZE ; i++ )
	{
		if ( ZAREA[ i ] == ' '  ||
		     ZAREA[ i ] == din  ||
		     ZAREA[ i ] == dout ) { break ; }
		ZSHADOW.replace( i, 1, 1, B_WHITE ) ;
	}
	for ( i = CURPOS-1 ; i > 0 ; i-- )
	{
		if ( ZAREA[ i ] == ' '  ||
		     ZAREA[ i ] == din  ||
		     ZAREA[ i ] == dout ) { break ; }
		ZSHADOW.replace( i, 1, 1, B_WHITE ) ;
	}
}


bool PEDIT01::getLabelItr( string label, vector<iline * >::iterator & it , int & posn )
{
	posn = 0 ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_label == label ) { return true ; }
		posn++ ;
	}
	return false ;
}


int PEDIT01::getLabelLine( string label )
{
	// Return index of the data vector corresponding to the label

	int posn ;

	vector<iline *>::iterator it ;

	if      ( label == ".ZFIRST" ) { return 0               ; }
	else if ( label == ".ZLAST" )  { return data.size() - 2 ; }

	posn = 0 ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_label == label ) { return posn ; }
		posn++ ;
	}
	return -1 ;
}


bool PEDIT01::formLineCmd( string cmd, bool excl, string & lc2, int & rept)
{
	// Split line command into the command (string) and repeat value (int), if allowed, and return in lc2 and retp
	// A 'rept' of -1 means it has not been entered

	// Stip off any remaining leading "- - - " for excluded lines

	int    j ;
	string t ;

	t.assign( 6, ' ' )   ;
	lc2.assign( 6, ' ' ) ;

	if ( excl )
	{
		for ( j = 0 ; j < cmd.size() ; j++ )
		{
			if      ( j%2 == 0 && cmd[ j ] == '-' ) { cmd[ j ] = ' ' ; }
			else if ( j%2 == 1 && cmd[ j ] == ' ' ) {                ; }
			else { break ; }
		}
	}

	for ( j = 0 ; j < cmd.size() ; j++ )
	{
		if ( isdigit( cmd[ j ] ) ) { t[ j ]   = cmd[ j ] ; }
		else                       { lc2[ j ] = cmd[ j ] ; }
	}

	t   = strip( t )   ;
	lc2 = strip( lc2 ) ;

	if ( aliasLCMDS.find( lc2 ) != aliasLCMDS.end() )
	{
		lc2 = aliasLCMDS[ lc2 ] ;
	}

	if ( t != "" && !findword( lc2, ReptOK ) )
	{
		MSG = "PEDT011A" ;
		return false     ;
	}

	if ( t != "" )
	{
		if ( datatype( t, 'W' ) ) { rept = ds2d( t ) ; }
		else                      { return false     ; }
	}
	else { rept = -1 ; }
	return true ;
}



vector<iline * >::iterator PEDIT01::getLineItr( int URID )
{
	// Return the iterator for a URID

	vector<iline * >::iterator it ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_URID == URID ) { return it ; }
	}
}


vector<iline * >::iterator PEDIT01::getLineBeforeItr( int URID )
{
	// Return the iterator for the line before a URID.  This may be a logically deleted line

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_URID == URID ) { return itt ; }
		itt = it ;
	}
}


int PEDIT01::getDataBlock( int URID )
{
	// Return the number of lines in an exluded block given any URID within that block
	// This does include logically deleted lines

	int bklines ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator its ;
	vector<iline * >::iterator ite ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if (  (*it)->il_deleted )      { continue ; }
		if ( !(*it)->il_excl    )      { its = it ; }
		if (  (*it)->il_URID == URID ) { break    ; }
	}
	its = getNextDataLine( its ) ;
	it++ ;
	for ( ; it != data.end() ; it++ )
	{
		if (  (*it)->il_deleted )  { continue         ; }
		if ( !(*it)->il_excl    )  { ite = it ; break ; }
	}

	bklines = 0 ;
	for ( it = its ; it != ite ; it++ )
	{
		bklines++ ;
	}
	return bklines ;
}


uint PEDIT01::getFirstEX( uint dl )
{
	// Return the line number of the first excluded line in a block

	int i ;

	for ( ; dl > 0 ; dl-- )
	{
		if ( data.at( dl )->il_deleted ) { continue ; }
		if ( data.at( dl )->il_excl )    { i = dl   ; }
		else                             { break    ; }
	}
	return i ;
}


int PEDIT01::getLastEX( int URID )
{
	// Return the URID of the last excluded line in a block

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_URID == URID ) { break ; }
	}
	itt = it ;
	it++     ;
	for ( ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_excl && !(*it)->il_deleted ) { break ; }
		itt = it ;
	}
	return (*itt)->il_URID ;

}


uint PEDIT01::getLastEX( uint dl )
{
	// Return the line number of the last excluded line in a block

	int i ;

	for ( ; dl < data.size() ; dl++ )
	{
		if (  data.at( dl )->il_deleted ) { continue ; }
		if (  data.at( dl )->il_excl )    { i = dl   ; }
		else                              { break    ; }
	}
	return i ;
}


int PEDIT01::getEXBlock( int URID )
{
	// Return the number of lines in an exluded block given any URID within that block
	// This does not included logically deleted lines

	int exlines ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator its ;
	vector<iline * >::iterator ite ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if (  (*it)->il_deleted )  { continue ; }
		if ( !(*it)->il_excl    )  { its = it ; }
		if (  (*it)->il_URID == URID ) { break ; }
	}
	it++ ;
	for ( ; it != data.end() ; it++ )
	{
		if (  (*it)->il_deleted )  { continue         ; }
		if ( !(*it)->il_excl    )  { ite = it ; break ; }
	}

	exlines = 0 ;
	its++ ;
	for ( it = its ; it != ite ; it++ )
	{
		if (  (*it)->il_deleted )  { continue ; }
		exlines++ ;
	}
	return exlines ;
}


uint PEDIT01::getLine( int URID )
{
	// Return the data line index dl, for a given URID ( URID must exist )

	uint dl ;

	for ( dl = 0 ; dl < data.size() ; dl++ )
	{
		if ( data.at( dl )->il_URID == URID ) { break ; }
	}
	return dl ;
}


int PEDIT01::getFileLine( int dl )
{
	// Return the file line index that corresponts to data line index dl in the data vector

	int i  ;
	int fl ;

	fl = 0 ;
	for ( i = 0 ; i < dl ; i++ )
	{
		if ( !data.at( i )->il_deleted &&
		      data.at( i )->il_file )   { fl++ ; }
	}
	return fl ;
}


int PEDIT01::getDataLine( int fl )
{
	// Return the data vector line that corresponts to line fl in the file
	// If not found, return bottom-of-data line

	int dl ;
	int j  ;

	j = 0 ;

	for ( dl = 0 ; dl < data.size() ; dl++ )
	{
		if ( !data.at( dl )->il_deleted &&
		      data.at( dl )->il_file )   { j++ ; }
		if ( fl == j ) { break ; }
	}
	if ( dl == data.size() ) { dl-- ; }
	return dl ;
}


int PEDIT01::getNextDataLine( int URID )
{
	// Return the next non-deleted data vector line URID that corresponts to the file line after URID

	bool found ;
	vector<iline * >::iterator it ;

	found = false ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_deleted || !(*it)->il_file ) { continue ; }
		if ( found ) { return (*it)->il_URID ; }
		if ( (*it)->il_URID == URID ) { found = true ; }
	}
	return 0 ;
}


uint PEDIT01::getNextDataLine( uint l )
{
	// Return the next non-deleted data vector line that corresponts to the line after l.

	for ( l++ ; l < data.size() ; l++ )
	{
		if ( !data.at( l )->il_deleted ) { break ; }
	}
	return l ;
}


uint PEDIT01::getNextDataLine( uint l, char ch )
{
	// Return the next non-deleted data vector line that corresponts to the line after l.
	// *File lines only*.  Return 0 if end of data reached

	for ( l++ ; l < data.size() ; l++ )
	{
		if ( !data.at( l )->il_deleted &&
		      data.at( l )->il_file ) { break    ; }
		if (  data.at( l )->il_bod )  { return 0 ; }
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


uint PEDIT01::getPrevDataLine( uint l, char ch )
{
	// Return the previous non-deleted data vector line that corresponts to the line before l.
	// *File lines only*

	if ( l == 0 ) { return 0 ; }

	for ( l-- ; l > 0 ; l-- )
	{
		if ( !data.at( l )->il_deleted &&
		      data.at( l )->il_file )   { break ; }
	}
	return l ;
}


vector<iline * >::iterator PEDIT01::getValidDataLine( vector<iline * >::iterator it )
{
	// Return a valid (ie. non-deleted) data line on or after iterator it

	for ( ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_deleted ) { break ; }
	}
	return it ;
}


uint PEDIT01::getValidDataLine( uint l )
{
	// Return a valid (ie. non-deleted) data line on or after line l

	for ( ; l < data.size() ; l++ )
	{
		if ( !data.at( l )->il_deleted ) { break ; }
	}
	return l ;
}


uint PEDIT01::getValidDataLine( uint l, char ch )
{
	// Return a valid (ie. non-deleted) data line on or after line l
	// *File lines only*

	for ( ; l < data.size() ; l++ )
	{
		if ( !data.at( l )->il_deleted &&
		      data.at( l )->il_file )   { return l ; }
	}
	return l ;
}


int PEDIT01::getNextSpecial( char dir, char t )
{
	// Return the next data vector line after l with flag il_????
	// Supported: C changed   ( flag il_chg   )
	//            K command   ( il_lcc not blank )
	//            E error     ( flag il_error )
	//            I info      ( flag il_info  )
	//            L label     ( il_label not blank )
	//            M message   ( flag il_msg   )
	//            N note      ( flag il_note  )
	//            U undo/redo ( flag il_undo or il_redo )
	//            X excluded  ( flag il_excl  )
	//            S any special line
	//            (col,prof,tabs,mask,bnds,msg,info or note)

	int  l     ;
	bool found ;

	switch ( dir )
	{
		case 'N': l = topLine + 1 ;
			  break       ;
		case 'P': l = topLine - 1 ;
			  break       ;
		case 'F': l = 1       ;
			  break       ;
		case 'L': l = data.size() - 1 ;
			  break       ;
	}

	if ( l < 0 )             { l = 0             ; }
	if ( l > data.size()-1 ) { l = data.size()-1 ; }

	found = false ;
	while ( true )
	{
		if ( !data.at( l )->il_deleted )
		{
			switch ( t )
			{
				case 'C': if ( data.at( l )->il_chg   ) { found = true ; }
					  break ;
				case 'K': if ( data.at( l )->il_lcc != "" ) { found = true ; }
					  break ;
				case 'E': if ( data.at( l )->il_error ) { found = true ; }
					  break ;
				case 'I': if ( data.at( l )->il_info  ) { found = true ; }
					  break ;
				case 'L': if ( data.at( l )->il_label != "" ) { found = true ; }
					  break ;
				case 'M': if ( data.at( l )->il_msg   ) { found = true ; }
					  break ;
				case 'N': if ( data.at( l )->il_note  ) { found = true ; }
					  break ;
				case 'U': if ( data.at( l )->il_undo  ||
					       data.at( l )->il_redo  ) { found = true ; }
					  break ;
				case 'X': if ( data.at( l )->il_excl  ) { found = true ; }
					  break ;
				case 'S': if ( data.at( l )->isSpecial() ) { found = true ; }
					  break ;
			}
		}
		if ( found ) { break ; }
		if ( dir == 'N' || dir == 'F' )
		{
			l++ ;
			if ( l >= data.size() ) { break ; }
		}
		else
		{
			l-- ;
			if ( l < 0 ) { break ; }
		}
	}
	if ( !found ) { return topLine ; }
	return l ;
}


int PEDIT01::getRangeSize( int sURID, int eURID )
{
	// Return size of eURID1-sURID1 (ignoring logically deleted lines)

	int sz ;

	vector<iline * >::iterator it ;

	sz = 0 ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_deleted )       { continue ; }
		if ( (*it)->il_URID == sURID ) { break    ; }
	}
	for ( ; it != data.end() ; it++ )
	{
		if ( (*it)->il_deleted ) { continue ; }
		sz++ ;
		if ( (*it)->il_URID == eURID ) { break ; }
	}
	return sz ;
}


bool PEDIT01::URIDonScreen( int URID )
{
	// Return true if the passed URID is on the screen (so we know if we need to reposition topLine)

	int i ;

	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		if ( s2data.at( i ).ipos_URID == URID ) { return true ; }
	}
	return false ;
}


int PEDIT01::truncateSize( int sURID )
{
	// Return number of valid lines (ie to the Bottom Of Data line) after passed URID
	// (including the line containing the URID)

	int i ;

	vector<iline * >::iterator it  ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_deleted ) { continue ; }
		if ( (*it)->il_URID == sURID ) { break ; }
	}
	i = 0 ;
	for ( ; it != data.end() ; it++ )
	{
		if ( (*it)->il_deleted ||
		    !(*it)->il_file ) { continue ; }
		if ( (*it)->il_bod  ) { break    ; }
		i++ ;
	}
	return i ;
}


string PEDIT01::overlay( string s1, string s2, bool & success )
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


void PEDIT01::copyPrefix( ipline & d, iline * & s )
{
	// Don't copy file line status (changed, error, undo and redo)
	d.ip_file  = s->il_file  ;
	d.ip_note  = s->il_note  ;
	d.ip_prof  = s->il_prof  ;
	d.ip_col   = s->il_col   ;
	d.ip_bnds  = s->il_bnds  ;
	d.ip_mask  = s->il_mask  ;
	d.ip_tabs  = s->il_tabs  ;
	d.ip_info  = s->il_info  ;
	d.ip_excl  = s->il_excl  ;
	d.ip_hex   = s->il_hex   ;
	d.ip_msg   = s->il_msg   ;
}


void PEDIT01::copyPrefix( iline * & d, ipline & s )
{
	// Don't copy file line status (changed, error, undo and redo)
	d->il_file  = s.ip_file  ;
	d->il_note  = s.ip_note  ;
	d->il_prof  = s.ip_prof  ;
	d->il_col   = s.ip_col   ;
	d->il_bnds  = s.ip_bnds  ;
	d->il_mask  = s.ip_mask  ;
	d->il_tabs  = s.ip_tabs  ;
	d->il_info  = s.ip_info  ;
	d->il_excl  = s.ip_excl  ;
	d->il_hex   = s.ip_hex   ;
	d->il_msg   = s.ip_msg   ;
}


void PEDIT01::addSpecial( char t, int p, vector<string> & s )
{
	int i ;

	vector<iline * >::iterator it ;
	iline * p_iline ;

	it = data.begin() ;
	advance( it, p+1 )  ;
	for ( i = 0 ; i < s.size() ; i++ )
	{
		p_iline = new iline( taskid() ) ;
		switch ( t )
		{
			case 'I': p_iline->il_info = true ; break ;
			case 'M': p_iline->il_msg  = true ; break ;
			case 'N': p_iline->il_note = true ; break ;
			case 'P': p_iline->il_prof = true ; break ;
		}
		p_iline->put_idata( s.at( i ) ) ;
		it = data.insert( it, p_iline ) ;
		it++ ;
	}
	rebuildZAREA = true ;
}


void PEDIT01::addSpecial( char t, int p, string & s )
{
	vector<iline * >::iterator it ;
	iline * p_iline ;

	it = data.begin()  ;
	advance( it, p+1 ) ;
	p_iline = new iline( taskid() ) ;
	switch ( t )
	{
		case 'I': p_iline->il_info = true ; break ;
		case 'M': p_iline->il_msg  = true ; break ;
		case 'N': p_iline->il_note = true ; break ;
	}
	p_iline->put_idata( s, 0 ) ;
	it = data.insert( it, p_iline ) ;
	rebuildZAREA = true ;
}


void PEDIT01::removeRecoveryData()
{
	// Delete all logically deleted lines and remove entries from the data vector
	// Flatten all remaining data
	// Clear the global Undo/Redo/File stacks
	// (Make a copy of the data vector using copy_if that contains only records to be deleted as iterator is invalidated after an erase() )

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

	copy_if( data.begin(), data.end(), back_inserter( tdata ), [](iline * & a) { return a->il_deleted ; } ) ;

	new_end = remove_if( data.begin(), data.end(), [](iline * & a) { return a->il_deleted ; } ) ;
	data.erase( new_end, data.end() ) ;

	for_each( tdata.begin(), tdata.end(), [](iline * & a) { delete a           ; } ) ;
	for_each(  data.begin(),  data.end(), [](iline * & a) { a->flatten_idata() ; } ) ;

	it = data.begin()            ;
	(*it)->clear_Global_Undo()   ;
	(*it)->clear_Global_Redo()   ;
	(*it)->clear_Global_File_level() ;
	topLine = getLine( topURID ) ;
	MSG = "PEDT011H"             ;
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
			       [](iline * & a) { return (a->il_deleted && !a->il_file) ; } ) ;

	new_end = remove_if( data.begin(), data.end(),
			       [](iline * & a) { return (a->il_deleted && !a->il_file) ; } ) ;
	data.erase( new_end, data.end() ) ;

	for_each( tdata.begin(), tdata.end(), [](iline * & a) { delete a               ; } ) ;
	for_each(  data.begin(),  data.end(), [](iline * & a) { a->remove_redo_idata() ; } ) ;

	it = data.begin()            ;
	(*it)->reset_Global_Undo()   ;
	(*it)->clear_Global_Redo()   ;
	topLine = getLine( topURID ) ;
}


void PEDIT01::removeProfLines()
{
	// Delete PROFILE lines in Data
	// Reposition topLine as lines before may have been removed or topLine itself, deleted

	int topURID ;
	vector<iline * >::iterator new_end ;
	vector<iline * >tdata              ;

	while ( data.at( topLine )->il_prof )
	{
		topLine = getNextDataLine( topLine ) ;
	}
	topURID = data.at( topLine )->il_URID ;

	copy_if( data.begin(), data.end(), back_inserter( tdata ),
			       [](iline * & a) { return a->il_prof ; } ) ;

	new_end = remove_if( data.begin(), data.end(),
			       [](iline * & a) { return a->il_prof ; } ) ;
	data.erase( new_end, data.end() ) ;

	for_each( tdata.begin(), tdata.end(), [](iline * & a) { delete a ; } ) ;
	topLine = getLine( topURID ) ;
}


void PEDIT01::removeSpecialLines()
{
	// Logically delete all temporary lines from the Data vector (can be undone)
	// (col,prof,tabs,mask,bnds,msg,info and note)

	// Reposition topLine as lines before may have been removed or topLine itself, deleted

	int topURID ;
	int lvl     ;

	while ( !data.at( topLine )->il_file && !data.at( topLine )->il_tod && !data.at( topLine )->il_bod )
	{
		topLine = getNextDataLine( topLine ) ;
	}
	topURID = data.at( topLine )->il_URID ;

	lvl = ++Level ;
	for_each( data.begin(), data.end(),
		[ &lvl ](iline * & a)
		{
			if ( a->isSpecial() ) { a->set_deleted( lvl ) ; }
		} ) ;
	topLine = getLine( topURID ) ;
}


bool PEDIT01::getTabLocation( int & pos )
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


void PEDIT01::copyToClipboard( vector<ipline> & vip )
{
	// Copy only data lines in vip (from copy/move) to lspf table CLIPTABL
	// cutReplace - clear clipboard before copy, else append at end of current contents

	int i   ;
	int t   ;
	int CRP ;
	int pos ;

	string UPROF    ;
	string CLIPNAME ;
	string LINE     ;

	vdefine( "CLIPNAME LINE", &CLIPNAME, &LINE ) ;
	vdefine( "CRP", &CRP ) ;
	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( CLIPTABL, WRITE, UPROF ) ;
	if ( RC  > 8 ) { vdelete( "CLIPNAME LINE CRP" ) ; return ; }
	if ( RC == 8 )
	{
		tbcreate( CLIPTABL, "", "CLIPNAME LINE", WRITE, NOREPLACE, UPROF ) ;
		if ( RC > 0 ) { vdelete( "CLIPNAME LINE CRP" ) ; return ; }
	}

	tbvclear( CLIPTABL ) ;
	CLIPNAME = clipboard ;

	pos = 0 ;
	if ( cutReplace ) { clearClipboard( CLIPNAME ) ; }
	else
	{
		tbtop( CLIPTABL ) ;
		tbsarg( CLIPTABL, "", "NEXT", "(CLIPNAME,EQ)" ) ;
		while ( true )
		{
			tbscan( CLIPTABL, "", "", "", "", "NOREAD", "CRP" ) ;
			if ( RC == 8 ) { break ; }
			if ( RC  > 8 ) { tbclose( CLIPTABL) ; vdelete( "CLIPNAME LINE CRP" ) ; return ; }
			pos = CRP ;
		}
		if ( pos > 0 )
		{
			tbtop( CLIPTABL ) ;
			tbskip( CLIPTABL, pos, "", "", "", "NOREAD" ) ;
		}
	}

	tbvclear( CLIPTABL ) ;
	CLIPNAME = clipboard ;

	t = 0 ;
	for ( i = 0 ; i < vip.size() ; i++ )
	{
		if ( !vip[ i ].ip_file ) { continue ; }
		LINE = vip[ i ].ip_data ;
		tbadd( CLIPTABL ) ;
		if ( RC > 0 ) { tbclose( CLIPTABL) ; vdelete( "CLIPNAME LINE CRP" ) ; return ; }
		t++ ;
	}

	tbclose( CLIPTABL ) ;
	MSG  = "PEDT012C"   ;
	ZCMD = ""           ;
	vdelete( "CLIPNAME LINE CRP" ) ;
	vreplace( "ZEDLNES", d2ds( t ) )  ;
	vreplace( "CLIPNAME", clipboard ) ;
}


void PEDIT01::getClipboard( vector<ipline> & vip )
{
	// Get lines from clipboard and put them in vector vip
	// pasteKeep - don't clear clipboard after paste

	string UPROF    ;
	string CLIPNAME ;
	string LINE     ;
	ipline t        ;

	vip.clear() ;

	vdefine( "CLIPNAME LINE", &CLIPNAME, &LINE ) ;
	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( CLIPTABL, WRITE, UPROF ) ;
	if ( RC  > 0 ) { vdelete( "CLIPNAME LINE" ) ; return ; }

	tbvclear( CLIPTABL ) ;
	CLIPNAME = clipboard ;
	tbsarg( CLIPTABL, "", "NEXT", "(CLIPNAME,EQ)" ) ;

	tbtop( CLIPTABL ) ;
	while ( true )
	{
		tbscan( CLIPTABL ) ;
		if ( RC == 8 ) { break ; }
		if ( RC  > 8 ) { tbclose( CLIPTABL) ; vdelete( "CLIPNAME LINE" ) ; return ; }
		t.ip_data = LINE ;
		vip.push_back( t ) ;
	}

	if ( !pasteKeep ) { clearClipboard( CLIPNAME ) ; }
	tbclose( CLIPTABL ) ;
	vdelete( "CLIPNAME LINE" ) ;
}


void PEDIT01::clearClipboard( string clip )
{
	tbvclear( CLIPTABL ) ;
	vreplace( "CLIPNAME", clip ) ;
	tbsarg( CLIPTABL, "", "NEXT" ) ;

	tbtop( CLIPTABL ) ;
	while ( true )
	{
		tbscan( CLIPTABL ) ;
		if ( RC == 8 ) { break  ; }
		if ( RC  > 8 ) { return ; }
		tbdelete( CLIPTABL )    ;
		if ( RC  > 0 ) { break  ; }
	}
	tbtop( CLIPTABL ) ;
}


void PEDIT01::cleanup_custom()
{
	// Called if there is an abnormal termination in the program
	// Try writing out the data to a file so we can reload later (if file has been changed)

	string f ;

	log( "E", "Control given to EDIT cleanup procedure due to an abnormal event" << endl ) ;
//      std::ofstream fout( "/tmp/editorsession", ios::binary ) ;
//      fout.write((char*) &data, sizeof data ) ;
//      fout.close() ;

	if ( !profRecover )
	{
		log( "E", "File not saved as RECOVER is set off" << endl ) ;
		EditList.erase( ZFILE ) ;
		ZRC  = 0 ;
		ZRSN = 0 ;
		return   ;
	}

	if ( !fileChanged )
	{
		log( "E", "File not saved as no changes made during edit session." << endl ) ;
		EditList.erase( ZFILE ) ;
		ZRC  = 0 ;
		ZRSN = 0 ;
		return   ;
	}

	f = ZFILE + ".abend" ;
	vector<iline * >::iterator it ;
	std::ofstream fout( f.c_str() ) ;

	if ( !fout.is_open() )
	{
		ZRESULT = "Open Error" ;
		RC = 16 ;
		return  ;
	}

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_file     ||
		      (*it)->il_deleted  ) { continue ; }
		fout << (*it)->get_idata() << endl ;
	}

	fout.close() ;
	log( "E", "File saved to " << f << endl ) ;
	ZRC  = 0 ;
	ZRSN = 8 ;
	EditList.erase( ZFILE )  ;
	vreplace( "ZEDABRC", f ) ;
	vput( "ZEDABRC", PROFILE ) ;
	return ;
}


void PEDIT01::copyFileData( vector<string> & tdata, int sidx, int eidx )
{
	// Copy entries between sidx and eidx ignoring logically deleted, and non-file lines

	int i ;

	for ( i = sidx ; i <= eidx ; i++ )
	{
		if ( data.at( i )->il_deleted ||
		    !data.at( i )->il_file )  { continue ; }
		tdata.push_back( data.at( i )->get_idata() ) ;
	}
}


void PEDIT01::compareFiles( string s )
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
	string t2  ;
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

	char buffer[256] ;

	vector<iline * >::iterator it ;
	vector<string>Changes ;

	path temp1 ;
	path temp2 ;

	v_list = "CFILE ECPBRDF ECPICAS ECPIREF ECPIBLK ECPITBE" ;

	vcopy( "ZUSER", ZUSER, MOVE ) ;
	vcopy( "ZSCREEN", ZSCREEN, MOVE ) ;

	vdefine( v_list, &CFILE, &ECPBRDF, &ECPICAS, &ECPIREF, &ECPIBLK, &ECPITBE) ;
	vget( v_list, PROFILE ) ;

	if ( s == "" )
	{
		MSG = "" ;
		display( "PEDIT013", MSG, "ZCMD3" ) ;
		if ( RC  > 8 ) { abend()                    ; }
		if ( RC == 8 ) { vdelete( v_list ) ; return ; }
	}
	else if ( s == "*" )
	{
		CFILE = ZFILE ;
	}
	else if ( s[ 0 ] != '/' )
	{
		CFILE = ZFILE.substr( 0, ZFILE.find_last_of( '/' )+1 ) + s ;
	}
	else
	{
		CFILE = s ;
	}

	if ( !is_regular_file( CFILE ) )
	{
		MSG = "PEDT012O"  ;
		vdelete( v_list ) ;
		return ;
	}

	dparms = "" ;
	if ( ECPBRDF == "/" ) { dparms = " -y " ;          }
	if ( ECPICAS == "/" ) { dparms = dparms + " -i " ; }
	if ( ECPIREF == "/" ) { dparms = dparms + " -b " ; }
	if ( ECPIBLK == "/" ) { dparms = dparms + " -B " ; }
	if ( ECPITBE == "/" ) { dparms = dparms + " -E " ; }

	temp1  = temp_directory_path() / unique_path( ZUSER + "-" + ZSCREEN + "-%%%%-%%%%" ) ;
	tname1 = temp1.native() ;

	temp2  = temp_directory_path() / unique_path( ZUSER + "-" + ZSCREEN + "-%%%%-%%%%" ) ;
	tname2 = temp2.native() ;

	std::ofstream fout( tname1.c_str() ) ;
	std::ofstream of ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_file || (*it)->il_deleted ) { continue ; }
		t1 = (*it)->get_idata() ;
		t1 = strip( t1, 'T', ' ' ) ;
		if ( profXTabs )
		{
			t2 = "" ;
			for ( i = 0 ; i < t1.size() ; i++ )
			{
				if ( (i % 8 == 0) && t1.size() > i+7 && t1.compare( i, 8, "        " ) == 0 )
				{
					t2.push_back( '\t' ) ;
					i = i + 7 ;
				}
				else if ( t1[ i ] != ' ' ) { break ; }
				else                       { t2.push_back( ' ' ) ; }

			}
			if ( i < t1.size() )
			{
				t2 = t2 + t1.substr( i ) ;
			}
			fout << t2 << endl ;
		}
		else
		{
			fout << t1 << endl ;
		}
	}
	fout.close() ;

	cmd = "diff " + dparms + CFILE + " " + tname1 ;
	of.open( tname2 ) ;
	FILE* pipe{popen( cmd.c_str(), "r" ) } ;
	while( fgets( buffer, sizeof( buffer ), pipe ) != nullptr )
	{
		file   = buffer ;
		result = file.substr(0, file.size() - 1 ) ;
		of << result << endl ;
	}
	pclose( pipe ) ;
	of.close()     ;
	remove( tname1 ) ;

	if ( ECPBRDF == "/" )
	{
		browse( tname2 ) ;
		if ( ZRC == 4 ) { MSG = "PEDT012P" ; }
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
		if ( isdigit( inLine[ 0 ] ) )
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
		else if ( inLine[0] == '<' )
		{
			Changes.push_back ( removeTabs( inLine.substr( 2 ) ) ) ;
		}
		else if ( inLine[0] == '>' )
		{
			data.at(( getDataLine( d1+o ) ))->il_label = ".O" + right( d2ds( lab ), 4, '0' ) ;
			lab++ ;
			o++   ;
		}
	}
	fin.close() ;
	if ( Changes.size() > 0 )
	{
		addSpecial( 'I', getDataLine( d1 ), Changes ) ;
	}
	if ( !rl ) { MSG = "PEDT012P" ; }
	rebuildZAREA = true ;
	remove( tname2 )  ;
	vdelete( v_list ) ;
}


string PEDIT01::removeTabs( string s )
{
	int j   ;
	int pos ;

	pos = s.find( '\t' ) ;
	while ( pos != string::npos )
	{
		j = 8 - (pos % 8 ) ;
		s.replace( pos, 1,  j, ' ' )  ;
		pos = s.find( '\t', pos + 1 ) ;
	}
	return s ;
}


string PEDIT01::rshiftCols( int n, string s )
{
	if ( LeftBnd > s.size() ) { return s ; }
	s.insert( LeftBnd-1, n, ' ' ) ;

	if ( RightBnd == 0 || RightBnd > s.size() ) { return s ; }
	return s.erase( RightBnd, n ) ;
}


string PEDIT01::lshiftCols( int n, string s )
{
	if ( RightBnd > 0 && RightBnd < s.size() )
	{
		s.insert( RightBnd, n, ' ' ) ;
	}

	if ( LeftBnd > s.size() ) { return s ; }
	return s.erase( LeftBnd-1, n ) ;
}


bool PEDIT01::rshiftData( int n, string s, string & t )
{
/* > Right shifting rules:
   1) scanning starts at left column
   2) first blank char is found
   3) the next non-blank char is found
   4) the next double blank char is found
   5) data from 3) to 4) is shifted 1 col to the right
   The above 5 steps are repeated until request is satisfied

   Without   1) losing data
	     2) shift beyond bound
	     3) deleting single blanks
	     4) deleting blanks within apostrophes
   else ==ERR>                                    */

	int i  ;
	int c1 ;
	int p1 ;
	int p2 ;
	int p3 ;

	t = s ;
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


bool PEDIT01::lshiftData( int n, string s, string & t )
{
/* < Left shifting rules:
   1) scanning starts at left column
   2) first blank char is found
   3) the next non-blank char is found
   4) the next double blank char is found
   5) data from 3) to 4) is shifted 1 col to the left
   The above 5 steps are repeated until request is satisfied
   Without   1) losing data
	     2) shift beyond bound
	     3) deleting single blanks
	     4) deleting blanks within apostrophes
   else ==ERR>                                   */

	int i  ;
	int c1 ;
	int p1 ;
	int p2 ;
	int p3 ;

	t = s ;
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


bool PEDIT01::textSplitData( string s, string & t1, string & t2 )
{
	int p ;
	int k ;

	p = aCol+startCol-10 ;
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
		t2 = s.substr( p ) ;
		t2.insert( 0, k, ' ' ) ;
	}
	return true ;
}


bool PEDIT01::checkLabel( string lab )
{
	int i ;
	int l ;

	l = lab.size() ;

	if ( l < 2 || l > 5 )  { return false ; }
	if ( lab[ 1 ] == 'Z' ) { return false ; }
	for ( i = 1 ; i < l ; i++ )
	{
		if ( !isdigit( lab[ i ] ) &&
		     !isupper( lab[ i ] ) ) { return false ; }
	}
	return true ;
}


void PEDIT01::getEditProfile( string prof )
{
	// ZEDPFLAG :
	//      [0]: Profile locked if '1'                                    [8]:  Hilight On if '1'
	//      [1]: Autosave On if '1'                                       [9]:  Logical tabs On if '1'
	//      [2]: Nulls if '1'                                             [10]: Hardware tabs On if '1'
	//      [3]: Caps On if '1'
	//      [4]: Hex On if '1'
	//      [5]: File Tabs (if '1', convert from spaces -> tabs on save)
	//      [6]: Recover (if '1', create a backup on edit entry)
	//      [7]: SETUNDO On if '1'

	// Defaults: Autosave Off, recover, undo on, Hilight on
	// If RECOV is OFF, set saveLevel = -1 to de-activate this function

	string UPROF    ;
	string tabName  ;
	string flds     ;
	string v_list   ;

	tabName = ZAPPLID + "EDIT" ;

	flds   = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC ZEDPHLLG" ;
	v_list = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;

	vdefine( v_list, &ZEDPFLAG, &ZEDPMASK, &ZEDPBNDL, &ZEDPBNDR, &ZEDPTABC, &ZEDPTABS, &ZEDPTABZ, &ZEDPRCLC ) ;
	vdefine( "ZEDPTYPE ZEDPHLLG", &ZEDPTYPE, &ZEDPHLLG ) ;

	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( tabName, NOWRITE, UPROF, SHARE ) ;
	if ( RC  > 8 ) { abend() ; }
	if ( RC == 8 )
	{
		tbcreate( tabName, "ZEDPTYPE", flds, WRITE, NOREPLACE, UPROF ) ;
		if ( RC > 0 ) { abend() ; }
	}

	tbvclear( tabName ) ;
	ZEDPTYPE = prof     ;

	tbget( tabName ) ;
	if      ( RC >  8 ) { abend() ; }
	else if ( RC == 8 )
	{
		tbclose( tabName ) ;
		if ( RC > 0 ) { abend() ; }
		tbopen( tabName, WRITE, UPROF ) ;
		if ( RC > 0 ) { abend() ; }
		ZEDPFLAG = "000000111000000000000000" ;
		ZEDPMASK    = ""     ;
		ZEDPBNDL    = "1"    ;
		ZEDPBNDR    = "0"    ;
		ZEDPTABC    = " "    ;
		ZEDPTABS    = ""     ;
		ZEDPTABZ    = "8"    ;
		ZEDPRCLC    = ZHOME + "/" ;
		ZEDPHLLG    = "AUTO" ;
		tbadd( tabName )     ;
		MSG = "PEDT013D"     ;
	}
	tbclose( tabName ) ;
	if ( RC > 0 ) { abend() ; }
	profLock    = ( ZEDPFLAG[0]  == '1' ) ;
	profSave    = ( ZEDPFLAG[1]  == '1' ) ;
	profNulls   = ( ZEDPFLAG[2]  == '1' ) ;
	profCaps    = ( ZEDPFLAG[3]  == '1' ) ;
	profHex     = ( ZEDPFLAG[4]  == '1' ) ;
	profXTabs   = ( ZEDPFLAG[5]  == '1' ) ;
	profRecover = ( ZEDPFLAG[6]  == '1' ) ;
	undoON      = ( ZEDPFLAG[7]  == '1' ) ;
	profHilight = ( ZEDPFLAG[8]  == '1' ) ;
	profSTabs   = ( ZEDPFLAG[9]  == '1' ) ;
	profHTabs   = ( ZEDPFLAG[10] == '1' ) ;
	maskLine    = ZEDPMASK         ;
	tabsLine    = ZEDPTABS         ;
	tabsChar    = ZEDPTABC[ 0 ]    ;
	LeftBnd     = ds2d( ZEDPBNDL ) ;
	RightBnd    = ds2d( ZEDPBNDR ) ;
	profXTabz   = ds2d( ZEDPTABZ ) ;
	recoverLoc  = ZEDPRCLC         ;
	profLang    = ZEDPHLLG         ;
	vdelete( v_list ) ;
	vdelete( "ZEDPTYPE ZEDPHLLG" ) ;

	if ( !undoON ) { saveLevel = -1 ; }
}


void PEDIT01::saveEditProfile( string prof )
{
	// ZEDPFLAG :
	//      [0]: Profile locked    [8]:  Hilight On
	//      [1]: Save              [9]:  Logical Tabs On
	//      [2]: Nulls             [10]: Hardware Tabs On
	//      [3]: Caps
	//      [4]: Hex
	//      [5]: File Tabs
	//      [6]: Recover On
	//      [7]: SETUNDO On

	string UPROF    ;
	string tabName  ;
	string v_list   ;

	tabName = ZAPPLID + "EDIT" ;

	v_list = "ZEDPFLAG ZEDPMASK ZEDPBNDL ZEDPBNDR ZEDPTABC ZEDPTABS ZEDPTABZ ZEDPRCLC" ;

	vdefine( v_list, &ZEDPFLAG, &ZEDPMASK, &ZEDPBNDL, &ZEDPBNDR, &ZEDPTABC, &ZEDPTABS, &ZEDPTABZ, &ZEDPRCLC ) ;
	vdefine( "ZEDPTYPE ZEDPHLLG", &ZEDPTYPE, &ZEDPHLLG ) ;

	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( tabName, WRITE, UPROF ) ;
	if ( RC  > 0 ) { abend() ; }

	ZEDPTYPE    = prof   ;
	ZEDPFLAG[0] = ZeroOne[ profLock ] ;

	if ( !profLock )
	{
		ZEDPFLAG[1]  = ZeroOne[ profSave    ] ;
		ZEDPFLAG[2]  = ZeroOne[ profNulls   ] ;
		ZEDPFLAG[3]  = ZeroOne[ profCaps    ] ;
		ZEDPFLAG[4]  = ZeroOne[ profHex     ] ;
		ZEDPFLAG[5]  = ZeroOne[ profXTabs   ] ;
		ZEDPFLAG[6]  = ZeroOne[ profRecover ] ;
		ZEDPFLAG[7]  = ZeroOne[ undoON      ] ;
		ZEDPFLAG[8]  = ZeroOne[ profHilight ] ;
		ZEDPFLAG[9]  = ZeroOne[ profSTabs   ] ;
		ZEDPFLAG[10] = ZeroOne[ profHTabs   ] ;
		ZEDPMASK     = maskLine ;
		ZEDPTABS     = tabsLine ;
		ZEDPTABC     = tabsChar ;
		ZEDPBNDL     = d2ds( LeftBnd )   ;
		ZEDPBNDR     = d2ds( RightBnd )  ;
		ZEDPTABZ     = d2ds( profXTabz ) ;
		ZEDPRCLC     = recoverLoc        ;
		ZEDPHLLG     = profLang          ;
	}
	tbmod( tabName )   ;
	if ( RC > 8 ) { abend() ; }
	tbclose( tabName ) ;
	vput( "ZEDPROF", PROFILE ) ;
	vdelete( v_list )    ;
	vdelete( "ZEDPTYPE ZEDPHLLG" ) ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PEDIT01 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
