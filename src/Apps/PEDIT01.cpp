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
/* Edit file (no save function)                                                              */
/*                                                                                           */
/*                                                                                           */
/* RC/RSN codes returned                                                                     */
/*  0/0   Okay - No saves made                                                               */
/*  0/4   Okay - Data saved                                                                  */
/*  8/4   Cannot use edit on file.  Browse instead                                           */
/*  8/8   Error saving data                                                                  */

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


using namespace boost ;
using namespace std ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PEDIT01

#define CLINESZ   8
#define Global_Undo iline::Global_Undo   
#define Global_Redo iline::Global_Redo
#define recoverOFF  iline::recoverOFF 


int  iline::maxURID    = 0     ;
bool iline::recoverOFF = false ;
stack<ichange> iline::Global_Undo ;
stack<ichange> iline::Global_Redo ;

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

	vdefine( "ZCMD ZVERB ZROW1 ZROW2 ZAREA ZSHADOW ZAREAT ZFILE", &ZCMD, &ZVERB, &ZROW1, &ZROW2, &ZAREA, &ZSHADOW, &ZAREAT, &ZFILE ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &ZAREAW, &ZAREAD ) ;
	vdefine( "ZSCROLLA ZCOL1 ZLINES", &ZSCROLLA, &ZCOL1, &ZLINES ) ;
	vdefine( "ZAPPLID ZEDSAVE ZEDNULLS ZEDLOCK ZEDCAPS ZEDHEX ZEDTABS ZEDTABSZ", &ZAPPLID, &ZEDSAVE, &ZEDNULLS, &ZEDLOCK, &ZEDCAPS, &ZEDHEX, &ZEDTABS, &ZEDTABSZ ) ;

	vget( "ZEDSAVE ZEDNULLS ZEDLOCK ZEDCAPS ZEDHEX ZEDTABS ZEDTABSZ", PROFILE ) ;
	vget( "ZAPPLID", SHARED ) ;

	pquery( "PEDIT012", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 ) { abend() ; }

	if ( ZEDSAVE   == "" || ZEDSAVE  == "ON"  ) { profSave  = true  ; }
	else                                        { profSave  = false ; }
	if ( ZEDNULLS  == "" || ZEDNULLS == "OFF" ) { profNulls = false ; }
	else                                        { profNulls = true  ; }
	if ( ZEDLOCK   == "" || ZEDLOCK  == "OFF" ) { profLock  = false ; }
	else                                        { profLock  = true  ; }
	if ( ZEDCAPS   == "" || ZEDCAPS  == "OFF" ) { profCaps  = false ; }
	else                                        { profCaps  = true  ; }
	if ( ZEDHEX    == "" || ZEDHEX   == "OFF" ) { profHex   = false ; }
	else                                        { profHex   = true  ; }
	if ( ZEDTABS   == "" || ZEDTABS  == "OFF" ) { profTabs  = false ; }
	else                                        { profTabs  = true  ; }
	if ( ZEDTABSZ  == "" ) { profTabSz  = 8                ; }
	else                   { profTabSz  = ds2d( ZEDTABSZ ) ; }

	ZDATAW = ZAREAW - CLINESZ ;
	ZASIZE = ZAREAW * ZAREAD  ;

	sdg.assign( ZDATAW, N_GREEN )   ;
	sdy.assign( ZDATAW, N_YELLOW )  ;
	sdw.assign( ZDATAW, N_WHITE )   ;
	sdr.assign( ZDATAW, N_RED )     ;
	slg.assign( CLINESZ, N_GREEN )  ;
	sly.assign( CLINESZ, N_YELLOW ) ;
	slw.assign( CLINESZ, N_WHITE )  ;
	slr.assign( CLINESZ, N_RED )    ;

	div.assign( ZAREAW-1, '-' )    ;

	clipboard = "DEFAULT"  ;
	CLIPTABL  = "EDITCLIP" ;
}


void PEDIT01::Edit()
{
	int t  ;

	RC = 0 ;

	read_file() ;
	if ( RC > 0 ) { setmsg( "PSYS01E" ) ; return ; }
	MSG = "" ;

	CURFLD = "ZCMD" ;
	CURPOS = 1      ;

        while ( true )
        {
		if ( rebuildZAREA ) fill_dynamic_area() ;
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
			else 		{ continue ; }
		}

		clearCursor() ;
		if ( abbrev( "CANCEL", upper( ZCMD ), 3 ) ) { break ; }

		if ( ZCMD[ 0 ] == '&' )   { OCMD = ZCMD ; ZCMD = ZCMD.substr( 1 ) ; }
		else                      { OCMD = ""   ;                           }

		if ( ZCURFLD == "ZAREA" ) { CURFLD = ZCURFLD ; CURPOS = ZCURPOS ; aRow = ((ZCURPOS-1) / ZAREAW + 1) ; aCol = ((ZCURPOS-1) % ZAREAW +1) ; }
		else                      { CURFLD = "ZCMD"  ; CURPOS = 1       ; aRow = 0                          ; aCol = 0                         ; }

		MSG = "" ;

		getZAREAchanges() ;
		updateData()      ;

		actionPCMD()      ;
		if ( MSG != "" )  { continue ; }

		actionLineCommands() ;

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;
		ZCMD = upper( ZCMD ) ;

		if ( ZVERB == "DOWN" )
		{
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
					if ( data[ topLine ]->il_excl || data[ topLine ]->il_deleted ) continue ;
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
					if ( data[ topLine ]->il_excl || data[ topLine ]->il_deleted ) { continue ; }
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
					startCol = startCol + ZSCROLLN ;
			}
		}
		if ( topLine < 0 ) topLine = 0 ;
        }
	vput( "ZSCROLL", PROFILE ) ;

	vector<iline * >::iterator it ; 
	for ( it = data.begin() ; it != data.end() ; it++ ) { delete (*it) ; }
	data.clear() ;
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
		if ( saveFile() )
		{
			setmsg( "PEDT01P" ) ;
		}
		else
		{
			return false ;
		}
	}
	if ( !profLock )
	{
		if ( profSave  ) { ZEDSAVE  = "ON"  ; }
		else             { ZEDSAVE  = "OFF" ; }
		if ( profNulls ) { ZEDNULLS = "ON"  ; }
		else             { ZEDNULLS = "OFF" ; }
		if ( profCaps  ) { ZEDCAPS  = "ON"  ; }
		else             { ZEDCAPS  = "OFF" ; }
		if ( profHex   ) { ZEDHEX   = "ON"  ; }
		else             { ZEDHEX   = "OFF" ; }
		if ( profTabs  ) { ZEDTABS  = "ON"  ; }
		else             { ZEDTABS  = "OFF" ; }
		ZEDTABSZ = d2ds( profTabSz ) ;
		vput( "ZEDSAVE ZEDNULLS ZEDCAPS ZEDHEX ZEDTABS ZEDTABSZ", PROFILE ) ;
	}
	if ( profLock ) { ZEDLOCK  = "ON"  ; }
	else            { ZEDLOCK  = "OFF" ; }
	vput( "ZEDLOCK", PROFILE ) ;
	return true ;
}


void PEDIT01::read_file()
{
	iline * p_iline ;

	idata t ;
	profTabs = false ;

	string inLine ;
	int pos, j ;
	ifstream fin( ZFILE.c_str() ) ;

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
	rebuildZAREA  = true  ;
	fileChanged   = false ;

	data.clear()       ;
	iline::maxURID = 0 ;

	p_iline          = new iline ;
	p_iline->il_tod  = true      ;
	p_iline->put_idata( centre( " TOP OF DATA ", ZAREAW, '*' ), 0 ) ;
	data.push_back( p_iline )    ;

	data[ 0 ]->clear_Global_Redo() ;
	data[ 0 ]->clear_Global_Undo() ;

	p_iline           = new iline ;
	p_iline->il_note  = true      ;
	p_iline->put_idata( " FILE READ.  TABS WILL BE CHANGED TO SPACES!!!", 0 ) ;
	data.push_back( p_iline )     ;

	p_iline          = new iline ;
	p_iline->il_note = true      ;
	p_iline->put_idata( " WARNING!!! EDITOR UNDER CONSTRUCTION.  DO NOT SAVE FILES CHANGED !!!!", 0 ) ;
	data.push_back( p_iline )    ;

	p_iline          = new iline ;
	p_iline->il_note = true      ;
	p_iline->put_idata( "", 0 )  ;
	data.push_back( p_iline )    ;

	while ( getline( fin, inLine ) )
	{
		p_iline = new iline ;
		pos = inLine.find( '\t' ) ;
		while ( pos != string::npos )
		{
			profTabs = true ;
			j = 8 - (pos % 8 ) ;
			inLine.replace( pos, 1,  j, ' ' )  ;
			pos = inLine.find( '\t', pos + 1 ) ;
		}
		if ( maxCol < inLine.size() ) maxCol = inLine.size() ;
		p_iline->il_file = true   ;
		p_iline->put_idata( inLine, 0 ) ;
		data.push_back( p_iline )    ;
	}
	maxCol++ ;
	p_iline = new iline ;
	p_iline->il_bod  = true ;
	p_iline->put_idata( centre( " BOTTOM OF DATA ", ZAREAW, '*' ), 0 ) ;
	data.push_back( p_iline )    ;
	p_iline->clear_Global_Undo() ;
	fin.close() ;
}



