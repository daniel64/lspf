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

#undef  MOD_NAME
#undef  LOGOUT

#define MOD_NAME pVPOOL
#define LOGOUT   aplog

using namespace boost::filesystem ;

// ************************************************************************** FUNCTION POOL SECTION *******************************************************************************************


void fPOOL::define( int & RC, string name, string * addr, nameCHCK check )
{
	fVAR var ;
	RC = 0   ;

	if ( (check == CHECK) && !isvalidName( name ) ) { RC = 20 ; return  ; }

	var.fVAR_string_ptr = addr   ;
	var.fVAR_type       = STRING ;
	var.fVAR_explicit   = true   ;
	POOL[ name ].push( var )     ;
}


void fPOOL::define( int & RC, string name, int * addr )
{
	fVAR var ;
	RC = 0   ;

	if ( !isvalidName( name ) ) { RC = 20 ; return ; }

	var.fVAR_int_ptr  = addr    ;
	var.fVAR_type     = INTEGER ;
	var.fVAR_explicit = true    ;
	POOL[ name ].push( var )    ;
}


bool fPOOL::ifexists( int & RC, string name, nameCHCK check )
{
	RC = 0 ;

	if ( (check == CHECK) && !isvalidName( name ) ) { RC = 20 ; return false ; }

	if ( POOL.find( name ) == POOL.end() ) { return false; }
	return true ;
}


dataType fPOOL::getType( int & RC, string name, nameCHCK check )
{
	RC = 0    ;

        if ( (check == CHECK) && !isvalidName( name ) ) { RC = 20 ; return ERROR ; }

	if ( POOL.find( name ) == POOL.end() ) { RC = 8 ; return ERROR ; }

	return POOL[ name ].top().fVAR_type ;
}



void fPOOL::put( int & RC, int maxRC, string name, string value, nameCHCK check )
{
	fVAR var ;
	map<string, stack< fVAR> >::iterator it ;

	RC = 0 ;

	if ( (check == CHECK) & !isvalidName( name ) ) { RC = 20 ; return  ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		var.fVAR_string   = value  ;
		var.fVAR_type     = STRING ;
		var.fVAR_explicit = false  ;
		POOL[ name ].push( var )   ;
		return ;
	}

	if ( POOL[ name ].top().fVAR_type != STRING )
	{
		RC = 20 ;
		log( "E", "Function pool definition of " << name << " is not of type string" << endl ) ;
		if ( RC > maxRC ) { RC = 20 ; log( "E", "Maximum return code of " << maxRC << " exceeded for fPOOL.put of " << name << " RC=8" << endl ) ; }
		return ;
	}

	if ( it->second.top().fVAR_explicit ) { *(it->second.top().fVAR_string_ptr) = value ; }
	else                                  {   it->second.top().fVAR_string      = value ; }
}


void fPOOL::put( int & RC, int maxRC, string name, int value )
{
	fVAR var ;
	map<string, stack< fVAR> >::iterator it ;

	RC = 0 ;

        if ( !isvalidName( name ) ) { RC = 20 ; return  ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		var.fVAR_int      = value   ;
		var.fVAR_type     = INTEGER ;
		var.fVAR_explicit = false   ;
		POOL[ name ].push( var )    ;
		return ;
	}

	if ( POOL[ name ].top().fVAR_type != INTEGER )
	{
		RC = 20 ;
		log( "E", "Function pool definition of " << name << " is not of type int" << endl ) ;
		if ( RC > maxRC ) { RC = 20 ; log( "E", "Maximum return code of " << maxRC << " exceeded for fPOOL.put of " << name << " RC=8" << endl ) ; }
		return ;

	}

	if ( it->second.top().fVAR_explicit ) { *(it->second.top().fVAR_int_ptr) = value ; }
	else                                  {   it->second.top().fVAR_int      = value ; }
}


string fPOOL::get( int & RC, int maxRC, string name, nameCHCK check )
{
	map<string, stack< fVAR> >::iterator it ;

	RC = 0 ;

	if ( (check == CHECK) & !isvalidName( name ) ) { RC = 20 ; return "" ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		RC = 8 ;
		if ( RC > maxRC ) { RC = 20 ; log( "E", "Maximum return code of " << maxRC << " exceeded for fPOOL.get of " << name << " RC=8" << endl ) ; }
		return "" ;
	}

	if ( it->second.top().fVAR_type != STRING )
	{
		RC = 20 ;
		log( "E", "Function pool definition of " << name << " is not of type string" << endl ) ;
		if ( RC > maxRC ) { RC = 20 ; log( "E", "Maximum return code of " << maxRC << " exceeded for fPOOL.get of " << name << " RC=8" << endl ) ; }
		return "" ;

	}

	if ( it->second.top().fVAR_explicit ) { return *it->second.top().fVAR_string_ptr ; }
	else                                  { return  it->second.top().fVAR_string     ; }
}


