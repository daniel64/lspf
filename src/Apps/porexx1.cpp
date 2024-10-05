/* Compile with ::                                                                                      */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libporexx1.so -o libporexx1.so -lrexx -lrexxapi porexx1.cpp */

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

/********************************************************************************************************/
/*                                                                                                      */
/* OOREXX-lspf interface module                                                                         */
/*                                                                                                      */
/* Setup command handler and call REXX interpreter                                                      */
/* (command handler lspfServiceHandler used for address ISPEXEC rexx statements for lspf services)      */
/*                                                                                                      */
/* Setup exit RXSIO to echo say and trace output to the screen via the rdisplay() method and            */
/* get data from standard input with REXX pull using the pull() method.                                 */
/*                                                                                                      */
/* PARM word 1 is the rexx to invloke                                                                   */
/* PARM words 2 onwards are the parameters for the rexx (Arg(1))                                        */
/*                                                                                                      */
/* Search order:                                                                                        */
/*     Directory of REXX being run                                                                      */
/*     ALTLIB-determined paths                                                                          */
/*     REXX_PATH env variable                                                                           */
/*     PATH      env variable                                                                           */
/*                                                                                                      */
/* Note that OOREXX will add extensions .rex and .REX when searching for the rexx to execute even       */
/* when a fully qualified name has been passed, and this will have precedence over the file name        */
/* without the extension ie. Search order is repeated without the extension.                            */
/*                                                                                                      */
/* Use system variable ZOREXPGM to refer to this module rather than the name, to allow different        */
/* rexx implementations to co-exist.                                                                    */
/*                                                                                                      */
/* Default address environment is ISPEXEC.                                                              */
/*                                                                                                      */
/*  RC/RSN codes returned                                                                               */
/*   0/0  Okay and ZRESULT set to result if not numeric                                                 */
/*  16/4  Missing rexx name                                                                             */
/*  20/condition.code ZRESULT is set to condition.message                                               */
/*  result/0 if no condition set and result is numeric                                                  */
/*                                                                                                      */
/* Take class-scope mutex lock around create interpreter and terminate() as these seem to hang when     */
/* they run together in background tasks (first terminate() hangs then everything attempting to         */
/* create a new interpreter instance)                                                                   */
/*                                                                                                      */
/********************************************************************************************************/

#include <boost/filesystem.hpp>

#include "../lspfall.h"

#include "../pTSOenv.h"
#include "../pTSOenv.cpp"

#include "porexx1.h"

#include <csignal>
#include <oorexxapi.h>


using namespace std ;
using namespace boost::filesystem ;

RexxObjectPtr RexxEntry lspfServiceHandler( RexxExitContext*,
					    RexxStringObject,
					    RexxStringObject ) ;

RexxObjectPtr RexxEntry TSOServiceHandler( RexxExitContext*,
					   RexxStringObject,
					   RexxStringObject ) ;

int REXXENTRY rxsioExit( RexxExitContext*,
			 int,
			 int,
			 PEXIT ) ;

int getRexxVariable( pApplication*,
		     string,
		     string& ) ;

int setRexxVariable( string,
		     string ) ;

void issueLineMessage( pApplication*,
		       int ) ;

int getAllRexxVariables( pApplication* ) ;
int setAllRexxVariables( pApplication* ) ;

boost::mutex porexx1::mtx ;


/* ************************************ ***************************** ********************************* */
/* ************************************     REXX package routines     ********************************* */
/* ************************************ ***************************** ********************************* */

/* **************************************************************************************************** */
/* Currently implemented:                                                                               */
/* DROP                                                                                                 */
/* SYSVAR                                                                                               */
/* LISTDSI                                                                                              */
/* OUTTRAP                                                                                              */
/* **************************************************************************************************** */

RexxRoutine1( int, drop, RexxStringObject, vars )
{
	//
	// Implement drop function.
	// Routine just calls drop() method in class TSOENV.
	//
	//   RETURN:
	//    0 Normal completion.
	//    8 Not a valid lspf variable name.  Not dropped.
	//   20 Severe error.

	void* vptr = context->GetApplicationData() ;
	porexx1* thisAppl = static_cast<porexx1*>(vptr) ;

	return thisAppl->drop( context, upper( context->CString( vars ) ) ) ;
}


RexxRoutine1( RexxStringObject, sysvar, RexxStringObject, type )
{
	//
	// Implement sysvar function.
	// Routine just calls sysvar() method in class TSOENV.
	//

	void* vptr = context->GetApplicationData() ;
	TSOENV* thisAppl  = static_cast<TSOENV*>(vptr) ;

	string var = upper( context->CString( type ) ) ;

	if ( !findword( var, "SYSENV SYSUID SYSNEST SYSLTERM SYSWTERM SYSNODE SYSISPF" ) )
	{
		context->RaiseException1( Rexx_Error_Invalid_argument_user_defined,
					  context->NewStringFromAsciiz( "SYSVAR variable not supported" ) ) ;
		return context->String( "" ) ;
	}

	return context->String( thisAppl->sysvar( var ).c_str() ) ;
}


RexxRoutine1( RexxStringObject, outtrap, ARGLIST, arg )
{
	//
	// Implement outtrap function.
	// Routine just calls outtrap() method in class TSOENV.
	//

	void* vptr = context->GetApplicationData() ;

	porexx1* thisAppl = static_cast<porexx1*>(vptr) ;

	RexxObjectPtr obj1 = context->ArrayAt( arg, 1 ) ;
	RexxObjectPtr obj2 = context->ArrayAt( arg, 2 ) ;

	size_t sz = context->ArraySize( arg ) ;

	string x1 ;
	string x2 = "-1" ;

	if ( obj1 == NULLOBJECT )
	{
		context->RaiseException2( Rexx_Error_Incorrect_call_minarg,
					  context->NewStringFromAsciiz( "outtrap" ),
					  context->NewStringFromAsciiz( "1" ) ) ;
		return context->String( "" ) ;
	}
	else
	{
		x1 = context->CString( obj1 ) ;
	}

	if ( obj2 != NULLOBJECT )
	{
		x2 = context->CString( obj2 ) ;
		if ( !datatype( x2, 'W' ) )
		{
			context->RaiseException1( Rexx_Error_Invalid_argument_user_defined,
						  context->NewStringFromAsciiz( "Argument 2 not numeric" ) ) ;
			return context->String( "" ) ;
		}
	}

	if ( sz > 2 )
	{
		context->RaiseException2( Rexx_Error_Incorrect_call_maxarg,
					  context->NewStringFromAsciiz( "outtrap" ),
					  context->NewStringFromAsciiz( "2" ) ) ;
		return context->String( "" ) ;
	}

	return context->String( thisAppl->outtrap( x1, ds2d( x2 ) ).c_str() ) ;
}


RexxRoutine2( int, listdsi, RexxStringObject, dsn, OPTIONAL_RexxStringObject, type )
{
	//
	// Implement listdsi function.
	// Routine just calls listdsi() method in class TSOENV.
	//

	void* vptr = context->GetApplicationData() ;
	porexx1* thisAppl = static_cast<porexx1*>(vptr) ;

	string type_str ;

	if ( type != NULLOBJECT )
	{
		type_str = upper( context->CString( type ) ) ;
		if ( type_str != "FILE" && type_str != "" )
		{
			context->RaiseException1( Rexx_Error_Invalid_argument_user_defined,
						  context->NewStringFromAsciiz( "If entered, second argument must be 'FILE'" ) ) ;
			return 20 ;
		}
	}

	return thisAppl->listdsi( context, context->CString( dsn ), type_str ) ;
}


RexxRoutineEntry lspf_routines[] =
{
	REXX_TYPED_ROUTINE( drop, drop ),
	REXX_TYPED_ROUTINE( sysvar, sysvar ),
	REXX_TYPED_ROUTINE( outtrap, outtrap ),
	REXX_TYPED_ROUTINE( listdsi, listdsi ),
	REXX_LAST_ROUTINE()
} ;


RexxPackageEntry lspf_package_entry =
{
	STANDARD_PACKAGE_HEADER
	REXX_INTERPRETER_5_0_0,
	"lspfPackage",
	"1.0.0",
	nullptr,
	nullptr,
	lspf_routines,
	nullptr
} ;


OOREXX_GET_PACKAGE( lspf ) ;

LSPF_APP_MAKER( porexx1 )


/* ************************************ ***************************** ********************************* */
/* ************************************   REXX/lspf inteface module   ********************************* */
/* ************************************ ***************************** ********************************* */

porexx1::porexx1()
{
	STANDARD_HEADER( "OOREXX-lspf interface module", "1.0.1" )
}


void porexx1::application()
{
	//
	// Execute as a REXX procedure if not a builtin command.
	//

	string rxpath ;
	string val1 ;
	string msg ;
	string w1 ;
	string w2 ;

	bool call_uabend = false ;

	w1 = word( PARM, 1 ) ;

	if ( w1 == "-" )
	{
		while ( true )
		{
			say( "READY" ) ;
			pull( &PARM ) ;
			w2 = strip( upper( PARM ) ) ;
			if ( w2 == "" || findword( w2, "END QUIT BYE EXIT" ) )
			{
				return ;
			}
			if ( !builtin( PARM ) )
			{
				break ;
			}
			else if ( RC == 0 )
			{
				return ;
			}
		}
	}
	else if ( w1 == "*" )
	{
		while ( true )
		{
			say( "READY" ) ;
			pull( &PARM ) ;
			w2 = strip( upper( PARM ) ) ;
			if ( w2 == "" )
			{
				continue ;
			}
			if ( findword( w2, "END QUIT BYE EXIT" ) )
			{
				return ;
			}
			if ( !builtin( PARM ) )
			{
				break ;
			}
		}
	}
	else if ( builtin( PARM ) )
	{
		ZRC = RC ;
		return ;
	}

	rxpath   = sysexec() ;
	rexxName = word( PARM, 1 ) ;

	if ( upper( rexxName ) == "EX" || upper( rexxName ) == "EXEC" )
	{
		rexxName = word( PARM, 2 ) ;
		PARM     = subword( PARM, 3 ) ;
	}
	else
	{
		PARM = subword( PARM, 2 ) ;
	}

	if ( rexxName.size() > 0 && rexxName.front() == '%' ) { rexxName.erase( 0, 1 ) ; }

	control( "SIGTERM", "IGNORE" ) ;

	if ( rexxName == "" )
	{
		llog( "E", "POREXX1 error. No REXX passed" << endl ) ;
		ZRC     = 16 ;
		ZRSN    = 4  ;
		ZRESULT = "No REXX passed" ;
		setmsg( "PSYS012Y" ) ;
		return ;
	}

	RexxInstance* instance   ;
	RexxThreadContext* threadContext ;
	RexxArrayObject args     ;
	RexxCondition condition  ;
	RexxDirectoryObject cond ;
	RexxObjectPtr result     ;
	RexxLibraryPackage package ;

	RexxOption             options[ 7 ] ;
	RexxContextEnvironment environments[ 3 ] ;
	RexxContextExit        exits[ 2 ] ;

	options[ 0 ].optionName = INITIAL_ADDRESS_ENVIRONMENT ;
	options[ 0 ].option     = "ISPEXEC" ;
	options[ 1 ].optionName = APPLICATION_DATA ;
	options[ 1 ].option     = (void*)this ;
	options[ 2 ].optionName = DIRECT_ENVIRONMENTS ;
	options[ 2 ].option     = (void*)environments ;
	options[ 3 ].optionName = EXTERNAL_CALL_PATH  ;
	options[ 3 ].option     = rxpath.c_str() ;
	options[ 4 ].optionName = DIRECT_EXITS ;
	options[ 4 ].option     = (void*)exits ;
	options[ 5 ].optionName = REGISTER_LIBRARY ;
	options[ 5 ].option     = (void*)&package ;
	options[ 6 ].optionName = "" ;
	options[ 6 ].option     = (void*)nullptr ;

	exits[ 0 ].sysexit_code = RXSIO ;
	exits[ 0 ].handler      = rxsioExit ;
	exits[ 1 ].sysexit_code = RXENDLST ;
	exits[ 1 ].handler      = nullptr ;

	package.registeredName = "lspfPackage" ;
	package.table          = &lspf_package_entry ;

	environments[ 0 ].name    = "ISPEXEC" ;
	environments[ 0 ].handler = lspfServiceHandler ;
	environments[ 1 ].name    = "TSO" ;
	environments[ 1 ].handler = TSOServiceHandler ;
	environments[ 2 ].name    = ""   ;
	environments[ 2 ].handler = nullptr ;

	lock() ;
	if ( !RexxCreateInterpreter( &instance, &threadContext, options ) )
	{
		unlock() ;
		showErrorScreen( "PSYE034T" ) ;
		return ;
	}

	unlock() ;

	args = threadContext->NewArray( 1 ) ;
	threadContext->ArrayPut( args, threadContext->String( PARM.c_str() ), 1 ) ;

	result = threadContext->CallProgram( rexxName.c_str(), args ) ;

	if ( threadContext->CheckCondition() )
	{
		cond = threadContext->GetConditionInfo() ;
		threadContext->DecodeConditionInfo( cond, &condition ) ;
		llog( "E", "POREXX1 error running REXX.: " << rexxName << endl ) ;
		llog( "E", "   Parameters . . . . . . .: " << PARM << endl ) ;
		llog( "E", "   Condition Code . . . . .: " << condition.code << endl ) ;
		llog( "E", "   Condition Error Text . .: " << threadContext->CString( condition.errortext ) << endl ) ;
		llog( "E", "   Condition Message. . . .: " << threadContext->CString( condition.message ) << endl ) ;
		llog( "E", "   Line Error Occured . . .: " << condition.position << endl ) ;
		ZRC = 20 ;
		if ( condition.code == Rexx_Error_Program_unreadable_notfound )
		{
			msg = "COMMAND "+ rexxName + " NOT FOUND" ;
			say( msg ) ;
			if ( background() )
			{
				notify( msg ) ;
			}
			ZRSN    = 997 ;
			ZRESULT = "Not Found" ;
		}
		else
		{
			ZRSN  = condition.code ;
			val1  = "Error on line: "+ d2ds( condition.position ) ;
			val1 += ".  Condition code: " ;
			val1 += d2ds( ZRSN ) ;
			val1 += ".  Error Text: " ;
			val1 += threadContext->CString( condition.errortext ) ;
			val1 += ".  Message: " ;
			val1 += threadContext->CString( condition.message ) ;
			if ( val1.size() > 512 ) { val1.erase( 512 ) ; }
			showErrorScreen( "PSYS012Z", val1 ) ;
			ZRESULT = "Abended" ;
			call_uabend = true ;
		}
	}
	else if ( result != NULLOBJECT )
	{
		ZRESULT = threadContext->CString( result ) ;
		if ( datatype( ZRESULT, 'W' ) )
		{
			ZRC     = ds2d( ZRESULT ) ;
			ZRESULT = "" ;
		}
	}
	instance->Terminate() ;

	if ( call_uabend ) { uabend() ; }

	if ( w1 == "*" || ( ( w1 == "-" ) && ZRESULT == "Not Found" ) )
	{
		PARM = w1 ;
		application() ;
	}
}


int REXXENTRY rxsioExit( RexxExitContext* context,
			 int exitnumber,
			 int subfunction,
			 PEXIT ParmBlock )

{
	//
	// REXX exit RXSIO.  Handle subfunction RXSIOSAY and RXSIOTRC to call rdisplay() method for say/trace output.
	//                   Handle subfunction RXSIOTRD to call pull() method to get user data for pull/arg pull requests.
	//                   Handle subfunction RXSIODTR to call pull() method for interactve debug input.
	//

	void* vptr ;

	switch ( subfunction )
	{
	case RXSIOSAY:
	case RXSIOTRC:
	{
		vptr = context->GetApplicationData() ;
		pApplication* thisAppl  = static_cast<pApplication*>(vptr) ;
		RXSIOSAY_PARM* say_parm = (RXSIOSAY_PARM *)ParmBlock ;
		if ( say_parm->rxsio_string.strptr )
		{
			thisAppl->rdisplay( string( say_parm->rxsio_string.strptr, say_parm->rxsio_string.strlength ), false ) ;
		}
		return RXEXIT_HANDLED ;
	}

	case RXSIOTRD:
	{
		string ans ;
		vptr = context->GetApplicationData() ;
		pApplication* thisAppl  = static_cast<pApplication*>(vptr) ;
		RXSIOTRD_PARM* trd_parm = (RXSIOTRD_PARM *)ParmBlock ;
		thisAppl->pull( &ans ) ;
		if ( ans.size() >= trd_parm->rxsiotrd_retc.strlength )
		{
			trd_parm->rxsiotrd_retc.strptr = (char *)RexxAllocateMemory( ans.size() + 1 ) ;
		}
		strcpy( trd_parm->rxsiotrd_retc.strptr, ans.c_str() ) ;
		trd_parm->rxsiotrd_retc.strlength = ans.size() ;
		return RXEXIT_HANDLED ;
	}

	case RXSIODTR:
	{
		string ans ;
		vptr = context->GetApplicationData() ;
		pApplication* thisAppl  = static_cast<pApplication*>(vptr) ;
		RXSIODTR_PARM* dtr_parm = (RXSIODTR_PARM *)ParmBlock ;
		thisAppl->pull( &ans ) ;
		if ( ans.size() >= dtr_parm->rxsiodtr_retc.strlength )
		{
			dtr_parm->rxsiodtr_retc.strptr = (char *)RexxAllocateMemory( ans.size() + 1 ) ;
		}
		strcpy( dtr_parm->rxsiodtr_retc.strptr, ans.c_str() ) ;
		dtr_parm->rxsiodtr_retc.strlength = ans.size() ;
		return RXEXIT_HANDLED ;
	}
	}

	return RXEXIT_NOT_HANDLED ;
}


void porexx1::showErrorScreen( const string& msg,
			       string val1 )
{
	//
	// Show error screen PSYSER3 for message 'msg' and ZVAL1 val1.
	//

	size_t i ;

	int l    ;
	int maxw ;

	string* t ;

	if ( background() ) { return ; }

	vdefine( "ZSCRMAXW", &maxw ) ;
	vdefine( "ZVAL1", &val1 ) ;
	vreplace( "ZERRMSG", msg ) ;
	vreplace( "ZERRDSC", "" ) ;
	vreplace( "ZERRSRC", "" ) ;
	vreplace( "ZERRRX", rexxName ) ;

	vget( "ZSCRMAXW", SHARED ) ;
	getmsg( msg, "ZERRSM", "ZERRLM" ) ;

	vcopy( "ZERRLM", t, LOCATE ) ;

	maxw = maxw - 6 ;
	l    = 0 ;
	do
	{
		++l ;
		if ( t->size() > size_t( maxw ) )
		{
			i = t->find_last_of( ' ', maxw ) ;
			i = ( i == string::npos ) ? maxw : i + 1 ;
			vreplace( "ZERRLM"+ d2ds( l ), t->substr( 0, i ) ) ;
			t->erase( 0, i ) ;
		}
		else
		{
			vreplace( "ZERRLM"+d2ds( l ), *t ) ;
			*t = "" ;
		}
	} while ( t->size() > 0 ) ;

	display( "PSYSER3" ) ;
}


RexxObjectPtr RexxEntry lspfServiceHandler( RexxExitContext* context,
					    RexxStringObject address,
					    RexxStringObject command )
{
	//
	// Called on address ISPEXEC REXX statement.
	// Call lspf using the ispexec interface.
	//
	// On entry, update lspf function pool with the rexx variable pool values.
	// On exit, update the rexx variable pool with the lspf function pool values.
	//

	int sRC   ;

	void* vptr ;

	string s = context->CString( command ) ;

	vptr = context->GetApplicationData() ;
	pApplication* thisAppl = static_cast<pApplication*>(vptr) ;

	getAllRexxVariables( thisAppl ) ;

	thisAppl->ispexec( s ) ;
	sRC = thisAppl->RC ;

	setAllRexxVariables( thisAppl ) ;
	return context->WholeNumber( sRC ) ;
}


RexxObjectPtr RexxEntry TSOServiceHandler( RexxExitContext* context,
					   RexxStringObject address,
					   RexxStringObject command )
{
	//
	// Called on address TSO REXX statement.
	// Implement a few simple TSO functions to make it easier to run mainframe REXX procedures under lspf.
	//
	// These are all implemented in the TSOENV class:
	//    ALTLIB
	//    DROP
	//    EXECIO
	//    LISTDSI
	//    OUTTRAP
	//    SYSVAR
	//
	// as well as builtin commands defined in gls.
	//

	TSOENV* thisAppl = static_cast<TSOENV*>( context->GetApplicationData() ) ;

	return context->WholeNumber( thisAppl->TSOEntry( context, command ) ) ;
}


int getAllRexxVariables( pApplication* thisAppl )
{
	//
	// For all variables in the rexx variable pool, set the lspf function pool variable.
	// Executed on entry to the command handler from a REXX procedure before any lspf
	// function is called.
	//
	// If the vdefine is for an INTEGER, convert value to a number and vreplace.
	//

	int rc ;

	string n ;
	string v ;

	set<string>& vl = thisAppl->vilist() ;

	SHVBLOCK var ;

	while ( true )
	{
		var.shvnext            = nullptr ;
		var.shvname.strptr     = nullptr ;  /* let REXX allocate the memory */
		var.shvname.strlength  = 0 ;
		var.shvnamelen         = 0 ;
		var.shvvalue.strptr    = nullptr ;  /* let REXX allocate the memory */
		var.shvvalue.strlength = 0 ;
		var.shvvaluelen        = 0 ;
		var.shvcode            = RXSHV_NEXTV ;
		var.shvret             = 0 ;
		rc = RexxVariablePool( &var ) ;     /* get the variable             */
		if ( rc != RXSHV_OK) { break ; }
		n = string( var.shvname.strptr, var.shvname.strlength ) ;
		v = string( var.shvvalue.strptr, var.shvvalue.strlength ) ;
		if ( isvalidName( n ) )
		{
			if ( vl.count( n ) > 0 )
			{
				thisAppl->vreplace( n, ds2d( v ) ) ;
			}
			else
			{
				thisAppl->vreplace( n, v ) ;
			}
		}
		RexxFreeMemory( (void*)var.shvname.strptr ) ;
		RexxFreeMemory( var.shvvalue.strptr ) ;
		if ( var.shvret & RXSHV_LVAR ) { break ; }
	}

	return rc ;
}


int setAllRexxVariables( pApplication* thisAppl )
{
	//
	// For all variables in the application function pool, set the rexx variable.
	// Executed before returning to the REXX procedure after a call to the command handler
	// and after all lspf functions have been called
	//
	// Note: vslist and vilist return the same variable reference.
	//

	int rc = 0 ;

	set<string>& vl = thisAppl->vslist() ;

	string* val1 ;
	string  val2 ;

	for ( auto it = vl.begin() ; it != vl.end() ; ++it )
	{
		thisAppl->vcopy( *it, val1, LOCATE ) ;
		rc = setRexxVariable( *it, *val1 )  ;
	}

	thisAppl->vilist() ;
	for ( auto it = vl.begin() ; it != vl.end() ; ++it )
	{
		thisAppl->vcopy( *it, val2, MOVE ) ;
		rc = setRexxVariable( *it, val2 ) ;
	}

	return rc ;
}


int getRexxVariable( pApplication* thisAppl,
		     string n,
		     string& v )
{
	//
	// Get variable value from Rexx variable pool and update the application function pool.
	//

	int rc ;

	const char* name = n.c_str() ;

	SHVBLOCK var ;                       /* variable pool control block   */

	var.shvcode = RXSHV_SYFET ;          /* do a symbolic fetch operation */
	var.shvret  = 0 ;                    /* clear return code field       */
	var.shvnext = nullptr ;              /* no next block                 */
	var.shvvalue.strptr    = nullptr ;   /* let REXX allocate the memory  */
	var.shvvalue.strlength = 0 ;
	var.shvvaluelen        = 0 ;
					     /* set variable name string      */
	MAKERXSTRING( var.shvname, name, n.size() ) ;

	rc = RexxVariablePool( &var ) ;
	if ( rc == 0 )
	{
		v = string( var.shvvalue.strptr, var.shvvalue.strlength ) ;
		thisAppl->vreplace( n, v ) ;
		rc = thisAppl->RC ;
	}
	RexxFreeMemory( var.shvvalue.strptr ) ;

	return rc ;
}


int setRexxVariable( string n,
		     string v )
{
	const char* name = n.c_str() ;
	char* value      = (char*)v.c_str() ;

	SHVBLOCK var ;                       /* variable pool control block   */

	var.shvcode = RXSHV_SYSET ;          /* do a symbolic set operation   */
	var.shvret  = 0 ;                    /* clear return code field       */
	var.shvnext = nullptr ;              /* no next block                 */
					     /* set variable name string      */
	MAKERXSTRING( var.shvname, name, n.size() ) ;
					     /* set value string              */
	MAKERXSTRING( var.shvvalue, value, v.size() ) ;
	var.shvvaluelen = v.size()      ;    /* set value length              */

	return RexxVariablePool( &var ) ;
}