bool PEDIT01::saveFile()
{
	vector<iline * >::iterator it ;
	// ofstream fout( ZFILE.c_str() ) ;
	ofstream fout( "/tmp/pdfeditor" ) ;

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
		fout << (*it)->get_idata() << endl ;
		fileChanged = false ;
	}

	fout.close() ;
	if ( fileChanged ) { MSG = "PEDT01Q" ; ZRC = 8 ; ZRSN = 8 ; return false ; }
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

	fl = getFileLine( topLine ) ;

	for ( i = 0 ; i < ZAREAD ; i++ ) { s2data[ i ] = ip ; }

	for ( dl = topLine ; dl < data.size() ; dl++ )
	{
		if ( data[ dl ]->il_deleted ) { continue ; }
		if ( data[ dl ]->il_file )    { fl++     ; }
		if ( data[ dl ]->il_excl )
		{
			elines = getEXBlock( data[ dl ]->il_URID ) ;
			ip.ipo_line = dl ;
			ip.ipo_URID = data[ dl ]->il_URID ;
			s2data[ sl ] = ip ;
			ZAREA   = ZAREA + din + "------" + dout + centre( " " + d2ds( elines ) + " line(s) excluded ", ZDATAW, '-' ) ;
			ZSHADOW = ZSHADOW + slr + sdr ;
			if ( ZAREA.size() >= ZASIZE ) break ;
			sl++ ;
			dl = dl + elines - 1 ;
			fl = getFileLine( dl+1 ) ;
			continue ;
		}
		if ( data[ dl ]->il_lc1 == "" )
		{
			if ( data[ dl ]->il_label == "" )
			{
				if ( data[ dl ]->il_newisrt )     { lcc = "''''''" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->il_error )  { lcc = "==ERR>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->il_file  )  { lcc = right( d2ds( fl ), 6, '0' ) ; ZSHADOW = ZSHADOW + sly ; }
				else if ( data[ dl ]->il_note  )  { lcc = "=NOTE>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->il_col   )  { lcc = "=COLS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->il_bnds  )  { lcc = "=BNDS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->il_prof  )  { lcc = "=PROF>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->il_chg   )  { lcc = "==CHG>" ; ZSHADOW = ZSHADOW + slr ; }
				else                              { lcc = "******" ; ZSHADOW = ZSHADOW + slg ; }
			}
			else
			{
				lcc     = left( data[ dl ]->il_label, 6 ) ;
				ZSHADOW = ZSHADOW + slr                   ;
			}
		}
		else
		{
			lcc     = left( data[ dl ]->il_lc1, 6 ) ;
			ZSHADOW = ZSHADOW + slr                 ;
		}
		if ( data[ dl ]->il_file )
		{
			if ( data[ dl ]->il_hex || profHex )
			{
				t1 = substr( data[ dl ]->get_idata(), startCol, ZDATAW ) ;
				t2 = cs2xs( t1 ) ;
				ln = data[ dl ]->get_idata().size() - startCol + 1 ;
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
				ip.ipo_URID  = data[ dl ]->il_URID ;
				s2data[ sl ] = ip ;
				sl           = sl + 4 ;
			}
			else
			{
				ZAREA   = ZAREA + din + lcc + din + substr( data[ dl ]->get_idata(), startCol, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdg ;
				ip.ipo_line  = dl ;
				ip.ipo_URID  = data[ dl ]->il_URID ;
				s2data[ sl ] = ip ;
				sl++ ;
			}
		}
		else
		{
			if ( data[ dl ]->il_col )
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
			}
			else if ( data[ dl ]->il_bnds )
			{
				tmp = "<        >" ;
				ZAREA = ZAREA + din + lcc + dout + substr( tmp, 1, ZDATAW ) ;
			}
			else
			{
				ZAREA = ZAREA + din + lcc + dout + substr( data[ dl ]->get_idata(), 1, ZDATAW ) ;
			}
			ZSHADOW = ZSHADOW + sdw ;
			ip.ipo_line  = dl ;
			ip.ipo_URID  = data[ dl ]->il_URID ;
			s2data[ sl ] = ip ;
			sl++ ;
		}
		if ( ZAREA.size() >= ZAREAW * ZAREAD ) break ;
	}
	//	addHilight( data, fileType, topLine, startCol, ZAREAW, ZAREAD, ZSHADOW ) ;

	ZAREA.resize( ZASIZE, ' ' ) ;
	ZSHADOW.resize( ZASIZE, N_GREEN ) ;

	CAREA        = ZAREA ;
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
	int l   ;
	int dl  ;
	int off ;
	int sp  ;
	int ep  ;

	string lc ;

	const char duserMod(0x3) ;
	const char ddataMod(0x4) ;

	sTouched.clear() ;
	sChanged.clear() ;

	l = topLine ;
	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		dl            = s2data[ i ].ipo_line ;
		off           = i * ZAREAW  ;
		sTouched[ i ] = false       ;
		sChanged[ i ] = false       ;
		if ( ZAREA[ off ] == ddataMod )
		{
			lc = "      " ;
			ignore = true ;
			if ( data[ dl ]->il_lc1 == "" && data[ dl ]->il_label == "" )
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
				if ( data[ dl ]->il_lc1 == "" ) { data[ dl ]->il_label = "" ; }
				else                            { data[ dl ]->il_lc1   = "" ; }
			}
			else
			{
				data[ dl ]->il_lc1 = lc ;
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
			data[ dl ]->il_chg = true ;
			sChanged[ i ]      = true ;
			rebuildZAREA       = true ;
		}
		l++ ;
	}
}


void PEDIT01::updateData()
{
	int i  ;
	int j  ;
	int k  ;
	int dl ;

	string t ;

	iline * p_iline ;

	vector<iline * >::iterator it ;

	Level++ ;
	for ( i = (sChanged.size() - 1) ; i >=0 ; i-- )
	{
		dl = s2data[ i ].ipo_line ;
		if ( dl == -1 ) { continue ; }
		if ( data[ dl ]->il_newisrt )
		{
			if ( !sTouched[ i ] && !sChanged[ i ] )
			{
				for ( it = data.begin(), j = 0 ; j < dl ; j++ ) { it++ ; }
				delete *it            ;
				it = data.erase( it ) ;
				placeCursor( (*it)->il_URID, 3 ) ;
				rebuildZAREA = true   ;
				continue              ;
			}
			else
			{
				data[ dl ]->il_newisrt = false  ;
				if ( startCol > 1 )
				{
					t  = substr( data[ dl ]->get_idata(), 1, startCol-1 ) + strip( ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ), 'T', ' ' ) ;
					if ( profCaps ) { t = upper( t ) ; }
					data[ dl ]->put_idata( t, Level ) ;
				}
				else
				{
					t = strip( ZAREA.substr( CLINESZ+(i*ZAREAW), ZDATAW ), 'T', ' ' ) ;
					if ( profCaps ) { t = upper( t ) ; }
					data[ dl ]->put_idata( t, Level ) ;
				}
				it = getLineItr( data[ dl ]->il_URID ) ;
				k = (*it)->get_idata().find_first_not_of( ' ', startCol-1 ) + CLINESZ + 1 ;
				it++ ;
				p_iline = new iline        ;
				p_iline->il_file    = true ;
				p_iline->il_newisrt = true ;
				p_iline->put_idata( "", Level ) ;
				data.insert( it, p_iline ) ;
				placeCursor( p_iline->il_URID, 5, k ) ;
				rebuildZAREA  = true  ;
				fileChanged   = true  ;
				continue              ;
			}
		}
		if ( !sChanged[ i ] ) { continue ; }
		debug1( " processing screen line " << i << " for data line " << dl << endl ; )
		for ( it = data.begin(), j = 0 ; j <= dl ; j++ ) { it++ ; }
		t = data[ dl ]->get_idata() ;
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
		data[ dl ]->put_idata( t, Level ) ;
		rebuildZAREA  = true  ;
		fileChanged   = true  ;
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

	bool found  ;

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

	string::const_iterator itss ;
	string::const_iterator itse ;

	regex regexp ;

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
				if ( (*it)->il_bod ) { break  ; }
				if ( (*it)->il_tod ) { continue  ; }
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
		ZCMD = ""           ;
		rebuildZAREA = true ;
	}
	else if ( w1 == "FLIP" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		for ( it = data.begin() ; it != data.end() ; it++ )
		{
			if ( (*it)->il_bod || (*it)->il_tod || (*it)->il_deleted ) { continue ; }
			(*it)->il_excl = !(*it)->il_excl ;
		}
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
		}
		else if ( w2.size() < 7 && datatype( w2, 'W' ) )
		{
			if ( w2 == "0" ) { topLine = getDataLine( 1 ) ; }
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
			if ( profSave  ) { ZEDSAVE  = "ON"  ; }
			else             { ZEDSAVE  = "OFF" ; }
			if ( profNulls ) { ZEDNULLS = "ON"  ; }
			else             { ZEDNULLS = "OFF" ; }
			if ( profLock  ) { ZEDLOCK  = "ON"  ; }
			else             { ZEDLOCK  = "OFF" ; }
			if ( profCaps  ) { ZEDCAPS  = "ON"  ; }
			else             { ZEDCAPS  = "OFF" ; }
			if ( profHex   ) { ZEDHEX   = "ON"  ; }
			else             { ZEDHEX   = "OFF" ; }
			if ( profTabs  ) { ZEDTABS  = "ON"  ; }
			else             { ZEDTABS  = "OFF" ; }
			ZEDTABSZ = d2ds( profTabSz ) ;
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
			if ( recoverOFF ) { ZEDRECOV = "OFF" ; }
			else              { ZEDRECOV = "STG" ; }
				
			rebuildZAREA = true  ;

			dl = topLine ;
			for ( it = data.begin(), i = 0 ; i <= dl ; i++ ) { it++ ; }

			p_iline  = new iline ;
			p_iline->il_prof = true ;
			p_iline->put_idata( left( "....PROF(" + ZAPPLID + ")....UNDO(" + ZEDRECOV + ")....AUTOSAVE("+ZEDSAVE+")", ZDATAW, '.' ), Level ) ;
			it = data.insert( it, p_iline ) ;
			it++ ;

			p_iline  = new iline ;
			p_iline->il_prof = true ;
			p_iline->put_idata( left( "....NUM(OFF)....CAPS("+ZEDCAPS+")....HEX("+ZEDHEX+")", ZDATAW, '.' ), Level ) ;
			it = data.insert( it, p_iline ) ;
			it++ ;

			p_iline  = new iline ;
			p_iline->il_prof = true ;
			p_iline->put_idata( left( "....TABS("+ZEDTABS+")....SAVE(TABS)....TABSIZE("+ZEDTABSZ+")", ZDATAW, '.' ), Level ) ;
			it = data.insert( it, p_iline ) ;
			it++ ;
			
			p_iline  = new iline ;
			p_iline->il_prof = true ;
			p_iline->put_idata( left( "....NULLS("+ZEDNULLS+")....PROFILE LOCK("+ZEDLOCK+")", ZDATAW, '.' ), Level ) ;
			data.insert( it, p_iline ) ;
			rebuildZAREA = true  ;
		}
		else if ( ws != 2 )        { MSG = "PEDT011" ; return ; }
		else if ( w2 == "SAVE"   ) { profSave = true  ; }
		else if ( w2 == "NOSAVE" ) { profSave = false ; }
		else if ( w2 == "LOCK"   ) { profLock = true  ; }
		else if ( w2 == "UNLOCK" ) { profLock = false ; }
		else                       { MSG = "PEDT011" ; return ; }
		ZCMD = "" ;
	}
	else if ( w1 == "RECOV" || w1 == "RECOVER" )
	{
		if      ( ws > 2 )      { MSG = "PEDT011" ; return ; }
		else if ( w2 == "ON" )  { recoverOFF = false                       ; }
		else if ( w2 == "OFF" ) { recoverOFF = true ; removeRecoveryData() ; }
		else                    { MSG = "PEDT011" ; return ; }
		ZCMD = ""    ;
	}
	else if ( w1 == "REDO" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		actionREDO() ;
		ZCMD = ""    ;
	}
	else if ( w1 == "RC" || w1 == "RCHANGE" )
	{
		if ( !find_parms.fcx_cset ) { MSG = "PEDT019" ; return ; }
		ZCMD = ""           ;
		rebuildZAREA = true ;
	}
	else if ( w1 == "RF" || w1 == "RFIND" )
	{
		if ( !find_parms.fcx_fset ) { MSG = "PEDT019" ; return ; }
		ZCMD = ""           ;
		rebuildZAREA = true ;
	}
	else if ( w1 == "RES" || w1 == "RESET" )
	{
		if ( w2 == "" )
		{
			Level++ ;
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if ( (*it)->il_note || (*it)->il_col || (*it)->il_prof ) 
				{
					(*it)->put_idata( "", Level ) ;
					(*it)->set_il_deleted()       ;
					rebuildZAREA = true ;
				}
				(*it)->il_excl  = false ;
				(*it)->il_hex   = false ;
				(*it)->il_chg   = false ;
				(*it)->il_error = false ;
				(*it)->il_lc1   = ""    ;
			}
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
		else if ( ( w2 == "LABEL" && w3 == "" ) )
		{
			for ( it = data.begin() ; it != data.end() ; it++ ) { (*it)->il_label = "" ; }
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
				if (  data[ dl ]->il_deleted ) { continue ; }
				data[ dl ]->il_excl = true ;
			}
		}
		else
		{
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
			for ( dl = 1 ; dl < data.size()-1 ; dl++ )
			{
				if (  data[ dl ]->il_deleted ) { continue ; }
				if (  data[ dl ]->il_excl    ) { continue ; }
				if ( !data[ dl ]->il_file    ) { continue ; }
				if ( !found )
				{
					if ( find_parms.fcx_slab != "" && ( data[ dl ]->il_label == find_parms.fcx_slab ) ) { found = true ; }
					if ( !found ) { continue ; }
				}
				if ( find_parms.fcx_regreq )
				{
					itss = data[ dl ]->get_idata().begin() ;
					itse = data[ dl ]->get_idata().end()   ;
					if ( find_parms.fcx_scol > 1 )
					{
						if ( find_parms.fcx_scol <= data[ dl ]->get_idata().size() )
						{
							for ( i = 1 ; i < find_parms.fcx_scol ; i++ ) { itss++ ; }
						}
						else { continue ; }
					}
					if ( find_parms.fcx_ecol > 0 )
					{
						if ( find_parms.fcx_ecol <= data[ dl ]->get_idata().size() )
						{
							itse = data[ dl ]->get_idata().begin() ;
							for ( i = 1 ; i < find_parms.fcx_ecol ; i++ ) { itse++ ; }
						}
					}
					if ( regex_search( itss, itse, regexp ) ) { data[ dl ]->il_excl = true ; j++ ; }
					if ( find_parms.fcx_elab != "" && ( data[ dl ]->il_label == find_parms.fcx_elab ) ) { break ; }
					continue ;
				}
				if ( find_parms.fcx_asis )
				{
					p1 = data[ dl ]->get_idata().find( find_parms.fcx_string, find_parms.fcx_scol-1 ) ;
				}
				else
				{
					p1 = upper( data[ dl ]->get_idata()).find( find_parms.fcx_string, find_parms.fcx_scol-1 ) ;
				}
				if ( p1 != string::npos )
				{
					if ( find_parms.fcx_ecol == 0 || ( p1 < find_parms.fcx_ecol ) ) { data[ dl ]->il_excl = true ; j++ ; }
				}
				if ( find_parms.fcx_elab != "" && ( data[ dl ]->il_label == find_parms.fcx_elab ) ) { break ; }
			}
			MSG = "PEDT01L" ;
			ZLINES = d2ds( j ) ;
		}
		rebuildZAREA = true ;
		ZCMD = ""           ;
	}
	else { MSG = "PEDT011" ; }
}


void PEDIT01::actionLineCommands()
{
	// For each line in the data vector, action the line commands
	// If not using put_idata to update the record, call put_ichange() to update the global Undo stack/

	// For copy/move/repeat preserve flags: file, note, prof, col, excl and hex

	int j    ;
	int k    ;
	int size ;

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

	if ( !checkLineCommands() ) { return ; }

	for ( itc = icmds.begin() ; itc != icmds.end() ; itc++ )
	{
		if ( itc->icmd_Rpt == -1 ) { itc->icmd_Rpt = 1 ; }
		if ( itc->icmd_COMMAND == "BNDS" )
		{
			Level++;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12()       ;
			p_iline          = new iline ;
			p_iline->il_bnds = true      ;
			p_iline->put_idata( "", Level )  ;
			il_itr++ ;
			il_itr = data.insert( il_itr, p_iline ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "COL" || itc->icmd_COMMAND == "COLS" )
		{
			Level++;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12()   ;
			p_iline          = new iline ;
			p_iline->il_col  = true      ;
			p_iline->put_idata( "", Level )  ;
			il_itr++ ;
			il_itr = data.insert( il_itr, p_iline ) ;
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "C" || itc->icmd_COMMAND == "M" )
		{
			Level++ ;
			vip.clear()   ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				ip.ip_file = (*il_itr)->il_file ;
				ip.ip_note = (*il_itr)->il_note ;
				ip.ip_prof = (*il_itr)->il_prof ;
				ip.ip_col  = (*il_itr)->il_col  ;
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
				(*il_ite)->clearLc12() ;
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
						p_iline  = new iline ;
						p_iline->il_file = vip[ j ].ip_file ;
						p_iline->il_note = vip[ j ].ip_note ;
						p_iline->il_prof = vip[ j ].ip_prof ;
						p_iline->il_col  = vip[ j ].ip_col  ;
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
		//	debug1( " dje itc->icmd_COMMAND " << itc->icmd_COMMAND << endl);
		//	debug1( " dje itc->icmd_ABO " << itc->icmd_ABO << endl);
		//	debug1( " dje itc->icmd_Rpt " << itc->icmd_Rpt << endl);
		//	debug1( " dje itc->icmd_OSize " << itc->icmd_OSize << endl);
		//	debug1( " dje itc->icmd_overlay " << itc->icmd_overlay << endl);
		//	debug1( " dje itc->icmd_cutpaste " << itc->icmd_cutpaste << endl);
		//	debug1( " dje itc->icmd_sURID " << itc->icmd_sURID << endl);
		//	debug1( " dje itc->icmd_eURID " << itc->icmd_eURID << endl);
		//	debug1( " dje itc->icmd_dURID " << itc->icmd_dURID << endl);
		//	debug1( " dje itc->icmd_oURID " << itc->icmd_oURID << endl);
			Level++ ;
			vip.clear()   ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			(*il_itr)->clearLc12() ;
			(*il_ite)->clearLc12() ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_bod ) { break ; }
				ip.ip_file = (*il_itr)->il_file ;
				ip.ip_note = (*il_itr)->il_note ;
				ip.ip_prof = (*il_itr)->il_prof ;
				ip.ip_col  = (*il_itr)->il_col  ;
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
				(*il_ite)->clearLc12() ;
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
						(*ita)->clearLc12() ;
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
						p_iline  = new iline ;
						p_iline->il_file = vip[ j ].ip_file ;
						p_iline->il_note = vip[ j ].ip_note ;
						p_iline->il_prof = vip[ j ].ip_prof ;
						p_iline->il_col  = vip[ j ].ip_col  ;
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
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->put_idata( "", Level ) ;
				(*il_itr)->set_il_deleted()       ;
				il_itr++ ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "DD" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			(*il_itr)->clearLc12() ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->put_idata( "", Level ) ;
				(*il_itr)->set_il_deleted()       ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "F" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if (  (*il_itr)->il_bod )    { break                      ; }
				if ( !(*il_itr)->il_excl )   { break                      ; }
				if (  (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->il_excl = false ;
				il_itr++ ;
			}
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "HX" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->il_hex = true ;
				il_itr++ ;
			}
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "HXX" || itc->icmd_COMMAND == "HXHX" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			(*il_itr)->clearLc12() ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->il_hex = true ;
			}
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "I" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			k = (*il_itr)->get_idata().find_first_not_of( ' ', startCol-1 ) + CLINESZ + 1 ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				p_iline = new iline        ;
				p_iline->il_file    = true ;
				p_iline->il_newisrt = true ;
				p_iline->put_idata( "", Level ) ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
				placeCursor( (*il_itr)->il_URID, 5, k ) ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "L" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->il_excl = false ;
				il_itr++ ;
			}
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "LC" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->put_idata( lower( (*il_itr)->get_idata() ), Level ) ;
				il_itr++ ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "LCC" || itc->icmd_COMMAND == "LCLC" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			(*il_itr)->clearLc12() ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->put_idata( lower( (*il_itr)->get_idata() ), Level ) ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "MD" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )     { break                     ; }
				if ( (*il_itr)->il_file )    { continue                  ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				p_iline = new iline     ;
				p_iline->il_file = true ;
				p_iline->put_idata( (*il_itr)->get_idata(), Level ) ;
				(*il_itr)->put_idata( "", Level ) ;
				(*il_itr)->set_il_deleted() ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
				il_itr++ ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "MMD" || itc->icmd_COMMAND == "MDMD" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			(*il_itr)->clearLc12() ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_file )    { continue ; }
				if ( (*il_itr)->il_deleted ) { continue ; }
				p_iline = new iline        ;
				p_iline->il_file    = true ;
				p_iline->put_idata( (*il_itr)->get_idata(), Level ) ;
				(*il_itr)->put_idata( "", Level ) ;
				(*il_itr)->set_il_deleted()       ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
				il_ite = getLineItr( itc->icmd_eURID )  ;
				il_ite++ ;
			}
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
			ip.ip_excl = (*il_itr)->il_excl ;
			ip.ip_hex  = (*il_itr)->il_hex  ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				p_iline          = new iline  ;
				p_iline->il_file = ip.ip_file ;
				p_iline->il_note = ip.ip_note ;
				p_iline->il_prof = ip.ip_prof ;
				p_iline->il_col  = ip.ip_col  ;
				p_iline->il_bnds = ip.ip_bnds ;
				p_iline->il_excl = ip.ip_excl ;
				p_iline->il_hex  = ip.ip_hex  ;
				p_iline->put_idata( tmp, Level ) ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "RR" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			(*il_itr)->clearLc12() ;
			for ( ; ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				ip.ip_file = (*il_itr)->il_file ;
				ip.ip_note = (*il_itr)->il_note ;
				ip.ip_prof = (*il_itr)->il_prof ;
				ip.ip_col  = (*il_itr)->il_col  ;
				ip.ip_bnds = (*il_itr)->il_bnds ;
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
					p_iline          = new iline        ;
					p_iline->il_file = vip[ k ].ip_file ;
					p_iline->il_note = vip[ k ].ip_note ;
					p_iline->il_prof = vip[ k ].ip_prof ;
					p_iline->il_col  = vip[ k ].ip_col  ;
					p_iline->il_bnds = vip[ k ].ip_bnds ;
					p_iline->il_excl = vip[ k ].ip_excl ;
					p_iline->il_hex  = vip[ k ].ip_hex  ;
					p_iline->put_idata( vip[ k ].ip_data, Level ) ;
					il_ite++ ;
					il_ite = data.insert( il_ite, p_iline ) ;
				}
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "UC" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->put_idata( upper( (*il_itr)->get_idata() ), Level ) ;
				il_itr++ ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "UCC" || itc->icmd_COMMAND == "UCUC" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			(*il_itr)->clearLc12() ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
					if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->put_idata( upper( (*il_itr)->get_idata() ), Level ) ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "X" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			for ( j = 0 ; j < itc->icmd_Rpt ; j++ )
			{
				if ( (*il_itr)->il_bod )    { break                     ; }
				if ( (*il_itr)->il_deleted ) { j-- ; il_itr++ ; continue ; }
				(*il_itr)->il_excl = true ;
				il_itr++ ;
			}
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == "XX" )
		{
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			(*il_itr)->clearLc12() ;
			(*il_ite)->clearLc12() ;
			il_ite++ ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted  ) { continue ; }
				(*il_itr)->il_excl = true ;
			}
			rebuildZAREA = true ;
		}
		else if ( itc->icmd_COMMAND == ")" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			(*il_itr)->put_idata( string( itc->icmd_Rpt, ' ' ) + (*il_itr)->get_idata(), Level ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "))" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			(*il_itr)->clearLc12() ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted  ) { continue ; }
				(*il_itr)->put_idata( string( itc->icmd_Rpt, ' ' ) + (*il_itr)->get_idata(), Level ) ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "(" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			(*il_itr)->clearLc12() ;
			(*il_itr)->put_idata( substr( (*il_itr)->get_idata(), itc->icmd_Rpt+1 ), Level ) ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_COMMAND == "((" )
		{
			Level++ ;
			il_itr = getLineItr( itc->icmd_sURID ) ;
			il_ite = getLineItr( itc->icmd_eURID ) ;
			il_ite++ ;
			(*il_itr)->clearLc12() ;
			for ( ; il_itr != il_ite ; il_itr++ )
			{
				if ( (*il_itr)->il_deleted ) { continue ; }
				(*il_itr)->put_idata( substr( (*il_itr)->get_idata(), itc->icmd_Rpt+1 ), Level ) ;
			}
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
		else if ( itc->icmd_cutpaste )
		{
			Level++;
			getClipboard( vip ) ;
			il_itr  = getLineItr( itc->icmd_dURID ) ;
			(*il_itr)->clearLc12() ;
			if ( itc->icmd_ABO == 'B' ) { il_itr = getLineBeforeItr( itc->icmd_dURID ) ; }
			for ( j = 0 ; j < vip.size() ; j++ )
			{
				p_iline          = new iline  ;
				p_iline->il_file = true       ;
				p_iline->put_idata( vip[ j ].ip_data, Level ) ;
				il_itr++ ;
				il_itr = data.insert( il_itr, p_iline ) ;
			}
			vreplace( "ZCPLINES", d2ds( vip.size() ) ) ;
			MSG  = "PEDT012D"   ;
			ZCMD = ""           ;
			rebuildZAREA = true ;
			fileChanged  = true ;
		}
	}
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
		if ( xformLineCmd( (*it)->il_lc1, lc2, rept ) )
		{
			if ( wordpos( lc2, blkcmds + " " + sglcmds )   == 0 ) { MSG = "PEDT012" ; return false ; }
			if ( (*it)->il_tod && wordpos( lc2, todlcmds ) == 0 ) { MSG = "PEDT013" ; return false ; }
			if ( (*it)->il_bod && wordpos( lc2, bodlcmds ) == 0 ) { MSG = "PEDT013" ; return false ; }
			if ( lc2 == "OO" )
			{
				if ( !iu_oo )
				{
					oo.icmd_sURID = (*it)->il_URID ;
					iu_oo = true ;
				}
				else
				{
					if ( oo.icmd_eURID > 0 ) { MSG = "PEDT01Y" ; return false ; }
					oo.icmd_eURID = (*it)->il_URID ;
					oo.icmd_OSize = getRangeSize( oo.icmd_sURID, oo.icmd_eURID ) ;
					if ( iu_cmd1 )
					{
						if ( cmd1.icmd_eURID == 0 ) { MSG = "PEDT01W" ; return false ; }
						if ( !cmd1.icmd_ABO_req )   { MSG = "PEDT01Y" ; return false ; }
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
						if ( !cmd2.icmd_ABO_req )  { MSG = "PEDT01Y" ; return false ; }
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
				if ( iu_cmd1 && cmd1.icmd_COMMAND != lc2 ) { MSG = "PEDT01Y" ; return false ; }
				if ( iu_cmd1 && cmd1.icmd_eURID > 0 )      { MSG = "PEDT01W" ; return false ; }
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
					if ( (cmd1.icmd_Rpt != -1 && rept != -1) && cmd1.icmd_Rpt != rept ) { MSG = "PEDT011B" ; return false ; }
					if (  cmd1.icmd_Rpt == -1 ) { cmd1.icmd_Rpt = rept ; }
					cmd1.icmd_eURID = (*it)->il_URID ;
					if ( wordpos( lc2, ABOReq ) == 0 )
					{ 
						icmds.push_back( cmd1 ) ;
						iu_cmd1  = false        ;
						(*it)->clearLc12()      ;
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
							if ( oo.icmd_eURID == 0 ) { MSG = "PEDT01W" ; return false ; }
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
						else if ( cutActive )
						{
							cmd1.icmd_cutpaste = true ;
							icmds.push_back( cmd1 )   ;
							cmd1.icmd_clear()         ;
							iu_cmd1  = false          ;
						}
					}
				}
			}
			else
			{
				if ( wordpos( lc2, ABOList ) > 0 )
				{
					if ( iu_abo ) { MSG = "PEDT01V" ; return false ; }
					else if ( pasteActive )
					{
						abo.icmd_cutpaste = true  ;
						abo.icmd_dURID    = (*it)->il_URID  ;
						abo.icmd_ABO      = lc2[ 0 ] ;
						icmds.push_back( abo ) ;
						abo.icmd_clear()       ;
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
							if ( iu_cmd1 ) { MSG = "PEDT01W" ; return false ; }
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
							else if ( cutActive || pasteActive )
							{
								cmd1.icmd_cutpaste = true ;
								icmds.push_back( cmd1 )   ;
								cmd1.icmd_clear()         ;
								iu_cmd1  = false          ;
							}
						}
						else
						{
							if ( iu_cmd2 ) { MSG = "PEDT01Y" ; return false ; }
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
								if ( oo.icmd_eURID == 0 ) { MSG = "PEDT01W" ; return false ; }
								cmd2.icmd_ABO   = oo.icmd_ABO   ;
								cmd2.icmd_dURID = oo.icmd_sURID ;
								cmd2.icmd_OSize = oo.icmd_OSize ;
								cmd2.icmd_oURID = oo.icmd_eURID ;
								icmds.push_back( cmd2 )  ;
								oo.icmd_clear()          ;
								iu_cmd2  = false         ;
								iu_oo    = false         ;
							}
							else if ( cutActive )
							{
								cmd2.icmd_cutpaste = true ;
								icmds.push_back( cmd2 )   ;
								cmd2.icmd_clear()         ;
								iu_cmd2  = false          ;
							}
						}
					}
				}
			}
		}
		else
		{
			if ( MSG == "" ) { MSG = "PEDT01Y" ; }
			return false ;
		}
	}
	if ( iu_cmd1 ) { MSG = "PEDT01W" ; return false ; }
	if ( iu_cmd2 ) { MSG = "PEDT01Z" ; return false ; }
	if ( iu_abo  ) { MSG = "PEDT01Z" ; return false ; }
	if ( iu_oo   ) { MSG = "PEDT01Z" ; return false ; }
	return true ;
}