int fPOOL::get( int & RC, int maxRC, dataType type, string name )
{
	map<string, stack< fVAR> >::iterator it ;

	RC = 0 ;

	if ( !isvalidName( name ) ) { RC = 20  ; return 0 ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		RC = 8 ;
		if ( RC > maxRC ) { RC = 20 ; log( "E", "Maximum return code of " << maxRC << " exceeded for fPOOL.get of " << name << " RC=8" << endl ) ; }
		return 0 ;
	}

	if ( it->second.top().fVAR_type != type )
	{
		RC = 20 ;
		log( "E", "Function pool definition of " << name << " is not of type int" << endl ) ;
		if ( RC > maxRC ) { RC = 20 ; log( "E", "Maximum return code of " << maxRC << " exceeded for fPOOL.get of " << name << " RC=8" << endl ) ; }
		return 0 ;

	}
	if ( it->second.top().fVAR_explicit ) { return *it->second.top().fVAR_int_ptr ; }
	else                                  { return  it->second.top().fVAR_int     ; }
}


void fPOOL::dlete( int & RC, string name, nameCHCK check )
{
	map<string, stack< fVAR> >::iterator it ;

	RC = 0 ;

	if ( (check == CHECK) & !isvalidName( name ) ) { RC = 20 ; return ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		RC = 8 ;
		return ;
	}

	if ( POOL[ name ].size() == 1 )
	{
		POOL.erase( it ) ;
	}
	else
	{
		POOL[ name ].pop() ;
	}
}


void fPOOL::dlete_gen( int & RC, string name )
{
	// After an erase, the iterator is invalidated.  Set to previous position or begin() if 
	// the first element is being erased.
	// RC = 0  Normal completion with at least one variable deleted
	// RC = 8  No variables found to delete

	bool fst ;

	map<string, stack< fVAR> >::iterator it1 ;
	map<string, stack< fVAR> >::iterator it2 ;

	RC  = 8    ;
	fst = true ;

	for ( it1 = POOL.begin() ; it1 != POOL.end() ; )
	{
		if ( it1->first[0] < name[0] ) { it1++ ; continue ; }
		if ( it1->first[0] > name[0] ) { break ; }
		if ( abbrev( it1->first, name ) ) 
		{
			POOL.erase( it1 ) ;
			RC = 0 ;
			if ( fst ) { it1 = POOL.begin() ; }
			else       { it1 = it2 ; it1++  ; }
		}
		else { it2 = it1 ; it1++ ; fst = false ; }
	}
}


void fPOOL::reset()
{
	int RC ;
	RC = 0 ;

	POOL.clear() ;
}


// ************************************************************************** VARIABLE POOL SECTION *******************************************************************************************


void pVPOOL::put( int & RC, string name, string value, vTYPE vtype )
{
	pVAR val ;
	map<string, pVAR>::iterator it ;

	RC = 0 ;

	if ( !isvalidName( name ) ) { RC = 20 ; return  ; }
	if ( readOnly )             { RC = 12 ; return  ; }
	if ( value.size() > 32767 ) { RC = 16 ; value.resize( 32767 ) ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		val.pVAR_value  = value ;
		val.pVAR_system = ( vtype == SYSTEM ) ;
		POOL[ name ]    = val  ;
		changed         = true ;
	}
	else
	{
		if ( it->second.pVAR_system && vtype != SYSTEM ) { RC = 20 ;                                        }
		else						 { it->second.pVAR_value = value ; changed = true ; }
	}
}


string pVPOOL::get( int & RC, string name )
{
	map<string, pVAR>::iterator it ;

	RC = 0 ;

	if ( !isvalidName( name ) ) { RC = 20 ; return NULL ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() ) { RC = 8 ; return NULL ; }

	return it->second.pVAR_value ;
}


void pVPOOL::erase( int & RC, string name )
{
	map<string, pVAR>::iterator it;

	RC = 0 ;

	if ( !isvalidName( name ) ) { RC = 20 ; return  ; }
	if ( readOnly )             { RC = 12 ; return  ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() ) { RC = 8 ; return ; }

	if ( it->second.pVAR_system ) { RC = 20 ;                           }
	else                          { POOL.erase( it ) ; changed = true ; }
}


bool pVPOOL::isSystem( int & RC, string name )
{
	map<string, pVAR>::iterator it ;

	RC = 0 ;

	if ( !isvalidName( name ) ) { RC = 20 ; return false ; }

	it = POOL.find( name ) ;
	if ( it == POOL.end() ) { RC = 8 ; return false ; }

	return it->second.pVAR_system ;
}


void pVPOOL::load( int & RC, string currAPPLID, string path )
{
	string s     ;
	string hdr   ;
	string var   ;
	string value ;

	char x, y    ;
	int  i, k    ;
	int  n1, n2  ;
	char * buf1  ;
	size_t buf1Size = 1024  ;

	s = path + currAPPLID + "PROF" ;

	ifstream profile ;
	debug1(" profile dataset is " << s << endl ) ;
	profile.open( s.c_str() , ios::binary ) ;

	buf1 = new char[ buf1Size ] ;

	profile.read (buf1 , 2);
	if ( memcmp( buf1, "\x00\x84", 2 ) )
	{
		RC = 20 ;
		log( "E", "Not a valid PROFILE file " << currAPPLID << ".  Appears corrupt" << endl );
		profile.close() ;
		delete[] buf1   ;
		return  ;
	}

	profile.get( x ) ;
	i = static_cast< int >( x ) ;
	if ( i > 1 )
	{
		RC = 20 ;
		log( "E", "Invalid profile format version.  Version " << i << " not supported or profile is corrupt for " << currAPPLID << endl ) ;
		profile.close() ;
		delete[] buf1   ;
		return  ;
	}

	profile.get( x ) ;
      	i = static_cast< int >( x ) ;
	if ( i < 0 ) { i = 256 + i ; }
       	profile.read (buf1 , i);
	hdr.assign( buf1, i ) ;
        debug1(" PROFILE Header " << hdr << endl ) ;

	while ( true )
	{
		profile.get( x ) ;
		if ( profile.fail() != 0 ) { break ; } ; 
		i = static_cast< int >( x ) ;
		if ( i < 0 ) { i = 256 + i ; }
		profile.read (buf1 , i) ;
		var.assign( buf1, i )   ;
		profile.get( x ) ;
		if ( profile.fail() != 0 ) { RC = 20 ; break ; }
		profile.get( y ) ;
		if ( profile.fail() != 0 ) { RC = 20 ; break ; }
		n1 = static_cast< int >( x ) ;
		n2 = static_cast< int >( y ) ;
		if ( n1 < 0 ) { n1 = 256 + n1 ; }
		if ( n2 < 0 ) { n2 = 256 + n2 ; }
		k = 256 * n1 + n2 ;
		if ( k > buf1Size )
		{
			delete[] buf1 ;
			buf1Size = k  ;
			buf1     = new char[ buf1Size ] ;
		}
		profile.read (buf1, k ) ;
		if ( profile.fail() != 0 ) { RC = 20 ; break ; }
		value.assign( buf1, k ) ;
		debug2("Restoring " << var << " to value >>" << value << "<<" << endl ) ;
		put( RC, var, value ) ;
		debug2( "Result of put RC = " << RC << endl ) ;
	}
	profile.close() ;
	delete[] buf1   ;
	if ( RC > 0 )
	{
		log( "E", "Unexpected EOF reading saved profile variables.  Profile " << currAPPLID << " appears corrupt" << endl );
	}
	else
	{
		log( "I", "Pool " << currAPPLID << " restored from saved variables in profile dataset " << s << endl ) ;
	}
	resetChanged() ;
}


void pVPOOL::save( int & RC, string currAPPLID )
{
	string s ;
	int    i ;

	log( "I", "Saving pool " << currAPPLID << " to path " << path << endl ) ;

	if ( !changed )
	{
		RC = 4  ;
		log( "I", "No variables in the pool have been changed.  Ignoring save" <<  endl ) ;
		return  ;
	}
	if ( readOnly )
	{
		RC = 4  ;
		log( "I", "Cannot save a pool in read only mode" <<  endl ) ;
		return  ;
	}

	if ( exists( path ) )
	{
		if ( !is_directory( path ) )
		{
			log( "E", "Directory " << path << " is not a regular directory for profile save" <<  endl ) ;
			RC = 20 ;
			return  ;
		}
	}
	else
	{
		log( "E", "Directory " << path << " does not exist for profile save " <<  endl ) ;
		RC = 20 ;
		return  ;
	}
	if ( path.back() != '/' ) { path = path + "/" ; }
	s = path + currAPPLID + "PROF" ;

	ofstream profile ;
	profile.open( s.c_str(), ios::binary | ios::out ) ;
	profile << (char)00  ;  //
	profile << (char)132 ;  // x084 denotes a profile 
	profile << (char)1   ;  // PROFILE format version 1
	profile << (char)44  ;  // Header length
	profile << "HDR                                         " ;

	map< string, pVAR >::iterator it ;
	for ( it = POOL.begin() ; it != POOL.end() ; it++ )
	{
		debug2( "Saving profile variable " << it->first << " for pool " << currAPPLID << endl ) ;
		debug2( "value: " << it->second.pVAR_value << endl ) ;
		i = it->first.size() ;
		profile << (char)i ;
		profile.write (it->first.c_str(), i) ;
		i = it->second.pVAR_value.size() ;
		profile << (char)( i >> 8 ) ;
		profile << (char)( i ) ;
		profile.write (it->second.pVAR_value.c_str(), i) ;
	}
	profile.close() ;
	resetChanged()  ;
	debug1( "Saved pool okay to filename " << s << endl ) ;
}


// ************************************************************************** POOL MANAGER SECTION *******************************************************************************************


poolMGR::poolMGR()
{
	log( "I", "Constructor Creating SYSTEM and DEFAULT pools " << endl ) ;

	pVPOOL pool ;

	POOLs_shared [ "@DEFSHAR" ]  = pool ;
	POOLs_profile[ "@DEFPROF" ]  = pool ;
	POOLs_profile[ "@ROXPROF" ]  = pool ;
	shrdPooln = 0 ;
}


void poolMGR::setPOOLsreadOnly( int & RC )
{
	RC = 0    ;

	POOLs_profile[ "@ROXPROF" ].setreadOnly() ;
	POOLs_profile[ "@DEFPROF" ].setreadOnly() ;
}


void poolMGR::defaultVARs( int & RC, string name, string value, poolType pType )
{
	RC = 0    ;

	switch ( pType )
	{
	case SHARED:
		POOLs_shared[ "@DEFSHAR" ].put( RC, name, value, SYSTEM ) ;
		break ;
	case PROFILE:
		POOLs_profile[ "@DEFPROF" ].put( RC, name, value, SYSTEM ) ;
		break ;
	default:
		RC = 0 ;
	}
}


void poolMGR::createPool( int & RC, poolType pType, string path )
{
	string value ;
	string var   ;
	string s     ;

	map<string, pVPOOL>::iterator sp_it   ;
	map<string, pVPOOL>::iterator pp_it   ;

	pVPOOL pool ;

	RC = 0 ;

	switch( pType )
	{
	case SHARED:
		shrdPooln++ ;
		shrdPool = "#" + right( d2ds( shrdPooln ), 7, '0' ) ;
		debug1( "New shared POOL name " << shrdPool << endl ) ;
		sp_it = POOLs_shared.find( shrdPool ) ;
		if ( sp_it != POOLs_shared.end() )
		{
			RC = 20 ;
			log( "C", "SHARED POOL " << shrdPool << " already exists.  Logic Error " << endl ) ;
			return ;
		}
		pool.path = "" ;
		POOLs_shared[ shrdPool ]           = pool ;
		POOLs_shared[ shrdPool ].refCount  = 1    ;
		break ;
	case PROFILE:
		debug1( "New profile pool name " << currAPPLID << endl ) ;
		pp_it = POOLs_profile.find( currAPPLID ) ;
		if ( pp_it == POOLs_profile.end() )
		{
			if ( path.back() != '/' ) { path = path + "/" ; }
			s = path + currAPPLID + "PROF" ;
			if ( exists( s ) )
			{
				if ( !is_regular_file( s ) )
				{
					log( "E", "File " << s << " is not a regular file for profile load" <<  endl ) ;
					RC = 20 ;
				}
				else
				{
					pool.path = path ;
					POOLs_profile[ currAPPLID ] = pool      ;
					POOLs_profile[ currAPPLID ].refCount = 1 ;
					log( "I", "Pool " << currAPPLID << " created okay.  Reading saved variables from profile dataset" << endl ) ;
					POOLs_profile[ currAPPLID ].load(  RC, currAPPLID, path ) ;
				}
			}
			else
			{
				if ( exists( path ) )
				{
					if ( !is_directory( path ) )
					{
						log( "E", "Directory " << path << " is not a regular directory for profile load" <<  endl ) ;
						RC = 20 ;
					}
					else
					{
						log( "I", "Profile " << currAPPLID + "PROF does not exist.  Creating default" <<  endl ) ;
						pool.path = path                           ;
						POOLs_profile[ currAPPLID ] = pool        ;
						pp_it = POOLs_profile.find( currAPPLID ) ;
						POOLs_profile[ currAPPLID ].refCount = 1   ;
						log( "I", "Profile Pool " << currAPPLID << " created okay in path " << path << endl ) ;
					}
				}
				else
				{
					log( "E", "Directory " << path << " does not exist for profile load" <<  endl ) ;
					RC = 20 ;
				}
			}
		}
		else
		{
			log( "I", "Pool " << currAPPLID << " already exists.  Incrementing use count " << endl ) ;
			POOLs_profile[ currAPPLID ].refCount++ ;
		}
		break ;
	default:
		RC = 20 ;
	}
}


void poolMGR::destroyPool( int & RC, poolType pType )
{

	string s ;
	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator pp_it ;
	map<string, pVAR>::iterator v_it    ;

	RC = 0   ;

	switch( pType )
	{
	case SHARED:
		log( "I", "Destroying pool " << shrdPool << endl ) ;
		sp_it = POOLs_shared.find( shrdPool ) ;
		if ( sp_it == POOLs_shared.end() )
		{
			RC = 20 ;
			log( "C", "poolMGR cannot find SHARED POOL " << shrdPool << " Logic error" << endl ) ;
			return ;
		}
		if ( POOLs_shared[ shrdPool ].refCount == 1 )
		{
			POOLs_shared.erase( shrdPool ) ;
			log( "I", "Pool " << shrdPool << " destroyed okay " << endl ) ;
		}
		else
		{
			POOLs_shared[ shrdPool ].refCount-- ;
			log( "I", "Pool " << shrdPool << " still in use.  Decrementing use count to " << POOLs_shared[ shrdPool ].refCount << endl ) ;
		}
		break ;
	case PROFILE:
		log( "I", "Destroying pool " << currAPPLID << endl ) ;
		pp_it = POOLs_profile.find( currAPPLID ) ;
		if ( pp_it == POOLs_profile.end() )
		{
			RC = 20 ;
			log( "C", "poolMGR cannot find profile pool " << currAPPLID << " Logic error" << endl ) ;
			return ;
		}
		if ( POOLs_profile[ currAPPLID ].refCount == 1 )
		{
			POOLs_profile[ currAPPLID ].save( RC, currAPPLID ) ;
			if ( RC > 4 )
			{
				log( "E", "Pool " << currAPPLID << " cannot be saved" << endl ) ;
				return ;
			}
			POOLs_profile.erase( currAPPLID ) ;
			log( "I", "Pool " << currAPPLID << " destroyed okay " << endl ) ;
		}
		else
		{
			POOLs_profile[ currAPPLID ].refCount-- ;
			log( "I", "Pool " << currAPPLID << " still in use.  Decrementing use count to " << POOLs_profile[ currAPPLID ].refCount << endl ) ;
		}
		break ;
	default:
		RC = 20 ;
	}
}


void poolMGR::statistics()
{
	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator pp_it ;
	string Mode ;

	log( "-", "POOL STATISTICS:" << endl ) ;
	log( "-", "         Current APPLID . . . . . . . . " << currAPPLID << endl ) ;
	log( "-", "         Current SHARED POOL ID . . . . " << shrdPool << endl ) ;
	log( "-", "         Number of shared pools . . . . " << POOLs_shared.size() << endl ) ;
	log( "-", "         Number of profile pools. . . . " << POOLs_profile.size() << endl ) ;
	log( "-", "" << endl ) ;
	log( "-", "         Shared pool details:" << endl ) ;
	for ( sp_it = POOLs_shared.begin() ; sp_it != POOLs_shared.end() ; sp_it++ )
	{
		if ( sp_it->second.readOnly ) { Mode = "RO" ; }
		else                          { Mode = "UP" ; }
		log( "-", "            Pool " << setw(8) << sp_it->first << "  use count: " << setw(4) << POOLs_shared[ sp_it->first ].refCount <<
		          "  " << Mode << "  entries: " << setw(5) << sp_it->second.POOL.size() << endl ) ;
	}
	log( "-", "" << endl ) ;
	log( "-", "         PROFILE pool details:" << endl ) ;
	for ( pp_it = POOLs_profile.begin() ; pp_it != POOLs_profile.end() ; pp_it++ )
	{
		if ( pp_it->second.readOnly ) { Mode = "RO" ; }
		else                          { Mode = "UP" ; }
		log( "-", "            Pool " << setw(8) << pp_it->first << "  use count: " << setw(4) << POOLs_profile[ pp_it->first ].refCount <<
		          "  " << Mode << "  entries: " << setw(5) << pp_it->second.POOL.size() << "  path: " << POOLs_profile[ pp_it->first ].path << endl ) ;
	}
	log( "-", "*************************************************************************************************************" << endl ) ;
}


void poolMGR::snap()
{
	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator pp_it ;
	map<string, pVAR>::iterator v_it    ;
	string vtype ;

	RC = 0 ;

	log( "-", "POOL VARIABLES:" << endl ) ;
	log( "-", "         Shared pool details:" << endl ) ;
	for ( sp_it = POOLs_shared.begin() ; sp_it != POOLs_shared.end() ; sp_it++ )
	{
		log( "-", "         Pool " << setw(8) << sp_it->first << " use count:" << setw(3) << POOLs_shared[ sp_it->first ].refCount <<
		          " entries: " << sp_it->second.POOL.size() << endl ) ;
		for ( v_it = sp_it->second.POOL.begin() ; v_it != sp_it->second.POOL.end() ; v_it++ )
		{
			if ( sp_it->second.isSystem( RC, v_it->first ) ) { vtype = "S" ; }
			else                                             { vtype = "N" ; }
			log( "-", setw(8) << v_it->first << " :" << vtype << ": " << sp_it->second.get( RC, v_it->first ) << "<< " << endl ) ;
		}
	}
	log( "-", endl ) ;
	log( "-", "         PROFILE pool details:" << endl ) ;
	for ( pp_it = POOLs_profile.begin() ; pp_it != POOLs_profile.end() ; pp_it++ )
	{
		log( "-", "         Pool " << setw(8) << pp_it->first << " use count: " << setw(3) << POOLs_profile[ pp_it->first ].refCount <<
		          " entries: " << pp_it->second.POOL.size() << endl ) ;
		log( "-", "                            path: " << setw(3) << POOLs_profile[ pp_it->first ].path << endl ) ;
		for ( v_it = pp_it->second.POOL.begin() ; v_it != pp_it->second.POOL.end() ; v_it++ )
		{
			if ( sp_it->second.isSystem( RC, v_it->first ) ) { vtype = "S" ; }
			else                                             { vtype = "N" ; }
			log( "-", setw(8) << v_it->first << " :" << vtype << ": " << pp_it->second.get( RC, v_it->first ) << "<< " << endl ) ;
		}
	}
	log( "-", "*************************************************************************************************************" << endl ) ;
}


void poolMGR::setAPPLID( int & RC, string m_APPLID )
{
	RC = 0 ;

        if ( !isvalidName4( m_APPLID ) )
	{
		RC = 20 ;
		log( "C", "Invalid APPLID name format passed to pool manager >>" << m_APPLID << "<<" << endl ) ;
		return  ;
	}

	currAPPLID = m_APPLID ;
}


void poolMGR::setshrdPool( int & RC, string m_shrdPool )
{
	map<string, pVPOOL>::iterator sp_it ;

	RC = 0 ;

	sp_it = POOLs_shared.find( m_shrdPool ) ;
	if ( sp_it == POOLs_shared.end() )
	{
		RC = 20 ;
		log( "C", "poolMGR cannot find pool " << m_shrdPool << ".  Pool must be created before setting pool name" << endl ) ;
		return  ;
	}
	shrdPool = m_shrdPool ;
}


string poolMGR::vlist( int & RC, poolType pType, int lvl )
{
	string vlist ;
	map<string, pVPOOL>::iterator p_it ;
	map<string, pVAR>::iterator   v_it ;

	RC    = 0  ;
	vlist = "" ;

	switch( pType )
	{
	case SHARED:
		switch ( lvl )
		{
		case 1:
			p_it = POOLs_shared.find( shrdPool ) ;
			break ;
		case 2:
			p_it = POOLs_shared.find( "@DEFSHAR" ) ;
			break ;
		default:
			RC = 20   ;
			return "" ;
		}
		break ;
	case PROFILE:
		switch ( lvl )
		{
		case 1:
			p_it = POOLs_profile.find( currAPPLID ) ;
			break ;
		case 2:
			p_it = POOLs_profile.find( "@ROXPROF" ) ;
			break ;
		case 3:
			p_it = POOLs_profile.find( "@DEFPROF" ) ;
			break ;
		case 4:
			p_it = POOLs_profile.find( "ISPS" ) ;
			break ;
		default:
			RC = 20   ;
			return "" ;
		}
		break ;

	default:
		RC = 20   ;
		return "" ;
	}
	for ( v_it = p_it->second.POOL.begin() ; v_it != p_it->second.POOL.end() ; v_it++ )
	{
		vlist = vlist + " " + v_it->first ;
	}
	return vlist ;
}


bool poolMGR::ifexists( int & RC, string name )
{
	// Pool search order: SHARED then PROFILE
	// RC = 0  variable found, return true
	// RC = 8  variable not found, return false
	// RC = 20 severe error

	map<string, pVPOOL>::iterator p_it  ;
	RC = 0 ;

        if ( !isvalidName( name ) ) { RC = 20 ; return false ; }

	locateSubPool( RC, p_it, name, SHARED ) ;
	if ( RC == 0 ) { return true ; }
	else if ( RC == 8 )
	{
		locateSubPool( RC, p_it, name, PROFILE ) ;
		if ( RC == 0 ) { return true ; }
	}
	return false ;
}



void poolMGR::put( int & RC, string name, string value, poolType pType, vTYPE vtype )
{
	// Pool search order:  ASIS - SHARED then PROFILE
	// RC = 0  variable put okay
	// RC = 12 variable not put as in read-only status
	// RC = 20 severe error
	// If poolType is PROFILE, remove variable from the SHARED pool even if it is not found in the PROFILE pool
	// Don't propergate back this return code from the SHARED pool erase try

	int RC2 ;

	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator pp_it ;
	map<string, pVPOOL>::iterator p_it  ;

	RC  = 0 ;
	RC2 = 0 ;

        if ( !isvalidName( name ) ) { RC = 20 ; return ; }

	sp_it = POOLs_shared.find( shrdPool ) ;
	if ( sp_it == POOLs_shared.end() )
	{
		RC = 20 ;
		log( "C", "poolMGR cannot find pool " << shrdPool << " Logic error" << endl ) ;
		return  ;
	}

	switch( pType )
	{
	case ASIS:
		locateSubPool( RC, p_it, name, SHARED ) ;
		if ( RC == 0 ) { p_it->second.put( RC, name, value, vtype ) ; }
		else if ( RC == 8 )
		{
			locateSubPool( RC, p_it, name, PROFILE ) ;
			if ( RC == 0 ) { p_it->second.put( RC, name, value, vtype ) ; }
			else if ( RC == 8 ) { sp_it->second.put( RC, name, value, vtype ) ; }
		}
		break ;
	case SHARED:
		locateSubPool( RC, p_it, name, SHARED ) ;
		if ( RC == 0 )      {  p_it->second.put( RC, name, value, vtype ) ; }
		else if ( RC == 8 ) { sp_it->second.put( RC, name, value, vtype ) ; }
		break ;
	case PROFILE:
		pp_it = POOLs_profile.find( currAPPLID ) ;
		locateSubPool( RC, p_it, name, PROFILE ) ;
		if ( RC == 0 )      {  p_it->second.put( RC, name, value, vtype ) ; }
		else if ( RC == 8 ) { pp_it->second.put( RC, name, value, vtype ) ; }
		locateSubPool( RC2, p_it, name, SHARED ) ;
		if ( RC2 == 0 )     {  p_it->second.erase( RC2, name ) ; }
		break ;
	default:
		RC = 20 ;
		return  ;
	}
	debug2( "....PUT end RC/Appl/Shr/name/value " << RC << " " << currAPPLID << " " << shrdPool << " " << name << " " << value << endl ) ;
}


string poolMGR::get( int & RC, string name, poolType pType )
{
	// Pool search order: ASIS - SHARED then PROFILE
	// RC = 0  variable found
	// RC = 8  variable not found
	// RC = 20 severe error

	map<string, pVPOOL>::iterator p_it  ;
	RC = 0 ;

	debug2( "....GET function APPL/name/pType/ " << currAPPLID << " " << name << " " << pType << endl ) ;

        if ( !isvalidName( name ) ) { RC = 20 ; return "" ; }

	switch ( pType )
	{
	case ASIS:
		locateSubPool( RC, p_it, name, SHARED ) ;
		if ( RC == 0 ) { return p_it->second.get( RC, name ) ; }
		else if ( RC == 8 )
		{
			locateSubPool( RC, p_it, name, PROFILE ) ;
			if ( RC == 0 ) { return p_it->second.get( RC, name ) ; }
		}
		break ;
	case PROFILE:
		locateSubPool( RC, p_it, name, PROFILE ) ;
		if ( RC == 0 ) { return p_it->second.get( RC, name ) ; }
		break ;
	case SHARED:
		locateSubPool( RC, p_it, name, SHARED ) ;
		if ( RC == 0 ) { return p_it->second.get( RC, name ) ; }
		break ;
	default:
		RC = 20   ;
		log( "C", "poolMGR get function passed invalid POOL TYPE." << pType <<  " Logic error" << endl ) ;
	}
	return "" ;
}


void poolMGR::erase( int & RC, string name, poolType pType )
{
	// Pool search order: ASIS - SHARED then PROFILE
	// RC = 0  variable found and erased
	// RC = 8  variable not found
	// RC = 12 variable not erased as in read-only status
	// RC = 20 severe error

	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator p_it ;
	map<string, pVAR>::iterator v_it   ;

	RC = 0 ;

	if ( !isvalidName( name ) ) { RC = 20 ; return  ; }

	sp_it = POOLs_shared.find( shrdPool ) ;
	if ( sp_it == POOLs_shared.end() )
	{
		RC = 20 ;
		log( "C", "poolMGR has invalid APPLID.  Logic error." << endl ) ;
		return  ;
	}

	switch( pType )
	{
	case ASIS:
		locateSubPool( RC, p_it, name, SHARED ) ;
		if ( RC == 0 ) { p_it->second.erase( RC, name ) ; }
		else if ( RC == 8 )
		{
			locateSubPool( RC, p_it, name, PROFILE ) ;
			if ( RC == 0 ) { p_it->second.erase( RC, name ) ; }
		}
		break ;
	case PROFILE:
		locateSubPool( RC, p_it, name, PROFILE ) ;
		if ( RC == 0 ) { p_it->second.erase( RC, name ) ; }
		break ;
	case SHARED:
		locateSubPool( RC, p_it, name, SHARED ) ;
		if ( RC == 0 ) { p_it->second.erase( RC, name ) ; }
		break ;
	case BOTH:
		locateSubPool( RC, p_it, name, SHARED ) ;
		if ( RC == 0 ) { p_it->second.erase( RC, name ) ; }
		locateSubPool( RC, p_it, name, PROFILE ) ;
		if ( RC == 0 ) { p_it->second.erase( RC, name ) ; }
		break ;
	default:
		RC = 20 ;
		log( "C", "poolMGR erase function passed invalid POOL TYPE." << pType <<  " Logic error" << endl ) ;
		return  ;
	}
}


void poolMGR::locateSubPool( int & RC, map<string, pVPOOL>::iterator & p_it, string name, poolType pType )
{
	// Locate the sub-pool for variable name.
	// RC=0 variable found.  Pool iterator, p_it, will be valid on return
	// RC=8 if variable not found

	// Sub-Pool search order
	// SHARED: CURRENT @DEFSHAR
	// PROFILE: APPLID, @ROXPROF @DEFPROF ISPS

	map<string, pVAR>::iterator v_it ;

	RC = 0 ;
	switch ( pType )
	{
	case PROFILE:
		p_it = POOLs_profile.find( currAPPLID ) ;
		v_it = p_it->second.POOL.find( name ) ;
		if ( v_it == p_it->second.POOL.end() )
		{
			p_it = POOLs_profile.find( "@ROXPROF" ) ;
			v_it = p_it->second.POOL.find( name ) ;
			if ( v_it == p_it->second.POOL.end() )
			{
				p_it = POOLs_profile.find( "@DEFPROF" ) ;
				v_it = p_it->second.POOL.find( name ) ;
				if ( v_it == p_it->second.POOL.end() )
				{
					p_it = POOLs_profile.find( "ISPS" ) ;
					v_it = p_it->second.POOL.find( name ) ;
					if ( v_it == p_it->second.POOL.end() ) { RC = 8 ; }
				}
			}
		}
		break ;
	case SHARED:
		p_it = POOLs_shared.find( shrdPool ) ;
		v_it = p_it->second.POOL.find( name ) ;
		if ( v_it == p_it->second.POOL.end() )
		{
			p_it = POOLs_shared.find( "@DEFSHAR" ) ;
			v_it = p_it->second.POOL.find( name )  ;
			if ( v_it == p_it->second.POOL.end() ) { RC = 8 ; }
		}
		break ;
	default:
		RC = 20   ;
		log( "C", "poolMGR locateSubPool() function passed invalid POOL TYPE." << pType <<  " Logic error" << endl ) ;
		return ;
	}
	return ;
}
