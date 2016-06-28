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

using namespace std;

int  lspfAddpop( pApplication *, string )   ;
int  lspfBrowse( pApplication *, string )   ;
int  lspfDisplay( pApplication *, string )  ;
int  lspfControl( pApplication *, string )  ;
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
int  lspfTBSarg( pApplication *, string )   ;
int  lspfTBSave( pApplication *, string )   ;
int  lspfTBScan( pApplication *, string )   ;
int  lspfTBSkip( pApplication *, string )   ;
int  lspfTBSort( pApplication *, string )   ;
int  lspfTBTop( pApplication *, string )    ;
int  lspfTBVClear( pApplication *, string ) ;
int  lspfVerase( pApplication *, string )   ;
int  lspfVget( pApplication *, string )     ;
int  lspfVput( pApplication *, string )     ;

map<string, int(*)(pApplication *,string)> lspfServices = { { "ADDPOP",   lspfAddpop   },
							    { "BROWSE",   lspfBrowse   },
							    { "DISPLAY",  lspfDisplay  },
							    { "CONTROL",  lspfControl  },
							    { "EDIT",     lspfEdit     },
							    { "GETMSG",   lspfGetmsg   },
							    { "LIBDEF",   lspfLibdef   },
							    { "PQUERY",   lspfPquery   },
							    { "RDISPLAY", lspfRDisplay },
							    { "REMPOP",   lspfRempop   },
							    { "SELECT",   lspfSelect   },
							    { "SETMSG",   lspfSetmsg   },
							    { "TBADD",    lspfTBAdd    },
							    { "TBBOTTOM", lspfTBBottom },
							    { "TBCREATE", lspfTBCreate },
							    { "TBCCLOSE", lspfTBClose  },
							    { "TBDELETE", lspfTBDelete },
							    { "TBDISPL",  lspfTBDispl  },
							    { "TBEND",    lspfTBEnd    },
							    { "TBERASE",  lspfTBErase  },
							    { "TBEXIST",  lspfTBExist  },
							    { "TBGET",    lspfTBGet    },
							    { "TBMOD",    lspfTBMod    },
							    { "TBPUT",    lspfTBPut    },
							    { "TBOPEN",   lspfTBOpen   },
							    { "TBQUERY",  lspfTBQuery  },
							    { "TBSARG",   lspfTBSarg   },
							    { "TBSAVE",   lspfTBSave   },
							    { "TBSCAN",   lspfTBScan   },
							    { "TBSKIP",   lspfTBSkip   },
							    { "TBSORT",   lspfTBSort   },
							    { "TBTOP",    lspfTBTop    },
							    { "TBVCLEAR", lspfTBVClear },
							    { "VERASE",   lspfVerase   },
							    { "VGET",     lspfVget     },
							    { "VPUT",     lspfVput     } } ;

class POREXX1 : public pApplication
{
	public:
		void application() ;
	private:
};
