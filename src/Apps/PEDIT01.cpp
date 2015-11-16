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


void PEDIT01::application()
{
	log( "I", "Application PEDIT01 starting.  Parms are " << PARM << endl ) ;

	int p1 ;
	int p2 ;
	int t  ;

	string panel("") ;

	pcleanup = static_cast<void (pApplication::*)()>(&PEDIT01::cleanup_custom) ;

	p1 = pos( "FILE(", PARM ) ;
	if ( p1 == 0 )
	{
		log( "E", "Invalid parameter format passed to PEDIT01" << endl ; )
		abend() ;
		return  ;
	}
	p2 = pos( ")", PARM, p1 ) ;
	file = substr( PARM, p1 + 5, p2 - p1 - 5 ) ;

	p1 = pos( "PANEL(", PARM ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", PARM, p1 )  ;
		panel = substr( PARM, p1 + 7, p2 - p1 - 6 ) ;
	}

	ZFILE = file ;
	log( "I", "Displaying file " << file << " using panel " << panel << endl ) ;

	initialise() ;

	RC = 0 ;
	read_file() ;
	if ( RC > 0 ) { setmsg( "PSYS01E" ) ; cleanup() ; return ; }
	MSG = "" ;

	CURFLD = "ZCMD" ;
	CURPOS = 1      ;

        while ( true )
        {
		if ( rebuildZAREA ) fill_dynamic_area() ;

		ZROW1 = right( d2ds( firstLine ), 8, '0' )    ;
		ZROW2 = right( d2ds( maxLines - 2 ), 8, '0' ) ;
		ZCOL1 = right( d2ds( startCol ), 7, '0' )     ;

		if ( MSG == "" )
		{
			if ( OCMD[ 0 ] == '&' ) { ZCMD = OCMD ; }
			else                    { ZCMD = ""   ; }
		}

		display( "PEDIT011", MSG, CURFLD, CURPOS ) ;

		if ( RC  > 8 )                   { abend()   ;         }
		if ( RC == 8 ) if ( termOK() )   { cleanup() ; break ; }
		               else              { continue  ;         }

		if ( abbrev( "CANCEL", upper( ZCMD ), 3 ) ) { cleanup() ; break ; }

		if ( ZCMD[ 0 ] == '&' )   { OCMD = ZCMD ; ZCMD = ZCMD.substr( 1 ) ; }
		else                      { OCMD = ""   ;                           }

		if ( ZCURFLD == "ZAREA" ) { CURFLD = ZCURFLD ; CURPOS = ZCURPOS ; aRow = ((ZCURPOS-1) / ZAREAW + 1) ; aCol = ((ZCURPOS-1) % ZAREAW +1) ; }
		else                      { CURFLD = "ZCMD"  ; CURPOS = 1       ; aRow = 0                          ; aCol = 0                         ; }

		MSG = "" ;

		log( "A", "Cursor at " << aRow << " " << aCol << endl ; )
		log( "A", "Getting ZAREA changes" << endl ; )
		getZAREAchanges() ;

		log( "A", "Doing updates" << endl ; )
		updateData()      ;

		log( "A", "Actioning primary command" << endl ; )
		actionPCMD()      ;
		if ( MSG != "" ) { continue ; }

		log( "A", "Actioning single line commands" << endl ; )
		actionSLCMDS()     ;
		if ( MSG != "" ) { continue ; }

		log( "A", "Actioning block line commands" << endl ; )
		actionBLCMDS()     ;
		if ( MSG != "" ) { continue ; }

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;
		ZCMD = upper( ZCMD ) ;

		if ( ZVERB == "DOWN" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				firstLine = maxLines - ZAREAD ;
			}
			else
			{
				t = 0 ;
				for ( ; firstLine < ( maxLines - 1 ) ; firstLine++ )
				{
					if ( data[ firstLine ]->iline_excl || data[ firstLine ]->iline_hidden || data[ firstLine ]->iline_deleted ) continue ;
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
				firstLine = 0 ;
			}
			else
			{
				firstLine = firstLine - ZSCROLLN  ;
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
		if ( firstLine < 0 ) firstLine = 0 ;
        }
	vput( "ZSCROLL", PROFILE ) ;
        return ;
}



void PEDIT01::initialise()
{
	vdefine( "ZCMD ZVERB ZROW1 ZROW2 ZAREA ZSHADOW ZAREAT ZFILE", &ZCMD, &ZVERB, &ZROW1, &ZROW2, &ZAREA, &ZSHADOW, &ZAREAT, &ZFILE ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &ZAREAW, &ZAREAD ) ;
	vdefine( "ZSCROLLA ZCOL1 ZLINES", &ZSCROLLA, &ZCOL1, &ZLINES ) ;
	vdefine( "ZAPPLID ZPSAVE ZPNULLS ZPLOCK ZPCAPS ZPHEX", &ZAPPLID, &ZPSAVE, &ZPNULLS, &ZPLOCK, &ZPCAPS, &ZPHEX ) ;

	vget( "ZPSAVE ZPNULLS ZPLOCK ZPCAPS ZPHEX", PROFILE ) ;
	vget( "ZAPPLID", SHARED ) ;


	if ( ZPSAVE  == "" || ZPSAVE  == "ON"  ) { profSave  = true  ; }
	else                                     { profSave  = false ; }
	if ( ZPNULLS == "" || ZPNULLS == "OFF" ) { profNulls = false ; }
	else                                     { profNulls = true  ; }
	if ( ZPLOCK  == "" || ZPLOCK  == "OFF" ) { profLock  = false ; }
	else                                     { profLock  = true  ; }
	if ( ZPCAPS  == "" || ZPCAPS  == "OFF" ) { profCaps  = false ; }
	else                                     { profCaps  = true  ; }
	if ( ZPHEX   == "" || ZPHEX   == "OFF" ) { profHex   = false ; }
	else                                     { profHex   = true  ; }

	firstLine     = 0 ;
	startCol      = 1 ;
	maxCol        = 1 ;
	undoLevel     = 0 ;
	rebuildZAREA  = true  ;
	fileChanged   = false ;
	REDOavailable = false ;

	pquery( "PEDIT011", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 ) abend() ;

	ZDATAW = ZAREAW - 8 ;
	ZASIZE = ZAREAW * ZAREAD ;

	sdg.assign( ZDATAW, N_GREEN )  ;
	sdy.assign( ZDATAW, N_YELLOW ) ;
	sdw.assign( ZDATAW, N_WHITE )  ;
	sdr.assign( ZDATAW, N_RED )    ;
	slg.assign( 8, N_GREEN )       ;
	sly.assign( 8, N_YELLOW )      ;
	slw.assign( 8, N_WHITE )       ;
	slr.assign( 8, N_RED )         ;

	div.assign( ZAREAW-1, '-' )    ;

	blkcmds  = "CC MM DD HXX HHX RR XX (( )) UCC LCC MDD MMD" ;
	sglcmds  = "C M D R X I ( ) UC LC COL COLS HX MD S A B O F L";
	spllcmds = "COL COLS A B I C M D R CC MM DD RR";
	todlcmds = "COL COLS A I";
	bodlcmds = "B";
	ABOReq   = "CC MM C M";
	ABOList  = "A B O";
	MultOK   = "C M D HX X R UC LC RR (( )) ( ) F L";
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
		if ( profSave  ) { ZPSAVE  = "ON"  ; }
		else             { ZPSAVE  = "OFF" ; }
		if ( profNulls ) { ZPNULLS = "ON"  ; }
		else             { ZPNULLS = "OFF" ; }
		if ( profCaps  ) { ZPCAPS  = "ON"  ; }
		else             { ZPCAPS  = "OFF" ; }
		if ( profHex   ) { ZPHEX   = "ON"  ; }
		else             { ZPHEX   = "OFF" ; }
		vput( "ZPSAVE ZPNULLS ZPCAPS ZPHEX", PROFILE ) ;
	}
	if ( profLock ) { ZPLOCK  = "ON"  ; }
	else            { ZPLOCK  = "OFF" ; }
	vput( "ZPLOCK", PROFILE ) ;
	return true ;
}


void PEDIT01::read_file()
{
	iline * p_iline ;

	string inLine ;
	int pos, i, j ;
	ifstream fin( file.c_str() ) ;

	if ( !fin.is_open() )
	{
		ZRESULT = "Open Error" ;
		RC = 16 ;
		return  ;
	}
	maxLines = 1 ;
	p_iline             = new iline ;
	p_iline->iline_tod  = true ;
	p_iline->iline_data = centre( " TOP OF DATA ", ZAREAW, '*' ) ;
	data.push_back( p_iline )  ;

	p_iline              = new iline ;
	p_iline->iline_note  = true ;
	p_iline->iline_data  = " FILE READ.  TABS WILL BE CHANGED TO SPACES!!!" ;
	data.push_back( p_iline )  ;

	p_iline             = new iline ;
	p_iline->iline_note = true ;
	p_iline->iline_data = " WARNING!!! EDITOR UNDER CONSTRUCTION.  DO NOT SAVE FILES CHANGED !!!!" ;
	data.push_back( p_iline )  ;

	p_iline             = new iline ;
	p_iline->iline_note = true ;
	p_iline->iline_data = ""   ;
	data.push_back( p_iline )  ;

	while ( getline( fin, inLine ) )
	{
		p_iline = new iline ;
		pos = inLine.find( '\t' ) ;
		while ( pos != string::npos )
		{
			j = 8 - (pos % 8 ) ;
			inLine.replace( pos, 1,  j, ' ' ) ;
			pos = inLine.find( '\t', pos + 1 );
		}
		if ( maxCol < inLine.size() ) maxCol = inLine.size() ;
		p_iline->iline_file = true   ;
		p_iline->iline_data = inLine ;
		data.push_back( p_iline )    ;
		maxLines++ ;
	}
	maxCol++   ;
	maxLines++ ;
	p_iline = new iline ;
	p_iline->iline_bod  = true ;
	p_iline->iline_data = centre( " BOTTOM OF DATA ", ZAREAW, '*' ) ;
	data.push_back( p_iline ) ;
	fin.close() ;
}



bool PEDIT01::saveFile()
{
	vector<iline * >::iterator it ;

	log( "I", "Saving file " << file << endl ) ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( !(*it)->iline_file    ) { continue ; }
		if (  (*it)->iline_hidden  ) { continue ; }
		if (  (*it)->iline_deleted ) { continue ; }
		debug1( "FILE SAVE>>>" << (*it)->iline_data << endl ) ;
		fileChanged = false ;
	}

	if ( fileChanged ) { MSG = "PEDT01Q" ; ZRC = 8 ; ZRSN = 8 ; return false ; }
	ZRC  = 0 ;
	ZRSN = 4 ;
	return true ;
}


void PEDIT01::fill_dynamic_area()
{
	string w  ;
	string t1 ;
	string t2 ;
	string t3 ;
	string t4 ;
	string lcc ;
	string tmp ;
	string no  ;
	int    i, l, wI, wL, ln   ;
	int    h, t               ;
	int    dl, sl, fl         ;

	static string din( 1, 0x01 )  ;
	static string dout( 1, 0x02 ) ;

	t3.assign( ZDATAW, ' ' ) ;
	t4.assign( ZDATAW, ' ' ) ;

	ZAREA   = "" ;
	ZSHADOW = "" ;

	s2data.clear() ;
	sl = 0         ;
	fl = firstLine ;

	for ( i = 0 ; i < ZAREAD ; i++ ) { s2data[ i ] = -1 ; }
	for ( dl = firstLine ; dl < data.size() ; dl++ )
	{
		if ( data[ dl ]->iline_deleted ) { continue ; }
		if ( data[ dl ]->iline_hidden  ) { continue ; }
		if ( data[ dl ]->iline_file )    { fl++     ; }
		if ( data[ dl ]->iline_excl )
		{
			for( ; dl > 0 ; dl-- )
			{
				if ( data[ dl-1 ]->iline_hidden  ) { continue ; }
				if ( data[ dl-1 ]->iline_deleted ) { continue ; }
				if ( !data[ dl-1 ]->iline_excl )   { break ; }
				firstLine-- ;
			}
			h = 0 ;
			for( ; dl < data.size() ; dl++ )
			{
				if ( data[ dl-1 ]->iline_hidden  ) { continue ; }
				if ( data[ dl-1 ]->iline_deleted ) { continue ; }
				if ( data[ dl ]->iline_file  )     { fl++         ; }
				if ( !data[ dl ]->iline_excl )     { dl-- ; break ; }
				h++ ;
			}
			s2data[ sl ]   = dl ;
			ZAREA   = ZAREA + din + "------" + dout + centre( " " + d2ds( h ) + " line(s) excluded ", ZDATAW, '-' ) ;
			ZSHADOW = ZSHADOW + slr + sdr ;
			if ( ZAREA.size() >= ZASIZE ) break ;
			sl++ ;
			continue ;
		}
		if ( data[ dl ]->iline_lcc == "" )
		{
			if ( data[ dl ]->iline_label == "" )
			{
				if ( data[ dl ]->iline_newisrt )     { lcc = "''''''" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->iline_error )  { lcc = "==ERR>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->iline_file  )  { lcc = right( d2ds( fl ), 6, '0' ) ; ZSHADOW = ZSHADOW + sly ; }
				else if ( data[ dl ]->iline_note  )  { lcc = "=NOTE>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->iline_col   )  { lcc = "=COLS>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->iline_prof  )  { lcc = "=PROF>" ; ZSHADOW = ZSHADOW + slr ; }
				else if ( data[ dl ]->iline_chg   )  { lcc = "==CHG>" ; ZSHADOW = ZSHADOW + slr ; }
				else                                 { lcc = "******" ; ZSHADOW = ZSHADOW + slg ; }
			}
			else
			{
				lcc     = left( data[ dl ]->iline_label, 6 ) ;
				ZSHADOW = ZSHADOW + slr                      ;
			}
		}
		else
		{
			lcc     = left( data[ dl ]->iline_lcc, 6 ) ;
			ZSHADOW = ZSHADOW + slr                    ;
		}
		if ( data[ dl ]->iline_file )
		{
			if ( data[ dl ]->iline_hex || profHex )
			{
				t1 = substr( data[ dl ]->iline_data, startCol, ZDATAW ) ;
				t2 = cs2xs( t1 ) ;
				ln = data[ dl ]->iline_data.size() - startCol + 1 ;
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
				ZAREA   = ZAREA + din + lcc + din  + t1  ;
				ZAREA   = ZAREA + "       " + dout + t3  ;
				ZAREA   = ZAREA + "       " + dout + t4  ;
				ZAREA   = ZAREA + dout + div ;
				ZSHADOW = ZSHADOW + sdg ;
				ZSHADOW = ZSHADOW + slw + sdg ;
				ZSHADOW = ZSHADOW + slw + sdg ;
				ZSHADOW = ZSHADOW + slw + sdw ;
				s2data[ sl ] = dl ;
				sl = sl + 4 ;
			}
			else
			{
				ZAREA   = ZAREA + din + lcc + din + substr( data[ dl ]->iline_data, startCol, ZDATAW ) ;
				ZSHADOW = ZSHADOW + sdg ;
				s2data[ sl ] = dl ;
				sl++ ;
			}
		}
		else
		{
			if ( data[ dl ]->iline_col )
			{
				tmp = substr( "+----+----+----+---", startCol%10+1, 10-startCol%10 ) ;
				if ( startCol%10 == 0 ) { tmp = "" ; t = startCol / 10 ; }
				else t = startCol / 10 + 1 ;
				for ( i = 0 ; i < (ZDATAW/10+1) ; i++ )
				{
					tmp = tmp + d2ds( t%10 ) + "----+----" ;
					t++ ;
				}
				ZAREA = ZAREA + din + lcc + dout + substr( tmp, 1, ZDATAW ) ;
			}
			else
			{
				ZAREA = ZAREA + din + lcc + dout + substr( data[ dl ]->iline_data, 1, ZDATAW ) ;
			}
			ZSHADOW = ZSHADOW + sdw ;
			s2data[ sl ] = dl ;
			sl++ ;
		}
		if ( ZAREA.size() >= ZAREAW * ZAREAD ) break ;
	}
	//	addHilight( data, fileType, firstLine, startCol, ZAREAW, ZAREAD, ZSHADOW ) ;

	ZAREA.resize( ZASIZE, ' ' ) ;
	ZSHADOW.resize( ZASIZE, N_GREEN ) ;

	CAREA        = ZAREA ;
	rebuildZAREA = false ;
}


void PEDIT01::getZAREAchanges()
{
	int i   ;
	int j   ;
	int k   ;
	int l   ;
	int dl  ;
	int off ;
	string lc ;

	const char duserMod(0x3) ;
	const char ddataMod(0x4) ;

	sTouched.clear() ;
	sChanged.clear() ;

	l = firstLine ;
	for ( i = 0 ; i < ZAREAD ; i++ )
	{
		dl            = s2data[ i ] ;
		off           = i * ZAREAW  ;
		sTouched[ i ] = false       ;
		sChanged[ i ] = false       ;
		if ( ZAREA[ off ] == ddataMod )
		{
			lc = "      " ;
			k  = 0        ;
			if ( data[ dl ]->iline_lcc == "" && data[ dl ]->iline_label == "" )
			{
				for ( j = 1+off ; j < 1+off+6 ; j++ )
				{
					if ( ZAREA[ j ] != CAREA[ j ] ) { lc.replace( k, 1, 1, ZAREA[ j ] ) ; }
					k++ ;
				}
			}
			else
			{
				lc = ZAREA.substr( 1+off , 6 ) ;
			}
			lc = strip( upper( lc ) ) ;
			if ( lc == "" )
			{
				if ( data[ dl ]->iline_lcc == "" ) { data[ dl ]->iline_label = "" ; }
				else                               { data[ dl ]->iline_lcc   = "" ; }
			}
			else
			{
				data[ dl ]->iline_lcc = lc ;
			}
			rebuildZAREA = true ;
		}
		if ( ZAREA[ off + 7 ] == duserMod )
		{
			sTouched[ i ]         = true ;
			rebuildZAREA          = true ;
		}
		else if ( ZAREA[ off + 7 ] == ddataMod )
		{
			data[ dl ]->iline_chg = true ;
			sChanged[ i ]         = true ;
			rebuildZAREA          = true ;
		}
		l++ ;
	}
}


void PEDIT01::updateData()
{
	int i  ;
	int j  ;
	int dl ;

	iline * p_iline ;
	vector<iline * >::iterator it ;

	for ( i = sChanged.size() - 1 ; i >=0 ; i-- )
	{
		dl = s2data[ i ] ;
		if ( dl == -1 ) { continue ; }
		if ( data[ dl ]->iline_newisrt )
		{
			if ( !sTouched[ i ] && !sChanged[ i ] )
			{
				for ( it = data.begin(), j = 0 ; j < dl ; j++ ) { it++ ; }
				delete *it          ;
				data.erase( it )    ;
				maxLines--          ;
				rebuildZAREA = true ;
				continue            ;
			}
			else
			{
				data[ dl ]->iline_newisrt = false ;
				data[ dl ]->iline_lcc     = "I"   ;
				if ( startCol > 1 ) { data[ dl ]->iline_data = substr( data[ dl ]->iline_data, 1, startCol-1 ) + strip( ZAREA.substr( 8+(i*ZAREAW), ZAREAW-8 ), 'T', ' ' ) ; }
				else                { data[ dl ]->iline_data = strip( ZAREA.substr( 8+(i*ZAREAW), ZAREAW-8 ), 'T', ' ' ) ; }
				if ( profCaps ) { data[ dl ]->iline_data = upper( p_iline->iline_data ) ; }
				rebuildZAREA  = true  ;
				fileChanged   = true  ;
				REDOavailable = false ;
				continue              ;
			}
		}
		if ( !sChanged[ i ] ) { continue ; }
		undoLevel++ ;
		debug1( " processing screen line " << i << " for data line " << dl << endl ; )
		p_iline  = new iline   ;
		for ( it = data.begin(), j = 0 ; j <= dl ; j++ ) { it++ ; }
		data.insert( it, p_iline ) ;
		*p_iline = *data[ dl ]     ;
		data[ dl ]->iline_hidden = true ;
		data[ dl ]->iline_label  = ""   ;
		p_iline->iline_lvlp = data[ dl ]->iline_lvl ;
		p_iline->iline_lvl  = undoLevel             ;
		maxLines++                 ;
		if ( startCol > 1 ) { p_iline->iline_data = substr( p_iline->iline_data, 1, startCol-1 ) + strip( ZAREA.substr( 8+(i*ZAREAW), ZAREAW-8 ), 'T', ' ' ) ; }
		else                { p_iline->iline_data = strip( ZAREA.substr( 8+(i*ZAREAW), ZAREAW-8 ), 'T', ' ' ) ; }
		if ( profCaps ) { p_iline->iline_data = upper( p_iline->iline_data ) ; }
		rebuildZAREA  = true  ;
		fileChanged   = true  ;
		REDOavailable = false ;
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
	bool found ;
	string w1  ;
	string w2  ;
	string w3  ;
	string w4  ;
	string wr  ;

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

	if ( w1 == "CAPS" )
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
		return    ;
	}

	if ( w1 == "C" || w1 == "CHANGE" )
	{
		if ( setFindChangeExcl( 'C' ) > 0 ) { MSG = "PEDT018" ; return ; }
		ZCMD = ""           ;
		rebuildZAREA = true ;
		return              ;
	}

	if ( w1 == "DEL" || w1 == "DELETE" )
	{
		if ( ws != 3 ) { MSG = "PEDT011" ; return ; }
		if ( w2 == "X" || w2 == "EX" || w2 == "EXCLUDED" )
		{
			if ( w3 != "ALL" ) { MSG = "PEDT011" ; return ; }
			i = 0 ;
			undoLevel++ ;
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if (  (*it)->iline_deleted ) { continue ; }
				if (  (*it)->iline_hidden  ) { continue ; }
				if ( !(*it)->iline_excl    ) { continue ; }
				(*it)->iline_lcc = "" ;
				if ( (*it)->iline_bod ) { break  ; }
				(*it)->iline_deleted = true      ;
				(*it)->iline_lvl     = undoLevel ;
				i++ ;
			}
			if ( i == 0 ) { undoLevel-- ; }
			ZLINES = d2ds( i )    ;
			MSG    = "PEDT01M"    ;
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
			REDOavailable = false ;
		}
		else if ( w2 == "NX" )
		{
			if ( w3 != "ALL" ) { MSG = "PEDT011" ; return ; }
			i = 0 ;
			undoLevel++ ;
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if ( (*it)->iline_deleted ) { continue ; }
				if ( (*it)->iline_hidden  ) { continue ; }
				if ( (*it)->iline_excl    ) { continue ; }
				(*it)->iline_lcc = "" ;
				if ( (*it)->iline_bod ) { break  ; }
				if ( (*it)->iline_tod ) { continue  ; }
				(*it)->iline_deleted = true      ;
				(*it)->iline_lvl     = undoLevel ;
				i++ ;
			}
			if ( i == 0 ) { undoLevel-- ; }
			ZLINES = d2ds( i )    ;
			MSG    = "PEDT01M"    ;
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
			REDOavailable = false ;
		}
		else { MSG = "PEDT011" ; return ; }
		ZCMD = ""           ;
		rebuildZAREA = true ;
		return              ;
	}


	if ( w1 == "F" || w1 == "FIND" )
	{
		if ( setFindChangeExcl( 'F' ) > 0 ) { return ; }
		ZCMD = ""           ;
		rebuildZAREA = true ;
		return              ;
	}

	if ( w1 == "FLIP" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		for ( it = data.begin() ; it != data.end() ; it++ )
		{
			if ( (*it)->iline_bod || (*it)->iline_tod ) { continue ; }
			(*it)->iline_excl = !(*it)->iline_excl ;
		}
		ZCMD = ""           ;
		rebuildZAREA = true ;
		return              ;
	}

	if ( w1 == "HEX" )
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
		return    ;
	}

	if ( w1 == "PROF" || w1 == "PROFILE" )
	{
		if ( ws == 1 )
		{
			if ( profSave  ) { ZPSAVE  = "ON"  ; }
			else             { ZPSAVE  = "OFF" ; }
			if ( profNulls ) { ZPNULLS = "ON"  ; }
			else             { ZPNULLS = "OFF" ; }
			if ( profLock  ) { ZPLOCK  = "ON"  ; }
			else             { ZPLOCK  = "OFF" ; }
			if ( profCaps  ) { ZPCAPS  = "ON"  ; }
			else             { ZPCAPS  = "OFF" ; }
			if ( profHex   ) { ZPHEX   = "ON"  ; }
			else             { ZPHEX   = "OFF" ; }
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if ( (*it)->iline_prof ) 
				{
					delete *it  ;
					itt = it    ;
					itt--       ;
					data.erase( it ) ;
					it = itt    ;
					maxLines--  ;
				}
			}
			rebuildZAREA = true  ;

			dl = firstLine ;
			for ( it = data.begin(), i = 0 ; i <= dl ; i++ ) { it++ ; }

			p_iline  = new iline ;
			p_iline->iline_prof = true ;
			p_iline->iline_data = "....PROF(" + ZAPPLID + ")...UNDO(STG)...AUTOSAVE("+ZPSAVE+")..." ;
			it = data.insert( it, p_iline ) ;
			it++ ;

			p_iline  = new iline ;
			p_iline->iline_prof = true ;
			p_iline->iline_data = "....NUM(OFF)...CAPS("+ZPCAPS+")...HEX("+ZPHEX+")...." ;
			it = data.insert( it, p_iline ) ;
			it++ ;

			p_iline  = new iline ;
			p_iline->iline_prof = true ;
			p_iline->iline_data = "....NULLS("+ZPNULLS+")....PROFILE LOCK("+ZPLOCK+")...." ;
			data.insert( it, p_iline ) ;
			maxLines = maxLines + 3 ;
			rebuildZAREA = true  ;
		}
		else if ( ws != 2 )        { MSG = "PEDT011" ; return ; }
		else if ( w2 == "SAVE"   ) { profSave = true  ; }
		else if ( w2 == "NOSAVE" ) { profSave = false ; }
		else if ( w2 == "LOCK"   ) { profLock = true  ; }
		else if ( w2 == "UNLOCK" ) { profLock = false ; }
		else                       { MSG = "PEDT011" ; return ; }
		ZCMD = "" ;
		return    ;
	}

	if ( w1 == "REDO" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		actionREDO() ;
		ZCMD = ""    ;
		return       ;
	}

	if ( w1 == "RC" || w1 == "RCHANGE" )
	{
		if ( !find_parms.fcx_cset ) { MSG = "PEDT019" ; return ; }
		ZCMD = ""           ;
		rebuildZAREA = true ;
		return              ;
	}


	if ( w1 == "RF" || w1 == "RFIND" )
	{
		if ( !find_parms.fcx_fset ) { MSG = "PEDT019" ; return ; }
		ZCMD = ""           ;
		rebuildZAREA = true ;
		return              ;
	}

	if ( w1 == "RES" || w1 == "RESET" )
	{
		if ( w2 == "" )
		{
			for ( it = data.begin() ; it != data.end() ; it++ )
			{
				if ( (*it)->iline_note || (*it)->iline_col || (*it)->iline_prof ) 
				{
					delete *it  ;
					itt = it    ;
					itt--       ;
					data.erase( it ) ;
					it = itt    ;
					maxLines--  ;
					rebuildZAREA = true ;
				}
				(*it)->iline_excl  = false ;
				(*it)->iline_hex   = false ;
				(*it)->iline_chg   = false ;
				(*it)->iline_error = false ;
				(*it)->iline_lcc   = ""    ;
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
			for ( it = its ; it != ite ; it++ ) { (*it)->iline_lcc = "" ; }
			rebuildZAREA = true ;
		}
		else if ( ( w2 == "LABEL" && w3 == "" ) )
		{
			for ( it = data.begin() ; it != data.end() ; it++ ) { (*it)->iline_label = "" ; }
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
			for ( it = its ; it != ite ; it++ ) { (*it)->iline_excl = false ; }
			rebuildZAREA = true ;
		}

		else  { MSG = "PEDT011" ; return ; }
		ZCMD = "" ;
		return    ;
	}

	if ( w1 == "SAVE" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		saveFile()   ;
		ZCMD = ""    ;
		return       ;
	}

	if ( w1 == "SHOW" )
	{
		log( "A", " Dumping array contents:: (undo level) " << undoLevel << endl ; )
		log( "A", " ++++++++++++++++++++++++" << endl ; )
		for ( it = data.begin() ; it != data.end() ; it++ )
		{
			if ( !(*it)->iline_file ) { continue ; }
			log( "A", " Current  lvl: " << (*it)->iline_lvl << endl  ; )
			log( "A", " Previous lvl: " << (*it)->iline_lvlp << endl ; )
			log( "A", " Next     lvl: " << (*it)->iline_lvln << endl ; )
			if ( (*it)->iline_hidden ) { log( "A", " Line is Hidden" << endl ; ) }
			if ( (*it)->iline_excl   ) { log( "A", " Line is Excluded" << endl ; ) }
			if ( (*it)->iline_deleted) { log( "A", " Line is Logically deleted" << endl ; ) }
			log( "A", " Line command: " << (*it)->iline_lcc << endl ; )
			log( "A", " Line label  : " << (*it)->iline_label << endl ; )
			log( "A", " Record      : " << (*it)->iline_data << endl ; )
			log( "A", " ++++++++++++++++++++++++++++++++++++++++++++++++" << endl ; )
		}
		ZCMD = ""    ;
		return       ;
	}

	if ( w1 == "UNDO" )
	{
		if ( ws > 1 ) { MSG = "PEDT011" ; return ; }
		actionUNDO() ;
		ZCMD = ""    ;
		return       ;
	}

	if ( w1 == "X" || w1 == "EX" || w1 == "EXCLUDE" )
	{
		if ( ws == 1 ) { MSG = "PEDT011" ; return ; }
		if ( w2 == "ALL" && ws == 2 )
		{
			for ( dl = 1 ; dl < data.size()-1 ; dl++ )
			{
				if (  data[ dl ]->iline_hidden  ) { continue ; }
				if (  data[ dl ]->iline_deleted ) { continue ; }
				data[ dl ]->iline_excl = true ;
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
				if (  data[ dl ]->iline_hidden  ) { continue ; }
				if (  data[ dl ]->iline_deleted ) { continue ; }
				if (  data[ dl ]->iline_excl    ) { continue ; }
				if ( !data[ dl ]->iline_file    ) { continue ; }
				if ( !found )
				{
					if ( find_parms.fcx_slab != "" && ( data[ dl ]->iline_label == find_parms.fcx_slab ) ) { found = true ; }
					if ( !found ) { continue ; }
				}
				if ( find_parms.fcx_regreq )
				{
					itss = data[ dl ]->iline_data.begin() ;
					itse = data[ dl ]->iline_data.end()   ;
					if ( find_parms.fcx_scol > 1 )
					{
						if ( find_parms.fcx_scol <= data[ dl ]->iline_data.size() )
						{
							for ( i = 1 ; i < find_parms.fcx_scol ; i++ ) { itss++ ; }
						}
						else { continue ; }
					}
					if ( find_parms.fcx_ecol > 0 )
					{
						if ( find_parms.fcx_ecol <= data[ dl ]->iline_data.size() )
						{
							itse = data[ dl ]->iline_data.begin() ;
							for ( i = 1 ; i < find_parms.fcx_ecol ; i++ ) { itse++ ; }
						}
					}
					if ( regex_search( itss, itse, regexp ) ) { data[ dl ]->iline_excl = true ; j++ ; }
					if ( find_parms.fcx_elab != "" && ( data[ dl ]->iline_label == find_parms.fcx_elab ) ) { break ; }
					continue ;
				}
				if ( find_parms.fcx_asis )
				{
					p1 = data[ dl ]->iline_data.find( find_parms.fcx_string, find_parms.fcx_scol-1 ) ;
				}
				else
				{
					p1 = upper( data[ dl ]->iline_data).find( find_parms.fcx_string, find_parms.fcx_scol-1 ) ;
				}
				if ( p1 != string::npos )
				{
					if ( find_parms.fcx_ecol == 0 || ( p1 < find_parms.fcx_ecol ) ) { data[ dl ]->iline_excl = true ; j++ ; }
				}
				if ( find_parms.fcx_elab != "" && ( data[ dl ]->iline_label == find_parms.fcx_elab ) ) { break ; }
			}
			MSG = "PEDT01L" ;
			ZLINES = d2ds( j ) ;
		}
		rebuildZAREA = true ;
		ZCMD = ""           ;
		return              ;
	}
	MSG = "PEDT011" ;
}


void PEDIT01::actionSLCMDS()
{
	// For each line in the data vector, action the single line commands
	// For any change to the data, set REDOavailable to false

	int i    ;
	int j    ;
	int dl   ;
	int mult ;
	string smult ;
	string lcc   ;

	iline * p_iline ;
	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->iline_lcc == ""  ) { continue ; }
		if ( (*it)->iline_lcc.substr( 0, 1 ) == "." )
		{
			if ( (*it)->iline_lcc.size() == 1 ) { MSG = "PEDT014" ; return ; }
			for ( itt = data.begin() ; itt != data.end() ; itt++ )
			{
				if ( (*itt)->iline_label == (*it)->iline_lcc ) { (*itt)->iline_label = "" ; break ; }
			}
			(*it)->iline_label = (*it)->iline_lcc ;
			(*it)->iline_lcc   = ""               ;
			rebuildZAREA = true  ;
			continue             ;
		}

		smult = "      " ;
		lcc   = "      " ;
		mult  = 1        ;
		for ( j = 0 ; j < (*it)->iline_lcc.size() ; j++ )
		{
			if ( isdigit( (*it)->iline_lcc[ j ] ) ) { smult[ j ] = (*it)->iline_lcc[ j ] ; }
			else                                    { lcc[ j ]   = (*it)->iline_lcc[ j ] ; }
		}
		smult = strip( smult ) ;
		lcc   = strip( lcc   ) ;
		if ( smult != "" )
		{
			if ( datatype( smult, 'W' ) ) { mult = ds2d( smult ) ; }
			else                          { log( "E", "Invalid line command multiplier " << smult << endl ; ) }
		}

		if ( wordpos( lcc, blkcmds ) == 0 && wordpos( lcc, sglcmds ) == 0 ) { MSG = "PEDT012" ; return ; }
		if ( (*it)->iline_tod && wordpos( lcc, todlcmds ) == 0 ) { MSG = "PEDT013" ; return ; }
		if ( (*it)->iline_bod && wordpos( lcc, bodlcmds ) == 0 ) { MSG = "PEDT013" ; return ; }

		if ( lcc == "D" )
		{
			undoLevel++ ;
			(*it)->iline_lcc = "" ;
			for ( j = 0 ; j < mult ; j++ )
			{
				if ( (*it)->iline_bod ) { break  ; }
				p_iline = new iline  ;
				*p_iline          = **it      ;
				(*it)->iline_hidden = true      ;
				p_iline->iline_deleted = true      ;
				p_iline->iline_lvl     = undoLevel ;
				it++                             ;
				it = data.insert( it, p_iline )    ;
			}
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
			REDOavailable = false ;
			continue              ;
		}
		if ( lcc == "I" )
		{
			undoLevel++ ;
			(*it)->iline_lcc = ""  ;
			for ( j = 0 ; j < mult ; j++ )
			{
				p_iline = new iline  ;
				p_iline->iline_file    = true      ;
				p_iline->iline_newisrt = true      ;
				p_iline->iline_lvl     = undoLevel ;
				it++                 ;
				it = data.insert( it, p_iline )    ;
				maxLines++    ;
			}
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
			REDOavailable = false ;
			continue              ;
		}
		if ( lcc == "HX" )
		{
			(*it)->iline_lcc = ""   ;
			(*it)->iline_hex = true ;
			for ( j = 1 ; j < mult ; j++ )
			{
				it++ ;
				if ( (*it)->iline_bod ) { break  ; }
				if ( (*it)->iline_hidden  ) { j--; continue ; }
				if ( (*it)->iline_deleted ) { j--; continue ; }
				(*it)->iline_hex = true  ;
			}
			rebuildZAREA = true   ;
			continue              ;
		}
		if ( lcc == "R" )
		{
			undoLevel++ ;
			(*it)->iline_lcc  = ""  ;
			for ( j = 0 ; j < mult ; j++ )
			{
				p_iline = new iline ;
				*p_iline             = **it      ;
				p_iline->iline_label = ""        ;
				p_iline->iline_lvl   = undoLevel ;
				it++                             ;
				it = data.insert( it, p_iline )  ;
				maxLines++ ;
			}
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
			REDOavailable = false ;
			continue              ;
		}
		if ( lcc == "X" )
		{
			(*it)->iline_lcc  = "" ;
			(*it)->iline_excl = true ;
			for ( j = 1 ; j < mult ; j++ )
			{
				it++ ;
				if ( (*it)->iline_bod ) { break  ; }
				if ( (*it)->iline_hidden  ) { j--; continue ; }
				if ( (*it)->iline_deleted ) { j--; continue ; }
				(*it)->iline_excl = true ;
			}
			rebuildZAREA = true ;
			continue            ;
		}
		if ( lcc == "COL" || lcc == "COLS" )
		{
			(*it)->iline_lcc = "" ;
			p_iline = new iline   ;
			p_iline->iline_col = true ;
			it++ ;
			it = data.insert( it, p_iline ) ;
			maxLines++           ;
			rebuildZAREA = true  ;
			continue             ;
		}
		if ( lcc == "UC" )
		{
			undoLevel++ ;
			(*it)->iline_lcc = "" ;
			for ( j = 0 ; j < mult ; j++ )
			{
				if ( (*it)->iline_bod ) { break  ; }
				p_iline = new iline ;
				*p_iline             = **it ;
				(*it)->iline_hidden  = true ;
				p_iline->iline_data  = upper( p_iline->iline_data ) ;
				p_iline->iline_lvl   = undoLevel ;
				it++                             ;
				it = data.insert( it, p_iline )  ;
				maxLines++ ;
			}
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
			REDOavailable = false ;
			continue              ;
		}
		if ( lcc == "LC" )
		{
			undoLevel++ ;
			(*it)->iline_lcc = "" ;
			for ( j = 0 ; j < mult ; j++ )
			{
				if ( (*it)->iline_bod ) { break  ; }
				p_iline = new iline ;
				*p_iline             = **it ;
				(*it)->iline_hidden  = true ;
				p_iline->iline_data  = lower( p_iline->iline_data ) ;
				p_iline->iline_lvl   = undoLevel ;
				it++                             ;
				it = data.insert( it, p_iline )  ;
				maxLines++ ;
			}
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
			REDOavailable = false ;
			continue              ;
		}
	}
}


void PEDIT01::actionBLCMDS()
{
	// For each line in the data vector, action the block line command
}


void PEDIT01::actionUNDO()
{
	// Restore labels as these are removed from hidden records (as they can change in the mean time)
	//

	int i  ;
	int dl ;

	iline * p_iline ;
	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	if ( undoLevel == 0 )
	{
		log( "A", "No more undo's available" << endl ; )
		MSG = "PEDT017" ;
		return ;
	}
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->iline_hidden ) { continue ; }
		if ( (*it)->iline_lvl == undoLevel )
		{
			if ( (*it)->iline_lvlp == -1 )  // Undo insert/repeat
			{
				(*it)->iline_deleted = !(*it)->iline_deleted   ;
				rebuildZAREA  = true ;
				fileChanged   = true ;
				REDOavailable = true ;
				continue ;
			}
			log( "A", " Found record to undo: " << endl  ; )  // Undo update
			for ( itt = it ; ; itt-- )
			{
				log( "A", "Checking record" << endl ; )
				if ( (*itt)->iline_lvl == (*it)->iline_lvlp ) { break ; }
				log( "A", "--No match.  Next...." << endl ; )
			}
			log( "A", " Current record found   : " << (*it)->iline_data << endl  ; )
			log( "A", " Previous version found : " << (*itt)->iline_data << endl  ; )
			(*itt)->iline_hidden = false ;
			(*itt)->iline_lvln   = (*it)->iline_lvl ;
			delete *it ;
			data.erase( it ) ;
			rebuildZAREA  = true ;
			fileChanged   = true ;
			REDOavailable = true ;
			break ;
		}
	}
	undoLevel-- ;
}


void PEDIT01::actionREDO()
{
	// Restore labels as these are removed from hidden records (as they can change in the mean time)
	//
	bool found ;
	int i  ;
	int dl ;

	undoLevel++ ;
	log( "A", " Re-doing level " << undoLevel << endl ; )

	iline * p_iline ;
	vector<iline * >::iterator it  ;
	vector<iline * >::iterator itt ;

	if ( !REDOavailable )
	{
		log( "A", "No redo's available" << endl ; )
		MSG = "PEDT018" ;
		return ;
	}

	found = false ;

	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->iline_hidden ) { continue ; }
		if ( (*it)->iline_lvl == undoLevel && (*it)->iline_lvlp == -1 )  //Redo Insert (undo logical delete)
		{
			log( "A", " Found record to redo: " << endl  ; )
			found = true ;
			(*it)->iline_deleted = !(*it)->iline_deleted ;
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
			continue              ;
		}

		if ( (*it)->iline_lvln == undoLevel )
		{
			log( "A", " Found record to redo: " << endl  ; )
			found = true ;
			for ( itt = it ; ; itt-- )
			{
				log( "A", "Checking record" << endl ; )
				if ( (*itt)->iline_lvl == (*it)->iline_lvln ) { break ; }
				log( "A", "--No match.  Next...." << endl ; )
			}
			log( "A", " Current record found   : " << (*it)->iline_data << endl  ; )
			log( "A", " Previous version found : " << (*itt)->iline_data << endl  ; )
			p_iline  = new iline ;
			*p_iline = **itt ;
			(*it)->iline_hidden   = true  ;
			p_iline->iline_hidden = false ;
			it++ ;
			it = data.insert( it, p_iline ) ;
			maxLines++            ;
			rebuildZAREA  = true  ;
			fileChanged   = true  ;
		}
	}
	if ( !found )
	{
		log( "A", "No more redo's available" << endl ; )
		MSG = "PEDT018" ;
		undoLevel--     ;
	}
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


bool PEDIT01::returnLabelItr(string label, vector<iline * >::iterator & it , int & posn )
{
	posn = 0 ;
	for ( it = data.begin() ; it != data.end() ; it++ )
	{
		if ( (*it)->iline_label == label ) { return true ; }
		posn++ ;
	}
	return false ;
}


void PEDIT01::cleanup_custom()
{
	log( "E", "Customised cleanup procedure" << endl ) ;

}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PEDIT01 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }

