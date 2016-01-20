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
/* (command handler lspfCommandHandler used for address ISPEXEC rexx statements for lspf services)      */
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

pApplication * thisAppl ;

RexxObjectPtr RexxEntry lspfCommandHandler( RexxExitContext *, RexxStringObject, RexxStringObject ) ;

int getRexxVariable( string, string & ) ;
int setRexxVariable( string, string ) ;
int getAllRexxVariables() ;
int setAllRexxVariables() ;

void lspfSyntaxError( string ) ;

int  lspfBrowse( string )   ;
int  lspfControl( string )  ;
int  lspfDisplay( string )  ;
int  lspfEdit( string )     ;
int  lspfSelect( string )   ;
int  lspfVPUT( string )     ;
int  lspfVGET( string )     ;


void POREXX1::application()
{
	log( "I", "Application POREXX1 starting." << endl ) ;

	int i ;
	int j ;

	bool found ;

	size_t version  ;
	string rxsource ;
	string RexxExec ;

	thisAppl = this ;

	RexxInstance *instance   ;
	RexxThreadContext *threadContext ;
	RexxArrayObject args     ;
	RexxCondition condition  ;
	RexxDirectoryObject cond ;
	RexxObjectPtr result     ;

	RexxContextEnvironment environments[ 2 ] ;
	RexxOption             options[ 2 ]      ;

	environments[ 0 ].handler = lspfCommandHandler ;
	environments[ 0 ].name    = "ISPEXEC" ;
	environments[ 1 ].handler = NULL ;
	environments[ 1 ].name    = ""   ;

	options[ 0 ].optionName = DIRECT_ENVIRONMENTS ;
	options[ 0 ].option     = (void *)environments ;
	options[ 1 ].optionName = ""   ;

	rxsource = word( PARM, 1 )    ;
	PARM     = subword( PARM, 2 ) ;
	found    = false              ;

	if ( rxsource[ 0 ] == '/' ) { RexxExec = rxsource ; }
	else
	{
		j = getpaths( ZORXPATH ) ;
		for ( i = 1 ; i <= j ; i++ )
		{
			RexxExec = getpath( ZORXPATH, i ) + "/" + rxsource ;
			if ( !exists( RexxExec ) ) { continue ; }
			if ( is_regular_file( RexxExec ) ) { found = true ; break ; }
			log( "E", "POREXX1 error. " << rxsource << " found but is not a regular file" << endl ) ;
			setmsg( "PSYS011B" ) ;
			cleanup() ;
			return    ;
		}
		if ( !found )
		{
			log( "E", "POREXX1 error. " << rxsource << " not found in ZORXPATH concatination" << endl ) ;
			setmsg( "PSYS011C" ) ;
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

		result = threadContext->CallProgram( RexxExec.c_str(), args) ;

		if ( threadContext->CheckCondition() )
		{
			cond = threadContext->GetConditionInfo() ;
			threadContext->DecodeConditionInfo( cond, &condition ) ;
			log( "E", "POREXX1 error running REXX.: " << rxsource << endl ) ;
			log( "E", "   CONDITION CODE . . . . .: " << condition.code << endl ) ;
			log( "E", "   CONDITION ERROR TEXT . .: " << threadContext->CString( condition.errortext ) << endl ) ;
			log( "E", "   CONDITION MESSAGE. . . .: " << threadContext->CString( condition.message ) << endl ) ;
			setmsg( "PSYS01M" ) ;
			ZRC     = 20 ;
			ZRSN    = condition.code ;
			ZRESULT = threadContext->CString( condition.message ) ;
		}
		else
		{
			if ( result != NULLOBJECT )
			{
				CSTRING resultString = threadContext->CString( result ) ;
				debug1( "Data passed back from REXX is :" << resultString << endl ) ;
				ZRESULT = resultString ;
			}
		}
		instance->Terminate() ;
	}
	cleanup() ;
	return    ;
}


int getAllRexxVariables()
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

	SHVBLOCK var ;                              /* variable pool control block*/
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


int setAllRexxVariables()
{
	// For all variables in the application function pool, set the rexx variable.
	// Executed before returning to the REXX procedure after a call to the command handler
	// and after all lspf functions have been called

	int i  ;
	int ws ;

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
		setRexxVariable( w, (*vs) ) ;
	}
	vl = thisAppl->vilist() ;
	ws = words( vl ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		w = word( vl, i ) ;
		thisAppl->vcopy( w, vi, MOVE ) ;
		setRexxVariable( w, vi ) ;
	}
}


int getRexxVariable( string n, string & v )
{
	// Get variable value from Rexx variable pool and update the application function pool

	int rc ;

	const char * name = n.c_str() ;
	char * value      = (char *)v.c_str() ;
	char valbuf[ 256 ] ;

	SHVBLOCK var ;                       /* variable pool control block*/
	var.shvcode = RXSHV_SYFET ;          /* do a symbolic fetch operation*/
	var.shvret  = 0 ;                    /* clear return code field */
	var.shvnext =(PSHVBLOCK)0 ;          /* no next block */
					     /* set variable name string */
	MAKERXSTRING( var.shvname, name, n.size() ) ;
	MAKERXSTRING( var.shvvalue, valbuf, sizeof( valbuf ) ) ;
					     /* get value string */
	rc = RexxVariablePool( &var ) ;      /* get the variable */
	if ( rc == 0 )
	{
		v = string( valbuf, var.shvvaluelen ) ;
		thisAppl->vreplace( n, v ) ;
		rc = thisAppl->RC          ;
	}
	return rc ;
}


int setRexxVariable( string n, string v )
{
	const char * name = n.c_str() ;
	char * value      = (char *)v.c_str() ;

	SHVBLOCK var ;                       /* variable pool control block*/
	var.shvcode = RXSHV_SYSET  ;         /* do a symbolic set operation*/
	var.shvret  = 0 ;                    /* clear return code field */
	var.shvnext = (PSHVBLOCK)0 ;         /* no next block */
					     /* set variable name string */
	MAKERXSTRING( var.shvname, name, n.size() ) ;
					     /* set value string */
	MAKERXSTRING( var.shvvalue, value, v.size() ) ;
	var.shvvaluelen = strlen( value ) ;  /* set value length */
	return RexxVariablePool( &var ) ;    /* set the variable */
}


RexxObjectPtr RexxEntry lspfCommandHandler( RexxExitContext *context,
					    RexxStringObject address,
					    RexxStringObject command )
{
	// Called on address ISPEXEC REXX statement
	// Translate string passed to the correct lspf dialogue function/parameters and call

	// On entry, update lspf function pool with the rexx variable pool values
	// On exit, update the rexx variable pool with the lspf function pool values

	int sRC ;
	int rc  ;
	int p1  ;
	int p2  ;

	string w1 ;
	string w2 ;

	const string InvalidServices = "VDEFINE VDELETE VCOPY VREPLACE" ;

	string s1 = context->CString( address ) ;
	string s2 = context->CString( command ) ;

	w1 = word( s2, 1 ) ;
	if ( findword( w1, InvalidServices ) )
	{
		log( "E", "Service " + w1 + " not valid from OOREXX"<<endl) ;
		context->RaiseCondition( "FAILURE", command, NULLOBJECT, context->WholeNumber( -1 ) ) ;
	}

	getAllRexxVariables() ;
	if ( w1 == "BROWSE" )
	{
		sRC = lspfBrowse( s2 ) ;
		if ( sRC > 8 || sRC < 0 )
		{
			context->RaiseCondition( "ERROR", command, NULLOBJECT, context->WholeNumber( sRC ) ) ;
		}
	}
	else if ( w1 == "DISPLAY" )
	{
		sRC = lspfDisplay( s2 ) ;
		if ( sRC > 8 || sRC < 0 )
		{
			context->RaiseCondition( "ERROR", command, NULLOBJECT, context->WholeNumber( sRC ) ) ;
		}
	}
	else if ( w1 == "CONTROL" )
	{
		sRC = lspfControl( s2 ) ;
		if ( sRC > 8 || sRC < 0 )
		{
			context->RaiseCondition( "ERROR", command, NULLOBJECT, context->WholeNumber( sRC ) ) ;
		}
	}
	else if ( w1 == "EDIT" )
	{
		sRC = lspfEdit( s2 ) ;
		if ( sRC > 8 || sRC < 0 )
		{
			context->RaiseCondition( "ERROR", command, NULLOBJECT, context->WholeNumber( sRC ) ) ;
		}
	}
	else if ( w1 == "SELECT" )
	{
		sRC = lspfSelect( s2 ) ;
		if ( sRC > 8 || sRC < 0 )
		{
			context->RaiseCondition( "ERROR", command, NULLOBJECT, context->WholeNumber( sRC ) ) ;
		}
	}
	else if ( w1 == "VGET" )
	{
		sRC = lspfVGET( s2 ) ;
		if ( sRC > 8 || sRC < 0 )
		{
			context->RaiseCondition( "ERROR", command, NULLOBJECT, context->WholeNumber( sRC ) ) ;
		}
	}
	else if ( w1 == "VPUT" )
	{
		sRC = lspfVPUT( s2 ) ;
		if ( sRC > 8 || sRC < 0 )
		{
			context->RaiseCondition( "ERROR", command, NULLOBJECT, context->WholeNumber( sRC ) ) ;
		}
	}
	else
	{
		log( "E", "Unknown service " + w1 <<endl) ;
		context->RaiseCondition( "FAILURE", command, NULLOBJECT, context->WholeNumber( -1 ) ) ;
		return NULLOBJECT;
	}

	setAllRexxVariables() ;
	return context->WholeNumber( sRC ) ;
}


