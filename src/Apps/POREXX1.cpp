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


/* PARM word 1 is the rexx to invloke                                                                   */
/* PARM words 2 onwards are the parameters for the rexx (Arg(1))                                        */
/* ZORXPATH is used to find the rexx if not a fully qualified name passed                               */
/* Use system variable ZOREXPGM to refer to this                                                        */

/* RC/RSN codes returned                                                                                */
/*   0/0  Okay                                                                                          */
/*   8/4  Rexx not found in ZORXPATH concatination                                                      */
/*   8/8  Found but not a regular file                                                                  */
/*  20/condition.code ZRESULT is set to condition.message                                               */

#include <iostream>
#include <string>
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

void POREXX1::application()
{
	log( "I", "Application POREXX1 starting." << endl ) ;

	int i ;
	int j ;
	
	bool found ;

	vdefine( "ZCMD ZVERB", &ZCMD, &ZVERB ) ;

	size_t version  ;
	string rxsource ;
	string RexxExec ;

	RexxInstance *instance   ;
	RexxThreadContext *threadContext ;
	RexxOption options[2]    ;
	RexxArrayObject args     ;
	RexxCondition condition  ;
	RexxDirectoryObject cond ;
	RexxObjectPtr result     ;
	
	options[0].optionName = NULL  ;

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
			ZRC     = 8 ;
			ZRSN    = 8 ;
			ZRESULT = "Not a file" ;
			cleanup()   ;
			return      ;
		}
		if ( !found )
		{
			log( "E", "POREXX1 error. " << rxsource << " not found in ZORXPATH concatination" << endl ) ;
			ZRC     = 8 ;
			ZRSN    = 4 ;
			ZRESULT = "Not found" ;
			cleanup()   ;
			return      ;
		}
	}

	if ( RexxCreateInterpreter( &instance, &threadContext, options ) )
	{
		args = threadContext->NewArray( 1 ) ;
		threadContext->ArrayPut(args, threadContext->String( PARM.c_str() ), 1 ) ;
		version = threadContext->InterpreterVersion();

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
			ZRC     = 20 ;
			ZRSN    = condition.code ;
			ZRESULT = threadContext->CString( condition.message ) ;
		}
		else
		{
		        if (result != NULLOBJECT)
        		{
				CSTRING resultString = threadContext->CString(result);
				debug1( "Data passed back from REXX is :" << resultString << endl ) ;
        		}
		}
		instance->Terminate() ;
	}
	cleanup() ;
	return    ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new POREXX1 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
