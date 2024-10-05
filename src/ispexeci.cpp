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

/* *************************************** ********************************************* **************************************** */
/* ***************************************      ispexeci - ISPEXEC interface module      **************************************** */
/* *************************************** ********************************************* **************************************** */


namespace lspf {


void execiAddpop( pApplication*, const string&, errblock& ) ;
void execiBrowse( pApplication*, const string&, errblock& ) ;
void execiDeq( pApplication*, const string&, errblock& ) ;
void execiDisplay( pApplication*, const string&, errblock& ) ;
void execiDsinfo( pApplication*, const string&, errblock& ) ;
void execiControl( pApplication*, const string&, errblock& ) ;
void execiEdit( pApplication*, const string&, errblock& ) ;
void execiEdrec( pApplication*, const string&, errblock& ) ;
void execiEnq( pApplication*, const string&, errblock& ) ;
void execiFtclose( pApplication*, const string&, errblock& ) ;
void execiFterase( pApplication*, const string&, errblock& ) ;
void execiFtincl( pApplication*, const string&, errblock& ) ;
void execiFtopen( pApplication*, const string&, errblock& ) ;
void execiGetmsg( pApplication*, const string&, errblock& )  ;
void execiLibdef( pApplication*, const string&, errblock& ) ;
void execiList( pApplication*, const string&, errblock& ) ;
void execiLMclose( pApplication*, const string&, errblock& ) ;
void execiLMddisp( pApplication*, const string&, errblock& ) ;
void execiLMdfree( pApplication*, const string&, errblock& ) ;
void execiLMdinit( pApplication*, const string&, errblock& ) ;
void execiLMfree( pApplication*, const string&, errblock& ) ;
void execiLMget( pApplication*, const string&, errblock& ) ;
void execiLMinit( pApplication*, const string&, errblock& ) ;
void execiLMquery( pApplication*, const string&, errblock& ) ;
void execiLMMadd( pApplication*, const string&, errblock& ) ;
void execiLMMdel( pApplication*, const string&, errblock& ) ;
void execiLMMfind( pApplication*, const string&, errblock& ) ;
void execiLMMlist( pApplication*, const string&, errblock& ) ;
void execiLMMrep( pApplication*, const string&, errblock& ) ;
void execiLMopen( pApplication*, const string&, errblock& ) ;
void execiLMput( pApplication*, const string&, errblock& ) ;
void execiLog( pApplication*, const string&, errblock& ) ;
void execiNotify( pApplication*, const string&, errblock& ) ;
void execiPquery( pApplication*, const string&, errblock& ) ;
void execiPull( pApplication*, const string&, errblock& ) ;
void execiQbaselib( pApplication*, const string&, errblock& ) ;
void execiQlibdef( pApplication*, const string&, errblock& ) ;
void execiQueryenq( pApplication*, const string&, errblock& ) ;
void execiQScan( pApplication*, const string&, errblock& ) ;
void execiQtabopen( pApplication*, const string&, errblock& ) ;
void execiRDisplay( pApplication*, const string&, errblock& ) ;
void execiRempop( pApplication*, const string&, errblock& ) ;
void execiSelect( pApplication*, const string&, errblock& ) ;
void execiSetmsg( pApplication*, const string&, errblock& ) ;
void execiSubmit( pApplication*, const string&, errblock& ) ;
void execiTBAdd( pApplication*, const string&, errblock& ) ;
void execiTBBottom( pApplication*, const string&, errblock& ) ;
void execiTBCreate( pApplication*, const string&, errblock& ) ;
void execiTBClose( pApplication*, const string&, errblock& ) ;
void execiTBDelete( pApplication*, const string&, errblock& ) ;
void execiTBDispl( pApplication*, const string&, errblock& ) ;
void execiTBEnd( pApplication*, const string&, errblock& ) ;
void execiTBErase( pApplication*, const string&, errblock& ) ;
void execiTBExist( pApplication*, const string&, errblock& ) ;
void execiTBGet( pApplication*, const string&, errblock& ) ;
void execiTBMod( pApplication*, const string&, errblock& ) ;
void execiTBPut( pApplication*, const string&, errblock& ) ;
void execiTBOpen( pApplication*, const string&, errblock& ) ;
void execiTBQuery( pApplication*, const string&, errblock& ) ;
void execiTBSarg( pApplication*, const string&, errblock& ) ;
void execiTBSave( pApplication*, const string&, errblock& ) ;
void execiTBScan( pApplication*, const string&, errblock& ) ;
void execiTBSkip( pApplication*, const string&, errblock& ) ;
void execiTBSort( pApplication*, const string&, errblock& ) ;
void execiTBStats( pApplication*, const string&, errblock& ) ;
void execiTBTop( pApplication*, const string&, errblock& ) ;
void execiTBVClear( pApplication*, const string&, errblock& ) ;
void execiVerase( pApplication*, const string&, errblock& ) ;
void execiVget( pApplication*, const string&, errblock& ) ;
void execiView( pApplication*, const string&, errblock& ) ;
void execiVput( pApplication*, const string&, errblock& ) ;

map<string, void(*)(pApplication*, const string&, errblock&)> execiServices =
      { { "ADDPOP",   execiAddpop   },
	{ "BROWSE",   execiBrowse   },
	{ "CONTROL",  execiControl  },
	{ "DEQ",      execiDeq      },
	{ "DISPLAY",  execiDisplay  },
	{ "DSINFO",   execiDsinfo   },
	{ "EDIT",     execiEdit     },
	{ "EDREC",    execiEdrec    },
	{ "ENQ",      execiEnq      },
	{ "FTCLOSE",  execiFtclose  },
	{ "FTERASE",  execiFterase  },
	{ "FTINCL",   execiFtincl   },
	{ "FTOPEN",   execiFtopen   },
	{ "GETMSG",   execiGetmsg   },
	{ "LIBDEF",   execiLibdef   },
	{ "LIST",     execiList     },
	{ "LMCLOSE",  execiLMclose  },
	{ "LMDDISP",  execiLMddisp  },
	{ "LMDFREE",  execiLMdfree  },
	{ "LMDINIT",  execiLMdinit  },
	{ "LMFREE",   execiLMfree   },
	{ "LMGET",    execiLMget    },
	{ "LMINIT",   execiLMinit   },
	{ "LMMADD",   execiLMMadd   },
	{ "LMMDEL",   execiLMMdel   },
	{ "LMMFIND",  execiLMMfind  },
	{ "LMMLIST",  execiLMMlist  },
	{ "LMMREP",   execiLMMrep   },
	{ "LMOPEN",   execiLMopen   },
	{ "LMPUT",    execiLMput    },
	{ "LMQUERY",  execiLMquery  },
	{ "LOG",      execiLog      },
	{ "NOTIFY",   execiNotify   },
	{ "PQUERY",   execiPquery   },
	{ "PULL",     execiPull     },
	{ "QBASELIB", execiQbaselib },
	{ "QLIBDEF",  execiQlibdef  },
	{ "QSCAN",    execiQScan    },
	{ "QTABOPEN", execiQtabopen },
	{ "QUERYENQ", execiQueryenq },
	{ "RDISPLAY", execiRDisplay },
	{ "REMPOP",   execiRempop   },
	{ "SELECT",   execiSelect   },
	{ "SETMSG",   execiSetmsg   },
	{ "SUBMIT",   execiSubmit   },
	{ "TBADD",    execiTBAdd    },
	{ "TBBOTTOM", execiTBBottom },
	{ "TBCLOSE",  execiTBClose  },
	{ "TBCREATE", execiTBCreate },
	{ "TBDELETE", execiTBDelete },
	{ "TBDISPL",  execiTBDispl  },
	{ "TBEND",    execiTBEnd    },
	{ "TBERASE",  execiTBErase  },
	{ "TBEXIST",  execiTBExist  },
	{ "TBGET",    execiTBGet    },
	{ "TBMOD",    execiTBMod    },
	{ "TBOPEN",   execiTBOpen   },
	{ "TBPUT",    execiTBPut    },
	{ "TBQUERY",  execiTBQuery  },
	{ "TBSARG",   execiTBSarg   },
	{ "TBSAVE",   execiTBSave   },
	{ "TBSCAN",   execiTBScan   },
	{ "TBSKIP",   execiTBSkip   },
	{ "TBSORT",   execiTBSort   },
	{ "TBSTATS",  execiTBStats  },
	{ "TBTOP",    execiTBTop    },
	{ "TBVCLEAR", execiTBVClear },
	{ "VERASE",   execiVerase   },
	{ "VGET",     execiVget     },
	{ "VIEW",     execiView     },
	{ "VPUT",     execiVput     } } ;


void ispexeci( pApplication* thisAppl, const string& s, errblock& err )
{
	//
	// Call sub_vars to resolve all dialogue variables in string 's' first.
	// Then parse string to call the correct pApplication method for the service.
	//
	// Except for keywords relating to paths, files and patterns, the interface is case insensitive.
	//

	string s1 ;
	string w1 ;

	const string InvalidServices = "VDEFINE VDELETE VCOPY VMASK VREPLACE VRESET" ;

	map<string, void(*)(pApplication*, const string&, errblock&)>::iterator it ;

	s1 = thisAppl->sub_vars( s ) ;
	w1 = upper( word( s1, 1 ) ) ;

	if ( w1 == "ISPEXEC" )
	{
		idelword( s1, 1, 1 ) ;
		w1 = upper( word( s1, 1 ) ) ;
	}
	else if ( w1 == "ISREDIT" )
	{
		return ;
	}

	err.setRC( 0 ) ;
	err.setsrc( s1 ) ;
	err.setdialogsrc() ;

	if ( findword( w1, InvalidServices ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE019A", w1 ) ;
		return ;
	}

	it = execiServices.find( w1 ) ;
	if ( it == execiServices.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE019B", w1 ) ;
		return ;
	}

	it->second( thisAppl, s1, err ) ;
}


void execiAddpop( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	int i_row = 0 ;
	int i_col = 0 ;

	string ap_loc ;
	string ap_row ;
	string ap_col ;

	string str = upper( subword( s, 2 ) ) ;

	ap_loc = parseString1( err, str, "POPLOC()" ) ;
	if ( err.error() ) { return ; }

	ap_row = parseString1( err, str, "ROW()" ) ;
	if ( err.error() ) { return ; }
	if ( ap_row != "" )
	{
		if ( ap_row.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "ROW" ) ;
			return ;
		}
		if ( !datatype( ap_row, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "ROW" ) ;
			return ;
		}
		i_row = ds2d( ap_row ) ;
	}

	ap_col = parseString1( err, str, "COLUMN()" ) ;
	if ( err.error() ) { return ; }
	if ( ap_col != "" )
	{
		if ( ap_col.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "COLUMN" ) ;
			return ;
		}
		if ( !datatype( ap_col, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "COLUMN" ) ;
			return ;
		}
		i_col = ds2d( ap_col ) ;
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->addpop( ap_loc,
			  i_row,
			  i_col ) ;
}


void execiBrowse( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string br_panel ;
	string br_file ;
	string br_dataid ;
	string br_reclen ;

	string str = subword( s, 2 ) ;

	br_file = parseString1( err, str, "FILE()" ) ;
	if ( err.error() ) { return ; }

	if ( err.RSN4() )
	{
		br_dataid = parseString1( err, str, "DATAID()" ) ;
		if ( err.error() ) { return ; }
	}

	br_panel = parseString1( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	br_reclen = parseString1( err, str, "RECLEN()" ) ;
	if ( err.error() ) { return ; }

	if ( br_reclen != "" && !datatype( br_reclen, 'W' ) )
	{
		err.seterrid( TRACE_INFO(), "PSYS013T", "RECLEN" ) ;
		return ;
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->browse( br_file,
			  iupper( br_panel ),
			  iupper( br_dataid ),
			  ( br_reclen == "" ) ? 0 : ds2d( br_reclen ) ) ;
}


void execiDeq( pApplication* thisAppl,
	       const string& s,
	       errblock& err )
{
	string deq_qname ;
	string deq_rname ;

	string str = upper( subword( s, 2 ) ) ;

	enqSCOPE deq_scope = GLOBAL ;

	deq_qname = extractKWord( err, str, "QNAME()" ) ;
	if ( err.error() ) { return ; }

	deq_rname = extractKWord( err, str, "RNAME()" ) ;
	if ( err.error() ) { return ; }

	if ( parseString2( str, "LOCAL" ) )
	{
		deq_scope = LOCAL ;
	}
	else if ( parseString2( str, "GLOBAL" ) )
	{
		deq_scope = GLOBAL ;
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->deqv( deq_qname,
			deq_rname,
			deq_scope ) ;
}


void execiControl( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string s1 = upper( s ) ;

	thisAppl->control( word( s1, 2 ),
			   word( s1, 3 ),
			   subword( s1, 4 ) ) ;
}


void execiDsinfo( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = subword( s, 2 ) ;

	string ds_file ;

	ds_file = parseString1( err, str, "DATASET()" ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN4() )
	{
		ds_file = parseString1( err, str, "FILE()" ) ;
		if ( err.error() ) { return ; }
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->dsinfo( ds_file ) ;
}


void execiDisplay( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	int i_csrpos = 1 ;

	string di_panel ;
	string di_msg   ;
	string di_cursor ;
	string di_csrpos ;
	string di_buffer ;
	string di_retbuf ;
	string di_msgloc ;

	di_panel = parseString1( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	di_msg = parseString1( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	di_cursor = parseString1( err, str, "CURSOR()" ) ;
	if ( err.error() ) { return ; }

	di_csrpos = parseString1( err, str, "CSRPOS()" ) ;
	if ( err.error() ) { return ; }
	if ( di_csrpos != "" )
	{
		if ( di_csrpos.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "CSRPOS" ) ;
			return ;
		}
		if ( !datatype( di_csrpos, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "CSRPOS" ) ;
			return ;
		}
		i_csrpos = ds2d( di_csrpos ) ;
	}

	di_buffer = parseString1( err, str, "COMMAND()" ) ;
	if ( err.error() ) { return ; }

	di_retbuf = parseString1( err, str, "RETBUFFR()" ) ;
	if ( err.error() ) { return ; }

	di_msgloc = parseString1( err, str, "MSGLOC()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->display( di_panel,
			   di_msg,
			   di_cursor,
			   i_csrpos,
			   di_buffer,
			   di_retbuf,
			   di_msgloc ) ;
}


void execiEdit( pApplication* thisAppl,
		const string& s,
		errblock& err )
{
	string str = subword( s, 2 ) ;

	string ed_panel ;
	string ed_macro ;
	string ed_profile ;
	string ed_file  ;
	string ed_lcmds ;
	string ed_confc ;
	string ed_presv ;
	string ed_parm  ;
	string ed_reclen ;
	string ed_dataid ;

	ed_file = parseString1( err, str, "FILE()" ) ;
	if ( err.error() ) { return ; }

	if ( err.RSN4() )
	{
		ed_dataid = parseString1( err, str, "DATAID()" ) ;
		if ( err.error() ) { return ; }
	}

	ed_panel = parseString1( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	ed_macro = parseString1( err, str, "MACRO()" ) ;
	if ( err.error() ) { return ; }

	ed_profile = parseString1( err, str, "PROFILE()" ) ;
	if ( err.error() ) { return ; }

	ed_lcmds = parseString1( err, str, "LINECMDS()" ) ;
	if ( err.error() ) { return ; }

	ed_confc = parseString1( err, str, "CONFIRM()" ) ;
	if ( err.error() ) { return ; }

	ed_parm  = parseString1( err, str, "PARM()" ) ;
	if ( err.error() ) { return ; }

	ed_reclen = parseString1( err, str, "RECLEN()" ) ;
	if ( err.error() ) { return ; }

	if ( ed_reclen != "" && !datatype( ed_reclen, 'W' ) )
	{
		err.seterrid( TRACE_INFO(), "PSYS013T", "RECLEN" ) ;
		return ;
	}

	if ( parseString2( str, "PRESERVE" ) )
	{
		ed_presv = "PRESERVE" ;
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->edit( ed_file,
			iupper( ed_panel ),
			ed_macro,
			iupper( ed_profile ),
			iupper( ed_dataid ),
			iupper( ed_lcmds ),
			iupper( ed_confc ),
			ed_presv,
			iupper( ed_parm ),
			( ed_reclen == "" ) ? 0 : ds2d( ed_reclen ) ) ;
}


void execiEdrec( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->edrec( upper( word( s, 2 ) ) ) ;
}


void execiEnq( pApplication* thisAppl,
	       const string& s,
	       errblock& err )
{
	string enq_qname ;
	string enq_rname ;

	string str = upper( subword( s, 2 ) ) ;

	enqDISP  enq_disp  = EXC    ;
	enqSCOPE enq_scope = GLOBAL ;

	enq_qname = extractKWord( err, str, "QNAME()" ) ;
	if ( err.error() ) { return ; }

	enq_rname = extractKWord( err, str, "RNAME()" ) ;
	if ( err.error() ) { return ; }

	if ( parseString2( str, "LOCAL" ) )
	{
		enq_scope = LOCAL ;
	}
	else if ( parseString2( str, "GLOBAL" ) )
	{
		enq_scope = GLOBAL ;
	}

	if      ( str == "SHR" ) { enq_disp = SHR ; }
	else if ( str == ""    ) { enq_disp = EXC ; }
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->enqv( enq_qname,
			enq_rname,
			enq_disp,
			enq_scope ) ;
}


void execiFtclose( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = subword( s, 2 ) ;

	string ft_name ;
	string ft_libs ;

	ft_libs = parseString1( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	ft_name = parseString1( err, str, "NAME()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 1 )
	{
		parseString2( str, "NOREPL" ) ;
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->ftclose( ft_name,
			   ft_libs,
			   str ) ;
}


void execiFterase( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = subword( s, 3 ) ;

	string ft_name ;
	string ft_libs ;

	ft_name = word( s, 2 ) ;

	ft_libs = parseString1( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->fterase( ft_name,
			   ft_libs ) ;
}


void execiFtincl( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	if ( words( s ) > 3 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 4 ) ) ;
		return ;
	}

	thisAppl->ftincl( word( s, 2 ),
			  word( s, 3 ) ) ;
}


void execiFtopen( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->ftopen( word( s, 2 ) ) ;
}


void execiGetmsg( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string gm_msg  ;
	string gm_smsg ;
	string gm_lmsg ;
	string gm_alarm ;
	string gm_help ;
	string gm_type ;
	string gm_window ;

	gm_msg = parseString1( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	gm_smsg = parseString1( err, str, "SHORTMSG()" ) ;
	if ( err.error() ) { return ; }

	gm_lmsg = parseString1( err, str, "LONGMSG()" ) ;
	if ( err.error() ) { return ; }

	gm_alarm = parseString1( err, str, "ALARM()" ) ;
	if ( err.error() ) { return ; }

	gm_help = parseString1( err, str, "HELP()" ) ;
	if ( err.error() ) { return ; }

	gm_type = parseString1( err, str, "TYPE()" ) ;
	if ( err.error() ) { return ; }

	gm_window = parseString1( err, str, "WINDOW()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->getmsg( gm_msg,
			  gm_smsg,
			  gm_lmsg,
			  gm_alarm,
			  gm_help,
			  gm_type,
			  gm_window ) ;
}


void execiLibdef( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = subword( s, 2 ) ;

	string ld_files = parseString1( err, str, "ID()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 3 )
	{
		err.seterrid( TRACE_INFO(), "PSYE022K" ) ;
		return ;
	}

	iupper( str ) ;

	thisAppl->libdef( word( str, 1 ),
			  word( str, 2 ),
			  ld_files,
			  word( str, 3 ) ) ;
}


void execiList( pApplication* thisAppl,
		const string& s,
		errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	if ( words( str ) > 1 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->list( str ) ;
}


void execiLMclose( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lm_dataid ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmclose( lm_dataid ) ;
}


void execiLMddisp( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lm_listid ;

	lm_listid = parseString1( err, str, "LISTID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmddisp( lm_listid ) ;
}


void execiLMdfree( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lm_listid ;

	lm_listid = parseString1( err, str, "LISTID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmdfree( lm_listid ) ;
}


void execiLMdinit( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = subword( s, 2 ) ;

	string lm_listidv ;
	string lm_level ;

	lm_level = parseString1( err, str, "LEVEL()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	lm_listidv = parseString1( err, str, "LISTID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmdinit( lm_listidv,
			   lm_level ) ;
}


void execiLMfree( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lm_dataid ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmfree( lm_dataid ) ;
}


void execiLMget( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lm_dataid ;
	string lm_mode ;
	string lm_dataloc ;
	string lm_datalenv ;
	string lm_maxlen ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	lm_mode = parseString1( err, str, "MODE()" ) ;
	if ( err.error() ) { return ; }

	lm_dataloc = parseString1( err, str, "DATALOC()" ) ;
	if ( err.error() ) { return ; }

	lm_datalenv = parseString1( err, str, "DATALEN()" ) ;
	if ( err.error() ) { return ; }

	lm_maxlen = parseString1( err, str, "MAXLEN()" ) ;
	if ( err.error() ) { return ; }

	if ( !datatype( lm_maxlen, 'W' ) )
	{
		err.seterrid( TRACE_INFO(), "PSYS013T", "MAXLEN" ) ;
		return ;
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmget( lm_dataid,
			 lm_mode,
			 lm_dataloc,
			 lm_datalenv,
			 ds2d( lm_maxlen ) ) ;
}


void execiLMinit( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = subword( s, 2 ) ;

	string lm_dataset ;
	string lm_ddname  ;
	string lm_dataidv ;
	string lm_enq ;

	LM_DISP lm_disp = LM_SHR ;

	lm_dataset = parseString1( err, str, "DATASET()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	lm_ddname = parseString1( err, str, "DDNAME()" ) ;
	if ( err.error() ) { return ; }

	lm_dataidv = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	lm_enq = parseString1( err, str, "ENQ()" ) ;
	if ( err.error() ) { return ; }

	if ( lm_enq == "SHR" )
	{
		lm_disp = LM_SHR ;
	}
	else if ( lm_enq == "EXCLU" )
	{
		lm_disp = LM_EXCLU ;
	}
	else if ( lm_enq != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", lm_enq ) ;
		return ;
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lminit( lm_dataidv,
			  lm_dataset,
			  lm_ddname,
			  lm_disp ) ;
}


void execiLMMadd( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = subword( s, 2 ) ;

	string lm_dataid ;
	string lm_member ;

	lm_member = parseString1( err, str, "MEMBER()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmmadd( lm_dataid,
			  lm_member ) ;
}


void execiLMMdel( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = subword( s, 2 ) ;

	string lm_dataid ;
	string lm_member ;

	lm_member = parseString1( err, str, "MEMBER()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmmdel( lm_dataid,
			  lm_member ) ;
}


void execiLMMfind( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = subword( s, 2 ) ;

	string lm_dataid ;
	string lm_member ;
	string lm_stats ;

	lm_member = parseString1( err, str, "MEMBER()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	lm_stats = parseString1( err, str, "STATS()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmmfind( lm_dataid,
			   lm_member,
			   lm_stats ) ;
}


void execiLMMlist( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = subword( s, 2 ) ;

	string lm_dataid ;
	string lm_option ;
	string lm_member ;
	string lm_stats ;
	string lm_pattern ;

	lm_pattern = parseString1( err, str, "PATTERN()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	lm_option = parseString1( err, str, "OPTION()" ) ;
	if ( err.error() ) { return ; }

	lm_member = parseString1( err, str, "MEMBER()" ) ;
	if ( err.error() ) { return ; }

	lm_stats = parseString1( err, str, "STATS()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmmlist( lm_dataid,
			   lm_option,
			   lm_member,
			   lm_stats,
			   lm_pattern ) ;
}


void execiLMMrep( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = subword( s, 2 ) ;

	string lm_dataid ;
	string lm_member ;

	lm_member = parseString1( err, str, "MEMBER()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmmrep( lm_dataid,
			  lm_member ) ;
}


void execiLMopen( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lm_dataid ;
	string lm_option ;
	string lm_orgv ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	lm_option = parseString1( err, str, "OPTION()" ) ;
	if ( err.error() ) { return ; }

	lm_orgv = parseString1( err, str, "ORG()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmopen( lm_dataid,
			  lm_option,
			  lm_orgv ) ;
}


void execiLMput( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lm_dataid ;
	string lm_mode ;
	string lm_dataloc ;
	string lm_datalen ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	lm_mode = parseString1( err, str, "MODE()" ) ;
	if ( err.error() ) { return ; }

	lm_dataloc = parseString1( err, str, "DATALOC()" ) ;
	if ( err.error() ) { return ; }

	lm_datalen = parseString1( err, str, "DATALEN()" ) ;
	if ( err.error() ) { return ; }

	if ( !datatype( lm_datalen, 'W' ) )
	{
		err.seterrid( TRACE_INFO(), "PSYS013T", "DATALEN" ) ;
		return ;
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmput( lm_dataid,
			 lm_mode,
			 lm_dataloc,
			 ds2d( lm_datalen ) ) ;
}


void execiLMquery( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lm_dataid ;
	string lm_dsorgv ;
	string lm_dsnv   ;
	string lm_ddnv   ;
	string lm_enqv   ;
	string lm_openv  ;

	lm_dataid = parseString1( err, str, "DATAID()" ) ;
	if ( err.error() ) { return ; }

	lm_dsnv    = parseString1( err, str, "DATASET()" ) ;
	if ( err.error() ) { return ; }

	lm_ddnv    = parseString1( err, str, "DDNAME()" ) ;
	if ( err.error() ) { return ; }

	lm_enqv    = parseString1( err, str, "ENQ()" ) ;
	if ( err.error() ) { return ; }

	lm_dsorgv  = parseString1( err, str, "DSORG()" ) ;
	if ( err.error() ) { return ; }

	lm_openv   = parseString1( err, str, "OPEN()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->lmquery( lm_dataid,
			   lm_dsnv,
			   lm_ddnv,
			   lm_enqv,
			   lm_openv,
			   lm_dsorgv ) ;
}


void execiLog( pApplication* thisAppl,
	       const string& s,
	       errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string lg_msgid ;

	lg_msgid = parseString1( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->log( lg_msgid ) ;
}


void execiNotify( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string msg = strip( subword( s, 2 ) ) ;

	size_t p ;

	char quote ;

	if ( msg.size() > 1 && ( msg.front() == '"' || msg.front() == '\'' ) )
	{
		quote = msg.front() ;
		p     = msg.find( quote, 1 ) ;
		if ( p == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
			return ;
		}
		if ( p != msg.size() - 1 )
		{
			err.seterrid( TRACE_INFO(), "PSYE037G" ) ;
			return ;
		}
		msg = msg.substr( 1, p-1 ) ;
	}

	thisAppl->notify( msg, false ) ;
}


void execiPquery( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string pq_panel ;
	string pq_arean ;
	string pq_areat ;
	string pq_width ;
	string pq_depth ;
	string pq_row   ;
	string pq_col   ;

	pq_panel = parseString1( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	pq_arean = parseString1( err, str, "AREANAME()" ) ;
	if ( err.error() ) { return ; }

	pq_areat = parseString1( err, str, "AREATYPE()" ) ;
	if ( err.error() ) { return ; }

	pq_width = parseString1( err, str, "WIDTH()" ) ;
	if ( err.error() ) { return ; }

	pq_depth = parseString1( err, str, "DEPTH()" ) ;
	if ( err.error() ) { return ; }

	pq_row = parseString1( err, str, "ROW()" ) ;
	if ( err.error() ) { return ; }

	pq_col = parseString1( err, str, "COLUMN()" ) ;
	if ( err.error() ) { return ; }

	thisAppl->pquery( pq_panel,
			  pq_arean,
			  pq_areat,
			  pq_width,
			  pq_depth,
			  pq_row,
			  pq_col ) ;
}


void execiPull( pApplication* thisAppl,
		const string& s,
		errblock& err )
{
	int ws = words( s ) ;

	if ( ws == 1 )
	{
		err.seterrid( TRACE_INFO(), "PSYE019C", "PULL destination variable" ) ;
		return ;
	}
	else if ( ws > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->pull( upper( word( s, 2 ) ) ) ;
}


void execiQbaselib( pApplication* thisAppl,
		    const string& s,
		    errblock& err )
{
	string str     ;

	string qb_lib  ;
	string qb_id   ;

	qb_lib  = upper( word( s, 2 ) ) ;
	str     = upper( subword( s, 3 ) ) ;

	qb_id = parseString1( err, str, "ID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->qbaselib( qb_lib,
			    qb_id ) ;
}


void execiQlibdef( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str     ;

	string ql_lib  ;
	string ql_type ;
	string ql_id   ;

	ql_lib  = upper( word( s, 2 ) ) ;
	str     = upper( subword( s, 3 ) ) ;
	ql_type = parseString1( err, str, "TYPE()" ) ;
	if ( err.error() ) { return ; }

	ql_id = parseString1( err, str, "ID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->qlibdef( ql_lib,
			   ql_type,
			   ql_id ) ;
}


void execiQueryenq( pApplication* thisAppl,
		    const string& s,
		    errblock& err )
{
	string str ;

	string qe_table ;
	string qe_qname ;
	string qe_rname ;
	string qe_limit ;
	string qe_save ;
	string qe_req ;

	bool qe_wait ;

	int i_limit ;

	str = subword( s, 2 ) ;

	qe_save = parseString1( err, str, "SAVE()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	qe_qname = parseString1( err, str, "QNAME()" ) ;
	if ( err.error() ) { return ; }

	qe_rname = parseString1( err, str, "RNAME()" ) ;
	if ( err.error() ) { return ; }

	qe_req = parseString1( err, str, "REQ()" ) ;
	if ( err.error() ) { return ; }

	qe_table = parseString1( err, str, "TABLE()" ) ;
	if ( err.error() ) { return ; }

	qe_limit = parseString1( err, str, "LIMIT()" ) ;
	if ( err.error() ) { return ; }
	if ( qe_limit != "" && !datatype( qe_limit, 'W' ) )
	{
		err.seterrid( TRACE_INFO(), "PSYS013T", "LIMIT" ) ;
		return ;
	}
	i_limit = ( qe_limit != "" ) ? ds2d( qe_limit ) : 5000 ;

	qe_wait = parseString2( str, "WAIT" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->queryenq( qe_table,
			    qe_qname,
			    qe_rname,
			    qe_req,
			    ( qe_wait ) ? "WAIT" : "",
			    i_limit,
			    qe_save ) ;
}


void execiQScan( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	string qsc_qname ;
	string qsc_rname ;

	string str = upper( subword( s, 2 ) ) ;

	enqDISP  qsc_disp  = EXC    ;
	enqSCOPE qsc_scope = GLOBAL ;

	qsc_qname = extractKWord( err, str, "QNAME()" ) ;
	if ( err.error() ) { return ; }

	qsc_rname = extractKWord( err, str, "RNAME()" ) ;
	if ( err.error() ) { return ; }

	if ( parseString2( str, "LOCAL" ) )
	{
		qsc_scope = LOCAL ;
	}
	else if ( parseString2( str, "GLOBAL" ) )
	{
		qsc_scope = GLOBAL ;
	}

	if      ( str == "SHR" ) { qsc_disp = SHR ; }
	else if ( str == ""    ) { qsc_disp = EXC ; }
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->qscanv( qsc_qname,
			  qsc_rname,
			  qsc_disp,
			  qsc_scope ) ;
}


void execiQtabopen( pApplication* thisAppl,
		    const string& s,
		    errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string qt_list ;

	qt_list = parseString1( err, str, "LIST()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->qtabopen( qt_list ) ;
}


void execiRDisplay( pApplication* thisAppl,
		    const string& s,
		    errblock& err )
{
	//
	// Call rdisplay(s,false) which does not do dialogue variable substitution as this has
	// already been done.
	//

	thisAppl->rdisplay( substr( s, 10 ), false ) ;
}


void execiRempop( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	thisAppl->rempop( upper( subword( s, 2 ) ) ) ;
}


void execiSelect( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	selobj sel ;

	if ( !sel.parse( err, subword( s, 2 ) ) ) { return ; }

	thisAppl->select( sel ) ;
}


void execiSetmsg( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string s_msg ;
	string s_msgloc ;

	msgSET t ;

	s_msg = parseString1( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	s_msgloc = parseString1( err, str, "MSGLOC()" ) ;
	if ( err.error() ) { return ; }

	if      ( str == "COND" ) { t = COND   ; }
	else if ( str == "" )     { t = UNCOND ; }
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->setmsg( s_msg,
			  t,
			  s_msgloc ) ;
}


void execiSubmit( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	selobj sel ;

	if ( !sel.parse( err, subword( s, 2 ) ) ) { return ; }

	thisAppl->submit( sel ) ;
}


void execiTBAdd( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	int i_numrows = 0 ;

	string str        ;

	string tb_name    ;
	string tb_namelst ;
	string tb_numrows ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_namelst = parseString1( err, str, "SAVE()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_namelst != "" ) { tb_namelst = "(" + tb_namelst + ")" ; }

	tb_numrows = parseString1( err, str, "MULT()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_numrows != "" )
	{
		if ( tb_numrows.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "MULT" ) ;
			return ;
		}
		if ( !datatype( tb_numrows, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "MULT" ) ;
			return ;
		}
		i_numrows = ds2d( tb_numrows ) ;
	}

	thisAppl->tbadd( tb_name,
			 tb_namelst,
			 str,
			 i_numrows ) ;
}


void execiTBBottom( pApplication* thisAppl,
		    const string& s,
		    errblock& err )
{
	string str       ;

	string tb_name   ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString1( err, str, "SAVENAME()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString1( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_crpnm = parseString1( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	thisAppl->tbbottom( tb_name,
			    tb_savenm,
			    tb_rowid,
			    str,
			    tb_crpnm ) ;
}


void execiTBClose( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str      ;

	string tb_name  ;
	string tb_nname ;
	string tb_libs  ;

	tb_name  = upper( word( s, 2 ) ) ;
	str      = subword( s, 3 ) ;

	tb_libs = parseString1( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;
	tb_nname = parseString1( err, str, "NAME()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbclose( tb_name,
			   tb_nname,
			   tb_libs ) ;
}


void execiTBCreate( pApplication* thisAppl,
		    const string& s,
		    errblock& err )
{
	string str      ;

	string tb_name  ;
	string tb_keys  ;
	string tb_names ;
	string tb_libs  ;

	tbWRITE tb_write = WRITE ;
	tbDISP  tb_disp  = NON_SHARE ;
	tbREP   tb_rep   = NOREPLACE ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_libs = parseString1( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;
	tb_keys  = parseString1( err, str, "KEYS()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_keys != "" ) { tb_keys = "(" + tb_keys + ")" ; }

	tb_names = parseString1( err, str, "NAMES()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_names != "" ) { tb_names = "(" + tb_names + ")" ; }

	if ( not parseString2( str, "WRITE" ) )
	{
		if ( parseString2( str, "NOWRITE" ) )
		{
			tb_write = NOWRITE ;
		}
	}

	if ( parseString2( str, "REPLACE" ) )
	{
		tb_rep = REPLACE ;
	}

	if ( parseString2( str, "SHARE" ) )
	{
		tb_disp = SHARE ;
	}

	if ( words( str ) > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbcreate( tb_name,
			    tb_keys,
			    tb_names,
			    tb_write,
			    tb_rep,
			    tb_libs,
			    tb_disp ) ;
}


void execiTBDelete( pApplication* thisAppl,
		    const string& s,
		    errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbdelete( upper( word( s, 2 ) ) ) ;
}


void execiTBDispl( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	int i_csrpos = 1 ;
	int i_csrrow = 0 ;

	string str  ;

	string tb_name  ;
	string tb_panel ;
	string tb_msg   ;
	string tb_cursor  ;
	string tb_csrrow  ;
	string tb_csrpos  ;
	string tb_autosel ;
	string tb_posn    ;
	string tb_rowid   ;
	string tb_msgloc  ;

	tb_name = upper( word( s, 2 ) ) ;
	str = upper( subword( s, 3 ) ) ;

	tb_panel = parseString1( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	tb_msg = parseString1( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	tb_cursor = parseString1( err, str, "CURSOR()" ) ;
	if ( err.error() ) { return ; }

	tb_csrrow = parseString1( err, str, "CSRROW()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_csrrow != "" )
	{
		if ( tb_csrrow.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "CSRROW" ) ;
			return ;
		}
		if ( !datatype( tb_csrrow, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "CSRROW" ) ;
			return ;
		}
		i_csrrow = ds2d( tb_csrrow ) ;
	}

	tb_csrpos = parseString1( err, str, "CSRPOS()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_csrpos != "" )
	{
		if ( tb_csrpos.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "CSRPOS" ) ;
			return ;
		}
		if ( !datatype( tb_csrpos, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "CSRPOS" ) ;
			return ;
		}
		i_csrpos = ds2d( tb_csrpos ) ;
	}

	tb_autosel = parseString1( err, str, "AUTOSEL()" ) ;
	if ( err.error() ) { return ; }

	tb_posn = parseString1( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString1( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_msgloc = parseString1( err, str, "MSGLOC()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbdispl( tb_name,
			   tb_panel,
			   tb_msg,
			   tb_cursor,
			   i_csrrow,
			   i_csrpos,
			   tb_autosel,
			   tb_posn,
			   tb_rowid,
			   tb_msgloc ) ;
}


void execiTBEnd( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbend( upper( word( s, 2 ) ) ) ;
}


void execiTBErase( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str ;

	string tb_name ;
	string tb_libs ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_libs = parseString1( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tberase( tb_name,
			   tb_libs ) ;
}


void execiTBExist( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbexist( upper( word( s, 2 ) ) ) ;
}


void execiTBGet( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	string str ;

	string tb_name   ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString1( err, str, "SAVENAME()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString1( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_crpnm = parseString1( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	thisAppl->tbget( tb_name,
			 tb_savenm,
			 tb_rowid,
			 str,
			 tb_crpnm ) ;
}


void execiTBMod( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	string str ;

	string tb_name ;
	string tb_savenm ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString1( err, str, "SAVE()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_savenm != "" ) { tb_savenm = "(" + tb_savenm + ")" ; }

	thisAppl->tbmod( tb_name,
			 tb_savenm,
			 str ) ;
}


void execiTBPut( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	string str ;

	string tb_name ;
	string tb_savenm ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString1( err, str, "SAVE()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_savenm != "" ) { tb_savenm = "(" + tb_savenm + ")" ; }

	thisAppl->tbput( tb_name,
			 tb_savenm,
			 str ) ;
}


void execiTBOpen( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str ;

	string tb_name ;
	string tb_libs ;

	tbWRITE tb_write = WRITE ;
	tbDISP  tb_disp  = NON_SHARE ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	if ( not parseString2( str, "WRITE" ) )
	{
		if ( parseString2( str, "NOWRITE" ) )
		{
			tb_write = NOWRITE ;
		}
	}

	tb_libs = parseString1( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;
	if ( parseString2( str, "SHARE" ) )
	{
		tb_disp = SHARE ;
	}

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbopen( tb_name,
			  tb_write,
			  tb_libs,
			  tb_disp ) ;
}


void execiTBQuery( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str ;

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

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_keys = parseString1( err, str, "KEYS()" ) ;
	if ( err.error() ) { return ; }

	tb_names = parseString1( err, str, "NAMES()" ) ;
	if ( err.error() ) { return ; }

	tb_rownum = parseString1( err, str, "ROWNUM()" ) ;
	if ( err.error() ) { return ; }

	tb_keynum = parseString1( err, str, "KEYNUM()" ) ;
	if ( err.error() ) { return ; }

	tb_namenum = parseString1( err, str, "NAMENUM()" ) ;
	if ( err.error() ) { return ; }

	tb_pos = parseString1( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	tb_srtflds = parseString1( err, str, "SORTFLDS()" ) ;
	if ( err.error() ) { return ; }

	tb_sarglst = parseString1( err, str, "SARGLIST()" ) ;
	if ( err.error() ) { return ; }

	tb_sargcnd = parseString1( err, str, "SARGCOND()" ) ;
	if ( err.error() ) { return ; }

	tb_sargdir = parseString1( err, str, "SARGDIR()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbquery( tb_name,
			   tb_keys,
			   tb_names,
			   tb_rownum,
			   tb_keynum,
			   tb_namenum,
			   tb_pos,
			   tb_srtflds,
			   tb_sarglst,
			   tb_sargcnd,
			   tb_sargdir ) ;
}


void execiTBSarg( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	bool t1 ;
	bool t2 ;

	string str ;

	string tb_name ;
	string tb_dir  ;
	string tb_arglst  ;
	string tb_namecnd ;

	tb_name  = upper( word( s, 2 ) )    ;
	str      = upper( subword( s, 3 ) ) ;

	tb_arglst = parseString1( err, str, "ARGLIST()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_arglst != "" ) { tb_arglst = "(" + tb_arglst + ")" ; }

	tb_namecnd = parseString1( err, str, "NAMECOND()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_namecnd != "" ) { tb_namecnd = "(" + tb_namecnd + ")" ; }

	t1 = parseString2( str, "NEXT" ) ;
	t2 = parseString2( str, "PREVIOUS" ) ;

	if ( t1 && t2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE019G", "NEXT", "PREVIOUS" ) ;
		return ;
	}
	else
	{
		tb_dir = ( t2 ) ? "PREVIOUS" : "NEXT" ;
	}

	if ( words( str ) > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbsarg( tb_name,
			  tb_arglst,
			  tb_dir,
			  tb_namecnd ) ;
}


void execiTBSave( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str ;

	string tb_name  ;
	string tb_nname ;
	string tb_libs  ;

	tb_name  = upper( word( s, 2 ) ) ;
	str      = subword( s, 3 ) ;

	tb_nname = parseString1( err, str, "NAME()" ) ;
	if ( err.error() ) { return ; }

	tb_libs = parseString1( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbsave( tb_name,
			  iupper( tb_nname ),
			  tb_libs ) ;
}


void execiTBScan( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	bool t1 ;
	bool t2 ;

	string str ;

	string tb_name    ;
	string tb_namelst ;
	string tb_savenm  ;
	string tb_rowid   ;
	string tb_crpnm   ;
	string tb_condlst ;
	string tb_dir     ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_namelst = parseString1( err, str, "ARGLIST()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_namelst != "" ) { tb_namelst = "(" + tb_namelst + ")" ; }

	tb_savenm = parseString1( err, str, "SAVENAME()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString1( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_crpnm = parseString1( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	tb_condlst = parseString1( err, str, "CONDLIST()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_condlst != "" ) { tb_condlst = "(" + tb_condlst + ")" ; }

	t1 = parseString2( str, "NEXT" ) ;
	t2 = parseString2( str, "PREVIOUS" ) ;

	if ( t1 && t2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE019G", "NEXT", "PREVIOUS" ) ;
		return ;
	}

	if ( t1 )
	{
		tb_dir = "NEXT" ;
	}
	else if ( t2 )
	{
		tb_dir = "PREVIOUS" ;
	}

	thisAppl->tbscan( tb_name,
			  tb_namelst,
			  tb_savenm,
			  tb_rowid,
			  tb_dir,
			  str,
			  tb_crpnm,
			  tb_condlst ) ;
}


void execiTBSkip( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	int i_num = 1 ;

	string str ;

	string tb_name ;
	string tb_num ;
	string tb_savenm ;
	string tb_crpnm ;
	string tb_rowid ;
	string tb_row ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_num = parseString1( err, str, "NUMBER()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_num != "" )
	{
		if ( tb_num.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "NUMBER" ) ;
			return ;
		}
		if ( !datatype( tb_num, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "NUMBER" ) ;
			return ;
		}
		i_num = ds2d( tb_num ) ;
	}

	tb_savenm = parseString1( err, str, "SAVENAME()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString1( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_row = parseString1( err, str, "ROW()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_row != "" )
	{
		if ( tb_row.size() > 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYS013S", "ROW" ) ;
			return ;
		}
		if ( !datatype( tb_row, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYS013T", "ROW" ) ;
			return ;
		}
		if ( tb_num == "" ) { i_num = 0 ; }
	}

	tb_crpnm = parseString1( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	thisAppl->tbskip( tb_name,
			  i_num,
			  tb_savenm,
			  tb_rowid,
			  tb_row,
			  str,
			  tb_crpnm ) ;
}


void execiTBSort( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str ;

	string tb_name ;
	string tb_flds ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_flds = parseString1( err, str, "FIELDS()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}
	if ( tb_flds != "" ) { tb_flds = "(" + tb_flds + ")" ; }

	thisAppl->tbsort( tb_name,
			  tb_flds ) ;
}


void execiTBStats( pApplication* thisAppl,
		   const string& s,
		   errblock& err )
{
	string str ;

	string tb_name ;
	string tb_cdate ;
	string tb_ctime ;
	string tb_udate ;
	string tb_utime ;
	string tb_user  ;
	string tb_rowcreat ;
	string tb_rowcurr  ;
	string tb_rowupd   ;
	string tb_tableupd ;
	string tb_service  ;
	string tb_retcode  ;
	string tb_status1  ;
	string tb_status2  ;
	string tb_status3  ;
	string tb_library  ;
	string tb_virtsize ;
	string tb_cdate4d  ;
	string tb_udate4d  ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_library = parseString1( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;

	tb_cdate = parseString1( err, str, "CDATE()" ) ;
	if ( err.error() ) { return ; }

	tb_ctime = parseString1( err, str, "CTIME()" ) ;
	if ( err.error() ) { return ; }

	tb_udate = parseString1( err, str, "UDATE()" ) ;
	if ( err.error() ) { return ; }

	tb_utime = parseString1( err, str, "UTIME()" ) ;
	if ( err.error() ) { return ; }

	tb_user  = parseString1( err, str, "USER()" ) ;
	if ( err.error() ) { return ; }

	tb_rowcreat = parseString1( err, str, "ROWCREAT()" ) ;
	if ( err.error() ) { return ; }

	tb_rowcurr  = parseString1( err, str, "ROWCURR()" ) ;
	if ( err.error() ) { return ; }

	tb_rowupd   = parseString1( err, str, "ROWUPD()" ) ;
	if ( err.error() ) { return ; }

	tb_tableupd = parseString1( err, str, "TABLEUPD()" ) ;
	if ( err.error() ) { return ; }

	tb_service  = parseString1( err, str, "SERVICE()" ) ;
	if ( err.error() ) { return ; }

	tb_retcode  = parseString1( err, str, "RETCODE()" ) ;
	if ( err.error() ) { return ; }

	tb_status1  = parseString1( err, str, "STATUS1()" ) ;
	if ( err.error() ) { return ; }

	tb_status2  = parseString1( err, str, "STATUS2()" ) ;
	if ( err.error() ) { return ; }

	tb_status3  = parseString1( err, str, "STATUS3()" ) ;
	if ( err.error() ) { return ; }

	tb_virtsize = parseString1( err, str, "VIRTSIZE()" ) ;
	if ( err.error() ) { return ; }

	tb_cdate4d  = parseString1( err, str, "CDATE4D()" ) ;
	if ( err.error() ) { return ; }

	tb_udate4d  = parseString1( err, str, "UDATE4D()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbstats( tb_name,
			   tb_cdate,
			   tb_ctime,
			   tb_udate,
			   tb_utime,
			   tb_user,
			   tb_rowcreat,
			   tb_rowcurr,
			   tb_rowupd,
			   tb_tableupd,
			   tb_service,
			   tb_retcode,
			   tb_status1,
			   tb_status2,
			   tb_status3,
			   tb_library,
			   tb_virtsize,
			   tb_cdate4d,
			   tb_udate4d ) ;
}


void execiTBTop( pApplication* thisAppl,
		 const string& s,
		 errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbtop( upper( word( s, 2 ) ) ) ;
}


void execiTBVClear( pApplication* thisAppl,
		    const string& s,
		    errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbvclear( upper( word( s, 2 ) ) ) ;
}


void execiVerase( pApplication* thisAppl,
		  const string& s,
		  errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string vars ;

	poolType pType ;

	vars = parseString3( err, str ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		replace( vars.begin(), vars.end(), ',', ' ' ) ;
		if ( words( vars ) == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
		}
	}
	else
	{
		vars = word( str, 1 ) ;
		str  = subword( str, 2 ) ;
	}

	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else if ( str == "BOTH"    ) { pType = BOTH    ; }
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->verase( vars, pType ) ;
}


void execiVget( pApplication* thisAppl,
		const string& s,
		errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string vars ;

	poolType pType ;

	vars = parseString3( err, str ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		replace( vars.begin(), vars.end(), ',', ' ' ) ;
		if ( words( vars ) == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
		}
	}
	else
	{
		vars = word( str, 1 ) ;
		str  = subword( str, 2 ) ;
	}

	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->vget( vars, pType ) ;
}


void execiView( pApplication* thisAppl,
		const string& s,
		errblock& err )
{
	string str = subword( s, 2 ) ;

	string vw_panel ;
	string vw_macro ;
	string vw_profile ;
	string vw_dataid ;
	string vw_file  ;
	string vw_lcmds ;
	string vw_confc ;
	string vw_chgwarn ;
	string vw_parm ;

	vw_file = parseString1( err, str, "FILE()" ) ;
	if ( err.error() ) { return ; }

	if ( err.RSN4() )
	{
		vw_dataid = parseString1( err, str, "DATAID()" ) ;
		if ( err.error() ) { return ; }
	}

	vw_panel = parseString1( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	vw_macro = parseString1( err, str, "MACRO()" ) ;
	if ( err.error() ) { return ; }

	vw_profile = parseString1( err, str, "PROFILE()" ) ;
	if ( err.error() ) { return ; }

	vw_lcmds = parseString1( err, str, "LINECMDS()" ) ;
	if ( err.error() ) { return ; }

	vw_confc = parseString1( err, str, "CONFIRM()" ) ;
	if ( err.error() ) { return ; }

	vw_chgwarn = parseString1( err, str, "CHGWARN()" ) ;
	if ( err.error() ) { return ; }

	vw_parm = parseString1( err, str, "PARM()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->view( vw_file,
			iupper( vw_panel ),
			vw_macro,
			iupper( vw_profile ),
			iupper( vw_dataid ),
			iupper( vw_lcmds ),
			iupper( vw_confc ),
			iupper( vw_chgwarn ),
			iupper( vw_parm ) ) ;
}


void execiVput( pApplication* thisAppl,
		const string& s,
		errblock& err )
{
	string str = upper( subword( s, 2 ) ) ;

	string vars ;

	poolType pType ;

	vars = parseString3( err, str ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		replace( vars.begin(), vars.end(), ',', ' ' ) ;
		if ( words( vars ) == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
		}
	}
	else
	{
		vars = word( str, 1 ) ;
		str  = subword( str, 2 ) ;
	}

	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return ;
	}

	thisAppl->vput( vars, pType ) ;
}


}