int lspfBrowse( string s )
{
	int p1  ;
	int p2  ;

	bool   rlt ;

	string str  ;
	string pan  ;
	string fl   ;

	str = subword( s, 2 ) ;

	fl  = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( s ) ; return 20 ; }

	thisAppl->browse( fl, pan ) ;
	return thisAppl->RC ;
}


int lspfControl( string s )
{
	if ( words( s ) != 3 ) { lspfSyntaxError( s ) ; return 20 ; }

	thisAppl->control( word( s, 2 ), word( s, 3 ) ) ;
	return thisAppl->RC ;
}


int lspfDisplay( string s )
{
	int p1  ;
	int p2  ;

	bool   rlt ;

	string str  ;
	string pan  ;
	string msg  ;
	string cursor ;
	string csrpos ;

	str = subword( s, 2 ) ;

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	msg = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	cursor = parseString( rlt, str, "CURSOR()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	csrpos = parseString( rlt, str, "CSRPOS()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( s ) ; return 20 ; }

	if ( csrpos == "" ) { csrpos = "1" ; }

	thisAppl->display( pan, msg, cursor, ds2d( csrpos ) ) ;
	return thisAppl->RC ;
}


int lspfEdit( string s )
{
	int p1  ;
	int p2  ;

	bool   rlt ;

	string str  ;
	string pan  ;
	string fl   ;

	str = subword( s, 2 ) ;

	fl  = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( s ) ; return 20 ; }

	thisAppl->edit( fl, pan ) ;
	return thisAppl->RC ;
}


int lspfSelect( string s )
{
	int p1  ;
	int p2  ;
	int sRC ;

	bool rlt ;

	string str ;

	string SEL_PGM     ;
	string SEL_PARM    ;
	string SEL_NEWAPPL ;
	bool   SEL_NEWPOOL ;
	bool   SEL_PASSLIB ;

	str = subword( s, 2 ) ;

	selectParse( sRC, str, SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB ) ;
	if ( sRC > 0 )
	{
		lspfSyntaxError( s ) ;
		return 20 ;
	}
	if ( substr( SEL_PGM, 1, 1 ) == "&" )
	{
		thisAppl->vcopy( substr( SEL_PGM, 2 ), SEL_PGM, MOVE ) ;
	}

	thisAppl->select( SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB ) ;
	return thisAppl->RC ;
}


int lspfVPUT( string s )
{
	int p1  ;
	int p2  ;

	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = subword( s, 2 ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	if ( words( vars ) == 0 )
	{
		vars = word( s, 2 )    ;
		str  = subword( s, 3 ) ;
	}

	str = strip( str ) ;
	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else                         { lspfSyntaxError( s ) ; return 20 ; }

	thisAppl->vput( vars, pType ) ;
	return thisAppl->RC ;
}


int lspfVGET( string s )
{
	int p1  ;
	int p2  ;

	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = subword( s, 2 ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt ) { lspfSyntaxError( s ) ; return 20 ; }

	if ( words( vars ) == 0 )
	{
		vars = word( s, 2 )    ;
		str  = subword( s, 3 ) ;
	}

	str = strip( str ) ;
	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else                         { lspfSyntaxError( s ) ; return 20 ; }

	thisAppl->vget( vars, pType ) ;
	return thisAppl->RC ;
}


void lspfSyntaxError( string s )
{
	thisAppl->RC = 20 ;
	thisAppl->checkRCode( "Syntax error in service: "+s ) ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new POREXX1 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }

