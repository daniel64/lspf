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

using namespace std ;

enum P_CMDS
{
	PC_AUTOSAVE,
	PC_BOUNDS,
	PC_BROWSE,
	PC_CHANGE,
	PC_CAPS,
	PC_COLUMN,
	PC_COMPARE,
	PC_CREATE,
	PC_CUT,
	PC_DELETE,
	PC_EDIT,
	PC_EXCLUDE,
	PC_FIND,
	PC_FLIP,
	PC_HEX,
	PC_HIDE,
	PC_HILIGHT,
	PC_LOCATE,
	PC_NULLS,
	PC_PASTE,
	PC_PROFILE,
	PC_PRESERVE,
	PC_RECOVERY,
	PC_REDO,
	PC_REPLACE,
	PC_RESET,
	PC_SAVE,
	PC_SETUNDO,
	PC_SORT,
	PC_TABS,
	PC_UNDO,
	PC_XTABS
} ;

enum L_CMDS
{
	LC_A,
	LC_AK,
	LC_B,
	LC_BK,
	LC_BOUNDS,
	LC_C,
	LC_CC,
	LC_CHANGE,
	LC_COLS,
	LC_D,
	LC_DD,
	LC_F,
	LC_HX,
	LC_HXX,
	LC_I,
	LC_L,
	LC_LC,
	LC_LCC,
	LC_M,
	LC_MM,
	LC_MASK,
	LC_MD,
	LC_MDD,
	LC_MN,
	LC_MNN,
	LC_O,
	LC_OK,
	LC_OO,
	LC_OOK,
	LC_PASTE,
	LC_R,
	LC_RR,
	LC_S,
	LC_SI,
	LC_SLC,
	LC_SLCC,
	LC_SLD,
	LC_SLDD,
	LC_SLTC,
	LC_SLTCC,
	LC_SRC,
	LC_SRCC,
	LC_SRD,
	LC_SRDD,
	LC_SRTC,
	LC_SRTCC,
	LC_TABS,
	LC_TJ,
	LC_TJJ,
	LC_TR,
	LC_TRR,
	LC_TS,
	LC_T,
	LC_TT,
	LC_TX,
	LC_TXX,
	LC_UC,
	LC_UCC,
	LC_W,
	LC_WW,
	LC_X,
	LC_XX,
	LC_XI
} ;

class icmd
{
	private:
		L_CMDS icmd_CMD     ;
		string icmd_CMDSTR  ;
		int    icmd_sURID   ;
		int    icmd_eURID   ;
		int    icmd_dURID   ;
		int    icmd_lURID   ;
		int    icmd_Rpt     ;
		char   icmd_ABOW    ;
		bool   icmd_swap    ;
		bool   icmd_cut     ;
		bool   icmd_paste   ;
		bool   icmd_create  ;
		icmd()
		{
			icmd_CMDSTR  = " "   ;
			icmd_sURID   = 0     ;
			icmd_eURID   = 0     ;
			icmd_dURID   = 0     ;
			icmd_lURID   = 0     ;
			icmd_ABOW    = ' '   ;
			icmd_Rpt     = 0     ;
			icmd_swap    = false ;
			icmd_cut     = false ;
			icmd_paste   = false ;
			icmd_create  = false ;
		}
		void icmd_clear()
		{
			icmd_CMDSTR  = " "   ;
			icmd_sURID   = 0     ;
			icmd_eURID   = 0     ;
			icmd_dURID   = 0     ;
			icmd_lURID   = 0     ;
			icmd_ABOW    = ' '   ;
			icmd_Rpt     = 0     ;
			icmd_swap    = false ;
			icmd_cut     = false ;
			icmd_paste   = false ;
			icmd_create  = false ;
		}
	friend class PEDIT01 ;
} ;


class idata
{
	private:
		int    id_level   ;
		bool   id_deleted ;
		string id_data    ;
		idata()
		{
			id_level   = 0     ;
			id_deleted = false ;
			id_data    = ""    ;
		}
	friend class iline ;
} ;


class ipos
{
	public:
		uint   ipos_dl   ;
		uint   ipos_hex  ;
		int    ipos_URID ;
		ipos()
		{
			ipos_dl   = 0 ;
			ipos_hex  = 0 ;
			ipos_URID = 0 ;
		}
	void clear()
		{
			ipos_dl   = 0 ;
			ipos_hex  = 0 ;
			ipos_URID = 0 ;
		}
} ;


