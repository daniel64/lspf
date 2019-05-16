/* Compile with ::                                                                                       */
/* g++  -shared -fPIC -std=c++11 -Wl,-soname,libPEDRXM1.so -lrexx -lrexxapi -o libPEDRXM1.so PEDRXM1.cpp */
/*
  Copyright (c) 2017 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify
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

/* REXX-EDIT macro interface program                                                           */
/*                                                                                             */
/* Setup the macro interface block and the REXX ISPEXEC and ISREDIT call handlers.             */
/* Call ISREDIT procedure of the EDITOR.  MIB pointer is passed via the ApplUserData[] map     */
/* to this routine, and via the APPLICATION_DATA field for the rexx environment hanlders.      */
/*                                                                                             */
/* Search order:                                                                               */
/*     ZORXPATH                                                                                */
/*     REXX_PATH env variable                                                                  */
/*     PATH      env variable                                                                  */
/*                                                                                             */
/* Macro functions running in the EDITOR application, use the macro application function pool. */
/* Addressibility is via the miblock.macAppl field.                                            */
/*                                                                                             */
/* ZRC/ZRSN codes returned                                                                     */
/*   0/0  Okay                                                                                 */


#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "eHilight.cpp"
#include <oorexxapi.h>
#include "PEDIT01.h"
#include "PEDRXM1.h"


using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PEDRXM1

RexxObjectPtr RexxEntry lspfServiceHandler( RexxExitContext*, RexxStringObject, RexxStringObject ) ;
RexxObjectPtr RexxEntry editServiceHandler( RexxExitContext*, RexxStringObject, RexxStringObject ) ;

int getRexxVariable( pApplication*, string, string& ) ;
int setRexxVariable( string, string )    ;
int getAllRexxVariables( pApplication* ) ;
int setAllRexxVariables( pApplication* ) ;


PEDRXM1::PEDRXM1()
{
	set_appdesc( "OOREXX Edit macro interface program" ) ;
	set_appver( "1.0.0" ) ;
}


void PEDRXM1::application()
{
	int nlvl ;

	miblock  tmiBlock ;
	PEDIT01* editAppl ;

	void* vptr = ApplUserData[ ds2d( word( PARM, 1 ) ) ] ;
	mibptr     = static_cast<miblock*>( vptr ) ;
	editAppl   = static_cast<PEDIT01*>( mibptr->editAppl ) ;

	if ( mibptr->nestlvl == 255 )
	{
		mibptr->seterror( "PEDM012H", 24 ) ;
		macroError()            ;
		mibptr->setExitRC( 24 ) ;
		return ;
	}

	nlvl = mibptr->nestlvl + 1 ;
	if ( nlvl > 1 )
	{
		tmiBlock = *mibptr ;
		mibptr->clear() ;
		mibptr->setMacro( word( editAppl->pcmd.get_cmd(), 1 ) )  ;
		if ( !mibptr->getMacroFileName( mibptr->rxpath2 ) )
		{
			mibptr->RC > 8 ? tmiBlock.seterror( "PEDM012Q", 20 ) : tmiBlock.seterror( "PEDT015A", 20 ) ;
			*mibptr = tmiBlock ;
			macroError() ;
			editAppl->pcmd.clear() ;
			mibptr->setExitRC( 20 ) ;
			return ;
		}
	}

	mibptr->parms   = subword( editAppl->pcmd.get_cmd(), 2 ) ;
	mibptr->macAppl = this ;
	mibptr->nestlvl = nlvl ;
	editAppl->nestLevel = nlvl ;

	start_rexx() ;

	if ( !mibptr->macro )
	{
		vreplace( "ZSTR1", mibptr->emacro ) ;
		setmsg( "PEDM012I" ) ;
	}
	else if ( nlvl > 1 && mibptr->getExitRC() > 8 )
	{
		vreplace( "ZSTR1", mibptr->emacro ) ;
		if ( !mibptr->msgset() )
		{
			vreplace( "ZSTR2", d2ds( mibptr->getExitRC() ) ) ;
			mibptr->seterror( "PEDT014X", mibptr->getExitRC() ) ;
		}
		macroError() ;
		mibptr->setExitRC( 28 ) ;
	}

	for_each( editAppl->data.begin(), editAppl->data.end(),
		[ nlvl ](iline*& a)
		{
			a->clearLabel( nlvl ) ;
		} ) ;

	if ( nlvl > 1 )
	{
		tmiBlock.exitRC    = mibptr->exitRC    ;
		tmiBlock.processed = mibptr->processed ;
		if ( mibptr->fatal )
		{
			tmiBlock.fatal = mibptr->fatal ;
			tmiBlock.msgid = mibptr->msgid ;
			tmiBlock.setRC( mibptr->RC )   ;
			tmiBlock.RSN   = mibptr->RSN   ;
		}
		*mibptr = tmiBlock ;
	}
	editAppl->nestLevel = mibptr->nestlvl ;
	editAppl->pcmd.clear_msg() ;
}


void PEDRXM1::start_rexx()
{
	RexxInstance* instance   ;
	RexxThreadContext* threadContext ;
	RexxArrayObject args     ;
	RexxCondition condition  ;
	RexxDirectoryObject cond ;
	RexxObjectPtr result     ;

	RexxContextEnvironment environments[ 3 ] ;
	RexxOption             options[ 4 ]      ;

	environments[ 0 ].handler = lspfServiceHandler ;
	environments[ 0 ].name    = "ISPEXEC" ;
	environments[ 1 ].handler = editServiceHandler ;
	environments[ 1 ].name    = "ISREDIT" ;
	environments[ 2 ].handler = NULL ;
	environments[ 2 ].name    = ""   ;

	options[ 0 ].optionName = APPLICATION_DATA ;
	options[ 0 ].option     = (void*)mibptr    ;
	options[ 1 ].optionName = DIRECT_ENVIRONMENTS ;
	options[ 1 ].option     = (void*)environments ;
	options[ 2 ].optionName = EXTERNAL_CALL_PATH  ;
	options[ 2 ].option     = mibptr->rxpath1.c_str() ;
	options[ 3 ].optionName = "" ;

	rexxName = mibptr->mfile ;

	if ( RexxCreateInterpreter( &instance, &threadContext, options ) )
	{
		args   = threadContext->NewArray( 0 ) ;
		result = threadContext->CallProgram( rexxName.c_str(), args ) ;
		if ( threadContext->CheckCondition() )
		{
			cond = threadContext->GetConditionInfo() ;
			threadContext->DecodeConditionInfo( cond, &condition ) ;
			llog( "E", "POREXX1 error running REXX.: "<< mibptr->emacro << endl ) ;
			llog( "E", "   Condition Code . . . . .: "<< condition.code << endl ) ;
			llog( "E", "   Condition Error Text . .: "<< threadContext->CString( condition.errortext ) << endl ) ;
			llog( "E", "   Condition Message. . . .: "<< threadContext->CString( condition.message ) << endl ) ;
			llog( "E", "   Line Error Occured . . .: "<< condition.position << endl ) ;
			vreplace( "ZERR1", d2ds( condition.code ) ) ;
			vreplace( "ZERR2", threadContext->CString( condition.errortext ) ) ;
			vreplace( "ZERR3", threadContext->CString( condition.message ) ) ;
			vreplace( "ZERR4", d2ds( condition.position ) ) ;
			vreplace( "ZERR5", mibptr->emacro ) ;
			display( "REXERROR" ) ;
			mibptr->setRC( 20 )   ;
			mibptr->setExitRC( condition.code ) ;
		}
		else if ( mibptr->fatal )
		{
			mibptr->setExitRC( 28 ) ;
		}
		else if ( result != NULLOBJECT )
		{
			mibptr->setExitRC( threadContext->CString( result ) ) ;
		}
		else
		{
			mibptr->setExitRC( 0 ) ;
		}
		instance->Terminate() ;
	}
}


void PEDRXM1::macroError()
{
	// Issue the macro error screen.
	// Don't show error panel if RC=28 as it has already been shown.

	if ( mibptr->RC == 28 ) { return ; }

	vreplace( "STR", mibptr->keyword )   ;
	vreplace( "ZERR1", mibptr->sttment ) ;

	if ( mibptr->val1 != "" )
	{
		vreplace( "ZVAL1", mibptr->val1 ) ;
	}
	getmsg( mibptr->msgid, "ZERRSM", "ZERRLM", "ZERRALRM", "ZERRHM" ) ;
	vreplace( "ZERRMSG", mibptr->msgid ) ;

	vreplace( "ZERR2", d2ds( mibptr->RC ) ) ;
	vreplace( "ZERR3", mibptr->emacro ) ;

	display( "ISRERROR" ) ;

	mibptr->setRC( 28 )  ;
}


RexxObjectPtr RexxEntry lspfServiceHandler( RexxExitContext* context,
					    RexxStringObject address,
					    RexxStringObject command )
{
	// Called on address ISPEXEC REXX statement
	// Call lspf using the ispexec interface

	// On entry, update macro application function pool with the rexx variable pool values
	// On exit, update the rexx variable pool with the macro appl function pool values, and set value RC in REXX

	int sRC  ;

	void* vptr ;
	miblock* mibptr ;

	string s = context->CString( command ) ;

	vptr   = context->GetApplicationData() ;
	mibptr = static_cast<miblock*>( vptr ) ;
	PEDRXM1* macAppl = static_cast<PEDRXM1*>( mibptr->macAppl ) ;

	if ( mibptr->fatal )
	{
		return context->WholeNumber( 28 ) ;
	}

	getAllRexxVariables( macAppl ) ;

	macAppl->ispexec( s ) ;
	sRC = macAppl->RC     ;

	setAllRexxVariables( macAppl ) ;
	return context->WholeNumber( sRC ) ;
}


RexxObjectPtr RexxEntry editServiceHandler( RexxExitContext* context,
					    RexxStringObject address,
					    RexxStringObject command )
{
	// Called on address ISREDIT REXX statement
	// Substitute variables in the edit command and call the EDITOR ISREDIT interface

	// On entry, update macro application function pool with the rexx variable pool values
	// On exit, update the rexx variable pool with the macro appl function pool values, and set value RC in REXX

	int sRC ;

	void* vptr ;
	miblock* mibptr ;

	string s = context->CString( command ) ;

	vptr   = context->GetApplicationData() ;
	mibptr = static_cast<miblock*>( vptr ) ;
	PEDIT01* editAppl = static_cast<PEDIT01*>( mibptr->editAppl ) ;
	PEDRXM1* macAppl  = static_cast<PEDRXM1*>( mibptr->macAppl  ) ;

	if ( mibptr->fatal )
	{
		return context->WholeNumber( 28 ) ;
	}

	getAllRexxVariables( macAppl ) ;

	editAppl->isredit( macAppl->sub_vars( s ) ) ;

	if ( mibptr->fatal )
	{
		macAppl->macroError() ;
		sRC = mibptr->RC ;
	}
	else if ( mibptr->runmacro )
	{
		editAppl->pcmd.set_cmd( mibptr->sttment, editAppl->defNames ) ;
		if ( editAppl->pcmd.error() )
		{
			return context->WholeNumber( 20 ) ;
		}
		macAppl->select( "PGM(PEDRXM1) PARM( "+ d2ds( mibptr->etaskid ) +" )" ) ;
		sRC = mibptr->getExitRC() ;
	}
	else
	{
		sRC = mibptr->RC ;
	}

	setAllRexxVariables( macAppl ) ;

	return context->WholeNumber( sRC ) ;
}


int getAllRexxVariables( pApplication* macAppl )
{
	// For all variables in the rexx variable pool, set the lspf function pool variable.
	// Executed on entry to the command handler from a REXX procedure before any lspf
	// function is called

	// If the vdefine is for an INTEGER, convert value to a number and vreplace

	int rc ;

	string n ;
	string v ;

	set<string>& vl = macAppl->vilist() ;

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
		rc = RexxVariablePool( &var ) ;     /* get the variable             */
		if ( rc != RXSHV_OK) { break ; }
		n = string( var.shvname.strptr, var.shvname.strlength ) ;
		v = string( var.shvvalue.strptr, var.shvvalue.strlength ) ;
		if ( isvalidName( n ) )
		{
			if ( vl.count( n ) > 0 )
			{
				macAppl->vreplace( n, ds2d( v ) ) ;
			}
			else
			{
				macAppl->vreplace( n, v ) ;
			}
		}
		RexxFreeMemory( (void*)var.shvname.strptr )  ;
		RexxFreeMemory( (void*)var.shvvalue.strptr ) ;
		if ( var.shvret & RXSHV_LVAR ) { break ; }
	}
	return rc ;
}


int setAllRexxVariables( pApplication* macAppl )
{
	// For all variables in the application function pool, set the rexx variable.
	// Executed before returning to the REXX procedure after a call to the command handler
	// and after all lspf functions have been called

	// Note: vslist and vilist return the same set<string> reference

	int rc = 0 ;

	set<string>& vl = macAppl->vslist() ;

	string* val1 ;
	string  val2 ;

	for ( auto it = vl.begin() ; it != vl.end() ; ++it )
	{
		macAppl->vcopy( *it, val1, LOCATE ) ;
		rc = setRexxVariable( *it, *val1 )  ;
	}

	macAppl->vilist() ;
	for ( auto it = vl.begin() ; it != vl.end() ; ++it )
	{
		macAppl->vcopy( *it, val2, MOVE ) ;
		rc = setRexxVariable( *it, val2 ) ;
	}
	return rc ;
}


int getRexxVariable( pApplication* macAppl, string n, string & v )
{
	// Get variable value from Rexx variable pool and update the application function pool

	int rc ;

	const char* name = n.c_str() ;

	SHVBLOCK var ;                       /* variable pool control block   */
	var.shvcode = RXSHV_SYFET ;          /* do a symbolic fetch operation */
	var.shvret  = 0 ;                    /* clear return code field       */
	var.shvnext = NULL ;                 /* no next block                 */
	var.shvvalue.strptr    = NULL ;      /* let REXX allocate the memory  */
	var.shvvalue.strlength = 0 ;
	var.shvvaluelen        = 0 ;
					     /* set variable name string      */
	MAKERXSTRING( var.shvname, name, n.size() ) ;

	rc = RexxVariablePool( &var ) ;
	if ( rc == 0 )
	{
		v = string( var.shvvalue.strptr, var.shvvalue.strlength ) ;
		macAppl->vreplace( n, v ) ;
		rc = macAppl->RC          ;
	}
	RexxFreeMemory( (void*)var.shvvalue.strptr ) ;
	return rc ;
}


int setRexxVariable( string n, string v )
{
	const char* name = n.c_str() ;
	char* value      = (char*)v.c_str() ;

	SHVBLOCK var ;                       /* variable pool control block   */
	var.shvcode = RXSHV_SYSET  ;         /* do a symbolic set operation   */
	var.shvret  = 0 ;                    /* clear return code field       */
	var.shvnext = NULL ;                 /* no next block                 */
					     /* set variable name string      */
	MAKERXSTRING( var.shvname, name, n.size() ) ;
					     /* set value string              */
	MAKERXSTRING( var.shvvalue, value, v.size() ) ;
	var.shvvaluelen = v.size()      ;    /* set value length              */
	return RexxVariablePool( &var ) ;
}

// ============================================================================================ //

extern "C" { pApplication* maker() { return new PEDRXM1 ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
