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

#define _CREATE 0x01
#define _INSERT 0x02
#define _UPDATE 0x04


// *******************************************************************************************************************************
// *************************************************** TABLE SECTION *************************************************************
// *******************************************************************************************************************************


Table::~Table()
{
	//
	// Free table row storage when deleting a table.
	//

	for ( auto row : table )
	{
		delete row ;
	}
}


void Table::loadRow( errblock& err,
		     vector<string>* row )
{
	err.setRC( 0 ) ;

	if ( table.size() >= MXTAB_SZ )
	{
		err.seterrid( TRACE_INFO(), "PSYE013F" ) ;
		return ;
	}

	row->insert( row->begin(), d2ds( ++max_urid ) ) ;
	table.push_back( row ) ;
	changed = true ;
}


void Table::reserveSpace( int tot_rows )
{
	table.reserve( tot_rows ) ;
}


vector<vector<string>*>::iterator Table::getKeyItr( errblock& err,
						    fPOOL* funcPool )
{
	//
	// Return the table row iterator of a row for a keyed table or table.end() if not found.
	// Search is made from the start of the table.
	// Use the column values from the function pool and set these to null if not found.
	//
	// Set CRPX to the found row, in case the position is required (eg. to set the CRP after sort).
	//

	uint i ;

	vector<string> keys ;
	vector<vector<string>*>::iterator it ;

	keys.reserve( num_keys ) ;

	for ( auto pt = tab_keys2.begin() ; pt != tab_keys2.end() ; ++pt )
	{
		keys.push_back( funcPool->get2( err, 8, *pt ) ) ;
		if ( err.RC8() )
		{
			funcPool->put2( err, *pt, "" ) ;
		}
		if ( err.error() ) { return table.end() ; }
	}

	CRPX = 1 ;
	for ( it = table.begin() ; it != table.end() ; ++it )
	{
		for ( i = 0 ; i < num_keys ; ++i )
		{
			if ( (*it)->at( i+2 ) != keys.at( i ) )  { break ; }
		}
		if ( i == num_keys ) { return it ; }
		++CRPX ;
	}

	return table.end() ;
}


void Table::loadfuncPool( errblock& err,
			  fPOOL* funcPool,
			  const string& tb_save_name )
{
	//
	// Set row variables (including extension variables) in function pool from row pointed to by the CRP.
	//

	uint i  ;
	uint ws ;

	string names = "" ;

	vector<vector<string>*>::iterator it ;
	vector<string>::iterator pt ;

	it = table.begin() + CRP - 1 ;

	for ( i = 2, pt = tab_all2.begin() ; pt != tab_all2.end() ; ++pt, ++i )
	{
		funcPool->put2( err, *pt, (*it)->at( i ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( has_extvars( *it ) )
	{
		const string& tbelst = get_extvar_list( *it ) ;
		for ( ws = words( tbelst ), i = 1 ; i <= ws ; ++i )
		{
			funcPool->put2( err, word( tbelst, i ), (*it)->at( num_all+2+i ) ) ;
			if ( err.error() ) { return ; }
		}
		names = "("+ tbelst +")" ;
	}

	if ( tb_save_name != "" )
	{
		funcPool->put2( err, tb_save_name, names ) ;
	}
}




void Table::storeExtensionVarNames( errblock& err,
				    fPOOL* funcPool,
				    const string& tb_save_name )
{
	//
	// Store extension variable names to tb_save_name function pool variable.  (Null if there are none).
	// For use when NOREAD specified with tb_save_name, otherwise it is set by loadfuncPool().
	//

	vector<vector<string>*>::iterator it = table.begin() + CRP - 1 ;

	if ( has_extvars( *it ) )
	{
		funcPool->put2( err, tb_save_name, "("+ get_extvar_list( *it ) +")" ) ;
	}
	else
	{
		funcPool->put2( err, tb_save_name, "" ) ;
	}
}




void Table::loadFields( errblock& err,
			fPOOL* funcPool,
			const string& tb_namelst,
			vector<string>* row )
{
	//
	// Load row fields (including namelst but not the URID) from the function pool into string vector row.
	//

	uint i  ;
	uint ws ;

	string var ;

	for ( auto& f : tab_all2 )
	{
		row->push_back( funcPool->get2( err, 8, f ) ) ;
		if ( err.RC8() )
		{
			funcPool->put2( err, f, "" ) ;
		}
		if ( err.error() ) { return ; }
	}

	if ( tb_namelst != "" )
	{
		row->push_back( space( tb_namelst ) ) ;
		for ( ws = words( tb_namelst ), i = 1 ; i <= ws ; ++i )
		{
			var = word( tb_namelst, i ) ;
			row->push_back( funcPool->get1( err, 8, var ) ) ;
			if ( err.RC8() )
			{
				funcPool->put2( err, var, "" ) ;
			}
			if ( err.error() ) { return ; }
		}
	}
}


void Table::storeFields( errblock& err,
			 fPOOL* funcPool,
			 const string& tb_save_name,
			 const string& tb_rowid_name,
			 const string& tb_noread,
			 const string& tb_crp_name )
{
	//
	// If requested, store the table fields to the function pool.
	//
	// Also (if requested):
	//   1) store extention variables to tb_save_name.
	//   2) Create/store row-id for row if necessary.
	//   3) store CRP to tb_crp_name.
	//

	uint rid  ;
	uint urid ;

	if ( tb_noread != "NOREAD" )
	{
		loadfuncPool( err,
			      funcPool,
			      tb_save_name ) ;
		if ( err.error() ) { return ; }
	}
	else if ( tb_save_name != "" )
	{
		storeExtensionVarNames( err,
					funcPool,
					tb_save_name ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_rowid_name != "" )
	{
		urid    = ds2d( table.at( CRP-1 )->at( 0 ) ) ;
		auto it = urid2rid.find( urid ) ;
		if ( it == urid2rid.end() )
		{
			rid = ++max_rid ;
			add_rid( urid, rid ) ;
		}
		else
		{
			rid = it->second ;
		}
		storeIntValue( err,
			       funcPool,
			       tb_rowid_name,
			       rid ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_crp_name != "" )
	{
		storeIntValue( err,
			       funcPool,
			       tb_crp_name,
			       CRP ) ;
		if ( err.error() ) { return ; }
	}
}


void Table::storeIntValue( errblock& err,
			   fPOOL* funcPool,
			   const string& var,
			   int val,
			   int len )
{
	//
	// Store an integer value in the function pool.  If the entry has not been vdefined as an integer,
	// convert to a string and pad on the left with zeroes, length len.
	//

	fVAR* pvar = funcPool->getfVAR( err, var ) ;
	if ( err.error() ) { return ; }

	if ( pvar )
	{
		pvar->put( val, len ) ;
	}
	else
	{
		funcPool->put2( err, var, d2ds( val, len ) ) ;
	}
}


void Table::add_rid( uint urid,
		     uint rid )
{
	urid2rid[ urid ] = rid ;
	rid2urid[ rid ]  = urid ;
}


void Table::del_rid( vector<vector<string>*>::iterator it )
{
	auto itr = urid2rid.find( ds2d( (*it)->at( 0 ) ) ) ;

	if ( itr != urid2rid.end() )
	{
		rid2urid.erase( itr->second ) ;
		urid2rid.erase( itr ) ;
	}
}


bool Table::tableClosedforTask( const errblock& err )
{
	return ( openTasks.count( err.ptid ) == 0 ) ;
}


bool Table::tableOpenedforTask( const errblock& err )
{
	return ( openTasks.count( err.ptid ) > 0 ) ;
}


void Table::addTasktoTable( const errblock& err )
{
	auto it = openTasks.find( err.ptid ) ;
	if ( it == openTasks.end() )
	{
		openTasks.insert( make_pair( err.ptid, 1 ) ) ;
	}
	else
	{
		++it->second ;
	}
}


void Table::removeTaskUsefromTable( errblock& err )
{
	auto it = openTasks.find( err.ptid ) ;
	if ( it->second == 1 )
	{
		openTasks.erase( it ) ;
	}
	else
	{
		--it->second ;
	}
}


void Table::removeTaskfromTable( errblock& err )
{
	openTasks.erase( err.ptid ) ;
}


bool Table::notInUse()
{
	return ( openTasks.size() == 0 ) ;
}


void Table::set_path( const string& f )
{
	tab_ipath = f.substr( 0, f.find_last_of( '/' ) ) ;
}


string Table::listTasks()
{
	auto it  = openTasks.begin() ;
	string t = d2ds( it->first ) ;

	for ( ++it ; it != openTasks.end() ; ++it )
	{
		t += "," + d2ds( it->first ) ;
	}

	return t ;
}


void Table::tbadd( errblock& err,
		   fPOOL* funcPool,
		   const string& tb_namelst,
		   const string& tb_order,
		   int tb_num_of_rows )
{
	//
	// Add a row to a table after the CRP (if not sorted) or in the sort position of a sorted table if
	// ORDER has been specified.
	//
	// RC =  0  Okay.
	// RC =  8  Keyed tables only.  Row already exists, CRP set to TOP.
	// RC = 16  Numeric conversion error.
	// RC = 20  Severe error.
	//
	// Add extension variables as varname list + values to the row vector after
	// defined keys and fields (start posn at (all_flds + 1) ).
	//
	// If ORDER specified on a sorted table, sort the table again after adding the record and reset CRP.
	//
	// If the table has no fields, tb_namelst must be specified.
	//

	string prefix( "\x00\x00\x00\x00", 4 ) ;

	string URID ;

	vector<string>* row ;
	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	if ( table.size() >= MXTAB_SZ )
	{
		err.seterrid( TRACE_INFO(), "PSYE013F" ) ;
		return ;
	}

	if ( tb_num_of_rows > 0 )
	{
		if ( firstMult || table.capacity() == table.size() )
		{
			table.reserve( table.size() + tb_num_of_rows ) ;
			firstMult = false ;
		}
	}

	if ( tb_order == "" ) { sort_ir = "" ; }

	if ( num_keys > 0 )
	{
		it = getKeyItr( err, funcPool ) ;
		if ( err.error() ) { return ; }
		if ( it != table.end() )
		{
			CRP = 0 ;
			err.setRC( 8 ) ;
			return ;
		}
	}

	if ( num_all == 0 && tb_namelst == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013K", "TBADD" ) ;
		return ;
	}

	row = new vector<string> ;
	row->clear() ;
	row->push_back( d2ds( ++max_urid ) ) ;
	prefix[ 3 ] |= ( pr_create ) ? _CREATE : _INSERT ;
	row->push_back( prefix ) ;

	loadFields( err,
		    funcPool,
		    tb_namelst,
		    row ) ;
	if ( err.error() ) { delete row ; return ; }

	it = table.insert( table.begin() + CRP, row ) ;
	++CRP ;

	if ( tb_order == "ORDER" && sort_ir != "" )
	{
		URID = (*it)->at( 0 ) ;
		tbsort( err, sort_ir ) ;
		if ( err.error() ) { return ; }
		for ( it = table.begin() ; it != table.end() ; ++it )
		{
			++CRP ;
			if ( URID == (*it)->at( 0 ) ) { break ; }
		}
	}

	changed = true ;
	updated = true ;

	tab_user = err.user ;

	if ( pr_create )
	{
		rowcreat = table.size() ;
	}
}


void Table::tbbottom( errblock& err,
		      fPOOL* funcPool,
		      const string& tb_save_name,
		      const string& tb_rowid_name,
		      const string& tb_noread,
		      const string& tb_crp_name )
{
	//
	// Set the CRP to the last row in the table.
	//
	// RC =  0  Okay.
	// RC =  8  Table Empty.  CRP set to top.
	// RC = 20  Severe error.
	//

	err.setRC( 0 ) ;

	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013M", "NOREAD", tb_noread ) ;
		return ;
	}

	if ( table.size() == 0 )
	{
		CRP = 0 ;
		err.setRC( 8 ) ;
		if ( tb_crp_name != "" )
		{
			storeIntValue( err, funcPool, tb_crp_name, CRP ) ;
		}
		return ;
	}

	CRP = table.size() ;

	storeFields( err,
		     funcPool,
		     tb_save_name,
		     tb_rowid_name,
		     tb_noread,
		     tb_crp_name ) ;
}


void Table::tbdelete( errblock& err,
		      fPOOL* funcPool )
{
	//
	// Delete a row in the table.
	//
	// Non-keyed tables - this is the row pointed to by the CRP.
	// Keyed tables     - this is the row pointed to by the current contents of the key variables.
	//
	// After deletion, the CRP always points to the row before the one deleted.
	//
	// RC =  0  Okay.
	// RC =  8  Keyed tables.  Row with key value does not exist.  CRP set to TOP.
	//          Non-keyed tables.  CRP was at TOP (zero).
	// RC = 20  Severe error.
	//

	err.setRC( 0 ) ;

	vector<vector<string>*>::iterator it ;

	if ( num_keys > 0 )
	{
		it = getKeyItr( err, funcPool ) ;
		if ( err.error() ) { return ; }
		if ( it == table.end() )
		{
			CRP = 0 ;
			err.setRC( 8 ) ;
			return ;
		}
		del_rid( it ) ;
		delete *it ;
		table.erase( it ) ;
		CRP = CRPX - 1    ;
		changed  = true ;
		updated  = true ;
		tab_user = err.user ;
	}
	else if ( CRP == 0 )
	{
		err.setRC( 8 ) ;
	}
	else
	{
		--CRP ;
		it = table.begin() + CRP ;
		del_rid( it ) ;
		delete *it  ;
		table.erase( it ) ;
		changed  = true ;
		updated  = true ;
		tab_user = err.user ;
	}

	if ( tab_klst )
	{
		++pflgToken ;
	}
}


void Table::tbexist( errblock& err,
		     fPOOL* funcPool )
{
	//
	// Test for the existance of a row.
	//
	// Non-keyed tables - call not valid.
	// Keyed tables     - use the current value of the key variables.
	//
	// CRP is positioned at this row if found else TOP.
	//
	// RC =  0  Okay.
	// RC =  8  Keyed tables.  Row does not exist.  CRP is set to TOP (zero).
	//          Non-keyed tables.  Not valid.  CRP is set to TOP (zero).
	// RC = 20  Severe error.
	//

	err.setRC( 0 ) ;

	CRP = 0 ;

	if ( num_keys == 0 )
	{
		err.setRC( 8 ) ;
		return ;
	}

	if ( getKeyItr( err, funcPool ) == table.end() )
	{
		if ( err.ok() ) { err.setRC( 8 ) ; }
		return ;
	}
	CRP = CRPX ;
}


void Table::tbget( errblock& err,
		   fPOOL* funcPool,
		   const string& tb_save_name,
		   const string& tb_rowid_name,
		   const string& tb_noread,
		   const string& tb_crp_name  )
{
	//
	// Access row in the table.
	//
	// Non-keyed tables - use the CRP.
	// Keyed tables     - use the current value of the key in the dialogue variable.
	//
	// RC =  0  Okay.
	// RC =  8  CRP was at TOP(zero) for non-keyed tables or key not found for keyed tables.
	// RC = 20  Severe error.
	//
	// ROWID name and CRP name are for output only (not used for finding the record).
	//
	// Note:  According to ISPF Services Guide for z/OS 2.3 and later, for keyed tables the search is made
	//        starting after the current row pointer (This is not in the service description but in the explanation of RC = 8).
	//        Manuals before this z/OS release do not state this.
	//        This implementation starts the search from the top but if anyone has a definitive answer to this, let me know.
	//

	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013M", "NOREAD", tb_noread ) ;
		return ;
	}

	if ( num_keys > 0 )
	{
		it = getKeyItr( err, funcPool ) ;
		if ( err.error() ) { return ; }
		CRP = ( it == table.end() ) ? 0 : CRPX ;
	}

	if ( CRP == 0 )
	{
		if ( tb_crp_name != "" )
		{
			storeIntValue( err, funcPool, tb_crp_name, CRP ) ;
			if ( err.error() ) { return ; }
		}
		err.setRC( 8 ) ;
		return ;
	}

	storeFields( err,
		     funcPool,
		     tb_save_name,
		     tb_rowid_name,
		     tb_noread,
		     tb_crp_name ) ;
}


void Table::tbmod( errblock& err,
		   fPOOL* funcPool,
		   const string& tb_namelst,
		   const string& tb_order )
{
	//
	// Unconditionally update the current row in a table.
	//
	// Non-keyed tables - always same as a tbadd.
	// Keyed tables     - update row if match found on keys, else perform tbadd at the bottom of the table.
	//
	// CRP always points to the row added/updated.
	//
	// RC =  0  Okay.  Keyed tables - row updated.  Non-keyed tables new row added.
	// RC =  8  Row did not match - row added for keyed tables.
	// RC = 16  Numeric conversion error.
	// RC = 20  Severe error.
	//
	// Extension variables must be re-specified or they will be lost (tb_namelst).
	//
	// If ORDER specified on a sorted table, sort again in case tbmod has changed the order and reset CRP.
	//

	string URID ;

	vector<string>* row ;
	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	if ( tb_order == "" ) { sort_ir = "" ; }

	if ( num_all == 0 && tb_namelst == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013K", "TBMOD" ) ;
		return ;
	}

	if ( num_keys == 0 )
	{
		tbadd( err, funcPool, tb_namelst, tb_order, 0 ) ;
		return ;
	}

	it = getKeyItr( err, funcPool ) ;
	if ( err.error() ) { return ; }
	if ( it == table.end() )
	{
		CRP = table.size() ;
		tbadd( err, funcPool, tb_namelst, tb_order, 0 ) ;
		if ( err.error() ) { return ; }
		if ( err.RC0() )
		{
			err.setRC( 8 ) ;
		}
	}
	else
	{
		row  = new vector<string> ;
		CRP  = CRPX ;
		URID = (*it)->at( 0 ) ;
		row->push_back( URID ) ;
		row->push_back( (*it)->at( 1 ) ) ;
		row->at( 1 )[ 3 ] |= _UPDATE ;
		loadFields( err,
			    funcPool,
			    tb_namelst,
			    row ) ;
		if ( err.error() ) { delete row ; return ; }
		delete *it ;
		(*it) = row  ;
		if ( tb_order == "ORDER" && sort_ir != "" )
		{
			tbsort( err, sort_ir ) ;
			if ( err.error() ) { return ; }
			for ( it = table.begin() ; it != table.end() ; ++it )
			{
				++CRP ;
				if ( URID == (*it)->at( 0 ) ) { break ; }
			}
		}
	}

	changed  = true ;
	updated  = true ;
	tab_user = err.user ;

	if ( tab_klst )
	{
		++pflgToken ;
	}
}


void Table::tbput( errblock& err,
		   fPOOL* funcPool,
		   const string& tb_namelst,
		   const string& tb_order )
{
	//
	// Update the current row in a table.
	//
	// Non-keyed tables - use row pointed to by the CRP.
	// Keyed tables     - key variables must match row at CRP.
	//
	// RC =  0  OK.
	// RC =  8  Keyed tables - keys do not match current row or CRP was set to top.  CRP set to top (0).
	//          Non-keyed tables - CRP was at top.
	// RC = 16  Numeric conversion error for sorted tables.
	// RC = 20  Severe error.
	//
	// If ORDER specified on a sorted table, sort again in case tbput has changed the order and reset CRP.
	//

	string URID ;

	err.setRC( 0 ) ;

	vector<string>* row ;
	vector<vector<string>*>::iterator it ;

	if ( tb_order == "" ) { sort_ir = "" ; }

	if ( CRP == 0 )
	{
		err.setRC( 8 ) ;
		return ;
	}

	if ( num_all == 0 && tb_namelst == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013K", "TBPUT" ) ;
		return ;
	}

	it = table.begin() + CRP - 1 ;

	row = new vector<string> ;

	URID = (*it)->at( 0 ) ;
	row->push_back( URID ) ;
	row->push_back( (*it)->at( 1 ) ) ;
	row->at( 1 )[ 3 ] |= _UPDATE ;

	loadFields( err,
		    funcPool,
		    tb_namelst,
		    row ) ;
	if ( err.error() )
	{
		delete row ;
		return ;
	}

	for ( uint i = 2 ; i <= ( num_keys + 1 ) ; ++i )
	{
		if ( row->at( i ) != (*it)->at( i ) )
		{
			CRP = 0 ;
			err.setRC( 8 ) ;
			delete row ;
			return ;
		}
	}

	delete *it ;
	(*it) = row ;

	if ( tb_order == "ORDER" && sort_ir != "" )
	{
		tbsort( err, sort_ir ) ;
		if ( err.error() ) { return ; }
		for ( it = table.begin() ; it != table.end() ; ++it )
		{
			++CRP ;
			if ( URID == (*it)->at( 0 ) ) { break ; }
		}
	}

	changed  = true ;
	updated  = true ;
	tab_user = err.user ;

	if ( tab_klst )
	{
		++pflgToken ;
	}
}


void Table::tbquery( errblock& err,
		     fPOOL* funcPool,
		     const string& tb_keyn,
		     const string& tb_varn,
		     const string& tb_rownn,
		     const string& tb_keynn,
		     const string& tb_namenn,
		     const string& tb_crpn,
		     const string& tb_sirn,
		     const string& tb_lstn,
		     const string& tb_condn,
		     const string& tb_dirn )
{
	//
	// Return information about an open table.
	//

	err.setRC( 0 ) ;

	if ( tb_keyn != "" )
	{
		funcPool->put2( err, tb_keyn, ( tab_keys1 == "" ) ? "" : "("+ tab_keys1 +")" ) ;
		if ( err.error() ) { return ; }
	}
	if ( tb_varn != "" )
	{
		funcPool->put2( err, tb_varn, ( tab_flds == "" ) ? "" : "("+ tab_flds +")" ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_rownn != "" ) { storeIntValue( err, funcPool, tb_rownn, table.size() ) ; }
	if ( err.error() ) { return ; }

	if ( tb_keynn != "" ) { storeIntValue( err, funcPool, tb_keynn, num_keys ) ; }
	if ( err.error() ) { return ; }

	if ( tb_namenn != "") { storeIntValue( err, funcPool, tb_namenn, num_flds ) ; }
	if ( err.error() ) { return ; }

	if ( tb_crpn != "" )  { storeIntValue( err, funcPool, tb_crpn, CRP ) ; }
	if ( err.error() ) { return ; }

	if ( tb_sirn != "" ) { funcPool->put2( err, tb_sirn, sort_ir ) ; }
	if ( err.error() ) { return ; }

	if ( tb_lstn != "" ) { funcPool->put2( err, tb_lstn, sa_namelst ) ; }
	if ( err.error() ) { return ; }

	if ( tb_condn != "" ) { funcPool->put2( err, tb_condn, sa_cond_pairs ) ; }
	if ( err.error() ) { return ; }

	if ( tb_dirn != "" ) { funcPool->put2( err, tb_dirn, sa_dir ) ; }
	if ( err.error() ) { return ; }
}


void Table::tbquery( bool& cpairs )
{
	//
	// Return cpairs true if table has condition pairs.
	// For use with file tailoring.
	//
	// Internal use only.
	//

	cpairs = ( strip( sa_cond_pairs ) != "" ) ;
}


void Table::tbquery( uint& lcrp )
{
	//
	// Return the CRP ;
	//
	// Internal use only.
	//

	lcrp = CRP ;
}


void Table::tbsarg( errblock& err,
		    fPOOL* funcPool,
		    string tb_namelst,
		    const string& tb_dir,
		    string tb_cond_pairs )
{
	//
	// Establish a search argument for a table to be used with TBSCAN.
	//
	// Use non-null column values to create the search argument.
	// Extension variables need to be specified in namelist.
	// Cond pairs specify the conditions to use for the paired columns (default EQ).
	//
	// RC =  0  Okay. Search arguments set.
	// RC =  8  No search arguments set (all column variables null and namelst not specified).
	// RC = 20  Severe error.
	//

	int i ;

	string val ;
	string nl_namelst ;
	string nl_cond_pairs ;

	vector<string>::iterator pt ;

	set<string>names ;

	err.setRC( 0 ) ;

	if ( tb_dir != "NEXT" && tb_dir != "PREVIOUS" && tb_dir != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013P", tb_dir ) ;
		return ;
	}

	nl_namelst = getNameList( err, tb_namelst ) ;
	if ( err.error() ) { return ; }

	nl_cond_pairs = getNameList( err, tb_cond_pairs ) ;
	if ( err.error() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013O", tb_cond_pairs ) ;
		return ;
	}

	sarg.clear() ;
	for ( i = 2, pt = tab_all2.begin() ; pt != tab_all2.end() ; ++pt, ++i )
	{
		val = funcPool->get2( err, 8, *pt ) ;
		if ( err.error() ) { return ; }
		if ( val != "" )
		{
			sarg.push_back( tbsearch( *pt, val, i ) ) ;
		}
		names.insert( *pt ) ;
	}

	setscan( err,
		 funcPool,
		 sarg,
		 names,
		 nl_namelst,
		 nl_cond_pairs ) ;
	if ( err.error() ) { return ; }

	if ( sarg.size() == 0 )
	{
		err.setRC( 8 ) ;
		return ;
	}

	sa_dir        = ( tb_dir == "" ) ? "NEXT" : tb_dir ;
	sa_namelst    = tb_namelst ;
	sa_cond_pairs = tb_cond_pairs ;
}


void Table::setscan( errblock& err,
		     fPOOL* funcPool,
		     vector<tbsearch>& scan,
		     set<string>& names,
		     const string& nl_namelst,
		     const string& nl_cond_pairs )
{
	//
	// Setup the search arguments in the scan vector for TBSCAN and TBSARG.
	//
	// Notes:
	// TBSARG: Current value of all table variables (already loaded into the scan vector).
	//         nl_namelst is for extension variables, the values of which will be used in the search.
	//         Default condition is EQ.  Ignore nulls except for extension variables.
	//
	// TBSCAN: Only use variables from nl_namelst (any type).  Don't ignore nulls.
	//

	uint i  ;
	uint j  ;
	uint ws ;

	string var  ;
	string val  ;
	string cond ;

	vector<tbsearch>::iterator it ;
	vector<string>::iterator pt ;

	err.setRC( 0 ) ;

	for ( ws = words( nl_namelst ), i = 1 ; i <= ws ; ++i )
	{
		var = word( nl_namelst, i ) ;
		if ( names.count( var ) > 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE013Y", var ) ;
			return ;
		}
		val = funcPool->get1( err, 8, var ) ;
		if ( err.error() ) { return ; }
		if ( err.RC8() )
		{
			funcPool->put2( err, var, "" ) ;
			if ( err.error() ) { return ; }
		}
		for ( j = 1, pt = tab_all2.begin() ; pt != tab_all2.end() ; ++pt, ++j )
		{
			if ( var == *pt ) { break ; }
		}
		scan.push_back( tbsearch( var, val, ( j > tab_all2.size() ? -1 : j+1 ) ) ) ;
		names.insert( var ) ;
	}

	ws = words( nl_cond_pairs ) ;
	if ( ws % 2 == 1 )
	{
		err.seterrid( TRACE_INFO(), "PSYE013R" ) ;
		return ;
	}

	for ( i = 1 ; i <= ws ; i += 2 )
	{
		var  = word( nl_cond_pairs, i ) ;
		cond = word( nl_cond_pairs, i+1 ) ;
		if ( !findword( var, tab_all1 ) && !findword( var, nl_namelst ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE013S", var ) ;
			return ;
		}
		it = find_if( scan.begin(), scan.end(),
			[ &var ]( const tbsearch& tbs )
			{
				return ( var == tbs.tbs_var ) ;
			} ) ;
		if ( it != scan.end() && !it->set_condition( cond ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE013T", cond ) ;
			return ;
		}
	}
}


void Table::tbscan( errblock& err,
		    fPOOL* funcPool,
		    int zswind,
		    string tb_namelst,
		    const string& tb_save_name,
		    const string& tb_rowid_name,
		    const string& tb_dir,
		    const string& tb_noread,
		    const string& tb_crp_name,
		    string tb_condlst )
{
	//
	// Scan table according to parameters tb_namelst/tb_condlst/tb_dir if specified or the
	// search parameters set by a previous TBSARG call.
	//
	// For a forward search  - Start from the row after the current CRP to the end of the table.
	// For a backward search - Start from the row before the current CRP to the top of the table.
	//                         If CRP at top, start with the last row to the top of the table.
	//
	// tb_condlst contains the condidtions to use for variables in tb_namelst (1:1 between the two lists).
	// Only use variables in tb_namelst - not other table variables.
	//
	// Note:  Only character comparison is performed.  When comparing interger numeric values, left pad with '0'
	//        to the maximum size required to get the desired results.
	//
	// RC =  0  Okay.  CRP set to row found.
	// RC =  8  Row not found.  CRP set to top (zero).
	// RC = 20  Severe error.
	//

	int p1 ;
	int yy ;

	uint i ;
	uint ws ;
	uint size ;
	uint s_match ;

	bool s_next ;
	bool found  ;
	bool end_loop ;

	string s_dir = sa_dir ;

	string var ;
	string cond ;

	string date1 ;
	string date2 ;

	string nl_namelst ;
	string nl_condlst ;
	string nl_cond_pairs ;

	string* compstr1 ;
	string* compstr2 ;

	vector<tbsearch> scan ;
	vector<tbsearch>* pscan ;
	vector<tbsearch>::iterator it ;

	set<string>names ;

	err.setRC( 0 ) ;

	if ( tb_dir != "NEXT" && tb_dir != "PREVIOUS" && tb_dir != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013P", tb_dir ) ;
		return ;
	}

	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013M", "NOREAD", tb_noread ) ;
		return ;
	}

	nl_condlst = getNameList( err, tb_condlst ) ;
	if ( err.error() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013Q", tb_condlst ) ;
		return ;
	}

	nl_namelst = getNameList( err, tb_namelst ) ;
	if ( err.error() ) { return ; }

	if ( tb_dir != "" )
	{
		if ( nl_namelst == "" )
		{
			sa_dir = tb_dir ;
		}
		s_dir = tb_dir ;
	}
	else if ( nl_namelst != "" )
	{
		s_dir = "NEXT" ;
	}

	if ( nl_namelst == "" )
	{
		if ( sarg.size() == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE013U", tab_name ) ;
			return ;
		}
		pscan = &sarg ;
	}
	else
	{
		ws = words( nl_namelst ) ;
		if ( words( nl_condlst ) > ws )
		{
			err.seterrid( TRACE_INFO(), "PSYE014C" ) ;
			return ;
		}
		nl_cond_pairs = "" ;
		for ( i = 1 ; i <= ws ; ++i )
		{
			var  = word( nl_namelst, i ) ;
			cond = word( nl_condlst, i ) ;
			if ( cond == "" ) { cond = "EQ" ; }
			nl_cond_pairs += var + " " + cond + " " ;
		}
		scan.clear() ;
		setscan( err, funcPool, scan, names, nl_namelst, nl_cond_pairs ) ;
		if ( err.error() ) { return ; }
		pscan = &scan ;
	}

	found  = false ;
	s_next = ( s_dir == "NEXT" ) ;
	size   = table.size() ;

	while ( size > 0 )
	{
		if ( s_next )
		{
			++CRP ;
			if ( CRP > size ) { break ; }
		}
		else if ( CRP == 0 )
		{
			CRP = size ;
		}
		else
		{
			--CRP ;
			if ( CRP == 0 ) { break ; }
		}
		s_match = 0 ;
		for ( it = pscan->begin() ; it != pscan->end() ; ++it )
		{
			p1 = it->tbs_position ;
			if ( p1 == -1 )
			{
				if ( !has_extvars( CRP-1 ) ) { break ; }
				p1 = get_extvar_pos( CRP-1, it->tbs_var ) ;
				if ( p1 == 0 ) { break ; }
				p1 += num_all + 2 ;
			}
			if ( it->tbs_y > 0 )
			{
				date1 = table.at( CRP-1 )->at( p1 ) ;
				date2 = it->tbs_val ;
				if ( date1.size() > it->tbs_y )
				{
					yy = ds2d( date1.substr( it->tbs_y - 1, 2 ) ) ;
					date1.insert( it->tbs_y - 1, ( yy <= zswind ) ? "20" : "19" ) ;
				}
				if ( date2.size() > it->tbs_y )
				{
					yy = ds2d( date2.substr( it->tbs_y - 1, 2 ) ) ;
					date2.insert( it->tbs_y - 1, ( yy <= zswind ) ? "20" : "19" ) ;
				}
				compstr1 = &date1 ;
				compstr2 = &date2 ;
			}
			else
			{
				compstr1 = &table.at( CRP-1 )->at( p1 ) ;
				compstr2 = &it->tbs_val ;
			}
			if ( it->tbs_generic )
			{
				switch ( it->tbs_condition )
				{
				case S_EQ:
					end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) != 0 ) ;
					break ;

				case S_NE:
					end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) == 0 ) ;
					break ;

				case S_LE:
					end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) > 0 ) ;
					break ;

				case S_LT:
					end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) >= 0 ) ;
					break ;

				case S_GE:
					end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) < 0 ) ;
					break ;

				case S_GT:
					end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) <= 0 ) ;
					break ;
				}
			}
			else
			{
				switch ( it->tbs_condition )
				{
				case S_EQ:
					end_loop = ( *compstr1 != *compstr2 ) ;
					break ;

				case S_NE:
					end_loop = ( *compstr1 == *compstr2 ) ;
					break ;

				case S_LE:
					end_loop = ( *compstr1 > *compstr2 ) ;
					break ;

				case S_LT:
					end_loop = ( *compstr1 >= *compstr2 ) ;
					break ;

				case S_GE:
					end_loop = ( *compstr1 < *compstr2 ) ;
					break ;

				case S_GT:
					end_loop = ( *compstr1 <= *compstr2 ) ;
					break ;
				}
			}
			if ( end_loop ) { break ; }
			++s_match ;
		}
		if ( s_match == pscan->size() )
		{
			found = true ;
			break ;
		}
	}

	if ( !found )
	{
		CRP = 0 ;
		if ( tb_crp_name != "" )
		{
			storeIntValue( err, funcPool, tb_crp_name, CRP ) ;
			if ( err.error() ) { return ; }
		}
		err.setRC( 8 ) ;
		return ;
	}

	storeFields( err,
		     funcPool,
		     tb_save_name,
		     tb_rowid_name,
		     tb_noread,
		     tb_crp_name ) ;
}


void Table::tbskip( errblock& err,
		    fPOOL* funcPool,
		    int num,
		    const string& tb_save_name,
		    const string& tb_rowid_name,
		    const string& tb_row,
		    const string& tb_noread,
		    const string& tb_crp_name )
{
	//
	// Move CRP to a position in the table and read the row into the dialogue variables.
	// Position using tb_row if specified, then use num.
	//
	// RC =  0  Okay.
	// RC =  8  CRP would be outside the table or ROWID not found. CRP set to top (zero).
	// RC = 16  Truncation has occured.
	// RC = 20  Severe error.
	//

	string s_rid ;

	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( TRACE_INFO(), "PSYE013M", "NOREAD", tb_noread ) ;
		return ;
	}

	if ( tb_row != "" )
	{
		if ( tb_row.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "ROW" ) ;
			return ;
		}
		if ( !datatype( tb_row, 'W') )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "ROW" ) ;
			return ;
		}
		auto itr = rid2urid.find( ds2d( tb_row ) ) ;
		if ( itr == rid2urid.end() )
		{
			CRP = 0 ;
			if ( tb_crp_name != "" )
			{
				storeIntValue( err, funcPool, tb_crp_name, CRP ) ;
				if ( err.error() ) { return ; }
			}
			err.setRC( 8 ) ;
			return ;
		}
		s_rid = d2ds( itr->second ) ;
		CRP   = 1 ;
		for ( it = table.begin() ; it != table.end() ; ++it )
		{
			if ( s_rid == (*it)->at( 0 ) ) { break ; }
			++CRP ;
		}
		if ( it == table.end() )
		{
			CRP = 0 ;
			if ( tb_crp_name != "" )
			{
				storeIntValue( err, funcPool, tb_crp_name, CRP ) ;
				if ( err.error() ) { return ; }
			}
			err.setRC( 8 ) ;
			return ;
		}
	}

	if ( num != 0 )
	{
		if ( ( ( CRP + num ) < 1 ) || ( ( CRP + num ) > table.size() ) )
		{
			CRP = 0 ;
			if ( tb_crp_name != "" )
			{
				storeIntValue( err, funcPool, tb_crp_name, CRP ) ;
				if ( err.error() ) { return ; }
			}
			err.setRC( 8 ) ;
			return ;
		}
		CRP += num ;
	}
	else if ( CRP == 0 )
	{
		if ( tb_crp_name != "" )
		{
			storeIntValue( err, funcPool, tb_crp_name, CRP ) ;
			if ( err.error() ) { return ; }
		}
		err.setRC( 8 ) ;
		return ;
	}

	storeFields( err,
		     funcPool,
		     tb_save_name,
		     tb_rowid_name,
		     tb_noread,
		     tb_crp_name ) ;
}


void Table::tbskip( errblock& err,
		    fPOOL* funcPool,
		    const string& tb_rowid_name,
		    const string& tb_urid,
		    const string& tb_noread,
		    const string& tb_crp_name )
{
	//
	// Move CRP to a position in the table and read the row into the dialogue variables.
	// Position using tb_urid.
	//
	// Internal use only.
	//
	// RC =  0  Okay.
	// RC =  8  URID not found. CRP set to top (zero).
	// RC = 20  Severe error.
	//

	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	CRP = 1 ;
	for ( it = table.begin() ; it != table.end() ; ++it )
	{
		if ( tb_urid == (*it)->at( 0 ) ) { break ; }
		++CRP ;
	}
	if ( it == table.end() )
	{
		CRP = 0 ;
		if ( tb_crp_name != "" )
		{
			storeIntValue( err, funcPool, tb_crp_name, CRP ) ;
			if ( err.error() ) { return ; }
		}
		err.setRC( 8 ) ;
		return ;
	}

	storeFields( err,
		     funcPool,
		     "",
		     tb_rowid_name,
		     tb_noread,
		     tb_crp_name ) ;
}


void Table::tbsort( errblock& err,
		    string tb_fields )
{
	//
	// Sort table into a user-specified order.
	//
	// FIELD
	// FIELD,C
	// FIELD,C,A,FIELD2
	// FIELD,C,A,FIELD2,N
	// FIELD,C,A,FIELD2,N,D
	//

	int f1 ;

	size_t i ;

	vector<string> s_parm ;
	vector<int> s_field ;
	vector<bool> s_char ;
	vector<bool> s_asc ;

	err.setRC( 0 ) ;

	replace( tb_fields.begin(), tb_fields.end(), ',', ' ' ) ;

	word( tb_fields, s_parm ) ;

	for ( i = 0 ; i < s_parm.size() ; ++i )
	{
		string& s_temp = s_parm[ i ] ;
		switch ( i%3 )
		{
		case 0:
			f1 = wordpos( s_temp, tab_all1 ) + 1 ;
			if ( f1 == 1 )
			{
				err.seterrid( TRACE_INFO(), "PSYE013X", s_temp ) ;
				return ;
			}
			if ( find( s_field.begin(), s_field.end(), f1 ) != s_field.end() )
			{
				err.seterrid( TRACE_INFO(), "PSYE013Y", s_temp ) ;
				return ;
			}
			s_field.push_back( f1 ) ;
			break ;

		case 1:
			if ( s_temp != "C" && s_temp != "N" )
			{
				err.seterrid( TRACE_INFO(), "PSYE014A", s_temp ) ;
				return ;
			}
			s_char.push_back( ( s_temp == "C" ) ) ;
			break ;

		case 2:
			if ( s_temp != "A" && s_temp != "D" )
			{
				err.seterrid( TRACE_INFO(), "PSYE014B", s_temp ) ;
				return ;
			}
			s_asc.push_back( ( s_temp == "A" ) ) ;
			break ;
		}
	}

	s_char.push_back( true ) ;
	s_asc.push_back( true ) ;

	stable_sort( table.begin(), table.end(),
		[ &s_field, &s_char, &s_asc ]( const vector<string>* a, const vector<string>* b )
		{
			for ( size_t j = 0 ; j < s_field.size() ; ++j )
			{
				int k = s_field[ j ] ;
				if ( s_char[ j ] )
				{
					 if ( a->at( k ) == b->at( k ) ) { continue ; }
					 return ( s_asc[ j ] ) ? a->at( k ) < b->at( k ) :
								 a->at( k ) > b->at( k ) ;
				}
				else
				{
					 int ia = ds2d( a->at( k ) ) ;
					 int ib = ds2d( b->at( k ) ) ;
					 if ( ia == ib ) { continue ; }
					 return ( s_asc[ j ] ) ? ia < ib :
								 ia > ib ;
				}
			}
			return false ;
		} ) ;

	sort_ir = space( tb_fields ) ;
	replace( sort_ir.begin(), sort_ir.end(), ' ', ',' ) ;

	CRP = 0 ;
	changed  = true ;
	tab_user = err.user ;
}


void Table::tbstats( errblock& err,
		     fPOOL* funcPool,
		     const string& tb_cdate,
		     const string& tb_ctime,
		     const string& tb_udate,
		     const string& tb_utime,
		     const string& tb_user,
		     const string& tb_rowcreat,
		     const string& tb_rowcurr,
		     const string& tb_rowupd,
		     const string& tb_tableupd,
		     const string& tb_service,
		     const string& tb_retcode,
		     const string& tb_status2,
		     const string& tb_virtsize,
		     const string& tb_cdate4d,
		     const string& tb_udate4d )
{
	//
	// Return table statistics.
	//
	// RC =  0  Normal completion.
	// RC = 20  Severe error.
	//

	char buf[ 12 ] ;

	struct tm* time_info = nullptr ;

	if ( tb_cdate != "" )
	{
		time_info = localtime( &ctime ) ;
		strftime( buf, sizeof( buf ), "%y/%m/%d", time_info ) ;
		funcPool->put2( err, tb_cdate, buf ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_ctime != "" )
	{
		time_info = localtime( &ctime ) ;
		strftime( buf, sizeof( buf ), "%H.%M.%S", time_info ) ;
		funcPool->put2( err, tb_ctime, buf ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_udate != "" )
	{
		if ( utime == 0 )
		{
			funcPool->put2( err, tb_udate, "NOUPDATE" ) ;
		}
		else
		{
			time_info = localtime( &utime ) ;
			strftime( buf, sizeof( buf ), "%y/%m/%d", time_info ) ;
			funcPool->put2( err, tb_udate, buf ) ;
		}
		if ( err.error() ) { return ; }
	}

	if ( tb_utime != "" )
	{
		if ( utime == 0 )
		{
			funcPool->put2( err, tb_utime, "NOUPDATE" ) ;
		}
		else
		{
			time_info = localtime( &utime ) ;
			strftime( buf, sizeof( buf ), "%H.%M.%S", time_info ) ;
			funcPool->put2( err, tb_utime, buf ) ;
		}
		if ( err.error() ) { return ; }
	}

	if ( tb_user != "" )
	{
		funcPool->put2( err, tb_user, tab_user ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_rowcreat != "" )
	{
		funcPool->put2( err, tb_rowcreat, d2ds( rowcreat, 8 ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_rowcurr != "" )
	{
		funcPool->put2( err, tb_rowcurr, d2ds( table.size(), 8 ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_rowupd != "" )
	{
		int i = 0 ;
		for ( auto r : table )
		{
			if ( r->at( 1 )[ 3 ] & _UPDATE )
			{
				++i ;
			}
		}
		funcPool->put2( err, tb_rowupd, d2ds( i, 8 ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_tableupd != "" )
	{
		funcPool->put2( err, tb_tableupd, d2ds( tableupd, 8 ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_service != "" )
	{
		switch ( tab_service )
		{
		case    TB_ADD:
			funcPool->put2( err, tb_service, "TBADD" ) ;
			break ;

		case    TB_BOTTOM:
			funcPool->put2( err, tb_service, "TBBOTTOM" ) ;
			break ;

		case    TB_CREATE:
			funcPool->put2( err, tb_service, "TBCREATE" ) ;
			break ;

		case    TB_DELETE:
			funcPool->put2( err, tb_service, "TBDELETE" ) ;
			break ;

		case    TB_EXISTS:
			funcPool->put2( err, tb_service, "TBEXISTS" ) ;
			break ;

		case    TB_GET:
			funcPool->put2( err, tb_service, "TBGET" ) ;
			break ;

		case    TB_MOD:
			funcPool->put2( err, tb_service, "TBMOD" ) ;
			break ;

		case    TB_NONE:
			funcPool->put2( err, tb_service, "" ) ;
			break ;

		case    TB_OPEN:
			funcPool->put2( err, tb_service, "TBOPEN" ) ;
			break ;

		case    TB_PUT:
			funcPool->put2( err, tb_service, "TBPUT" ) ;
			break ;

		case    TB_QUERY:
			funcPool->put2( err, tb_service, "TBQUERY" ) ;
			break ;

		case    TB_SARG:
			funcPool->put2( err, tb_service, "TBSARG" ) ;
			break ;

		case    TB_SAVE:
			funcPool->put2( err, tb_service, "TBSAVE" ) ;
			break ;

		case    TB_SCAN:
			funcPool->put2( err, tb_service, "TBSCAN" ) ;
			break ;

		case    TB_SKIP:
			funcPool->put2( err, tb_service, "TBSKIP" ) ;
			break ;

		case    TB_SORT:
			funcPool->put2( err, tb_service, "TBSORT" ) ;
			break ;

		case    TB_STATS:
			funcPool->put2( err, tb_service, "TBSTATS" ) ;
			break ;

		case    TB_TOP:
			funcPool->put2( err, tb_service, "TBTOP" ) ;
			break ;

		case    TB_VCLEAR:
			funcPool->put2( err, tb_service, "TBVCLEAR" ) ;
			break ;

		}
		if ( err.error() ) { return ; }
	}

	if ( tb_retcode != "" )
	{
		funcPool->put2( err, tb_retcode, d2ds( lastcc, 2 ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_status2 != "" )
	{
		funcPool->put2( err, tb_status2, ( tab_WRITE == NOWRITE ) ? ( tab_DISP == NON_SHARE ) ? "2" : "4" :
									    ( tab_DISP == NON_SHARE ) ? "3" : "5"  ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_virtsize != "" )
	{
		funcPool->put2( err, tb_virtsize, d2ds( get_virtsize(), 12 ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_cdate4d != "" )
	{
		time_info = localtime( &ctime ) ;
		strftime( buf, sizeof( buf ), "%Y/%m/%d", time_info ) ;
		funcPool->put2( err, tb_cdate4d, buf ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_udate4d != "" )
	{
		if ( utime == 0 )
		{
			funcPool->put2( err, tb_udate4d, "NOUPDATE" ) ;
		}
		else
		{
			time_info = localtime( &utime ) ;
			strftime( buf, sizeof( buf ), "%Y/%m/%d", time_info ) ;
			funcPool->put2( err, tb_udate4d, buf ) ;
		}
		if ( err.error() ) { return ; }
	}
}


void Table::tbtop( errblock& err )
{
	//
	// Set the CRP to the top of the table, before the first row.
	//
	// RC =  0  Normal completion.
	// RC = 20  Severe error.
	//

	err.setRC( 0 ) ;
	CRP = 0 ;
}


void Table::tbvclear( errblock& err,
		      fPOOL* funcPool )
{
	//
	// Set all dialogue variables corresponding to the table rows when table created, to null.
	//
	// RC =  0  Normal completion.
	// RC = 20  Severe error.
	//

	for ( auto& f : tab_all2 )
	{
		funcPool->put2( err, f, "" ) ;
		if ( err.error() ) { return ; }
	}
}


void Table::fillfVARs( errblock& err,
		       fPOOL* funcPool,
		       int   zswind,
		       const set<string>& tb_fields,
		       const set<string>& tb_clear,
		       map<string, pair<string,uint>>& tb_lenvars,
		       bool scan,
		       int  depth,
		       int  csrrow,
		       int& ztdtop,
		       int& ztdrows,
		       int& ztdvrows,
		       int& idr,
		       char dir,
		       char scrolla,
		       int  zscrolln,
		       int  modppage )
{
	//
	// Fill the function pool variables (of the form model_fieldname.line) from the table for depth lines
	// starting at table position ztdtop. (Use CRP position if ztdtop not specified, ie -1).
	// Only fill variables that are on the tbmodel (either table or normal variables).
	//
	// Create function pool variable .ZURID.line to hold the URID of the table row corresponding to that screen line.
	//
	// Also pass back the relative line index, idr, for the line matching the passed csrrow (if any).
	// (Passed csrrow is the panel CSRROW() parameter or .CSRROW control variable).
	//
	// tb_fields and tb_clear variable names have already been checked.
	//
	// Update )FIELD length variables to the maximum field size on the current display.
	//

	uint i ;
	uint j ;

	string var ;
	string val ;
	string sufx ;

	bool lenvars_updated = false ;

	vector<vector<string>*>::iterator itt ;

	map<int, string>::iterator itf ;
	map<int, pair<string,uint>*>::iterator itl ;
	set<string>::iterator itg ;

	map<int, string> tab2fields ;
	map<int, pair<string,uint>*> tab4fields ;

	set<string> tab3fields ;
	set<string> set2fields ;

	err.setRC( 0 ) ;

	if ( scan && sarg.size() == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE046G" ) ;
		return ;
	}

	if ( ztdtop == -1 ) { ztdtop = CRP ; }
	if ( ztdtop == 0  ) { ztdtop = 1   ; }

	ztdrows = table.size() ;

	if ( ztdrows == 0 )
	{
		ztdvrows = 0 ;
		return ;
	}

	csrrow -= ztdtop ;

	for ( i = 0 ; i < num_all ; ++i )
	{
		const string& str = tab_all2[ i ] ;
		if ( tb_fields.count( str ) > 0 )
		{
			tab2fields[ i+2 ] = str ;
			set2fields.insert( str ) ;
		}
		if ( tb_lenvars.size() > 0 )
		{
			auto it = tb_lenvars.find( str ) ;
			if ( it != tb_lenvars.end() )
			{
				if ( it->second.first != "" )
				{
					val = funcPool->get2( err, 8, it->second.first ) ;
					if ( err.error() ) { return ; }
					if ( isnumeric( val ) )
					{
						j = ds2d( val ) ;
						if ( j >= 1 && j <= 32767 )
						{
							it->second.second = max( j, it->second.second ) ;
						}
					}
				}
				tab4fields[ i+2 ] = &it->second ;
			}
		}
	}

	if ( tb_fields.size() > set2fields.size() )
	{
		for ( auto it = tb_fields.begin() ; it != tb_fields.end() ; ++it )
		{
			if ( set2fields.count( *it ) == 0 )
			{
				tab3fields.insert( *it ) ;
				if ( !funcPool->ifexists( err, *it ) )
				{
					funcPool->put2( err, *it, "" ) ;
					if ( err.error() ) { return ; }
				}
			}
		}
	}

	if ( dir != ' ' )
	{
		ztdtop = setscroll( zswind, scan, dir, scrolla, ztdtop, ztdrows, zscrolln, modppage ) ;
	}

	for ( ztdvrows = 0, itt = table.begin() + ( ztdtop - 1 ) ; ztdvrows < depth && itt != table.end() ; ++itt )
	{
		if ( scan && !matchsarg( zswind, *itt ) )
		{
			if ( ztdvrows == 0 ) { ++ztdtop ; }
			continue ;
		}
		sufx = "." + d2ds( ztdvrows ) ;
		for ( itg = tb_clear.begin() ; itg != tb_clear.end() ; ++itg )
		{
			funcPool->put2( err, *itg, "" ) ;
			if ( err.error() ) { return ; }
		}
		funcPool->put3( ".ZURID" + sufx, (*itt)->at( 0 ) ) ;
		for ( itf = tab2fields.begin() ; itf != tab2fields.end() ; ++itf )
		{
			const string& str = (*itt)->at( itf->first ) ;
			funcPool->put2( err, itf->second, str ) ;
			if ( err.error() ) { return ; }
			funcPool->put3( itf->second + sufx, str ) ;
		}
		for ( itg = tab3fields.begin() ; itg != tab3fields.end() ; ++itg )
		{
			funcPool->put3( *itg + sufx, funcPool->get2( err, 8, *itg ) ) ;
			if ( err.error() ) { return ; }
		}
		for ( itl = tab4fields.begin() ; itl != tab4fields.end() ; ++itl )
		{
			const string& str = (*itt)->at( itl->first ) ;
			if ( itl->second->second < str.size() )
			{
				itl->second->second = str.size() ;
				lenvars_updated     = true ;
			}
		}
		if ( has_extvars( *itt ) )
		{
			const string& enames = get_extvar_list( *itt ) ;
			for ( j = 1, i = num_all+3 ; i < (*itt)->size() ; ++i, ++j )
			{
				var = word( enames, j ) ;
				if ( tb_fields.count( var ) == 0 ) { continue ; }
				const string& str = (*itt)->at( i ) ;
				funcPool->put2( err, var, str ) ;
				if ( err.error() ) { return ; }
				funcPool->put3( var + sufx, str ) ;
			}
		}
		++ztdvrows ;
	}

	if ( lenvars_updated )
	{
		for ( itl = tab4fields.begin() ; itl != tab4fields.end() ; ++itl )
		{
			if ( itl->second->first != "" )
			{
				funcPool->put2( err, itl->second->first, itl->second->second ) ;
				if ( err.error() ) { return ; }
			}
		}
	}

	idr = ( csrrow < ztdvrows ) ? csrrow : -1 ;
}


int Table::setscroll( int zswind,
		      bool scan,
		      char dir,
		      char scrolla,
		      int  ztdtop,
		      int  ztdrows,
		      int  zscrolln,
		      int  modppage )
{
	//
	// Scroll ztdtop to the correct position before building screen.
	// Done here to support ROWS(SCAN).
	//

	int k ;
	int l ;

	vector<vector<string>*>::iterator itf ;
	vector<vector<string>*>::reverse_iterator itr ;

	if ( scan )
	{
		if ( dir == 'U' )
		{
			if ( scrolla == 'M' )
			{
				ztdtop = 1 ;
			}
			else if ( ztdtop > 1 )
			{
				itr = ( ztdtop >= ztdrows ) ? table.rbegin() : table.rbegin() + ( ztdrows - ztdtop ) ;
				for ( k = 0 ; k < zscrolln && ztdtop > 0 && itr != table.rend() ; ++itr, --ztdtop )
				{
					if ( matchsarg( zswind, *itr ) ) { ++k ; }
				}
			}
		}
		else if ( dir == 'D' )
		{
			if ( scrolla == 'M' )
			{
				itr = table.rbegin() ;
				l   = modppage - 1 ;
				for ( ztdtop = ztdrows, k = 0 ; k < l && ztdtop > 0 && itr != table.rend() ; ++itr, --ztdtop )
				{
					if ( matchsarg( zswind, *itr ) ) { ++k ; }
				}
			}
			else if ( ztdtop <= ztdrows )
			{
				itf = table.begin() + ( ztdtop - 1 ) ;
				for ( k = 0 ; k < zscrolln && ztdtop <= ztdrows && itf != table.end() ; ++itf, ++ztdtop )
				{
					if ( matchsarg( zswind, *itf ) ) { ++k ; }
				}
			}
		}
	}
	else if ( dir == 'U' )
	{
		ztdtop = ( scrolla == 'M' || ztdtop <= zscrolln ) ? 1 : ( ztdtop - zscrolln ) ;
	}
	else if ( dir == 'D' )
	{
		ztdtop = ( scrolla == 'M' )              ? ( ztdrows - modppage + 1 ) :
			 ( ztdtop + zscrolln > ztdrows ) ? ( ztdrows + 1 ) : ( ztdtop + zscrolln ) ;
	}

	return max( 1, ztdtop ) ;
}


bool Table::matchsarg( int zswind,
		       vector<string>* row )
{
	//
	// Return true if the table record pointed to by row matches
	// the search arguments set by the TBSARG service call.
	//

	int p1 ;
	int yy ;

	bool end_loop = false ;

	uint s_match = 0 ;

	string date1 ;
	string date2 ;

	string* compstr1 ;
	string* compstr2 ;

	vector<tbsearch>::iterator it ;

	for ( it = sarg.begin() ; it != sarg.end() ; ++it )
	{
		p1 = it->tbs_position ;
		if ( p1 == -1 )
		{
			if ( !has_extvars( row ) ) { break ; }
			p1 = get_extvar_pos( row, it->tbs_var ) ;
			if ( p1 == 0 ) { break ; }
			p1 += num_all + 2 ;
		}
		if ( it->tbs_y > 0 )
		{
			date1 = row->at( p1 ) ;
			date2 = it->tbs_val ;
			if ( date1.size() > it->tbs_y )
			{
				yy = ds2d( date1.substr( it->tbs_y - 1, 2 ) ) ;
				date1.insert( it->tbs_y - 1, ( yy <= zswind ) ? "20" : "19" ) ;
			}
			if ( date2.size() > it->tbs_y )
			{
				yy = ds2d( date2.substr( it->tbs_y - 1, 2 ) ) ;
				date2.insert( it->tbs_y - 1, ( yy <= zswind ) ? "20" : "19" ) ;
			}
			compstr1 = &date1 ;
			compstr2 = &date2 ;
		}
		else
		{
			compstr1 = &row->at( p1 ) ;
			compstr2 = &it->tbs_val ;
		}
		if ( it->tbs_generic )
		{
			switch ( it->tbs_condition )
			{
			case S_EQ:
				end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) != 0 ) ;
				break ;

			case S_NE:
				end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) == 0 ) ;
				break ;

			case S_LE:
				end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) > 0 ) ;
				break ;

			case S_LT:
				end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) >= 0 ) ;
				break ;

			case S_GE:
				end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) < 0 ) ;
				break ;

			case S_GT:
				end_loop = ( compstr1->compare( 0, it->tbs_size, *compstr2 ) <= 0 ) ;
				break ;
			}
		}
		else
		{
			switch ( it->tbs_condition )
			{
			case S_EQ:
				end_loop = ( *compstr1 != *compstr2 ) ;
				break ;

			case S_NE:
				end_loop = ( *compstr1 == *compstr2 ) ;
				break ;

			case S_LE:
				end_loop = ( *compstr1 > *compstr2 ) ;
				break ;

			case S_LT:
				end_loop = ( *compstr1 >= *compstr2 ) ;
				break ;

			case S_GE:
				end_loop = ( *compstr1 < *compstr2 ) ;
				break ;

			case S_GT:
				end_loop = ( *compstr1 <= *compstr2 ) ;
				break ;
			}
		}
		if ( end_loop ) { break ; }
		++s_match ;
	}

	return ( s_match == sarg.size() ) ;
}


void Table::loadRows( errblock& err,
		      std::ifstream* fin,
		      const string& tb_name,
		      const string& filename,
		      const string& hdr,
		      const string& sir,
		      uint ver,
		      uint num_rows,
		      uint all_flds )
{
	//
	// Routine to load V1, V2, V3 and V4 format tables from a disk file.
	// Called by the tableMGR::loadTable method.
	//

	uint i ;
	uint j ;
	uint k ;
	uint l ;

	uint n1 ;
	uint n2 ;

	char  z[ 3 ] ;
	char* buf1 ;

	string prefix( "\x00\x00\x00\x00", 4 ) ;

	string val ;

	size_t buf1Size = 1024 ;

	vector<string>* row ;

	reserveSpace( num_rows ) ;

	if ( sir != "" )
	{
		tbsort( err, sir ) ;
		if ( err.error() ) { return ; }
	}

	buf1 = new char[ buf1Size ] ;

	for ( l = 0 ; l < num_rows ; ++l )
	{
		row = new vector<string> ;
		if ( ver > 3 )
		{
			fin->read( z, 1 ) ;
			if ( fin->fail() != 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
				delete[] buf1 ;
				delete row ;
				return ;
			}
			i = (unsigned char)z[ 0 ] ;
			fin->read( buf1, i ) ;
			if ( fin->fail() != 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
				delete[] buf1 ;
				delete row ;
				return ;
			}
			prefix.assign( buf1, i ) ;
		}
		row->push_back( prefix ) ;
		for ( j = 0 ; j < all_flds ; ++j )
		{
			fin->read( z, 2 ) ;
			if ( fin->fail() != 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
				delete[] buf1 ;
				delete row;
				return ;
			}
			n1 = (unsigned char)z[ 0 ] ;
			n2 = (unsigned char)z[ 1 ] ;
			i = ( n1 << 8 ) + n2 ;
			if ( i > buf1Size )
			{
				delete[] buf1 ;
				buf1Size = i  ;
				buf1     = new char[ buf1Size ] ;
			}
			fin->read( buf1, i ) ;
			if ( fin->fail() != 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
				delete[] buf1 ;
				delete row ;
				return ;
			}
			val.assign( buf1, i ) ;
			row->push_back( val ) ;
		}
		if ( ver > 1 )
		{
			fin->read( z, 2 ) ;
			if ( fin->fail() != 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
				delete[] buf1 ;
				delete row ;
				return ;
			}
			n1 = (unsigned char)z[ 0 ] ;
			n2 = (unsigned char)z[ 1 ] ;
			i = ( n1 << 8 ) + n2 ;
			for ( j = 0 ; j < i ; ++j )
			{
				fin->read( z, 2 ) ;
				if ( fin->fail() != 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
					delete[] buf1 ;
					delete row ;
					return ;
				}
				n1 = (unsigned char)z[ 0 ] ;
				n2 = (unsigned char)z[ 1 ] ;
				k = ( n1 << 8 ) + n2 ;
				if ( k > buf1Size )
				{
					delete[] buf1 ;
					buf1Size = i  ;
					buf1     = new char[ buf1Size ] ;
				}
				fin->read( buf1, k ) ;
				if ( fin->fail() != 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
					delete[] buf1 ;
					delete row ;
					return ;
				}
				val.assign( buf1, k ) ;
				row->push_back( val ) ;
			}
			fin->read( z, 1 ) ;
			if ( fin->fail() != 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
				delete[] buf1 ;
				delete row ;
				return ;
			}
			if ( z[ 0 ] != char(0xFF) )
			{
				err.seterrid( TRACE_INFO(), "PSYE014J", filename ) ;
				delete[] buf1 ;
				delete row ;
				return ;
			}
		}
		loadRow( err, row ) ;
		if ( err.error() )
		{
			delete[] buf1 ;
			delete row ;
			return ;
		}
	}

	delete[] buf1 ;

	if ( ver > 1 )
	{
		fin->read( z, 1 ) ;
		if ( fin->fail() != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE014M", tb_name, filename ) ;
			return ;
		}
		if ( z[ 0 ] != char(0xFF) )
		{
			err.seterrid( TRACE_INFO(), "PSYE014K", filename ) ;
			return ;
		}
	}

	set_path( filename ) ;
	reset_changed() ;

	pr_create = false ;

	parse_header( hdr ) ;
}


void Table::saveTable( errblock& err,
		       const string& tb_name,
		       const string& filename )
{
	//
	// Save table to disk.
	//
	// Version 2 file format adds extension variable support and record/file end markers, 0xFF.
	// Version 3 file format increases num_of_rows field from 2-bytes to 3-bytes (max 16,777,215 rows).
	// Version 4 file format adds a prefix to each record stored on disk.  Max length 255 bytes but currently only 4 bytes.
	//
	// Maximum field length is 65,535 and limited to MXTAB_SZ rows.
	//
	// filename is the location of the table (using the first path if not found in the concatination).
	//

	uint i ;
	uint j ;
	uint k ;
	uint size ;
	uint evar ;

	bool file_exists ;

	string hdr ;
	string prefix ;
	string filename1 ;
	string filename2 ;

	std::ofstream otable ;

	err.setRC( 0 ) ;

	filename1 = filename ;

	file_exists = exists( filename ) ;

	if ( file_exists )
	{
		filename2 = filename ;
		filename1.insert( filename.find_last_of( '/' ) + 1, "~" ) ;
		filename2.insert( filename.find_last_of( '/' ) + 1, "~~" ) ;
	}

	size = table.size() ;
	otable.open( filename1.c_str(), ios::binary | ios::out ) ;
	if ( !otable.is_open() )
	{
		err.seterrid( TRACE_INFO(), "PSYE014D", tb_name, filename ) ;
		return ;
	}

	if ( changed )
	{
		time( &utime ) ;
	}

	if ( updated && !pr_create )
	{
		++tableupd ;
	}

	hdr = create_header() ;

	otable << (char)00  ;         //
	otable << (char)133 ;         // x0085 denotes a table.
	otable << (char)4   ;         // Table file format.  Version 4.
	otable << (char)hdr.size() ;  // Header length.
	otable << hdr ;
	otable << (char)01  ;         // Number of fields following the header record (only the Sort Information Record for now).
	otable << (char)sort_ir.size() ;
	otable << sort_ir              ;
	otable << (char)( size >> 16 ) ;
	otable << (char)( size >> 8 )  ;
	otable << (char)( size )       ;
	otable << (char)num_keys       ;
	otable << (char)num_flds       ;

	for ( auto& f : tab_all2 )
	{
		otable << (char)f.size() ;
		otable << f ;
	}

	for ( i = 0 ; i < size ; ++i )
	{
		prefix = table.at( i )->at( 1 ) ;
		otable << (char)prefix.size() ;
		otable << prefix ;
		for ( j = 2 ; j <= num_all + 1 ; ++j )
		{
			k = table.at( i )->at( j ).size() ;
			if ( k > 65535 )
			{
				k = 65535 ;
				otable << (char)( k >> 8 ) ;
				otable << (char)( k ) ;
				otable << table.at( i )->at( j ).substr( 0, k ) ;
			}
			else
			{
				otable << (char)( k >> 8 ) ;
				otable << (char)( k ) ;
				otable << table.at( i )->at( j ) ;
			}
		}
		evar = table.at( i )->size() - num_all - 2 ;
		otable << (char)( evar >> 8 ) ;
		otable << (char)( evar ) ;
		for ( ; j < table.at( i )->size() ; ++j )
		{
			k = table.at( i )->at( j ).size() ;
			if ( k > 65535 )
			{
				k = 65535 ;
				otable << (char)( k >> 8 ) ;
				otable << (char)( k ) ;
				otable << table.at( i )->at( j ).substr( 0, k ) ;
			}
			else
			{
				otable << (char)( k >> 8 ) ;
				otable << (char)( k ) ;
				otable << table.at( i )->at( j ) ;
			}
		}
		otable << (char)0xFF ;
	}
	otable << (char)0xFF ;
	otable.close() ;

	if ( file_exists )
	{
		rename( filename, filename2 ) ;
		rename( filename1, filename ) ;
		remove( filename2 ) ;
	}

	reset_changed() ;
	tab_opath = filename.substr( 0, filename.find_last_of( '/' ) ) ;
}


void Table::cmdsearch( errblock& err,
		       fPOOL* funcPool,
		       const string& cmd )
{
	//
	// cmdsearch is not part of the normal table services for applications.
	// It's used for retrieving abbreviated commands from a command table.
	// Use TBSARG/TBSCAN for normal applications.
	//
	// RC =  0  Okay.
	// RC =  4  Commmand not found.
	// RC = 20  Severe error.
	//

	int trunc ;

	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	if ( !tab_cmds )
	{
		err.seterrid( TRACE_INFO(), "PSYE019D", "cmdsearch issued for a non-command table" ) ;
		return  ;
	}

	for ( it = table.begin() ; it != table.end() ; ++it )
	{
		trunc = ds2d( (*it)->at( 3 ) ) ;
		if ( trunc == 0 )
		{
			if ( (*it)->at( 2 ) == cmd ) { break ; }
		}
		else
		{
			if ( abbrev( (*it)->at( 2 ), cmd, trunc ) ) { break ; }
		}
	}

	if ( it == table.end() )
	{
		err.setRC( 4 ) ;
		return ;
	}

	funcPool->put2( err, "ZCTVERB", (*it)->at( 2 ) ) ;
	if ( err.error() ) { return ; }

	funcPool->put2( err, "ZCTTRUNC", (*it)->at( 3 ) ) ;
	if ( err.error() ) { return ; }

	funcPool->put2( err, "ZCTACT", (*it)->at( 4 ) ) ;
	if ( err.error() ) { return ; }

	funcPool->put2( err, "ZCTDESC", (*it)->at( 5 ) ) ;
	if ( err.error() ) { return ; }
}


int Table::get_virtsize()
{
	//
	// Return a rough estimate of the virtual storage used by a table.
	//

	int memory = sizeof( Table ) + ( table.size() * ( sizeof( vector<string>* ) + sizeof( vector<string> ) ) ) ;

	for ( const auto row : table )
	{
		for ( const auto& str : *row )
		{
		      memory += str.size() ;
		}
		memory += ( row->size() * sizeof( string ) ) ;
	}

	memory += ( sarg.size() * sizeof( tbsearch ) ) ;

	memory += ( tab_keys2.size() * sizeof( string ) ) ;
	for ( const auto& k : tab_keys2 )
	{
		memory += k.size() ;
	}

	memory += ( tab_all2.size() * sizeof( string ) ) ;
	for ( const auto& k : tab_all2 )
	{
		memory += k.size() ;
	}

	memory += ( urid2rid.size() * sizeof( urid2rid ) ) ;

	memory += ( rid2urid.size() * sizeof( rid2urid ) ) ;

	memory += ( openTasks.size() * sizeof( openTasks ) ) ;

	return memory ;
}


string Table::create_header()
{
	//
	// Store the following TBSTATS fields in the table file header section:
	//   1) ctime
	//   2) utime
	//   3) user
	//   4) rowcreat
	//   5) tableupd
	//
	//
	// HDR1 format:
	//    chars 'HDR1'
	//    1-byte - number of fields
	//
	//    field format:
	//      1-byte data length (not including the length field)
	//      data
	//
	// HDR1LLdataLdataLdataLdataLdata
	//

	string t1 ;
	string t2 ;
	string t3 ;
	string t4 ;

	time_t ctime1 = ctime ;
	time_t utime1 = utime ;

	unsigned char c1 ;
	unsigned char c2 ;

	for ( uint i = 0 ; i < sizeof( time_t ) ; ++i )
	{
		c1     = ctime1 ;
		t1     = string( 1, c1 ) + t1 ;
		ctime1 >>= 8 ;
		c2     = utime1 ;
		t2     = string( 1, c2 ) + t2 ;
		utime1 >>= 8 ;
	}

	t3 = d2cs( rowcreat ) ;
	t4 = d2cs( tableupd ) ;

	return "HDR1" +
	       string( 1, 5 ) +
	       string( 1, t1.size() ) + t1 +
	       string( 1, t2.size() ) + t2 +
	       string( 1, tab_user.size()) + tab_user +
	       string( 1, t3.size() ) + t3 +
	       string( 1, t4.size() ) + t4 ;
}


void Table::parse_header( const string& hdr )
{
	//
	// HDR1 format:
	//    chars 'HDR1'
	//    1-byte - number of fields
	//
	//    field format:
	//      1-byte data length (not including the length field)
	//      data
	//
	// HDR1NLdataLdataLdataLdataLdata
	//

	uint num_fields ;

	uint j ;
	uint l ;

	string tx ;

	unsigned char c1 ;

	ctime    = 0 ;
	utime    = 0 ;
	rowcreat = 0 ;
	tableupd = 0 ;
	tab_user = "" ;

	if ( hdr.size() > 4 && hdr.compare( 0, 4, "HDR1" ) == 0 )
	{
		num_fields = (unsigned char)hdr[ 4 ] ;
		j = 5 ;
		for ( uint i = 0 ; i < num_fields ; ++i )
		{
			l = (unsigned char)hdr[ j ] ;
			++j ;
			tx = hdr.substr( j, l ) ;
			switch ( i )
			{
			case 0:
				for ( uint i = 0 ; i < sizeof( time_t ) ; ++i )
				{
					c1    = tx[ i ] ;
					ctime <<= 8 ;
					ctime |= c1  ;
				}
				break ;

			case 1:
				for ( uint i = 0 ; i < sizeof( time_t ) ; ++i )
				{
					c1    = tx[ i ] ;
					utime <<= 8 ;
					utime |= c1  ;
				}
				break ;

			case 2:
				tab_user = tx ;
				break ;

			case 3:
				for ( uint i = 0 ; i < sizeof( int ) ; ++i )
				{
					c1       = tx[ i ] ;
					rowcreat <<= 8 ;
					rowcreat |= c1  ;
				}
				break ;

			case 4:
				for ( uint i = 0 ; i < sizeof( int ) ; ++i )
				{
					c1       = tx[ i ] ;
					tableupd <<= 8 ;
					tableupd |= c1  ;
				}
				break ;

			}
			j += l ;
		}
	}
}


bool Table::has_extvars( uint pos )
{
	//
	// Return true if row pointed to by pos has extension variables.
	//

	return ( table.at( pos )->size() > ( num_all + 2 ) ) ;
}


bool Table::has_extvars( vector<string>* row )
{
	//
	// Return true if row pointed to by pointer row has extension variables.
	//

	return ( row->size() > ( num_all + 2 ) ) ;
}


const string& Table::get_extvar_list( uint pos )
{
	//
	// Return extension variable name list at row pointed to by pos.
	//

	return table.at( pos )->at( num_all + 2 ) ;
}


const string& Table::get_extvar_list( vector<string>* row )
{
	//
	// Return extension variable name list at row pointed to by pointer row.
	//

	return row->at( num_all + 2 ) ;
}


int Table::get_extvar_pos( uint pos,
			   const string& evar )
{
	//
	// Return the position of the extension variable evar at row pointed to by pos.
	// First position is 1. 0 not found.
	//

	return wordpos( evar, get_extvar_list( pos ) ) ;
}


int Table::get_extvar_pos( vector<string>* row,
			   const string& evar )
{
	//
	// Return the position of the extension variable evar at row pointed to by pointer row.
	// First position is 1. 0 not found.
	//

	return wordpos( evar, get_extvar_list( row ) ) ;
}


// *******************************************************************************************************************************
// *************************************************** TABLE MANAGER SECTION *****************************************************
// *******************************************************************************************************************************

tableMGR::tableMGR( int i ) : tableMGR()
{
	zswind = i ;
}


tableMGR::tableMGR()
{
	maxId = 0 ;
}


tableMGR::~tableMGR()
{
	for ( auto& tab : tables )
	{
		delete tab.second ;
	}
}


Table* tableMGR::createTable( errblock& err,
			      const string& tb_name,
			      const string& tb_keys,
			      const string& tb_names,
			      tbWRITE tb_WRITE,
			      tbDISP tb_DISP )
{
	//
	// Create an empty table object.
	//
	// Returns: Address of the created table.
	//
	// Procedure called by TBCREATE and for TBOPEN when table not loaded.
	//
	// Lock mtx is held when this procedure is called so no need to hold it.
	//
	// This procedure does not set the return code and does not return an error.
	// Add task id of the nested dialogue parent to the list of in-use tasks.
	//

	Table* tab = new Table( tb_name,
				tb_keys,
				tb_names,
				tb_WRITE,
				tb_DISP,
				++maxId ) ;

	tab->addTasktoTable( err ) ;

	return tab ;
}


Table* tableMGR::loadTable( errblock& err,
			   const string& tb_name,
			   const string& filename,
			   tbWRITE tb_WRITE,
			   tbDISP tb_DISP,
			   bool no_add )
{
	//
	// Routine to load V1, V2, V3 and V4 format tables from a disk file.
	//
	// Returns: Address of the table.
	//
	// Called by TBOPEN and TBSTATS.
	//
	// Enqueue input file while loading the table.  Release when loaded if NOWRITE.
	//

	uint i ;
	uint j ;
	uint k ;

	uint n1 ;
	uint n2 ;
	uint n3 ;
	uint ver ;

	uint num_rows ;
	uint num_keys ;
	uint num_flds ;
	uint all_flds ;

	char  x ;
	char  z[ 3 ] ;
	char  buf1[ 256 ] ;

	string s    ;
	string hdr  ;
	string sir  ;
	string keys ;
	string flds ;

	Table* tab = nullptr ;

	std::ifstream fin ;

	fin.open( filename.c_str(), ios::binary | ios::in ) ;
	if ( !fin.is_open() )
	{
		err.seterrid( TRACE_INFO(), "PSYE014D", tb_name, filename ) ;
		return nullptr ;
	}

	fin.read( buf1, 2);
	if ( fin.gcount() != 2 || memcmp( buf1, "\x00\x85", 2 ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE014E", tb_name, filename ) ;
		fin.close() ;
		return nullptr ;
	}

	fin.get( x ) ;
	ver = (unsigned char)x ;
	if ( ver > 4 )
	{
		err.seterrid( TRACE_INFO(), "PSYE014F", d2ds( ver ), filename ) ;
		fin.close() ;
		return nullptr ;
	}

	fin.get( x ) ;
	i = (unsigned char)x ;
	fin.read( buf1, i ) ;
	hdr.assign( buf1, i ) ;
	fin.get( x ) ;
	i = (unsigned char)x ;
	for ( j = 0 ; j < i ; ++j )
	{
		fin.get( x ) ;
		k = (unsigned char)x ;
		fin.read( buf1, k ) ;
		switch ( j )
		{
		case 0: sir.assign( buf1, k ) ;
			break ;
		default:
			err.seterrid( TRACE_INFO(), "PSYE014G", d2ds( i ) ) ;
			fin.close() ;
			return nullptr ;
		}
	}
	if ( ver > 2 )
	{
		fin.read( z, 3 ) ;
		n1 = (unsigned char)z[ 0 ] ;
		n2 = (unsigned char)z[ 1 ] ;
		n3 = (unsigned char)z[ 2 ] ;
		num_rows = ( n1 << 16 ) + ( n2 << 8 ) + n3 ;
	}
	else
	{
		fin.read( z, 2 ) ;
		n1 = (unsigned char)z[ 0 ] ;
		n2 = (unsigned char)z[ 1 ] ;
		num_rows = ( n1 << 8 ) + n2 ;
	}

	fin.get( x ) ;
	n1 = (unsigned char)x ;
	num_keys = n1  ;
	fin.get( x ) ;
	n1 = (unsigned char)x ;
	num_flds = n1 ;
	all_flds = num_keys + num_flds ;

	keys = "" ;
	flds = "" ;

	for ( j = 0 ; j < num_keys ; ++j )
	{
		fin.get( x ) ;
		if ( fin.fail() != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE014H", tb_name, filename ) ;
			fin.close() ;
			return nullptr ;
		}
		i = (unsigned char)x ;
		fin.read( buf1, i ) ;
		keys = keys + s.assign( buf1, i ) + " " ;
	}

	for ( j = 0 ; j < num_flds ; ++j )
	{
		fin.get( x ) ;
		if ( fin.fail() != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE014I", tb_name, filename ) ;
			fin.close() ;
			return nullptr ;
		}
		i = (unsigned char)x ;
		fin.read( buf1, i ) ;
		flds = flds + s.assign( buf1, i ) + " " ;
	}

	tab = createTable( err,
			   tb_name,
			   keys,
			   flds,
			   tb_WRITE,
			   tb_DISP ) ;
	enq( tab, filename ) ;

	tab->loadRows( err,
		       &fin,
		       tb_name,
		       filename,
		       hdr,
		       sir,
		       ver,
		       num_rows,
		       all_flds ) ;

	fin.close() ;

	if ( err.error() )
	{
		deq( tab ) ;
		delete tab ;
		return nullptr ;
	}

	if ( !no_add )
	{
		tables.insert( make_pair( tb_name, tab ) ) ;
	}

	if ( tb_WRITE == NOWRITE )
	{
		deq( tab ) ;
	}

	return tab ;
}


void tableMGR::saveTable( errblock& err,
			  const string& tb_func,
			  const string& tb_name,
			  const string& tb_newname,
			  const string& tb_paths )
{
	//
	// Save a table to disk.
	//
	// This can be called by tbclose() or tbsave().
	//
	// tb_paths can be a concatination.  Save to path where table is located or, if not found,
	// the first file in the list.
	//

	string filename ;

	Table* tab ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", tb_func, tb_name, 12 ) ;
		return ;
	}

	filename = locate( err, ( tb_newname == "" ) ? tb_name : tb_newname, tb_paths ) ;
	if ( err.error() ) { return ; }

	if ( filename == "" )
	{
		filename = getpath( tb_paths, 1 ) ;
		filename = ( tb_newname == "" ) ? filename + tb_name : filename + tb_newname ;
	}

	tab = qscan( filename ) ;
	if ( tab && tab->tableClosedforTask( err ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013W", tb_func, tb_name, 20 ) ;
		return ;
	}

	tab = it->second ;
	tab->saveTable( err,
			tb_name,
			filename ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( ( tb_func == "TBSAVE" ) ? TB_SAVE : TB_NONE, err.getRC() ) ;
	}
}


void tableMGR::destroyTable( errblock& err,
			     const string& tb_name,
			     const string& tb_func )
{
	//
	// Reduce use count of a table and delete from storage if not in use.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	//
	// If the table is removed from storage, also release any enqueues it has,
	// otherwise remove task id of the nested dialogue parent from the list of in-use tasks.
	//

	Table* tab ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", tb_func, tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->removeTaskUsefromTable( err ) ;
	if ( tab->notInUse() )
	{
		deq( tab ) ;
		delete tab ;
		tables.erase( it ) ;
	}
}


void tableMGR::closeTablesforTask( errblock& err )
{
	Table* tab ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	for ( auto it = tables.begin() ; it != tables.end() ; )
	{
		tab = it->second ;
		if ( tab->tableOpenedforTask( err ) )
		{
			tab->removeTaskfromTable( err ) ;
			if ( tab->notInUse() )
			{
				deq( tab ) ;
				delete tab ;
				it = tables.erase( it ) ;
				continue ;
			}
		}
		++it ;
	}
}


void tableMGR::statistics()
{
	string t ;

	multimap<string, Table*>::iterator it ;
	vector<tbsearch>::iterator its ;
	map<Table*, path>::iterator ite ;

	Table* tab ;

	errblock err ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	llog( "I", ".STATS" << endl ) ;
	llog( "-", "Table Statistics:" <<endl ) ;
	llog( "-", "         Number of tables loaded . . . " << tables.size() <<endl ) ;
	llog( "-", "          Table details:" <<endl ) ;
	for ( it = tables.begin() ; it != tables.end() ; ++it )
	{
		tab = it->second ;
		llog( "-", "" <<endl ) ;
		llog( "-", "                  Table: "+ it->first <<endl ) ;
		t  = ( tab->tab_WRITE == WRITE ) ? "WRITE" : "NOWRITE" ;
		t += ( tab->tab_DISP  == SHARE ) ? ",SHARE " : " " ;
		t += ( tab->changed ) ? "(Modified)"  : "" ;
		llog( "-", "                 Status: Opened "+ t <<endl ) ;
		llog( "-", "              Use Count: " << tab->openTasks.size() <<endl ) ;
		if ( tab->num_keys > 0 )
		{
			llog( "-", "                   Keys: " << tab->tab_keys1 <<endl ) ;
		}
		llog( "-", "                 Fields: " << tab->tab_flds <<endl ) ;
		llog( "-", "         Number Of Rows: " << tab->table.size() <<endl ) ;
		llog( "-", "        Opened by Tasks: " << tab->listTasks() <<endl ) ;
		if ( tab->tab_ipath != "" )
		{
			llog( "-", "             Input Path: " << tab->tab_ipath <<endl ) ;
		}
		if ( tab->tab_opath != "" )
		{
			llog( "-", "            Output Path: " << tab->tab_opath <<endl ) ;
		}
		ite = table_enqs.find( tab ) ;
		if ( ite != table_enqs.end() )
		{
			llog( "-", "       Table Enqueue On: " << ite->second.string() <<endl ) ;
		}
		llog( "-", "    Current Row Pointer: " << tab->CRP <<endl ) ;
		if ( tab->sarg.size() > 0 )
		{
			llog( "-", "Current Search Argument. " <<endl ) ;
			llog( "-", "        Condition Pairs: " << tab->sa_cond_pairs <<endl ) ;
			llog( "-", "       Search Direction: " << tab->sa_dir <<endl ) ;
			llog( "-", "    Extension Variables: " << tab->sa_namelst <<endl ) ;
			for ( its = tab->sarg.begin() ; its != tab->sarg.end() ; ++its )
			{
				t = ( its->tbs_generic ) ? " (generic)" : "" ;
				llog( "-", "                  Field Name: "+ its->tbs_var <<endl ) ;
				llog( "-", "                 Field Value: "+ its->tbs_val + t <<endl ) ;
				llog( "-", "             Field Condition: "+ its->get_condition() <<endl ) ;
			}
		}
		if ( tab->sort_ir != "" )
		{
			llog( "-", "Sort Information Record: "+ tab->sort_ir <<endl ) ;
		}
		llog( "-", "------------------------ " <<endl ) ;
	}

	if ( table_enqs.size() > 0 )
	{
		llog( "-", "" <<endl ) ;
		llog( "-", "Table Enqueue Information:" <<endl ) ;
		llog( "-", "Table      Enqueue" <<endl ) ;
		llog( "-", "--------   -------------------------------------" <<endl ) ;
		for ( ite = table_enqs.begin() ; ite != table_enqs.end() ; ++ite )
		{
			llog( "-", "" << setw(11) << std::left << ite->first->tab_name
						  << std::left << ite->second.string() << endl) ;
		}
	}
	llog( "-", "*************************************************************************************************************" <<endl ) ;
}


void tableMGR::fillfVARs( errblock& err,
			  fPOOL* funcPool,
			  const string& tb_name,
			  const set<string>& tb_fields,
			  const set<string>& tb_clear,
			  map<string, pair<string,uint>>& tb_lenvars,
			  bool scan,
			  int  lines,
			  int  csrrow,
			  int& ztdtop,
			  int& ztdrows,
			  int& ztdvrows,
			  int& idr,
			  char dir,
			  char scrolla,
			  int  zscrolln,
			  int  modppage )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBDISPL", tb_name, 12 ) ;
		return ;
	}

	it->second->fillfVARs( err,
			       funcPool,
			       zswind,
			       tb_fields,
			       tb_clear,
			       tb_lenvars,
			       scan,
			       lines,
			       csrrow,
			       ztdtop,
			       ztdrows,
			       ztdvrows,
			       idr,
			       dir,
			       scrolla,
			       zscrolln,
			       modppage ) ;
}


void tableMGR::tbadd( errblock& err,
		      fPOOL* funcPool,
		      const string& tb_name,
		      const string& tb_namelst,
		      const string& tb_order,
		      int tb_num_of_rows )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	int i ;
	int ws ;

	string w ;

	Table* tab ;

	for ( ws = words( tb_namelst ), i = 1 ; i <= ws ; ++i )
	{
		w = word( tb_namelst, i ) ;
		if ( !isvalidName( w ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
			return ;
		}
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBADD", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbadd( err,
		    funcPool,
		    tb_namelst,
		    tb_order,
		    tb_num_of_rows ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_ADD, err.getRC() ) ;
	}
}


void tableMGR::tbbottom( errblock& err,
			 fPOOL* funcPool,
			 const string& tb_name,
			 const string& tb_save_name,
			 const string& tb_rowid_name,
			 const string& tb_noread,
			 const string& tb_crp_name  )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	string w ;

	if ( namesNotValid( w, tb_save_name, tb_rowid_name, tb_crp_name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBBOTTOM", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbbottom( err,
		       funcPool,
		       tb_save_name,
		       tb_rowid_name,
		       tb_noread,
		       tb_crp_name ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_BOTTOM, err.getRC() ) ;
	}
}


void tableMGR::tbcreate( errblock& err,
			 const string& tb_name,
			 const string& tb_keys,
			 const string& tb_names,
			 tbWRITE tb_WRITE,
			 tbREP tb_REP,
			 const string& tb_paths,
			 tbDISP tb_DISP )
{
	//
	// Create a new table in virtual storage and open it for processing.
	//
	// RC =  0  Normal.
	// RC =  4  Normal - duplicate table replaced.
	// RC =  8  REPLACE not specified and table exists,
	//          or REPLACE specified with table open in SHARE or SHARE requested,
	//          or SHARE specified but a different task has it open NON_SHARE.
	// RC = 12  Table file in use.  Enqueue failure.
	// RC = 20  Severe error.
	//
	// Exists => With WRITE   - In storage and/or on disk.
	//                NOWRITE - In storage only.
	//
	// Only allow 1 table shared, or any number non-shared for a given table name.
	//
	// If WRITE specified obtain the enqueue for the duration the table is open.
	// Use first file in the input library if table not found.
	//

	int i ;
	int ws ;

	string filename = "" ;
	string w ;

	Table* tab ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	multimap<string, Table*>::iterator it ;

	if ( tb_WRITE == WRITE )
	{
		filename = locate( err, tb_name, tb_paths ) ;
		if ( err.error() ) { return ; }
	}

	for ( ws = words( tb_keys ), i = 1 ; i <= ws ; ++i )
	{
		w = word( tb_keys, i ) ;
		if ( !isvalidName( w ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE022J", w, "key" ) ;
			return ;
		}
	}

	for ( ws = words( tb_names ), i = 1; i <= ws ; ++i )
	{
		w = word( tb_names, i ) ;
		if ( !isvalidName( w ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE022J", w, "field" ) ;
			return ;
		}
	}

	it = getTableIterator2( err, tb_name ) ;
	if ( it != tables.end() )
	{
		tab = it->second ;
		if ( tb_REP != REPLACE || tab->tab_DISP == SHARE || tb_DISP == SHARE )
		{
			err.setRC( 8 ) ;
			return ;
		}
		deq( tab ) ;
		delete tab ;
		tables.erase( it ) ;
		err.setRC( 4 ) ;
	}
	else if ( tb_DISP == SHARE && tables.find( tb_name ) != tables.end() )
	{
		err.setRC( 8 ) ;
		return ;
	}
	else if ( filename != "" )
	{
		if ( tb_REP == REPLACE )
		{
			err.setRC( 4 ) ;
		}
		else
		{
			err.setRC( 8 ) ;
			return ;
		}
	}

	if ( tb_WRITE == WRITE && filename == "" && getpaths( tb_paths ) > 0 )
	{
		filename = getpath( tb_paths, 1 ) + tb_name ;
	}

	if ( filename != "" && qscan( filename ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013W", "TBCREATE", tb_name, 12 ) ;
		return ;
	}

	tab = createTable( err,
			   tb_name,
			   tb_keys,
			   tb_names,
			   tb_WRITE,
			   tb_DISP ) ;
	if ( tab )
	{
		tables.insert( make_pair( tb_name, tab ) ) ;
		tab->set_service_cc( TB_CREATE, err.getRC() ) ;
		tab->pr_create = true ;
	}

	if ( filename != "" )
	{
		enq( tab, filename ) ;
	}
}


void tableMGR::tbdelete( errblock& err,
			 fPOOL* funcPool,
			 const string& tb_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBDELETE", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbdelete( err, funcPool ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_DELETE, err.getRC() ) ;
	}
}


void tableMGR::tberase( errblock& err,
			const string& tb_name,
			const string& tb_paths )
{
	//
	// Delete a table from the output library.  Table must not be opened in WRITE mode.
	//
	// RC =  0  Normal completion.
	// RC =  8  Table not found.
	// RC = 12  Table open.
	// RC = 20  Severe error.
	//

	string filename ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto ret = tables.equal_range( tb_name ) ;
	for ( auto it = ret.first ; it != ret.second ; ++it )
	{
		if ( it->second->tab_WRITE == WRITE )
		{
			err.seterrid( TRACE_INFO(), "PSYE013W", "TBERASE", tb_name, 12 ) ;
			return ;
		}
	}

	filename = locate( err, tb_name, tb_paths ) ;
	if ( err.error() ) { return ; }

	if ( filename != "" )
	{
		try
		{
			remove( filename ) ;
		}
		catch ( boost::filesystem::filesystem_error &e )
		{
			err.seterrid( TRACE_INFO(), "PSYS012C", e.what() ) ;
			return ;
		}
		catch (...)
		{
			err.seterrid( TRACE_INFO(), "PSYS012C", "Entry: "+ filename ) ;
			return ;
		}
	}
	else
	{
		err.setRC( 8 ) ;
	}
}


void tableMGR::tbexist( errblock& err,
			fPOOL* funcPool,
			const string& tb_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBEXIST", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbexist( err, funcPool ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_EXISTS, err.getRC() ) ;
	}
}


void tableMGR::tbget( errblock& err,
		      fPOOL* funcPool,
		      const string& tb_name,
		      const string& tb_save_name,
		      const string& tb_rowid_name,
		      const string& tb_noread,
		      const string& tb_crp_name  )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	string w ;

	if ( namesNotValid( w, tb_save_name, tb_rowid_name, tb_crp_name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBGET", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbget( err,
		    funcPool,
		    tb_save_name,
		    tb_rowid_name,
		    tb_noread,
		    tb_crp_name ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_GET, err.getRC() ) ;
	}
}


void tableMGR::tbmod( errblock& err,
		      fPOOL* funcPool,
		      const string& tb_name,
		      const string& tb_namelst,
		      const string& tb_order )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	int i ;
	int ws ;

	string w ;

	Table* tab ;

	for ( ws = words( tb_namelst ), i = 1 ; i <= ws ; ++i )
	{
		w = word( tb_namelst, i ) ;
		if ( !isvalidName( w ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
			return ;
		}
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBMOD", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbmod( err,
		    funcPool,
		    tb_namelst,
		    tb_order ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_MOD, err.getRC() ) ;
	}
}


Table* tableMGR::tbopen( errblock& err,
			 const string& tb_name,
			 tbWRITE tb_WRITE,
			 const string& tb_paths,
			 tbDISP tb_DISP )
{
	//
	// TBOPEN a table.
	//
	// RC =  0  Normal completion.
	// RC =  8  Table does not exist in search path.
	// RC = 12  Table already open by this or another task or inconsistent WRITE/NOWRITE parmameters.
	// RC = 20  Severe error.
	//
	// Returns: Address of the table.
	//
	// Only allow 1 table shared, or any number non-shared for a given table name.
	//
	// If table already loaded, use count can be increased only if the request
	// is SHARE when loaded SHARE and WRITE/NOWRITE match. Any other combination is invalid.
	//

	string filename ;

	multimap<string, Table*>::iterator it ;

	Table* tab ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	if ( tb_DISP == SHARE )
	{
		it = tables.find( tb_name ) ;
		if ( it != tables.end() && it->second->tab_DISP == NON_SHARE )
		{
			err.seterrid( TRACE_INFO(), "PSYE013Z", "TBOPEN", tb_name, 12 ) ;
			return nullptr ;
		}
	}

	tab = getTableAddress2( err, tb_name ) ;
	if ( tab )
	{
		if ( tab->tab_DISP == SHARE && tb_DISP == SHARE )
		{
			if ( tab->tab_WRITE == tb_WRITE )
			{
				tab->addTasktoTable( err ) ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE014R", tb_name ) ;
				tab = nullptr ;
			}
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE013Z", "TBOPEN", tb_name, 12 ) ;
			tab = nullptr ;
		}
		return tab ;
	}

	filename = locate( err, tb_name, tb_paths ) ;
	if ( err.error() ) { return tab ; }

	if ( filename == "" )
	{
		err.setRC( 8 ) ;
		return tab ;
	}

	if ( qscan( filename ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013W", "TBOPEN", tb_name, 12 ) ;
		return tab ;
	}

	tab = loadTable( err, tb_name, filename, tb_WRITE, tb_DISP ) ;
	if ( err.error() )
	{
		return nullptr ;
	}
	tab->tab_service = TB_OPEN ;

	return tab ;
}


void tableMGR::tbput( errblock& err,
		      fPOOL* funcPool,
		      const string& tb_name,
		      const string& tb_namelst,
		      const string& tb_order )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	int i ;
	int ws ;

	string w ;

	Table* tab ;

	for ( ws = words( tb_namelst ), i = 1 ; i <= ws ; ++i )
	{
		w = word( tb_namelst, i ) ;
		if ( !isvalidName( w ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
			return ;
		}
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBPUT", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbput( err,
		    funcPool,
		    tb_namelst,
		    tb_order ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_PUT, err.getRC() ) ;
	}
}


void tableMGR::tbquery( errblock& err,
			fPOOL* funcPool,
			const string& tb_name,
			const string& tb_keyn,
			const string& tb_varn,
			const string& tb_rownn,
			const string& tb_keynn,
			const string& tb_namenn,
			const string& tb_crpn,
			const string& tb_sirn,
			const string& tb_lstn,
			const string& tb_condn,
			const string& tb_dirn )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	string w ;

	if ( namesNotValid( w, tb_keyn, tb_varn, tb_rownn, tb_keynn ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}
	if ( namesNotValid( w, tb_namenn, tb_crpn, tb_sirn, tb_lstn ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}
	if ( namesNotValid( w, tb_condn, tb_dirn ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBQUERY", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbquery( err,
		      funcPool,
		      tb_keyn,
		      tb_varn,
		      tb_rownn,
		      tb_keynn,
		      tb_namenn,
		      tb_crpn,
		      tb_sirn,
		      tb_lstn,
		      tb_condn,
		      tb_dirn ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_QUERY, err.getRC() ) ;
	}
}


void tableMGR::tbquery( errblock& err,
			const string& tb_name,
			bool& cpairs )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	err.setRC( 0 ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBQUERY", tb_name, 12 ) ;
		return ;
	}

	it->second->tbquery( cpairs ) ;
}


void tableMGR::tbquery( errblock& err,
			const string& tb_name,
			uint& lcrp )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	err.setRC( 0 ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBQUERY", tb_name, 12 ) ;
		return ;
	}

	it->second->tbquery( lcrp ) ;
}


void tableMGR::tbsarg( errblock& err,
		       fPOOL* funcPool,
		       const string& tb_name,
		       const string& tb_namelst,
		       const string& tb_dir,
		       const string& tb_cond_pairs )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBSARG", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbsarg( err,
		     funcPool,
		     tb_namelst,
		     tb_dir,
		     tb_cond_pairs ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_SARG, err.getRC() ) ;
	}
}


void tableMGR::tbscan( errblock& err,
		       fPOOL* funcPool,
		       const string& tb_name,
		       const string& tb_namelst,
		       const string& tb_save_name,
		       const string& tb_rowid_name,
		       const string& tb_dir,
		       const string& tb_noread,
		       const string& tb_crp_name,
		       const string& tb_cond_pairs )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	string w ;

	if ( namesNotValid( w, tb_save_name, tb_rowid_name, tb_crp_name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBSCAN", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbscan( err,
		     funcPool,
		     zswind,
		     tb_namelst,
		     tb_save_name,
		     tb_rowid_name,
		     tb_dir,
		     tb_noread,
		     tb_crp_name,
		     tb_cond_pairs ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_SCAN, err.getRC() ) ;
	}
}


void tableMGR::tbskip( errblock& err,
		       fPOOL* funcPool,
		       const string& tb_name,
		       int num,
		       const string& tb_save_name,
		       const string& tb_rowid_name,
		       const string& tb_row,
		       const string& tb_noread,
		       const string& tb_crp_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	string w ;

	if ( namesNotValid( w, tb_save_name, tb_rowid_name, tb_crp_name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBSKIP", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbskip( err,
		     funcPool,
		     num,
		     tb_save_name,
		     tb_rowid_name,
		     tb_row,
		     tb_noread,
		     tb_crp_name ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_SKIP, err.getRC() ) ;
	}
}


void tableMGR::tbskip( errblock& err,
		       fPOOL* funcPool,
		       const string& tb_name,
		       const string& tb_rowid_name,
		       const string& tb_urid,
		       const string& tb_noread,
		       const string& tb_crp_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	string w ;

	if ( namesNotValid( w, tb_rowid_name, tb_crp_name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBSKIP", tb_name, 12 ) ;
		return ;
	}

	it->second->tbskip( err,
			    funcPool,
			    tb_rowid_name,
			    tb_urid,
			    tb_noread,
			    tb_crp_name ) ;
}


void tableMGR::tbsort( errblock& err,
		       const string& tb_name,
		       const string& tb_fields )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBSORT", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbsort( err, tb_fields ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_SORT, err.getRC() ) ;
	}
}


void tableMGR::tbstats( errblock& err,
			fPOOL* funcPool,
			const string& tb_name,
			const string& tb_cdate,
			const string& tb_ctime,
			const string& tb_udate,
			const string& tb_utime,
			const string& tb_user,
			const string& tb_rowcreat,
			const string& tb_rowcurr,
			const string& tb_rowupd,
			const string& tb_tableupd,
			const string& tb_service,
			const string& tb_retcode,
			const string& tb_status1,
			const string& tb_status2,
			const string& tb_status3,
			const string& tb_library,
			const string& tb_virtsize,
			const string& tb_cdate4d,
			const string& tb_udate4d )
{
	//
	// Return statistics for a table.  Table does not need to be open.
	//
	// If table is not loaded but is in the input library chain, load table separately
	// from the other tables and delete afterwards.
	//
	// STATUS-name1:
	// Set by the table manager.
	// 1) Table exists in the input library chain.
	// 2) Table does not exist in the input library chain.
	// 3) Table input library not allocated.
	//
	// STATUS-name2:
	// 1) Table is not open in this nested dialgue.
	// 2) Table is open in NOWRITE in this nested dialgue.
	// 3) Table is open in WRITE in this nested dialgue.
	// 4) Table is open in SHARED NOWRITE in this nested dialgue.
	// 5) Table is open in SHARED WRITE in this nested dialgue.
	//
	// STATUS-name3:
	// Set by the table manager.
	// 1) Table is available for WRITE mode.
	// 2) Table is not available for WRITE mode.
	//

	bool table_loaded = false ;

	string filename ;

	Table* tab ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	string w ;

	err.setRC( 0 ) ;

	if ( namesNotValid( w, tb_cdate, tb_ctime, tb_udate, tb_utime, tb_user ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	if ( namesNotValid( w, tb_rowcreat, tb_rowcurr, tb_rowupd, tb_tableupd, tb_service ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	if ( namesNotValid( w, tb_retcode, tb_status1, tb_status2, tb_status3, tb_virtsize ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	if ( namesNotValid( w, tb_cdate4d, tb_udate4d ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w, "variable" ) ;
		return ;
	}

	filename = locate( err, tb_name, tb_library ) ;
	if ( err.error() ) { return ; }

	if ( tb_status1 != "" )
	{
		funcPool->put2( err, tb_status1, ( tb_library == "" ) ? "3" :
						 ( filename   == "" ) ? "2" : "1" ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_status3 != "" )
	{
		funcPool->put2( err, tb_status3, status3( err, tb_name ) ? "1" : "2" ) ;
		if ( err.error() ) { return ; }
	}

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		if ( filename == "" )
		{
			if ( tb_status2 != "" )
			{
				funcPool->put2( err, tb_status2, "1" ) ;
			}
			return ;
		}
		tab = loadTable( err,
				 tb_name,
				 filename,
				 NOWRITE,
				 SHARE,
				 true ) ;
		if ( err.error() ) { return ; }
		table_loaded = true ;
	}
	else
	{
		tab = it->second ;
	}

	tab->tbstats( err,
		      funcPool,
		      tb_cdate,
		      tb_ctime,
		      tb_udate,
		      tb_utime,
		      tb_user,
		      tb_rowcreat,
		      tb_rowcurr,
		      tb_rowupd,
		      tb_tableupd,
		      tb_service,
		      tb_retcode,
		      tb_status2,
		      tb_virtsize,
		      tb_cdate4d,
		      tb_udate4d ) ;
	if ( err.error() )
	{
		if ( table_loaded )
		{
			delete tab ;
		}
		return ;
	}

	if ( table_loaded )
	{
		delete tab ;
		if ( tb_retcode != "" )
		{
			funcPool->put2( err, tb_retcode, "" ) ;
			if ( err.error() ) { return ; }
		}
		if ( tb_status2 != "" )
		{
			funcPool->put2( err, tb_status2, "1" ) ;
			if ( err.error() ) { return ; }
		}
	}
	else
	{
		tab->set_service_cc( TB_STATS, err.getRC() ) ;
	}
}


bool tableMGR::status3( errblock& err,
			const string& tb_name )
{
	//
	// Determine if a table can be opened WRITE.
	//
	// This can only be done when:
	//   1) Table is not open anywhere.
	//   2) Table is already opened SHARE/WRITE.
	//   3) Table is already opened non-SHARE/NOWRITE everywhere (except for the task opening WRITE where table must be closed).
	//

	Table* tab ;

	auto ret = tables.equal_range( tb_name ) ;

	if ( ret.first == ret.second )
	{
		return true ;
	}

	tab = ret.first->second ;
	if ( tab->tab_DISP == SHARE )
	{
		return ( tab->tab_WRITE == WRITE ) ;
	}

	for ( auto it = ret.first ; it != ret.second ; ++it )
	{
		tab = it->second ;
		if ( tab->tab_WRITE == WRITE || tab->tableOpenedforTask( err ) )
		{
			return false ;
		}
	}

	return true ;
}


void tableMGR::tbtop( errblock& err,
		      const string& tb_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBTOP", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbtop( err ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_TOP, err.getRC() ) ;
	}
}


void tableMGR::tbvclear( errblock& err,
			 fPOOL* funcPool,
			 const string& tb_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	Table* tab ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBVCLEAR", tb_name, 12 ) ;
		return ;
	}

	tab = it->second ;
	tab->tbvclear( err, funcPool ) ;
	if ( err.ok() )
	{
		tab->set_service_cc( TB_VCLEAR, err.getRC() ) ;
	}
}


void tableMGR::qtabopen( errblock& err,
			 fPOOL* funcPool,
			 const string& tb_list )
{
	//
	// Obtain a list of currently open lspf tables.
	//
	// RC =  0  Normal completion.
	// RC =  4  Incomplete list.  Insufficient space to construct variable name.
	// RC = 12  Variable name prefix too long (max 7 characters).
	// RC = 20  Severe error.
	//

	int i ;

	string var ;

	multimap<string, Table*>::iterator it ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	if ( !isvalidName( tb_list ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "QTABOPEN", tb_list ) ;
		return ;
	}

	if ( tb_list.size() > 7 )
	{
		err.seterrid( TRACE_INFO(), "PSYE014T", 12 ) ;
		return ;
	}

	funcPool->put2( err, tb_list+"0", tables.size() ) ;
	if ( err.error() ) { return ; }

	for ( i = 1, it = tables.begin() ; it != tables.end() ; ++it, ++i )
	{
		var = tb_list + d2ds( i ) ;
		if ( var.size() > 8 )
		{
			err.setRC( 4 ) ;
			return ;
		}
		funcPool->put2( err, var, it->first ) ;
		if ( err.error() ) { return ; }
	}
}


void tableMGR::cmdsearch( errblock& err,
			  fPOOL* funcPool,
			  string tb_name,
			  const string& cmd,
			  const string& paths,
			  bool  try_load )
{
	//
	// Search table 'tb_name' for command 'cmd'.  Load table if not already in storage and try_load set.
	//
	// Application command tables should already be loaded by SELECT processing so it is not
	// necessary to add LIBDEF ZTLIB/ZTUSR paths to 'paths' or to try to load these tables.
	//
	// RC =  0  Okay.
	// RC =  4  Command not found ( Table::cmdsearch() routine ).
	// RC =  8  Table not found ( tableMGR::tbopen() routine ).
	// RC = 20  Severe error.
	//

	Table* tab ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	tb_name += "CMDS" ;

	tab = getTableAddress2( err, tb_name ) ;
	if ( !tab )
	{
		if ( !try_load )
		{
			err.setRC( 4 ) ;
			return ;
		}
		tab = tbopen( err,
			      tb_name,
			      NOWRITE,
			      paths,
			      SHARE ) ;
		if ( !tab ) { return ; }
	}
	tab->cmdsearch( err,
			funcPool,
			cmd ) ;
}


bool tableMGR::writeableTable( errblock& err,
			       const string& tb_name,
			       const string& tb_func )
{
	//
	// Return the writeable status of a table.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	//

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", tb_func, tb_name, 12 ) ;
		return false ;
	}

	return ( it->second->tab_WRITE == WRITE ) ;
}


string tableMGR::locate( errblock& err,
			 const string& tb_name,
			 const string& tb_paths )
{
	//
	// Locate a table file in the library input chain.
	//

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	locator loc( tb_paths, tb_name ) ;

	loc.asis().locate() ;

	if ( loc.errors() )
	{
		err.seterrid( TRACE_INFO(), loc.msgid(), loc.mdata() ) ;
		return "" ;
	}

	return loc.entry() ;
}


void tableMGR::enq( Table* tab,
		    const string& filename )
{
	//
	// Enqueue a table.
	//

	table_enqs[ tab ] = filename ;
}


void tableMGR::deq( Table* tab )
{
	//
	// Dequeue a table.
	//

	table_enqs.erase( tab ) ;
}


Table* tableMGR::qscan( const string& filename )
{
	//
	// Return the table pointer for an enqueue.  NULL if not held.
	//

	for ( auto ite = table_enqs.begin() ; ite != table_enqs.end() ; ++ite )
	{
		if ( filename == ite->second )
		{
			return ite->first ;
		}
	}

	return nullptr ;
}


uint tableMGR::getid( errblock& err,
		      const string& tb_name )
{
	//
	// Return a table's unique id.
	//

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE013G", "TBDISPL", tb_name, 12 ) ;
		return 0 ;
	}

	return it->second->getid() ;
}


Table* tableMGR::getTableAddress1( errblock& err,
				   const string& tb_name )
{
	//
	// Return table address if it is in-use by the requestor, else NULL.
	//

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		return nullptr ;
	}

	return it->second ;
}


Table* tableMGR::getTableAddress2( errblock& err,
				   const string& tb_name )
{
	//
	// Return table address if it is shared, or in-use by the requestor, else NULL.
	//

	auto it = getTableIterator2( err, tb_name ) ;
	if ( it == tables.end() )
	{
		return nullptr ;
	}

	return it->second ;
}


multimap<string, Table*>::iterator tableMGR::getTableIterator1( errblock& err,
								const string& tb_name )
{
	//
	// Return the iterator for table tb_name.
	// There can be only 1 shared table or any number of non-shared tables for a given table name.
	//
	// Return table match if it is in-use by the requestor, else tables.end().
	//

	auto ret = tables.equal_range( tb_name ) ;

	for ( auto it = ret.first ; it != ret.second ; ++it )
	{
		if ( it->second->tableOpenedforTask( err ) )
		{
			return it ;
		}
	}

	return tables.end() ;
}


multimap<string, Table*>::iterator tableMGR::getTableIterator2( errblock& err,
								const string& tb_name )
{
	//
	// Return the iterator for table tb_name.
	// There can be only 1 shared table or any number of non-shared tables for a given table name.
	//
	// Return table match if it is shared, or in-use by the requestor, else tables.end().
	//

	auto ret = tables.equal_range( tb_name ) ;

	for ( auto it = ret.first ; it != ret.second ; ++it )
	{
		if ( it->second->tab_DISP == SHARE || it->second->tableOpenedforTask( err ) )
		{
			return it ;
		}
	}

	return tables.end() ;
}
