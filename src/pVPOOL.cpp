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

// ******************************************************************************************************************************
// ************************************************** FUNCTION POOL SECTION *****************************************************
// ******************************************************************************************************************************


void fPOOL::define( errblock& err, const string& name, string * addr, nameCHCK check )
{
	fVAR var ;

	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool DEFINE", name ) ;
		return ;
	}

	var.fVAR_string_ptr = addr   ;
	var.fVAR_type       = STRING ;
	var.fVAR_defined    = true   ;
	POOL[ name ].push( var )     ;
}


void fPOOL::define( errblock& err, const string& name, int * addr )
{
	fVAR var ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool DEFINE", name ) ;
		return ;
	}

	var.fVAR_int_ptr = addr    ;
	var.fVAR_type    = INTEGER ;
	var.fVAR_defined = true    ;
	POOL[ name ].push( var )   ;
}


void fPOOL::dlete( errblock& err, const string& name, nameCHCK check )
{
	// Remove the vdefine for a variable from the function pool.

	// Use * for all vdefined variables (except system variables that have been defined in the
	// pApplication constructor)

	// RC =  0 OK
	// RC =  8 Variable not found in the defined area of the function pool
	// RC = 20 Severe error

	map<string, stack< fVAR> >::iterator it ;

	const string vdsys( "ZCURFLD ZCURPOS ZTDMARK ZTDDEPTH ZTDROWS ZTDSELS ZTDTOP ZTDVROWS ZAPPNAME" ) ;

	err.setRC( 0 ) ;

	if ( name == "*")
	{
		for ( it = POOL.begin() ; it != POOL.end() ; it++ )
		{
			while ( !findword( it->first, vdsys ) && it->second.top().fVAR_defined )
			{
				it = POOL.erase( it ) ;
				if ( it == POOL.end() ) { return ; }
			}
		}
		return ;
	}

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool DELETE", name ) ;
		return ;
	}

	it = POOL.find( name ) ;

	if ( it == POOL.end() || !it->second.top().fVAR_defined )
	{
		err.setRC( 8 ) ;
		return ;
	}

	if ( it->second.size() == 1 ) { POOL.erase( it ) ; }
	else                          { it->second.pop() ; }
}


string fPOOL::get( errblock& err, int maxRC, const string& name, nameCHCK check )
{
	map<string, stack< fVAR> >::iterator it ;

	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool GET", name ) ;
		return "";
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		if ( 8 > maxRC )
		{
			err.seterrid( "PSYE013B", d2ds( maxRC ), name ) ;
		}
		return "" ;
	}

	if ( it->second.top().fVAR_type != STRING )
	{
		err.seterrid( "PSYE013C", name ) ;
		return "" ;
	}

	if ( it->second.top().fVAR_defined ) { return *it->second.top().fVAR_string_ptr ; }
	else                                 { return  it->second.top().fVAR_string     ; }
}


int fPOOL::get( errblock& err, int maxRC, dataType type, const string& name )
{
	map<string, stack< fVAR> >::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool GET", name ) ;
		return 0 ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		if ( 8 > maxRC )
		{
			err.seterrid( "PSYE013B", d2ds( maxRC ), name ) ;
		}
		return 0 ;
	}

	if ( it->second.top().fVAR_type != type )
	{
		err.seterrid( "PSYE013D", name ) ;
		return 0 ;
	}
	if ( it->second.top().fVAR_defined ) { return *it->second.top().fVAR_int_ptr ; }
	else                                 { return  it->second.top().fVAR_int     ; }
}


dataType fPOOL::getType( errblock& err, const string& name, nameCHCK check )
{
	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool GETTYPE", name ) ;
		return ERROR ;
	}


	if ( POOL.find( name ) == POOL.end() )
	{
		err.setRC( 8 ) ;
		return ERROR ;
	}

	return POOL[ name ].top().fVAR_type ;
}


bool fPOOL::ifexists( errblock& err, const string& name, nameCHCK check )
{
	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool IFEXISTS", name ) ;
		return false ;
	}

	return POOL.find( name ) != POOL.end() ;
}


void fPOOL::put( errblock& err, const string& name, const string& value, nameCHCK check )
{
	// RC =  0 OK
	// RC = 20 Severe error

	fVAR var ;
	map<string, stack< fVAR> >::iterator it ;

	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool PUT", name ) ;
		return ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		var.fVAR_string   = value  ;
		var.fVAR_type     = STRING ;
		var.fVAR_defined  = false  ;
		POOL[ name ].push( var )   ;
		return ;
	}

	if ( it->second.top().fVAR_type != STRING )
	{
		err.seterrid( "PSYE013C", name ) ;
		return ;
	}

	if ( it->second.top().fVAR_defined ) { *(it->second.top().fVAR_string_ptr) = value ; }
	else                                 {   it->second.top().fVAR_string      = value ; }
}


void fPOOL::put( errblock& err, const string& name, int value )
{
	// RC =  0 OK
	// RC = 20 Severe error

	fVAR var ;
	map<string, stack< fVAR> >::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool PUT", name ) ;
		return ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		var.fVAR_int      = value   ;
		var.fVAR_type     = INTEGER ;
		var.fVAR_defined  = false   ;
		POOL[ name ].push( var )    ;
		return ;
	}

	if ( it->second.top().fVAR_type != INTEGER )
	{
		err.seterrid( "PSYE013D", name ) ;
		return ;
	}

	if ( it->second.top().fVAR_defined ) { *(it->second.top().fVAR_int_ptr) = value ; }
	else                                 {   it->second.top().fVAR_int      = value ; }
}


void fPOOL::reset( errblock& err )
{
	err.setRC( 0 ) ;
	POOL.clear() ;
}


void fPOOL::setmask( errblock& err, const string& name, const string& mask )
{
	map<string, stack< fVAR> >::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool SETMASK", name ) ;
		return ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return ;
	}
	it->second.top().fVAR_mask = mask ;
}


string fPOOL::vilist( int& RC, vdType defn )
{
	string vl ;
	map<string, stack< fVAR> >::iterator it ;

	vl.reserve( 9*POOL.size() ) ;

	RC = 8 ;
	for ( it = POOL.begin() ; it != POOL.end() ; it++ )
	{
		if ( !isvalidName( it->first ) ) { continue ; }
		if ( it->second.top().fVAR_type != INTEGER ) { continue ; }
		if ( it->second.top().fVAR_defined )
		{
			if ( defn == IMPLICIT ) { continue ; }
		}
		else
		{
			if ( defn == DEFINED  ) { continue ; }
		}
		vl += " " + it->first ;
		RC  = 0 ;
	}
	return vl ;
}


string fPOOL::vslist( int& RC, vdType defn )
{
	string vl ;
	map<string, stack< fVAR> >::iterator it ;

	vl.reserve( 9*POOL.size() ) ;

	RC = 8 ;
	for ( it = POOL.begin() ; it != POOL.end() ; it++ )
	{
		if ( !isvalidName( it->first ) ) { continue ; }
		if ( it->second.top().fVAR_type != STRING ) { continue ; }
		if ( it->second.top().fVAR_defined )
		{
			if ( defn == IMPLICIT ) { continue ; }
		}
		else
		{
			if ( defn == DEFINED  ) { continue ; }
		}
		vl += " " + it->first ;
		RC  = 0 ;
	}
	return vl ;
}


string * fPOOL::vlocate( errblock& err, const string& name, nameCHCK check )
{
	map<string, stack< fVAR> >::iterator it ;

	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool VLOCATE", name ) ;
		return NULL ;
	}

	it = POOL.find( name ) ;

	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return NULL ;
	}

	if ( it->second.top().fVAR_type != STRING )
	{
		err.seterrid( "PSYE013C", name ) ;
		return NULL ;
	}

	if ( it->second.top().fVAR_defined ) { return  it->second.top().fVAR_string_ptr ; }
	else                                 { return &it->second.top().fVAR_string     ; }
}


// *******************************************************************************************************************************
// *************************************************** VARIABLE POOL SECTION *****************************************************
// *******************************************************************************************************************************


void pVPOOL::put( errblock& err, const string& name, const string& value, vTYPE vtype )
{
	// RC =  0 Normal completion
	// RC = 12 Variable in read-only
	// RC = 16 Truncation occured
	// RC = 20 Severe error

	pVAR var ;

	string t ;

	const string * p_str ;

	map<string, pVAR>::iterator it ;

	t     = "" ;
	p_str = &value ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "variable pool PUT", name ) ;
		return ;
	}
	if ( readOnly )
	{
		err.seterrid( "PSYE015B", 12 ) ;
		return ;
	}
	if ( p_str->size() > 32767 )
	{
		err.seterrid( "PSYE015A", name, 16 ) ;
		t.assign( value, 0, 32766 ) ;
		p_str = &t ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		var.pVAR_value  = *p_str ;
		var.pVAR_system = ( vtype == SYSTEM ) ;
		var.pVAR_type   = pV_VALUE ;
		POOL[ name ]    = var ;
	}
	else
	{
		if ( it->second.pVAR_type != pV_VALUE )
		{
			err.seterrid( "PSYE015D" ) ;
			return ;
		}
		if ( it->second.pVAR_system && vtype != SYSTEM )
		{
			err.seterrid( "PSYE014O", name ) ;
			return ;
		}
		it->second.pVAR_value = *p_str ;
	}
	changed = true ;
}


string pVPOOL::get( errblock& err, const string& name )
{
	// RC =  0 Normal completion
	// RC =  8 Variable not found
	// RC = 20 Severe error

	// Generate the value for pV_type != pV_VALUE (these are date/time entries created on access)

	// TODO: Format date variables according to ZDATEF

	int    p1 ;
	string t  ;

	time_t rawtime        ;
	struct tm * time_info ;
	char   buf[ 12 ]      ;

	err.setRC( 0 ) ;

	map<string, pVAR>::iterator it ;
	std::stringstream stream;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "variable pool GET", name ) ;
		return "" ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return "" ;
	}

	if ( it->second.pVAR_type != pV_VALUE &&
	     it->second.pVAR_type != pV_ZTIMEL  )
	{
		time( &rawtime ) ;
		time_info = localtime( &rawtime ) ;
	}

	switch( it->second.pVAR_type )
	{
		case pV_VALUE:    return it->second.pVAR_value ;
		case pV_ZTIME:    strftime( buf, sizeof(buf), "%H:%M", time_info ) ;
				  buf[ 5  ] = 0x00 ;
				  return buf       ;
		case pV_ZTIMEL:   stream << microsec_clock::local_time() ;
				  t  = stream.str()  ;
				  p1 = t.find( ' ' ) ;
				  return t.substr( p1+1, 11 ) ;
		case pV_ZDATE:    strftime( buf, sizeof(buf), "%d/%m/%y", time_info ) ;
				  buf[ 8  ] = 0x00 ;
				  return buf       ;
		case pV_ZDATEL:   strftime( buf, sizeof(buf), "%d/%m/%Y", time_info ) ;
				  buf[ 10 ] = 0x00 ;
				  return buf       ;
		case pV_ZDAY:     strftime( buf, sizeof(buf), "%d", time_info ) ;
				  buf[ 2  ] = 0x00 ;
				  return buf       ;
		case pV_ZDAYOFWK: strftime( buf, sizeof(buf), "%A", time_info ) ;
				  buf[ 9  ] = 0x00 ;
				  return buf      ;
		case pV_ZDATESTD: strftime( buf, sizeof(buf), "%Y/%m/%d", time_info ) ;
				  buf[ 10 ] = 0x00 ;
				  return buf       ;
		case pV_ZMONTH:   strftime( buf, sizeof(buf), "%m", time_info ) ;
				  buf[ 2  ] = 0x00 ;
				  return buf       ;
		case pV_ZJDATE:   strftime( buf, sizeof(buf), "%y.%j", time_info ) ;
				  buf[ 6  ] = 0x00 ;
				  return buf       ;
		case pV_ZJ4DATE:  strftime( buf, sizeof(buf), "%Y.%j", time_info ) ;
				  buf[ 8  ] = 0x00 ;
				  return buf       ;
		case pV_ZYEAR:    strftime( buf, sizeof(buf), "%y", time_info ) ;
				  buf[ 2  ] = 0x00 ;
				  return buf       ;
		case pV_ZSTDYEAR: strftime( buf, sizeof(buf), "%Y", time_info ) ;
				  buf[ 4  ] = 0x00 ;
				  return buf       ;
		default:          err.seterrid( "PSYE014P", name ) ;
				  return "" ;
	}
}


string * pVPOOL::vlocate( errblock& err, const string& name )
{
	// RC =  0 Normal completion
	// RC =  4 Variable generated on access.  NULL returned.
	// RC =  8 Variable not found
	// RC = 20 Severe error

	map<string, pVAR>::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "variable pool VLOCATE", name ) ;
		return NULL ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return NULL ;
	}

	if ( it->second.pVAR_type != pV_VALUE )
	{
		err.setRC( 4 ) ;
		return NULL ;
	}

	return &it->second.pVAR_value ;
}


void pVPOOL::erase( errblock& err, const string& name )
{
	// RC =  0 Normal completion
	// RC =  8 Variable not found
	// RC = 12 Pool read-only or a system variable
	// RC = 16 Variable in the SYSTEM PROFILE pool (ISPS)
	// RC = 20 Severe error

	map<string, pVAR>::iterator it;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "variable pool ERASE", name ) ;
		return ;
	}
	if ( readOnly )
	{
		err.seterrid( "PSYE015B", 12 ) ;
		return ;
	}
	if ( sysProf  )
	{
		err.seterrid( "PSYE015C", 16 ) ;
		return ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return ;
	}

	if ( it->second.pVAR_type != pV_VALUE )
	{
		err.seterrid( "PSYE015D" ) ;
		return ;
	}

	if ( it->second.pVAR_system )
	{
		err.seterrid( "PSYE015E", 12 ) ;
		return ;
	}

	POOL.erase( it ) ;
	changed = true   ;
}


bool pVPOOL::isSystem( errblock& err, const string& name )
{
	// RC =  0 Normal completion
	// RC =  8 Variable not found
	// RC = 20 Severe error

	map<string, pVAR>::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "variable pool ISSYSTEM", name ) ;
		return false ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return false   ;
	}

	return it->second.pVAR_system ;
}


void pVPOOL::createGenEntries()
{
	pVAR val ;

	val.pVAR_value  = ""   ;
	val.pVAR_system = true ;

	val.pVAR_type = pV_ZTIME    ; POOL[ "ZTIME"    ] = val ;
	val.pVAR_type = pV_ZTIMEL   ; POOL[ "ZTIMEL"   ] = val ;
	val.pVAR_type = pV_ZDATE    ; POOL[ "ZDATE"    ] = val ;
	val.pVAR_type = pV_ZDATEL   ; POOL[ "ZDATEL"   ] = val ;
	val.pVAR_type = pV_ZDAY     ; POOL[ "ZDAY"     ] = val ;
	val.pVAR_type = pV_ZDAYOFWK ; POOL[ "ZDAYOFWK" ] = val ;
	val.pVAR_type = pV_ZDATESTD ; POOL[ "ZDATESTD" ] = val ;
	val.pVAR_type = pV_ZMONTH   ; POOL[ "ZMONTH"   ] = val ;
	val.pVAR_type = pV_ZJDATE   ; POOL[ "ZJDATE"   ] = val ;
	val.pVAR_type = pV_ZJ4DATE  ; POOL[ "ZJ4DATE"  ] = val ;
	val.pVAR_type = pV_ZYEAR    ; POOL[ "ZYEAR"    ] = val ;
	val.pVAR_type = pV_ZSTDYEAR ; POOL[ "ZSTDYEAR" ] = val ;
}


void pVPOOL::load( errblock& err, const string& currAPPLID, const string& path )
{
	// RC = 0  Normal completion
	// RC = 20 Severe error

	string s     ;
	string hdr   ;
	string var   ;
	string value ;

	char x      ;
	int  i, k   ;
	int  n1, n2 ;
	char * buf1 ;
	char z[ 2 ] ;

	size_t buf1Size = 1024  ;

	s = path + currAPPLID + "PROF" ;

	std::ifstream profile ;
	debug1(" profile dataset is " << s << endl ) ;
	profile.open( s.c_str() , ios::binary ) ;
	if ( !profile.is_open() )
	{
		err.seterrid( "PSYE015F", currAPPLID, s ) ;
		return ;
	}

	buf1 = new char[ buf1Size ] ;

	profile.read (buf1, 2 ) ;
	if ( memcmp( buf1, "\x00\x84", 2 ) )
	{
		err.seterrid( "PSYE015G", currAPPLID, s ) ;
		profile.close() ;
		delete[] buf1   ;
		return  ;
	}

	profile.get( x ) ;
	i = static_cast< int >( x ) ;
	if ( i > 1 )
	{
		err.seterrid( "PSYE015H", d2ds( i ), currAPPLID ) ;
		profile.close() ;
		delete[] buf1   ;
		return  ;
	}

	profile.get( x ) ;
	i = static_cast< int >( x ) ;
	if ( i < 0 ) { i = 256 + i ; }
	profile.read (buf1, i ) ;
	hdr.assign( buf1, i )   ;
	debug1(" PROFILE Header " << hdr << endl ) ;

	while ( true )
	{
		profile.get( x ) ;
		if ( profile.eof() ) { break ; }
		i = static_cast< int >( x ) ;
		if ( i < 0 ) { i = 256 + i ; }
		profile.read (buf1 , i) ;
		if ( profile.fail() != 0 ) { err.setRC( 20 ) ; break ; }
		var.assign( buf1, i ) ;
		profile.read( z, 2 )  ;
		if ( profile.fail() != 0 ) { err.setRC( 20 ) ; break ; }
		n1 = static_cast< int >( z[ 0 ] ) ;
		n2 = static_cast< int >( z[ 1 ] ) ;
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
		if ( profile.fail() != 0 ) { err.setRC( 20 ) ; break ; }
		value.assign( buf1, k ) ;
		put( err, var, value ) ;
	}
	profile.close() ;
	delete[] buf1   ;
	if ( err.error() )
	{
		err.seterrid( "PSYE015I", currAPPLID ) ;
	}
	else
	{
		log( "I", "Pool " << currAPPLID << " restored from saved variables in profile dataset " << s << endl ) ;
	}
	resetChanged() ;
}


void pVPOOL::save( errblock& err, const string& currAPPLID )
{
	// RC = 0  Normal completion
	// RC = 4  Save not performed.  Pool in read-only or no changes made to pool
	// RC = 20 Severe error

	string s ;
	int    i ;

	err.setRC( 0 ) ;

	if ( readOnly || !changed ) { err.setRC( 4 ) ; return ; }

	if ( exists( path ) )
	{
		if ( !is_directory( path ) )
		{
			err.seterrid( "PSYE015J", path ) ;
			return ;
		}
	}
	else
	{
		err.seterrid( "PSYE015K", path ) ;
		return ;
	}
	if ( path.back() != '/' ) { path += "/" ; }
	s = path + currAPPLID + "PROF" ;

	std::ofstream profile ;
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
		profile.write( it->first.c_str(), i ) ;
		i = it->second.pVAR_value.size() ;
		profile << (char)( i >> 8 ) ;
		profile << (char)( i ) ;
		profile.write( it->second.pVAR_value.c_str(), i ) ;
	}
	profile.close() ;
	resetChanged()  ;
	debug1( "Saved pool okay to filename " << s << endl ) ;
}


// ******************************************************************************************************************************
// ************************************************** POOL MANAGER SECTION ******************************************************
// ******************************************************************************************************************************


poolMGR::poolMGR()
{
	// Create pools @DEFSHAR, @DEFPROF and @ROXPROF
	// @DEFPROF and @ROXPROF (Read Only Extention PROFILE) not currently used

	log( "I", "Constructor Creating SYSTEM and DEFAULT pools " << endl ) ;

	pVPOOL pool ;

	POOLs_shared [ "@DEFSHAR" ] = pool ;
	POOLs_shared [ "@DEFSHAR" ].createGenEntries() ;
	POOLs_profile[ "@DEFPROF" ] = pool ;
	POOLs_profile[ "@ROXPROF" ] = pool ;
	shrdPooln = 0 ;
}


void poolMGR::setPOOLsReadOnly()
{
	// Neither of these pools is currently used

	POOLs_profile[ "@ROXPROF" ].setReadOnly() ;
	POOLs_profile[ "@DEFPROF" ].setReadOnly() ;
}


void poolMGR::defaultVARs( errblock& err, const string& name, const string& value, poolType pType )
{
	err.setRC( 0 ) ;

	switch ( pType )
	{
	case SHARED:
		POOLs_shared[ "@DEFSHAR" ].put( err, name, value, SYSTEM ) ;
		break ;

	case PROFILE:
		POOLs_profile[ "@DEFPROF" ].put( err, name, value, SYSTEM ) ;
		break ;

	default:
		err.seterrid( "PSYE017D" ) ;
	}
}


void poolMGR::createPool( errblock& err, poolType pType, string path )
{
	// RC = 0  Pool created and loaded from existing file if a PROFILE pool
	// RC = 4  Pool created but not loaded as PROFILE file does not exist (for PROFILE pools only)
	// RC = 20 Severe error

	string s ;

	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator pp_it ;

	pVPOOL pool  ;

	err.setRC( 0 ) ;

	switch( pType )
	{
	case SHARED:
		shrdPooln++ ;
		shrdPool = "#" + right( d2ds( shrdPooln ), 7, '0' ) ;
		debug1( "New shared POOL name " << shrdPool << endl ) ;
		sp_it = POOLs_shared.find( shrdPool ) ;
		if ( sp_it != POOLs_shared.end() )
		{
			err.setRC( 20 ) ;
			log( "C", "SHARED POOL " << shrdPool << " already exists.  Logic Error " << endl ) ;
			return ;
		}
		pool.path     = "" ;
		pool.refCount = 1  ;
		POOLs_shared[ shrdPool ] = pool ;
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
					err.setRC( 20 ) ;
				}
				else
				{
					pool.path     = path ;
					pool.refCount = 1    ;
					POOLs_profile[ currAPPLID ] = pool ;
					log( "I", "Pool " << currAPPLID << " created okay.  Reading saved variables from profile dataset" << endl ) ;
					POOLs_profile[ currAPPLID ].load( err, currAPPLID, path ) ;
					if ( currAPPLID == "ISPS" ) { POOLs_profile[ "ISPS" ].sysProf = true ; }
				}
			}
			else
			{
				if ( exists( path ) )
				{
					if ( !is_directory( path ) )
					{
						log( "E", "Directory " << path << " is not a regular directory for profile load" <<  endl ) ;
						err.setRC( 20 ) ;
					}
					else
					{
						log( "I", "Profile " << currAPPLID+"PROF does not exist.  Creating default" <<  endl ) ;
						pool.path     = path ;
						pool.refCount = 1    ;
						POOLs_profile[ currAPPLID ] = pool ;
						log( "I", "Profile Pool " << currAPPLID << " created okay in path " << path << endl ) ;
						err.setRC( 4 ) ;
					}
				}
				else
				{
					log( "E", "Directory " << path << " does not exist for profile load" <<  endl ) ;
					err.setRC( 20 ) ;
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
		err.setRC( 20 ) ;
	}
}


void poolMGR::createPool( int ls )
{
	// Create the logical-screen variable pool and add defaults.
	// This pool is not accessible by applications (for internal use only).

	pVPOOL pool  ;
	errblock err ;

	POOLs_lscreen[ ls ] = pool ;
	POOLs_lscreen[ ls ].put( err, "ZMSGID",   "",  USER ) ;
	POOLs_lscreen[ ls ].put( err, "ZSCRNAME", "",  USER ) ;
	POOLs_lscreen[ ls ].put( err, "ZPROW",    "0", USER ) ;
	POOLs_lscreen[ ls ].put( err, "ZPCOL",    "0", USER ) ;
	POOLs_lscreen[ ls ].put( err, "ZSCRNAM2", "",  USER ) ;
	POOLs_lscreen[ ls ].put( err, "ZSHMSGID", "N", USER ) ;
	POOLs_lscreen[ ls ].put( err, "ZSHPANID", "N", USER ) ;
}


void poolMGR::destroyPool( errblock& err, poolType pType )
{

	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator pp_it ;

	err.setRC( 0 ) ;

	switch( pType )
	{
	case SHARED:
		log( "I", "Destroying pool " << shrdPool << endl ) ;
		sp_it = POOLs_shared.find( shrdPool ) ;
		if ( sp_it == POOLs_shared.end() )
		{
			err.setRC( 20 ) ;
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
			err.setRC( 20 ) ;
			log( "C", "poolMGR cannot find profile pool " << currAPPLID << " Logic error" << endl ) ;
			return ;
		}
		if ( POOLs_profile[ currAPPLID ].refCount == 1 )
		{
			POOLs_profile[ currAPPLID ].save( err, currAPPLID ) ;
			if ( err.error() )
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
		err.setRC( 20 ) ;
	}
}


void poolMGR::destroyPool( int ls )
{
	// Remove the logical-screen pool when a logical screen is closed

	if ( POOLs_lscreen.count( ls ) > 0 )
	{
		POOLs_lscreen.erase( ls ) ;
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
		log( "-", "            Pool " << setw(8) << sp_it->first << "  use count: " << setw(4) << sp_it->second.refCount <<
			  "  " << Mode << "  entries: " << setw(5) << sp_it->second.POOL.size() << endl ) ;
	}
	log( "-", "" << endl ) ;
	log( "-", "         Profile pool details:" << endl ) ;
	for ( pp_it = POOLs_profile.begin() ; pp_it != POOLs_profile.end() ; pp_it++ )
	{
		if ( pp_it->second.readOnly ) { Mode = "RO" ; }
		else                          { Mode = "UP" ; }
		log( "-", "            Pool " << setw(8) << pp_it->first << "  use count: " << setw(4) << pp_it->second.refCount <<
			  "  " << Mode << "  entries: " << setw(5) << pp_it->second.POOL.size() << "  path: " << pp_it->second.path << endl ) ;
	}
	log( "-", "*************************************************************************************************************" << endl ) ;
}


void poolMGR::snap()
{
	errblock err ;

	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator pp_it ;
	map<string, pVAR>::iterator v_it    ;
	string vtype ;

	log( "-", "POOL VARIABLES:" << endl ) ;
	log( "-", "         Shared pool details:" << endl ) ;
	for ( sp_it = POOLs_shared.begin() ; sp_it != POOLs_shared.end() ; sp_it++ )
	{
		log( "-", "         Pool " << setw(8) << sp_it->first << " use count:" << setw(3) << sp_it->second.refCount <<
			  " entries: " << sp_it->second.POOL.size() << endl ) ;
		for ( v_it = sp_it->second.POOL.begin() ; v_it != sp_it->second.POOL.end() ; v_it++ )
		{
			if ( sp_it->second.isSystem( err, v_it->first ) ) { vtype = "S" ; }
			else                                             { vtype = "N" ; }
			log( "-", setw(8) << v_it->first << " :" << vtype << ": " << sp_it->second.get( err, v_it->first ) << "<< " << endl ) ;
		}
	}
	log( "-", endl ) ;
	log( "-", "         PROFILE pool details:" << endl ) ;
	for ( pp_it = POOLs_profile.begin() ; pp_it != POOLs_profile.end() ; pp_it++ )
	{
		log( "-", "         Pool " << setw(8) << pp_it->first << " use count: " << setw(3) << pp_it->second.refCount <<
			  " entries: " << pp_it->second.POOL.size() << endl ) ;
		log( "-", "                            path: " << setw(3) << pp_it->second.path << endl ) ;
		for ( v_it = pp_it->second.POOL.begin() ; v_it != pp_it->second.POOL.end() ; v_it++ )
		{
			if ( pp_it->second.isSystem( err, v_it->first ) ) { vtype = "S" ; }
			else                                             { vtype = "N" ; }
			log( "-", setw(8) << v_it->first << " :" << vtype << ": " << pp_it->second.get( err, v_it->first ) << "<< " << endl ) ;
		}
	}
	log( "-", "*************************************************************************************************************" << endl ) ;
}


void poolMGR::setAPPLID( errblock& err, const string& m_APPLID )
{
	err.setRC( 0 ) ;

	if ( !isvalidName4( m_APPLID ) )
	{
		err.setRC( 20 ) ;
		log( "C", "Invalid APPLID name format passed to pool manager '" << m_APPLID << "'" << endl ) ;
		return  ;
	}

	currAPPLID = m_APPLID ;
}


void poolMGR::setShrdPool( errblock& err, const string& m_shrdPool )
{
	err.setRC( 0 ) ;

	if ( POOLs_shared.count( m_shrdPool ) == 0 )
	{
		err.setRC( 20 ) ;
		log( "C", "poolMGR cannot find pool "+ m_shrdPool +".  Pool must be created before setting pool name" << endl ) ;
		return  ;
	}
	shrdPool = m_shrdPool ;
}


string poolMGR::vlist( int& RC, poolType pType, int lvl )
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
	vlist.reserve( 9*p_it->second.POOL.size() ) ;
	for ( v_it = p_it->second.POOL.begin() ; v_it != p_it->second.POOL.end() ; v_it++ )
	{
		vlist += " " + v_it->first ;
	}
	return vlist ;
}


void poolMGR::put( errblock& err, const string& name, const string& value, poolType pType, vTYPE vtype )
{
	// Pool search order:  ASIS - SHARED then PROFILE
	// RC = 0  variable put okay
	// RC = 12 variable not put as in read-only status
	// RC = 20 severe error

	// for put PROFILE, delete the variable from the SHARED pool

	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator pp_it ;
	map<string, pVPOOL>::iterator p_it  ;

	errblock err2 ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "pool manager PUT", name ) ;
		return ;
	}

	sp_it = POOLs_shared.find( shrdPool ) ;
	if ( sp_it == POOLs_shared.end() )
	{
		err.seterrid( "PSYE017E", shrdPool ) ;
		return ;
	}

	switch( pType )
	{
	case ASIS:
		locateSubPool( err, p_it, name, SHARED ) ;
		if      ( err.RC0() ) { p_it->second.put( err, name, value, vtype ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, p_it, name, PROFILE ) ;
			if      ( err.RC0() ) {  p_it->second.put( err, name, value, vtype ) ; }
			else if ( err.RC8() ) { sp_it->second.put( err, name, value, vtype ) ; }
		}
		break ;
	case SHARED:
		locateSubPool( err, p_it, name, SHARED ) ;
		if      ( err.RC0() ) {  p_it->second.put( err, name, value, vtype ) ; }
		else if ( err.RC8() ) { sp_it->second.put( err, name, value, vtype ) ; }
		break ;
	case PROFILE:
		pp_it = POOLs_profile.find( currAPPLID ) ;
		locateSubPool( err, p_it, name, PROFILE ) ;
		if      ( err.RC0() ) {  p_it->second.put( err, name, value, vtype ) ; }
		else if ( err.RC8() ) { pp_it->second.put( err, name, value, vtype ) ; }
		locateSubPool( err2, p_it, name, SHARED ) ;
		if ( err2.RC0() )     {  p_it->second.erase( err, name ) ;  }
		break ;
	default:
		err.seterrid( "PSYE017C" ) ;
		return  ;
	}
}


void poolMGR::put( errblock& err, int ls, const string& name, const string& value )
{
	// Set a variable from the logical-screen pool
	// Pool is created on first access

	err.setRC( 0 ) ;

	if ( POOLs_lscreen.count( ls ) == 0 ) { createPool( ls ) ; }

	POOLs_lscreen[ ls ].put( err, name, value, USER ) ;
}


string poolMGR::get( errblock& err, const string& name, poolType pType )
{
	// Pool search order: ASIS - SHARED then PROFILE
	// RC = 0  variable found
	// RC = 8  variable not found
	// RC = 20 severe error

	// for get PROFILE, delete the variable from the SHARED pool even if not found in the PROFILE pool

	errblock err2 ;

	map<string, pVPOOL>::iterator p_it  ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "pool manager GET", name ) ;
		return "" ;
	}

	switch ( pType )
	{
	case ASIS:
		locateSubPool( err, p_it, name, SHARED ) ;
		if      ( err.RC0() ) { return p_it->second.get( err, name ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, p_it, name, PROFILE ) ;
			if ( err.RC0() ) { return p_it->second.get( err, name ) ; }
		}
		break ;
	case PROFILE:
		locateSubPool( err2, p_it, name, SHARED ) ;
		if ( err2.RC0() ) { p_it->second.erase( err, name ) ; }
		locateSubPool( err, p_it, name, PROFILE ) ;
		if ( err.RC0() ) { return p_it->second.get( err, name ) ; }
		break ;
	case SHARED:
		locateSubPool( err, p_it, name, SHARED ) ;
		if ( err.RC0() ) { return p_it->second.get( err, name ) ; }
		break ;
	default:
		err.seterrid( "PSYE017C" ) ;
	}
	return "" ;
}


string poolMGR::get( errblock& err, int ls, const string& name )
{
	// Retrieve a variable from the logical-screen pool
	// Pool is created on first access

	if ( POOLs_lscreen.count( ls ) == 0 ) { createPool( ls ) ; }

	return POOLs_lscreen[ ls ].get( err, name ) ;
}


string * poolMGR::vlocate( errblock& err, const string& name, poolType pType )
{
	// Pool search order: ASIS - SHARED then PROFILE
	// RC = 0  variable found
	// RC = 8  variable not found
	// RC = 20 severe error

	map<string, pVPOOL>::iterator p_it  ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "pool manager VLOCATE", name ) ;
		return NULL ;
	}

	switch ( pType )
	{
	case ASIS:
		locateSubPool( err, p_it, name, SHARED ) ;
		if      ( err.RC0() ) { return p_it->second.vlocate( err, name ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, p_it, name, PROFILE ) ;
			if ( err.RC0() ) { return p_it->second.vlocate( err, name ) ; }
		}
		break ;
	case PROFILE:
		locateSubPool( err, p_it, name, PROFILE ) ;
		if ( err.RC0() ) { return p_it->second.vlocate( err, name ) ; }
		break ;
	case SHARED:
		locateSubPool( err, p_it, name, SHARED ) ;
		if ( err.RC0() ) { return p_it->second.vlocate( err, name ) ; }
		break ;
	default:
		err.seterrid( "PSYE017C" ) ;
	}
	return NULL ;
}



void poolMGR::erase( errblock& err, const string& name, poolType pType )
{
	// Pool search order: ASIS - SHARED then PROFILE
	// RC = 0  variable found and erased
	// RC = 8  variable not found
	// RC = 12 variable not erased as pool in read-only status, or system variable
	// RC = 16 variable not erased as in the ISPS SYSTEM profile pool
	// RC = 20 severe error

	map<string, pVPOOL>::iterator sp_it ;
	map<string, pVPOOL>::iterator p_it  ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "pool manager ERASE", name ) ;
		return ;
	}

	sp_it = POOLs_shared.find( shrdPool ) ;
	if ( sp_it == POOLs_shared.end() )
	{
		err.seterrid( "PSYE017A", shrdPool ) ;
		return  ;
	}

	switch( pType )
	{
	case ASIS:
		locateSubPool( err, p_it, name, SHARED ) ;
		if ( err.RC0() ) { p_it->second.erase( err, name ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, p_it, name, PROFILE ) ;
			if ( err.RC0() ) { p_it->second.erase( err, name ) ; }
		}
		break ;
	case PROFILE:
		locateSubPool( err, p_it, name, PROFILE ) ;
		if ( err.RC0() ) { p_it->second.erase( err, name ) ; }
		break ;
	case SHARED:
		locateSubPool( err, p_it, name, SHARED ) ;
		if ( err.RC0() ) { p_it->second.erase( err, name ) ; }
		break ;
	case BOTH:
		locateSubPool( err, p_it, name, SHARED ) ;
		if ( err.RC0() ) { p_it->second.erase( err, name ) ; }
		locateSubPool( err, p_it, name, PROFILE ) ;
		if ( err.RC0() ) { p_it->second.erase( err, name ) ; }
		break ;
	}
}


void poolMGR::locateSubPool( errblock& err, map<string, pVPOOL>::iterator& p_it, const string& name, poolType pType )
{
	// Locate the sub-pool for variable name.
	// RC = 0 variable found.  Pool iterator, p_it, will be valid on return
	// RC = 8 if variable not found

	// Sub-Pool search order
	// SHARED:  CURRENT @DEFSHAR
	// PROFILE: APPLID, @ROXPROF @DEFPROF ISPS

	map<string, pVAR>::iterator v_it ;

	err.setRC( 0 ) ;

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
					if ( v_it == p_it->second.POOL.end() ) { err.setRC( 8 ) ; }
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
			if ( v_it == p_it->second.POOL.end() ) { err.setRC( 8 ) ; }
		}
		break ;
	default:
		err.seterrid( "PSYE017B" ) ;
	}
	return ;
}
