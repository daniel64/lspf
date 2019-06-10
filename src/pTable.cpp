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

// *******************************************************************************************************************************
// *************************************************** TABLE SECTION *************************************************************
// *******************************************************************************************************************************

Table::~Table()
{
	// Free table row storage when deleting a table

	for ( auto it = table.begin() ; it != table.end() ; ++it )
	{
		delete (*it) ;
	}
}


void Table::loadRow( errblock& err,
		     vector<string>* row )
{
	err.setRC( 0 ) ;

	if ( table.size() >= MXTAB_SZ )
	{
		err.seterrid( "PSYE013F" ) ;
		return ;
	}
	row->insert( row->begin(), d2ds( ++maxURID ) ) ;
	table.push_back( row ) ;
	changed = true ;
}


void Table::reserveSpace( int tot_rows )
{
	table.reserve( tot_rows ) ;
}


vector<vector<string>*>::iterator Table::getKeyItr( errblock& err,
						    fPOOL& funcPOOL )
{
	// Return the table row iterator of a row for a keyed table or table.end() if not found.
	// Use the column values from the function pool and set these to null if not found.

	// Set CRPX to the found row, in case the position is required (eg to set the CRP)

	uint i ;

	string var ;

	vector<string> keys ;
	vector<vector<string>*>::iterator it ;

	keys.reserve( num_keys ) ;

	for ( auto pt = tab_vkeys.begin() ; pt != tab_vkeys.end() ; ++pt )
	{
		keys.push_back( funcPOOL.get( err, 8, *pt, NOCHECK ) ) ;
		if ( err.RC8() )
		{
			funcPOOL.put2( err, *pt, "" ) ;
		}
		if ( err.error() ) { return table.end() ; }
	}

	CRPX = 1 ;
	for ( it = table.begin() ; it != table.end() ; ++it )
	{
		for ( i = 1 ; i <= num_keys ; ++i )
		{
			if ( (*it)->at( i ) != keys.at( i-1 ) )  { break ; }
		}
		if ( i > num_keys ) { return it ; }
		++CRPX ;
	}
	return table.end() ;
}


void Table::loadfuncPOOL( errblock& err,
			  fPOOL& funcPOOL,
			  const string& tb_savenm )
{
	// Set row variables (including extension variables) in function pool from row pointed to by the CRP

	uint i  ;
	uint ws ;

	string tbelst = "" ;

	vector<vector<string>*>::iterator it ;
	vector<string>::iterator pt ;

	it = table.begin() + CRP - 1 ;

	for ( i = 1, pt = tab_vall.begin() ; pt != tab_vall.end() ; ++pt, ++i )
	{
		funcPOOL.put2( err, *pt, (*it)->at( i ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( (*it)->size() > num_all+1 )
	{
		tbelst = (*it)->at( num_all+1 ) ;
		for ( ws = words( tbelst ), i = 1 ; i <= ws ; ++i )
		{
			funcPOOL.put1( err, word( tbelst, i ), (*it)->at( num_all+1+i ) ) ;
			if ( err.error() ) { return ; }
		}
		tbelst = "("+ tbelst +")" ;
	}

	if ( tb_savenm != "" )
	{
		funcPOOL.put1( err, tb_savenm, tbelst ) ;
	}
}




void Table::saveExtensionVarNames( errblock& err,
				   fPOOL& funcPOOL,
				   const string& tb_savenm )
{
	// Save extension variable names in tb_savenm function pool variable.  (Null if there are none)
	// For use when NOREAD specified with tb_savenm, otherwise it is set by loadfuncPool().

	vector<vector<string>*>::iterator it ;

	it = table.begin() + CRP - 1 ;

	if ( (*it)->size() > num_all+1 )
	{
		funcPOOL.put1( err, tb_savenm, "("+ (*it)->at( num_all+1 ) +")" ) ;
	}
	else
	{
		funcPOOL.put1( err, tb_savenm, "" ) ;
	}
}




void Table::loadFields( errblock& err,
			fPOOL& funcPOOL,
			const string& tb_namelst,
			vector<string>* row )
{
	// Load row fields (including namelst but not the URID) from the function pool into string vector row

	uint i  ;
	uint ws ;

	string var ;

	vector<string>::iterator pt ;

	for ( pt = tab_vall.begin() ; pt != tab_vall.end() ; ++pt )
	{
		row->push_back( funcPOOL.get( err, 8, *pt, NOCHECK ) ) ;
		if ( err.RC8() )
		{
			funcPOOL.put2( err, *pt, "" ) ;
		}
		if ( err.error() ) { return ; }
	}

	if ( tb_namelst != "" )
	{
		row->push_back( space( tb_namelst ) ) ;
		for ( ws = words( tb_namelst ), i = 1 ; i <= ws ; ++i )
		{
			var = word( tb_namelst, i ) ;
			row->push_back( funcPOOL.get( err, 8, var ) ) ;
			if ( err.RC8() )
			{
				funcPOOL.put1( err, var, "" ) ;
			}
			if ( err.error() ) { return ; }
		}
	}
}


void Table::storeIntValue( errblock& err,
			   fPOOL& funcPOOL,
			   const string& var,
			   int val,
			   int len )
{
	// Store an integer value in the function pool.  If the entry has not been vdefined as an integer,
	// convert to a string and pad on the left with zeroes, length 8.

	dataType var_type ;

	var_type = funcPOOL.getType( err, var ) ;
	if ( err.error() ) { return ; }

	if ( var_type == INTEGER )
	{
		funcPOOL.put1( err, var, val ) ;
	}
	else
	{
		funcPOOL.put1( err, var, d2ds( val, len ) ) ;
	}
}


void Table::loadFields_save( errblock& err,
			     fPOOL& funcPOOL,
			     const string& tb_savenm,
			     const string& tb_rowid_vn,
			     const string& tb_noread,
			     const string& tb_crp_name )
{
	// If requested, load the table fields into the function pool.
	// Also save (if specified)
	//        1) extention variables into tb_savenm
	//        2) row id into tb_rowid_vn
	//        3) CRP into tb_crp_name

	if ( tb_noread != "NOREAD" )
	{
		loadfuncPOOL( err, funcPOOL, tb_savenm ) ;
		if ( err.error() ) { return ; }
	}
	else if ( tb_savenm != "" )
	{
		saveExtensionVarNames( err, funcPOOL, tb_savenm ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_rowid_vn != "" )
	{
		funcPOOL.put1( err, tb_rowid_vn, table.at( CRP-1 )->at( 0 ) ) ;
		if ( err.error() ) { return ; }
	}

	if ( tb_crp_name != "" )
	{
		storeIntValue( err, funcPOOL, tb_crp_name, CRP ) ;
		if ( err.error() ) { return ; }
	}
}


bool Table::tableClosedforTask( errblock& err )
{
	return ( openTasks.count( err.ptid ) == 0 ) ;
}


bool Table::tableOpenedforTask( errblock& err )
{
	return ( openTasks.count( err.ptid ) > 0 ) ;
}


void Table::addTasktoTable( errblock& err )
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


string Table::listTasks()
{
	auto it  = openTasks.begin() ;
	string t = d2ds( it->first ) ;

	for ( ++it ; it != openTasks.end() ; ++it )
	{
		t += ", " + d2ds( it->first ) ;
	}
	return t ;
}


void Table::tbadd( errblock& err,
		   fPOOL& funcPOOL,
		   string tb_namelst,
		   string tb_order,
		   int tb_num_of_rows )
{
	// Add a row to a table after the CRP (if not sorted) or in the sort position of a sorted table if
	// ORDER has been specified

	// RC = 0  Okay
	// RC = 8  Keyed tables only.  Row already exists, CRP set to TOP
	// RC = 16 Numeric conversion error
	// RC = 20 Severe error

	// Add extension variables as varname list + values to the row vector after defined keys and fields (start posn @ 1+all_flds)

	// If ORDER specified on a sorted table, sort the table again after adding the record and reset CRP

	// If the table has no fields, tb_namelst must be specified.

	string key  ;
	string val  ;
	string URID ;

	vector<string> keys ;
	vector<string>* row ;
	vector<vector<string>*>::iterator it ;

	iupper( tb_namelst ) ;
	iupper( tb_order )   ;

	err.setRC( 0 ) ;

	if ( table.size() >= MXTAB_SZ )
	{
		err.seterrid( "PSYE013F" ) ;
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
		it = getKeyItr( err, funcPOOL ) ;
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
		err.seterrid( "PSYE013K", "TBADD" ) ;
		return ;
	}

	row = new vector<string> ;
	row->clear() ;
	row->push_back( d2ds( ++maxURID ) ) ;

	loadFields( err, funcPOOL, tb_namelst, row ) ;
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
}


void Table::tbbottom( errblock& err,
		      fPOOL& funcPOOL,
		      string tb_savenm,
		      string tb_rowid_vn,
		      string tb_noread,
		      string tb_crp_name )
{
	// RC = 0  Okay
	// RC = 8  Table Empty.  CRP set to top
	// RC = 20 Severe error

	err.setRC( 0 ) ;

	iupper( tb_savenm )   ;
	iupper( tb_rowid_vn ) ;
	iupper( tb_noread )   ;
	iupper( tb_crp_name ) ;

	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( "PSYE013M", "NOREAD", tb_noread ) ;
		return ;
	}

	if ( table.size() == 0 )
	{
		CRP = 0 ;
		err.setRC( 8 ) ;
		if ( tb_crp_name != "" )
		{
			storeIntValue( err, funcPOOL, tb_crp_name, CRP ) ;
		}
		return ;
	}

	CRP = table.size() ;

	loadFields_save( err,
			 funcPOOL,
			 tb_savenm,
			 tb_rowid_vn,
			 tb_noread,
			 tb_crp_name ) ;
}


void Table::tbdelete( errblock& err,
		      fPOOL& funcPOOL )
{
	// Delete a row in the table.
	// For keyed tables, this is the row pointed to by the current contents of the key variables
	// For non-keyed tables, this is the row pointed to by the CRP

	// After deletion, the CRP always points to the row before the one deleted

	// RC = 0  Okay
	// RC = 8  Keyed tables.  Row with key value does not exist.  CRP set to TOP
	//         Non-keyed tables.  CRP was at TOP (zero)
	// RC = 20 Severe error

	err.setRC( 0 ) ;

	vector<vector<string>*>::iterator it ;

	if ( num_keys > 0 )
	{
		it = getKeyItr( err, funcPOOL ) ;
		if ( err.error() ) { return ; }
		if ( it == table.end() )
		{
			CRP = 0 ;
			err.setRC( 8 ) ;
			return ;
		}
		delete (*it) ;
		table.erase( it ) ;
		CRP = CRPX - 1    ;
		changed = true ;
	}
	else if ( CRP == 0 )
	{
		err.setRC( 8 ) ;
	}
	else
	{
		--CRP ;
		it = table.begin() + CRP ;
		delete (*it)       ;
		table.erase( it )  ;
		changed = true ;
	}
}


void Table::tbexist( errblock& err,
		     fPOOL& funcPOOL )
{
	// Test for the existance of a row
	// For keyed tables, use the current value of the key variables.
	// For non-keyed tables, call not valid
	//
	// CRP is positioned at this row if found else TOP.

	// RC = 0  Okay
	// RC = 8  Keyed tables.  Row does not exist.  CRP is set to TOP (zero)
	//         Non-keyed tables.  Not valid.  CRP is set to TOP (zero)
	// RC = 20 Severe error

	err.setRC( 0 ) ;

	CRP = 0 ;

	if ( num_keys == 0 ) { err.setRC( 8 ) ; return ; }

	if ( getKeyItr( err, funcPOOL ) == table.end() )
	{
		if ( !err.error() ) { err.setRC( 8 ) ; }
		return ;
	}
	CRP = CRPX ;
}


void Table::tbget( errblock& err,
		   fPOOL& funcPOOL,
		   string tb_savenm,
		   string tb_rowid_vn,
		   string tb_noread,
		   string tb_crp_name )
{
	// Access row in the table.
	// For table with keys, use the current value of the key in the dialogue variable.
	// For non-keyed tables, use the CRP

	// RC = 0  Okay
	// RC = 8  CRP was at TOP(zero) for non-keyed tables or key not found for keyed tables
	// RC = 20 Severe error

	// ROWID name and CRP name are for output only (not used for finding the record)

	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	iupper( tb_savenm )   ;
	iupper( tb_rowid_vn ) ;
	iupper( tb_noread )   ;
	iupper( tb_crp_name ) ;

	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( "PSYE013M", "NOREAD", tb_noread ) ;
		return ;
	}

	if ( num_keys > 0 )
	{
		it = getKeyItr( err, funcPOOL ) ;
		if ( err.error() ) { return ; }
		CRP = ( it == table.end() ) ? 0 : CRPX ;
	}

	if ( CRP == 0 )
	{
		if ( tb_crp_name != "" )
		{
			storeIntValue( err, funcPOOL, tb_crp_name, CRP ) ;
			if ( err.error() ) { return ; }
		}
		err.setRC( 8 ) ;
		return ;
	}

	loadFields_save( err,
			 funcPOOL,
			 tb_savenm,
			 tb_rowid_vn,
			 tb_noread,
			 tb_crp_name ) ;
}


void Table::tbmod( errblock& err,
		   fPOOL& funcPOOL,
		   string tb_namelst,
		   string tb_order )
{
	// Unconditionally update the current row in a table using the CRP.

	// tbmod - Update row if match found on key (keyed tables),  else perform tbadd at the bottom of the table
	//         Non-keyed tables always same as a tbadd
	//         CRP always points to the row added/updated

	// RC = 0  Okay.  Keyed tables - row updated.  Non-keyed tables new row added
	// RC = 8  Row did not match - row added for keyed tables
	// RC = 16 Numeric conversion error
	// RC = 20 Severe error

	// Extension variables must be re-specified or they will be lost (tb_namelst)

	// If ORDER specified on a sorted table, sort again in case tbmod has changed the order and reset CRP

	string key  ;
	string val  ;
	string URID ;

	vector<string>* row ;
	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	if ( tb_order == "" ) { sort_ir = "" ; }

	iupper( tb_namelst ) ;
	iupper( tb_order )   ;

	if ( num_all == 0 && tb_namelst == "" )
	{
		err.seterrid( "PSYE013K", "TBMOD" ) ;
		return ;
	}

	if ( num_keys == 0 )
	{
		tbadd( err, funcPOOL, tb_namelst, tb_order, 0 ) ;
		return ;
	}

	it = getKeyItr( err, funcPOOL ) ;
	if ( err.error() ) { return ; }
	if ( it == table.end() )
	{
		CRP = table.size() ;
		tbadd( err, funcPOOL, tb_namelst, tb_order, 0 ) ;
		if ( err.RC0() ) { err.setRC( 8 ) ; }
	}
	else
	{
		row  = new vector<string> ;
		CRP  = CRPX ;
		URID = (*it)->at( 0 ) ;
		row->push_back( URID ) ;
		loadFields( err, funcPOOL, tb_namelst, row ) ;
		if ( err.error() ) { delete row ; return ; }
		delete (*it) ;
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
	changed = true ;
}


void Table::tbput( errblock& err,
		   fPOOL& funcPOOL,
		   string tb_namelst,
		   string tb_order )
{
	// Update a row in a table.
	// For non-keyed tables, use row pointed to by the CRP
	// For keyed tables, key variables must match row at CRP

	// RC = 0  OK
	// RC = 8  Keyed tables - keys do not match current row or CRP was set to top.  CRP set to top (0)
	//         Non-keyed tables - CRP was at top
	// RC = 16 Numeric conversion error for sorted tables
	// RC = 20 Severe error

	// If ORDER specified on a sorted table, sort again in case tbput has changed the order and reset CRP

	string key  ;
	string val  ;
	string URID ;

	err.setRC( 0 ) ;

	vector<string>* row ;
	vector<vector<string>*>::iterator it ;

	if ( tb_order == "" ) { sort_ir = "" ; }

	iupper( tb_namelst ) ;
	iupper( tb_order )   ;

	if ( CRP == 0 ) { err.setRC( 8 ) ; return ; }

	if ( num_all == 0 && tb_namelst == "" )
	{
		err.seterrid( "PSYE013K", "TBPUT" ) ;
		return ;
	}

	it = table.begin() + CRP - 1 ;

	row = new vector<string> ;

	URID = (*it)->at( 0 ) ;
	row->push_back( URID ) ;

	loadFields( err, funcPOOL, tb_namelst, row ) ;
	if ( err.error() ) { delete row ; return ; }

	for ( uint i = 1 ; i <= num_keys ; ++i )
	{
		if ( row->at( i ) != (*it)->at( i ) )
		{
			CRP = 0 ;
			err.setRC( 8 ) ;
			delete row ;
			return ;
		}
	}

	delete (*it) ;
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
	changed = true ;
}


void Table::tbquery( errblock& err,
		     fPOOL& funcPOOL,
		     string tb_keyn,
		     string tb_varn,
		     string tb_rownn,
		     string tb_keynn,
		     string tb_namenn,
		     string tb_crpn,
		     string tb_sirn,
		     string tb_lstn,
		     string tb_condn,
		     string tb_dirn )
{
	err.setRC( 0 ) ;

	iupper( tb_keyn )   ;
	iupper( tb_varn )   ;
	iupper( tb_rownn )  ;
	iupper( tb_keynn )  ;
	iupper( tb_namenn ) ;
	iupper( tb_crpn )   ;
	iupper( tb_sirn )   ;
	iupper( tb_lstn )   ;
	iupper( tb_condn )  ;
	iupper( tb_dirn )   ;

	if ( tb_keyn != "" )  { funcPOOL.put1( err, tb_keyn, tab_keys ) ; }
	if ( err.error() ) { return ; }
	if ( tb_varn != "" )  { funcPOOL.put1( err, tb_varn, tab_flds ) ; }
	if ( err.error() ) { return ; }
	if ( tb_rownn != "" ) { storeIntValue( err, funcPOOL, tb_rownn, table.size() ) ; }
	if ( err.error() ) { return ; }
	if ( tb_keynn != "" ) { storeIntValue( err, funcPOOL, tb_keynn, num_keys ) ; }
	if ( err.error() ) { return ; }
	if ( tb_namenn != "") { storeIntValue( err, funcPOOL, tb_namenn, num_flds ) ; }
	if ( err.error() ) { return ; }
	if ( tb_crpn != "" )  { storeIntValue( err, funcPOOL, tb_crpn, CRP ) ; }
	if ( err.error() ) { return ; }
	if ( tb_sirn != "" ) { funcPOOL.put1( err, tb_sirn, sort_ir ) ; }
	if ( err.error() ) { return ; }
	if ( tb_lstn != "" ) { funcPOOL.put1( err, tb_lstn, sa_namelst ) ; }
	if ( err.error() ) { return ; }
	if ( tb_condn != "" ) { funcPOOL.put1( err, tb_condn, sa_cond_pairs ) ; }
	if ( err.error() ) { return ; }
	if ( tb_dirn != "" ) { funcPOOL.put1( err, tb_dirn, sa_dir ) ; }
	if ( err.error() ) { return ; }
}


void Table::tbsarg( errblock& err,
		    fPOOL& funcPOOL,
		    string tb_namelst,
		    string tb_dir,
		    string tb_cond_pairs )
{

	// RC =  0  Okay. Search arguments set
	// RC =  8  No search arguments set (all column variables null and namelst not specified)
	// RC = 20  Severe error

	int i ;

	string val ;
	string nl_namelst ;
	string nl_cond_pairs ;

	vector<string>::iterator pt ;

	set<string>names ;

	err.setRC( 0 ) ;

	iupper( tb_dir ) ;

	if ( tb_dir != "NEXT" && tb_dir != "PREVIOUS" && tb_dir != "" )
	{
		err.seterrid( "PSYE013P", tb_dir ) ;
		return ;
	}

	nl_cond_pairs = getNameList( err, tb_cond_pairs ) ;
	if ( err.error() )
	{
		err.seterrid( "PSYE013O", tb_cond_pairs ) ;
		return ;
	}

	nl_namelst = getNameList( err, tb_namelst ) ;
	if ( err.error() ) { return ; }

	sarg.clear() ;
	for ( i = 1, pt = tab_vall.begin() ; pt != tab_vall.end() ; ++pt, ++i )
	{
		val = funcPOOL.get( err, 8, *pt, NOCHECK ) ;
		if ( err.error() ) { return ; }
		if ( val != "" )
		{
			sarg.push_back( tbsearch( *pt, val, i ) ) ;
		}
		names.insert( *pt ) ;
	}

	setscan( err, funcPOOL, sarg, names, nl_namelst, nl_cond_pairs ) ;
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
		     fPOOL& funcPOOL,
		     vector<tbsearch>& scan,
		     set<string>& names,
		     string& nl_namelst,
		     string& nl_cond_pairs )
{
	// Setup the search arguments in the scan vector for TBSCAN and TBSARG.

	// Notes:
	// TBSARG: Current value of all table variables (already loaded into the scan vector)
	//         nl_namelst is for extension variables, the values of which will be used in the search.
	//         Default condition is EQ.  Ignore nulls except for extension variables.

	// TBSCAN: Only use variables from nl_namelst (any type).  Don't ignore nulls.

	uint i  ;
	uint j  ;
	uint ws ;

	string var  ;
	string val  ;
	string cond ;

	vector<tbsearch>::iterator it ;
	vector<string>::iterator pt ;

	err.setRC( 0 ) ;

	iupper( nl_namelst ) ;
	iupper( nl_cond_pairs ) ;

	for ( ws = words( nl_namelst ), i = 1 ; i <= ws ; ++i )
	{
		var = word( nl_namelst, i ) ;
		if ( names.count( var ) > 0 )
		{
			err.seterrid( "PSYE013Y", var ) ;
			return ;
		}
		val = funcPOOL.get( err, 8, var ) ;
		if ( err.error() ) { return ; }
		if ( err.RC8() )
		{
			funcPOOL.put1( err, var, "" ) ;
		}
		for ( j = 1, pt = tab_vall.begin() ; pt != tab_vall.end() ; ++pt, ++j )
		{
			if ( var == *pt ) { break ; }
		}
		scan.push_back( tbsearch( var, val, ( j > tab_vall.size() ? -1 : j ) ) ) ;
		names.insert( var ) ;
	}

	ws = words( nl_cond_pairs ) ;
	if ( ws % 2 == 1 )
	{
		err.seterrid( "PSYE013R" ) ;
		return ;
	}

	for ( i = 1 ; i <= ws ; i += 2 )
	{
		var  = word( nl_cond_pairs, i ) ;
		cond = word( nl_cond_pairs, i+1 ) ;
		if ( !findword( var, tab_all ) && !findword( var, nl_namelst ) )
		{
			err.seterrid( "PSYE013S", var ) ;
			return ;
		}
		it = find_if( scan.begin(), scan.end(),
			[ &var ]( const tbsearch& tbs )
			{
				return ( var == tbs.tbs_var ) ;
			} ) ;
		if ( it != scan.end() && !it->setCondition( cond ) )
		{
			err.seterrid( "PSYE013T", cond ) ;
			return ;
		}
	}
}


void Table::tbscan( errblock& err,
		    fPOOL& funcPOOL,
		    string tb_namelst,
		    string tb_savenm,
		    string tb_rowid_vn,
		    string tb_dir,
		    string tb_noread,
		    string tb_crp_name,
		    string tb_condlst )
{
	// Scan table from current CRP according to parameters tb_namelst/tb_condlst/tb_dir if specified
	// or the search parameters set by a previous TBSARG call.

	// tb_condlst contains the condidtions to use for variables in tb_namelst (1:1 between the two lists).
	// Only use variables in tb_namelst not other table variables.

	// RC = 0   Okay.  CRP set to row found.
	// RC = 8   Row not found.  CRP set to top (zero)
	// RC = 20  Severe error

	int i  ;
	int ws ;
	int p1 ;

	uint size    ;
	uint s_match ;

	bool s_next  ;
	bool found   ;
	bool endloop ;

	string s_dir = sa_dir ;

	string val    ;
	string var    ;
	string cond   ;
	string tbelst ;

	string nl_namelst ;
	string nl_condlst ;
	string nl_cond_pairs ;

	vector<tbsearch> scan ;
	vector<tbsearch>* pscan ;
	vector<tbsearch>::iterator it ;

	set<string>names ;

	err.setRC( 0 ) ;

	iupper( tb_savenm )   ;
	iupper( tb_rowid_vn ) ;
	iupper( tb_dir )      ;
	iupper( tb_noread )   ;
	iupper( tb_crp_name ) ;

	if ( tb_dir != "NEXT" && tb_dir != "PREVIOUS" && tb_dir != "" )
	{
		err.seterrid( "PSYE013P", tb_dir ) ;
		return ;
	}

	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( "PSYE013M", "NOREAD", tb_noread ) ;
		return ;
	}

	nl_condlst = getNameList( err, tb_condlst ) ;
	if ( err.error() )
	{
		err.seterrid( "PSYE013Q", tb_condlst ) ;
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
			err.seterrid( "PSYE013U", tab_name ) ;
			return ;
		}
		pscan = &sarg ;
	}
	else
	{
		ws = words( nl_namelst ) ;
		if ( words( nl_condlst ) > ws )
		{
			err.seterrid( "PSYE014C" ) ;
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
		setscan( err, funcPOOL, scan, names, nl_namelst, nl_cond_pairs ) ;
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
			if ( CRP < 1 ) { break ; }
		}
		s_match = 0     ;
		endloop = false ;
		for ( it = pscan->begin() ; it != pscan->end() ; ++it )
		{
			p1 = it->tbs_pos ;
			if ( p1 == -1 )
			{
				if ( table.at( CRP-1 )->size() == num_all + 1 ) { break ; }
				tbelst = table.at( CRP-1 )->at( num_all + 1 ) ;
				p1 = wordpos( it->tbs_var, tbelst ) ;
				if ( p1 == 0 ) { break ; }
				p1 += num_all + 1 ;
			}
			if ( it->tbs_gen )
			{
				switch ( it->tbs_cond )
				{
				case s_EQ:
					if ( table.at( CRP-1 )->at( p1 ).compare( 0, it->tbs_size, it->tbs_val ) != 0 )
					{
						endloop = true ;
					}
					break ;

				case s_NE:
					if ( table.at( CRP-1 )->at( p1 ).compare( 0, it->tbs_size, it->tbs_val ) == 0 )
					{
						endloop = true ;
					}
					break ;

				case s_LE:
					if ( table.at( CRP-1 )->at( p1 ).compare( 0, it->tbs_size, it->tbs_val ) > 0 )
					{
						endloop = true ;
					}
					break ;

				case s_LT:
					if ( table.at( CRP-1 )->at( p1 ).compare( 0, it->tbs_size, it->tbs_val ) >= 0 )
					{
						endloop = true ;
					}
					break ;

				case s_GE:
					if ( table.at( CRP-1 )->at( p1 ).compare( 0, it->tbs_size, it->tbs_val ) < 0 )
					{
						endloop = true ;
					}
					break ;

				case s_GT:
					if ( table.at( CRP-1 )->at( p1 ).compare( 0, it->tbs_size, it->tbs_val ) <= 0 )
					{
						endloop = true ;
					}
				}
			}
			else
			{
				switch ( it->tbs_cond )
				{
				case s_EQ:
					if ( table.at( CRP-1 )->at( p1 ) != it->tbs_val )
					{
						endloop = true ;
					}
					break ;

				case s_NE:
					if ( table.at( CRP-1 )->at( p1 ) == it->tbs_val )
					{
						endloop = true ;
					}
					break ;

				case s_LE:
					if ( table.at( CRP-1 )->at( p1 )  > it->tbs_val )
					{
						endloop = true ;
					}
					break ;

				case s_LT:
					if ( table.at( CRP-1 )->at( p1 ) >= it->tbs_val )
					{
						endloop = true ;
					}
					break ;

				case s_GE:
					if ( table.at( CRP-1 )->at( p1 )  < it->tbs_val )
					{
						endloop = true ;
					}
					break ;

				case s_GT:
					if ( table.at( CRP-1 )->at( p1 ) <= it->tbs_val )
					{
						endloop = true ;
					}
				}
			}
			if ( endloop ) { break ; }
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
			storeIntValue( err, funcPOOL, tb_crp_name, CRP ) ;
			if ( err.error() ) { return ; }
		}
		err.setRC( 8 ) ;
		return ;
	}

	loadFields_save( err,
			 funcPOOL,
			 tb_savenm,
			 tb_rowid_vn,
			 tb_noread,
			 tb_crp_name ) ;
}


void Table::tbskip( errblock& err,
		    fPOOL& funcPOOL,
		    int num,
		    string tb_savenm,
		    string tb_rowid_vn,
		    const string& tb_rowid,
		    string tb_noread,
		    string tb_crp_name )
{
	// Move CRP to a position in the table and read the row into the dialogue variables
	// Position using tb_rowid (URID) if specified, else use num

	// RC = 0  Okay
	// RC = 8  CRP would be outside the table. CRP set to top (zero)
	// RC = 12 Table not open
	// RC = 16 Truncation has occured
	// RC = 20 Severe error

	string val ;
	string var ;

	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	iupper( tb_savenm )   ;
	iupper( tb_rowid_vn ) ;
	iupper( tb_noread )   ;
	iupper( tb_crp_name ) ;

	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( "PSYE013M", "NOREAD", tb_noread ) ;
		return ;
	}

	if ( tb_rowid != "" )
	{
		CRP = 1 ;
		for ( it = table.begin() ; it != table.end() ; ++it )
		{
			if ( tb_rowid == (*it)->at( 0 ) ) { break ; }
			++CRP ;
		}
		if ( it == table.end() )
		{
			CRP = 0 ;
			if ( tb_crp_name != "" )
			{
				storeIntValue( err, funcPOOL, tb_crp_name, CRP ) ;
				if ( err.error() ) { return ; }
			}
			err.setRC( 8 ) ;
			return ;
		}
	}
	else
	{
		if ( ( ( CRP + num ) < 1 ) || ( ( CRP + num ) > table.size() ) )
		{
			CRP = 0 ;
			if ( tb_crp_name != "" )
			{
				storeIntValue( err, funcPOOL, tb_crp_name, CRP ) ;
				if ( err.error() ) { return ; }
			}
			err.setRC( 8 ) ;
			return ;
		}
		CRP += num ;
	}

	loadFields_save( err,
			 funcPOOL,
			 tb_savenm,
			 tb_rowid_vn,
			 tb_noread,
			 tb_crp_name ) ;
}


void Table::tbsort( errblock& err,
		    string tb_fields )
{
	// FIELD
	// FIELD,C
	// FIELD,C,A,FIELD2
	// FIELD,C,A,FIELD2,N
	// FIELD,C,A,FIELD2,N,D

	int i  ;
	int f1 ;
	int ws ;

	string s_fields ;
	string s_temp   ;

	vector<string> s_parm ;
	vector<int> s_field   ;
	vector<bool> s_char   ;
	vector<bool> s_asc    ;

	int nsort ;

	string temp ;

	err.setRC( 0 ) ;

	iupper( tb_fields ) ;
	temp = tb_fields    ;

	replace( tb_fields.begin(), tb_fields.end(), ',', ' ' ) ;
	for ( ws = words( tb_fields ), i = 1 ; i <= ws ; ++i )
	{
		s_parm.push_back( word( tb_fields, i ) ) ;
	}

	s_fields = "" ;
	while ( true )
	{
		if ( s_parm.size() == 0 ) { break ; }
		s_temp   = s_parm.front() ;
		s_fields = s_temp + " " + s_fields ;
		f1 = wordpos( s_temp, tab_all ) ;
		if ( f1 == 0 )
		{
			err.seterrid( "PSYE013X", temp ) ;
			return ;
		}
		s_field.push_back( f1 ) ;
		if ( s_parm.size() == 1 ) { break ; }
		s_parm.erase( s_parm.begin() ) ;

		s_temp = s_parm.front() ;
		if ( s_temp != "C" && s_temp != "N" )
		{
			err.seterrid( "PSYE013X", temp ) ;
			return ;
		}
		s_char.push_back( (s_temp == "C") ) ;
		if ( s_parm.size() == 1 ) { break ; }
		s_parm.erase( s_parm.begin() ) ;

		s_temp = s_parm.front() ;
		if ( s_temp != "A" && s_temp != "D" )
		{
			err.seterrid( "PSYE013X", temp ) ;
			return ;
		}
		s_asc.push_back( (s_temp == "A") ) ;
		s_parm.erase( s_parm.begin() ) ;
	}

	s_char.push_back( true ) ;
	s_asc.push_back( true )  ;

	nsort   = s_field.size() ;
	sort_ir = temp ;

	replace( sort_ir.begin(), sort_ir.end(), ' ', ',' ) ;

	stable_sort( table.begin(), table.end(),
		[ &s_field, &s_char, &s_asc, nsort ]( const vector<string>* a, const vector<string>* b )
		{
			for ( int j = 0 ; j < nsort ; ++j )
			{
				int k = s_field[ j ] ;
				if ( s_char[ j ] )
				{
					 if ( a->at( k ) == b->at( k ) ) { continue ; }
					 if ( s_asc[ j ] )
					 {
						 return a->at( k ) < b->at( k ) ;
					 }
					 else
					 {
						 return a->at( k ) > b->at( k ) ;
					 }
				}
				else
				{
					 int ia = ds2d( a->at( k ) ) ;
					 int ib = ds2d( b->at( k ) ) ;
					 if ( ia == ib ) { continue ; }
					 if ( s_asc[ j ] )
					 {
						 return ia < ib ;
					 }
					 else
					 {
						 return ia > ib ;
					 }
				}
			}
			return false ;
		} ) ;
	CRP = 0 ;
	changed = true ;
}


void Table::tbtop( errblock& err )
{
	// Set the CRP to the top of the table, before the first row

	// RC = 0  Normal completion
	// RC = 20 Severe error

	err.setRC( 0 ) ;
	CRP = 0 ;
}


void Table::tbvclear( errblock& err,
		      fPOOL& funcPOOL )
{
	// Set all dialogue variables corresponding to the table rows when table created, to null

	// RC = 0  Normal completion
	// RC = 20 Severe error

	for ( auto pt = tab_vall.begin() ; pt != tab_vall.end() ; ++pt )
	{
		funcPOOL.put2( err, *pt, "" ) ;
		if ( err.error() ) { return ; }
	}
}


void Table::fillfVARs( errblock& err,
		       fPOOL& funcPOOL,
		       const set<string>& tb_fields,
		       const set<string>& tb_clear,
		       bool scan,
		       uint depth,
		       int  posn,
		       uint csrrow,
		       int& idr )
{
	// Fill the function pool variables ( of the form model_fieldname.line ) from the table for depth lines
	// starting at table position posn. (Use CRP position if posn not specified)
	// Only fill variables that are on the tbmodel (either table or normal variables)

	// Create function pool variable .ZURID.line to hold the URID of the table row corresponding to that screen line

	// Also pass back the relative line index, idr, for the line matching the passed csrrow (if any).
	// (Passed csrrow is the panel CSRROW() parameter or .CSRROW control variable)

	// tb_fields and tb_clear variable names have already been checked.

	// BUG:  SCAN not supported yet

	uint j ;
	uint k ;
	uint l ;

	string var    ;
	string sufx   ;
	string enames ;

	vector<vector<string>*>::iterator itt ;

	map<int, string>::iterator itf ;
	set<string>::iterator itg ;

	map<int, string> tab2fields ;

	set<string> tab3fields ;
	set<string> set2fields ;

	err.setRC( 0 ) ;

	if ( posn == -1 ) { posn = CRP ; }
	if ( posn == 0  ) { posn = 1   ; }

	funcPOOL.put2( err, "ZTDTOP", posn ) ;
	if ( err.error() ) { return ; }

	funcPOOL.put2( err, "ZTDROWS", table.size() ) ;
	if ( err.error() ) { return ; }

	csrrow -= posn ;
	idr     = -1   ;

	for ( j = 0 ; j < num_all ; ++j )
	{
		const string& t = tab_vall[ j ] ;
		if ( tb_fields.count( t ) > 0 )
		{
			tab2fields[ j+1 ] = t ;
			set2fields.insert( t ) ;
		}
	}

	if ( tb_fields.size() > set2fields.size() )
	{
		for ( auto it = tb_fields.begin() ; it != tb_fields.end() ; ++it )
		{
			if ( set2fields.count( *it ) == 0 )
			{
				tab3fields.insert( *it ) ;
			}
		}
	}


	for ( itt = table.begin() + posn - 1, k = 0 ; k < depth && itt != table.end() ; ++k, ++itt )
	{
		sufx = "." + d2ds( k ) ;
		for ( itg = tb_clear.begin() ; itg != tb_clear.end() ; ++itg )
		{
			funcPOOL.put2( err, *itg, "" ) ;
			if ( err.error() ) { return ; }
		}
		funcPOOL.put3( err, ".ZURID" + sufx, (*itt)->at( 0 ) ) ;
		for ( itf = tab2fields.begin() ; itf != tab2fields.end() ; ++itf )
		{
			funcPOOL.put2( err, itf->second, (*itt)->at( itf->first ) ) ;
			if ( err.error() ) { return ; }
			funcPOOL.put3( err, itf->second + sufx, (*itt)->at( itf->first ) ) ;
			if ( err.error() ) { return ; }
		}
		for ( itg = tab3fields.begin() ; itg != tab3fields.end() ; ++itg )
		{
			funcPOOL.put3( err, *itg + sufx, funcPOOL.get( err, 8, *itg ) ) ;
			if ( err.error() ) { return ; }
		}
		if ( (*itt)->size() > num_all+1 )
		{
			enames = (*itt)->at( num_all+1 ) ;
			for ( l = 1, j = num_all+2 ; j < (*itt)->size() ; ++j, ++l )
			{
				var = word( enames, l ) ;
				if ( tb_fields.count( var ) == 0 ) { continue ; }
				funcPOOL.put1( err, var, (*itt)->at( j ) ) ;
				if ( err.error() ) { return ; }
				funcPOOL.put3( err, var + sufx, (*itt)->at( j ) ) ;
				if ( err.error() ) { return ; }
			}
		}
	}

	if ( k > csrrow ) { idr = csrrow ; }
}


void Table::saveTable( errblock& err,
		       const string& tb_name,
		       const string& filename )
{
	// Save table to disk.
	// Version 2 file format adds extension variable support and record/file end markers, 0xFF.
	// Version 3 file format increases num_of_rows field from 2-bytes to 3-bytes (max 16,777,215 rows)

	// Maximum field value is 65,535 and limited to MXTAB_SZ rows.

	// filename is the location of the table (using the first path if not found in the concatination).

	uint i ;
	uint j ;
	uint k ;
	uint size ;
	uint evar ;

	std::ofstream otable ;

	err.setRC( 0 ) ;

	size = table.size() ;
	otable.open( filename.c_str(), ios::binary | ios::out ) ;
	if ( !otable.is_open() )
	{
		err.seterrid( "PSYE014D", tb_name, filename ) ;
		return ;
	}

	otable << (char)00  ;  //
	otable << (char)133 ;  // x085 denotes a table
	otable << (char)3   ;  // Table file format.  Version 3 (3-byte size field)
	otable << (char)44  ;  // Header length
	otable << "HDR                                         " ;
	otable << (char)01  ;  // Number of fields following the header record (only the Sort Information Record for now)
	otable << (char)sort_ir.size() ;
	otable << sort_ir              ;
	otable << (char)( size >> 16 ) ;
	otable << (char)( size >> 8 )  ;
	otable << (char)( size )       ;
	otable << (char)num_keys       ;
	otable << (char)num_flds       ;
	for ( auto pt = tab_vall.begin() ; pt != tab_vall.end() ; ++pt )
	{
		otable << (char)pt->size() ;
		otable << *pt ;
	}
	for ( i = 0 ; i < size ; ++i )
	{
		for ( j = 1 ; j <= num_all ; ++j )
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
		evar = table.at( i )->size() - num_all - 1 ;
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

	resetChanged() ;
	tab_opath = filename.substr( 0, filename.find_last_of( '/' ) ) ;
}


void Table::cmdsearch( errblock& err,
		       fPOOL& funcPOOL,
		       const string& cmd )
{
	// cmdsearch is not part of the normal table services for applications.
	// It's used for retrieving abbreviated commands from a command table.
	// Use TBSARG/TBSCAN for normal applications

	// RC = 0  Okay
	// RC = 4  Commmand not found
	// RC = 20 Severe error

	int trunc ;

	vector<vector<string>*>::iterator it ;

	err.setRC( 0 ) ;

	if ( !tab_cmds )
	{
		err.seterror( "cmdsearch issued for a non-command table" ) ;
		return  ;
	}

	for ( it = table.begin() ; it != table.end() ; ++it )
	{
		trunc = ds2d( (*it)->at( 2 ) ) ;
		if ( trunc == 0 )
		{
			if ( (*it)->at( 1 ) == cmd ) { break ; }
		}
		else
		{
			if ( abbrev( (*it)->at( 1 ), cmd, trunc ) ) { break ; }
		}
	}

	if ( it == table.end() )
	{
		err.setRC( 4 ) ;
		return ;
	}

	funcPOOL.put2( err, "ZCTVERB", (*it)->at( 1 ) ) ;
	if ( err.error() ) { return ; }

	funcPOOL.put2( err, "ZCTTRUNC", (*it)->at( 2 ) ) ;
	if ( err.error() ) { return ; }

	funcPOOL.put2( err, "ZCTACT", (*it)->at( 3 ) ) ;
	if ( err.error() ) { return ; }

	funcPOOL.put2( err, "ZCTDESC", (*it)->at( 4 ) ) ;
	if ( err.error() ) { return ; }
}


// *******************************************************************************************************************************
// *************************************************** TABLE MANAGER SECTION *****************************************************
// *******************************************************************************************************************************

tableMGR::~tableMGR()
{
	multimap<string, Table*>::iterator it ;

	for ( it = tables.begin() ; it != tables.end() ; ++it )
	{
		delete it->second ;
	}
}


multimap<string, Table*>::iterator tableMGR::createTable( errblock& err,
							  const string& tb_name,
							  string keys,
							  string flds,
							  tbWRITE tb_WRITE,
							  tbDISP tb_DISP )
{
	// Procedure called for TBCREATE and for TBOPEN when table not loaded.
	// Lock mtx is held when this procedure is called so no need to hold it.
	// This procedure does not set the return code.
	// Add task id of the nested SELECT parent to the list of in-use tasks.

	// Returns: Iterator to the inserted table.

	string temp ;

	iupper( keys ) ;
	iupper( flds ) ;

	Table* t = new Table ;

	t->tab_WRITE = tb_WRITE ;
	t->tab_DISP  = tb_DISP  ;
	t->changed   = true     ;
	t->tab_name  = tb_name  ;
	t->tab_keys  = space( keys ) ;
	t->num_keys  = words( keys ) ;
	t->tab_flds  = space( flds ) ;
	t->num_flds  = words( flds ) ;
	t->tab_all   = t->tab_keys + " " + t->tab_flds ;
	t->num_all   = t->num_keys + t->num_flds ;
	t->tab_cmds  = ( t->tab_all == "ZCTVERB ZCTTRUNC ZCTACT ZCTDESC" ) ;

	for ( uint i = 1 ; i <= t->num_keys ; ++i )
	{
		temp = word( t->tab_keys, i ) ;
		t->tab_vall.push_back( temp ) ;
		t->tab_vkeys.push_back( temp ) ;
	}

	for ( uint i = 1 ; i <= t->num_flds ; ++i )
	{
		t->tab_vall.push_back( word( t->tab_flds, i ) ) ;
	}

	t->addTasktoTable( err ) ;

	return tables.insert( make_pair( tb_name, t ) ) ;
}


multimap<string, Table*>::iterator tableMGR::loadTable( errblock& err,
							const string& tb_name,
							tbWRITE tb_WRITE,
							const string& tb_paths,
							tbDISP tb_DISP )
{
	// Routine to load V1, V2 and V3 format tables from a disk file

	// RC = 0  Normal completion
	// RC = 8  Table does not exist in search path
	// RC = 12 Table already open by this or another task or inconsistent WRITE/NOWRITE parmameters
	// RC = 20 Severe error

	// Returns: Iterator to the table entry.

	// Only allow 1 table shared, or any number non-shared for a given table name.

	// If table already loaded, use count can be increased only if the request
	// is SHARE when loaded SHARE and WRITE/NOWRITE match. Any other combination is invalid.

	// Enqueue input file while loading the table.  Release when loaded if NOWRITE.

	uint i ;
	uint j ;
	uint k ;
	uint l ;
	uint n1 ;
	uint n2 ;
	uint n3 ;
	uint ver ;

	uint num_rows ;
	uint num_keys ;
	uint num_flds ;
	uint all_flds ;

	char  x ;
	char  buf1[ 256 ] ;
	char* buf2   ;
	char  z[ 3 ] ;

	string filename ;
	string path ;
	string s    ;
	string hdr  ;
	string sir  ;
	string val  ;
	string keys ;
	string flds ;

	size_t buf2Size = 1024 ;

	vector<string>* row ;

	std::ifstream table ;

	multimap<string, Table*>::iterator it ;

	errblock err1 ;

	err1.ptid = err.ptid ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	if ( tb_DISP == SHARE )
	{
		it = tables.find( tb_name ) ;
		if ( it != tables.end() && it->second->tab_DISP == NON_SHARE )
		{
			err.seterrid( "PSYE013Z", "TBOPEN", tb_name, 12 ) ;
			return tables.end() ;
		}
	}

	it = getTableIterator2( err, tb_name ) ;
	if ( it != tables.end() )
	{
		if ( it->second->tab_DISP == SHARE && tb_DISP == SHARE )
		{
			if ( it->second->tab_WRITE == tb_WRITE )
			{
				it->second->addTasktoTable( err ) ;
			}
			else
			{
				err.seterrid( "PSYE014R", tb_name ) ;
			}
		}
		else
		{
			err.seterrid( "PSYE013Z", "TBOPEN", tb_name, 12 ) ;
		}
		return it ;
	}

	filename = locate( err, tb_name, tb_paths ) ;
	if ( err.error() ) { return it ; }

	if ( filename == "" )
	{
		err.setRC( 8 ) ;
		return it ;
	}

	if ( qscan( filename ) )
	{
		err.seterrid( "PSYE013W", "TBOPEN", tb_name, 12 ) ;
		return it ;
	}

	table.open( filename.c_str(), ios::binary | ios::in ) ;
	if ( !table.is_open() )
	{
		err.seterrid( "PSYE014D", tb_name, filename ) ;
		destroyTable( err1, tb_name ) ;
		return it ;
	}

	table.read( buf1, 2);
	if ( memcmp( buf1, "\x00\x85", 2 ) )
	{
		err.seterrid( "PSYE014E", tb_name, filename ) ;
		table.close() ;
		destroyTable( err1, tb_name ) ;
		return it ;
	}

	table.get( x ) ;
	ver = (unsigned char)x ;
	if ( ver > 3 )
	{
		err.seterrid( "PSYE014F", d2ds( ver ), filename ) ;
		table.close() ;
		destroyTable( err1, tb_name ) ;
		return it ;
	}

	table.get( x ) ;
	i = (unsigned char)x ;
	table.read( buf1, i ) ;
	hdr.assign( buf1, i ) ;
	table.get( x ) ;
	i = (unsigned char)x ;
	for ( j = 0 ; j < i ; ++j )
	{
		table.get( x ) ;
		k = (unsigned char)x ;
		table.read( buf1, k ) ;
		switch ( j )
		{
		case 0: sir.assign( buf1, k ) ;
			break ;
		default:
			err.seterrid( "PSYE014G", d2ds( j+1 ) ) ;
			table.close() ;
			destroyTable( err1, tb_name ) ;
			return it ;
		}
	}
	if ( ver > 2 )
	{
		table.read( z, 3 ) ;
		n1 = (unsigned char)z[ 0 ] ;
		n2 = (unsigned char)z[ 1 ] ;
		n3 = (unsigned char)z[ 2 ] ;
		num_rows = n1 * 65536 + n2 * 256 + n3 ;
	}
	else
	{
		table.read( z, 2 ) ;
		n1 = (unsigned char)z[ 0 ] ;
		n2 = (unsigned char)z[ 1 ] ;
		num_rows = n1 * 256 + n2 ;
	}

	table.get( x ) ;
	n1 = (unsigned char)x ;
	num_keys = n1  ;
	table.get( x ) ;
	n1 = (unsigned char)x ;
	num_flds = n1 ;
	all_flds = num_keys + num_flds ;

	keys = "" ;
	flds = "" ;

	for ( j = 0 ; j < num_keys ; ++j )
	{
		table.get( x ) ;
		if ( table.fail() != 0 )
		{
			err.seterrid( "PSYE014H", tb_name, filename ) ;
			table.close() ;
			destroyTable( err1, tb_name ) ;
			return it ;
		}
		i = (unsigned char)x ;
		table.read( buf1, i ) ;
		keys = keys + s.assign( buf1, i ) + " " ;
	}
	for ( j = 0 ; j < num_flds ; ++j )
	{
		table.get( x ) ;
		if ( table.fail() != 0 )
		{
			err.seterrid( "PSYE014I", tb_name, filename ) ;
			table.close() ;
			destroyTable( err1, tb_name ) ;
			return it ;
		}
		i = (unsigned char)x ;
		table.read( buf1, i ) ;
		flds = flds + s.assign( buf1, i ) + " " ;
	}

	it = createTable( err, tb_name, keys, flds, tb_WRITE, tb_DISP ) ;

	enq( it->second, filename ) ;

	it->second->reserveSpace( num_rows ) ;
	if ( sir != "" )
	{
		it->second->tbsort( err, sir ) ;
		if ( err.error() )
		{
			destroyTable( err1, tb_name ) ;
			return it ;
		}
	}
	buf2 = new char[ buf2Size ] ;
	for ( l = 0 ; l < num_rows ; ++l )
	{
		row = new vector<string> ;
		for ( j = 0 ; j < all_flds ; ++j )
		{
			table.read( z, 2 ) ;
			if ( table.fail() != 0 )
			{
				err.seterrid( "PSYE014M", tb_name, filename ) ;
				table.close() ;
				delete[] buf2 ;
				delete row    ;
				destroyTable( err1, tb_name ) ;
				return it ;
			}
			n1 = (unsigned char)z[ 0 ] ;
			n2 = (unsigned char)z[ 1 ] ;
			i = n1 * 256 + n2 ;
			if ( i > buf2Size )
			{
				delete[] buf2 ;
				buf2Size = i  ;
				buf2     = new char[ buf2Size ] ;
			}
			table.read( buf2, i ) ;
			if ( table.fail() != 0 )
			{
				err.seterrid( "PSYE014M", tb_name, filename ) ;
				table.close() ;
				delete[] buf2 ;
				delete row    ;
				destroyTable( err1, tb_name ) ;
				return it ;
			}
			val.assign( buf2, i ) ;
			debug2( "Value read for row "<< l <<" position "<< j <<" '"+ val +"'" <<endl ) ;
			row->push_back( val ) ;
		}
		if ( ver > 1 )
		{
			table.read( z, 2 ) ;
			if ( table.fail() != 0 )
			{
				err.seterrid( "PSYE014M", tb_name, filename ) ;
				table.close() ;
				delete[] buf2 ;
				delete row    ;
				destroyTable( err1, tb_name ) ;
				return it ;
			}
			n1 = (unsigned char)z[ 0 ] ;
			n2 = (unsigned char)z[ 1 ] ;
			i = n1 * 256 + n2 ;
			for ( j = 0 ; j < i ; ++j )
			{
				table.read( z, 2 ) ;
				if ( table.fail() != 0 )
				{
					err.seterrid( "PSYE014M", tb_name, filename ) ;
					table.close() ;
					delete[] buf2 ;
					delete row    ;
					destroyTable( err1, tb_name ) ;
					return it ;
				}
				n1 = (unsigned char)z[ 0 ] ;
				n2 = (unsigned char)z[ 1 ] ;
				k = n1 * 256 + n2 ;
				if ( k > buf2Size )
				{
					delete[] buf2 ;
					buf2Size = i  ;
					buf2     = new char[ buf2Size ] ;
				}
				table.read( buf2, k ) ;
				if ( table.fail() != 0 )
				{
					err.seterrid( "PSYE014M", tb_name, filename ) ;
					table.close() ;
					delete[] buf2 ;
					delete row    ;
					destroyTable( err1, tb_name ) ;
					return it ;
				}
				val.assign( buf2, k ) ;
				row->push_back( val ) ;
			}
			table.read( z, 1 ) ;
			if ( table.fail() != 0 )
			{
				err.seterrid( "PSYE014M", tb_name, filename ) ;
				table.close() ;
				delete[] buf2 ;
				delete row    ;
				destroyTable( err1, tb_name ) ;
				return it ;
			}
			if ( z[ 0 ] != char(0xFF) )
			{
				err.seterrid( "PSYE014J", filename ) ;
				table.close() ;
				delete[] buf2 ;
				delete row    ;
				destroyTable( err1, tb_name ) ;
				return it ;
			}
		}
		it->second->loadRow( err, row ) ;
		if ( err.error() )
		{
			table.close() ;
			delete[] buf2 ;
			delete row    ;
			destroyTable( err1, tb_name ) ;
			return it ;
		}
	}
	if ( ver > 1 )
	{
		table.read( z, 1 ) ;
		if ( table.fail() != 0 )
		{
			err.seterrid( "PSYE014M", tb_name, filename ) ;
			table.close() ;
			delete[] buf2 ;
			destroyTable( err1, tb_name ) ;
			return it ;
		}
		if ( z[ 0 ] != char(0xFF) )
		{
			err.seterrid( "PSYE014K", filename ) ;
			table.close() ;
			delete[] buf2 ;
			destroyTable( err1, tb_name ) ;
			return it ;
		}
	}

	it->second->tab_ipath = filename.substr( 0, filename.find_last_of( '/' ) ) ;
	it->second->resetChanged() ;

	debug2( "Number of rows loaded from table " << l <<endl ) ;
	if ( l != num_rows )
	{
		err.seterrid( "PSYE014L", d2ds( l ), d2ds( num_rows ) ) ;
		destroyTable( err1, tb_name ) ;
	}
	table.close() ;
	delete[] buf2 ;

	if ( tb_WRITE == NOWRITE )
	{
		deq( it->second ) ;
	}

	return it ;
}


void tableMGR::qtabopen( errblock& err,
			 fPOOL& funcPOOL,
			 const string& tb_list )
{
	// RC = 0  Normal completion
	// RC = 12 Variable name prefix too long (max 7 characters)
	// RC = 20 Severe error

	int i ;

	string var ;

	multimap<string, Table*>::iterator it ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	if ( !isvalidName( tb_list ) )
	{
		err.seterrid( "PSYE013A", "QTABOPEN", tb_list ) ;
		return ;
	}

	if ( tb_list.size() > 7 )
	{
		err.seterrid( "PSYE014T", 12 ) ;
		return ;
	}

	funcPOOL.put2( err, tb_list+"0", tables.size() ) ;
	if ( err.error() ) { return ; }

	for ( i = 1, it = tables.begin() ; it != tables.end() ; ++it, ++i )
	{
		var = tb_list + d2ds( i ) ;
		if ( var.size() > 8 ) { return ; }
		funcPOOL.put2( err, var, it->first ) ;
		if ( err.error() ) { return ; }
	}
}


void tableMGR::saveTable( errblock& err,
			  const string& tb_func,
			  const string& tb_name,
			  const string& tb_newname,
			  const string& tb_paths )
{
	// This can be called by tbclose() or tbsave().

	// tb_paths can be a concatination.  Save to path where table is located or, if not found,
	// the first file in the list.

	string filename ;

	Table* tab ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", tb_func, tb_name, 12 ) ;
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
		err.seterrid( "PSYE013W", tb_func, tb_name, 20 ) ;
		return ;
	}

	it->second->saveTable( err, tb_name, filename ) ;
}


void tableMGR::destroyTable( errblock& err,
			     const string& tb_name,
			     const string& tb_func )
{
	// RC =  0 Normal completion
	// RC = 12 Table not open

	// If the table is removed from storage, also release any enqueues it has,
	// otherwise remove task id of the nested SELECT parent from the list of in-use tasks.

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", tb_func, tb_name, 12 ) ;
		return ;
	}

	it->second->removeTaskUsefromTable( err ) ;
	if ( it->second->notInUse() )
	{
		deq( it->second )  ;
		delete it->second  ;
		tables.erase( it ) ;
	}
}


void tableMGR::closeTablesforTask( errblock& err )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	for ( auto it = tables.begin() ; it != tables.end() ; )
	{
		if ( it->second->tableOpenedforTask( err ) )
		{
			it->second->removeTaskfromTable( err ) ;
			if ( it->second->notInUse() )
			{
				deq( it->second ) ;
				delete it->second ;
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
	map<Table*, string>::iterator ite ;

	errblock err ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	llog( "I", ".STATS" << endl ) ;
	llog( "-", "Table Statistics:" <<endl ) ;
	llog( "-", "         Number of tables loaded . . . " << tables.size() <<endl ) ;
	llog( "-", "          Table details:" <<endl ) ;
	for ( it = tables.begin() ; it != tables.end() ; ++it )
	{
		llog( "-", "" <<endl ) ;
		llog( "-", "                  Table: "+ it->first <<endl ) ;
		t  = ( it->second->tab_WRITE == WRITE ) ? "WRITE,"  : "NOWRITE," ;
		t += ( it->second->tab_DISP  == SHARE ) ? "SHARED " : "NON-SHARED " ;
		t += ( it->second->changed ) ? "(Modified)"  : "" ;
		llog( "-", "                 Status: Opened "+ t <<endl ) ;
		llog( "-", "              Use Count: " << it->second->openTasks.size() <<endl ) ;
		if ( it->second->num_keys > 0 )
		{
			llog( "-", "                   Keys: " << setw(3) << it->second->tab_keys <<endl ) ;
		}
		llog( "-", "                 Fields: " << it->second->tab_flds <<endl ) ;
		llog( "-", "         Number Of Rows: " << it->second->table.size() <<endl ) ;
		llog( "-", "        Opened by Tasks: " << it->second->listTasks() <<endl ) ;
		if ( it->second->tab_ipath != "" )
		{
			llog( "-", "             Input Path: " << it->second->tab_ipath <<endl ) ;
		}
		if ( it->second->tab_opath != "" )
		{
			llog( "-", "            Output Path: " << it->second->tab_opath <<endl ) ;
		}
		ite = table_enqs.find( it->second ) ;
		if ( ite != table_enqs.end() )
		{
			llog( "-", "       Table Enqueue On: " << ite->second <<endl ) ;
		}
		llog( "-", "    Current Row Pointer: " << it->second->CRP <<endl ) ;
		if ( it->second->sarg.size() > 0 )
		{
			llog( "-", "Current Search Argument. " <<endl ) ;
			llog( "-", "        Condition Pairs: " << it->second->sa_cond_pairs <<endl ) ;
			llog( "-", "       Search Direction: " << it->second->sa_dir <<endl ) ;
			llog( "-", "    Extension Variables: " << it->second->sa_namelst <<endl ) ;
			for ( its = it->second->sarg.begin() ; its != it->second->sarg.end() ; ++its )
			{
				t = ( its->tbs_gen ) ? " (generic)" : "" ;
				llog( "-", "             Field Name: "+ its->tbs_var <<endl ) ;
				llog( "-", "            Field Value: "+ its->tbs_val + t <<endl ) ;
				llog( "-", "        Field Condition: "+ its->tbs_scond <<endl ) ;
			}
		}
		if ( it->second->sort_ir != "" )
		{
			llog( "-", "Sort Information Record: "+ it->second->sort_ir <<endl ) ;
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
						  << std::left << ite->second << endl) ;
		}
	}
	llog( "-", "*************************************************************************************************************" <<endl ) ;
}


void tableMGR::fillfVARs( errblock& err,
			  fPOOL& funcPOOL,
			  const string& tb_name,
			  const set<string>& tb_fields,
			  const set<string>& tb_clear,
			  bool scan,
			  int  depth,
			  int  posn,
			  int  csrrow,
			  int& idr )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBDISPL", tb_name, 12 ) ;
		return ;
	}
	it->second->fillfVARs( err, funcPOOL, tb_fields, tb_clear, scan, depth, posn, csrrow, idr ) ;
}


void tableMGR::tbcreate( errblock& err,
			 const string& tb_name,
			 string keys, string flds,
			 tbREP tb_REP,
			 tbWRITE tb_WRITE,
			 const string& tb_paths,
			 tbDISP tb_DISP )
{
	// RC =  0 Normal
	// RC =  4 Normal - duplicate table replaced
	// RC =  8 REPLACE not specified and table exists,
	//         or REPLACE specified with table open in SHARE or SHARE requested,
	//         or SHARE specified but a different task has it open NON_SHARE.
	// RC = 12 Table file in use.  Enqueue failure.
	// RC = 20 Severe error

	// Exists => With WRITE   - In storage and/or on disk
	//                NOWRITE - In storage only

	// Only allow 1 table shared, or any number non-shared for a given table name.

	// If WRITE specified obtain the enqueue for the duration the table is open.
	// Use first file in the input library if table not found.

	string filename = "" ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	iupper( keys ) ;
	iupper( flds ) ;

	multimap<string, Table*>::iterator it ;

	if ( tb_WRITE == WRITE )
	{
		filename = locate( err, tb_name, tb_paths ) ;
		if ( err.error() ) { return ; }
	}

	it = getTableIterator2( err, tb_name ) ;
	if ( it != tables.end() )
	{
		if ( tb_REP != REPLACE || it->second->tab_DISP == SHARE || tb_DISP == SHARE  )
		{
			err.setRC( 8 ) ;
			return ;
		}
		deq( it->second ) ;
		delete it->second ;
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
		err.seterrid( "PSYE013W", "TBCREATE", tb_name, 12 ) ;
		return ;
	}

	it = createTable( err, tb_name, keys, flds, tb_WRITE, tb_DISP ) ;

	if ( filename != "" )
	{
		enq( it->second, filename ) ;
	}
}


void tableMGR::tbget( errblock& err,
		      fPOOL& funcPOOL,
		      const string& tb_name,
		      const string& tb_savenm,
		      const string& tb_rowid_vn,
		      const string& tb_noread,
		      const string& tb_crp_name  )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBGET", tb_name, 12 ) ;
		return ;
	}
	it->second->tbget( err, funcPOOL, tb_savenm, tb_rowid_vn, tb_noread, tb_crp_name ) ;
}


void tableMGR::tbmod( errblock& err,
		      fPOOL& funcPOOL,
		      const string& tb_name,
		      const string& tb_namelst,
		      const string& tb_order )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBMOD", tb_name, 12 ) ;
		return ;
	}
	it->second->tbmod( err, funcPOOL, tb_namelst, tb_order ) ;
}


void tableMGR::tbput( errblock& err,
		      fPOOL& funcPOOL,
		      const string& tb_name,
		      const string& tb_namelst,
		      const string& tb_order )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBPUT", tb_name, 12 ) ;
		return ;
	}
	it->second->tbput( err, funcPOOL, tb_namelst, tb_order ) ;
}


void tableMGR::tbadd( errblock& err,
		      fPOOL& funcPOOL,
		      const string& tb_name,
		      const string& tb_namelst,
		      const string& tb_order,
		      int tb_num_of_rows )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBADD", tb_name, 12 ) ;
		return ;
	}
	it->second->tbadd( err, funcPOOL, tb_namelst, tb_order, tb_num_of_rows ) ;
}


void tableMGR::tbbottom( errblock& err,
			 fPOOL& funcPOOL,
			 const string& tb_name,
			 const string& tb_savenm,
			 const string& tb_rowid_vn,
			 const string& tb_noread,
			 const string& tb_crp_name  )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBBOTTOM", tb_name, 12 ) ;
		return ;
	}
	it->second->tbbottom( err, funcPOOL, tb_savenm, tb_rowid_vn, tb_noread, tb_crp_name ) ;
}


void tableMGR::tbdelete( errblock& err,
			 fPOOL& funcPOOL,
			 const string& tb_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBDELETE", tb_name, 12 ) ;
		return ;
	}
	it->second->tbdelete( err, funcPOOL ) ;
}


void tableMGR::tberase( errblock& err,
			const string& tb_name,
			const string& tb_paths )
{
	// RC =  0 Normal completion
	// RC =  8 Table not found
	// RC = 12 Table open
	// RC = 20 Severe error

	// Delete a table from the output library.  Table must not be opened in WRITE mode.

	string filename ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto ret = tables.equal_range( tb_name ) ;
	for ( auto it = ret.first ; it != ret.second ; ++it )
	{
		if ( it->second->tab_WRITE == WRITE )
		{
			err.seterrid( "PSYE013W", "TBERASE", tb_name, 12 ) ;
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
			err.seterrid( "PSYS012C", e.what() ) ;
			return ;
		}
		catch (...)
		{
			err.seterrid( "PSYS012C", "Entry: "+ filename ) ;
		}
	}
	else
	{
		err.setRC( 8 ) ;
	}
}


void tableMGR::tbexist( errblock& err,
			fPOOL& funcPOOL,
			const string& tb_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBEXIST", tb_name, 12 ) ;
		return ;
	}
	it->second->tbexist( err, funcPOOL ) ;
}


void tableMGR::tbquery( errblock& err,
			fPOOL& funcPOOL,
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

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBQUERY", tb_name, 12 ) ;
		return ;
	}
	it->second->tbquery( err,
			     funcPOOL,
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
}


void tableMGR::tbsarg( errblock& err,
		       fPOOL& funcPOOL,
		       const string& tb_name,
		       const string& tb_namelst,
		       const string& tb_dir,
		       const string& tb_cond_pairs )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBSARG", tb_name, 12 ) ;
		return ;
	}
	it->second->tbsarg( err, funcPOOL, tb_namelst, tb_dir, tb_cond_pairs ) ;
}


void tableMGR::tbscan( errblock& err,
		       fPOOL& funcPOOL,
		       const string& tb_name,
		       const string& tb_namelst,
		       const string& tb_savenm,
		       const string& tb_rowid_vn,
		       const string& tb_dir,
		       const string& tb_noread,
		       const string& tb_crp_name,
		       const string& tb_cond_pairs )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBSCAN", tb_name, 12 ) ;
		return ;
	}
	it->second->tbscan( err, funcPOOL, tb_namelst, tb_savenm, tb_rowid_vn, tb_dir, tb_noread, tb_crp_name, tb_cond_pairs ) ;
}


void tableMGR::tbskip( errblock& err,
		       fPOOL& funcPOOL,
		       const string& tb_name,
		       int num,
		       const string& tb_savenm,
		       const string& tb_rowid_vn,
		       const string& tb_rowid,
		       const string& tb_noread,
		       const string& tb_crp_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBSKIP", tb_name, 12 ) ;
		return ;
	}
	it->second->tbskip( err, funcPOOL, num, tb_savenm, tb_rowid_vn, tb_rowid, tb_noread, tb_crp_name ) ;
}


void tableMGR::tbsort( errblock& err,
		       const string& tb_name,
		       const string& tb_fields )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBSORT", tb_name, 12 ) ;
		return ;
	}
	it->second->tbsort( err, tb_fields ) ;
}


void tableMGR::tbtop( errblock& err,
		      const string& tb_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBTOP", tb_name, 12 ) ;
		return ;
	}
	it->second->tbtop( err ) ;
}


void tableMGR::tbvclear( errblock& err,
			 fPOOL& funcPOOL,
			 const string& tb_name )
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", "TBVCLEAR", tb_name, 12 ) ;
		return ;
	}
	it->second->tbvclear( err, funcPOOL ) ;
}


void tableMGR::cmdsearch( errblock& err,
			  fPOOL& funcPOOL,
			  string tb_name,
			  const string& cmd,
			  const string& paths,
			  bool  try_load )
{
	// Search table 'tb_name' for command 'cmd'.  Load table if not already in storage and try_load set.

	// Application command tables should already be loaded by SELECT processing so it is not
	// necessary to add LIBDEF ZTLIB/ZTUSR paths to 'paths' or to try to load these tables.

	// RC = 0  Okay
	// RC = 4  Command not found (from Table::cmdsearch() routine)
	// RC = 8  Table not found (loadTable() routine)
	// RC = 20 Severe error

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	tb_name += "CMDS" ;
	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		if ( not try_load )
		{
			err.setRC( 4 ) ;
			return ;
		}
		it = loadTable( err, tb_name, NOWRITE, paths, SHARE ) ;
		if ( it == tables.end() )
		{
			return ;
		}
	}
	it->second->cmdsearch( err, funcPOOL, cmd ) ;
}


bool tableMGR::writeableTable( errblock& err,
			       const string& tb_name,
			       const string& tb_func )
{
	// RC =  0 Normal completion
	// RC = 12 Table not open

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	auto it = getTableIterator1( err, tb_name ) ;
	if ( it == tables.end() )
	{
		err.seterrid( "PSYE013G", tb_func, tb_name, 12 ) ;
		return false ;
	}
	return ( it->second->tab_WRITE == WRITE ) ;
}


string tableMGR::locate( errblock& err,
			 const string& tb_name,
			 const string& tb_paths )
{
	int i ;
	int j ;

	string filename ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	for ( i = getpaths( tb_paths ), j = 1 ; j <= i ; ++j )
	{
		filename = getpath( tb_paths, j ) + tb_name ;
		try
		{
			if ( exists( filename ) )
			{
				if ( !is_regular_file( filename ) )
				{
					err.seterrid( "PSYE013L", filename ) ;
					return "" ;
				}
				return filename ;
			}
		}
		catch ( boost::filesystem::filesystem_error &e )
		{
			err.seterrid( "PSYS012C", e.what() ) ;
			return "" ;
		}
		catch (...)
		{
			err.seterrid( "PSYS012C", "Entry: "+ filename ) ;
			return "" ;
		}
	}

	return "" ;
}


multimap<string, Table*>::iterator tableMGR::getTableIterator1( errblock& err,
								const string& tb_name )
{
	//  Return the iterator for table tb_name.
	//  There can be only 1 shared table or any number of non-shared tables for a given table name.

	//  Return table match if it is in-use by the requestor, else tables.end()

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
	//  Return the iterator for table tb_name.
	//  There can be only 1 shared table or any number of non-shared tables for a given table name.

	//  Return table match if it is shared, or in-use by the requestor, else tables.end()

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


int tableMGR::enq( Table* tab,
		   const string& filename )
{
	table_enqs[ tab ] = filename ;

	return 0 ;
}


int tableMGR::deq( Table* tab )
{
	auto ite = table_enqs.find( tab ) ;
	if ( ite != table_enqs.end() )
	{
		table_enqs.erase( ite ) ;
	}

	return 0 ;
}


Table* tableMGR::qscan( const string& filename )
{
	// Return the table pointer for an enqueue.  NULL if not held.

	for ( auto ite = table_enqs.begin() ; ite != table_enqs.end() ; ++ite )
	{
		if ( filename == ite->second )
		{
			return ite->first ;
		}
	}

	return NULL ;
}
