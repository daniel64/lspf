/* Compile with ::                                                                                      */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpedrxm1.so -o libpedrxm1.so -lrexx -lrexxapi pedrxm1.cpp */
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

/***********************************************************************************************/
/*                                                                                             */
/* REXX-EDIT macro interface program                                                           */
/*                                                                                             */
/* Setup the macro interface block and the REXX ISPEXEC and ISREDIT call handlers.             */
/* Call ISREDIT procedure of the EDITOR.  MIB pointer is passed via application userdata       */
/* to this routine, and via the APPLICATION_DATA field for the rexx environment handlers.      */
/*                                                                                             */
/* Setup exit RXSIO to echo say and trace output to the screen via the rdisplay() method and   */
/* get data from standard input with REXX pull using the pull() method.                        */
/*                                                                                             */
/* Search order:                                                                               */
/*     ALTLIB-set path                                                                        */
/*     REXX_PATH env variable                                                                  */
/*     PATH      env variable                                                                  */
/*                                                                                             */
/* Default address environment is ISPEXEC.                                                     */
/*                                                                                             */
/* Macro functions running in the EDITOR application, use the macro application function pool. */
/* Addressibility is via the miblock.macAppl field.                                            */
/*                                                                                             */
/* ZRC/ZRSN codes returned                                                                     */
/*   0/0  Okay                                                                                 */
/*                                                                                             */
/***********************************************************************************************/


#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "../lspfall.h"

#include "../pTSOenv.h"
#include "../pTSOenv.cpp"

#include "ehilight.cpp"

#include <oorexxapi.h>

#include "pedit01.h"

#include "pedrxm1.h"


using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;


RexxObjectPtr RexxEntry lspfServiceHandler( RexxExitContext*,
					    RexxStringObject,
					    RexxStringObject ) ;

RexxObjectPtr RexxEntry editServiceHandler( RexxExitContext*,
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

int getAllRexxVariables( pApplication* ) ;
int setAllRexxVariables( pApplication* ) ;

bool is_pgmmacro( miblock* ) ;


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

	void*    vptr    = context->GetApplicationData() ;
	miblock* mibptr  = static_cast<miblock*>( vptr ) ;
	pedrxm1* macAppl = static_cast<pedrxm1*>( mibptr->macAppl ) ;

	return macAppl->drop( context, upper( context->CString( vars ) ) ) ;
}


RexxRoutine1( RexxStringObject, sysvar, RexxStringObject, type )
{
	//
	// Implement sysvar function.
	// Routine just calls sysvar() method in class TSOENV.
	//

	void*    vptr    = context->GetApplicationData() ;
	miblock* mibptr  = static_cast<miblock*>( vptr ) ;
	pedrxm1* macAppl = static_cast<pedrxm1*>( mibptr->macAppl ) ;

	string var = upper( context->CString( type ) ) ;

	if ( !findword( var, "SYSENV SYSUID SYSNEST SYSLTERM SYSWTERM SYSNODE SYSISPF" ) )
	{
		context->RaiseException1( Rexx_Error_Invalid_argument_user_defined,
					  context->NewStringFromAsciiz( "SYSVAR variable not supported" ) ) ;
		return context->String( "" ) ;
	}

	return context->String( macAppl->sysvar( var ).c_str() ) ;
}


RexxRoutine1( RexxStringObject, outtrap, ARGLIST, arg )
{
	//
	// Implement outtrap function.
	// Routine just calls outtrap() method in class TSOENV.
	//

	void*    vptr    = context->GetApplicationData() ;
	miblock* mibptr  = static_cast<miblock*>( vptr ) ;
	pedrxm1* macAppl = static_cast<pedrxm1*>( mibptr->macAppl ) ;

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

	return context->String( macAppl->outtrap( x1, ds2d( x2 ) ).c_str() ) ;
}


RexxRoutine2( int, listdsi, RexxStringObject, dsn, OPTIONAL_RexxStringObject, type )
{
	//
	// Implement listdsi function.
	// Routine just calls listdsi() method in class TSOENV.
	//

	void*    vptr    = context->GetApplicationData() ;
	miblock* mibptr  = static_cast<miblock*>( vptr ) ;
	pedrxm1* macAppl = static_cast<pedrxm1*>( mibptr->macAppl ) ;

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

	return macAppl->listdsi( context, context->CString( dsn ), type_str ) ;
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

LSPF_APP_MAKER( pedrxm1 )


pedrxm1::pedrxm1()
{
	STANDARD_HEADER( "OOREXX Edit macro interface program", "1.0.1" )
}


void pedrxm1::application()
{
	TRACE_FUNCTION() ;

	int nlvl ;

	miblock  tmiBlock ;
	pedit01* editAppl ;

	void* vptr = get_userdata( ds2d( word( PARM, 1 ) ) ) ;
	mibptr     = static_cast<miblock*>( vptr ) ;
	editAppl   = static_cast<pedit01*>( mibptr->editAppl ) ;

	nlvl = mibptr->nestlvl + 1 ;
	if ( nlvl > 1 )
	{
		tmiBlock = *mibptr ;
		mibptr->clear() ;
	}

	mibptr->parms   = subword( editAppl->pcmd.get_cmd(), 2 ) ;
	mibptr->macAppl = this ;
	mibptr->nestlvl = nlvl ;
	mibptr->errorsRet   = false ;
	editAppl->nestLevel = nlvl ;

	start_rexx() ;

	if ( !mibptr->macro )
	{
		vreplace( "ZMVAL1", mibptr->emacro ) ;
		setmsg( "PEDM012I" ) ;
	}
	else if ( nlvl > 1 && mibptr->exitRC() > 8 )
	{
		vreplace( "ZMVAL1", mibptr->emacro ) ;
		if ( !mibptr->msgset() )
		{
			vreplace( "ZMVAL2", d2ds( mibptr->exitRC() ) ) ;
			mibptr->seterror( "PEDT017E", mibptr->RC ) ;
		}
		if ( !mibptr->rexxError )
		{
			macroError() ;
		}
		mibptr->exitRC( 28 ) ;
	}

	editAppl->clearMacroLabels( nlvl ) ;

	if ( nlvl > 1 )
	{
		tmiBlock.exitRC( mibptr->exitRC() ) ;
		tmiBlock.processed = mibptr->processed ;
		if ( mibptr->fatal && ( !mibptr->rexxError || mibptr->isrError ) )
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


void pedrxm1::start_rexx()
{
	TRACE_FUNCTION() ;

	control( "SIGTERM", "IGNORE" ) ;

	RexxInstance* instance   ;
	RexxThreadContext* threadContext ;
	RexxArrayObject args     ;
	RexxCondition condition  ;
	RexxDirectoryObject cond ;
	RexxObjectPtr result     ;
	RexxLibraryPackage package ;

	RexxOption             options[ 7 ] ;
	RexxContextEnvironment environments[ 4 ] ;
	RexxContextExit        exits[ 2 ] ;

	options[ 0 ].optionName = INITIAL_ADDRESS_ENVIRONMENT ;
	options[ 0 ].option     = "ISPEXEC" ;
	options[ 1 ].optionName = APPLICATION_DATA ;
	options[ 1 ].option     = (void*)mibptr ;
	options[ 2 ].optionName = DIRECT_ENVIRONMENTS ;
	options[ 2 ].option     = (void*)environments ;
	options[ 3 ].optionName = EXTERNAL_CALL_PATH  ;
	options[ 3 ].option     = mibptr->rxpath1.c_str() ;
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

	environments[ 0 ].handler = lspfServiceHandler ;
	environments[ 0 ].name    = "ISPEXEC" ;
	environments[ 1 ].handler = editServiceHandler ;
	environments[ 1 ].name    = "ISREDIT" ;
	environments[ 2 ].handler = TSOServiceHandler ;
	environments[ 2 ].name    = "TSO" ;
	environments[ 3 ].handler = nullptr ;
	environments[ 3 ].name    = ""   ;

	rexxName = mibptr->mfile ;

	if ( !RexxCreateInterpreter( &instance, &threadContext, options ) )
	{
		showErrorScreen( "PSYE034T" ) ;
		return ;
	}

	args   = threadContext->NewArray( 0 ) ;
	result = threadContext->CallProgram( rexxName.c_str(), args ) ;
	if ( threadContext->CheckCondition() )
	{
		cond = threadContext->GetConditionInfo() ;
		threadContext->DecodeConditionInfo( cond, &condition ) ;
		llog( "E", "PEDRXM1 error running REXX.: "<< mibptr->emacro << endl ) ;
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
		mibptr->setRC( 20 ) ;
		mibptr->exitRC( condition.code ) ;
		mibptr->rexxError = true ;
	}
	else if ( mibptr->fatal )
	{
		mibptr->exitRC( 28 ) ;
	}
	else if ( result != NULLOBJECT )
	{
		mibptr->exitRC( threadContext->CString( result ) ) ;
	}
	else
	{
		mibptr->exitRC( 0 ) ;
	}
	instance->Terminate() ;
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
	miblock* mibptr ;

	switch ( subfunction )
	{
	case RXSIOSAY:
	case RXSIOTRC:
	{
		vptr   = context->GetApplicationData() ;
		mibptr = static_cast<miblock*>( vptr ) ;
		pedrxm1* macAppl = static_cast<pedrxm1*>( mibptr->macAppl ) ;
		RXSIOSAY_PARM* say_parm = (RXSIOSAY_PARM *)ParmBlock ;
		if ( say_parm->rxsio_string.strptr )
		{
			macAppl->rdisplay( string( say_parm->rxsio_string.strptr, say_parm->rxsio_string.strlength ), false ) ;
		}
		return RXEXIT_HANDLED ;
	}

	case RXSIOTRD:
	{
		string ans ;
		vptr   = context->GetApplicationData() ;
		mibptr = static_cast<miblock*>( vptr ) ;
		pedrxm1* macAppl = static_cast<pedrxm1*>( mibptr->macAppl ) ;
		RXSIOTRD_PARM* trd_parm = (RXSIOTRD_PARM *)ParmBlock ;
		macAppl->pull( &ans ) ;
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
		vptr   = context->GetApplicationData() ;
		mibptr = static_cast<miblock*>( vptr ) ;
		pedrxm1* macAppl = static_cast<pedrxm1*>( mibptr->macAppl ) ;
		RXSIODTR_PARM* dtr_parm = (RXSIODTR_PARM *)ParmBlock ;
		macAppl->pull( &ans ) ;
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


void pedrxm1::macroError( miblock* mibptr )
{
	TRACE_FUNCTION() ;

	if ( !mibptr->errorsRet )
	{
		macroError() ;
		mibptr->exitRC( 28 ) ;
		vcopy( "ZERRMSG", mibptr->zerrmsg ) ;
	}
	else
	{
		getmsg( mibptr->msgid,
			"ZERRSM",
			"ZERRLM",
			"ZERRALRM",
			"ZERRHM",
			"ZERRTYPE",
			"ZERRWIND" ) ;
		mibptr->zerrmsg = mibptr->msgid ;
		mibptr->msgid = "" ;
		mibptr->fatal = false ;
		mibptr->exitRC( 20 ) ;
	}

	vcopy( "ZERRSM",   mibptr->zerrsm ) ;
	vcopy( "ZERRLM" ,  mibptr->zerrlm ) ;
	vcopy( "ZERRALRM", mibptr->zerralrm ) ;
	vcopy( "ZERRHM",   mibptr->zerrhm ) ;
	vcopy( "ZERRTYPE", mibptr->zerrtype ) ;
	vcopy( "ZERRWIND", mibptr->zerrwind ) ;

	mibptr->setzerr = true ;
}


void pedrxm1::macroError()
{
	//
	// Issue the macro error screen unless CONTROL ERRORS RETURN is in effect.
	// Don't show error panel if RC=28 as it has already been shown.
	//

	TRACE_FUNCTION() ;

	vreplace( "ZERRMSG", mibptr->msgid ) ;

	vreplace( "ZMVAL1", mibptr->val1 ) ;
	vreplace( "ZMVAL2", mibptr->val2 ) ;
	vreplace( "ZMVAL3", mibptr->val3 ) ;

	getmsg( mibptr->msgid,
		"ZERRSM",
		"ZERRLM",
		"ZERRALRM",
		"ZERRHM",
		"ZERRTYPE",
		"ZERRWIND" ) ;

	control( "ERRORS", "STATUS" ) ;
	if ( RC == 4 )
	{
		mibptr->msgid = "" ;
		mibptr->fatal = false ;
		mibptr->exitRC( 0 ) ;
		return ;
	}

	if ( mibptr->RC == 28 )
	{
		return ;
	}

	vreplace( "STR", mibptr->keyword ) ;

	vreplace( "ZERR1", mibptr->sttment ) ;
	vreplace( "ZERR2", d2ds( mibptr->RC ) ) ;
	vreplace( "ZERR3", mibptr->rmacro ) ;

	display( "ISRERROR" ) ;

	mibptr->setRC( 28 ) ;
}


void pedrxm1::showErrorScreen( const string& msg,
			       string val1 )
{
	//
	// Show error screen PSYSER3 for message 'msg' and ZVAL1 val1.
	//

	TRACE_FUNCTION() ;

	size_t i ;

	int l    ;
	int maxw ;

	string* t ;

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
		if ( t->size() > maxw )
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


void pedrxm1::isredit( const string& s )
{
	TRACE_FUNCTION() ;

	pedit01* editAppl = static_cast<pedit01*>( mibptr->editAppl ) ;

	editAppl->isredit( s ) ;
}


RexxObjectPtr RexxEntry lspfServiceHandler( RexxExitContext* context,
					    RexxStringObject address,
					    RexxStringObject command )
{
	//
	// Called on address ISPEXEC REXX statement.
	// Call lspf using the ispexec interface.
	//
	// On entry, update macro application function pool with the rexx variable pool values.
	// On exit, update the rexx variable pool with the macro appl function pool values, and set value RC in REXX.
	//

	TRACE_FUNCTION() ;

	int sRC  ;

	void* vptr ;
	miblock* mibptr ;

	string s = context->CString( command ) ;

	if ( upper( word( s, 1 ) ) == "ISREDIT" )
	{
		return editServiceHandler( context, address, command ) ;
	}

	vptr   = context->GetApplicationData() ;
	mibptr = static_cast<miblock*>( vptr ) ;
	pedrxm1* macAppl = static_cast<pedrxm1*>( mibptr->macAppl ) ;

	getAllRexxVariables( macAppl ) ;

	macAppl->ispexec( s ) ;
	sRC = macAppl->RC ;

	setAllRexxVariables( macAppl ) ;

	return context->WholeNumber( sRC ) ;
}


RexxObjectPtr RexxEntry editServiceHandler( RexxExitContext* context,
					    RexxStringObject address,
					    RexxStringObject command )
{
	//
	// Called on address ISREDIT REXX statement.
	// Substitute variables in the edit command and call the EDITOR ISREDIT interface.
	//
	// On entry, update macro application function pool with the rexx variable pool values.
	// On exit, update the rexx variable pool with the macro appl function pool values, and set value RC in REXX.
	//

	TRACE_FUNCTION() ;

	int sRC ;

	bool errorsRet = false ;

	void* vptr ;
	miblock* mibptr ;

	string s = context->CString( command ) ;

	vptr   = context->GetApplicationData() ;
	mibptr = static_cast<miblock*>( vptr ) ;
	pedit01* editAppl = static_cast<pedit01*>( mibptr->editAppl ) ;
	pedrxm1* macAppl  = static_cast<pedrxm1*>( mibptr->macAppl  ) ;

	if ( mibptr->fatal )
	{
		return context->WholeNumber( 28 ) ;
	}

	getAllRexxVariables( macAppl ) ;

	editAppl->isredit( macAppl->sub_vars( s ) ) ;

	if ( mibptr->fatal )
	{
		mibptr->isrError = true ;
		macAppl->macroError() ;
		sRC = mibptr->RC ;
	}
	else if ( mibptr->runmacro )
	{
		if ( mibptr->nestlvl == 255 )
		{
			mibptr->seterror( "PEDM012H", 24 ) ;
			macAppl->macroError( mibptr ) ;
			mibptr->exitRC( 24 ) ;
			return context->WholeNumber( 24 ) ;
		}
		editAppl->pcmd.set_cmd( mibptr->sttment, editAppl->defNames ) ;
		if ( editAppl->pcmd.error() )
		{
			return context->WholeNumber( 20 ) ;
		}
		mibptr->set_macro( word( editAppl->pcmd.get_cmd(), 1 ), editAppl->defNames ) ;
		if ( mibptr->cmd_macro || !mibptr->pgm_macro )
		{
			if ( !mibptr->getMacroFileName( mibptr->rxpath2 ) )
			{
				if ( mibptr->cmd_macro )
				{
					mibptr->seterror( ( mibptr->RC > 8 ) ? "PEDM012Q" : "PEDT015A", 20 ) ;
					editAppl->pcmd.clear() ;
					macAppl->macroError( mibptr ) ;
					return context->WholeNumber( 20 ) ;
				}
				mibptr->pgm_macro = true ;
				iupper( mibptr->emacro ) ;
			}
			else
			{
				mibptr->cmd_macro = true ;
			}
		}
		macAppl->control( "ERRORS", "STATUS" ) ;
		if ( macAppl->RC == 4 )
		{
			errorsRet         = true ;
			mibptr->errorsRet = true ;
		}
		if ( mibptr->cmd_macro )
		{
			macAppl->select( "PGM(PEDRXM1) PARM("+ d2ds( mibptr->etaskid ) +")" ) ;
		}
		else if ( is_pgmmacro( mibptr ) )
		{
			macAppl->select( "PGM("+ mibptr->emacro +") PARM("+ d2ds( mibptr->etaskid ) +")" ) ;
		}
		else
		{
			return context->WholeNumber( 20 ) ;
		}
		if ( macAppl->RC == 20 && macAppl->ZRESULT == "Abended" )
		{
			macAppl->uabend() ;
		}
		if ( mibptr->setzerr )
		{
			macAppl->vreplace( "ZERRMSG",  mibptr->zerrmsg ) ;
			macAppl->vreplace( "ZERRSM",   mibptr->zerrsm ) ;
			macAppl->vreplace( "ZERRLM",   mibptr->zerrlm ) ;
			macAppl->vreplace( "ZERRALRM", mibptr->zerralrm ) ;
			macAppl->vreplace( "ZERRHM",   mibptr->zerrhm ) ;
			macAppl->vreplace( "ZERRTYPE", mibptr->zerrtype ) ;
			macAppl->vreplace( "ZERRWIND", mibptr->zerrwind ) ;
			mibptr->setzerr = false ;
		}
		if ( errorsRet )
		{
			mibptr->fatal = false ;
		}
		sRC = mibptr->exitRC() ;
	}
	else
	{
		sRC = mibptr->RC ;
	}

	setAllRexxVariables( macAppl ) ;

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

	TRACE_FUNCTION() ;

	miblock* mibptr  = static_cast<miblock*>( context->GetApplicationData() ) ;
	TSOENV* thisAppl = static_cast<TSOENV*>( mibptr->macAppl ) ;

	return context->WholeNumber( thisAppl->TSOEntry( context, command ) ) ;
}


int getAllRexxVariables( pApplication* macAppl )
{
	//
	// For all variables in the rexx variable pool, set the lspf function pool variable.
	// Executed on entry to the command handler from a REXX procedure before any lspf
	// function is called.
	//
	// If the vdefine is for an INTEGER, convert value to a number and vreplace.
	//

	TRACE_FUNCTION() ;

	int rc ;

	string n ;
	string v ;

	set<string>& vl = macAppl->vilist() ;

	SHVBLOCK var ;

	while ( true )
	{
		var.shvnext            = nullptr ;
		var.shvname.strptr     = nullptr ;     /* let REXX allocate the memory */
		var.shvname.strlength  = 0 ;
		var.shvnamelen         = 0 ;
		var.shvvalue.strptr    = nullptr ;     /* let REXX allocate the memory */
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
		RexxFreeMemory( var.shvvalue.strptr ) ;
		if ( var.shvret & RXSHV_LVAR ) { break ; }
	}
	return rc ;
}


int setAllRexxVariables( pApplication* macAppl )
{
	//
	// For all variables in the application function pool, set the rexx variable.
	// Executed before returning to the REXX procedure after a call to the command handler
	// and after all lspf functions have been called.
	//
	// Note: vslist and vilist return the same set<string> reference.
	//

	TRACE_FUNCTION() ;

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


int getRexxVariable( pApplication* macAppl,
		     string n,
		     string& v )
{
	//
	// Get variable value from Rexx variable pool and update the application function pool.
	//

	TRACE_FUNCTION() ;

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
		macAppl->vreplace( n, v ) ;
		rc = macAppl->RC          ;
	}
	RexxFreeMemory( var.shvvalue.strptr ) ;
	return rc ;
}


int setRexxVariable( string n,
		     string v )
{
	TRACE_FUNCTION() ;

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

bool is_pgmmacro( miblock* mibptr )
{
	//
	// Check program being executed as a macro exists, and contains symbol lspf_editmac_v1.
	//

	TRACE_FUNCTION() ;

	pedit01* editAppl = static_cast<pedit01*>( mibptr->editAppl ) ;
	pedrxm1* macAppl  = static_cast<pedrxm1*>( mibptr->macAppl ) ;

	locator loc( mibptr->zldpath, "lib" + mibptr->emacro, ".so" ) ;
	loc.locate() ;
	if ( loc.not_found() )
	{
		mibptr->seterror( "PEDT017P", 20 ) ;
		editAppl->pcmd.clear() ;
		macAppl->macroError( mibptr ) ;
		return false ;
	}

	dynloader loader( loc.entry() ) ;

	loader.open() ;
	if ( loader.errors() )
	{
		mibptr->seterror( "PEDT017Q", 20 ) ;
		editAppl->pcmd.clear() ;
		macAppl->macroError( mibptr ) ;
		return false ;
	}

	loader.lookup( "lspf_editmac_v1" ) ;
	if ( loader.errors() )
	{
		mibptr->seterror( "PEDT017Q", 20 ) ;
		editAppl->pcmd.clear() ;
		macAppl->macroError( mibptr ) ;
		return false ;
	}

	return true ;
}