void PEDIT01::actionUNDO()
{
	// Restore labels as these are removed from deleted records (as they can change in the mean time)

	// For a delete, restore the il_lvl from the idata stack (first record)

	// Get the URID/lvl for the top change and undo.  If the change before has the save lvl, repeat
	// until we get a URID/lvl for a different lvl or no more ichange records (URID==0)

	// Can take a while for large undo's.  Disable/enable timeout
	
	int   lvl ;
	ichange t ;

	control( "TIMEOUT", "DISABLE" ) ;

	vector<iline * >::iterator it  ;

	if ( recoverOFF ) { MSG = "PEDT01U" ; return ; }

	it = data.begin() ;
	t  = (*it)->get_Undo_URID() ;
	if ( t.iURID == 0 )
	{
		log( "A", "No more undo's available" << endl ; )
		MSG = "PEDT017" ;
		return ;
	}
	lvl = t.ilvl ;

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

	control( "TIMEOUT", "DISABLE" ) ;
	vector<iline * >::iterator it  ;
	
	if ( recoverOFF ) { MSG = "PEDT01U" ; return ; }

	log( "A", " Re-doing level " << Level << endl ; )

	Level++ ;	
	it = data.begin() ;
	t  = (*it)->get_Redo_URID() ;
	if ( t.iURID == 0 )
	{
		log( "A", "No redo's available" << endl ; )
		MSG = "PEDT018" ;
		return ;
	}
	lvl = t.ilvl ;

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

	vector<iline * >::iterator it  ;

	static char quote('\"')  ;
	static char apost('\'')  ;

	e_find t    ;

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
				else                               { w1 = find_parms.fcx_string  ; }
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
				t.fcx_string = "\\b" + t.fcx_string ;
				break ;
			case 'S':
				t.fcx_string = t.fcx_string + "\\b" ;
				break ;
			case 'W':
				t.fcx_string = "\\b" + t.fcx_string + "\\b" ;
				break ;
		}
	}

	t.fcx_fset = true ;
	if ( fcx_type == 'C') { t.fcx_cset = true ; }
	find_parms = t ;
	return 0 ;
}


void PEDIT01::clearCursor()
{
	cursorPlaced    = false ;
}


void PEDIT01::placeCursor( int URID, int pl, int offset )
{
	// cursorPlaceType: 1 Use URID to place cursor

	// cursorPlacePos   1 Start of line command area
	//                  2 End of line command field
	//                  3 First char of data area
	//                  4 First non-blank char of data area
	//                  5 Use position in cursorPlaceChar

	// cursorPlaceChar  Offset from start of line

	cursorPlaced    = true   ;
	cursorPlaceType = 1      ;
	cursorPlaceURID = URID   ;
	cursorPlacePos  = pl     ;
	cursorPlaceChar = offset ;
	//CURFLD = ;
	//CURPOS = ;
}


void PEDIT01::positionCursor()
{
	// Position cursor as determined by placeCursor() routine

	int i ;
	int screenLine ;

	if ( !cursorPlaced ) { return ; }

	screenLine = -1
;
	if ( cursorPlaceType == 1 )
	{
		for ( i = 0 ; i < ZAREAD ; i++ )
		{
			if ( s2data[ i ].ipo_URID == cursorPlaceURID )
			{
				debug1( " dje position URID found"<<endl);
				{	
					screenLine = i ;
					break ;
				}
			}
		}
	}

	if ( !screenLine == -1 ) { return ; }

	switch ( cursorPlacePos )
	{
		case 1:
			debug1( " dje positioning cursor1"<<endl);
			CURFLD = "ZAREA" ;
			CURPOS = ZAREAW * screenLine + 1 ;
			break ;
		case 3:
			debug1( " dje positioning cursor3"<<endl);
			CURFLD = "ZAREA" ;
			CURPOS = ZAREAW * screenLine + CLINESZ + 1 ;
			break ;
		case 5:
			debug1( " dje positioning cursor5"<<endl);
			CURFLD = "ZAREA" ;
			CURPOS = ZAREAW * screenLine + cursorPlaceChar ;
			break ;		
	}
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
	// Delete all logically deleted lines and remove entry from the data vector
	// Flatten all remaining data
	// Clear the globla Undo/Redo stacks
	// (Make a copy of the data vector using copy_if that contains only records to be deleted as iterator is invalidated after an erase() )

	vector<iline * >::iterator it      ;
	vector<iline * >::iterator new_end ;
	vector<iline * >tdata              ;

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

	it = data.begin()          ;
	(*it)->clear_Global_Undo() ;
	(*it)->clear_Global_Redo() ;
	MSG = "PEDT011H"           ;
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
		if ( !(*it)->il_deleted && !(*it)->il_excl && !(*it)->il_deleted ) { break ; }
		itt = it ;
	}
	return (*itt)->il_URID ;

}


int PEDIT01::getFileLine( int l )
{
	// Return the file line that corresponts to line l in the data vector

	int i  ;
	int fl ;

	fl = 0 ;
	for ( i = 0 ; i < l ; i++ )
	{
		if (  data[ i ]->il_deleted ) { continue ; }
		if ( !data[ i ]->il_file    ) { continue ; }
		fl++ ;
	}
	return fl ;

}


int PEDIT01::getDataLine( int l )
{
	// Return the data vector line that corresponts to line l in the file

	int i ;
	int j ;

	j = 0 ;

	for ( i = 0 ; i < data.size() ; i++ )
	{
		if (  data[ i ]->il_deleted ) { continue ; }
		if ( !data[ i ]->il_file    ) { continue ; }
		j++ ;
		if ( l == j ) { break ; }
	}

	if ( i == data.size() ) { i-- ; }
	return i ;

}


int PEDIT01::getRangeSize( int sURID, int eURID )
{
	// Return size of eURID1-sURID1 (ignoring logically deleted lines)

	int sz ;

	vector<iline * >::iterator it  ;

	sz = 0 ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->il_deleted ) { continue ; }
		if ( (*it)->il_URID == sURID ) { break ; }
	}
	for ( ; it != data.end() ; it++ )
	{
		if ( (*it)->il_deleted ) { continue ; }
		sz++ ;
		if ( (*it)->il_URID == eURID ) { break ; }
	}
	return sz ;

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


void PEDIT01::copyToClipboard( vector<ipline> & vip )
{
	// Copy lines in vip (from copy/move) to lspf table CLIPTABL
	// cutReplace

	int i   ;
	int CRP ;
	int pos ;

	string UPROF    ;
	string CLIPNAME ;
	string LINE     ;

	vdefine( "CLIPNAME LINE", &CLIPNAME, &LINE ) ;
	vdefine( "CRP", &CRP ) ;
	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( CLIPTABL, WRITE, UPROF, EXCLUSIVE ) ;
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
	vreplace( "ZCPLINES", d2ds( vip.size() ) ) ;
	MSG  = "PEDT012C" ;
	ZCMD = ""         ;
	vdelete( "CLIPNAME LINE CRP" ) ;
}


void PEDIT01::getClipboard( vector<ipline> & vip )
{
	// Get lines from clipboard and put them in vip
	// pasteKeep

	string UPROF    ;
	string CLIPNAME ;
	string LINE     ;
	ipline t        ;

	vip.clear() ;

	vdefine( "CLIPNAME LINE", &CLIPNAME, &LINE ) ;
	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( CLIPTABL, WRITE, UPROF, EXCLUSIVE ) ;
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
	// Try writing out the array to a file so we can reload later

	log( "E", "Customised cleanup procedure" << endl ) ;
	ofstream fout( "/tmp/editorsession", ios::binary ) ;
	fout.write((char*) &data, sizeof data ) ;
	fout.close() ;

}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PEDIT01 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }


