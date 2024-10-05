/*
  Copyright (c) 2022 Daniel John Erdos

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
/* CPP-EDIT macro interface class.                                                             */
/*                                                                                             */
/* This program is not compiled separetely.  Edit macro applications must inherit from this    */
/* class and implement pure virtual method start_pgm().                                        */
/*                                                                                             */
/* Setup the macro interface block.                                                            */
/* Call ISREDIT procedure of the EDITOR.  MIB pointer is passed via application userdata.      */
/*                                                                                             */
/* Macro functions running in the EDITOR application, use the macro application function pool. */
/*                                                                                             */
/***********************************************************************************************/


#include <boost/regex.hpp>
#include <list>

#include "../lspfall.h"

#include "ehilight.cpp"

#include "pedit01.h"
#include "pedmcp1.h"


void pedmcp1::application()
{
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

	mibptr->macAppl = this ;
	mibptr->parms   = subword( editAppl->pcmd.get_cmd(), 2 ) ;
	mibptr->nestlvl = nlvl ;
	editAppl->nestLevel = nlvl ;
	mibptr->errorsRet   = false ;

	start_pgm() ;

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
		mibptr->exitRC( 28 ) ;
	}

	editAppl->clearMacroLabels( nlvl ) ;

	if ( nlvl > 1 )
	{
		tmiBlock.exitRC( mibptr->exitRC() ) ;
		tmiBlock.processed = mibptr->processed ;
		if ( mibptr->fatal && mibptr->isrError )
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


void pedmcp1::macroError( miblock* mibptr )
{
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
		mibptr->setRC( mibptr->RC ) ;
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


void pedmcp1::macroError()
{
	//
	// Issue the macro error screen unless CONTROL ERRORS RETURN is in effect.
	// Don't show error panel if RC=28 as it has already been shown.
	//

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
		RC = mibptr->RC ;
		return ;
	}

	if ( mibptr->RC == 28 )
	{
		RC = 28 ;
		return ;
	}

	vreplace( "STR", mibptr->keyword ) ;

	vreplace( "ZERR1", mibptr->sttment ) ;
	vreplace( "ZERR2", d2ds( mibptr->RC ) ) ;
	vreplace( "ZERR3", mibptr->rmacro ) ;

	display( "ISRERROR" ) ;

	mibptr->RC = 28 ;
	RC = 28 ;
}


void pedmcp1::isredit( const string& s )
{
	bool errorsRet = false ;

	if ( mibptr->fatal )
	{
		RC = mibptr->RC ;
		return ;
	}

	pedit01* editAppl = static_cast<pedit01*>( mibptr->editAppl ) ;

	editAppl->isredit( s ) ;

	if ( mibptr->fatal )
	{
		mibptr->isrError = true ;
		macroError() ;
	}
	else if ( mibptr->runmacro )
	{
		if ( mibptr->nestlvl == 255 )
		{
			mibptr->seterror( "PEDM012H", 24 ) ;
			macroError( mibptr ) ;
			mibptr->exitRC( 24 ) ;
			return ;
		}
		editAppl->pcmd.set_cmd( mibptr->sttment, editAppl->defNames ) ;
		if ( editAppl->pcmd.error() )
		{
			return ;
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
					macroError( mibptr ) ;
					return ;
				}
				mibptr->pgm_macro = true ;
				iupper( mibptr->emacro ) ;
			}
			else
			{
				mibptr->cmd_macro = true ;
			}
		}
		control( "ERRORS", "STATUS" ) ;
		if ( RC == 4 )
		{
			errorsRet         = true ;
			mibptr->errorsRet = true ;
		}
		if ( mibptr->cmd_macro )
		{
			select( "PGM(PEDRXM1) PARM("+ d2ds( mibptr->etaskid ) +")" ) ;
		}
		else if ( is_pgmmacro( mibptr ) )
		{
			select( "PGM("+ mibptr->emacro +") PARM("+ d2ds( mibptr->etaskid ) +")" ) ;
		}
		else
		{
			RC = mibptr->RC ;
			return ;
		}
		if ( RC == 20 && ZRESULT == "Abended" )
		{
			uabend() ;
		}
		if ( mibptr->setzerr )
		{
			vreplace( "ZERRMSG",  mibptr->zerrmsg ) ;
			vreplace( "ZERRSM",   mibptr->zerrsm ) ;
			vreplace( "ZERRLM",   mibptr->zerrlm ) ;
			vreplace( "ZERRALRM", mibptr->zerralrm ) ;
			vreplace( "ZERRHM",   mibptr->zerrhm ) ;
			vreplace( "ZERRTYPE", mibptr->zerrtype ) ;
			vreplace( "ZERRWIND", mibptr->zerrwind ) ;
			mibptr->setzerr = false ;
		}
		if ( errorsRet )
		{
			mibptr->fatal = false ;
		}
		RC = mibptr->exitRC() ;
	}
	else
	{
		RC = mibptr->RC ;
	}
}


bool pedmcp1::is_pgmmacro( miblock* mibptr )
{
	//
	// Check program being executed as a macro exists, and contains symbol lspf_editmac_v1.
	//

	pedit01* editAppl = static_cast<pedit01*>( mibptr->editAppl ) ;

	locator loc( mibptr->zldpath, "lib" + mibptr->emacro, ".so" ) ;
	loc.locate() ;
	if ( loc.not_found() )
	{
		mibptr->seterror( "PEDT017P", 20 ) ;
		editAppl->pcmd.clear() ;
		macroError( mibptr ) ;
		return false ;
	}

	dynloader loader( loc.entry() ) ;

	loader.open() ;
	if ( loader.errors() )
	{
		mibptr->seterror( "PEDT017Q", 20 ) ;
		editAppl->pcmd.clear() ;
		macroError( mibptr ) ;
		return false ;
	}

	loader.lookup( "lspf_editmac_v1" ) ;
	if ( loader.errors() )
	{
		mibptr->seterror( "PEDT017Q", 20 ) ;
		editAppl->pcmd.clear() ;
		macroError( mibptr ) ;
		return false ;
	}

	return true ;
}


// ============================================================================================ //


extern "C"
{
	void lspf_editmac_v1()
	{
	}
}