class ipline
{
	private:
		bool   ip_file   ;
		bool   ip_note   ;
		bool   ip_prof   ;
		bool   ip_col    ;
		bool   ip_bnds   ;
		bool   ip_mask   ;
		bool   ip_tabs   ;
		bool   ip_excl   ;
		bool   ip_hex    ;
		bool   ip_chg    ;
		bool   ip_error  ;
		bool   ip_undo   ;
		bool   ip_redo   ;
		bool   ip_msg    ;
		bool   ip_info   ;
		bool   ip_nisrt  ;
		int    ip_profln ;
		string ip_data   ;
		ipline()
		{
			ip_file   = false ;
			ip_note   = false ;
			ip_prof   = false ;
			ip_col    = false ;
			ip_bnds   = false ;
			ip_mask   = false ;
			ip_tabs   = false ;
			ip_excl   = false ;
			ip_hex    = false ;
			ip_chg    = false ;
			ip_error  = false ;
			ip_undo   = false ;
			ip_redo   = false ;
			ip_msg    = false ;
			ip_info   = false ;
			ip_nisrt  = false ;
			ip_profln = 0     ;
			ip_data   = ""    ;
		}

	friend class PEDIT01 ;
} ;


class iline
{
	private:
		static map<int, stack<int>>Global_Undo ;
		static map<int, stack<int>>Global_Redo ;
		static map<int, stack<int>>Global_File_level ;
		static map<int, int>maxURID  ;
		static map<int, bool>setUNDO ;

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
		bool   il_undo    ;
		bool   il_redo    ;
		bool   il_mark    ;
		bool   il_msg     ;
		bool   il_info    ;
		bool   il_deleted ;
		bool   il_nisrt   ;
		string il_label   ;
		string il_lcc     ;
		int    il_profln  ;
		int    il_rept    ;
		int    il_URID    ;
		int    il_taskid  ;
		string il_Shadow  ;
		bool   il_vShadow ;
		bool   il_wShadow ;
		stack <idata> il_idata      ;
		stack <idata> il_idata_redo ;

		iline( int taskid )
		{
			il_file    = false  ;
			il_tod     = false  ;
			il_bod     = false  ;
			il_note    = false  ;
			il_prof    = false  ;
			il_col     = false  ;
			il_bnds    = false  ;
			il_mask    = false  ;
			il_tabs    = false  ;
			il_info    = false  ;
			il_msg     = false  ;
			il_excl    = false  ;
			il_hex     = false  ;
			il_chg     = false  ;
			il_error   = false  ;
			il_undo    = false  ;
			il_redo    = false  ;
			il_mark    = false  ;
			il_deleted = false  ;
			il_nisrt   = false  ;
			il_label   = ""     ;
			il_lcc     = ""     ;
			il_rept    = 0      ;
			il_profln  = 0      ;
			il_taskid  = taskid ;
			il_URID    = ++maxURID[ taskid ] ;
			il_Shadow  = ""     ;
			il_vShadow = false  ;
			il_wShadow = false  ;
		}
		void resetFilePrefix()
		{
			il_excl  = false ;
			il_chg   = false ;
			il_error = false ;
			il_undo  = false ;
			il_redo  = false ;
			il_mark  = false ;
		}
		void resetFileStatus()
		{
			il_chg   = false ;
			il_error = false ;
			il_undo  = false ;
			il_redo  = false ;
		}
		void setChngStatus()
		{
			il_chg   = true  ;
			il_error = false ;
			il_undo  = false ;
			il_redo  = false ;
		}
		void setErrorStatus()
		{
			il_chg   = false ;
			il_error = true  ;
			il_undo  = false ;
			il_redo  = false ;
		}
		void setUndoStatus()
		{
			il_chg   = false ;
			il_error = false ;
			il_undo  = true  ;
			il_redo  = false ;
		}
		void setRedoStatus()
		{
			il_chg   = false ;
			il_error = false ;
			il_undo  = false ;
			il_redo  = true  ;
		}
		void resetMarked()
		{
			il_mark = false ;
		}
		void toggleMarked()
		{
			il_mark = !il_mark ;
		}
		bool isValidFile()
		{
			return il_file && !il_deleted ;
		}
		bool isSpecial()
		{
			return il_note || il_prof || il_col  || il_bnds ||
			       il_mask || il_tabs || il_info || il_msg  ;
		}
		void clearLcc()
		{
			il_lcc = "" ;
		}
		void put_idata( const string& s, int level )
		{
			idata d ;

			if ( !setUNDO[ il_taskid ] )
			{
				d.id_data = s ;
				if ( il_idata.empty() ) { il_idata.push( d ) ; }
				else                    { il_idata.top() = d ; }
			}
			else
			{
				d.id_level = level ;
				d.id_data  = s     ;
				il_idata.push( d ) ;
				if ( Global_Undo[ il_taskid ].empty() ||
				     Global_Undo[ il_taskid ].top() != level )
				{
					Global_Undo[ il_taskid ].push( level ) ;
				}
				if ( il_file )
				{
					set_Global_File_level() ;
				}
			}
			il_vShadow = false  ;
			clear_Global_Redo() ;
			remove_redo_idata() ;
		}
		void put_idata( const string& s )
		{
			idata d ;

			if ( il_idata.empty() ) { d.id_data = s ; il_idata.push( d ) ; }
			else                    { il_idata.top().id_data = s         ; }
			il_vShadow = false  ;
		}
		void set_idata_minsize( int i )
		{
			if ( il_idata.top().id_data.size() < i )
			{
				il_idata.top().id_data.resize( i, ' ' ) ;
			}
		}
		void set_idata_upper( int Level )
		{
			put_idata( upper( il_idata.top().id_data ), Level ) ;
		}
		void set_idata_lower( int Level )
		{
			put_idata( lower( il_idata.top().id_data ), Level ) ;
		}
		bool set_idata_trim( int Level )
		{
			if ( il_idata.top().id_data.back() == ' ')
			{
				put_idata( strip( il_idata.top().id_data, 'T', ' '), Level ) ;
				return true ;
			}
			else { return false ; }
		}
		void set_idata_level( int level )
		{
			il_idata.top().id_level = level ;
			if ( Global_Undo[ il_taskid ].empty()     ||
			     Global_Undo[ il_taskid ].top() != level )
			{
				Global_Undo[ il_taskid ].push( level ) ;
			}
			if ( il_file )
			{
				set_Global_File_level() ;
			}
			clear_Global_Redo() ;
			remove_redo_idata() ;
		}
		void set_deleted( int Level )
		{
			if ( il_deleted ) { return ; }
			put_idata( "", Level ) ;
			il_idata.top().id_deleted = true ;
			il_deleted                = true ;
		}
		string get_idata()
		{
			return il_idata.top().id_data ;
		}
		string::const_iterator get_idata_begin()
		{
			return il_idata.top().id_data.begin() ;
		}
		string * get_idata_ptr()
		{
			return &il_idata.top().id_data ;
		}
		bool idata_is_empty()
		{
			return il_idata.empty() ;
		}
		void undo_idata()
		{
			if ( !setUNDO[ il_taskid ] ) { return ; }
			if ( !il_idata.empty() )
			{
				if ( il_idata.top().id_deleted )
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
		}
		void redo_idata()
		{
			if ( !setUNDO[ il_taskid ] ) { return ; }
			il_deleted = il_idata_redo.top().id_deleted ;
			il_idata.push( il_idata_redo.top() ) ;
			il_idata_redo.pop() ;
		}
		void clear_Global_Undo()
		{
			while ( !Global_Undo[ il_taskid ].empty() )
			{
				Global_Undo[ il_taskid ].pop() ;
			}
		}
		void clear_Global_Redo()
		{
			while ( !Global_Redo[ il_taskid ].empty() )
			{
				Global_Redo[ il_taskid ].pop() ;
			}
		}
		void reset_Global_Undo()
		{
			Global_Undo[ il_taskid ] = Global_File_level[ il_taskid ] ;
		}
		void clear_Global_File_level()
		{
			while ( !Global_File_level[ il_taskid ].empty() )
			{
				Global_File_level[ il_taskid ].pop() ;
			}
		}
		int get_Global_File_level()
		{
			if ( Global_File_level[ il_taskid ].empty() )
			{
				return 0 ;
			}
			else
			{
				return Global_File_level[ il_taskid ].top() ;
			}
		}
		void set_Global_File_level()
		{
			if ( Global_File_level[ il_taskid ].empty() ||
			     Global_File_level[ il_taskid ].top() != Global_Undo[ il_taskid ].top() )
			{
				Global_File_level[ il_taskid ].push( Global_Undo[ il_taskid ].top() ) ;
			}
		}
		void remove_Global_File_level()
		{
			Global_File_level[ il_taskid ].pop() ;
		}
		int get_Global_Undo_level()
		{
			if (  !setUNDO[ il_taskid ] ||
			     Global_Undo[ il_taskid ].empty() ) { return 0 ; }
			return Global_Undo[ il_taskid ].top() ;
		}
		int get_Global_Redo_level()
		{
			if (  !setUNDO[ il_taskid ] ||
			     Global_Redo[ il_taskid ].empty() ) { return 0 ; }
			return Global_Redo[ il_taskid ].top() ;
		}
		void move_Global_Undo2Redo()
		{
			if ( !setUNDO[ il_taskid ] ) { return ; }
			Global_Redo[ il_taskid ].push ( Global_Undo[ il_taskid ].top() ) ;
			Global_Undo[ il_taskid ].pop() ;
		}
		void move_Global_Redo2Undo()
		{
			if ( !setUNDO[ il_taskid ] ) { return ; }
			Global_Undo[ il_taskid ].push( Global_Redo[ il_taskid ].top() ) ;
			Global_Redo[ il_taskid ].pop() ;
		}
		void flatten_idata()
		{
			idata d ;

			d = il_idata.top() ;
			while ( !il_idata.empty() )
			{
				il_idata.pop() ;
			}
			while ( !il_idata_redo.empty() )
			{
				il_idata_redo.pop() ;
			}
			il_idata.push( d ) ;
		}
		void remove_redo_idata()
		{
			while ( !il_idata_redo.empty() )
			{
				il_idata_redo.pop() ;
			}
		}
		int get_idata_level()
		{
			if ( il_idata.empty() ) { return 0 ; }
			return il_idata.top().id_level ;
		}
		int get_idata_Redo_level()
		{
			if ( il_idata_redo.empty() ) { return 0 ; }
			return il_idata_redo.top().id_level ;
		}
		int get_Global_Undo_Size()
		{
			return Global_Undo[ il_taskid ].size() ;
		}
		int get_Global_Redo_Size()
		{
			return Global_Redo[ il_taskid ].size() ;
		}
		int get_Global_File_Size()
		{
			return Global_File_level[ il_taskid ].size() ;
		}

	friend class PEDIT01 ;
} ;


class c_range
{
	private:
		bool   c_vlab  ;
		bool   c_vcol  ;
		string c_slab  ;
		string c_elab  ;
		uint   c_sidx  ;
		uint   c_eidx  ;
		int    c_scol  ;
		int    c_ecol  ;
		bool   c_ocol  ;
		string c_rest  ;
	c_range()
	{
		c_vlab  = false ;
		c_vcol  = false ;
		c_slab  = ""    ;
		c_elab  = ""    ;
		c_sidx  = -1    ;
		c_eidx  = -1    ;
		c_scol  = 0     ;
		c_ecol  = 0     ;
		c_ocol  = false ;
		c_rest  = ""    ;
	}
	void c_range_clear()
	{
		c_slab  = ""    ;
		c_elab  = ""    ;
		c_sidx  = -1    ;
		c_eidx  = -1    ;
		c_scol  = 0     ;
		c_ecol  = 0     ;
		c_ocol  = false ;
		c_rest  = ""    ;
	}
	friend class PEDIT01 ;
} ;


class e_find
{
	private:
		string fcx_string  ;
		string fcx_ostring ;
		string fcx_cstring ;
		string fcx_rstring ;
		bool   fcx_success ;
		bool   fcx_error   ;
		bool   fcx_top     ;
		bool   fcx_bottom  ;
		bool   fcx_exclude ;
		char   fcx_dir     ;
		char   fcx_mtch    ;
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
		bool   fcx_reg     ;
		bool   fcx_ctext   ;
		bool   fcx_casis   ;
		bool   fcx_chex    ;
		bool   fcx_cpic    ;
		bool   fcx_creg    ;
		string fcx_slab    ;
		string fcx_elab    ;
		int    fcx_scol    ;
		int    fcx_ecol    ;
		int    fcx_ocol    ;
		bool   fcx_fset    ;
		bool   fcx_cset    ;
		bool   fcx_chngall ;
	e_find()
	{
		fcx_string  = ""    ;
		fcx_ostring = ""    ;
		fcx_cstring = ""    ;
		fcx_rstring = ""    ;
		fcx_success = true  ;
		fcx_error   = false ;
		fcx_top     = false ;
		fcx_bottom  = false ;
		fcx_exclude = false ;
		fcx_dir     = 'N'   ;
		fcx_mtch    = 'C'   ;
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
		fcx_reg     = false ;
		fcx_ctext   = false ;
		fcx_casis   = false ;
		fcx_chex    = false ;
		fcx_cpic    = false ;
		fcx_creg    = false ;
		fcx_slab    = ""    ;
		fcx_elab    = ""    ;
		fcx_scol    = 0     ;
		fcx_ecol    = 0     ;
		fcx_ocol    = 0     ;
		fcx_fset    = false ;
		fcx_cset    = false ;
		fcx_chngall = false ;
	}
	friend class PEDIT01 ;
} ;


class PEDIT01 : public pApplication
{
	public:
		void application() ;

	private:
		static map<string,bool>EditList  ;
		static e_find Global_efind_parms ;

		void showEditEntry()      ;
		void showEditRecovery()   ;
		void Edit()               ;
		void getEditProfile( const string& )  ;
		void delEditProfile( const string& )  ;
		void saveEditProfile( const string& ) ;
		void createEditProfile( const string&, const string& ) ;
		void resetEditProfile()   ;
		void cleanup_custom()     ;
		void initialise()         ;
		bool termOK()             ;
		void readFile()           ;
		bool saveFile()           ;
		void fill_dynamic_area()  ;
		void fill_hilight_shadow();
		void clr_hilight_shadow() ;
		void protNonDisplayChars();
		void convNonDisplayChars( string& ) ;
		void getZAREAchanges()    ;
		void updateData()         ;

		void actionFind()         ;
		void actionChange()       ;

		bool checkLineCommands()  ;
		void actionPrimCommand1() ;
		void actionPrimCommand2() ;
		void actionLineCommands() ;
		void actionLineCommand( vector<icmd>::iterator ) ;

		void actionZVERB()        ;
		void setLineLabels()      ;

		bool actionUNDO()         ;
		bool actionREDO()         ;
		void removeRecoveryData() ;

		uint getLine( int )       ;
		uint getFirstEX( uint )   ;
		int  getLastEX( vector<iline *>::iterator ) ;
		uint getLastEX( uint )    ;
		int  getExBlockSize( uint )   ;
		int  getDataBlockSize( uint ) ;
		int  getLastURID( vector<iline *>::iterator, int ) ;

