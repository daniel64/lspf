/*  Compile with ::                                                                                     */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPOREXX1.so -lrexx -lrexxapi -o libPOREXX1.so POREXX1.cpp */

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
/* OOREXX-lspf interface module                                                                         */
/*                                                                                                      */
/* Setup command handler and call REXX interpreter                                                      */
/* (command handler lspfServiceHandler used for address ISPEXEC rexx statements for lspf services)      */
/*                                                                                                      */
/* PARM word 1 is the rexx to invloke                                                                   */
/* PARM words 2 onwards are the parameters for the rexx (Arg(1))                                        */
/*                                                                                                      */
/* ZORXPATH is used to find the exec if a fully qualified name not passed                               */
/*                                                                                                      */
/* Use system variable ZOREXPGM to refer to this module rather than the name, to allow different        */
/* rexx implementations to co-exist.                                                                    */

/* RC/RSN codes returned                                                                                */
/*   0/0  Okay                                                                                          */
/*  20/condition.code ZRESULT is set to condition.message                                               */

#include <boost/filesystem.hpp>

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "POREXX1.h"

#include <oorexxapi.h>


using namespace std ;
using namespace boost::filesystem ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME POREXX1


RexxObjectPtr RexxEntry lspfServiceHandler( RexxExitContext *, RexxStringObject, RexxStringObject ) ;

int getRexxVariable( pApplication *, string, string & ) ;
int setRexxVariable( string, string ) ;
int getAllRexxVariables( pApplication * ) ;
int setAllRexxVariables( pApplication * ) ;

void lspfSyntaxError( pApplication *, const string & ) ;

void POREXX1::application()
{
	log( "I", "Application POREXX1 starting." << endl ) ;

	int i ;
	int j ;

	bool found ;

	size_t version  ;
	string rxsource ;
	string path     ;

	RexxInstance *instance   ;
	RexxThreadContext *threadContext ;
	RexxArrayObject args     ;
	RexxCondition condition  ;
	RexxDirectoryObject cond ;
	RexxObjectPtr result     ;

	RexxContextEnvironment environments[ 2 ] ;
	RexxOption             options[ 3 ]      ;

	environments[ 0 ].handler = lspfServiceHandler ;
	environments[ 0 ].name    = "ISPEXEC" ;
	environments[ 1 ].handler = NULL ;
	environments[ 1 ].name    = ""   ;

	options[ 0 ].optionName = APPLICATION_DATA;
	options[ 0 ].option     = (void *)this;
	options[ 1 ].optionName = DIRECT_ENVIRONMENTS ;
	options[ 1 ].option     = (void *)environments ;
	options[ 2 ].optionName = ""   ;

	rxsource = word( PARM, 1 )    ;
	PARM     = subword( PARM, 2 ) ;
	found    = false              ;

	if ( rxsource.size() > 0 && rxsource[ 0 ] == '%' ) { rxsource.erase( 0, 1 ) ; }
	if ( rxsource == "" )
	{
		log( "E", "POREXX1 error. No REXX passed" << endl ) ;
		ZRC     = 16 ;
		ZRSN    = 4  ;
		ZRESULT = "No REXX passed" ;
		cleanup() ;
		return    ;
	}

	if ( rxsource[ 0 ] == '/' ) { rexxName = rxsource ; }
	else
	{
		j = getpaths( ZORXPATH ) ;
		for ( i = 1 ; i <= j ; i++ )
		{
			path     = getpath( ZORXPATH, i ) ;
			if ( path.back() != '/' ) { path = path + "/" ; }
			rexxName = path + rxsource ;
			if ( !exists( rexxName ) ) { continue ; }
			if ( is_regular_file( rexxName ) ) { found = true ; break ; }
			log( "E", "POREXX1 error. " << rxsource << " found but is not a regular file" << endl ) ;
			setmsg( "PSYS012B" ) ;
			ZRC     = 16 ;
			ZRSN    = 12 ;
			ZRESULT = "Invalid REXX passed" ;
			cleanup() ;
			return    ;
		}
		if ( !found )
		{
			log( "E", "POREXX1 error. " << rxsource << " not found in ZORXPATH concatination" << endl ) ;
			setmsg( "PSYS012C" ) ;
			ZRC     = 16 ;
			ZRSN    = 8  ;
			ZRESULT = "REXX not found" ;
			cleanup() ;
			return    ;
		}
	}

	if ( RexxCreateInterpreter( &instance, &threadContext, options ) )
	{
		args = threadContext->NewArray( 1 ) ;
		threadContext->ArrayPut(args, threadContext->String( PARM.c_str() ), 1 ) ;
		version = threadContext->InterpreterVersion() ;

		log( "I", "Starting OOREXX Interpreter Version. .: " << version << endl ) ;
		log( "I", "Running program. . . . . . . . . . . .: " << rxsource << endl ) ;
		log( "I", "With parameters. . . . . . . . . . . .: " << PARM << endl ) ;

		result = threadContext->CallProgram( rexxName.c_str(), args) ;

		if ( threadContext->CheckCondition() )
		{
			cond = threadContext->GetConditionInfo() ;
			threadContext->DecodeConditionInfo( cond, &condition ) ;
			log( "E", "POREXX1 error running REXX.: " << rxsource << endl ) ;
			log( "E", "   CONDITION CODE . . . . .: " << condition.code << endl ) ;
			log( "E", "   CONDITION ERROR TEXT . .: " << threadContext->CString( condition.errortext ) << endl ) ;
			log( "E", "   CONDITION MESSAGE. . . .: " << threadContext->CString( condition.message ) << endl ) ;
			setmsg( "PSYS011M" ) ;
			ZRC     = 20 ;
			ZRSN    = condition.code ;
			ZRESULT = threadContext->CString( condition.message ) ;
		}
		else
		{
			if ( result != NULLOBJECT )
			{
				CSTRING resultString = threadContext->CString( result ) ;
			}
		}
		instance->Terminate() ;
	}
	cleanup() ;
	return    ;
}


RexxObjectPtr RexxEntry lspfServiceHandler( RexxExitContext *context,
					    RexxStringObject address,
					    RexxStringObject command )
{
	// Called on address ISPEXEC REXX statement
	// Call lspf using the ispexec interface

	// On entry, update lspf function pool with the rexx variable pool values
	// On exit, update the rexx variable pool with the lspf function pool values

	int sRC   ;

	void * vptr ;

	string s1 = context->CString( address ) ;
	string s2 = context->CString( command ) ;

	vptr = context->GetApplicationData() ;
	pApplication * thisAppl = static_cast<pApplication *>(vptr) ;

	getAllRexxVariables( thisAppl ) ;

	thisAppl->ispexec( s2 ) ;
	sRC = thisAppl->RC      ;

	setAllRexxVariables( thisAppl ) ;
	return context->WholeNumber( sRC ) ;
}


int getAllRexxVariables( pApplication * thisAppl )
{
	// For all variables in the rexx variable pool, set the lspf function pool variable.
	// Executed on entry to the command handler from a REXX procedure before any lspf
	// function is called

	// If the vdefine is for an INTEGER, convert value to a number and vreplace

	int rc ;

	string n  ;
	string v  ;
	string vl ;

	vl = thisAppl->vilist() ;

	SHVBLOCK var ;
	while ( true )
	{
		var.shvnext            = NULL ;
		var.shvname.strptr     = NULL ;     /* let REXX allocate the memory */
		var.shvname.strlength  = 0 ;
		var.shvnamelen         = 0 ;
		var.shvvalue.strptr    = NULL ;     /* let REXX allocate the memory */
		var.shvvalue.strlength = 0 ;
		var.shvvaluelen        = 0 ;
		var.shvcode            = RXSHV_NEXTV ;
		var.shvret             = 0 ;
		rc = RexxVariablePool( &var ) ;     /* get the variable */
		if ( rc != RXSHV_OK) { break ; }
		n = string( var.shvname.strptr, var.shvname.strlength ) ;
		v = string( var.shvvalue.strptr, var.shvvalue.strlength ) ;
		if ( isvalidName( n ) )
		{
			if ( findword( n, vl ) )
			{
				thisAppl->vreplace( n, ds2d( v ) ) ;
			}
			else
			{
				thisAppl->vreplace( n, v ) ;
			}
		}
		RexxFreeMemory( (void *)var.shvname.strptr )  ;
		RexxFreeMemory( (void *)var.shvvalue.strptr ) ;
		if ( var.shvret & RXSHV_LVAR ) { break ; }
	}
	return rc ;
}


int setAllRexxVariables( pApplication * thisAppl )
{
	// For all variables in the application function pool, set the rexx variable.
	// Executed before returning to the REXX procedure after a call to the command handler
	// and after all lspf functions have been called

	int i  ;
	int ws ;
	int rc ;

	string vl ;
	string w  ;
	string vi ;
	string * vs ;

	vl = thisAppl->vslist() ;
	ws = words( vl ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		w = word( vl, i ) ;
		thisAppl->vcopy( w, vs, LOCATE ) ;
		rc = setRexxVariable( w, (*vs) ) ;
	}
	vl = thisAppl->vilist() ;
	ws = words( vl ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		w = word( vl, i ) ;
		thisAppl->vcopy( w, vi, MOVE ) ;
		rc = setRexxVariable( w, vi ) ;
	}
	return rc ;
}


int getRexxVariable( pApplication * thisAppl, string n, string & v )
{
	// Get variable value from Rexx variable pool and update the application function pool

	int rc ;

	const char * name = n.c_str() ;

	SHVBLOCK var ;                       /* variable pool control block*/
	var.shvcode = RXSHV_SYFET ;          /* do a symbolic fetch operation*/
	var.shvret  = 0 ;                    /* clear return code field */
	var.shvnext = NULL ;                 /* no next block */
	var.shvvalue.strptr    = NULL ;      /* let REXX allocate the memory */
	var.shvvalue.strlength = 0 ;
	var.shvvaluelen        = 0 ;
					     /* set variable name string */
	MAKERXSTRING( var.shvname, name, n.size() ) ;

	rc = RexxVariablePool( &var ) ;
	if ( rc == 0 )
	{
		v = string( var.shvvalue.strptr, var.shvvalue.strlength ) ;
		thisAppl->vreplace( n, v ) ;
		rc = thisAppl->RC          ;
	}
	RexxFreeMemory( (void *)var.shvvalue.strptr ) ;
	return rc ;
}


int setRexxVariable( string n, string v )
{
	const char * name = n.c_str() ;
	char * value      = (char *)v.c_str() ;

	SHVBLOCK var ;                       /* variable pool control block*/
	var.shvcode = RXSHV_SYSET  ;         /* do a symbolic set operation*/
	var.shvret  = 0 ;                    /* clear return code field */
	var.shvnext = NULL ;                 /* no next block */
					     /* set variable name string */
	MAKERXSTRING( var.shvname, name, n.size() ) ;
					     /* set value string */
	MAKERXSTRING( var.shvvalue, value, v.size() ) ;
	var.shvvaluelen = v.size()      ;    /* set value length */
	return RexxVariablePool( &var ) ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new POREXX1 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }

