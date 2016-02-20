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


RexxObjectPtr RexxEntry lspfCommandHandler( RexxExitContext *, RexxStringObject, RexxStringObject ) ;

int getRexxVariable( pApplication *, string, string & ) ;
int setRexxVariable( string, string ) ;
int getAllRexxVariables( pApplication * ) ;
int setAllRexxVariables( pApplication * ) ;

void lspfSyntaxError( pApplication *, string ) ;

int  lspfAddpop( pApplication *, string )   ;
int  lspfBrowse( pApplication *, string )   ;
int  lspfControl( pApplication *, string )  ;
int  lspfDisplay( pApplication *, string )  ;
int  lspfEdit( pApplication *, string )     ;
int  lspfGetmsg( pApplication *, string )   ;
int  lspfLibdef( pApplication *, string )   ;
int  lspfPquery( pApplication *, string )   ;
int  lspfRDisplay( pApplication *, string ) ;
int  lspfRempop( pApplication *, string )   ;
int  lspfSelect( pApplication *, string )   ;
int  lspfSetmsg( pApplication *, string )   ;
int  lspfTBAdd( pApplication *, string )    ;
int  lspfTBBottom( pApplication *, string ) ;
int  lspfTBCreate( pApplication *, string ) ;
int  lspfTBClose( pApplication *, string )  ;
int  lspfTBDelete( pApplication *, string ) ;
int  lspfTBDispl( pApplication *, string )  ;
int  lspfTBEnd( pApplication *, string )    ;
int  lspfTBErase( pApplication *, string )  ;
int  lspfTBExist( pApplication *, string )  ;
int  lspfTBGet( pApplication *, string )    ;
int  lspfTBMod( pApplication *, string )    ;
int  lspfTBPut( pApplication *, string )    ;
int  lspfTBOpen( pApplication *, string )   ;
int  lspfTBQuery( pApplication *, string )  ;
int  lspfTBSarg( pApplication *, string )  ;
int  lspfTBSave( pApplication *, string )   ;
int  lspfTBScan( pApplication *, string )   ;
int  lspfTBSkip( pApplication *, string )   ;
int  lspfTBSort( pApplication *, string )   ;
int  lspfTBTop( pApplication *, string )    ;
int  lspfTBVClear( pApplication *, string ) ;
int  lspfVerase( pApplication *, string )   ;
int  lspfVput( pApplication *, string )     ;
int  lspfVget( pApplication *, string )     ;


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

	environments[ 0 ].handler = lspfCommandHandler ;
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
			setmsg( "PSYS011B" ) ;
			ZRC     = 16 ;
			ZRSN    = 12 ;
			ZRESULT = "Invalid REXX passed" ;
			cleanup() ;
			return    ;
		}
		if ( !found )
		{
			log( "E", "POREXX1 error. " << rxsource << " not found in ZORXPATH concatination" << endl ) ;
			setmsg( "PSYS011C" ) ;
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
			}
		}
		instance->Terminate() ;
	}
	cleanup() ;
	return    ;
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

	string w1 ;
	string w2 ;

	void * vptr ;

	const string InvalidServices = "VDEFINE VDELETE VCOPY VREPLACE VRESET" ;

	string s1 = context->CString( address ) ;
	string s2 = context->CString( command ) ;

	w1 = upper( word( s2, 1 ) ) ;
	if ( findword( w1, InvalidServices ) )
	{
		log( "E", "Service " + w1 + " not valid from OOREXX"<<endl) ;
		context->RaiseCondition( "FAILURE", command, NULLOBJECT, context->WholeNumber( -1 ) ) ;
		return NULLOBJECT;
	}

	vptr = context->GetApplicationData() ;
	pApplication * thisAppl = static_cast<pApplication *>(vptr) ;

	getAllRexxVariables( thisAppl ) ;
	if      ( w1 == "ADDPOP" )   { sRC = lspfAddpop( thisAppl, s2 )   ; }
	else if ( w1 == "BROWSE" )   { sRC = lspfBrowse( thisAppl, s2 )   ; }
	else if ( w1 == "DISPLAY" )  { sRC = lspfDisplay( thisAppl, s2 )  ; }
	else if ( w1 == "CONTROL" )  { sRC = lspfControl( thisAppl, s2 )  ; }
	else if ( w1 == "EDIT" )     { sRC = lspfEdit( thisAppl, s2 )     ; }
	else if ( w1 == "GETMSG" )   { sRC = lspfGetmsg( thisAppl, s2 )   ; }
	else if ( w1 == "LIBDEF" )   { sRC = lspfLibdef( thisAppl, s2 )   ; }
	else if ( w1 == "PQUERY" )   { sRC = lspfPquery( thisAppl, s2 )   ; }
	else if ( w1 == "RDISPLAY" ) { sRC = lspfRDisplay( thisAppl, s2 ) ; }
	else if ( w1 == "REMPOP" )   { sRC = lspfRempop( thisAppl, s2 )   ; }
	else if ( w1 == "SELECT" )   { sRC = lspfSelect( thisAppl, s2 )   ; }
	else if ( w1 == "SETMSG" )   { sRC = lspfSetmsg( thisAppl, s2 )   ; }
	else if ( w1 == "TBADD" )    { sRC = lspfTBAdd( thisAppl, s2 )    ; }
	else if ( w1 == "TBBOTTOM" ) { sRC = lspfTBBottom( thisAppl, s2 ) ; }
	else if ( w1 == "TBCLOSE" )  { sRC = lspfTBClose( thisAppl, s2 )  ; }
	else if ( w1 == "TBCREATE" ) { sRC = lspfTBCreate( thisAppl, s2 ) ; }
	else if ( w1 == "TBDELETE" ) { sRC = lspfTBDelete( thisAppl, s2 ) ; }
	else if ( w1 == "TBDISPL" )  { sRC = lspfTBDispl( thisAppl, s2 )  ; }
	else if ( w1 == "TBEND" )    { sRC = lspfTBEnd( thisAppl, s2 )    ; }
	else if ( w1 == "TBERASE" )  { sRC = lspfTBErase( thisAppl, s2 )  ; }
	else if ( w1 == "TBEXIST" )  { sRC = lspfTBExist( thisAppl, s2 )  ; }
	else if ( w1 == "TBGET" )    { sRC = lspfTBGet( thisAppl, s2 )    ; }
	else if ( w1 == "TBMOD" )    { sRC = lspfTBMod( thisAppl, s2 )    ; }
	else if ( w1 == "TBPUT" )    { sRC = lspfTBPut( thisAppl, s2 )    ; }
	else if ( w1 == "TBOPEN" )   { sRC = lspfTBOpen( thisAppl, s2 )   ; }
	else if ( w1 == "TBQUERY" )  { sRC = lspfTBQuery( thisAppl, s2 )  ; }
	else if ( w1 == "TBSARG" )   { sRC = lspfTBSarg( thisAppl, s2 )   ; }
	else if ( w1 == "TBSAVE" )   { sRC = lspfTBSave( thisAppl, s2 )   ; }
	else if ( w1 == "TBSCAN" )   { sRC = lspfTBScan( thisAppl, s2 )   ; }
	else if ( w1 == "TBSKIP" )   { sRC = lspfTBSkip( thisAppl, s2 )   ; }
	else if ( w1 == "TBSORT" )   { sRC = lspfTBSort( thisAppl, s2 )   ; }
	else if ( w1 == "TBTOP" )    { sRC = lspfTBTop( thisAppl, s2 )    ; }
	else if ( w1 == "TBVCLEAR" ) { sRC = lspfTBVClear( thisAppl, s2 ) ; }
	else if ( w1 == "VERASE" )   { sRC = lspfVerase( thisAppl, s2 )   ; }
	else if ( w1 == "VGET" )     { sRC = lspfVget( thisAppl, s2 )     ; }
	else if ( w1 == "VPUT" )     { sRC = lspfVput( thisAppl, s2 )     ; }
	else
	{
		log( "E", "Unknown service " + w1 <<endl) ;
		context->RaiseCondition( "FAILURE", command, NULLOBJECT, context->WholeNumber( -1 ) ) ;
		return NULLOBJECT;
	}
	if ( sRC > 8 || sRC < 0 )
	{
		context->RaiseCondition( "ERROR", command, NULLOBJECT, context->WholeNumber( sRC ) ) ;
	}

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


int lspfAddpop( pApplication * thisAppl, string s )
{
	int    i_row ;
	int    i_col ;

	bool   rlt ;

	string str  ;
	string ap_loc  ;
	string ap_row  ;
	string ap_col  ;

	str = subword( s, 2 ) ;

	ap_loc = parseString( rlt, str, "POPLOC()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	ap_row = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( ap_row == "" ) { i_row = 0              ; }
	else                { i_row = ds2d( ap_row ) ; }

	ap_col = parseString( rlt, str, "COL()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( ap_col == "" ) { i_col = 0              ; }
	else                { i_col = ds2d( ap_col ) ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->addpop( ap_loc, i_row, i_col ) ;
	return thisAppl->RC ;
}


int lspfBrowse( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str  ;
	string pan  ;
	string fl   ;

	str = subword( s, 2 ) ;

	fl  = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->browse( fl, pan ) ;
	return thisAppl->RC ;
}


int lspfControl( pApplication * thisAppl, string s )
{
	if ( words( s ) != 3 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->control( word( s, 2 ), word( s, 3 ) ) ;
	return thisAppl->RC ;
}


int lspfDisplay( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str  ;
	string pan  ;
	string msg  ;
	string cursor ;
	string csrpos ;

	str = subword( s, 2 ) ;

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	msg = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	cursor = parseString( rlt, str, "CURSOR()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	csrpos = parseString( rlt, str, "CSRPOS()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( csrpos == "" ) { csrpos = "1" ; }

	thisAppl->display( pan, msg, cursor, ds2d( csrpos ) ) ;
	return thisAppl->RC ;
}


int lspfEdit( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str  ;
	string pan  ;
	string fl   ;

	str = subword( s, 2 ) ;

	fl  = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->edit( fl, pan ) ;
	return thisAppl->RC ;
}


int lspfGetmsg( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string msg  ;
	string smsg ;
	string lmsg ;
	string alm  ;
	string hlp  ;
	string typ  ;

	string str  ;

	str = subword( s, 2 ) ;

	msg  = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	smsg = parseString( rlt, str, "SHORTMSG()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	lmsg = parseString( rlt, str, "LONGMSG()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	alm  = parseString( rlt, str, "ALARM()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	hlp  = parseString( rlt, str, "HELP()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	typ = parseString( rlt, str, "TYPE()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->getmsg( msg, smsg, lmsg, alm, hlp, typ ) ;
	return thisAppl->RC ;
}


int lspfLibdef( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string ld_files ;
	string str      ;

	str      = subword( s, 3 ) ;
	ld_files = parseString( rlt, str, "ID()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->libdef( word( s, 2 ), ld_files ) ;
	return thisAppl->RC ;
}


int lspfPquery( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str      ;
	string pq_panel ;
	string pq_arean ;
	string pq_areat ;
	string pq_width ;
	string pq_depth ;
	string pq_row   ;
	string pq_col   ;

	str      = subword( s, 2 ) ;
	pq_panel = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_arean = parseString( rlt, str, "AREANAME()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_areat = parseString( rlt, str, "AREATYPE()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_width = parseString( rlt, str, "WIDTH()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_depth = parseString( rlt, str, "DEPTH()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_row   = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_col   = parseString( rlt, str, "COLUMN()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->pquery( pq_panel, pq_arean, pq_areat, pq_width, pq_depth, pq_row, pq_col ) ;
	return thisAppl->RC ;
}


int lspfRDisplay( pApplication * thisAppl, string s )
{
	thisAppl->rdisplay( substr( s, 10 ) ) ;
	return thisAppl->RC ;
}


int lspfRempop( pApplication * thisAppl, string s )
{
	string ap_all ;

	ap_all = subword( s, 2 ) ;

	if ( ap_all != "" && ap_all != "ALL" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->rempop( ap_all ) ;
	return thisAppl->RC ;
}


int lspfSelect( pApplication * thisAppl, string s )
{
	int sRC ;

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
		lspfSyntaxError( thisAppl, s ) ;
		return 20 ;
	}
	if ( SEL_PGM[ 0 ] == '&' )
	{
		thisAppl->vcopy( SEL_PGM.erase( 0, 1 ), SEL_PGM, MOVE ) ;
	}

	thisAppl->select( SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB ) ;
	return thisAppl->RC ;
}


int lspfSetmsg( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str   ;
	string s_msg ;

	msgSET t ;

	str = subword( s, 2 ) ;

	s_msg  = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	str = strip( str ) ;
	if      ( str == "COND" ) { t = COND   ; }
	else if ( str == "" )     { t = UNCOND ; }
	else    { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->setmsg( s_msg, t ) ;
	return thisAppl->RC ;
}


int lspfTBAdd( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str        ;
	string tb_name    ;
	string tb_namelst ;
	string tb_order   ;
	string tb_numrows ;

	tb_name  = word( s, 2 ) ;

	str        = subword( s, 3 ) ;
	tb_namelst = parseString( rlt, str, "SAVE()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_numrows = parseString( rlt, str, "MULT()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( tb_numrows == "" ) { tb_numrows = "1" ; }

	tb_order = strip( str ) ;
	if ( tb_order != "" && tb_order != "ORDER" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbadd( tb_name, tb_namelst, tb_order, ds2d( tb_numrows ) ) ;
	return thisAppl->RC ;
}


int lspfTBBottom( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;
	string tb_noread ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	tb_savenm  = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_crpnm   = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_noread = strip( str ) ;
	if ( tb_noread != "" && tb_noread != "NOREAD" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbbottom( tb_name, tb_savenm, tb_rowid, tb_noread, tb_crpnm ) ;
	return thisAppl->RC ;
}


int lspfTBClose( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str      ;
	string tb_name  ;
	string tb_nname ;
	string tb_path  ;

	tb_name  = word( s, 2 ) ;

	str      = subword( s, 3 ) ;
	tb_nname = parseString( rlt, str, "NAME()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_path  = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || words( str ) > 0 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbclose( tb_name, tb_nname, tb_path ) ;
	return thisAppl->RC ;
}


int lspfTBCreate( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str      ;
	string t        ;
	string tb_name  ;
	string tb_keys  ;
	string tb_names ;
	string tb_save  ;
	string tb_rep   ;
	string tb_paths ;
	string tb_disp  ;

	tbSAVE t_save ;
	tbDISP t_disp ;
	tbREP  t_rep  ;

	tb_name  = word( s, 2 ) ;

	str      = subword( s, 3 ) ;
	tb_keys  = parseString( rlt, str, "KEYS()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_names = parseString( rlt, str, "NAMES()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_paths = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	t_save = WRITE ;
	t = parseString( rlt, str, "WRITE" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t == "" )
	{
		t = parseString( rlt, str, "NOWRITE" ) ;
		if ( !rlt )    { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
		if ( t != "" ) { t_save = NOWRITE ; }
	}

	t_rep = NOREPLACE ;
	t = parseString( rlt, str, "REPLACE" ) ;
	if ( !rlt )    { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t != "" ) { t_rep = REPLACE ; }

	t_disp = EXCLUSIVE ;
	t = parseString( rlt, str, "SHARE" ) ;
	if ( !rlt )    { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t != "" ) { t_disp = SHARE ; }

	if ( words( str ) > 0 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbcreate( tb_name, tb_keys, tb_names, t_save, t_rep, tb_paths, t_disp ) ;
	return thisAppl->RC ;
}


int lspfTBDelete( pApplication * thisAppl, string s )
{
	if ( words( s ) != 2 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbdelete( word( s, 2 ) ) ;
	return thisAppl->RC ;
}


int lspfTBDispl( pApplication * thisAppl, string s )
{
	int i_csrpos ;
	int i_csrrow ;

	bool   rlt ;

	string str  ;
	string tb_name ;
	string tb_pan  ;
	string tb_msg  ;
	string tb_cursor  ;
	string tb_csrrow  ;
	string tb_csrpos  ;
	string tb_autosel ;
	string tb_posn    ;
	string tb_rowid   ;

	str = subword( s, 2 ) ;

	tb_pan     = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_msg     = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_cursor  = parseString( rlt, str, "CURSOR()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_csrrow  = parseString( rlt, str, "CSRROW()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( tb_csrrow == "" ) { i_csrrow = 1                ; }
	else                   { i_csrrow = ds2d( tb_csrrow ) ; }

	tb_csrpos  = parseString( rlt, str, "CSRPOS()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( tb_csrpos == "" ) { i_csrpos = 1                 ; }
	else                   { i_csrpos = ds2d( tb_csrpos ) ; }

	tb_autosel = parseString( rlt, str, "AUTOSEL()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_posn    = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbdispl( tb_name, tb_pan, tb_msg, tb_cursor, i_csrrow, i_csrpos, tb_autosel, tb_posn, tb_rowid ) ;
	return thisAppl->RC ;
}


int lspfTBEnd( pApplication * thisAppl, string s )
{
	if ( words( s ) != 2 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbend( word( s, 2 ) ) ;
	return thisAppl->RC ;
}


int lspfTBErase( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str     ;
	string tb_name ;
	string tb_path ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	tb_path = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tberase( tb_name, tb_path ) ;
	return thisAppl->RC ;
}


int lspfTBExist( pApplication * thisAppl, string s )
{
	if ( words( s ) != 2 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbexist( word( s, 2 ) ) ;
	return thisAppl->RC ;
}


int lspfTBGet( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;
	string tb_noread ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	tb_savenm  = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_crpnm   = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_noread = strip( str ) ;
	if ( tb_noread != "" && tb_noread != "NOREAD" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbget( tb_name, tb_savenm, tb_rowid, tb_noread, tb_crpnm ) ;
	return thisAppl->RC ;
}


int lspfTBMod( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_order  ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	tb_savenm  = parseString( rlt, str, "SAVE()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_order = strip( str ) ;
	if ( tb_order != "" && tb_order != "ORDER" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbmod( tb_name, tb_savenm, tb_order ) ;
	return thisAppl->RC ;
}


int lspfTBPut( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_order  ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	tb_savenm  = parseString( rlt, str, "SAVE()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_order = strip( str ) ;
	if ( tb_order != "" && tb_order != "ORDER" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbput( tb_name, tb_savenm, tb_order ) ;
	return thisAppl->RC ;
}


int lspfTBOpen( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str      ;
	string t        ;
	string tb_name  ;
	string tb_save  ;
	string tb_paths ;
	string tb_disp  ;

	tbSAVE t_save ;
	tbDISP t_disp ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	t_save = WRITE ;
	t = parseString( rlt, str, "WRITE" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t == "" )
	{
		t = parseString( rlt, str, "NOWRITE" ) ;
		if ( !rlt )    { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
		if ( t != "" ) { t_save = NOWRITE ; }
	}

	str      = subword( s, 4 ) ;
	tb_paths = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || words( str ) > 1 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_disp  = strip( str ) ;
	if      ( tb_disp == "SHARE" ) { t_disp = SHARE     ; }
	else if ( tb_disp == ""      ) { t_disp = EXCLUSIVE ; }
	else                           { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbopen( tb_name, t_save, tb_paths, t_disp ) ;
	return thisAppl->RC ;
}


int lspfTBQuery( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str        ;
	string tb_name    ;
	string tb_keys    ;
	string tb_names   ;
	string tb_rownum  ;
	string tb_keynum  ;
	string tb_namenum ;
	string tb_pos     ;
	string tb_srtflds ;
	string tb_sarglst ;
	string tb_sargcnd ;
	string tb_sargdir ;

	tb_name  = word( s, 2 )    ;
	str      = subword( s, 3 ) ;

	tb_keys  = parseString( rlt, str, "KEYS()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_names   = parseString( rlt, str, "NAMES()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rownum  = parseString( rlt, str, "ROWNUM()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_keynum  = parseString( rlt, str, "KEYNUM()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_namenum = parseString( rlt, str, "NAMENUM()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_pos     = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_srtflds = parseString( rlt, str, "SORTFLDS()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_sarglst = parseString( rlt, str, "SARGLIST()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_sargcnd = parseString( rlt, str, "SARGCOND()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_sargdir = parseString( rlt, str, "SARGDIR()" ) ;
	if ( !rlt || words( str ) > 0 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbquery( tb_name, tb_keys, tb_names, tb_rownum, tb_keynum, tb_namenum, tb_pos, tb_srtflds, tb_sarglst, tb_sargcnd, tb_sargdir ) ;
	return thisAppl->RC ;
}


int lspfTBSarg( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str      ;
	string tb_name  ;
	string tb_arglst  ;
	string tb_namecnd ;
	string tb_dir     ;

	tb_name  = word( s, 2 )    ;
	str      = subword( s, 3 ) ;

	tb_arglst  = parseString( rlt, str, "ARGLIST()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_namecnd = parseString( rlt, str, "NAMECOND()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_dir = strip( str ) ;
	if ( tb_dir == "" ) { tb_dir = "NEXT" ; }
	if ( tb_dir != "NEXT" && tb_dir != "PREVIOUS" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbsarg( tb_name, tb_arglst, tb_dir, tb_namecnd ) ;
	return thisAppl->RC ;
}


int lspfTBSave( pApplication * thisAppl, string s )
{
	bool   rlt ;

	string str      ;
	string tb_name  ;
	string tb_nname ;
	string tb_path  ;

	tb_name  = word( s, 2 )    ;
	str      = subword( s, 3 ) ;

	tb_nname = parseString( rlt, str, "NAME()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_path  = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || words( str ) > 0 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbsave( tb_name, tb_nname, tb_path ) ;
	return thisAppl->RC ;
}


int lspfTBScan( pApplication * thisAppl, string s )
{
	int    i_num ;

	bool   rlt   ;

	string str ;
	string t   ;
	string tb_name    ;
	string tb_arglst  ;
	string tb_savenm  ;
	string tb_rowid   ;
	string tb_crpnm   ;
	string tb_condlst ;
	string tb_dir     ;
	string tb_noread  ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	tb_arglst = parseString( rlt, str, "ARGLIST()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_savenm  = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_crpnm   = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_condlst = parseString( rlt, str, "CONDLIST()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_dir = "NEXT" ;
	t = parseString( rlt, str, "NEXT" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t == "" )
	{
		t = parseString( rlt, str, "PREVIOUS" ) ;
		if ( !rlt )    { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
		if ( t != "" ) { tb_dir = "PREVIOUS" ; }
	}

	tb_noread = strip( str ) ;
	if ( tb_noread != "" && tb_noread != "NOREAD" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbscan( tb_name, tb_arglst, tb_savenm, tb_rowid, tb_dir, tb_noread, tb_crpnm, tb_condlst ) ;
	return thisAppl->RC ;
}


int lspfTBSkip( pApplication * thisAppl, string s )
{
	int    i_num ;

	bool   rlt   ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_num    ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;
	string tb_row    ;
	string tb_noread ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	tb_num  = parseString( rlt, str, "NUMBER()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( tb_num == "" ) { i_num = 1                 ; }
	else                { i_num = ds2d( tb_num ) ; }

	tb_savenm  = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_row     = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_crpnm   = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_noread = strip( str ) ;
	if ( tb_noread != "" && tb_noread != "NOREAD" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbskip( tb_name, i_num, tb_savenm, tb_rowid, tb_row, tb_noread, tb_crpnm ) ;
	return thisAppl->RC ;
}


int lspfTBSort( pApplication * thisAppl, string s )
{
	int    i_num ;

	bool   rlt   ;

	string str       ;
	string tb_name   ;
	string tb_flds   ;

	tb_name = word( s, 2 )    ;
	str     = subword( s, 3 ) ;

	tb_flds = parseString( rlt, str, "FIELDS()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( strip( str ) != "" ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbsort( tb_name, tb_flds ) ;
	return thisAppl->RC ;
}


int lspfTBTop( pApplication * thisAppl, string s )
{
	if ( words( s ) != 2 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbtop( word( s, 2 ) ) ;
	return thisAppl->RC ;
}


int lspfTBVClear( pApplication * thisAppl, string s )
{
	if ( words( s ) != 2 ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbvclear( word( s, 2 ) ) ;
	return thisAppl->RC ;
}


int lspfVerase( pApplication * thisAppl, string s )
{
	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = subword( s, 2 ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

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
	else if ( str == "BOTH"    ) { pType = BOTH    ; }
	else                         { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->verase( vars, pType ) ;
	return thisAppl->RC ;
}


int lspfVput( pApplication * thisAppl, string s )
{
	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = subword( s, 2 ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

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
	else                         { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->vput( vars, pType ) ;
	return thisAppl->RC ;
}


int lspfVget( pApplication * thisAppl, string s )
{
	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = subword( s, 2 ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt ) { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

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
	else                         { lspfSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->vget( vars, pType ) ;
	return thisAppl->RC ;
}


void lspfSyntaxError( pApplication * thisAppl, string s )
{
	thisAppl->RC = 20 ;
	thisAppl->checkRCode( "Syntax error in service: "+s ) ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new POREXX1 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }

