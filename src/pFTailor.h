/*
  Copyright (c) 2021 Daniel John Erdos

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


#undef  TASKID
#define TASKID() taskid()

class ftline
{
       public:
	       string name ;
	       string line ;
	       int    ln   ;
	       bool   noft ;

	       ftline( const string& s1,
		       const string& s2,
		       int  i1,
		       bool b1 )
	       {
			name = s1 ;
			line = s2 ;
			ln   = i1 ;
			noft = b1 ;
	       }
} ;


struct exitInfo_ft
{
	fPOOL*   funcPool ;
	poolMGR* p_poolMGR ;
	string   zftxrc ;
	string   zftxmsg ;
	errblock* err ;
	set<string>* vars ;
	void*    pAppl ;
} ;


class pFTailor
{
	public:
		static poolMGR*  p_poolMGR ;
		static tableMGR* p_tableMGR ;
		static logger*   lg ;

	private:
		pFTailor()
		{
			istemp  = false ;
			zftxrc  = 0  ;
			zftxmsg = "" ;
#ifdef HAS_REXX_SUPPORT
			instance = nullptr ;
#endif
		}

		pFTailor( errblock&,
			  fPOOL*,
			  void*,
			  bool ) ;

		~pFTailor() ;

		void ftclose( errblock&,
			      const string& ) ;

		void ftincl( errblock&,
			     const string&,
			     bool ) ;

		fPOOL* funcPool ;

		int taskId ;

		int taskid() { return taskId ; }

		bool istemp ;

		string rxpath ;

		exitInfo_ft exitInfo1 ;

		const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$@" ;
		const string defs = ")DEFAULT )&?!<|>" ;

		void read_skeleton( errblock&,
				    const string&,
				    vector<ftline*>&,
				    vector<ftc_main*>&,
				    set<string>&,
				    bool,
				    bool = true,
				    bool = false,
				    const string& = "",
				    int = 0 ) ;

		void reinitialise() ;

		void free_storage( vector<ftline*>&,
				   vector<ftc_main*>& ) ;

		void preproc_skeleton( errblock&,
				       vector<ftline*>&,
				       vector<ftc_main*>& ) ;

		void proc_input( errblock&,
				 set<string>&,
				 const ftc_main* ) ;

		void proc_input( errblock&,
				 set<string>&,
				 const ftc_main*,
				 bool& ) ;

		void proc_input_do( errblock&,
				    set<string>&,
				    const ftc_main*,
				    bool& ) ;

		void proc_input_dot( errblock&,
				     set<string>&,
				     const ftc_main* ) ;

		void proc_input_rexx( errblock&,
				      const ftc_main* ) ;

		void proc_input_sel( errblock&,
				     const ftc_main*,
				     bool& ) ;

		void proc_input_set( errblock&,
				     const ftc_main* ) ;

		bool loop_cond( errblock&,
				const ftc_main*,
				int,
				int,
				int ) ;

		bool check_cond( errblock&,
				 const ftc_main* ) ;

		string apply_tabs( const string& ) ;

		string cond_subs( errblock&,
				  const ftc_main* ) ;

		string getDialogueVar( errblock&,
				       const string& ) ;

		void putDialogueVar( errblock&,
				     const string&,
				     const string& ) ;

		void tbopen( errblock&,
			     const string& ) ;

		void tbskip( errblock&,
			     const string& ) ;

		void tbtop( errblock&,
			    const string& ) ;

		void tbquery( errblock&,
			      const string&,
			      bool& ) ;

		void tbend( errblock&,
			    const string& ) ;

		void tbsarg( errblock&,
			     const string&,
			     const string& ) ;

		void tbscan( errblock&,
			     const string& ) ;

		string sub_var( errblock&,
				const string& ) ;

		int    get_val( errblock&,
				const ftc_main*,
				const string&,
				int ) ;

		string sub_vars( string ) ;

		string src_info( const ftc_main* ) ;

		string src_info( const string&,
				 int ) ;

		string src_info_line( const ftc_main* ) ;

		vector<string> ft_output ;

		ftc_tb* ft_curtab ;

		bool is_temp() { return istemp ; }

		void*  pAppl ;

		int    zftxrc ;
		string zftxmsg ;

		char char_ctrl ;
		char char_var ;
		char def3 ;
		char char_tab ;
		char char_cs1 ;
		char char_cs2 ;
		char char_cs3 ;

		string modname() { return "FTAILOR" ; }

		map<ftc_rexx*, vector<string>> rexx_inlines ;

#ifdef HAS_REXX_SUPPORT
		void start_rexx_interpreter( errblock& ) ;

		RexxInstance* instance   ;
		RexxThreadContext* threadContext ;
		RexxArrayObject args     ;
		RexxCondition condition  ;
		RexxDirectoryObject cond ;

		RexxOption options[ 4 ] ;
		RexxContextExit exits[ 4 ] ;

		friend int REXXENTRY rxterExit_ft( RexxExitContext*,
						   int,
						   int,
						   PEXIT ) ;

		friend int REXXENTRY rxiniExit_ft( RexxExitContext*,
						   int,
						   int,
						   PEXIT ) ;
#endif
		friend class pApplication ;
} ;
