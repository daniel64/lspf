/*  Compile with ::                                                                                    */
/* g++  -shared -fPIC -std=c++11 -Wl,-soname,libPEDIT01.so -lboost_regex  -o libPEDIT01.so PEDIT01.cpp */

/*
  Copyright (c) 2015 Daniel John Erdos

  This program iprogram oftware; you can redistribute it and/or modify
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
/* Edit file                                                                                 */
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
// #include "uHilight.cpp"
#include "PEDIT01.h"


using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PEDIT01

#define CLINESZ   8
#define Global_Undo iline::Global_Undo
#define Global_Redo iline::Global_Redo
#define recoverOFF  iline::recoverOFF


map<int,int> iline::maxURID ;
map<int, bool> iline::recoverOFF ;
map<int, stack<ichange> >iline::Global_Undo ;
map<int, stack<ichange> >iline::Global_Redo ;

map<string,bool>PEDIT01::EditList ;

void PEDIT01::application()
{
	log( "I", "Application PEDIT01 starting.  Parms are " << PARM << endl ) ;

	int p1 ;
	int p2 ;

	string * pt ;

	string panel("") ;

	initialise() ;

	if ( PARM == "ENTRYPNL" )
	{
		while ( true )
		{
			ZRC  = 0 ;
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
		p1 = pos( "FILE(", PARM ) ;
		if ( p1 == 0 )
		{
			log( "E", "Invalid parameter format passed to PEDIT01" << endl ; )
			abend() ;
		}
		p2 = pos( ")", PARM, p1 ) ;
		ZFILE = substr( PARM, p1 + 5, p2 - p1 - 5 ) ;
		p1 = pos( "PANEL(", PARM ) ;
		if ( p1 > 0 )
		{
			p2 = pos( ")", PARM, p1 )  ;
			panel = substr( PARM, p1 + 7, p2 - p1 - 6 ) ;
		}
		Edit() ;
	}
	cleanup() ;
}


void PEDIT01::initialise()
{
	control( "ABENDRTN", static_cast<void (pApplication::*)()>(&PEDIT01::cleanup_custom) ) ;

	vdefine( "ZCMD  ZVERB   ZROW1  ZROW2", &ZCMD, &ZVERB, &ZROW1, &ZROW2 ) ;
	vdefine( "ZAREA ZSHADOW ZAREAT ZFILE ZFILENM", &ZAREA, &ZSHADOW, &ZAREAT, &ZFILE, &ZFILENM ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &ZAREAW, &ZAREAD ) ;
	vdefine( "ZSCROLLA ZCOL1 ZLINES", &ZSCROLLA, &ZCOL1, &ZLINES ) ;
	vdefine( "ZAPPLID  ZEDPROF ZHOME", &ZAPPLID, &ZEDPROF, &ZHOME ) ;
	vdefine( "TYPE STR OCC LINES", &TYPE, &STR, &OCC, &LINES ) ;

	vget( "ZEDPROF", PROFILE ) ;
	vget( "ZAPPLID ZHOME", SHARED ) ;

	pquery( "PEDIT012", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 ) { abend() ; }

	if ( ZEDPROF == "" ) { ZEDPROF = "DEFAULT" ; }

	typList[ 'C' ] = "CHARS"  ;
	typList[ 'P' ] = "PREFIX" ;
	typList[ 'S' ] = "SUFFIX" ;
	typList[ 'W' ] = "WORD"   ;

	getEditProfile( ZEDPROF ) ;

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

	tabsOnRead  = false ;
	iline::maxURID[ taskid() ] = 0 ;
}


void PEDIT01::getEditProfile( string prof )
{
	// ZEDFLAG :
	//      [0]: Profile locked if '1'                                    [8]: Hilight On if true
	//      [1]: Autosave On if '1'
	//      [2]: Nulls if '1'
	//      [3]: Caps On if '1'
	//      [4]: Hex On if '1'
	//      [5]: Tabs (if '1', convert from spaces -> tabs on save)
	//      [6]: Backup (if '1', create a backup on edit entry)
	//      [7]: Recovery On if '1'

	// Defaults: Autosave Off, backup, recovery on, Hilight on

	string UPROF    ;
	string tabName  ;
	string flds     ;

	tabName = ZAPPLID + "EDIT" ;

	flds = "ZEDFLAG ZEDMASK ZEDBNDL ZEDBNDR ZEDTABC ZEDTABS ZEDTABZ ZEDBKLC" ;

	vdefine( flds, &ZEDFLAG, &ZEDMASK, &ZEDBNDL, &ZEDBNDR, &ZEDTABC, &ZEDTABS, &ZEDTABZ, &ZEDBKLC ) ;
	vdefine( "ZEDPTYPE", &ZEDPTYPE ) ;

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
		ZEDFLAG = "000000111000000000000000" ;
		ZEDMASK    = ""    ;
		ZEDBNDL    = "1"   ;
		ZEDBNDR    = "0"   ;
		ZEDTABC    = " "   ;
		ZEDTABS    = ""    ;
		ZEDTABZ    = "8"   ;
		ZEDBKLC    = ZHOME + "/" ;
		tbadd( tabName )   ;
	}
	tbclose( tabName ) ;
	if ( RC > 0 ) { abend() ; }
	profLock    = ( ZEDFLAG[0] == '1' ) ;
	profSave    = ( ZEDFLAG[1] == '1' ) ;
	profNulls   = ( ZEDFLAG[2] == '1' ) ;
	profCaps    = ( ZEDFLAG[3] == '1' ) ;
	profHex     = ( ZEDFLAG[4] == '1' ) ;
	profTabs    = ( ZEDFLAG[5] == '1' ) ;
	profBackup  = ( ZEDFLAG[6] == '1' ) ;
	profRecov   = ( ZEDFLAG[7] == '1' ) ;
	profHilight = ( ZEDFLAG[8] == '1' ) ;
	maskLine    = ZEDMASK ;
	tabsLine    = ZEDTABS ;
	tabsChar    = ZEDTABC[ 0 ]    ;
	LeftBnd     = ds2d( ZEDBNDL ) ;
	RightBnd    = ds2d( ZEDBNDR ) ;
	profTabz    = ds2d( ZEDTABZ ) ;
	backupLoc   = ZEDBKLC         ;
	vdelete( flds )  ;
	vdelete( "ZEDPTYPE" )  ;
}


void PEDIT01::saveEditProfile( string prof )
{
	// ZEDFLAG :
	//      [0]: Profile locked    [8]: Hilight On
	//      [1]: Save
	//      [2]: Nulls
	//      [3]: Caps
	//      [4]: Hex
	//      [5]: Tabs
	//      [6]: Backup
	//      [7]: Recovery

	string UPROF    ;
	string tabName  ;
	string flds     ;

	tabName = ZAPPLID + "EDIT" ;

	flds =   "ZEDFLAG ZEDMASK ZEDBNDL ZEDBNDR ZEDTABC ZEDTABS ZEDTABZ ZEDBKLC" ;
	vdefine( flds, &ZEDFLAG, &ZEDMASK, &ZEDBNDL, &ZEDBNDR, &ZEDTABC, &ZEDTABS, &ZEDTABZ, &ZEDBKLC ) ;
	vdefine( "ZEDPTYPE", &ZEDPTYPE ) ;
	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( tabName, WRITE, UPROF ) ;
	if ( RC  > 0 ) { abend() ; }

	ZEDPTYPE   = prof   ;
	ZEDFLAG[1] = ZeroOne[ profLock ] ;

	if ( !profLock )
	{
		ZEDFLAG[1] = ZeroOne[ profSave    ] ;
		ZEDFLAG[2] = ZeroOne[ profNulls   ] ;
		ZEDFLAG[3] = ZeroOne[ profCaps    ] ;
		ZEDFLAG[4] = ZeroOne[ profHex     ] ;
		ZEDFLAG[5] = ZeroOne[ profTabs    ] ;
		ZEDFLAG[6] = ZeroOne[ profBackup  ] ;
		ZEDFLAG[7] = ZeroOne[ profRecov   ] ;
		ZEDFLAG[8] = ZeroOne[ profHilight ] ;
		ZEDMASK    = maskLine ;
		ZEDTABS    = tabsLine ;
		ZEDTABC    = string( 1, tabsChar ) ;
		ZEDBNDL    = d2ds( LeftBnd )  ;
		ZEDBNDR    = d2ds( RightBnd ) ;
		ZEDTABZ    = d2ds( profTabz ) ;
		ZEDBKLC    = backupLoc        ;
	}
	tbmod( tabName )   ;
	if ( RC > 8 ) { abend() ; }
	tbclose( tabName ) ;
	vput( "ZEDPROF", PROFILE ) ;
	vdelete( flds )    ;
}


void PEDIT01::Edit()
{
	int p ;
	int t ;

	uint row ;

	RC = 0 ;

	MSG = "" ;
	read_file() ;
	if ( RC > 0 )
	{
		if ( MSG == "" ) { MSG = "PSYS01E" ; }
		setmsg( MSG ) ;
		return ;
	}

	CURFLD  = "ZCMD" ;
	CURPOS  = 1      ;
	ZFILENM = ZFILE  ;
	clearCursor()    ;

	cutActive   = false ;
	pasteActive = false ;

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

		positionCursor() ;
		display( "PEDIT012", MSG, CURFLD, CURPOS ) ;

		if ( RC  > 8 ) { abend() ; }
		if ( RC == 8 )
		{
			if ( termOK() ) { break    ; }
			else            { continue ; }
		}
		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		clearCursor() ;
		if ( abbrev( "CANCEL", upper( ZCMD ), 3 ) ) { break ; }

		if ( ZCMD[ 0 ] == '&' )   { OCMD = ZCMD ; ZCMD = ZCMD.substr( 1 ) ; }
		else                      { OCMD = ""   ;                           }

		if ( ZCURFLD == "ZAREA" )
		{
			CURFLD = ZCURFLD ;
			CURPOS = ZCURPOS ;
			aRow   = ((ZCURPOS-1) / ZAREAW + 1)   ;
			aCol   = ((ZCURPOS-1) % ZAREAW + 1)   ;
			aURID  = s2data.at( aRow-1 ).ipo_URID ;
			storeCursor( aURID, 4, aCol )         ;
		}
		else
		{
			CURFLD = "ZCMD" ;
			CURPOS = 1      ;
			aRow   = 0      ;
			aCol   = 0      ;
			aURID  = 0      ;
		}
		MSG = "" ;

		getZAREAchanges() ;
		updateData()      ;

		actionPCMD()      ;
		if ( MSG != "" )  { continue ; }

		actionLineCommands() ;

		if ( ZVERB == "DOWN" )
		{
			cursorPlaced = true ;
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				topLine = data.size() - ZAREAD ;
			}
			else
			{
				t = 0 ;
				for ( ; topLine < ( data.size() - 1 ) ; topLine++ )
				{
					if ( data.at( topLine )->il_excl || data.at( topLine )->il_deleted ) continue ;
					t++ ;
					if ( t > ZSCROLLN ) break ;
				}
			}
		}
		else if ( ZVERB == "UP" )
		{
			cursorPlaced = true ;
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
					if ( data.at( topLine )->il_excl || data.at( topLine )->il_deleted ) { continue ; }
					t++ ;
					if ( t > ZSCROLLN ) break ;
				}
			}
		}
		else if ( ZVERB == "LEFT" )
		{
			cursorPlaced = true ;
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
			cursorPlaced = true ;
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				startCol = maxCol - ZAREAW ;
				if ( startCol < 1 ) { startCol = 1 ; }
			}
			else
			{
					startCol = startCol + ZSCROLLN ;
			}
		}
		else if ( ZVERB == "RF" || ZVERB == "RFIND" )
		{
			if ( !find_parms.fcx_fset ) { MSG = "PEDT01C" ; continue ; }
			actionFind() ;
			if ( find_parms.fcx_success )
			{
				placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset+CLINESZ-startCol+2 ) ;
				if ( !URIDonScreen( find_parms.fcx_URID ) )
				{
					topLine = getLine( find_parms.fcx_URID ) ;
					topLine = getPrevDataLine( topLine ) ;
				}
				MSG  = "PEDT012G" ;
				TYPE = typList[ find_parms.fcx_mtch ] ;
				if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
				else                                { STR = find_parms.fcx_estring ; }
			}
			else
			{
				TYPE = typList[ find_parms.fcx_mtch ] ;
				if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
				else                                { STR = find_parms.fcx_estring ; }
				MSG  = "PEDT012H" ;
			}
			ZCMD = ""           ;
			rebuildZAREA = true ;
		}
		else if ( ZVERB == "RCHANGE" )
		{
			if ( !find_parms.fcx_cset ) { MSG = "PEDT01D" ; continue ; }
			actionChange()      ;
			ZCMD = ""           ;
			rebuildZAREA = true ;
		}
		else if ( aRow > 0 )
		{
			if ( !cursorPlaced )
			{
				if ( tabsLine != "" && getTabLocation( p ) )
				{
					if ( p > aCol+startCol-CLINESZ-2 )
					{
						row = aRow ;
						placeCursor( row, 4, p-startCol+CLINESZ+2 ) ;
					}
					else
					{
						t = getNextDataLine( s2data.at( aRow-1 ).ipo_URID ) ;
						placeCursor( t, 4, p-startCol+CLINESZ+2 ) ;
					}
				}
				else
				{
					t = getNextDataLine( s2data.at( aRow-1 ).ipo_URID ) ;
					placeCursor( t, 3 ) ;
				}
			}
		}
		if ( topLine < 0 ) { topLine = 0 ; }
	}
	vput( "ZSCROLL", PROFILE ) ;

	vector<iline * >::iterator it ;
	for_each( data.begin(), data.end(), [](iline * & a) { delete a ; } ) ;
	data.clear() ;
	saveEditProfile( ZEDPROF ) ;
	EditList.erase( ZFILE ) ;
	return ;
}



