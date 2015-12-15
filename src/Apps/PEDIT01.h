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


class icmd
{
	private:
		string icmd_COMMAND  ;
		int    icmd_sURID    ;
		int    icmd_Rpt      ;
		char   icmd_ABO      ;
		int    icmd_OSize    ;
		int    icmd_dURID    ;
		int    icmd_oURID    ;
		bool   icmd_ABO_req  ;
		bool   icmd_overlay  ;
		bool   icmd_cutpaste ;
		int    icmd_eURID    ;
		icmd()
		{
			icmd_COMMAND  = ""    ;
			icmd_sURID    = 0     ;
			icmd_ABO      = ' '   ;
			icmd_OSize    = 0     ;
			icmd_Rpt      = 0     ;
			icmd_dURID    = 0     ;
			icmd_oURID    = 0     ;
			icmd_ABO_req  = false ;
			icmd_overlay  = false ;
			icmd_cutpaste = false ;
			icmd_eURID    = 0     ;
		}
		void icmd_clear()
		{
			icmd_COMMAND  = ""    ;
			icmd_sURID    = 0     ;
			icmd_ABO      = ' '   ;
			icmd_OSize    = 0     ;
			icmd_Rpt      = 0     ;
			icmd_dURID    = 0     ;
			icmd_oURID    = 0     ;
			icmd_ABO_req  = false ;
			icmd_overlay  = false ;
			icmd_cutpaste = false ;
			icmd_eURID    = 0     ;
		}
	friend class PEDIT01 ;
} ;


class idata
{
	private:
		int    id_lvl  ;
		string id_data ;
		idata()
		{
			id_lvl    = 0  ;
			id_data   = "" ;
		}
	friend class iline   ;
	friend class PEDIT01 ;
} ;


class iposition
{
	public:
		int    ipo_line ;
		int    ipo_URID ;
		iposition()
		{
			ipo_line = 0 ;
			ipo_URID = 0 ;
		}
	friend class PEDIT01 ;
} ;



class ichange
{
	private:
		int    iURID   ;
		int    ilvl    ;
		char   iaction ;
		ichange()
		{
			iURID   = 0   ;
			ilvl    = 0   ;
			iaction = ' ' ;
		}

	friend class iline   ;
	friend class PEDIT01 ;
} ;


class ipline
{
	private:
		bool   ip_file ;
		bool   ip_note ;
		bool   ip_prof ;
		bool   ip_col  ;
		bool   ip_bnds ;
		bool   ip_mask ;
		bool   ip_excl ;
		bool   ip_hex  ;
		string ip_data ;
		ipline()
		{
			ip_file = false ;
			ip_note = false ;
			ip_prof = false ;
			ip_col  = false ;
			ip_bnds = false ;
			ip_mask = false ;
			ip_excl = false ;
			ip_hex  = false ;
			ip_data = ""    ;
		}

	friend class PEDIT01 ;
} ;


class iline
{
	private:
		static map<int, stack<ichange> >Global_Undo ;
		static map<int, stack<ichange> >Global_Redo ;
		static map<int, int>maxURID        ;
		static map<int, bool>recoverOFF    ;

		bool   il_file    ;
		bool   il_note    ;
		bool   il_prof    ;
		bool   il_col     ;
		bool   il_bnds    ;
		bool   il_mask    ;
		bool   il_tabs    ;
		bool   il_excl    ;
		bool   il_tod     ;
		bool   il_bod     ;
		bool   il_hex     ;
		bool   il_chg     ;
		bool   il_error   ;
		bool   il_deleted ;
		bool   il_newisrt ;
		string il_label   ;
		string il_lc1     ;
		string il_lc2     ;
		int    il_rept    ;
		int    il_URID    ;
		int    il_taskid  ;
		stack <idata> il_idata      ;
		stack <idata> il_idata_redo ;

		iline( int taskid )
		{
			il_file    = false  ;
			il_note    = false  ;
			il_prof    = false  ;
			il_col     = false  ;
			il_bnds    = false  ;
			il_mask    = false  ;
			il_tabs    = false  ;
			il_excl    = false  ;
			il_tod     = false  ;
			il_bod     = false  ;
			il_hex     = false  ;
			il_chg     = false  ;
			il_error   = false  ;
			il_deleted = false  ;
			il_newisrt = false  ;
			il_label   = ""     ;
			il_lc1     = ""     ;
			il_lc2     = ""     ;
			il_rept    = 0      ;
			il_taskid  = taskid ;
			maxURID[ taskid ] = maxURID[ taskid ] + 1 ;
			il_URID    = maxURID[ taskid ] ;
		}
		void clearLc12()
		{
			il_lc1 = "" ;
			il_lc2 = "" ;
		}
		void put_idata( string s, int lvl )
		{
			idata d   ;
			ichange t ;

			if ( recoverOFF[ il_taskid ] )
			{
				d.id_data = s      ;
				if   ( il_idata.empty() ) { il_idata.push( d ) ; }
				else { il_idata.top() = d ; }
			}
			else
			{
				d.id_lvl = lvl ;
				d.id_data = s  ;
				il_idata.push( d )    ;
				il_deleted = false    ;
				t.iURID = il_URID     ;
				t.ilvl  = lvl         ;
				Global_Undo[ il_taskid ].push( t ) ;
			}
			clear_Global_Redo() ;
		}
		void put_idata( string s )
		{
			idata d ;

			d.id_data = s ;
			if   ( il_idata.empty() ) { il_idata.push( d ) ; }
			else { il_idata.top().id_data = s              ; }
		}
		void set_il_deleted()
		{
			il_deleted = true ;
			if ( !recoverOFF[ il_taskid ] ) { Global_Undo[ il_taskid ].top().iaction = 'D' ; }
		}
		string get_idata()
		{
			return il_idata.top().id_data ;
		}
		void undo_idata()
		{
			if ( recoverOFF[ il_taskid ] ) { return ; }
			if ( !il_idata.empty() )
			{
				if ( Global_Undo[ il_taskid ].top().iaction == 'D' )
				{
					il_deleted = false ;
				}
				il_idata_redo.push( il_idata.top() ) ;
				il_idata.pop()                       ;
				if ( il_idata.empty() )
				{
					il_deleted = true ;
				}
			}
			Global_Redo[ il_taskid ].push ( Global_Undo[ il_taskid ].top() ) ;
			Global_Undo[ il_taskid ].pop() ;
		}
		void redo_idata()
		{
			if ( recoverOFF[ il_taskid ] ) { return ; }
			if ( Global_Redo[ il_taskid ].top().iaction == 'D' )
			{
				il_deleted = true ;
			}
			else
			{
				il_deleted = false  ;
			}
			il_idata.push( il_idata_redo.top() ) ;
			il_idata_redo.pop() ;
			Global_Undo[ il_taskid ].push( Global_Redo[ il_taskid ].top() ) ;
			Global_Redo[ il_taskid ].pop() ;
		}
		void clear_Global_Undo()
		{
			while ( true )
			{
				if ( Global_Undo[ il_taskid ].empty() ) { break ; }
				Global_Undo[ il_taskid ].pop() ;
			}
		}
		void clear_Global_Redo()
		{
			while ( true )
			{
				if ( Global_Redo[ il_taskid ].empty() ) { break ; }
				Global_Redo[ il_taskid ].pop() ;
			}
		}
		ichange get_Undo_URID()
		{
			ichange i ;
			if ( recoverOFF[ il_taskid ] || Global_Undo[ il_taskid ].empty() ) { return i ; }
			return Global_Undo[ il_taskid ].top() ;
		}
		ichange get_Redo_URID()
		{
			ichange i ;
			if ( recoverOFF[ il_taskid ] ) { return i ; }
			if ( Global_Redo[ il_taskid ].empty() ) { return i ; }
			return Global_Redo[ il_taskid ].top() ;
		}
		void flatten_idata()
		{
			idata  d  ;

			d.id_data = il_idata.top().id_data ;
			d.id_lvl  = il_idata.top().id_lvl  ;
			while ( true )
			{
				if ( il_idata.empty() ) { break ; }
				il_idata.pop() ;
			}
			while ( true )
			{
				if ( il_idata_redo.empty() ) { break ; }
				il_idata_redo.pop() ;
			}
			il_idata.push( d ) ;
		}
		int get_idata_lvl()
		{
			return il_idata.top().id_lvl ;
		}
		int get_Global_Undo_Size()
		{
			return Global_Undo[ il_taskid ].size() ;
		}
		int get_Global_Redo_Size()
		{
			return Global_Redo[ il_taskid ].size() ;
		}

	friend class PEDIT01 ;
} ;

class e_find
{
	private:
		string fcx_string  ;
		string fcx_estring ;
		string fcx_cstring ;
		string fcx_rstring ;
		bool   fcx_success ;
		int    fcx_occurs  ;
		int    fcx_URID    ;
		int    fcx_lines   ;
		int    fcx_offset  ;
		char   fcx_excl    ;
		bool   fcx_regreq  ;
		bool   fcx_text    ;
		bool   fcx_asis    ;
		bool   fcx_hex     ;
		bool   fcx_pic     ;
		bool   fcx_ctext   ;
		bool   fcx_casis   ;
		bool   fcx_chex    ;
		bool   fcx_cpic    ;
		bool   fcx_rreg    ;
		string fcx_slab    ;
		string fcx_elab    ;
		int    fcx_scol    ;
		int    fcx_ecol    ;
		int    fcx_oncol   ;
		bool   fcx_change  ;
		bool   fcx_fset    ;
		bool   fcx_cset    ;
		char   fcx_dir     ;
		bool   fcx_chngall ;
		char   fcx_mtch    ;
		char   fcx_prevcmd ;
	e_find()
	{
		fcx_string  = ""    ;
		fcx_estring = ""    ;
		fcx_cstring = ""    ;
		fcx_rstring = ""    ;
		fcx_success = true  ;
		fcx_occurs  = 0     ;
		fcx_URID    = 0     ;
		fcx_lines   = 0     ;
		fcx_offset  = 0     ;
		fcx_excl    = 'A'   ;
		fcx_regreq  = false ;
		fcx_text    = false ;
		fcx_asis    = false ;
		fcx_hex     = false ;
		fcx_pic     = false ;
		fcx_ctext   = false ;
		fcx_casis   = false ;
		fcx_chex    = false ;
		fcx_cpic    = false ;
		fcx_rreg    = false ;
		fcx_slab    = ""    ;
		fcx_elab    = ""    ;
		fcx_scol    = 0     ;
		fcx_ecol    = 0     ;
		fcx_oncol   = 0     ;
		fcx_fset    = false ;
		fcx_cset    = false ;
		fcx_dir     = 'N'   ;
		fcx_chngall = false ;
		fcx_mtch    = 'C'   ;
		fcx_prevcmd = ' '   ;
	}
	friend class PEDIT01 ;
} ;


class PEDIT01 : public pApplication
{
	public:
		void application() ;

	private:
		static map<string,bool>EditList  ;

		void showEditEntry()      ;
		void Edit()               ;
		void getEditProfile( string )  ;
		void saveEditProfile( string ) ;
		void cleanup_custom()     ;
		void initialise()         ;
		bool termOK()             ;
		void read_file()          ;
		bool saveFile()           ;
		void fill_dynamic_area()  ;
		void getZAREAchanges()    ;
		void updateData()         ;

		void actionFind()         ;
		void actionChange()       ;
		void actionExclude()      ;

		bool checkLineCommands()  ;
		void actionLineCommands() ;
		void actionPCMD()         ;

		void actionUNDO()         ;
		void actionREDO()         ;
		void removeRecoveryData() ;

		int  getLine( int )       ;
		int  getLastEX( int )     ;
		int  getFirstEX( int )    ;
		int  getEXBlock( int )    ;
		int  getDataBlock( int )  ;

		int  getFileLine( int )   ;
		int  getDataLine( int )   ;

		vector<iline * >::iterator getValidDataLine( vector<iline * >::iterator ) ;
		uint getValidDataLine( uint ) ;

		int  getNextDataLine( int )  ;
		int  getNextDataLine( uint ) ;
		vector<iline * >::iterator getNextDataLine( vector<iline * >::iterator ) ;
		int  getPrevDataLine( uint ) ;

		int  getRangeSize( int, int ) ;
		int  truncateSize( int ) ;

		bool URIDonScreen( int ) ;

		string overlay( string, string, bool & ) ;
		bool xformLineCmd( string, string &, int & ) ;

		void copyToClipboard( vector<ipline> & vip ) ;
		void clearClipboard( string )  ;
		void getClipboard( vector<ipline> & vip ) ;

		void clearCursor()       ;
		void storeCursor(  int, int, int=0 ) ;
		void placeCursor(  int, int, int=0 ) ;
		void placeCursor( uint, int, int=0 ) ;
		void positionCursor()    ;

		vector<iline * >::iterator getLineItr( int ) ;
		vector<iline * >::iterator getLineBeforeItr( int ) ;

		int  setFindChangeExcl( char ) ;
		int  getNextSpecial( int ) ;
		bool returnLabelItr( string, vector<iline * >::iterator & , int & ) ;

		bool getTabLocation( int & ) ;

		uint topLine             ;
		int  startCol            ;
		int  maxCol              ;
		int  aRow                ;
		int  aCol                ;
		int  aURID               ;
		int  Level               ;

		bool tabsOnRead          ;

		bool cursorPlaced        ;
		bool cursorPlaceHome     ;
		int  cursorPlaceUsing    ;
		int  cursorPlaceURID     ;
		int  cursorPlaceRow      ;
		int  cursorPlaceType     ;
		int  cursorPlaceOff      ;

		bool profSave            ;
		bool profNulls           ;
		bool profLock            ;
		bool profCaps            ;
		bool profHex             ;
		bool profTabs            ;
		int  profTabz            ;
		bool profBackup          ;
		bool profRecov           ;
		bool profHilight         ;

		bool stripST             ;
		bool convTabs            ;
		bool convSpaces          ;
		bool cutActive           ;
		bool cutReplace          ;
		bool pasteActive         ;
		bool pasteKeep           ;

		int  LeftBnd             ;
		int  RightBnd            ;

		string maskLine          ;
		string tabsLine          ;
		char   tabsChar          ;
		string backupLoc         ;

		vector<iline *> data     ;
		map<int, iposition> s2data ;
		map<int, bool> sChanged  ;
		map<int, bool> sTouched  ;
		map<bool, string>OnOff   ;
		map<bool, char>ZeroOne   ;
		vector<icmd> icmds       ;

		map< char, string > typList ;

		e_find find_parms  ;

		bool rebuildZAREA  ;
		bool rebuildShadow ;
		bool fileChanged   ;

		string CURFLD  ;
		int    CURPOS  ;
		string MSG     ;
		string MSG1    ;
		string ZCMD1   ;
		string OCMD    ;

		string ZFILE   ;
		string ZFILENM ;
		string ZROW1   ;
		string ZROW2   ;
		string ZCOL1   ;
		string ZAREA   ;
		string ZSHADOW ;
		string ZAREAT  ;
		int    ZAREAW  ;
		int    ZAREAD  ;
		int    ZDATAW  ;
		int    ZASIZE  ;
		string CAREA   ;
		string CSHADOW ;
		string ZLINES  ;

		string ZEDPROF  ;

		string ZEDPTYPE ;
		string ZEDFLAG  ;
		string ZEDMASK  ;
		string ZEDBNDL  ;
		string ZEDBNDR  ;
		string ZEDTABC  ;
		string ZEDTABS  ;
		string ZEDTABZ  ;
		string ZEDBKLC  ;

		string fileType  ;
		string clipboard ;
		string CLIPTABL  ;

		string sdg  ;
		string sdy  ;
		string sdyh ;
		string sdw  ;
		string sdr  ;
		string sdrh ;
		string sdb  ;

		string div  ;

		string slg  ;
		string sly  ;
		string slyh ;
		string slw  ;
		string slr  ;
		string slrh ;
		string slb  ;

		string TYPE ;
		string STR  ;
		string OCC  ;
		string LINES;

		const string blkcmds   = "CC MM DD HXX OO RR XX (( )) UCC LCC MMD" ;
		const string sglcmds   = "A B BNDS C COL COLS D F HX I L LC M MASK MD O R S TABS TS UC X ( )" ;
		const string spllcmds  = "COL COLS A B I C M D R CC MM DD RR" ;
		const string todlcmds  = "COL COLS A I BNDS MASK TABS" ;
		const string bodlcmds  = "B" ;
		const string ABOReq    = "CC MM C M" ;
		const string ABOList   = "A B O" ;
		const string ReptOK    = "C M D HX I MD X O R UC LC RR (( )) ( ) F L" ;
		const string CutCmds   = "C CC M MM" ;
		const string PasteCmds = "A B" ;

} ;