		int  getFileLine( uint ) ;
		uint getDataLine( int )  ;

		vector<iline * >::iterator getValidDataLine( vector<iline * >::iterator ) ;
		uint getValidDataFileLine( uint ) ;

		uint getNextDataLine( uint ) ;
		vector<iline * >::iterator getNextDataLine( vector<iline * >::iterator ) ;
		uint getNextDataFileLine( uint ) ;
		vector<iline * >::iterator getNextDataFileLine( vector<iline * >::iterator ) ;

		uint getPrevDataLine( uint ) ;
		uint getPrevDataFileLine( uint ) ;
		vector<iline * >::iterator getPrevDataFileLine( vector<iline * >::iterator ) ;

		void cleanupData()        ;
		void updateProfLines( vector<string>& ) ;
		void buildProfLines( vector<string>& )  ;
		void removeProfLines( )    ;
		void removeSpecialLines( vector<iline * >::iterator, vector<iline * >::iterator ) ;
		void processNISRTlines()  ;

		string removeTabs( string ) ;

		void copyFileData( vector<string> &, int, int  )  ;

		bool URIDonScreen( int ) ;

		string overlay( string, string, bool& ) ;
		bool formLineCmd( const string&, string&, int& ) ;

		void copyToClipboard( vector<ipline>& vip ) ;
		void getClipboard( vector<ipline>& vip ) ;
		void clearClipboard( string ) ;

		void createFile( uint, uint ) ;

		void clearCursor() ;
		void storeCursor(  int, int=0 ) ;
		void placeCursor(  int, int, int=0 ) ;
		void placeCursor( uint, int, int=0 ) ;
		void restoreCursor()   ;
		void positionCursor()  ;
		void moveColumn( int ) ;

		vector<iline * >::iterator getLineItr( int )       ;
		vector<iline * >::iterator getLineItr( uint )      ;
		vector<iline * >::iterator getLineItr( int, vector<iline *>::iterator ) ;

		bool setFindChangeExcl( char ) ;
		void setNotFoundMsg()          ;
		bool setCommandRange( string, c_range& ) ;
		int  getNextSpecial( int, int, char, char ) ;
		bool getLabelItr( const string&, vector<iline * >::iterator &, int& ) ;
		int  getLabelLine( const string& ) ;
		bool checkLabel( const string& ) ;

		bool getTabLocation( int& ) ;
		void copyPrefix( ipline &, iline *& ) ;
		void copyPrefix( iline * &,ipline& )  ;
		void addSpecial( char, int, vector<string>& ) ;
		void addSpecial( char, int, string& ) ;

		string rshiftCols( int, const string * ) ;
		string lshiftCols( int, const string * ) ;
		bool   rshiftData( int, const string *, string& ) ;
		bool   lshiftData( int, const string *, string& ) ;
		bool   textSplitData( const string&, string&, string& ) ;

		void   compareFiles( const string& ) ;
		string determineLang()   ;

		uint topLine             ;
		uint ptopLine            ;
		int  startCol            ;
		int  maxCol              ;
		int  aRow                ;
		int  aCol                ;
		int  aURID               ;
		bool aLCMD               ;
		int  Level               ;
		int  saveLevel           ;
		int  XTabz               ;

		bool tabsOnRead          ;
		bool abendRecovery       ;
		bool abendComplete       ;

		bool cursorPlaceHome     ;
		int  cursorPlaceUsing    ;
		int  cursorPlaceURID     ;
		int  cursorPlaceURIDO    ;
		int  cursorPlaceRow      ;
		int  cursorPlaceType     ;
		int  cursorPlaceOff      ;
		int  cursorPlaceOffO     ;

		bool   profASave         ;
		bool   profSaveP         ;
		bool   profNulla         ;
		bool   profNulls         ;
		bool   profLock          ;
		bool   profCaps          ;
		bool   profHex           ;
		bool   profTabs          ;
		bool   profATabs         ;
		bool   profXTabs         ;
		int    profXTabz         ;
		bool   profRecover       ;
		bool   profHilight       ;
		bool   profIfLogic       ;
		bool   profDoLogic       ;
		bool   profLogic         ;
		bool   profParen         ;
		string profLang          ;

		string detLang           ;
		string creFile           ;

		bool stripST             ;
		bool convTabs            ;
		bool convSpaces          ;
		bool creActive           ;
		bool cutActive           ;
		bool cutReplace          ;
		bool pasteActive         ;
		bool pasteKeep           ;
		bool sPreserve           ;
		bool hideExcl            ;

		int  LeftBnd             ;
		int  RightBnd            ;

		string maskLine          ;
		bool   colsOn            ;
		string tabsLine          ;
		char   tabsChar          ;
		string recoverLoc        ;

		vector<iline *> data     ;
		map<int, ipos> s2data    ;
		map<int, bool> sChanged  ;
		map<int, bool> sTouched  ;
		map<int, int>lchar       ;
		vector<icmd> icmds       ;


		e_find find_parms  ;

		bool rebuildZAREA  ;
		bool rebuildShadow ;
		bool fileChanged   ;

		hilight hlight ;

		string CURFLD  ;
		int    CURPOS  ;
		string MSG     ;
		string MSG1    ;
		string ZCMD1   ;
		string OCMD    ;

		string ZFILE   ;
		string ZCOL1   ;
		string ZCOL2   ;
		string ZAREA   ;
		string ZSHADOW ;
		string ZAREAT  ;
		int    ZAREAW  ;
		int    ZAREAD  ;
		int    ZDATAW  ;
		int    ZASIZE  ;
		string CAREA   ;
		string CSHADOW ;
		string XAREA   ;

		string ZEDPROF  ;
		string ZEDPROFT ;

		string ZEDPTYPE ;
		string ZEDPFLAG ;
		string ZEDPMASK ;
		string ZEDPBNDL ;
		string ZEDPBNDR ;
		string ZEDPTABC ;
		string ZEDPTABS ;
		string ZEDPTABZ ;
		string ZEDPRCLC ;
		string ZEDPHLLG ;

		string EETABCC  ;
		string EETABSS  ;
		string EEPRSPS  ;

		string fileType  ;
		string clipBoard ;
		string clipTable ;

		string sdg  ;
		string sdy  ;
		string sdyh ;
		string sdw  ;
		string sdr  ;
		string sdrh ;
		string sdb  ;
		string sdmk ;

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

		map<char, string>typList    = { { 'C', "CHARS"  },
						{ 'P', "PREFIX" },
						{ 'S', "SUFFIX" },
						{ 'W', "WORD"   } } ;

		map<bool, string>OnOff      = { { true,  "ON"  },
						{ false, "OFF" } } ;

		map<bool, char>ZeroOne      = { { true,  '1' },
						{ false, '0' } } ;

		map<string,P_CMDS> PrimCMDS = { { "AUTOSAVE", PC_AUTOSAVE },
						{ "BND",      PC_BOUNDS   },
						{ "BNDS",     PC_BOUNDS   },
						{ "BOU",      PC_BOUNDS   },
						{ "BOUND",    PC_BOUNDS   },
						{ "BOUNDS",   PC_BOUNDS   },
						{ "BROWSE",   PC_BROWSE   },
						{ "CHANGE",   PC_CHANGE   },
						{ "C",        PC_CHANGE   },
						{ "CHA",      PC_CHANGE   },
						{ "CHG",      PC_CHANGE   },
						{ "CAPS",     PC_CAPS     },
						{ "COLUMN",   PC_COLUMN   },
						{ "COL",      PC_COLUMN   },
						{ "COLS",     PC_COLUMN   },
						{ "COMPARE",  PC_COMPARE  },
						{ "COMP",     PC_COMPARE  },
						{ "CREATE",   PC_CREATE   },
						{ "CRE",      PC_CREATE   },
						{ "CUT",      PC_CUT      },
						{ "DELETE",   PC_DELETE   },
						{ "DEL",      PC_DELETE   },
						{ "EDIT",     PC_EDIT     },
						{ "EXLUDE",   PC_EXCLUDE  },
						{ "EXLUDED",  PC_EXCLUDE  },
						{ "EX",       PC_EXCLUDE  },
						{ "EXC",      PC_EXCLUDE  },
						{ "X",        PC_EXCLUDE  },
						{ "FIND",     PC_FIND     },
						{ "F",        PC_FIND     },
						{ "FLIP",     PC_FLIP     },
						{ "HEX",      PC_HEX      },
						{ "HIDE",     PC_HIDE     },
						{ "HILIGHT",  PC_HILIGHT  },
						{ "HI",       PC_HILIGHT  },
						{ "HILITE",   PC_HILIGHT  },
						{ "LOCATE",   PC_LOCATE   },
						{ "LOC",      PC_LOCATE   },
						{ "L",        PC_LOCATE   },
						{ "NULLS",    PC_NULLS    },
						{ "NULL",     PC_NULLS    },
						{ "PASTE",    PC_PASTE    },
						{ "PRESERVE", PC_PRESERVE },
						{ "PROFILE",  PC_PROFILE  },
						{ "PROF",     PC_PROFILE  },
						{ "RECVR",    PC_RECOVERY },
						{ "RECVRY",   PC_RECOVERY },
						{ "RECOV",    PC_RECOVERY },
						{ "RECOVRY",  PC_RECOVERY },
						{ "RECOVER",  PC_RECOVERY },
						{ "RECOVERY", PC_RECOVERY },
						{ "REDO",     PC_REDO     },
						{ "REPLACE",  PC_CREATE   },
						{ "REP",      PC_CREATE   },
						{ "REPL",     PC_CREATE   },
						{ "RESET",    PC_RESET    },
						{ "RES",      PC_RESET    },
						{ "SAVE",     PC_SAVE     },
						{ "SETUNDO",  PC_SETUNDO  },
						{ "SETU",     PC_SETUNDO  },
						{ "SORT",     PC_SORT     },
						{ "TABS",     PC_TABS     },
						{ "UNDO",     PC_UNDO     },
						{ "XTABS",    PC_XTABS    } } ;

		map<string,L_CMDS> LineCMDS = { { "A",        LC_A        },
						{ "AK",       LC_AK       },
						{ "B",        LC_B        },
						{ "BK",       LC_BK       },
						{ "BNDS",     LC_BOUNDS   },
						{ "C",        LC_C        },
						{ "CC",       LC_CC       },
						{ "COLS",     LC_COLS     },
						{ "D",        LC_D        },
						{ "DD",       LC_DD       },
						{ "F",        LC_F        },
						{ "HX",       LC_HX       },
						{ "HXX",      LC_HXX      },
						{ "I",        LC_I        },
						{ "L",        LC_L        },
						{ "LC",       LC_LC       },
						{ "LCC",      LC_LCC      },
						{ "M",        LC_M        },
						{ "MM",       LC_MM       },
						{ "MASK",     LC_MASK     },
						{ "MD",       LC_MD       },
						{ "MDD",      LC_MDD      },
						{ "MN",       LC_MN       },
						{ "MNN",      LC_MNN      },
						{ "O",        LC_O        },
						{ "OK",       LC_OK       },
						{ "OO",       LC_OO       },
						{ "OOK",      LC_OOK      },
						{ "R",        LC_R        },
						{ "RR",       LC_RR       },
						{ "S",        LC_S        },
						{ "SI",       LC_SI       },
						{ "TABS",     LC_TABS     },
						{ "TJ",       LC_TJ       },
						{ "TJJ",      LC_TJJ      },
						{ "TR",       LC_TR       },
						{ "TRR",      LC_TRR      },
						{ "TS",       LC_TS       },
						{ "T",        LC_T        },
						{ "TT",       LC_TT       },
						{ "TX",       LC_TX       },
						{ "TXX",      LC_TXX      },
						{ "UC",       LC_UC       },
						{ "UCC",      LC_UCC      },
						{ "W",        LC_W        },
						{ "WW",       LC_WW       },
						{ "X",        LC_X        },
						{ "XX",       LC_XX       },
						{ "XI",       LC_XI       },
						{ ")",        LC_SRC      },
						{ "))",       LC_SRCC     },
						{ "(",        LC_SLC      },
						{ "((",       LC_SLCC     },
						{ ">",        LC_SRD      },
						{ ">>",       LC_SRDD     },
						{ "<",        LC_SLD      },
						{ "<<",       LC_SLDD     },
						{ "]",        LC_SRTC     },
						{ "]]",       LC_SRTCC    },
						{ "[",        LC_SLTC     },
						{ "[[",       LC_SLTCC    } } ;

		map<string,string> aliasLCMDS = { { "COL",    "COLS" },
						  { "BND",    "BNDS" },
						  { "BOU",    "BNDS" },
						  { "BOUND",  "BNDS" },
						  { "BOUNDS", "BNDS" },
						  { "LLC",    "LCC"  },
						  { "LCLC",   "LCC"  },
						  { "HHX",    "HXX"  },
						  { "HXHX",   "HXX"  },
						  { "MMD",    "MDD"  },
						  { "MDMD",   "MDD"  },
						  { "MMN",    "MNN"  },
						  { "MNMN",   "MNN"  },
						  { "TAB",    "TABS" },
						  { "TTJ",    "TJJ"  },
						  { "TJTJ",   "TJJ"  },
						  { "TTR",    "TRR"  },
						  { "TRTR",   "TRR"  },
						  { "TTX",    "TXX"  },
						  { "TXTX",   "TXX"  },
						  { "UUC",    "UCC"  },
						  { "UCUC",   "UCC"  } } ;

		map<string,string> aliasNames = { { "CHG",      "CHA"   },
						  { "CHANGE",   "CHA"   },
						  { "CMDS",     "CMD"   },
						  { "COM",      "CMD"   },
						  { "COMMAND",  "CMD"   },
						  { "COMMANDS", "CMD"   },
						  { "ERROR",    "ERR"   },
						  { "ERRORS",   "ERR"   },
						  { "EX",       "X"     },
						  { "EXC",      "X"     },
						  { "EXCLUDE",  "X"     },
						  { "EXCLUDED", "X"     },
						  { "INFOLINE", "INFO"  },
						  { "HIDE",     "H"     },
						  { "LABEL",    "LAB"   },
						  { "LABELS",   "LAB"   },
						  { "MRK",      "T"     },
						  { "MARK",     "T"     },
						  { "MARKED",   "T"     },
						  { "MSGLINE",  "MSG"   },
						  { "NOTELINE", "NOTE"  },
						  { "SPECIAL",  "SPE"   } } ;

		map<string,string> abokLCMDS  = { { "AK",  "A"  },
						  { "BK",  "B"  },
						  { "OK",  "O"  },
						  { "OOK", "OO" } } ;

		const string BLKcmds   = "CC DD MM HXX LCC MDD MNN OO OOK RR TJJ TRR TT TXX XX WW UCC (( )) << >> [[ ]]" ;
		const string SPLcmds   = "A AK B BK C CC COLS D DD F I L M MM MD MDD MN MNN R RR S SI TX TXX X XX XI" ;
		const string TODcmds   = "A I" ;
		const string BODcmds   = "B COLS BNDS MASK TABS" ;
		const string ABOKReq   = "C CC M MM" ;
		const string Chkdist   = "C D M HX LC MD MN O OK TJ TR TX UC W X" ;
		const string ABOWList  = "A AK B BK O OK OO OOK W WW" ;
		const string OWBlock   = "OO WW" ;
		const string ReptOK    = "C D F HX I L M MD MN O OK R UC LC RR S TJ TR T TX W X (( )) ( ) << >> < > [[ ]] [ ]" ;
		const string XBlock    = "R (( )) ( ) << >> < > [[ ]] [ ]" ;
		const string CutCmds   = "C CC M MM" ;
		const string PasteCmds = "A B" ;
} ;