void PEDIT01::showEditEntry()
{
	MSG    = ""      ;
	display( "PEDIT011", MSG, "ZCMD1" ) ;
	if ( RC  > 8 ) { abend() ; }
	if ( RC == 8 ) { ZRC = 4 ; }
}


bool PEDIT01::termOK()
{
	if ( fileChanged && !profSave && ZCMD != "SAVE" ) { MSG = "PEDT01O" ; return false ; }
	if ( fileChanged )
	{
		if ( saveFile() ) { setmsg( "PEDT01P" ) ; }
		else              { return false        ; }
	}
	return true ;
}


void PEDIT01::read_file()
{
	int p   ;
	int j   ;
	int pos ;

	string ZDATE  ;
	string ZTIMEL ;
	string inLine ;

	idata t ;

	boost::system::error_code ec ;

	iline * p_iline ;
	vector<iline * >::iterator it ;

	ifstream fin( ZFILE.c_str() ) ;

	if ( EditList.find( ZFILE ) != EditList.end() )
	{
		MSG     = "PEDT013A" ;
		ZRESULT = "File In Use" ;
		RC      = 4 ;
		ZRC     = 4 ;
		ZRSN    = 0 ;
		return      ;
	}

	if ( !exists( ZFILE ) )
	{
	}

	if ( !fin.is_open() )
	{
		ZRESULT = "Open Error" ;
		RC = 16 ;
		return  ;
	}

	topLine   = 0 ;
	startCol  = 1 ;
	maxCol    = 1 ;
	Level     = 0 ;

	rebuildZAREA = true  ;
	fileChanged  = false ;

	data.clear() ;
	iline::maxURID[ taskid() ] = 0 ;

	p_iline          = new iline( taskid() ) ;
	p_iline->il_tod  = true   ;
	p_iline->put_idata( centre( " TOP OF DATA ", ZAREAW, '*' ), 0 ) ;
	data.push_back( p_iline ) ;

	data[ 0 ]->clear_Global_Redo() ;
	data[ 0 ]->clear_Global_Undo() ;

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
	p_iline->clear_Global_Undo() ;
	fin.close() ;

	it = data.begin() ;
	it++ ;
	p_iline          = new iline( taskid() ) ;
	p_iline->il_note = true      ;
	p_iline->put_idata( " WARNING!!!  EDITOR UNDER CONSTRUCTION.  SAVE FILES AT YOUR OWN RISK !!!!", 0 ) ;
	it = data.insert( it, p_iline ) ;
	it++ ;

	EditList[ ZFILE ] = true ;

	if ( tabsOnRead && !profTabs )
	{
		p_iline          = new iline( taskid() ) ;
		p_iline->il_note = true      ;
		p_iline->put_idata( " WARNING!!!  TABS HAVE BEEN DETECTED AND CONVERTED TO SPACES", 0 ) ;
		it = data.insert( it, p_iline ) ;
		it++ ;
		p_iline          = new iline( taskid() ) ;
		p_iline->il_note = true      ;
		p_iline->put_idata( " WARNING!!!  BUT PROFILE OPTION IS NOT TO CONVERT SPACES TO TABS ON SAVE !!!!", 0 ) ;
		it = data.insert( it, p_iline ) ;
		it++ ;
	}

	if ( profBackup )
	{
		vcopy( "Z4JDATE", ZDATE, MOVE ) ;
		vcopy( "ZTIMEL", ZTIMEL, MOVE ) ;
		p = ZFILE.find_last_of( '/' )   ;
		copy_file( ZFILE, backupLoc + ZFILE.substr( p+1 ) + "-" +ZDATE + "-" + ZTIMEL, ec ) ;
		if ( ec.value() != boost::system::errc::success )
		{
			MSG = "PEDT011F" ;
		}
	}
}


bool PEDIT01::saveFile()
{
	int i     ;
	string t1 ;
	string t2 ;
	string f  ;

	f = ZFILE  ;
	vector<iline * >::iterator it ;
	ofstream fout( f.c_str() ) ;

	if ( !fout.is_open() )
	{
		ZRESULT = "Open Error" ;
		RC = 16 ;
		return  false ;
	}

	log( "I", "Saving file " << ZFILE << endl ) ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_file     ) { continue ; }
		if (  (*it)->il_deleted  ) { continue ; }
		t1 = (*it)->get_idata() ;
		t1 = strip( t1, 'T', ' ' ) ;
		if ( profTabs )
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
  //    if ( error saving file ) { MSG = "PEDT01Q" ; ZRC = 8 ; ZRSN = 8 ; return false ; }
	ZRC  = 0 ;
	ZRSN = 4 ;
	return true ;
}


void PEDIT01::fill_dynamic_area()
{
	int i   ;
	int l   ;
	int ln  ;
	int t   ;
	int dl  ;
	int sl  ;
	int fl  ;
	int elines ;
	int blines ;
	int URID   ;

	iposition ip ;

	string t1 ;
	string t2 ;
	string t3 ;
	string t4 ;
	string lcc ;
	string tmp ;

	static string din( 1, 0x01 )  ;
	static string dout( 1, 0x02 ) ;

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
			elines = getEXBlock( data.at( dl )->il_URID ) ;
			ip.ipo_line = dl ;
			ip.ipo_URID = data.at( dl )->il_URID ;
			s2data.at( sl ) = ip ;
			tmp     = copies( "-  ", (ZAREAW - 30)/3 -2 ) + d2ds( elines ) + " Line(s) Not Displayed" ;
			ZAREA   = ZAREA + din + "- - - " + dout + substr( tmp, 1, ZAREAW-8 ) ;
			ZSHADOW = ZSHADOW + slr + sdw ;
			if ( ZAREA.size() >= ZASIZE ) break ;
			sl++ ;
			if ( data.at( topLine )->il_excl )
			{
				URID    = data.at( topLine )->il_URID ;
				topLine = getLine( getFirstEX( URID ) ) ;
				blines  = getDataBlock( URID ) ;
				dl      = topLine + blines  ;
			}
			else
			{
				blines = getDataBlock( data.at( dl )->il_URID ) ;
				dl = dl + blines -1 ;
			}
			fl = getFileLine( dl+1 ) ;
			continue ;
		}
		if ( data.at( dl )->il_lc1 == "" )
		{
			if ( data.at( dl )->il_label == "" )
			{
				if ( data.at( dl )->il_newisrt )     { lcc = "''''''" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_error )  { lcc = "==ERR>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_file  )
				{
					lcc = right( d2ds( fl ), 6, '0' ) ;
					ZSHADOW = ZSHADOW + slg ;
				}
				else if ( data.at( dl )->il_note  )  { lcc = "=NOTE>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_col   )  { lcc = "=COLS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_bnds  )  { lcc = "=BNDS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_mask  )  { lcc = "=MASK>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_prof  )  { lcc = "=PROF>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_tabs  )  { lcc = "=TABS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data.at( dl )->il_chg   )  { lcc = "==CHG>" ; ZSHADOW = ZSHADOW + slr ; }
				else                                 { lcc = "******" ; ZSHADOW = ZSHADOW + slr ; }
			}
			else
			{
				lcc     = left( data.at( dl )->il_label, 6 ) ;
				ZSHADOW = ZSHADOW + slr                      ;
			}
		}
		else
		{
			lcc     = left( data.at( dl )->il_lc1, 6 ) ;
			ZSHADOW = ZSHADOW + slr                 ;
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
				ip.ipo_line  = dl ;
				ip.ipo_URID  = data.at( dl )->il_URID ;
				s2data.at( sl ) = ip ;
				sl              = sl + 4 ;
			}
			else
			{
				ZAREA   = ZAREA + din + lcc + din + substr( data.at( dl )->get_idata(), startCol, ZDATAW ) ;
				if ( data.at( dl )->il_newisrt ) { ZSHADOW = ZSHADOW + sdyh ; }
				else                             { ZSHADOW = ZSHADOW + sdy  ; }
				ip.ipo_line     = dl ;
				ip.ipo_URID     = data.at( dl )->il_URID ;
				s2data.at( sl ) = ip ;
				sl++ ;
			}
		}
		else
		{
			if ( data.at( dl )->il_col )
			{
				tmp = substr( "+----+----+----+---", startCol%10+1, 10-startCol%10 ) ;
				if ( startCol%10 == 0 ) { tmp = "" ; t = startCol / 10 ; }
				else                    { t = startCol / 10 + 1        ; }
				for ( i = 0 ; i < (ZDATAW/10+1) ; i++ )
				{
					tmp = tmp + d2ds( t%10 ) + "----+----" ;
					t++ ;
				}
				ZAREA = ZAREA + din + lcc + dout + substr( tmp, 1, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdw ;
			}
			else if ( data.at( dl )->il_bnds )
			{
				tmp = copies( " ", LeftBnd-1 ) + "<" ;
				if ( RightBnd > 0 ) { tmp = tmp + copies( " ", RightBnd-tmp.size()-1 ) + ">" ; }
				ZAREA = ZAREA + din + lcc + din + substr( tmp, startCol, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdw ;
			}
			else if ( data.at( dl )->il_mask )
			{
				data.at( dl )->put_idata( maskLine ) ;
				ZAREA = ZAREA + din + lcc + din + substr( data.at( dl )->get_idata(), startCol, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdr ;
			}
			else if ( data.at( dl )->il_tabs )
			{
				data.at( dl )->put_idata( tabsLine ) ;
				ZAREA = ZAREA + din + lcc + din + substr( data.at( dl )->get_idata(), startCol, ZDATAW ) ;
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
			ip.ipo_line  = dl ;
			ip.ipo_URID  = data.at( dl )->il_URID ;
			s2data.at( sl ) = ip ;
			sl++ ;
		}
		if ( ZAREA.size() >= ZAREAW * ZAREAD ) break ;
	}
	//      addHilight( data, fileType, topLine, startCol, ZAREAW, ZAREAD, ZSHADOW ) ;

	ZAREA.resize( ZASIZE, ' ' ) ;
	ZSHADOW.resize( ZASIZE, N_GREEN ) ;

	CAREA        = ZAREA   ;
	CSHADOW      = ZSHADOW ;
	rebuildZAREA = false ;
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
	int dl  ;
	int off ;
	int sp  ;
	int ep  ;

	string lc ;

	const char duserMod(0x3) ;
	const char ddataMod(0x4) ;

	sTouched.clear() ;
	sChanged.clear() ;

	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		dl            = s2data.at( i ).ipo_line ;
		off           = i * ZAREAW  ;
		sTouched[ i ] = false       ;
		sChanged[ i ] = false       ;
		if ( ZAREA[ off ] == ddataMod )
		{
			lc = "      " ;
			ignore = true ;
			if ( data.at( dl )->il_lc1 == "" && data.at( dl )->il_label == "" )
			{
				for ( j = off+1 ; j < off+6 ; j++ )
				{
					if ( ZAREA[ j ] == ' ' || isdigit( ZAREA[ j ] ) ) { ZAREA[ j ] = ' ' ; continue ; }
					break ;
				}
				sp = j-off ;
				if ( aRow == (i + 1 ) && ( aCol > 1 && aCol < 9 ) )
				{
					for ( j = off+6 ; j > off ; j-- )
					{
						if ( j < (aCol + off -1 ) || ( ZAREA[ j ] != CAREA[ j ] ) )  { break ; }
					}

				}
				else
				{
					for ( j = off+6 ; j > off ; j-- )
					{
						if ( ZAREA[ j ] != CAREA[ j ] )  { break ; }
					}

				}
				ep = j - off + 1 ;
				lc = ZAREA.substr( sp+off, (ep - sp) ) ;
			}
			else
			{
				lc = ZAREA.substr( 1+off , 6 ) ;
			}
			lc = strip( upper( lc ) ) ;
			if ( lc == "" )
			{
				if ( data.at( dl )->il_lc1 == "" ) { data.at( dl )->il_label = "" ; }
				else                               { data.at( dl )->il_lc1   = "" ; }
			}
			else
			{
				data.at( dl )->il_lc1 = lc ;
			}
			rebuildZAREA = true ;
		}
		if ( ZAREA[ off + 7 ] == duserMod )
		{
			sTouched[ i ] = true ;
			rebuildZAREA  = true ;
		}
		else if ( ZAREA[ off + 7 ] == ddataMod )
		{
			data.at( dl )->il_chg = true ;
			sChanged[ i ]         = true ;
			rebuildZAREA          = true ;
		}
	}
}


void PEDIT01::updateData()
{
	int i  ;
	int j  ;
	int k  ;
	int dl ;
	int p  ;

	string t ;

	iline * p_iline ;

	vector<iline * >::iterator it ;

	Level++ ;
	for ( i = ZAREAD-1 ; i >= 0 ; i-- )
	{
		if ( s2data.at( i ).ipo_URID == 0 ) { continue ; }
		dl = s2data.at( i ).ipo_line ;
		if ( data.at( dl )->il_newisrt )
		{
			if ( !sTouched[ i ] && !sChanged[ i ] )
			{
				it = data.begin() ;
				advance( it, dl ) ;
				delete *it            ;
				it = data.erase( it ) ;
				placeCursor( (*it)->il_URID, 2 ) ;
				rebuildZAREA = true   ;
				continue              ;
			}
			else
			{
				data.at( dl )->il_newisrt = false  ;
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
						t =strip( ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ), 'T', ' ' ) ;
					}
				}
				if ( profCaps ) { t = upper( t ) ; }
				data.at( dl )->put_idata( t, Level ) ;
				if ( data.at( dl )->il_URID == aURID )
				{
					it = getLineItr( data.at( dl )->il_URID ) ;
					k = (*it)->get_idata().find_first_not_of( ' ', startCol-1 ) ;
					it++ ;
					p_iline = new iline( taskid() ) ;
					p_iline->il_file    = true ;
					p_iline->il_newisrt = true ;
					p_iline->put_idata( maskLine, Level ) ;
					it = data.insert( it, p_iline )       ;
					if ( k != string::npos ) { placeCursor( p_iline->il_URID, 4, k+CLINESZ+1 ) ; }
					else                     { placeCursor( p_iline->il_URID, 2 )             ; }
				}
				rebuildZAREA  = true ;
				fileChanged   = true ;
				continue             ;
			}
		}
		if ( !sChanged[ i ] ) { continue ; }
		if ( data.at( dl )->il_bnds )
		{
			t = ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ) ;
			p = LeftBnd - startCol ;
			if ( p >= 0 && p < ZDATAW && t[ p ] == ' ' ) { LeftBnd  = 1 ; }
			p = RightBnd - startCol ;
			if ( p >= 0 && p < ZDATAW && t[ p ] == ' ' ) { RightBnd = 0 ; }
			p = t.find( '<' ) ;
			if ( p != string::npos ) { LeftBnd  = startCol + p ; }
			p = t.find( '>' ) ;
			if ( p != string::npos )   { RightBnd = startCol + p ; }
			if ( RightBnd <= LeftBnd ) { RightBnd = 0            ; }
			rebuildZAREA  = true  ;
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
					t =strip( ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ), 'T', ' ' ) ;
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
				data.at( dl )->put_idata( t, Level ) ;
			}
			rebuildZAREA = true  ;
			fileChanged  = true  ;
		}
	}
}


void PEDIT01::actionPCMD()
{
	// Action primary command

	int dl ;
	int i  ;
	int j  ;
	int ws ;
	int p1 ;
	int topURID ;

	string w1   ;
	string w2   ;
	string w3   ;
	string w4   ;
	string wall ;

	iline * p_iline ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;
	vector<iline * >::iterator its ;
	vector<iline * >::iterator ite ;

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
	else if ( w1 == "BACKUP" )
	{
		if ( w2 == "PATH" )
		{
			backupLoc = subword( ZCMD, 3 ) ;
		}
		else if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
		else if ( w2 == "ON" || ws == 1 )
		{
			profBackup   = true ;
			rebuildZAREA = true ;
		}
		else if ( w2 == "OFF" )
		{
			profBackup   = false ;
			rebuildZAREA = true ;
		}
		else { MSG = "PEDT011" ; }
		ZCMD = "" ;
	}
	else if ( w1 == "CAPS" )
	{
		if ( ws != 2 ) { MSG = "PEDT011" ; return ; }
		if ( w2 == "ON" || ws == 1 )
		{
			profCaps = true ;
		}
		else if ( w2 == "OFF" )
		{
			profCaps = false ;
		}
		else { MSG = "PEDT011" ; }
		ZCMD = "" ;
	}
	else if ( w1 == "C" || w1 == "CHANGE" )
	{
		if ( setFindChangeExcl( 'C' ) > 0 ) { MSG = "PEDT018" ; return ; }
		actionChange()      ;
		ZCMD = ""           ;
		rebuildZAREA = true ;
	}
	else if ( w1 == "CUT" )
	{
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
		}
		if ( wall != "" )
		{
			if ( words( wall ) == 1 ) { clipboard = strip( wall ) ; }
			else                      { MSG = "PEDT012A"          ; }
		}
	}
	else if ( w1 == "DEL" || w1 == "DELETE" )
	{
		if ( ws != 3 ) { MSG = "PEDT011" ; return ; }
		topURID = data.at( topLine )->il_URID ;
		if ( w2 == "X" || w2 == "EX" || w2 == "EXCLUDED" )
		{
			if ( w3 != "ALL" ) { MSG = "PEDT011" ; return ; }
			i = 0 ;
			Level++ ;
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if (  (*it)->il_deleted ) { continue ; }
				if ( !(*it)->il_excl    ) { continue ; }
				(*it)->put_idata( "", Level ) ;
				(*it)->set_il_deleted()       ;
				i++ ;
			}
			if ( i == 0 )
			{
				Level-- ;
			}
			else
			{
				rebuildZAREA  = true  ;
				fileChanged   = true  ;
			}
			ZLINES = d2ds( i ) ;
			MSG    = "PEDT01M" ;
		}
		else if ( w2 == "NX" )
		{
			if ( w3 != "ALL" ) { MSG = "PEDT011" ; return ; }
			i = 0 ;
			Level++ ;
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if ( (*it)->il_deleted ) { continue ; }
				if ( (*it)->il_excl    ) { continue ; }
				if ( (*it)->il_bod     ) { break    ; }
				if ( (*it)->il_tod )     { continue ; }
				(*it)->put_idata( "", Level ) ;
				(*it)->set_il_deleted()       ;
				i++ ;
			}
			if ( i == 0 )
			{
				Level-- ;
			}
			else
			{
				rebuildZAREA  = true  ;
				fileChanged   = true  ;
			}
			ZLINES = d2ds( i )    ;
			MSG    = "PEDT01M"    ;
		}
		else { MSG = "PEDT011" ; return ; }
		ZCMD = ""           ;
		rebuildZAREA = true ;
		return              ;
	}
	else if ( w1 == "F" || w1 == "FIND" )
	{
		if ( setFindChangeExcl( 'F' ) > 0 ) { return ; }
		actionFind()        ;
		if ( find_parms.fcx_success )
		{
			placeCursor( find_parms.fcx_URID, 4, find_parms.fcx_offset+CLINESZ-startCol+2 ) ;
			if ( !URIDonScreen( find_parms.fcx_URID ) )
			{
				topLine = getLine( find_parms.fcx_URID ) ;
				topLine = getPrevDataLine( topLine ) ;
			}
			MSG  = "PEDT012G" ;
			TYPE = typList[ find_parms.fcx_mtch ] ;
			if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
			else                                { STR = find_parms.fcx_estring ; }
			OCC  = d2ds( find_parms.fcx_occurs ) ;
		}
		else if ( find_parms.fcx_occurs > 0 && find_parms.fcx_dir == 'A' )
		{
			TYPE  = typList[ find_parms.fcx_mtch ] ;
			if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
			else                                { STR = find_parms.fcx_estring ; }
			OCC   = d2ds( find_parms.fcx_occurs ) ;
			LINES = d2ds( find_parms.fcx_lines  ) ;
			MSG   = "PEDT012I" ;
		}
		else
		{
			TYPE = typList[ find_parms.fcx_mtch ] ;
			if ( find_parms.fcx_rstring != "" ) { STR = find_parms.fcx_rstring ; }
			else                                { STR = find_parms.fcx_estring ; }
			MSG  = "PEDT012H" ;
		}
		ZCMD = ""           ;
		rebuildZAREA = true ;
	}
	else if ( w1 == "FLIP" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		for_each( data.begin(), data.end(), [](iline * & a) { if ( !a->il_bod && !a->il_tod && !a->il_deleted ) { a->il_excl = !a->il_excl ; } } ) ;
		ZCMD = ""           ;
		rebuildZAREA = true ;
	}
	else if ( w1 == "HEX" )
	{
		if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
		if ( w2 == "ON" || ws == 1 )
		{
			profHex      = true ;
			rebuildZAREA = true ;
		}
		else if ( w2 == "OFF" )
		{
			profHex        = false ;
			rebuildZAREA = true ;
		}
		else { MSG = "PEDT011" ; }
		ZCMD = "" ;
	}
	else if ( w1 == "LOCATE" || w1 == "L" )
	{
		if ( ws != 2 ) { MSG = "PEDT011" ; return ; }
		if ( w2[ 0 ] == '.' )
		{
			if ( returnLabelItr( w2, it, p1 ) ) { topLine = p1 ; rebuildZAREA = true ; }
			else                                { MSG = "PEDT01F"                    ; }
		}
		else if ( w2 == "SPECIAL" || w2 == "SPE" )
		{
			topLine      = getNextSpecial( topLine ) ;
			rebuildZAREA = true ;
		}
		else if ( w2.size() < 9 && datatype( w2, 'W' ) )
		{
			if ( w2 == "0" )
			{
				topLine      = getDataLine( 1 ) ;
				rebuildZAREA = true ;
			}
			else
			{
				p1 = ds2d( w2 ) ;
				topLine      = getDataLine( p1 ) ;
				rebuildZAREA = true ;
			}
		}
	}
	else if ( w1 == "PASTE" )
	{
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
	}
	else if ( w1 == "PROF" || w1 == "PROFILE" )
	{
		if ( ws == 1 )
		{
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if ( (*it)->il_prof )
				{
					delete *it  ;
					itt = it    ;
					itt--       ;
					data.erase( it ) ;
					it = itt    ;
				}
			}
			rebuildZAREA = true  ;

			dl = topLine ;
			for ( it = data.begin(), i = 0 ; i <= dl ; i++ ) { it++ ; }

			p_iline  = new iline( taskid() ) ;
			p_iline->il_prof = true ;
			p_iline->put_idata( left( "...." + ZEDPTYPE + "....RECOVERY " + OnOff[ profRecov ] + "....AUTOSAVE "+OnOff[ profSave ]+"....NUM OFF....CAPS "+OnOff[ profCaps ], ZDATAW, '.' ), Level ) ;
			it = data.insert( it, p_iline ) ;
			it++ ;

			p_iline  = new iline( taskid() ) ;
			p_iline->il_prof = true ;
			p_iline->put_idata( left( "....CONVERT SPACES TO TABS " + OnOff[ profTabs ] + "....TABSIZE "+d2ds( profTabz )+"....HEX "+OnOff[ profHex ]+"....NULLS "+OnOff[ profNulls ], ZDATAW, '.' ), Level ) ;
			it = data.insert( it, p_iline ) ;
			it++ ;

			p_iline  = new iline( taskid() ) ;
			p_iline->il_prof = true ;
			p_iline->put_idata( left( "....BACKUP "+OnOff[ profBackup ]+"....BACKUP LOCATION " + backupLoc, ZDATAW, '.' ), Level ) ;
			it = data.insert( it, p_iline ) ;
			it++ ;

			p_iline  = new iline( taskid() ) ;
			p_iline->il_prof = true ;
			p_iline->put_idata( left( "....HILIGHT "+OnOff[ profHilight ]+" CPP....PROFILE LOCK "+OnOff[ profLock ], ZDATAW, '.' ), Level ) ;
			it = data.insert( it, p_iline ) ;
			it++ ;

			rebuildZAREA = true  ;
		}
		else if ( ws != 2 )         { MSG = "PEDT011" ; return ; }
		else if ( w2 == "SAVE"    ) { profSave = true  ; }
		else if ( w2 == "NOSAVE"  ) { profSave = false ; }
		else if ( w2 == "LOCK"    ) { profLock = true  ; }
		else if ( w2 == "UNLOCK"  ) { profLock = false ; }
		else if ( isvalidName( w2 ) ) { saveEditProfile( ZEDPROF ) ; getEditProfile( w2 ) ; ZEDPROF = w2 ; }
		else                          { MSG = "PEDT011" ; return ; }
		ZCMD = "" ;
	}
	else if ( w1 == "RECOV" || w1 == "RECOVER" )
	{
		if      ( ws > 2 )      { MSG = "PEDT011" ; return                 ; }
		else if ( w2 == "ON" )  { recoverOFF[ taskid() ] = false ; profRecov = true    ; }
		else if ( w2 == "OFF" ) { recoverOFF[ taskid() ] = true  ; profRecov = false   ; removeRecoveryData() ; rebuildZAREA = true ; }
		else                    { MSG = "PEDT011" ; return                 ; }
		ZCMD = ""    ;
	}
	else if ( w1 == "REDO" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		actionREDO() ;
		ZCMD = ""    ;
	}
	else if ( w1 == "RES" || w1 == "RESET" )
	{
		if ( w2 == "" )
		{
			Level++ ;
			topURID = data.at( topLine )->il_URID ;
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if ( (*it)->il_note || (*it)->il_col  || (*it)->il_prof ||
				     (*it)->il_bnds || (*it)->il_mask || (*it)->il_tabs )
				{
					(*it)->put_idata( "", Level ) ;
					(*it)->set_il_deleted() ;
					rebuildZAREA = true     ;
				}
				(*it)->il_excl  = false ;
				(*it)->il_hex   = false ;
				(*it)->il_chg   = false ;
				(*it)->il_error = false ;
				(*it)->clearLc12() ;
			}
			icmds.clear() ;
			topLine      = getLine( topURID ) ;
			rebuildZAREA = true  ;
		}
		else if ( (w2 == "CMD") || (w2 == "COMMAND") )
		{
			if ( w3 == "" )
			{
				its = data.begin() ;
				ite = data.end()   ;
			}
			else if ( w3[ 0 ] != '.' || w4[ 0 ] != '.' || ws != 4 ) { MSG = "PEDT016" ; return ; }
			else if ( !returnLabelItr( w3, its, i ) || !returnLabelItr( w4, ite, j ) ) { MSG = "PEDT01F" ; return ; }
			else if ( i > j )
			{
				it  = its ;
				its = ite ;
				ite = it  ;
			}
			for ( it = its ; it != ite ; it++ ) { (*it)->il_lc1 = "" ; }
			rebuildZAREA = true ;
		}
		else if ( ( (w2 == "LABEL" || w2 == "LABELS") && w3 == "" ) )
		{
			for_each( data.begin(), data.end(), [](iline * & a) { a->il_label = "" ; } ) ;
			rebuildZAREA = true ;
		}
		else if ( (w2 == "X") || (w2 == "EX") || (w2 == "EXC") || (w2 == "EXCLUDED") )
		{
			if ( w3 == "" )
			{
				its = data.begin() ;
				ite = data.end()   ;
			}
			else if ( w3[ 0 ] != '.' || w4[ 0 ] != '.' || ws != 4 ) { MSG = "PEDT016" ; return ; }
			else if ( !returnLabelItr( w3, its, i ) || !returnLabelItr( w4, ite, j ) ) { MSG = "PEDT01F" ; return ; }
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
	}
	else if ( w1 == "SAVE" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		saveFile()   ;
		ZCMD = ""    ;
	}
	else if ( w1 == "SHOW" )
	{
		log( "A", " Dumping contents:: (undo level) " << Level << endl ; )
		log( "A", " ++++++++++++++++++++++++++++++++++++++++" << endl ; )
		it = data.begin() ;
		log( "A", " Global UNDO stack size: " << (*it)->get_Global_Undo_Size() << endl  ; )
		log( "A", " Global REDO stack size: " << (*it)->get_Global_Redo_Size() << endl  ; )
		vector<icmd>::iterator itc         ;
		ZCMD = ""    ;
	}
	else if ( w1 == "SHOWALL" )
	{
		log( "A", " Dumping array contents:: (undo level) " << Level << endl ; )
		log( "A", " ++++++++++++++++++++++++" << endl ; )
		it = data.begin() ;
		log( "A", " Global UNDO stack size: " << (*it)->get_Global_Undo_Size() << endl  ; )
		log( "A", " Global REDO stack size: " << (*it)->get_Global_Redo_Size() << endl  ; )
		for ( ; it != data.end() ; it++ )
		{
			if ( !(*it)->il_file ) { continue ; }
			log( "A", " Current  lvl: " << (*it)->get_idata_lvl() << endl  ; )
			if ( (*it)->il_deleted) { log( "A", " Line is Deleted" << endl ; ) }
			if ( (*it)->il_excl   ) { log( "A", " Line is Excluded" << endl ; ) }
			log( "A", " Line command: " << (*it)->il_lc1 << endl ; )
			log( "A", " Line label  : " << (*it)->il_label << endl ; )
			log( "A", " Record      : " << (*it)->get_idata() << endl ; )
			log( "A", " ++++++++++++++++++++++++++++++++++++++++++++++++" << endl ; )
		}
		ZCMD = ""    ;
	}
	else if ( w1 == "TABS" )
	{
		if ( ws > 2 ) { MSG = "PEDT011" ; return ; }
		if ( w2.size() == 1 )
		{
			tabsChar = w2[ 0 ] ;
		}
		else if ( w2 == "ON" || ws == 1 )
		{
			profTabs     = true ;
			rebuildZAREA = true ;
		}
		else if ( w2 == "OFF" )
		{
			profTabs     = false ;
			rebuildZAREA = true ;
		}
		else { MSG = "PEDT011" ; }
		ZCMD = "" ;
	}
	else if ( w1 == "UNDO" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		actionUNDO() ;
		ZCMD = ""    ;
	}
	else if ( w1 == "X" || w1 == "EX" || w1 == "EXCLUDE" )
	{
		if ( ws == 1 ) { MSG = "PEDT011" ; return ; }
		if ( w2 == "ALL" && ws == 2 )
		{
			for ( dl = 1 ; dl < data.size()-1 ; dl++ )
			{
				if (  data.at( dl )->il_deleted ) { continue ; }
				data.at( dl )->il_excl = true ;
			}
		}
		else
		{
			actionExclude() ;
		}
		rebuildZAREA = true ;
		ZCMD = ""           ;
	}
	else { MSG = "PEDT011" ; }
}


void PEDIT01::actionLineCommands()
{
	// For each line in the data vector, action the line commands
	// For copy/move/repeat preserve flags: file, note, prof, col, excl and hex

	int j       ;
	int k       ;
	int size    ;
	int tURID   ;

	string tmp ;
	vector<ipline> vip   ;

	idata    t ;
	ipline  ip ;

	bool overlayOK  ;

	iline * p_iline  ;

	vector<iline * >::iterator il_itr  ;
	vector<iline * >::iterator il_ite  ;
	vector<iline * >::iterator ita     ;
	vector<ipline>::iterator new_end   ;
	vector<icmd>::iterator itc         ;

	map<int, bool>uridl                  ; //testing only

	if ( !checkLineCommands() ) { return ; }

	for ( ita = data.begin() ; ita != data.end() ; ita++ )
	{
		(*ita)->clearLc12() ;
	}
	if ( MSG != "" ) { return ; }

	for ( itc = icmds.begin() ; itc != icmds.end() ; itc++ )
	{
		if ( itc->icmd_Rpt == -1 ) { itc->icmd_Rpt = 1 ; }
		if ( itc->icmd_COMMAND == "BNDS" )
		{
			Level++;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			p_iline          = new iline( taskid() ) ;
			p_iline->il_bnds = true      ;
			p_iline->put_idata( "", Level )  ;
			il_itr = data.insert( il_itr, p_iline ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "COL" || itc->icmd_COMMAND == "COLS" )
		{
			Level++;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			p_iline          = new iline( taskid() ) ;
			p_iline->il_col  = true      ;
			p_iline->put_idata( "", Level )  ;
			il_itr = data.insert( il_itr, p_iline ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "C" || itc->icmd_COMMAND == "M" )
		{
			Level++ ;
			vip.clear()   ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				ip.ip_file = (*il_itr)->il_file ;
				ip.ip_note = (*il_itr)->il_note ;
				ip.ip_prof = (*il_itr)->il_prof ;
				ip.ip_col  = (*il_itr)->il_col  ;
				ip.ip_bnds = (*il_itr)->il_bnds ;
				ip.ip_mask = (*il_itr)->il_mask ;
				ip.ip_excl = (*il_itr)->il_excl ;
				ip.ip_hex  = (*il_itr)->il_hex  ;
				ip.ip_data = (*il_itr)->get_idata() ;
				vip.push_back( ip ) ;
				il_itr++ ;
			}
			overlayOK = true ;
			if ( itc->icmd_cutpaste )
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
							if ( !(*il_ite)->il_file )    { j--; il_ite++ ; continue ; }
							if (  (*il_ite)->il_deleted ) { j--; il_ite++ ; continue ; }
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
						p_iline->il_file = vip[ j ].ip_file ;
						p_iline->il_note = vip[ j ].ip_note ;
						p_iline->il_prof = vip[ j ].ip_prof ;
						p_iline->il_col  = vip[ j ].ip_col  ;
						p_iline->il_bnds = vip[ j ].ip_bnds ;
						p_iline->il_mask = vip[ j ].ip_mask ;
						p_iline->il_excl = vip[ j ].ip_excl ;
						p_iline->il_hex  = vip[ j ].ip_hex  ;
						p_iline->put_idata( vip[ j ].ip_data, Level ) ;
						il_ite++ ;
						il_ite = data.insert( il_ite, p_iline ) ;
					}
				}
			}
			if ( itc->icmd_COMMAND == "M" && overlayOK )
			{
				il_itr = getLineItr( itc->icmd_sURID ) ;
				for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
				{
					if ( (*il_itr)->il_bod )     { break                     ; }
					if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
					(*il_itr)->put_idata( "", Level ) ;
					(*il_itr)->set_il_deleted()       ;
					il_itr++ ;
				}
			}
			if ( !overlayOK && itc->icmd_COMMAND[ 0 ] == 'M' ) { MSG = "PEDT011D" ; }
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "CC" || itc->icmd_COMMAND == "MM" )
		{
			Level++ ;
			vip.clear()   ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_bod ) { break ; }
				ip.ip_file = (*il_itr)->il_file ;
				ip.ip_note = (*il_itr)->il_note ;
				ip.ip_prof = (*il_itr)->il_prof ;
				ip.ip_col  = (*il_itr)->il_col  ;
				ip.ip_bnds = (*il_itr)->il_bnds ;
				ip.ip_mask = (*il_itr)->il_mask ;
				ip.ip_excl = (*il_itr)->il_excl ;
				ip.ip_hex  = (*il_itr)->il_hex  ;
				ip.ip_data = (*il_itr)->get_idata() ;
				vip.push_back( ip ) ;
			}
			overlayOK = true ;
			if ( itc->icmd_cutpaste )
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
					if ( itc->icmd_oURID > 0 )
					{
						ita = getLineItr( itc->icmd_oURID ) ;
					}
					k = 0 ;
					while ( true )
					{
						for ( j = 0 ; j < vip.size() ; j++ )
						{
							if ( !(*il_ite)->il_file )    { j--; il_ite++ ; continue ; }
							if (  (*il_ite)->il_deleted ) { j--; il_ite++ ; continue ; }
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
						p_iline->il_file = vip[ j ].ip_file ;
						p_iline->il_note = vip[ j ].ip_note ;
						p_iline->il_prof = vip[ j ].ip_prof ;
						p_iline->il_col  = vip[ j ].ip_col  ;
						p_iline->il_bnds = vip[ j ].ip_bnds ;
						p_iline->il_mask = vip[ j ].ip_mask ;
						p_iline->il_excl = vip[ j ].ip_excl ;
						p_iline->il_hex  = vip[ j ].ip_hex  ;
						p_iline->put_idata( vip[ j ].ip_data, Level ) ;
						il_ite++ ;
						il_ite = data.insert( il_ite, p_iline ) ;
					}
				}
			}
			if ( itc->icmd_COMMAND == "MM" && overlayOK )
			{
				il_itr = getLineItr( itc->icmd_sURID ) ;
				il_ite = getLineItr( itc->icmd_eURID ) ;
				il_ite++ ;
				for ( ; il_itr != il_ite ; il_itr++ )
				{
					if ( (*il_itr)->il_deleted ) { continue ; }
					(*il_itr)->put_idata( "", Level ) ;
					(*il_itr)->set_il_deleted()       ;
				}
			}
			if ( !overlayOK && itc->icmd_COMMAND[ 0 ] == 'M' ) { MSG = "PEDT011D" ; }
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "D" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->put_idata( "", Level ) ;
				(*il_itr)->set_il_deleted()       ;
				il_itr++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "DD" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->put_idata( "", Level ) ;
				(*il_itr)->set_il_deleted()       ;

			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "F" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if (  (*il_itr)->il_bod )    { break                      ; }
				if ( !(*il_itr)->il_excl )   { break                      ; }
				if (  (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->il_excl = false ;
				il_itr++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "HX" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->il_hex = true ;
				il_itr++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "HXX" || itc->icmd_COMMAND == "HXHX" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->il_hex = true ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "I" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			k = (*il_itr)->get_idata().find_first_not_of( ' ', startCol-1 ) ;
			k = (k != string::npos) ? k + CLINESZ + 1 : CLINESZ + 1 ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				p_iline = new iline( taskid() )        ;
				p_iline->il_file    = true ;
				p_iline->il_newisrt = true ;
				p_iline->put_idata( maskLine, Level ) ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
				il_itr = getValidDataLine( il_itr ) ;
				placeCursor( (*il_itr)->il_URID, 4, k ) ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "L" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->il_excl = false ;
				il_itr++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "LC" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->put_idata( lower( (*il_itr)->get_idata() ), Level ) ;
				il_itr++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "LCC" || itc->icmd_COMMAND == "LCLC" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->put_idata( lower( (*il_itr)->get_idata() ), Level ) ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "MASK" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			p_iline          = new iline( taskid() ) ;
			p_iline->il_mask = true      ;
			p_iline->put_idata( maskLine ) ;
			il_itr = data.insert( il_itr, p_iline ) ;
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 2 ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "MD" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_file )    { continue                  ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				p_iline = new iline( taskid() )     ;
				p_iline->il_file = true ;
				p_iline->put_idata( (*il_itr)->get_idata(), Level ) ;
				(*il_itr)->put_idata( "", Level ) ;
				(*il_itr)->set_il_deleted() ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
				il_itr++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 2 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "MMD" || itc->icmd_COMMAND == "MDMD" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_file )    { continue ; }
				if ( (*il_itr)->il_deleted ) { continue ; }
				p_iline = new iline( taskid() )        ;
				p_iline->il_file    = true ;
				p_iline->put_idata( (*il_itr)->get_idata(), Level ) ;
				(*il_itr)->put_idata( "", Level ) ;
				(*il_itr)->set_il_deleted()       ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
				il_ite = getLineItr( itc->icmd_eURID )  ;
				il_ite++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 2 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "R" )
		{
			Level++ ;
			il_itr  = getLineItr( itc->icmd_sURID ) ;
			tmp     = (*il_itr)->get_idata() ;
			ip.ip_file = (*il_itr)->il_file ;
			ip.ip_note = (*il_itr)->il_note ;
			ip.ip_prof = (*il_itr)->il_prof ;
			ip.ip_col  = (*il_itr)->il_col  ;
			ip.ip_bnds = (*il_itr)->il_bnds ;
			ip.ip_mask = (*il_itr)->il_mask ;
			ip.ip_excl = (*il_itr)->il_excl ;
			ip.ip_hex  = (*il_itr)->il_hex  ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				p_iline          = new iline( taskid() )  ;
				p_iline->il_file = ip.ip_file ;
				p_iline->il_note = ip.ip_note ;
				p_iline->il_prof = ip.ip_prof ;
				p_iline->il_col  = ip.ip_col  ;
				p_iline->il_bnds = ip.ip_bnds ;
				p_iline->il_mask = ip.ip_mask ;
				p_iline->il_excl = ip.ip_excl ;
				p_iline->il_hex  = ip.ip_hex  ;
				p_iline->put_idata( tmp, Level ) ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 2 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "RR" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			for ( ; ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				ip.ip_file = (*il_itr)->il_file ;
				ip.ip_note = (*il_itr)->il_note ;
				ip.ip_prof = (*il_itr)->il_prof ;
				ip.ip_col  = (*il_itr)->il_col  ;
				ip.ip_bnds = (*il_itr)->il_bnds ;
				ip.ip_mask = (*il_itr)->il_mask ;
				ip.ip_excl = (*il_itr)->il_excl ;
				ip.ip_hex  = (*il_itr)->il_hex  ;
				ip.ip_data = (*il_itr)->get_idata() ;
				vip.push_back( ip ) ;
				if ( il_itr == il_ite ) { break ; }
			}
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				il_ite = getLineItr( itc->icmd_eURID ) ;
				for ( k = 0 ; k < vip.size() ; k++ )
				{
					p_iline          = new iline( taskid() )        ;
					p_iline->il_file = vip[ k ].ip_file ;
					p_iline->il_note = vip[ k ].ip_note ;
					p_iline->il_prof = vip[ k ].ip_prof ;
					p_iline->il_col  = vip[ k ].ip_col  ;
					p_iline->il_bnds = vip[ k ].ip_bnds ;
					p_iline->il_mask = vip[ k ].ip_mask ;
					p_iline->il_excl = vip[ k ].ip_excl ;
					p_iline->il_hex  = vip[ k ].ip_hex  ;
					p_iline->put_idata( vip[ k ].ip_data, Level ) ;
					il_ite++ ;
					il_ite = data.insert( il_ite, p_iline ) ;
				}
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "TABS" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			p_iline          = new iline( taskid() ) ;
			p_iline->il_tabs = true      ;
			p_iline->put_idata( tabsLine ) ;
			il_itr = data.insert( il_itr, p_iline ) ;
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 2 ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "TS" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			tmp = (*il_itr)->get_idata() ;
			if ( aURID == (*il_itr)->il_URID && aCol > CLINESZ )
			{
				if ( (*il_itr)->il_file && !(*il_itr)->il_deleted )
				{
					(*il_itr)->put_idata( substr( tmp, 1, aCol+startCol-10), Level ) ;
					p_iline = new iline( taskid() )           ;
					p_iline->il_file    = true ;
					p_iline->il_newisrt = true ;
					p_iline->put_idata( maskLine, Level ) ;
					tURID = p_iline->il_URID ;
					il_itr++ ;
					il_itr  = data.insert( il_itr, p_iline ) ;
					p_iline = new iline( taskid() )           ;
					p_iline->il_file = true ;
					p_iline->put_idata( substr( tmp, aCol+startCol-9), Level ) ;
					il_itr++ ;
					il_itr = data.insert( il_itr, p_iline ) ;
				}
			}
			else
			{
				p_iline = new iline( taskid() )        ;
				p_iline->il_file    = true ;
				p_iline->il_newisrt = true ;
				p_iline->put_idata( maskLine, Level ) ;
				tURID = p_iline->il_URID ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
			}
			k = tmp.find_first_not_of( ' ', startCol-1 ) ;
			k = (k != string::npos) ? k + CLINESZ + 1 : CLINESZ + 1 ;
			placeCursor( tURID, 4, k ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "UC" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->put_idata( upper( (*il_itr)->get_idata() ), Level ) ;
				il_itr++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "UCC" || itc->icmd_COMMAND == "UCUC" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
					if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->put_idata( upper( (*il_itr)->get_idata() ), Level ) ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "X" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )    { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->il_excl = true ;
				il_itr++ ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "XX" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted  ) { continue ; }
				(*il_itr)->il_excl = true ;
			}
			rebuildZAREA = true ;
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
		}
		else if ( itc->icmd_COMMAND == ")" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_itr = getValidDataLine( il_itr ) ;
			(*il_itr)->put_idata( string( itc->icmd_Rpt, ' ' ) + (*il_itr)->get_idata(), Level ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "))" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted  ) { continue ; }
				(*il_itr)->put_idata( string( itc->icmd_Rpt, ' ' ) + (*il_itr)->get_idata(), Level ) ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 1 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "(" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_itr = getValidDataLine( il_itr ) ;
			(*il_itr)->put_idata( substr( (*il_itr)->get_idata(), itc->icmd_Rpt+1 ), Level ) ;
			placeCursor( (*il_itr)->il_URID, 2 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "((" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->put_idata( substr( (*il_itr)->get_idata(), itc->icmd_Rpt+1 ), Level ) ;
			}
			il_itr = getValidDataLine( il_itr ) ;
			placeCursor( (*il_itr)->il_URID, 2 ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_cutpaste )
		{
			Level++;
			getClipboard( vip ) ;
			il_itr  = getLineItr( itc->icmd_dURID ) ;
			if ( itc->icmd_ABO == 'B' ) { il_itr = getLineBeforeItr( itc->icmd_dURID ) ; }
			for ( j = 0 ; j < vip.size() ; j++ )
			{
				p_iline          = new iline( taskid() )  ;
				p_iline->il_file = true       ;
				p_iline->put_idata( vip[ j ].ip_data, Level ) ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
			}
			vreplace( "ZEDLNES", d2ds( vip.size() ) ) ;
			vreplace( "CLIPNAME", clipboard ) ;
			MSG  = "PEDT012D"    ;
			ZCMD = ""            ;
			placeCursor( (*il_itr)->il_URID, 2 ) ;
			rebuildZAREA = true  ;
			fileChanged  = true  ;
		}
	}
	icmds.clear() ;
}


bool PEDIT01::checkLineCommands()
{
	// For each line in the data vector, check the line commands
	// Treat single commands on an exluded block as a block command

	// cmd1 - block commands, including single commands on excluded lines, treated as a block command and those requiring A/B/O
	// cmd2 - single commands requiring A/B/O
	// cmd3 - self-contained single commands and block commands (single commands on an excluded line), not required A/B/O
	// abo  - contains a, b or single o M/C positions
	// oo   - contains block OO M/C positions.  Use abo for single O positions

	icmd cmd1    ;
	icmd cmd2    ;
	icmd cmd3    ;
	icmd abo     ;
	icmd oo      ;
	int rept     ;
	string lc2   ;

	bool iu_cmd1 ;
	bool iu_cmd2 ;
	bool iu_abo  ;
	bool iu_oo   ;

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	icmds.clear() ;

	iu_cmd1 = false ;
	iu_cmd2 = false ;
	iu_abo  = false ;
	iu_oo   = false ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_lc1 == ""  ) { continue ; }
		if ( (*it)->il_lc1[ 0 ] == '.' )
		{
			if ( (*it)->il_lc1.size() == 1 ) { MSG = "PEDT014" ; return false ; }
			for ( itt = data.begin() ; itt != data.end() ; itt++ )
			{
				if ( (*itt)->il_label == (*it)->il_lc1 ) { (*itt)->il_label = "" ; break ; }
			}
			(*it)->il_label = (*it)->il_lc1 ;
			(*it)->il_lc1   = "" ;
			rebuildZAREA = true  ;
			continue             ;
		}
		if ( !xformLineCmd( (*it)->il_lc1, lc2, rept ) )
		{
			if ( MSG == "" ) { MSG = "PEDT01Y" ; }
			break ;
		}
		if ( wordpos( lc2, blkcmds + " " + sglcmds )   == 0 ) { MSG = "PEDT012" ; break ; }
		if ( (*it)->il_tod && wordpos( lc2, todlcmds ) == 0 ) { MSG = "PEDT013" ; break ; }
		if ( (*it)->il_bod && wordpos( lc2, bodlcmds ) == 0 ) { MSG = "PEDT013" ; break ; }
		if ( lc2 == "OO" )
		{
			if ( !iu_oo )
			{
				oo.icmd_sURID = (*it)->il_URID ;
				iu_oo = true ;
			}
			else
			{
				if ( oo.icmd_eURID > 0 ) { MSG = "PEDT01Y" ; break ; }
				oo.icmd_eURID = (*it)->il_URID ;
				oo.icmd_OSize = getRangeSize( oo.icmd_sURID, oo.icmd_eURID ) ;
				if ( iu_cmd1 )
				{
					if ( cmd1.icmd_eURID == 0 ) { MSG = "PEDT01W" ; break ; }
					if ( !cmd1.icmd_ABO_req )   { MSG = "PEDT01Y" ; break ; }
					cmd1.icmd_ABO   = 'O' ;
					cmd1.icmd_dURID = oo.icmd_sURID ;
					cmd1.icmd_oURID = oo.icmd_eURID ;
					cmd1.icmd_OSize = oo.icmd_OSize ;
					icmds.push_back( cmd1 ) ;
					cmd1.icmd_clear()       ;
					oo.icmd_clear()         ;
					iu_cmd1 = false         ;
					iu_oo   = false         ;
				}
				else if ( iu_cmd2 )
				{
					if ( !cmd2.icmd_ABO_req )  { MSG = "PEDT01Y" ; break ; }
					cmd2.icmd_ABO   = 'O' ;
					cmd2.icmd_dURID = oo.icmd_sURID ;
					cmd2.icmd_oURID = oo.icmd_eURID ;
					cmd2.icmd_OSize = oo.icmd_OSize ;
					icmds.push_back( cmd2 ) ;
					cmd2.icmd_clear()       ;
					oo.icmd_clear()         ;
					iu_cmd2 = false         ;
					iu_oo   = false         ;
				}
			}
		}
		else if ( wordpos( lc2, blkcmds ) > 0 )
		{
			if ( iu_cmd1 && cmd1.icmd_COMMAND != lc2 ) { MSG = "PEDT01Y" ; break ; }
			if ( iu_cmd1 && cmd1.icmd_eURID > 0 )      { MSG = "PEDT01W" ; break ; }
			if ( wordpos( lc2, ABOReq ) > 0 )
			{
				cmd1.icmd_ABO_req = true ;
			}
			if ( !iu_cmd1 )
			{
				cmd1.icmd_COMMAND = lc2 ;
				cmd1.icmd_sURID   = (*it)->il_URID ;
				cmd1.icmd_Rpt     = rept ;
				iu_cmd1           = true ;
			}
			else
			{
				if ( (cmd1.icmd_Rpt != -1 && rept != -1) && cmd1.icmd_Rpt != rept ) { MSG = "PEDT011B" ; break ; }
				if (  cmd1.icmd_Rpt == -1 ) { cmd1.icmd_Rpt = rept ; }
				cmd1.icmd_eURID = (*it)->il_URID ;
				if ( wordpos( lc2, ABOReq ) == 0 )
				{
					icmds.push_back( cmd1 ) ;
					iu_cmd1  = false        ;
				}
				else
				{
					if ( iu_abo )
					{
						cmd1.icmd_ABO   = abo.icmd_ABO ;
						cmd1.icmd_dURID = abo.icmd_dURID ;
						cmd1.icmd_OSize = abo.icmd_OSize ;
						icmds.push_back( cmd1 )  ;
						cmd1.icmd_clear()        ;
						iu_cmd1  = false         ;
						iu_abo   = false         ;
					}
					else if ( iu_oo )
					{
						if ( oo.icmd_eURID == 0 ) { MSG = "PEDT01W" ; break ; }
						cmd1.icmd_dURID = oo.icmd_sURID ;
						cmd1.icmd_OSize = oo.icmd_OSize ;
						cmd1.icmd_oURID = oo.icmd_eURID ;
						cmd1.icmd_ABO   = 'O'    ;
						icmds.push_back( cmd1 )  ;
						cmd1.icmd_clear()        ;
						oo.icmd_clear()          ;
						iu_cmd1  = false         ;
						iu_oo    = false         ;
					}
					else if ( cutActive && wordpos( lc2, CutCmds ) > 0 )
					{
						cmd1.icmd_cutpaste = true ;
						icmds.push_back( cmd1 )   ;
						cmd1.icmd_clear()         ;
						iu_cmd1   = false         ;
						cutActive = false         ;
					}
				}
			}
		}
		else
		{
			if ( wordpos( lc2, ABOList ) > 0 )
			{
				if ( iu_abo ) { MSG = "PEDT01V" ; break ; }
				else if ( pasteActive && wordpos( lc2, PasteCmds ) > 0 )
				{
					abo.icmd_cutpaste = true  ;
					abo.icmd_dURID    = (*it)->il_URID  ;
					abo.icmd_ABO      = lc2[ 0 ] ;
					icmds.push_back( abo ) ;
					abo.icmd_clear()       ;
					pasteActive = false    ;
				}
				else if ( iu_cmd1 && cmd1.icmd_ABO_req && cmd1.icmd_eURID > 0 )
				{
					cmd1.icmd_OSize = rept ;
					cmd1.icmd_dURID = (*it)->il_URID  ;
					cmd1.icmd_ABO   = lc2[ 0 ] ;
					icmds.push_back( cmd1 )  ;
					iu_cmd1 = false ;
					cmd1.icmd_clear() ;
				}
				else if ( iu_cmd2 && cmd2.icmd_ABO_req )
				{
					cmd2.icmd_dURID = (*it)->il_URID  ;
					cmd2.icmd_ABO   = lc2[ 0 ] ;
					cmd2.icmd_OSize = rept  ;
					icmds.push_back( cmd2 ) ;
					iu_cmd2 = false ;
					cmd2.icmd_clear() ;
				}
				else
				{
					abo.icmd_dURID = (*it)->il_URID ;
					abo.icmd_ABO   =  lc2[ 0 ]      ;
					abo.icmd_OSize = rept ;
					iu_abo         = true ;
				}
			}
			else
			{
				if ( wordpos( lc2, ABOReq ) == 0 )
				{
					if ( (*it)->il_excl )
					{
						cmd3.icmd_COMMAND = lc2 + lc2  ;
						cmd3.icmd_sURID   = (*it)->il_URID ;
						cmd3.icmd_eURID   = getLastEX( (*it)->il_URID ) ;
						cmd3.icmd_Rpt     = rept ;
						icmds.push_back( cmd3 )  ;
						cmd3.icmd_clear()        ;
					}
					else
					{
						cmd3.icmd_COMMAND = lc2  ;
						cmd3.icmd_sURID   = (*it)->il_URID ;
						cmd3.icmd_Rpt     = rept ;
						icmds.push_back( cmd3 )  ;
						cmd3.icmd_clear()        ;
					}
				}
				else
				{
					if ( (*it)->il_excl )
					{
						cmd1.icmd_ABO_req = true ;
						if ( iu_cmd1 ) { MSG = "PEDT01W" ; break ; }
						cmd1.icmd_COMMAND = lc2 + lc2 ;
						cmd1.icmd_sURID   = (*it)->il_URID ;
						cmd1.icmd_eURID   = getLastEX( (*it)->il_URID ) ;
						cmd1.icmd_Rpt     = rept ;
						iu_cmd1           = true ;
						if ( iu_abo )
						{
							cmd1.icmd_ABO   = abo.icmd_ABO   ;
							cmd1.icmd_dURID = abo.icmd_dURID ;
							cmd1.icmd_OSize = abo.icmd_OSize ;
							icmds.push_back( cmd1 )  ;
							cmd1.icmd_clear()        ;
							iu_cmd1  = false         ;
							iu_abo   = false         ;
						}
						else if ( cutActive && wordpos( lc2, CutCmds ) > 0 )
						{
							cmd1.icmd_cutpaste = true ;
							icmds.push_back( cmd1 )   ;
							cmd1.icmd_clear()         ;
							iu_cmd1     = false       ;
							cutActive   = false       ;
						}
					}
					else
					{
						if ( iu_cmd2 ) { MSG = "PEDT01Y" ; break ; }
						cmd2.icmd_COMMAND = lc2  ;
						cmd2.icmd_sURID   = (*it)->il_URID ;
						cmd2.icmd_Rpt     = rept ;
						cmd2.icmd_ABO_req = true ;
						iu_cmd2           = true ;
						if ( iu_abo )
						{
							cmd2.icmd_ABO   = abo.icmd_ABO   ;
							cmd2.icmd_dURID = abo.icmd_dURID ;
							cmd2.icmd_OSize = abo.icmd_OSize ;
							icmds.push_back( cmd2 )  ;
							cmd2.icmd_clear()        ;
							iu_cmd2  = false         ;
							iu_abo   = false         ;
						}
						else if ( iu_oo )
						{
							if ( oo.icmd_eURID == 0 ) { MSG = "PEDT01W" ; break ; }
							cmd2.icmd_ABO   = oo.icmd_ABO   ;
							cmd2.icmd_dURID = oo.icmd_sURID ;
							cmd2.icmd_OSize = oo.icmd_OSize ;
							cmd2.icmd_oURID = oo.icmd_eURID ;
							icmds.push_back( cmd2 )  ;
							oo.icmd_clear()          ;
							iu_cmd2  = false         ;
							iu_oo    = false         ;
						}
						else if ( cutActive && wordpos( lc2, CutCmds ) > 0 )
						{
							cmd2.icmd_cutpaste = true ;
							icmds.push_back( cmd2 )   ;
							cmd2.icmd_clear()         ;
							iu_cmd2   = false         ;
							cutActive = false         ;
						}
					}
				}
			}
		}
	}
	if ( MSG != "" )
	{
		placeCursor( (*it)->il_URID, 1 ) ;
		return false ;
	}
	if ( iu_cmd1 ) { MSG = "PEDT01W"  ; return false ; }
	if ( iu_cmd2 ) { MSG = "PEDT01Z"  ; return false ; }
	if ( iu_abo  ) { MSG = "PEDT01Z"  ; return false ; }
	if ( iu_oo   ) { MSG = "PEDT01Z"  ; return false ; }
	if ( cutActive   ) { MSG = "PEDT012E" ; return false ; }
	if ( pasteActive ) { MSG = "PEDT012F" ; return false ; }
	return true ;
}


void PEDIT01::actionUNDO()
{
	// Get the URID/lvl for the top change and undo.  If the change before has the save lvl, repeat
	// until we get a URID/lvl for a different lvl or no more ichange records (URID==0)

	// Can take a while for large undo's.  Disable/enable timeout

	int   lvl ;
	ichange t ;

	vector<iline * >::iterator it  ;

	if ( recoverOFF[ taskid() ] ) { MSG = "PEDT01U" ; return ; }

	it = data.begin() ;
	t  = (*it)->get_Undo_URID() ;
	if ( t.iURID == 0 )
	{
		log( "A", "No more undo's available" << endl ; )
		MSG = "PEDT017" ;
		return ;
	}
	lvl = t.ilvl ;

	control( "TIMEOUT", "DISABLE" ) ;
	for ( ; it != data.end() ; it++ )
	{
		if ( t.iURID == (*it)->il_URID )
		{
			(*it)->undo_idata() ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			t = (*it)->get_Undo_URID() ;
			if ( t.iURID == 0 || lvl != t.ilvl ) { break ; }
			it = data.begin() ;
		}
	}
	Level = t.ilvl ;
	control( "TIMEOUT", "ENABLE" ) ;
}


void PEDIT01::actionREDO()
{
	// Can take a while for large redo's.  Disable/enable timeout

	int   lvl ;
	ichange t ;

	vector<iline * >::iterator it  ;

	if ( recoverOFF[ taskid() ] ) { MSG = "PEDT01U" ; return ; }

	it = data.begin() ;
	t  = (*it)->get_Redo_URID() ;
	if ( t.iURID == 0 )
	{
		log( "A", "No redo's available" << endl ; )
		MSG = "PEDT018" ;
		return ;
	}
	lvl = t.ilvl ;

	control( "TIMEOUT", "DISABLE" ) ;
	for ( ; it != data.end() ; it++ )
	{
		if ( t.iURID == (*it)->il_URID )
		{
			(*it)->redo_idata() ;
			rebuildZAREA = true ;
			fileChanged  = true ;
			t = (*it)->get_Redo_URID() ;
			if ( t.iURID == 0 || lvl != t.ilvl ) { break ; }
			it = data.begin() ;
		}
	}
	Level = t.ilvl ;
	control( "TIMEOUT", "ENABLE" ) ;
}



int PEDIT01::setFindChangeExcl( char fcx_type )
{
	int i        ;
	int j        ;
	int p1       ;
	int p2       ;
	int ws       ;
	char c1      ;
	char c2      ;
	string delim ;
	string cmd   ;
	string ucmd  ;
	string w1    ;
	string pic   ;
	e_find t     ;

	vector<iline * >::iterator it  ;

	static char quote('\"')  ;
	static char apost('\'')  ;

	MSG  = ""                 ;
	cmd  = " " + subword( ZCMD, 2 ) + " " ;

	p1 = cmd.find( quote ) ;
	p2 = cmd.find( apost ) ;

	if      ( p1 == string::npos ) { delim = string( 1, apost ) ; }
	else if ( p2 == string::npos ) { delim = string( 1, quote ) ; }
	else if ( p1 < p2 )            { delim = string( 1, quote ) ; }
	else                           { delim = string( 1, apost ) ; }

	if ( p1 = pos( delim, cmd  ) )
	{
		p2  = pos( delim, cmd,  p1+1 ) ;
		if ( p2 == 0 ) { MSG = "PEDT01H" ; return 20 ; }
		c1 = toupper( cmd[ p1-2 ] ) ;
		c2 = toupper( cmd[ p2   ] ) ;
		if ( c1 == ' ' && c2 == ' ' ) { t.fcx_text = true ; }
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
		else                               { MSG = "PEDT01I" ; return 20 ; }
		if ( t.fcx_text )
		{
			t.fcx_string = upper( substr( cmd, (p1+1), (p2-p1-1) ) ) ;
		}
		else
		{
			t.fcx_string = substr( cmd, (p1+1), (p2-p1-1) ) ;
			t.fcx_asis   = true ;
		}
		cmd = delstr( cmd, (p1-1), (p2-p1+3) ) ;
	}
	else
	{
		t.fcx_text   = true ;
		t.fcx_string = upper( word( cmd, 1 ) ) ;
		cmd          = subword( cmd, 2 ) ;
	}

	t.fcx_estring = t.fcx_string ;

	if ( fcx_type == 'C' )
	{
		if ( p1 = pos( delim, cmd ) )
		{
			p2  = pos( delim, cmd, p1) ;
			if ( p2 == 0 ) { MSG = "PEDT01H" ; return 20 ; }
			c1 = toupper( cmd[ p1-2 ] ) ;
			c2 = toupper( cmd[ p2   ] ) ;
			if ( c1 == ' ' && c2 == ' ' ) { t.fcx_text = true ; }
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
			else                               { MSG = "PEDT01I" ; return 20 ; }
			if ( t.fcx_text )
			{
				t.fcx_cstring = substr( cmd, (p1+1), (p2-p1-1) ) ;
			}
			else
			{
				t.fcx_string = substr( cmd, (p1+1), (p2-p1-1) ) ;
				t.fcx_asis   = true ;
			}
			cmd = delstr( cmd, (p1-1), (p2-p1+3) ) ;
		}
	}

	if ( t.fcx_rreg ) { t.fcx_regreq = true ; }

	ucmd = upper( cmd ) ;
	if ( p1 = wordpos( "NEXT", ucmd ) )
	{
		t.fcx_dir = 'N' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 4 ) ;
	}
	else if ( p1 = wordpos( "PREV", ucmd ) )
	{
		t.fcx_dir = 'P' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 4 ) ;
	}
	else if ( p1 = wordpos( "FIRST", ucmd ) )
	{
		t.fcx_dir = 'F' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 5 ) ;
	}
	else if ( p1 = wordpos( "LAST", ucmd ) )
	{
		t.fcx_dir = 'L' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 4 ) ;
	}
	else if ( p1 = wordpos( "ALL", ucmd ) )
	{
		t.fcx_dir = 'A' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 3 ) ;
	}

	if ( fcx_type != 'X' )
	{
		if ( p1 = wordpos( "X", ucmd ) )
		{
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 1 ) ;
		}
		else if ( p1 = wordpos( "EX", ucmd ) )
		{
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 2 ) ;
		}
		else if ( p1 = wordpos( "NX", ucmd ) )
		{
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 2 ) ;
		}
	}

	if ( !t.fcx_rreg )
	{
		if ( p1 = wordpos( "CHARS", ucmd ) )
		{
			t.fcx_mtch  = 'C'  ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 5 ) ;
		}
		else if ( p1 = wordpos( "PRE", ucmd ) )
		{
			t.fcx_mtch   = 'P'  ;
			t.fcx_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 3 ) ;
		}
		else if ( p1 = wordpos( "PREFIX", ucmd ) )
		{
			t.fcx_mtch   = 'P'  ;
			t.fcx_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 6 ) ;
		}
		else if ( p1 = wordpos( "SUF", ucmd ) )
		{
			t.fcx_mtch   = 'S'  ;
			t.fcx_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 3 ) ;
		}
		else if ( p1 = wordpos( "SUFFIX", ucmd ) )
		{
			t.fcx_mtch   = 'S'  ;
			t.fcx_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 6 ) ;
		}
		else if ( p1 = wordpos( "WORD", ucmd ) )
		{
			t.fcx_mtch   = 'W'  ;
			t.fcx_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 4 ) ;
		}
	}

	ws = words( ucmd ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		w1 = word( ucmd, i ) ;
		if ( datatype( w1, 'W' ) )
		{
			if ( t.fcx_scol != 0 && t.fcx_ecol != 0 ) { MSG = "PEDT019" ; return 20 ; }
			j = ds2d( w1 ) ;
			if ( j < 1 || j > 65535 ) { MSG = "PEDT01J" ; return 20 ; }
			if ( t.fcx_scol == 0 ) { t.fcx_scol = j ; }
			else                   { t.fcx_ecol = j ; }
		}
		else if ( w1[ 0 ] == '.' )
		{
			if ( t.fcx_slab != "" && t.fcx_elab != "" ) { MSG = "PEDT01A" ; return 20 ; }
			if ( t.fcx_slab == "" ) { t.fcx_slab = w1 ; }
			else                    { t.fcx_elab = w1 ; }
		}
		else
		{
			if ( t.fcx_string != "" && fcx_type != 'C' ) { MSG = "PEDT01B" ; return 20 ; }
			if ( w1 == "*" )
			{
				if ( find_parms.fcx_string == "" ) { MSG = "PEDT01C" ; return 20 ; }
				else                               { w1  = find_parms.fcx_string ; }
			}
			if ( t.fcx_string != "" ) { t.fcx_cstring = w1 ; }
			else                      { t.fcx_string  = w1 ; }
		}
	}

	if ( t.fcx_scol != 0 && t.fcx_ecol == 0 ) { t.fcx_ecol = t.fcx_scol ; }
	if ( t.fcx_scol > t.fcx_ecol )
	{
		i          = t.fcx_scol ;
		t.fcx_scol = t.fcx_ecol ;
		t.fcx_ecol = i ;
	}
	if ( t.fcx_scol == 0 ) { t.fcx_scol = 1 ; }

	if ( t.fcx_string == "" || ( fcx_type == 'C' && t.fcx_cstring == "" ) ) { MSG = "PEDT01D" ; return 20 ; }

	if ( t.fcx_slab != "" &&  t.fcx_elab == "" )          { MSG = "PEDT01E" ; return 20 ; }
	if ( t.fcx_slab != "" && (t.fcx_slab == t.fcx_elab) ) { MSG = "PEDT01G" ; return 20 ; }
	if ( t.fcx_slab != "" )
	{
		if ( !returnLabelItr( t.fcx_slab, it, i ) ) { MSG = "PEDT01F" ; return 20 ; }
		if ( !returnLabelItr( t.fcx_elab, it, j ) ) { MSG = "PEDT01F" ; return 20 ; }
		if ( i > j )
		{
			w1         = t.fcx_slab ;
			t.fcx_slab = t.fcx_elab ;
			t.fcx_elab = w1 ;
		}
	}

	if ( t.fcx_hex )
	{
		if ( !isvalidHex( t.fcx_string ) )  { MSG = "PEDT01K" ; return 20 ; }
		t.fcx_string = xs2cs( t.fcx_string ) ;
		t.fcx_asis   = true                ;
	}

	if ( t.fcx_pic )
	{
		pic = "" ;
		j   = 0  ;
		// =  any character                   .  invalid characters
		// @  alphabetic characters           -  non-numeric characters
		// #  numeric characters              <  lower case alphabetics
		// $  special characters              >  upper case alphabetics
		// ¬  non-blank characters
		for ( i = 0 ; i < t.fcx_string.size() ; i++ )
		{
			switch ( t.fcx_string[ i ] )
			{
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

	if ( t.fcx_regreq && !t.fcx_rreg )
	{
		switch ( t.fcx_mtch )
		{
			case 'P':
				t.fcx_string = "[\\b|^](" + t.fcx_string + ")" ;
				break ;
			case 'S':
				t.fcx_string = "(" + t.fcx_string + ")\\b" ;
				break ;
			case 'W':
				t.fcx_string = "\\b(" + t.fcx_string + ")\\b" ;
				break ;
		}
	}

	t.fcx_fset = true ;
	if ( fcx_type == 'C') { t.fcx_cset = true ; }
	find_parms = t ;
	return 0 ;
}


void PEDIT01::actionFind()
{
	// dl has to be a uint so the corect getNextDataLine routine is called ( int is for URID )

	uint dl ;

	int c1 ;
	int c2 ;
	int i  ;
	int j  ;
	int p1 ;
	int oX ;
	int oY ;
	int offset ;

	bool found  ;
	bool found1 ;
	bool skip   ;

	vector<string>::iterator it  ;
	vector<string>::iterator itt ;
	vector<string>::iterator its ;
	vector<string>::iterator ite ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	regex regexp ;
	smatch results ;

	j = 0 ;

	if ( !find_parms.fcx_success && find_parms.fcx_dir == 'N' )
	{
		find_parms.fcx_dir = 'F' ;
	}
	else if ( !find_parms.fcx_success && find_parms.fcx_dir == 'P' )
	{
		find_parms.fcx_dir = 'L' ;
	}

	find_parms.fcx_success = false ;
	find_parms.fcx_rstring = "" ;
	find_parms.fcx_occurs  = 0  ;
	find_parms.fcx_URID    = 0  ;
	find_parms.fcx_lines   = 0  ;
	find_parms.fcx_offset  = 0  ;

	offset = aCol ;
	if ( find_parms.fcx_dir == 'F' || find_parms.fcx_dir == 'A' || find_parms.fcx_dir == 'L' ) { offset = 0 ; }

	if ( offset > ZAREAW ) { offset = offset - ZAREAW ; }
	if ( offset > 0 ) { oX = (offset % ZAREAW)-1 ; oY = offset / ZAREAW ; }
	else              { oX = 0        ; oY = 0 ; }
	if ( oX == -1 )   { oX = ZAREAW-1 ; oY--   ; }

	try
	{
		if ( find_parms.fcx_regreq )
		{
			if ( find_parms.fcx_asis )
			{
				regexp.assign( find_parms.fcx_string ) ;
			}
			else
			{
				regexp.assign( find_parms.fcx_string, boost::regex::ECMAScript|boost::regex::icase ) ;
			}
		}
	}
	catch  ( boost::regex_error& e )
	{
		if ( find_parms.fcx_regreq )
		{
			MSG = "PBRO01N" ;
			return          ;
		}
	}

	if      ( find_parms.fcx_dir == 'F' ) { dl = 1 ; }
	else if ( find_parms.fcx_dir == 'A' ) { dl = 1 ; }
	else if ( find_parms.fcx_dir == 'L' ) { dl = data.size()-2 ; }
	else if ( aRow == 0 )                 { dl = topLine                      ; }
	else                                  { dl = s2data.at( aRow-1 ).ipo_line ; }

	if ( dl == 0 ) { dl = 1 ; }

	dl   = getValidDataLine( dl ) ;
	while ( true )
	{
		skip = false ;
		c1   = 0     ;
		c2   = data[ dl ]->get_idata().size() - 1 ;

		if ( find_parms.fcx_scol > 0 )                               { c1 = find_parms.fcx_scol - 1 ; }
		if ( find_parms.fcx_ecol > 0 && c2 > find_parms.fcx_ecol-1 ) { c2 = find_parms.fcx_ecol - 1 ; }

		if ( LeftBnd  > c1+1 )                 { c1 = LeftBnd  - 1 ; }
		if ( RightBnd > 0 && RightBnd < c2+1 ) { c2 = RightBnd - 1 ; }

		if ( oX > 0 )
		{
			if ( oX > data[ dl ]->get_idata().size()-1 ) { oX = data[ dl ]->get_idata().size()-1 ; }
			if ( find_parms.fcx_dir == 'P' || find_parms.fcx_dir == 'L' )
			{
				if      ( oX < c1 )  { skip = true ; }
				else if ( oX <= c2 ) { c2   = oX-1 ; }
			}
			else
			{
				if      ( oX > c2  ) { skip = true ; }
				else if ( oX >= c1 ) { c1   = oX+1 ; }
			}
		}
		oX = 0 ;
		if ( skip || c1 >= c2 )
		{
			if ( find_parms.fcx_dir == 'L' || find_parms.fcx_dir == 'P' )
			{
				dl = getPrevDataLine( dl ) ;
			}
			else
			{
				dl = getNextDataLine( dl ) ;
			}
			if ( dl < 1 || dl > data.size()-2 ) { break ; }
			continue ;
		}

		if ( c1 > data[ dl ]->get_idata().size() -1 ) { abend() ; }
		if ( c2 > data[ dl ]->get_idata().size() -1 ) { abend() ; }
		if ( c1 < 0  ) { abend() ; }
		if ( c2 < 0  ) { abend() ; }
		if ( c1 > c2 ) { abend() ; }

		if ( find_parms.fcx_regreq )
		{
			found1 = true  ;
			found  = false ;
			itss = data[ dl ]->get_idata().begin() ;
			advance( itss, c1 ) ;
			itse = itss    ;
			advance( itse, c2-c1+1 ) ;
			if ( find_parms.fcx_oncol )
			{
				if ( regex_search( itss, itss, results, regexp ) )
				{
					find_parms.fcx_rstring = results[ 0 ] ;
					if ( itss == results[0].first )
					{
						found = true ;
						p1 = find_parms.fcx_scol-1 ;
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
						find_parms.fcx_rstring = results[ 0 ] ;
						for ( p1 = c1 ; itss != results[0].first ; itss++ ) { p1++ ; }
						c1 = p1 + 1 ;
						itss  = results[0].first ;
						itss++ ;
						if ( found1 ) { found1 = false ; find_parms.fcx_lines++ ; }
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
					if ( regex_search( itss, itse, results, regexp ) )
					{
						found = true ;
						find_parms.fcx_rstring = results[ 0 ] ;
						for ( p1 = c1 ; itss != results[0].first ; itss++ ) { p1++ ; }
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
			found1 = true ;
			while ( true )
			{
				found  = false ;
				if ( find_parms.fcx_dir == 'P' || find_parms.fcx_dir == 'L' )
				{
					if ( find_parms.fcx_asis ) { p1 = data[ dl ]->get_idata().rfind( find_parms.fcx_string, c2 )         ; }
					else                       { p1 = upper( data[ dl ]->get_idata()).rfind( find_parms.fcx_string, c2 ) ; }
					c2 = p1 - 1 ;
				}
				else
				{
					if ( find_parms.fcx_asis ) { p1 = data[ dl ]->get_idata().find( find_parms.fcx_string, c1 )         ; }
					else                       { p1 = upper( data[ dl ]->get_idata()).find( find_parms.fcx_string, c1 ) ; }
					c1 = p1 + 1 ;
				}
				if ( p1 != string::npos )
				{
					if ( find_parms.fcx_oncol )
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
				if ( find_parms.fcx_dir != 'A' || !found ) { break ; }
				find_parms.fcx_occurs++ ;
				if ( found1 ) { found1 = false ; find_parms.fcx_lines++ ; }
			}
		}
		if      ( find_parms.fcx_dir == 'A' ) { dl = getNextDataLine( dl ) ; }
		else if ( find_parms.fcx_dir == 'F' ) { if ( found ) { find_parms.fcx_dir = 'N' ; break ; } ; dl = getNextDataLine( dl ) ; }
		else if ( find_parms.fcx_dir == 'L' ) { if ( found ) { find_parms.fcx_dir = 'P' ; break ; } ; dl = getPrevDataLine( dl ) ; }
		else if ( find_parms.fcx_dir == 'N' ) { if ( found ) { break ; } ; dl = getNextDataLine( dl ) ; }
		else if ( find_parms.fcx_dir == 'P' ) { if ( found ) { break ; } ; getPrevDataLine( dl ) ; }
		if ( dl < 1 || dl > data.size()-2 ) { break ; }
	}
	find_parms.fcx_success = found ;
}


void PEDIT01::actionChange()
{
}


void PEDIT01::actionExclude()
{
	int i  ;
	int j  ;
	int p1 ;

	bool found ;

	regex regexp ;

	vector<iline * >::iterator it ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	if ( setFindChangeExcl( 'X' ) > 0 ) { return        ; }
	if ( find_parms.fcx_slab != "" )    { found = false ; }
	else                                { found = true  ; }
	j = 0 ;
	try
	{
		if ( find_parms.fcx_regreq )
		{
			if ( find_parms.fcx_asis )
				regexp.assign( find_parms.fcx_string )   ;
			else
				regexp.assign( find_parms.fcx_string, boost::regex::ECMAScript|boost::regex::icase ) ;
		}
	}
	catch  (boost::regex_error& e)
	{
		if ( find_parms.fcx_regreq )
		{
			MSG = "PEDT01N" ;
			return          ;
		}
	}
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if (  (*it)->il_deleted  || (*it)->il_excl || !(*it)->il_file ) { continue ; }
		if ( !found )
		{
			if ( find_parms.fcx_slab != "" && ( (*it)->il_label == find_parms.fcx_slab ) ) { found = true ; }
			if ( !found ) { continue ; }
		}
		if ( find_parms.fcx_regreq )
		{
			itss = (*it)->get_idata().begin() ;
			itse = (*it)->get_idata().end()   ;
			if ( find_parms.fcx_scol > 1 )
			{
				if ( find_parms.fcx_scol <= (*it)->get_idata().size() )
				{
					for ( i = 1 ; i < find_parms.fcx_scol ; i++ ) { itss++ ; }
				}
				else { continue ; }
			}
			if ( find_parms.fcx_ecol > 0 )
			{
				if ( find_parms.fcx_ecol <= (*it)->get_idata().size() )
				{
					itse = (*it)->get_idata().begin() ;
					for ( i = 1 ; i < find_parms.fcx_ecol ; i++ ) { itse++ ; }
				}
			}
			if ( regex_search( itss, itse, regexp ) ) { (*it)->il_excl = true ; j++ ; }
			if ( find_parms.fcx_elab != "" && ( (*it)->il_label == find_parms.fcx_elab ) ) { break ; }
			continue ;
		}
		if ( find_parms.fcx_asis )
		{
			p1 = (*it)->get_idata().find( find_parms.fcx_string, find_parms.fcx_scol-1 ) ;
		}
		else
		{
			p1 = upper( (*it)->get_idata()).find( find_parms.fcx_string, find_parms.fcx_scol-1 ) ;
		}
		if ( p1 != string::npos )
		{
			if ( find_parms.fcx_ecol == 0 || ( p1 < find_parms.fcx_ecol ) ) { (*it)->il_excl = true ; j++ ; }
		}
		if ( find_parms.fcx_elab != "" && ( (*it)->il_label == find_parms.fcx_elab ) ) { break ; }
	}
	MSG = "PEDT01L" ;
	ZLINES = d2ds( j ) ;
}


void PEDIT01::clearCursor()
{
	cursorPlaced    = false ;
	cursorPlaceHome = false ;
	cursorPlaceURID = 0     ;
	cursorPlaceRow  = 0     ;
}


void PEDIT01::placeCursor( int URID, int pt, int offset )
{
	// cursorPlaceUsing: 1 Use URID to place cursor(*)
	//                   2 Use Row to place cursor

	// cursorPlaceType   1 First char of the line command area
	//                   2 First char of data area
	//                   3 First non-blank char of data area (after startCol)
	//                   4 Use position in cursorPlaceOff

	// cursorPlaceOff    Offset from start of line

	cursorPlaced     = true   ;
	cursorPlaceHome  = false  ;
	cursorPlaceUsing = 1      ;
	cursorPlaceURID  = URID   ;
	cursorPlaceRow   = 0      ;
	cursorPlaceType  = pt     ;
	cursorPlaceOff   = offset ;
}


void PEDIT01::placeCursor( uint Row, int pt, int offset )
{
	// cursorPlaceUsing: 1 Use URID to place cursor
	//                   2 Use Row to place cursor(*)

	// cursorPlaceType   1 First char of the line command area
	//                   2 First char of data area
	//                   3 First non-blank char of data area (after startCol)
	//                   4 Use position in cursorPlaceOff

	// cursorPlaceOff    Offset from start of line


	cursorPlaced     = true   ;
	if ( Row > ZAREAD || s2data.at( Row-1 ).ipo_URID == 0 ) { cursorPlaceHome = true ; return ; }
	cursorPlaceHome  = false  ;
	cursorPlaceUsing = 2      ;
	cursorPlaceURID  = 0      ;
	cursorPlaceRow   = Row    ;
	cursorPlaceType  = pt     ;
	cursorPlaceOff   = offset ;
}


void PEDIT01::storeCursor( int URID, int pt, int offset )
{
	// cursorPlaceUsing: 1 Use URID to place cursor

	// cursorPlaceType   1 First char of the line command area
	//                   2 First char of data area
	//                   3 First non-blank char of data area (after startCol)
	//                   4 Use position in cursorPlaceOff

	// cursorPlaceOff    Offset from start of line

	cursorPlaceUsing = 1      ;
	cursorPlaceURID  = URID   ;
	cursorPlaceRow   = 0      ;
	cursorPlaceType  = pt     ;
	cursorPlaceOff   = offset ;
}



void PEDIT01::positionCursor()
{
	// Position cursor as determined by placeCursor() routine

	// cursorPlaceType  1 First char of the line command area
	//                  2 First char of data area
	//                  3 First non-blank char of data area (after startCol)
	//                  4 Use position in cursorPlaceOff

	// Hilight word on the cursor

	int i  ;
	int p  ;
	int dl ;
	int screenLine ;

	if ( !cursorPlaced ) { return ; }

	if ( cursorPlaceHome ) { CURFLD = "ZCMD" ; CURPOS = 1 ; return ; }

	screenLine = -1 ;

	switch ( cursorPlaceUsing )
	{
		case 1:
			for ( i = 0 ; i < ZAREAD ; i++ )
			{
				if ( s2data.at( i ).ipo_URID == cursorPlaceURID )
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
			dl = s2data.at( screenLine ).ipo_line ;
			p  = data.at( dl )->get_idata().find_first_not_of( ' ', startCol-1 ) ;
			p  = (p != string::npos) ? p + CLINESZ + 2 - startCol : CLINESZ + 1 ;
			CURPOS = ZAREAW * screenLine + p ;
			rebuildZAREA = true ;
			break ;
		case 4:
			CURFLD = "ZAREA" ;
			CURPOS = ZAREAW * screenLine + cursorPlaceOff ;
			break ;
	}
	ZSHADOW.replace( ZAREAW*screenLine+1, 6, 6, B_RED ) ;
	for ( i = CURPOS-1 ; i < ZASIZE ; i++ )
	{
		if ( ZAREA[ i ] == ' ' ) { break ; }
		ZSHADOW.replace( i, 1, 1, B_WHITE ) ;
	}
	for ( i = CURPOS-1 ; i > 0 ; i-- )
	{
		if ( ZAREA[ i ] == ' ' ) { break ; }
		ZSHADOW.replace( i, 1, 1, B_WHITE ) ;
	}
	rebuildShadow = true ;
}


bool PEDIT01::returnLabelItr(string label, vector<iline * >::iterator & it , int & posn )
{
	posn = 0 ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_label == label ) { return true ; }
		posn++ ;
	}
	return false ;
}


bool PEDIT01::xformLineCmd( string cmd, string & lc2, int & rept)
{
	// Split line command into the command (string) and repeat value (int), if allowed, and return in lc2 and retp
	// A 'rept' of -1 means it has not been entered

	int    j ;
	string t ;

	t.assign( 6, ' ' )   ;
	lc2.assign( 6, ' ' ) ;

	for ( j = 0 ; j < cmd.size() ; j++ )
	{
		if ( isdigit( cmd[ j ] ) ) { t[ j ]   = cmd[ j ] ; }
		else                       { lc2[ j ] = cmd[ j ] ; }
	}

	t   = strip( t )   ;
	lc2 = strip( lc2 ) ;

	if ( t != "" && wordpos( lc2, ReptOK ) == 0 )
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



void PEDIT01::removeRecoveryData()
{
	// Delete all logically deleted lines and remove entries from the data vector
	// Flatten all remaining data
	// Clear the global Undo/Redo stacks
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

	for ( it = tdata.begin() ; it != tdata.end() ; it++ )
	{
		delete (*it) ;
	}

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		(*it)->flatten_idata() ;
	}

	it = data.begin()            ;
	(*it)->clear_Global_Undo()   ;
	(*it)->clear_Global_Redo()   ;
	topLine = getLine( topURID ) ;
	MSG = "PEDT011H"             ;
}


vector<iline * >::iterator PEDIT01::getLineItr( int URID )
{
	vector<iline * >::iterator it ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_URID == URID ) { return it ; }
	}
}


vector<iline * >::iterator PEDIT01::getLineBeforeItr( int URID )
{
	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_URID == URID ) { return itt ; }
		itt = it ;
	}
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


int PEDIT01::getDataBlock( int URID )
{
	// Return the number of lines in an exluded block given any URID within that block
	// This does included logically deleted lines

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
	it++ ;
	for ( ; it != data.end() ; it++ )
	{
		if (  (*it)->il_deleted )  { continue         ; }
		if ( !(*it)->il_excl    )  { ite = it ; break ; }
	}

	bklines = 0 ;
	its++ ;
	for ( it = its ; it != ite ; it++ )
	{
		bklines++ ;
	}
	return bklines ;
}


int PEDIT01::getFirstEX( int URID )
{
	// Return the URID of the first excluded line in a block

	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if (  (*it)->il_deleted )     { continue ; }
		if ( !(*it)->il_excl    )     { itt = it ; }
		if ( (*it)->il_URID == URID ) { break    ; }
	}
	return (*itt)->il_URID ;
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


int PEDIT01::getLine( int URID )
{
	// Return the data line index l, for a given URID

	int i  ;

	for ( i = 0 ; i < data.size() ; i++ )
	{
		if ( data.at( i )->il_URID == URID ) { break ; }
	}
	return i ;

}


int PEDIT01::getFileLine( int d )
{
	// Return the file line index that corresponts to data line index d in the data vector

	int i  ;
	int fl ;

	fl = 0 ;
	for ( i = 0 ; i < d ; i++ )
	{
		if (  data.at( i )->il_deleted ) { continue ; }
		if ( !data.at( i )->il_file    ) { continue ; }
		fl++ ;
	}
	return fl ;

}


int PEDIT01::getDataLine( int f )
{
	// Return the data vector line that corresponts to line f in the file

	int i ;
	int j ;

	j = 0 ;

	for ( i = 0 ; i < data.size() ; i++ )
	{
		if (  data.at( i )->il_deleted ) { continue ; }
		if ( !data.at( i )->il_file    ) { continue ; }
		j++ ;
		if ( f == j ) { break ; }
	}

	if ( i == data.size() ) { i-- ; }
	return i ;

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


int PEDIT01::getNextDataLine( uint l )
{
	// Return the next non-deleted data vector line that corresponts to the line after l.

	for ( l++ ; l < data.size() ; l++ )
	{
		if ( !data.at( l )->il_deleted ) { break ; }
	}
	return l ;
}


vector<iline * >::iterator PEDIT01::getNextDataLine( vector<iline * >::iterator it )
{
	// Return the a valid (non-deleted) data line iterator after iterator it

	for ( it++ ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_deleted ) { break ; }
	}
	return it ;
}


int PEDIT01::getPrevDataLine( uint l )
{
	// Return the previous non-deleted data vector line that corresponts to the line before l.

	if ( l == 0 ) { return 0 ; }

	for ( l-- ; l > 0 ; l++ )
	{
		if ( !data.at( l )->il_deleted ) { break ; }
	}
	return l ;
}


vector<iline * >::iterator PEDIT01::getValidDataLine( vector<iline * >::iterator it )
{
	// Return the a valid (non-deleted) data line on or after iterator it

	for ( ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_deleted ) { break ; }
	}
	return it ;
}


uint PEDIT01::getValidDataLine( uint l )
{
	// Return a valid (non-deleted) data line on or after line l

	for ( ; l < data.size() ; l++ )
	{
		if ( !data.at( l )->il_deleted ) { break ; }
	}
	return l ;
}


int PEDIT01::getNextSpecial( int l )
{
	// Return the next special data vector line after l.

	int  ol    ;
	bool found ;

	found = false ;
	ol    = l     ;
	for ( l++ ; l < data.size() ; l++ )
	{
		if ( data.at( l )->il_deleted ) { continue ; }
		if ( data.at( l )->il_chg )     { found = true ; break ; }
		if ( data.at( l )->il_error )   { found = true ; break ; }
	}
	if ( !found ) { l = ol ; }
	return l ;
}


int PEDIT01::getRangeSize( int sURID, int eURID )
{
	// Return size of eURID1-sURID1 (ignoring logically deleted lines)

	int sz ;

	vector<iline * >::iterator it  ;

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
		if ( s2data.at( i ).ipo_URID == URID ) { return true ; }
	}
	return false ;
}


int PEDIT01::truncateSize( int sURID )
{
	// Return number of valid lines (ie to the Bottom Of Data line) after passed URID (including the line containing URID)

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
		if (  (*it)->il_deleted ) { continue ; }
		if ( !(*it)->il_file )    { continue ; }
		if (  (*it)->il_bod )     { break    ; }
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
	int l1 ;
	int l2 ;

	string s ;

	l1 = s1.size() ;
	l2 = s2.size() ;

	l = (l1 > l2) ? l1 : l2 ;
	s = string( l, ' ' ) ;
	s1.resize( l, ' ' )  ;
	s2.resize( l, ' ' )  ;

	for ( i = 0 ; i < l ; i++ )
	{
		if ( s2[ i ] != ' ' ) { s[ i ] = s2[ i ] ; if ( s1[ i ] != ' ' && s1[ i ] != s2[ i ] ) { success = false ; } }
		else                  { s[ i ] = s1[ i ] ; }
	}
	return s ;
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
	// Copy lines in vip (from copy/move) to lspf table CLIPTABL
	// cutReplace - clear clipboard before copy, else append at end of current contents

	int i   ;
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

	for ( i = 0 ; i < vip.size() ; i++ )
	{
		LINE = vip[ i ].ip_data ;
		tbadd( CLIPTABL ) ;
		if ( RC > 0 ) { tbclose( CLIPTABL) ; vdelete( "CLIPNAME LINE CRP" ) ; return ; }
	}

	tbclose( CLIPTABL ) ;
	MSG  = "PEDT012C"   ;
	ZCMD = ""           ;
	vdelete( "CLIPNAME LINE CRP" ) ;
	vreplace( "ZEDLNES", d2ds( vip.size() ) ) ;
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
	vreplace( "CLIPNAME", clipboard ) ;
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
	// Try writing out the array to a file so we can reload later

	string f ;

	log( "E", "Control given to EDIT cleanup procedure due to an abnormal event" << endl ) ;
//      ofstream fout( "/tmp/editorsession", ios::binary ) ;
//      fout.write((char*) &data, sizeof data ) ;
//      fout.close() ;

	f = ZFILE + ".abend" ;
	vector<iline * >::iterator it ;
	ofstream fout( f.c_str() ) ;

	if ( !fout.is_open() )
	{
		ZRESULT = "Open Error" ;
		RC = 16 ;
		return  ;
	}

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->il_file     ) { continue ; }
		if (  (*it)->il_deleted  ) { continue ; }
		fout << (*it)->get_idata() << endl ;
	}

	fout.close() ;
	log( "E", "File saved to " << f << endl ) ;
	ZRC  = 0 ;
	ZRSN = 8 ;
	EditList.erase( ZFILE ) ;
	return   ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PEDIT01 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
