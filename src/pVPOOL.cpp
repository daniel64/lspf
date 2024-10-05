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


using namespace boost::filesystem ;


// ******************************************************************************************************************************
// ****************************************************   fVAR SECTION   ********************************************************
// ******************************************************************************************************************************


string& fVAR::sget( const string& name )
{
	if ( fVAR_type == INTEGER )
	{
		fVAR_string = d2ds( *fVAR_int_ptr, fVAR_zconv ) ;
	}

	return *fVAR_string_ptr ;
}


int fVAR::iget( errblock& err,
		const string& name )
{
	err.setRC( 0 ) ;

	if ( fVAR_type == STRING )
	{
		if ( !isnumeric( *fVAR_string_ptr ) )
		{
			err.seterrid( TRACE_INFO(), "PSYV011A", name, 16 ) ;
			return 0 ;
		}
		return ds2d( *fVAR_string_ptr ) ;
	}

	return *fVAR_int_ptr ;
}


void fVAR::put( errblock& err,
		const string& name,
		const string& val )
{
	err.setRC( 0 ) ;

	if ( fVAR_type == INTEGER )
	{
		if ( !isnumeric( val ) )
		{
			err.seterrid( TRACE_INFO(), "PSYV011A", name, 16 ) ;
			return ;
		}
		*fVAR_int_ptr = ds2d( val ) ;
	}
	else
	{
		*fVAR_string_ptr = val ;
	}
}


void fVAR::put( int val,
		int len )
{
	if ( fVAR_type == INTEGER )
	{
		*fVAR_int_ptr = val ;
	}
	else
	{
		*fVAR_string_ptr = ( len > 0 ) ? d2ds( val, len ) : d2ds( val, fVAR_zconv ) ;
	}
}


bool fVAR::hasmask() const
{
	return ( fVAR_vtype != VED_NONE ) ;
}


bool fVAR::hasmask( string& m,
		    VEDIT_TYPE& t ) const
{
	m = fVAR_mask ;
	t = fVAR_vtype ;

	return ( fVAR_vtype != VED_NONE ) ;
}


bool fVAR::integer() const
{
	return ( fVAR_type == INTEGER ) ;
}


// ******************************************************************************************************************************
// ************************************************   FUNCTION POOL SECTION   ***************************************************
// ******************************************************************************************************************************


fPOOL::~fPOOL()
{
	//
	// Free dynamic storage for all variables in the function pool when the pool is deleted.
	//
	// pool_1 Application function pool dialogue variables (implicit and defined).
	// pool_2 Internal use only.
	//

	for ( auto it = pool_1.begin() ; it!= pool_1.end() ; ++it )
	{
		while ( !it->second.empty() )
		{
			delete it->second.top() ;
			it->second.pop() ;
		}
	}

	for ( auto it = pool_2.begin() ; it!= pool_2.end() ; ++it )
	{
		delete it->second ;
	}
}


void fPOOL::define( errblock& err,
		    const string& name,
		    string* addr,
		    bool check )
{
	err.setRC( 0 ) ;

	int zconv = 0 ;

	if ( check && !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool DEFINE", name ) ;
		return ;
	}

	if ( name.front() == 'Z' )
	{
		auto it = zint2str.find( name ) ;
		if ( it != zint2str.end() )
		{
			zconv = it->second ;
		}
	}

	pool_1[ name ].push( new fVAR( addr, zconv ) ) ;
}


void fPOOL::define( errblock& err,
		    const char* name,
		    string* addr )
{
	assert ( isvalidName( name ) ) ;

	define( err, name, addr, false ) ;
}


void fPOOL::define( errblock& err,
		    const string& name,
		    int* addr,
		    bool check )
{
	err.setRC( 0 ) ;

	int zconv = 0 ;

	if ( check && !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool DEFINE", name ) ;
		return ;
	}

	if ( name.front() == 'Z' )
	{
		auto it = zint2str.find( name ) ;
		if ( it != zint2str.end() )
		{
			zconv = it->second ;
		}
	}

	pool_1[ name ].push( new fVAR( addr, zconv ) )  ;
}


void fPOOL::define( errblock& err,
		    const char* name,
		    int* addr )
{
	assert ( isvalidName( name ) ) ;

	define( err, name, addr, false ) ;
}


void fPOOL::del( errblock& err,
		 const string& name )
{
	//
	// Remove the vdefine for a variable from the function pool (delete dynamic storage for fVAR first).
	//
	// Use '*' for all vdefined variables.  Implicitly defined variables are not affected.
	//
	// RC =  0 OK.
	// RC =  8 Variable not found in the defined area of the function pool.
	// RC = 20 Severe error.
	//

	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	if ( name == "*")
	{
		for ( it = pool_1.begin() ; it != pool_1.end() ; )
		{
			while ( !it->second.empty() && it->second.top()->fVAR_defined )
			{
				delete it->second.top() ;
				it->second.pop() ;
			}
			if ( it->second.empty() )
			{
				it = pool_1.erase( it ) ;
			}
			else
			{
				++it ;
			}
		}
		return ;
	}

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool DELETE", name ) ;
		return ;
	}

	it = pool_1.find( name ) ;

	if ( it == pool_1.end() || !it->second.top()->fVAR_defined )
	{
		err.setRC( 8 ) ;
		return ;
	}

	delete it->second.top() ;
	it->second.pop() ;

	if ( it->second.empty() ) { pool_1.erase( it ) ; }
}


const string& fPOOL::get1( errblock& err,
			   int maxRC,
			   const string& name,
			   bool check )
{
	//
	// Function pool GET (string).
	//

	fVAR* var ;

	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	if ( check && !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool GET", name ) ;
		return nullstr ;
	}

	it = pool_1.find( name ) ;
	if ( it == pool_1.end() )
	{
		err.setRC( 8 ) ;
		if ( 8 > maxRC )
		{
			err.seterrid( TRACE_INFO(), "PSYE013B", d2ds( maxRC ), name ) ;
		}
		return nullstr ;
	}

	var = it->second.top() ;

	return var->sget( name  ) ;
}


const string& fPOOL::get1( errblock& err,
			   int maxRC,
			   const char* name )
{
	//
	// Function pool GET (string).  Name has already been verified.
	//

	assert ( isvalidName( name ) ) ;

	return get1( err, maxRC, name, false ) ;
}


int fPOOL::get1( errblock& err,
		 int maxRC,
		 dataType type,
		 const string& name,
		 bool check )
{
	//
	// Function pool GET (integer).
	//

	fVAR* var ;

	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	if ( check && !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool GET", name ) ;
		return 0 ;
	}

	if ( type != INTEGER )
	{
		err.seterrid( TRACE_INFO(), "PSYE015R", name ) ;
		return 0 ;
	}

	it = pool_1.find( name ) ;
	if ( it == pool_1.end() )
	{
		err.setRC( 8 ) ;
		if ( 8 > maxRC )
		{
			err.seterrid( TRACE_INFO(), "PSYE013B", d2ds( maxRC ), name ) ;
		}
		return 0 ;
	}

	var = it->second.top() ;

	return var->iget( err, name  ) ;
}


int fPOOL::get1( errblock& err,
		 int maxRC,
		 dataType type,
		 const char* name )
{
	//
	// Function pool GET (integer).  Name has already been verified.
	//

	assert ( isvalidName( name ) ) ;

	return get1( err, maxRC, type, name, false ) ;
}


const string& fPOOL::get2( errblock& err,
			   int maxRC,
			   const string& name )
{
	//
	// Function pool GET (string).  Name has already been verified.
	//

	assert ( isvalidName( name ) ) ;

	return get1( err, maxRC, name, false ) ;
}


int fPOOL::get2( errblock& err,
		 int maxRC,
		 dataType type,
		 const string& name )
{
	//
	// Function pool GET (integer).  Name has already been verified.
	//

	assert ( isvalidName( name ) ) ;

	return get1( err, maxRC, type, name, false ) ;
}


const string& fPOOL::get3( errblock& err,
			   const string& name )
{
	//
	// Function pool GET from pool_2.  Internal use only.
	//

	err.setRC( 0 ) ;

	map<string, string*>::const_iterator it ;

	it = pool_2.find( name ) ;
	if ( it == pool_2.end() )
	{
		err.setRC( 8 ) ;
		return nullstr ;
	}

	return *(it->second) ;
}


fVAR* fPOOL::getfVAR( errblock& err,
		      const string& name )
{
	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool GETFVAR", name ) ;
		return nullptr ;
	}

	it = pool_1.find( name ) ;
	if ( it == pool_1.end() )
	{
		err.setRC( 8 ) ;
		return nullptr ;
	}

	return it->second.top() ;
}


bool fPOOL::ifexists( errblock& err,
		      const string& name )
{
	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool IFEXISTS", name ) ;
		return false ;
	}

	return pool_1.count( name ) > 0 ;
}


void fPOOL::put1( errblock& err,
		  const string& name,
		  const string& value,
		  bool check )
{
	//
	// Function pool PUT.  Variable name needs to be checked.
	//
	// RC =  0 OK.
	// RC = 20 Severe error.
	//

	fVAR* var ;

	map<string, stack<fVAR*>>::iterator it ;

	int zconv = 0 ;

	err.setRC( 0 ) ;

	if ( check && !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool PUT", name ) ;
		return ;
	}

	it = pool_1.find( name ) ;
	if ( it == pool_1.end() )
	{
		if ( name.front() == 'Z' )
		{
			auto it = zint2str.find( name ) ;
			if ( it != zint2str.end() )
			{
				zconv = it->second ;
			}
		}
		pool_1[ name ].push( new fVAR( value, zconv ) ) ;
	}
	else
	{
		var = it->second.top() ;
		var->put( err, name, value ) ;
	}
}



void fPOOL::put1( errblock& err,
		  const string& name,
		  const char* value )
{
	//
	// Function pool PUT.  Name has already been verified.
	//
	// RC =  0 OK.
	// RC = 20 Severe error.
	//

	assert ( isvalidName( name ) ) ;

	put1( err, name, value, false ) ;
}


void fPOOL::put1( errblock& err,
		  const string& name,
		  int value,
		  bool check )
{
	//
	// Function pool PUT.  Name has already been verified.
	//
	// RC =  0 OK.
	// RC = 20 Severe error.
	//

	fVAR* var ;

	int zconv = 0 ;

	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	if ( check && !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool PUT", name ) ;
		return ;
	}

	it = pool_1.find( name ) ;
	if ( it == pool_1.end() )
	{
		if ( name.front() == 'Z' )
		{
			auto it = zint2str.find( name ) ;
			if ( it != zint2str.end() )
			{
				zconv = it->second ;
			}
		}
		pool_1[ name ].push( new fVAR( value, zconv ) ) ;
	}
	else
	{
		var = it->second.top() ;
		var->put( value ) ;
	}
}


void fPOOL::put1( errblock& err,
		  const char* name,
		  int value )
{
	//
	// Function pool PUT.  Name has already been verified.
	//
	// RC =  0 OK.
	// RC = 20 Severe error.
	//

	assert ( isvalidName( name ) ) ;

	put1( err, name, value, false ) ;
}


void fPOOL::put2( errblock& err,
		  const string& name,
		  const string& value )
{
	//
	// Function pool PUT.  Name has already been verified.
	//
	// RC =  0 OK.
	// RC = 20 Severe error.
	//

	assert ( isvalidName( name ) ) ;

	put1( err, name, value, false ) ;
}


void fPOOL::put2( errblock& err,
		  const string& name,
		  int value )
{
	//
	// Function pool PUT.  Name has already been verified.
	//
	// RC =  0 OK.
	// RC = 20 Severe error.
	//

	assert ( isvalidName( name ) ) ;

	put1( err, name, value, false ) ;
}


void fPOOL::put3( const string& name,
		  const string& value )
{
	//
	// Function pool PUT to pool_2.  Internal use only.
	//

	map<string, string*>::iterator it = pool_2.find( name ) ;

	if ( it == pool_2.end() )
	{
		pool_2[ name ] = new string( value ) ;
	}
	else
	{
		*(it->second) = value ;
	}
}


void fPOOL::reset( errblock& err )
{
	//
	// Free dynamic storage for all variables in the function pool and clear the pool.
	//

	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	for ( it = pool_1.begin() ; it!= pool_1.end() ; ++it )
	{
		while ( !it->second.empty() )
		{
			delete it->second.top() ;
			it->second.pop() ;
		}
	}

	pool_1.clear() ;
}


void fPOOL::setmask( errblock& err,
		     const string& name,
		     const string& mask,
		     VEDIT_TYPE vtype )
{
	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool SETMASK", name ) ;
		return ;
	}

	it = pool_1.find( name ) ;
	if ( it == pool_1.end() || !it->second.top()->fVAR_defined )
	{
		err.setRC( 8 ) ;
		return ;
	}

	if ( vtype == VED_USER )
	{
		it->second.top()->fVAR_mask = mask ;
	}
	it->second.top()->fVAR_vtype = vtype ;
}


void fPOOL::getmask( errblock& err,
		     const string& name,
		     string& mask,
		     VEDIT_TYPE& vtype )
{
	fVAR* var ;

	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	it = pool_1.find( name ) ;
	if ( it == pool_1.end() || it->second.top()->fVAR_vtype == VED_NONE )
	{
		err.seterrid( TRACE_INFO(), "PSYE015S", name, 20 ) ;
		return ;
	}

	var   = it->second.top() ;
	mask  = var->fVAR_mask ;
	vtype = var->fVAR_vtype ;
}


bool fPOOL::hasmask( const string& name )
{
	map<string, stack<fVAR*>>::iterator it ;

	it = pool_1.find( name ) ;

	return ( it != pool_1.end() && it->second.top()->hasmask() ) ;
}


bool fPOOL::hasmask( const string& name,
		     string& mask,
		     VEDIT_TYPE& vtype )
{
	fVAR* var ;

	map<string, stack<fVAR*>>::iterator it ;

	it = pool_1.find( name ) ;

	if ( it == pool_1.end() || !it->second.top()->hasmask() )
	{
		return false ;
	}

	var   = it->second.top() ;
	mask  = var->fVAR_mask ;
	vtype = var->fVAR_vtype ;

	return true ;
}


set<string>& fPOOL::vlist( int& RC,
			   dataType type,
			   vdType defn )
{
	fVAR* var ;

	map<string, stack<fVAR*>>::iterator it ;

	varList.clear() ;

	RC = 8 ;
	for ( it = pool_1.begin() ; it != pool_1.end() ; ++it )
	{
		var = it->second.top() ;
		if (  var->fVAR_type != type ||
		   (  var->fVAR_defined && defn == IMPLICIT ) ||
		   ( !var->fVAR_defined && defn == DEFINED  ) )
		{
			continue ;
		}
		varList.insert( it->first ) ;
		RC = 0 ;
	}

	return varList ;
}


string* fPOOL::vlocate( errblock& err,
			const string& name )
{
	fVAR* var ;

	map<string, stack<fVAR*>>::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "function pool VLOCATE", name ) ;
		return nullptr ;
	}

	it = pool_1.find( name ) ;
	if ( it == pool_1.end() )
	{
		err.setRC( 8 ) ;
		return nullptr ;
	}

	var = it->second.top() ;
	if ( var->fVAR_type != STRING )
	{
		err.seterrid( TRACE_INFO(), "PSYE015O", name ) ;
		return nullptr ;
	}

	return var->fVAR_string_ptr ;
}


// *******************************************************************************************************************************
// *************************************************   VARIABLE POOL SECTION   ***************************************************
// *******************************************************************************************************************************


pVPOOL::~pVPOOL()
{
	//
	// Free dynamic storage for all variables in the pool when the pool is deleted.
	//

	map<string, pVAR*>::iterator it ;

	for ( it = POOL.begin() ; it != POOL.end() ; ++it )
	{
		delete it->second ;
	}
}


void pVPOOL::put( errblock& err,
		  const string& name,
		  const string& value,
		  vTYPE vtype )
{
	//
	// RC =  0 Normal completion.
	// RC = 12 Variable in read-only.
	// RC = 16 Truncation occured.
	// RC = 20 Severe error.
	//

	pVAR* var ;

	map<string, pVAR*>::iterator it ;

	err.setRC( 0 ) ;

	if ( readOnly )
	{
		err.seterrid( TRACE_INFO(), "PSYE015B", 12 ) ;
		return ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		if ( !isvalidName( name ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE013A", "variable pool PUT", name ) ;
			return ;
		}
		var = new pVAR ;
		var->pVAR_system = ( vtype == SYSTEM ) ;
		var->pVAR_type   = pV_VALUE ;
		POOL[ name ] = var ;
	}
	else
	{
		var = it->second ;
		if ( var->pVAR_type != pV_VALUE )
		{
			err.seterrid( TRACE_INFO(), "PSYE015D", name ) ;
			return ;
		}
		if ( var->pVAR_system && vtype != SYSTEM )
		{
			err.seterrid( TRACE_INFO(), "PSYE014O", name ) ;
			return ;
		}
	}
	if ( value.size() > 32767 )
	{
		err.seterrid( TRACE_INFO(), "PSYE015A", name, 16 ) ;
		var->pVAR_value.assign( value, 0, 32766 ) ;
	}
	else
	{
		var->pVAR_value = value ;
		if ( profile && name.front() == 'Z' )
		{
			if ( ( name.compare( 0, 3, "ZPF" ) == 0 ) ||
			     ( name.size() == 6 && name.compare( 0, 6, "ZKLUSE" ) == 0 ) ||
			     ( name.size() == 8 && name.compare( 0, 8, "ZPRIKEYS" ) == 0 ) )
			{
				++pfkgToken ;
			}
		}
	}
	changed = true ;
}


void pVPOOL::put( errblock& err,
		  map<string, pVAR*>::iterator v_it,
		  const string& value,
		  vTYPE vtype )
{
	//
	// This version of put uses the variable iterator and the variable will always exist (RC=0 from locateSubPool).
	//
	// RC =  0 Normal completion.
	// RC = 12 Variable in read-only.
	// RC = 16 Truncation occured.
	// RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	if ( readOnly )
	{
		err.seterrid( TRACE_INFO(), "PSYE015B", 12 ) ;
		return ;
	}

	pVAR* var = v_it->second ;
	if ( var->pVAR_type != pV_VALUE )
	{
		err.seterrid( TRACE_INFO(), "PSYE015D", v_it->first ) ;
		return ;
	}

	if ( var->pVAR_system && vtype != SYSTEM )
	{
		err.seterrid( TRACE_INFO(), "PSYE014O", v_it->first ) ;
		return ;
	}

	if ( value.size() > 32767 )
	{
		err.seterrid( TRACE_INFO(), "PSYE015A", v_it->first, 16 ) ;
		var->pVAR_value.assign( value, 0, 32766 ) ;
	}
	else
	{
		var->pVAR_value = value ;
		if ( profile && v_it->first.front() == 'Z' )
		{
			if ( ( v_it->first.compare( 0, 3, "ZPF" ) == 0 ) ||
			     ( v_it->first.size() == 6 && v_it->first.compare( 0, 6, "ZKLUSE" ) == 0 ) ||
			     ( v_it->first.size() == 8 && v_it->first.compare( 0, 8, "ZPRIKEYS" ) == 0 ) )
			{
				++pfkgToken ;
			}
		}
	}
	changed = true ;
}


string pVPOOL::get( errblock& err,
		    map<string, pVAR*>::iterator v_it )
{
	//
	// RC =  0 Normal completion.
	// RC =  8 Variable not found (set in pool manager).
	// RC = 20 Severe error.
	//
	// Generate the value for pV_type != pV_VALUE (these are date/time entries created on access).
	//

	int p1 ;
	int datef = 0 ;

	string t ;

	char buf[ 12 ] ;

	time_t rawtime ;
	struct tm* time_info = nullptr ;

	pVAR* var = v_it->second ;
	pVAR* tf ;

	err.setRC( 0 ) ;

	std::stringstream stream;

	if ( var->pVAR_type != pV_VALUE   &&
	     var->pVAR_type != pV_ZTIMEL  &&
	     var->pVAR_type != pV_ZTASKID &&
	     var->pVAR_type != pV_ZDEBUG )
	{
		time( &rawtime ) ;
		if ( rawtime == -1 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013B" ) ;
			return "" ;
		}
		time_info = localtime( &rawtime ) ;
		if ( !time_info )
		{
			err.seterrid( TRACE_INFO(), "PSYS013C" ) ;
			return "" ;
		}
	}

	if ( var->pVAR_type == pV_ZDATE ||
	     var->pVAR_type == pV_ZDATESTD )
	{
		auto it = POOL.find( "ZDATEF" ) ;
		if ( it != POOL.end() )
		{
			tf = it->second ;
			if ( tf->pVAR_value.compare( 0, 3, "DD/" ) == 0 )
			{
				datef = 0 ;
			}
			else if ( tf->pVAR_value.compare( 0, 3, "DD." ) == 0 )
			{
				datef = 1 ;
			}
			else if ( tf->pVAR_value.compare( 0, 3, "YY/" ) == 0 )
			{
				datef = 2 ;
			}
			if ( tf->pVAR_value.compare( 0, 3, "YY." ) == 0 )
			{
				datef = 3 ;
			}
		}
	}

	switch( var->pVAR_type )
	{
		case pV_VALUE:    return var->pVAR_value ;

		case pV_ZTIME:    strftime( buf, sizeof( buf ), "%H:%M", time_info ) ;
				  return buf ;

		case pV_ZTIMEL:   stream << microsec_clock::local_time() ;
				  t  = stream.str()  ;
				  p1 = t.find( ' ' ) ;
				  return t.substr( p1+1, 11 ) ;

		case pV_ZDATE:    switch ( datef )
				  {
				  case 0: strftime( buf, sizeof( buf ), "%d/%m/%y", time_info ) ;
					  break ;
				  case 1: strftime( buf, sizeof( buf ), "%d.%m.%y", time_info ) ;
					  break ;
				  case 2: strftime( buf, sizeof( buf ), "%y/%m/%d", time_info ) ;
					  break ;
				  case 3: strftime( buf, sizeof( buf ), "%y.%m.%d", time_info ) ;
					  break ;
				  }
				  return buf ;

		case pV_ZDAY:     strftime( buf, sizeof( buf ), "%d", time_info ) ;
				  return buf ;

		case pV_ZDAYOFWK: strftime( buf, sizeof( buf ), "%A", time_info ) ;
				  return buf ;

		case pV_ZDATESTD: switch ( datef )
				  {
				  case 0: strftime( buf, sizeof( buf ), "%d/%m/%Y", time_info ) ;
					  break ;
				  case 1: strftime( buf, sizeof( buf ), "%d.%m.%Y", time_info ) ;
					  break ;
				  case 2: strftime( buf, sizeof( buf ), "%Y/%m/%d", time_info ) ;
					  break ;
				  case 3: strftime( buf, sizeof( buf ), "%Y.%m.%d", time_info ) ;
					  break ;
				  }
				  return buf ;

		case pV_ZMONTH:   strftime( buf, sizeof( buf ), "%m", time_info ) ;
				  return buf ;

		case pV_ZJDATE:   strftime( buf, sizeof( buf ), "%y.%j", time_info ) ;
				  return buf ;

		case pV_ZJ4DATE:  strftime( buf, sizeof( buf ), "%Y.%j", time_info ) ;
				  return buf ;

		case pV_ZYEAR:    strftime( buf, sizeof( buf ), "%y", time_info ) ;
				  return buf ;

		case pV_ZSTDYEAR: strftime( buf, sizeof( buf ), "%Y", time_info ) ;
				  return buf ;

		case pV_ZDEBUG:   return d2ds( err.debugLevel(), 2 ) ;

		case pV_ZTASKID:  return d2ds( err.taskid, 5 ) ;

		default:          err.seterrid( TRACE_INFO(), "PSYE014P", v_it->first ) ;
				  return "" ;
	}
}


string* pVPOOL::vlocate( errblock& err,
			 map<string, pVAR*>::iterator v_it )
{
	//
	// RC =  0 Normal completion.
	// RC =  4 Variable generated on access.  NULL returned.
	// RC =  8 Variable not found (set in the pool manager).
	// RC = 20 Severe error (set in the pool manager).
	//

	err.setRC( 0 ) ;

	pVAR* var = v_it->second ;

	if ( var->pVAR_type != pV_VALUE )
	{
		err.setRC( 4 ) ;
		return nullptr ;
	}

	return &var->pVAR_value ;
}


void pVPOOL::erase( errblock& err,
		    map<string, pVAR*>::iterator v_it )
{
	//
	// RC =  0 Normal completion.
	// RC =  8 Variable not found (set in the pool manager).
	// RC = 12 Pool read-only or a system variable.
	// RC = 16 Variable in the SYSTEM PROFILE pool (ISPS).
	// RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	if ( readOnly )
	{
		err.seterrid( TRACE_INFO(), "PSYE015B", 12 ) ;
		return ;
	}

	pVAR* var = v_it->second ;

	if ( var->pVAR_system )
	{
		err.seterrid( TRACE_INFO(), "PSYE015E", v_it->first, 12 ) ;
		return ;
	}

	if ( issysProfile() )
	{
		err.seterrid( TRACE_INFO(), "PSYE015C", 16 ) ;
		return ;
	}

	if ( var->pVAR_type != pV_VALUE )
	{
		err.seterrid( TRACE_INFO(), "PSYE015D", v_it->first ) ;
		return ;
	}

	delete var ;
	POOL.erase( v_it ) ;
	changed = true ;
}


bool pVPOOL::isSystem( map<string, pVAR*>::iterator v_it )
{
	return v_it->second->pVAR_system ;
}


void pVPOOL::createGenEntries()
{
	POOL[ "ZTIME"    ] = new pVAR( pV_ZTIME    ) ;
	POOL[ "ZTIMEL"   ] = new pVAR( pV_ZTIMEL   ) ;
	POOL[ "ZDATE"    ] = new pVAR( pV_ZDATE    ) ;
	POOL[ "ZDAY"     ] = new pVAR( pV_ZDAY     ) ;
	POOL[ "ZDAYOFWK" ] = new pVAR( pV_ZDAYOFWK ) ;
	POOL[ "ZDATESTD" ] = new pVAR( pV_ZDATESTD ) ;
	POOL[ "ZMONTH"   ] = new pVAR( pV_ZMONTH   ) ;
	POOL[ "ZJDATE"   ] = new pVAR( pV_ZJDATE   ) ;
	POOL[ "ZJ4DATE"  ] = new pVAR( pV_ZJ4DATE  ) ;
	POOL[ "ZYEAR"    ] = new pVAR( pV_ZYEAR    ) ;
	POOL[ "ZSTDYEAR" ] = new pVAR( pV_ZSTDYEAR ) ;
	POOL[ "ZDEBUG"   ] = new pVAR( pV_ZDEBUG   ) ;
	POOL[ "ZTASKID"  ] = new pVAR( pV_ZTASKID  ) ;
}


void pVPOOL::load( errblock& err,
		   const string& applid,
		   const string& path )
{
	//
	// RC = 0  Normal completion.
	// RC = 20 Severe error.
	//

	string fname ;
	string hdr   ;
	string var   ;
	string value ;

	uint k  ;
	uint i  ;
	uint n1 ;
	uint n2 ;

	char* buf1  ;
	char z[ 2 ] ;
	char x      ;

	size_t buf1Size = 1024 ;

	err.setRC( 0 ) ;

	fname = path + applid + "PROF" ;

	std::ifstream profile ;
	profile.open( fname.c_str(), ios::binary ) ;
	if ( not profile.is_open() )
	{
		err.seterrid( TRACE_INFO(), "PSYE015F", applid, fname ) ;
		return ;
	}

	buf1 = new char[ buf1Size ] ;

	profile.read (buf1, 2 ) ;
	if ( memcmp( buf1, "\x00\x84", 2 ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE015G", applid, fname ) ;
		profile.close() ;
		delete[] buf1   ;
		return  ;
	}

	profile.get( x ) ;
	i = ( unsigned char )x ;
	if ( i > 1 )
	{
		err.seterrid( TRACE_INFO(), "PSYE015H", d2ds( i ), applid ) ;
		profile.close() ;
		delete[] buf1   ;
		return  ;
	}

	profile.get( x ) ;
	i = ( unsigned char )x  ;
	profile.read (buf1, i ) ;
	hdr.assign( buf1, i )   ;

	while ( true )
	{
		profile.get( x ) ;
		if ( profile.eof() ) { break ; }
		i = ( unsigned char )x  ;
		profile.read (buf1 , i) ;
		if ( profile.fail() != 0 ) { err.seterror() ; break ; }
		var.assign( buf1, i ) ;
		profile.read( z, 2 )  ;
		if ( profile.fail() != 0 ) { err.seterror() ; break ; }
		n1 = ( unsigned char )z[ 0 ] ;
		n2 = ( unsigned char )z[ 1 ] ;
		k = 256 * n1 + n2 ;
		if ( k > buf1Size )
		{
			delete[] buf1 ;
			buf1Size = k  ;
			buf1     = new char[ buf1Size ] ;
		}
		profile.read (buf1, k ) ;
		if ( profile.fail() != 0 ) { err.seterror() ; break ; }
		value.assign( buf1, k ) ;
		put( err, var, value ) ;
	}
	profile.close() ;
	delete[] buf1   ;
	if ( err.error() )
	{
		err.seterrid( TRACE_INFO(), "PSYE015I", applid ) ;
	}
	resetChanged() ;
}


void pVPOOL::save( errblock& err,
		   const string& applid )
{
	//
	// RC = 0  Normal completion.
	// RC = 4  Save not performed.  Pool in read-only or no changes made to pool.
	// RC = 20 Severe error.
	//

	int i ;

	string fname  ;
	string fname1 ;
	string fname2 ;

	bool file_exists ;

	err.setRC( 0 ) ;

	if ( readOnly || !changed )
	{
		err.setRC( 4 ) ;
		return ;
	}

	try
	{
		if ( exists( path ) )
		{
			if ( !is_directory( path ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE015J", path ) ;
				return ;
			}
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE015K", path ) ;
			return ;
		}
	}
	catch ( boost::filesystem::filesystem_error &e )
	{
		err.seterrid( TRACE_INFO(), "PSYS012C", e.what() ) ;
		return ;
	}
	catch (...)
	{
		err.seterrid( TRACE_INFO(), "PSYS012C", "Entry: "+ path ) ;
		return ;
	}

	if ( path.back() != '/' ) { path += "/" ; }
	fname = path + applid + "PROF" ;

	fname1 = fname ;

	file_exists = exists( fname ) ;

	if ( file_exists )
	{
		fname2 = fname ;
		fname1.insert( fname.find_last_of( '/' ) + 1, "~" ) ;
		fname2.insert( fname.find_last_of( '/' ) + 1, "~~" ) ;
	}

	std::ofstream profile ;
	profile.open( fname1.c_str(), ios::binary | ios::out ) ;
	if ( !profile.is_open() )
	{
		err.seterrid( TRACE_INFO(), "PSYE041M", "PROFILE", fname ) ;
		return ;
	}

	profile << (char)00  ;  //
	profile << (char)132 ;  // x0084 denotes a profile.
	profile << (char)1   ;  // PROFILE format version 1.
	profile << (char)44  ;  // Header length.
	profile << "HDR                                         " ;

	map<string, pVAR*>::iterator it ;
	for ( it = POOL.begin() ; it != POOL.end() ; ++it )
	{
		i = it->first.size() ;
		profile << (char)i ;
		profile.write( it->first.c_str(), i ) ;
		i = it->second->pVAR_value.size() ;
		profile << (char)( i >> 8 ) ;
		profile << (char)( i ) ;
		profile.write( it->second->pVAR_value.c_str(), i ) ;
	}
	profile.close() ;
	resetChanged()  ;

	if ( file_exists )
	{
		rename( fname, fname2 ) ;
		rename( fname1, fname ) ;
		remove( fname2 ) ;
	}
}


// ******************************************************************************************************************************
// ************************************************   POOL MANAGER SECTION   ****************************************************
// ******************************************************************************************************************************


poolMGR::poolMGR()
{
	//
	// Create pools @DEFSHAR, @DEFPROF and @ROXPROF.
	// @DEFPROF and @ROXPROF (Read Only Extention PROFILE) not currently used.
	//

	pVPOOL* pool = new pVPOOL ;

	pool->createGenEntries() ;
	POOLs_shared[ "@DEFSHAR" ] = pool ;

	POOLs_profile[ "@DEFPROF" ] = new pVPOOL ;
	POOLs_profile[ "@ROXPROF" ] = new pVPOOL ;

	shrdPool = 0  ;
	ppath    = "" ;
}


poolMGR::~poolMGR()
{
	//
	// Iterate over all remaining variable pools and delete to release dynamic storage.
	//

	for ( auto it = POOLs_shared.begin() ; it != POOLs_shared.end() ; ++it )
	{
		delete it->second ;
	}

	for ( auto it = POOLs_profile.begin() ; it != POOLs_profile.end() ; ++it )
	{
		delete it->second ;
	}

	for ( auto it = POOLs_lscreen.begin() ; it != POOLs_lscreen.end() ; ++it )
	{
		delete it->second ;
	}
}


void poolMGR::connect( int taskid,
		       const string& ppool,
		       int spool )
{
	//
	// Add profile and shared pool names to the task table.  Increment use count for both.
	//

	errblock err ;

	llog( "I", "Connecting task "<< taskid <<" to pool manager" <<endl ) ;
	llog( "I", "Profile pool: "<< ppool <<endl ) ;
	llog( "I", "Shared  pool: "<< d2ds( spool, 8 ) <<endl ) ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	task_table[ taskid ] = make_pair( ppool, spool ) ;

	POOLs_profile[ ppool ]->incRefCount() ;
	POOLs_shared[ d2ds( spool, 8 ) ]->incRefCount() ;
}


void poolMGR::disconnect( int taskid )
{
	//
	// Remove task from the profile/shared task tables and decrement use counts.
	// If pool no longer in use, save if a profile, and delete it.
	//

	errblock err ;

	string ppool ;
	string spool ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	auto it = task_table.find( taskid ) ;
	ppool = it->second.first ;
	spool = d2ds( it->second.second, 8 ) ;

	auto itp = POOLs_profile.find( ppool ) ;
	auto its = POOLs_shared.find( spool )  ;
	itp->second->decRefCount() ;
	its->second->decRefCount() ;

	llog( "I", "Disconnecting task "<< taskid <<" from pool manager"<< endl ) ;
	task_table.erase( it ) ;

	if ( !itp->second->inUse() )
	{
		itp->second->save( err, itp->first ) ;
		if ( err.error() )
		{
			llog( "E", "Pool "<< itp->first <<" cannot be saved"<< endl ) ;
		}
		else
		{
			delete itp->second ;
			POOLs_profile.erase( itp ) ;
		}
	}

	if ( !its->second->inUse() )
	{
		delete its->second ;
		POOLs_shared.erase( its ) ;
	}
}


void poolMGR::setProfilePath( errblock& err,
			      const string& p )
{
	err.setRC( 0 ) ;

	ppath = p ;
	if ( ppath.back() != '/' ) { ppath.push_back( '/' ) ; }

	try
	{
		if ( exists( ppath ) )
		{
			if ( !is_directory( ppath ) )
			{
				llog( "E", ppath << " is not a regular directory for profile load"<< endl ) ;
				err.seterror() ;
			}
		}
		else
		{
			llog( "E", "Directory "<< ppath <<" does not exist for profile load"<< endl ) ;
			err.seterror() ;
		}
	}
	catch ( boost::filesystem::filesystem_error &e )
	{
		err.seterrid( TRACE_INFO(), "PSYS012C", e.what() ) ;
	}
	catch (...)
	{
		err.seterrid( TRACE_INFO(), "PSYS012C", "Entry: "+ ppath ) ;
	}
}


void poolMGR::setPools( errblock& err )
{
	//
	// Lock is held when this is called, so don't lock (most routines).
	//

	err.setRC( 0 ) ;

	_shared = 0  ;
	_applid = "" ;

	if ( err.taskid == 0 )
	{
		llog( "E", "Logic error.  Task id cannot be zero "<< endl ) ;
		err.seterror() ;
		return ;
	}

	auto it = task_table.find( err.taskid ) ;
	if ( it == task_table.end() )
	{
		llog( "E", "Logic error.  Task "<< err.taskid <<" not connected to pool manager"<< endl ) ;
		err.seterror() ;
		return ;
	}

	_applid = it->second.first  ;
	_shared = it->second.second ;
}


void poolMGR::setPOOLsReadOnly()
{
	//
	// Neither of these pools is currently used.
	//

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	POOLs_profile[ "@ROXPROF" ]->setReadOnly() ;
	POOLs_profile[ "@DEFPROF" ]->setReadOnly() ;
}


void poolMGR::createProfilePool( errblock& err,
				 const string& ppool,
				 bool no_load )
{
	//
	// RC = 0  Pool created and loaded from existing file.
	// RC = 4  Pool created but not loaded as PROFILE file does not exist.
	// RC = 20 Severe error.
	//

	bool f_exists = false ;
	bool f_okay   = false ;

	string fname ;

	pair<map<string, pVPOOL*>::iterator, bool> result ;

	pVPOOL* pool ;

	err.setRC( 0 ) ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	if ( POOLs_profile.count( ppool ) > 0 ) { return ; }

	fname = ppath + ppool + "PROF" ;
	try
	{
		if ( exists( fname ) )
		{
			f_exists = true ;
			if ( is_regular_file( fname ) )
			{
				f_okay = true ;
			}
		}
	}
	catch ( boost::filesystem::filesystem_error &e )
	{
		err.seterrid( TRACE_INFO(), "PSYS012C", e.what() ) ;
		return ;
	}
	catch (...)
	{
		err.seterrid( TRACE_INFO(), "PSYS012C", "Entry: "+ fname ) ;
		return ;
	}

	if ( f_exists && !no_load )
	{
		if ( not f_okay )
		{
			llog( "E", "File "+ fname +" is not a regular file for profile load"<< endl ) ;
			err.seterror() ;
			return ;
		}
		else
		{
			result = POOLs_profile.insert( pair<string, pVPOOL*>( ppool, new pVPOOL ) ) ;
			pool   = result.first->second ;
			pool->path = ppath ;
			llog( "I", "Pool "+ ppool +" created okay.  Reading saved variables from profile dataset"<< endl ) ;
			pool->load( err, ppool, ppath ) ;
			if ( err.error() )
			{
				delete pool ;
				POOLs_profile.erase( ppool ) ;
				return ;
			}
			pool->setProfile() ;
			if ( ppool == "ISPS" )
			{
				pool->sysProfile() ;
			}
		}
	}
	else
	{
		llog( "I", "Profile "+ ppool +"PROF does not exist.  Creating default"<< endl ) ;
		result = POOLs_profile.insert( pair<string, pVPOOL*>( ppool, new pVPOOL ) ) ;
		pool   = result.first->second ;
		pool->path = ppath ;
		pool->put( err, "ZHTOP", HTOP ) ;
		llog( "I", "Profile Pool "+ ppool +" created okay in path "+ ppath <<endl ) ;
		err.setRC( 4 ) ;
	}
}


int poolMGR::createSharedPool()
{
	boost::lock_guard<boost::mutex> lock( mtx ) ;

	POOLs_shared[ d2ds( ++shrdPool, 8 ) ] = new pVPOOL ;

	return shrdPool ;
}


map<int, pVPOOL*>::iterator poolMGR::createPool( int ls )
{
	//
	// Create the logical-screen variable pool and add defaults.
	// This pool is not accessible by applications (internal use only).
	//
	// Returns:  Iterator to the pool.
	//
	// Lock is held when this is called, so don't lock (put and get routines).
	//

	errblock err ;

	pair<map<int, pVPOOL*>::iterator, bool> result ;

	result = POOLs_lscreen.insert( pair<int, pVPOOL*>( ls, new pVPOOL ) ) ;

	pVPOOL* pool = result.first->second ;

	pool->put( err, "ZMSGID",   "",  USER ) ;
	pool->put( err, "ZSCRNAME", "",  USER ) ;
	pool->put( err, "ZSCRNAM2", "",  USER ) ;
	pool->put( err, "ZSCRNAM3", "",  USER ) ;
	pool->put( err, "ZSHMSGID", "N", USER ) ;
	pool->put( err, "ZSHPANID", "N", USER ) ;
	pool->put( err, "ZSHUSRID", "N", USER ) ;
	pool->put( err, "ZREFUPDT", "Y", USER ) ;
	pool->put( err, "ZFHUPDT",  "Y", USER ) ;
	pool->put( err, "ZDEBUG",  "00", USER ) ;

	return result.first ;
}


void poolMGR::destroySystemPool( errblock& err )
{
	//
	// Save ISPS pool and remove it from storage.  Called when lspf is terminating.
	//

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	auto it = POOLs_profile.find( "ISPS" ) ;
	it->second->save( err, "ISPS" ) ;

	delete it->second ;
	POOLs_profile.erase( it ) ;
}


void poolMGR::destroyPool( int ls )
{
	//
	// Remove the logical-screen pool when a logical screen is closed.
	//

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	auto it = POOLs_lscreen.find( ls ) ;
	if ( it != POOLs_lscreen.end() )
	{
		delete it->second ;
		POOLs_lscreen.erase( it ) ;
	}
}


void poolMGR::statistics()
{
	string Mode ;

	errblock err ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	llog( "I", ".STATS" << endl ) ;
	llog( "-", "Pool Statistics:" << endl ) ;
	llog( "-", "         Number of shared pools . . . . " << POOLs_shared.size() << endl ) ;
	llog( "-", "         Number of profile pools. . . . " << POOLs_profile.size() << endl ) ;
	llog( "-", "         Number of connected tasks. . . " << task_table.size() << endl ) ;
	llog( "-", "         Profile directory. . . . . . . " << ppath << endl ) ;
	llog( "-", endl ) ;
	llog( "-", "         Shared pool details:" << endl ) ;

	for ( auto sp_it = POOLs_shared.begin() ; sp_it != POOLs_shared.end() ; ++sp_it )
	{
		Mode = sp_it->second->readOnly ? "RO" : "UP" ;
		llog( "-", "            Pool " << setw(8) << sp_it->first <<
			  "  use count: " << setw(4) << sp_it->second->refCount <<
			  "  " << Mode << "  entries: " << setw(5) << sp_it->second->POOL.size() << endl ) ;
	}
	llog( "-", endl ) ;
	llog( "-", "         Profile pool details:" << endl ) ;

	for ( auto pp_it = POOLs_profile.begin() ; pp_it != POOLs_profile.end() ; ++pp_it )
	{
		Mode = pp_it->second->readOnly ? Mode = "RO" : Mode = "UP" ;
		llog( "-", "            Pool " << setw(8) << pp_it->first << "  use count: " << setw(4) << pp_it->second->refCount <<
			  "  " << Mode << "  entries: " << setw(5) << pp_it->second->POOL.size() << "  path: " << pp_it->second->path << endl ) ;
	}

	llog( "-", endl ) ;
	llog( "-", "         Connected Tasks:" << endl ) ;
	for ( auto it = task_table.begin() ; it != task_table.end() ; ++it )
	{
		llog( "-", "            Id   " << setw(5) << it->first <<
		      " Profile: " << setw(4) << it->second.first <<
		      "  Shared: " << d2ds( it->second.second, 8 ) << endl) ;
	}
	llog( "-", "*************************************************************************************************************" << endl ) ;
}


void poolMGR::snap()
{
	errblock err ;

	string vtype ;

	map<string, pVPOOL*>::iterator sp_it ;
	map<string, pVPOOL*>::iterator pp_it ;
	map<string, pVAR*>::iterator v_it    ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	llog( "I", ".SNAP" << endl ) ;
	llog( "-", "Pool Variables:" << endl ) ;
	llog( "-", "         Shared pool details:" << endl ) ;

	for ( sp_it = POOLs_shared.begin() ; sp_it != POOLs_shared.end() ; ++sp_it )
	{
		llog( "-", endl ) ;
		llog( "-", "         Pool " << setw(8) << sp_it->first << " use count:" << setw(3) << sp_it->second->refCount <<
			  " entries: " << sp_it->second->POOL.size() << endl ) ;
		for ( v_it = sp_it->second->POOL.begin() ; v_it != sp_it->second->POOL.end() ; ++v_it )
		{
			vtype = sp_it->second->isSystem( v_it ) ? vtype = " (SYS) " : vtype = " (USR) " ;
			llog( "-", setw(8) << std::left << v_it->first << vtype << sp_it->second->get( err, v_it ) << "<< " << endl ) ;
		}
	}
	llog( "-", endl ) ;
	llog( "-", "         Profile pool details:" << endl ) ;

	for ( pp_it = POOLs_profile.begin() ; pp_it != POOLs_profile.end() ; ++pp_it )
	{
		llog( "-", endl ) ;
		llog( "-", "         Pool " << setw(8) << pp_it->first << " use count: " << setw(3) << pp_it->second->refCount <<
			  " entries: " << pp_it->second->POOL.size() << endl ) ;
		llog( "-", "                            path: " << setw(3) << pp_it->second->path << endl ) ;
		for ( v_it = pp_it->second->POOL.begin() ; v_it != pp_it->second->POOL.end() ; ++v_it )
		{
			vtype = pp_it->second->isSystem( v_it ) ? vtype = " (SYS) " : vtype = " (USR) " ;
			llog( "-", setw(8) << std::left << v_it->first << vtype << pp_it->second->get( err, v_it ) << "<< " << endl ) ;
		}
	}
	llog( "-", "*************************************************************************************************************" << endl ) ;
}


set<string>& poolMGR::vlist( errblock& err,
			     int& RC,
			     poolType pType,
			     int lvl )
{
	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator   v_it ;

	RC = 0 ;
	varList.clear() ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for vlist"<< endl ) ;
		return varList;
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for vlist"<< endl ) ;
		return varList;
	}

	switch( pType )
	{
	case SHARED:
		switch ( lvl )
		{
		case 1:
			p_it = POOLs_shared.find( d2ds( _shared, 8 ) ) ;
			break ;
		case 2:
			p_it = POOLs_shared.find( "@DEFSHAR" ) ;
			break ;
		default:
			RC = 20 ;
			return varList ;
		}
		break ;
	case PROFILE:
		switch ( lvl )
		{
		case 1:
			p_it = POOLs_profile.find( _applid ) ;
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
			RC = 20 ;
			return varList ;
		}
		break ;

	default:
		RC = 20 ;
		return varList ;
	}

	for ( v_it = p_it->second->POOL.begin() ; v_it != p_it->second->POOL.end() ; ++v_it )
	{
		varList.insert( v_it->first ) ;
	}

	return varList ;
}


void poolMGR::sysput( errblock& err,
		      const string& name,
		      const string& value,
		      poolType pType )
{
	//
	// Put variables to the profile ISPS pool or @DEFSHAR shared pool.
	// These calls are not associated with an application.
	//

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	switch ( pType )
	{
		case SHARED:
			POOLs_shared[ "@DEFSHAR" ]->put( err, name, value, SYSTEM ) ;
			break ;

		case PROFILE:
			POOLs_profile[ "ISPS" ]->put( err, name, value, SYSTEM ) ;
			break ;

		default:
			err.seterrid( TRACE_INFO(), "PSYE017D" ) ;
	}
}


void poolMGR::put( errblock& err,
		   const string& name,
		   const string& value,
		   poolType pType,
		   vTYPE vtype )
{
	//
	// Pool search order: ASIS - SHARED then PROFILE.
	//
	// RC = 0  variable put okay.
	// RC = 12 variable not put as in read-only status.
	// RC = 20 severe error.
	//
	// For put PROFILE, delete the variable from the SHARED pool.
	//

	map<string, pVPOOL*>::iterator sp_it ;
	map<string, pVPOOL*>::iterator pp_it ;
	map<string, pVPOOL*>::iterator p_it  ;

	map<string, pVAR*>::iterator v_it    ;

	errblock err2 ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "pool manager PUT", name ) ;
		return ;
	}

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for put "<<name<< endl ) ;
		return ;
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for put "<<name<< endl ) ;
		return ;
	}

	switch( pType )
	{
	case ASIS:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if      ( err.RC0() ) { p_it->second->put( err, v_it, value, vtype ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
			if      ( err.RC0() ) {  p_it->second->put( err, v_it, value, vtype ) ; }
			else if ( err.RC8() ) { sp_it->second->put( err, name, value, vtype ) ; }
		}
		break ;

	case SHARED:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if      ( err.RC0() ) {  p_it->second->put( err, v_it, value, vtype ) ; }
		else if ( err.RC8() ) { sp_it->second->put( err, name, value, vtype ) ; }
		break ;

	case PROFILE:
		locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
		if      ( err.RC0() ) {  p_it->second->put( err, v_it, value, vtype ) ; }
		else if ( err.RC8() ) { pp_it->second->put( err, name, value, vtype ) ; }
		locateSubPool( err2, sp_it, p_it, v_it, _shared, name ) ;
		if ( err2.RC0() )     {  p_it->second->erase( err, v_it ) ; }
		break ;

	default:
		err.seterrid( TRACE_INFO(), "PSYE017C" ) ;
	}
}


void poolMGR::put( errblock& err,
		   int ls,
		   const string& name,
		   const string& value )
{
	//
	// Set a variable in the logical-screen pool.
	// Pool is created on first access.
	//

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	err.setRC( 0 ) ;

	auto it = POOLs_lscreen.find( ls ) ;
	if ( it == POOLs_lscreen.end() )
	{
		it = createPool( ls ) ;
	}

	it->second->put( err, name, value, USER ) ;
}


string poolMGR::sysget( errblock& err,
			const string& name,
			poolType pType )
{
	//
	// Get variables from the profile ISPS pool or @DEFSHAR shared pool.
	// These calls are not associated with an application.
	//

	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator v_it   ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	switch ( pType )
	{
		case SHARED:
			p_it = POOLs_shared.find( "@DEFSHAR" ) ;
			break ;

		case PROFILE:
			p_it = POOLs_profile.find( "ISPS" ) ;
			break ;

		default:
			err.seterrid( TRACE_INFO(), "PSYE017D" ) ;
			return "" ;
	}

	v_it = p_it->second->POOL.find( name ) ;

	return p_it->second->get( err, v_it ) ;
}


string poolMGR::get( errblock& err,
		     const string& name,
		     poolType pType )
{
	//
	// Pool search order: ASIS - SHARED then PROFILE.
	//
	// RC = 0  variable found.
	// RC = 8  variable not found.
	// RC = 20 severe error.
	//
	// For get PROFILE, delete the variable from the SHARED pool even if not found in the PROFILE pool.
	//

	errblock err2 ;

	map<string, pVPOOL*>::iterator pp_it ;
	map<string, pVPOOL*>::iterator sp_it ;
	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator v_it ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "pool manager GET", name ) ;
		return "" ;
	}

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for get "<<name<< endl ) ;
		return "";
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for get "<<name<< endl ) ;
		return "";
	}

	switch ( pType )
	{
	case ASIS:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if      ( err.RC0() ) { return p_it->second->get( err, v_it ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
			if ( err.RC0() ) { return p_it->second->get( err, v_it ) ; }
		}
		break ;

	case SHARED:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { return p_it->second->get( err, v_it ) ; }
		break ;

	case PROFILE:
		locateSubPool( err2, sp_it, p_it, v_it, _shared, name ) ;
		if ( err2.RC0() ) { p_it->second->erase( err, v_it ) ; }
		locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
		if ( err.RC0() ) { return p_it->second->get( err, v_it ) ; }
		break ;

	default:
		err.seterrid( TRACE_INFO(), "PSYE017C" ) ;
	}

	return "" ;
}


string poolMGR::get( errblock& err,
		     int ls,
		     const string& name )
{
	//
	// Retrieve a variable from the logical-screen pool.
	// Pool is created on first access.
	//

	err.setRC( 0 ) ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	auto p_it = POOLs_lscreen.find( ls ) ;
	if ( p_it == POOLs_lscreen.end() )
	{
		p_it = createPool( ls ) ;
	}

	auto v_it = p_it->second->POOL.find( name ) ;
	if ( v_it == p_it->second->POOL.end() )
	{
		return "" ;
	}

	return p_it->second->get( err, v_it ) ;
}


string* poolMGR::vlocate( errblock& err,
			  const string& name,
			  poolType pType )
{
	//
	// Pool search order: ASIS - SHARED then PROFILE.
	//
	// RC = 0  variable found.
	// RC = 8  variable not found.
	// RC = 20 severe error.
	//

	map<string, pVPOOL*>::iterator pp_it ;
	map<string, pVPOOL*>::iterator sp_it ;
	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator v_it ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "pool manager VLOCATE", name ) ;
		return nullptr ;
	}

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for vlocate "<<name<< endl ) ;
		return nullptr ;
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for vlocate "<<name<< endl ) ;
		return nullptr;
	}

	switch ( pType )
	{
	case ASIS:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if      ( err.RC0() ) { return p_it->second->vlocate( err, v_it ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
			if ( err.RC0() ) { return p_it->second->vlocate( err, v_it ) ; }
		}
		break ;

	case SHARED:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { return p_it->second->vlocate( err, v_it ) ; }
		break ;

	case PROFILE:
		locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
		if ( err.RC0() ) { return p_it->second->vlocate( err, v_it ) ; }
		break ;

	default:
		err.seterrid( TRACE_INFO(), "PSYE017C" ) ;
	}

	return nullptr ;
}



void poolMGR::erase( errblock& err,
		     const string& name,
		     poolType pType )
{
	//
	// Pool search order: ASIS - SHARED then PROFILE.
	//
	// RC = 0  variable found and erased (for BOTH, either pool)
	// RC = 8  variable not found (for BOTH, neither pool)
	// RC = 12 variable not erased as pool in read-only status, or system variable.
	// RC = 16 variable not erased as in the ISPS SYSTEM profile pool.
	// RC = 20 severe error.
	//

	uint maxRC = 0 ;

	bool found = false ;

	map<string, pVPOOL*>::iterator pp_it ;
	map<string, pVPOOL*>::iterator sp_it ;
	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator v_it ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE013A", "pool manager ERASE", name ) ;
		return ;
	}

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for erase "<<name<< endl ) ;
		return ;
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for erase "<<name<< endl ) ;
		return ;
	}

	switch( pType )
	{
	case ASIS:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
			if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		}
		break ;

	case SHARED:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		break ;

	case PROFILE:
		locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
		if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		break ;

	case BOTH:
		locateSubPool( err, sp_it, p_it, v_it, _shared, name ) ;
		if ( err.RC0() )
		{
			found = true ;
			p_it->second->erase( err, v_it ) ;
		}
		if ( err.error() ) { break ; }
		maxRC = err.getRC() ;
		locateSubPool( err, pp_it, p_it, v_it, _applid, name ) ;
		if ( err.RC0() )
		{
			found = true ;
			p_it->second->erase( err, v_it ) ;
		}
		if ( err.error() ) { break ; }
		maxRC = max( maxRC, err.getRC() ) ;
		err.setRC( ( found && maxRC == 8 ) ? 0 : maxRC ) ;
		break ;
	}
}


void poolMGR::locateSubPool( errblock& err,
			     map<string, pVPOOL*>::iterator& pp_it,
			     map<string, pVPOOL*>::iterator& p_it,
			     map<string, pVAR*>::iterator& v_it,
			     const string& pool,
			     const string& name )
{
	//
	// Locate the variable name in a profile sub-pool.
	//
	// RC = 0 variable found.  Pool iterators pp_it, p_it and variable iterator v_it, will be valid on return.
	// RC = 8 if variable not found.
	//
	// Sub-Pool search order:
	// PROFILE: APPLID, @ROXPROF @DEFPROF ISPS.
	//
	// pp_it - iterator to the application profile sub-pool 'pool'.
	//  p_it - iterator to the profile sub-pool where variable is located, if found ( when RC=0 ).
	//  v_it - iterator to the variable 'name' if found ( when RC=0 ).
	//

	err.setRC( 0 ) ;

	p_it = POOLs_profile.find( pool ) ;
	if ( p_it == POOLs_profile.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE017E", pool ) ;
		return ;
	}

	pp_it = p_it ;
	v_it  = p_it->second->POOL.find( name ) ;
	if ( v_it == p_it->second->POOL.end() )
	{
		p_it = POOLs_profile.find( "@ROXPROF" ) ;
		v_it = p_it->second->POOL.find( name ) ;
		if ( v_it == p_it->second->POOL.end() )
		{
			p_it = POOLs_profile.find( "@DEFPROF" ) ;
			v_it = p_it->second->POOL.find( name ) ;
			if ( v_it == p_it->second->POOL.end() )
			{
				p_it = POOLs_profile.find( "ISPS" ) ;
				v_it = p_it->second->POOL.find( name ) ;
				if ( v_it == p_it->second->POOL.end() ) { err.setRC( 8 ) ; }
			}
		}
	}
}


