/* Compile with ::                                                                        */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libplfhist1.so -o libplfhist1.so plfhist1.cpp */

/*
  Copyright (c) 2024 Daniel John Erdos

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


/*************************************************************************************/
/*                                                                                   */
/* Panel Field History application.                                                  */
/*                                                                                   */
/* Field/value pairs are stored in a table of the panel name, in                     */
/* directory ZFLDHIST (defined in lspf.h).                                           */
/*                                                                                   */
/* On exit, if ZRC = 0 then ZRESULT will be placed in the cursor field.              */
/*                                                                                   */
/* In the panel, add )INIT statements:                                               */
/*   .HIST = field                                                                   */
/*   .HIST = 'field1,field2,field3'                                                  */
/*   .HIST = * to add all normal input fields greater that 1 character,              */
/*             except command line and scroll amount.                                */
/*                                                                                   */
/* .HIST can appear multiple times in the )INIT section.  The effect is cumulative.  */
/*                                                                                   */
/* The feld history program can be changed in opton 0.8 so, for example,             */
/* a different program can keep field values in a table of the field name rather     */
/* than the panel name.  All fields with the same name can then be stored and        */
/* displayed together.                                                               */
/*                                                                                   */
/*************************************************************************************/


#include "../lspfall.h"
#include "plfhist1.h"

#define MAX_KEEP 30

LSPF_APP_MAKER( plfhist1 )


plfhist1::plfhist1()
{
	STANDARD_HEADER( "Panel Field History application", "1.0.0" )
}


void plfhist1::application()
{
	string p1 ;
	string p2 ;
	string p3 ;
	string p4 ;
	string p5 ;
	string p6 ;

	auto it = pgm_parms.find( word( PARM, 1 ) ) ;
	if ( it == pgm_parms.end() )
	{
		llog( "E", "Invalid parameter passed to PLFHIST1: " << PARM << endl ) ;
		return ;
	}

	qname = "SPFEDIT" ;
	rname = "*" + word( PARM, 2 ) + "*" ;

	libdef( "TABLIB", "LIBRARY", "ZFLDHIST" ) ;

	switch ( it->second )
	{
	case RR_PLU:
		    p1 = word( PARM, 2 ) ;
		    p2 = word( PARM, 3 ) ;
		    p3 = subword( PARM, 4 ) ;
		    addFieldHistoryEntry( p1, p2, p3 ) ;
		    break ;

	case RR_PLD:
		    p1 = word( PARM, 2 ) ;
		    p2 = word( PARM, 3 ) ;
		    p3 = word( PARM, 4 ) ;
		    p4 = word( PARM, 5 ) ;
		    p5 = word( PARM, 6 ) ;
		    p6 = word( PARM, 7 ) ;
		    getHistoryEntries( p1, p2, p3, p4, p5, p6 ) ;
		    break ;

	}

	libdef( "TABLIB" ) ;
}


void plfhist1::addFieldHistoryEntry( const string& panel,
				     const string& field,
				     const string& value )
{
	//
	// Add the field value to the panel table.
	// Also delete any field entries that are greater than the maximum keep value.
	//

	int i ;

	string fhfield ;
	string fhvalue ;

	const string& table = panel ;

	const string vlist1 = "FHFIELD FHVALUE" ;

	vdefine( vlist1, &fhfield, &fhvalue ) ;

	openTableUP( table ) ;

	fhfield = field ;
	fhvalue = value ;
	tbsarg( table ) ;
	tbscan( table ) ;
	if ( RC > 0 )
	{
		tbadd( table ) ;
	}

	tbsort( table, "(FHFIELD,C,A)" ) ;

	tbtop( table ) ;
	tbvclear( table ) ;
	fhfield = field ;
	tbsarg( table ) ;

	tbscan( table ) ;
	for ( i = 1 ; RC == 0 ; ++i )
	{
		if ( i > MAX_KEEP )
		{
			while ( RC == 0 )
			{
				tbdelete( table ) ;
				if ( RC == 0 )
				{
					tbscan( table ) ;
				}
			}
			break ;
		}
		tbscan( table ) ;
	}

	closeTable( table ) ;

	vdelete( vlist1 ) ;
}


void plfhist1::getHistoryEntries( const string& panel,
				  const string& prow,
				  const string& pcol,
				  const string& frow,
				  const string& fcol,
				  const string& field )
{
	//
	// Display field history entries.
	// Retrieve the value the cursor is placed on, put it in ZRESULT and move to the top.
	// Also delete any entries that have been blanked out.
	//

	int zcurinx ;
	int ztdsels ;

	string fhfield ;
	string fhvalue ;
	string value1 ;
	string value2 ;

	const string temp = "FH1" + d2ds( taskid(), 5 ) ;

	vector<string> histories ;

	const string& table = panel ;

	const string vlist1 = "FHFIELD FHVALUE VALUE1 VALUE2" ;
	const string vlist2 = "ZCURINX ZTDSELS" ;

	vdefine( vlist1, &fhfield, &fhvalue, &value1, &value2 ) ;
	vdefine( vlist2, &zcurinx, &ztdsels ) ;

	ZRC = 4 ;

	openTableRO( table ) ;

	tbvclear( table ) ;
	fhfield = field ;
	tbtop( table ) ;
	tbsarg( table ) ;
	tbscan( table ) ;
	while ( RC == 0 )
	{
		histories.push_back( fhvalue ) ;
		tbscan( table ) ;
	}

	endTable( table ) ;

	if ( histories.size() == 0 )
	{
		set_shared_var( "STR1", field ) ;
		set_shared_var( "STR2", panel ) ;
		setmsg( "PSYE043Q" ) ;
		vdelete( vlist1, vlist2 ) ;
		return ;
	}

	addpop( "", ds2d( frow ) - ds2d( prow ), ds2d( fcol ) - ds2d( pcol ) - 3 ) ;

	tbcreate( temp,
		  "",
		  "(VALUE1,VALUE2)",
		  NOWRITE,
		  NOREPLACE ) ;
	for ( uint i = 0 ; i < histories.size() ; ++i )
	{
		value1 = histories[ i ] ;
		value2 = value1 ;
		tbadd( temp ) ;
	}
	tbtop( temp ) ;
	tbdispl( temp,
		 "PLFHSTA1",
		 "",
		 "",
		 0,
		 1,
		 "NO" ) ;
	if ( RC > 4 )
	{
		rempop() ;
		tbend( temp ) ;
		vdelete( vlist1, vlist2 ) ;
		return ;
	}
	while ( ztdsels > 0 )
	{
		tbput( temp ) ;
		if ( ztdsels > 1 )
		{
			tbdispl( temp ) ;
		}
		else
		{
			ztdsels = 0 ;
		}
	}
	if ( zcurinx > 0 )
	{
		tbtop( temp ) ;
		tbskip( temp, zcurinx ) ;
		if ( value1 != "" )
		{
			ZRC     = 0 ;
			ZRESULT = value2 ;
		}
	}
	histories.clear() ;
	tbtop( temp ) ;
	tbvclear( temp ) ;
	tbscan( temp, "VALUE1" ) ;
	while ( RC == 0 )
	{
		histories.push_back( value2 ) ;
		tbscan( temp, "VALUE1" ) ;
	}
	tbend( temp ) ;
	if ( histories.size() > 0 || ZRESULT != "" )
	{
		openTableUP( table ) ;
		for ( uint i = 0 ; i < histories.size() ; ++i )
		{
			fhfield = field ;
			fhvalue = histories[ i ] ;
			tbtop( table ) ;
			tbsarg( table ) ;
			tbscan( table ) ;
			if ( RC == 0 )
			{
				tbdelete( table ) ;
			}

		}
		if ( ZRESULT != "" )
		{
			fhfield = field ;
			fhvalue = ZRESULT ;
			tbtop( table ) ;
			tbsarg( table ) ;
			tbscan( table ) ;
			if ( RC == 0 )
			{
				tbdelete( table ) ;
				if ( RC == 0 )
				{
					tbtop( table ) ;
					tbadd( table ) ;
					tbsort( table, "(FHFIELD,C,A)" ) ;
				}
			}
		}
		closeTable( table ) ;
	}

	rempop() ;

	vdelete( vlist1, vlist2 ) ;
}


void plfhist1::openTableRO( const string& table )
{
	wait_enqueue() ;

	tbopen( table, NOWRITE, "TABLIB" ) ;
	if ( RC == 8 )
	{
		createDefaultTable( table ) ;
		tbopen( table, NOWRITE, "TABLIB" ) ;
		if ( RC > 0 )
		{
			llog( "E", "Table "+ table +" cannot be opened NOWRITE.  RC="<< RC <<endl ) ;
			uabend() ;
		}
	}
}


void plfhist1::openTableUP( const string& table )
{
	wait_enqueue() ;

	tbopen( table, WRITE, "TABLIB" ) ;
	if ( RC == 8 )
	{
		createDefaultTable( table ) ;
		tbopen( table, WRITE, "TABLIB" ) ;
		if ( RC > 0 )
		{
			llog( "E", "Table "+ table +" cannot be opened WRITE.  RC="<< RC <<endl ) ;
			uabend() ;
		}
	}
}


void plfhist1::closeTable( const string& table )
{
	//
	// Save and close table.
	//

	tbclose( table, "", "TABLIB" ) ;

	release_enqueue() ;
}


void plfhist1::endTable( const string& table )
{
	//
	// Close table without saving.
	//

	tbend( table ) ;

	release_enqueue() ;
}


void plfhist1::createDefaultTable( const string& table )
{
	//
	// Create default table.
	//

	tbcreate( table,
		  "",
		  "(FHFIELD,FHVALUE)",
		  WRITE,
		  NOREPLACE,
		  "TABLIB" ) ;
	if ( RC > 0 )
	{
		llog( "E", "Table "+ table +" cannot be created.  RC="<< RC <<endl ) ;
		uabend() ;
	}

	tbclose( table, "", "TABLIB" ) ;
}


void plfhist1::wait_enqueue()
{
	//
	// Enqueue exclusive (wait) using table name so only one task has it open at a time.
	//

	RC = 8 ;
	while ( RC == 8 )
	{
		enq( qname, rname ) ;
		if ( RC == 8 )
		{
			boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
		}
	}
}


void plfhist1::release_enqueue()
{
	deq( qname, rname ) ;
}


void plfhist1::set_shared_var( const string& var,
			       const string& val )
{
	//
	// Store a variable in the SHARED pool.
	//

	vreplace( var, val ) ;
	vput( var, SHARED ) ;
}