void poolMGR::locateSubPool( errblock& err,
			     map<string, pVPOOL*>::iterator& sp_it,
			     map<string, pVPOOL*>::iterator& p_it,
			     map<string, pVAR*>::iterator& v_it,
			     int pool,
			     const string& name )
{
	//
	// Locate the variable name in the shared pool sub-pool.
	//
	// RC = 0 variable found.  Pool iterator p_it and variable iterator v_it, will be valid on return.
	// RC = 8 if variable not found.
	//
	// Sub-Pool search order:
	// SHARED:  CURRENT @DEFSHAR
	//
	// sp_it - iterator to the shared sub-pool 'pool'.
	//  p_it - iterator to the shared sub-pool where variable is located, if found ( when RC=0 ).
	//  v_it - iterator to the variable 'name' if found ( when RC=0 ).
	//

	err.setRC( 0 ) ;

	p_it = POOLs_shared.find( d2ds( pool, 8 ) ) ;
	if ( p_it == POOLs_shared.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE017E", pool ) ;
		return ;
	}

	sp_it = p_it ;
	v_it  = p_it->second->POOL.find( name ) ;
	if ( v_it == p_it->second->POOL.end() )
	{
		p_it = POOLs_shared.find( "@DEFSHAR" ) ;
		v_it = p_it->second->POOL.find( name )  ;
		if ( v_it == p_it->second->POOL.end() ) { err.setRC( 8 ) ; }
	}
}
