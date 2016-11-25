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

class Table
{
	public:
		Table()
		{
			refCount       = 1     ;
			CRP            = 0     ;
			maxURID        = 0     ;
			changed        = false ;
			tab_temporary  = false ;
			tab_cmds       = false ;
			tab_path       = ""    ;
			sa_namelst     = ""    ;
			sa_cond_pairs  = ""    ;
			sa_dir         = ""    ;
			sort_ir        = ""    ;
		}
	private:
		int    ownerTask      ;
		bool   tab_temporary  ;
		bool   tab_cmds       ;
		string tab_keys       ;
		string tab_flds       ;
		string tab_all        ;
		string tab_path       ;
		bool   changed        ;
		int    refCount       ;
		int    maxURID        ;
		int    num_keys       ;
		int    num_flds       ;
		int    num_all        ;
		int    CRP            ;
		string sort_ir        ;
		string sa_namelst     ;
		string sa_cond_pairs  ;
		string sa_dir         ;
		tbDISP tab_DISP       ;

		void   saveTable( int & RC,
				  const string & m_name,
				  const string & m_path ) ;

		void   loadRow( int & RC,
				vector< string > & m_flds ) ;

		void   reserveSpace( int tot_rows ) ;

		void   resetChanged() { changed = false ; }

		void   fillfVARs( int & RC,
				  fPOOL & funcPOOL,
				  int depth,
				  int posn ) ;

		int    getCRP() { return CRP ; }

		void   cmdsearch( int & RC,
				  fPOOL & funcPOOL,
				  const string & cmd ) ;

		void   tbadd( int & RC,
			      fPOOL & funcPOOL,
			      string tb_namelst,
			      string tb_order,
			      int tb_num_of_rows )  ;

		void   tbbottom( int & RC,
				 fPOOL & funcPOOL,
				 string tb_savenm,
				 string tb_rowid_vn,
				 string tb_noread,
				 string tb_crp_name ) ;

		void   tbdelete( int & RC,
				 fPOOL & funcPOOL ) ;

		void   tbexist( int & RC,
				fPOOL & funcPOOL ) ;

		void   tbget( int & RC,
			      fPOOL & funcPOOL,
			      string tb_savenm,
			      string tb_rowid_vn,
			      string tb_noread,
			      string tb_crp_name ) ;

		void   tbmod( int & RC,
			      fPOOL & funcPOOL,
			      string tb_namelst,
			      string tb_order ) ;

		void   tbput( int & RC,
			      fPOOL & funcPOOL,
			      string tb_namelst,
			      string tb_order ) ;

		void   tbquery( int & RC,
				fPOOL & funcPOOL,
				string tb_keyn,
				string tb_varn,
				string tb_rownn,
				string tb_keynn,
				string tb_namenn,
				string tb_crpn,
				string tb_sirn,
				string tb_lstn,
				string tb_condn,
				string tb_dirn ) ;

		void   tbsarg( int & RC,
			       fPOOL & funcPOOL,
			       string tb_namelst,
			       string tb_next_prev,
			       string tb_cond_pairs ) ;

		void   tbscan( int &RC,
			       fPOOL & funcPOOL,
			       string tb_namelst,
			       string tb_varname,
			       string tb_rowid_vn,
			       string tb_next_prev,
			       string tb_read,
			       string tb_crp_name,
			       string tb_condlst ) ;

		void   tbskip( int & RC,
			       fPOOL & funcPOOL,
			       int num,
			       string tb_savenm,
			       string tb_rowid_vn,
			       string tb_rowid,
			       string tb_noread,
			       string tb_crp_name ) ;

		void   tbsort( int & RC,
			       string tb_fields ) ;
		void   tbtop( int & RC ) ;
		void   tbvclear( int & RC,
				 fPOOL & funcPOOL ) ;

		vector< vector<string> > table ;
		map< string, tbsearch > sarg   ;

		friend class tableMGR ;
} ;


class tableMGR
{
	public:
		tableMGR() ;
		void   createTable( int & RC,
				    int task,
				    const string & tb_name,
				    string keys,
				    string flds,
				    bool m_temporary,
				    tbREP m_REP,
				    const string & m_path,
				    tbDISP m_DISP ) ;

		void   saveTable( int & RC,
				  int task,
				  const string & tb_name,
				  const string & m_newname,
				  const string & m_path,
				  bool m_err=true ) ;

		void   tbadd( int & RC,
			      fPOOL & funcPOOL,
			      const string & tb_name,
			      const string & tb_namelst,
			      const string & tb_order,
			      int tb_num_of_rows ) ;

		void   tbsort( int & RC,
			       const string & tb_name,
			       const string & tb_fields ) ;

		void   statistics() ;
		void   snap() ;

		void   cmdsearch( int & RC,
				  fPOOL & funcPOOL,
				  string tb_name,
				  const string & cmd,
				  const string & paths ) ;

		void   loadTable( int & RC,
				  int task,
				  const string & tb_name,
				  tbDISP=EXCLUSIVE,
				  const string & src="" ) ;

	private:
		void   fillfVARs( int & RC,
				  fPOOL & funcPOOL,
				  const string & tb_name,
				  int depth,
				  int posn ) ;

		int    getCRP( int & RC,
			       const string & tb_name ) ;

		void   destroyTable( int & RC,
				     int task,
				     const string & tb_name ) ;

		bool   isloaded( const string & tb_name ) ;

		bool   tablexists( const string & tb_name,
				   const string & tb_path ) ;

		void   tbbottom( int & RC,
				 fPOOL & funcPOOL,
				 const string & tb_name,
				 const string & tb_savenm,
				 const string & tb_rowid_vn,
				 const string & tb_noread,
				 const string & tb_crp_name  ) ;

		void   tbdelete( int & RC,
				 fPOOL & funcPOOL,
				 const string & tb_name ) ;

		void   tberase( int & RC,
				const string & tb_name,
				const string & tb_path ) ;

		void   tbexist( int & RC,
				fPOOL & funcPOOL,
				const string & tb_name ) ;

		void   tbget( int & RC,
			      fPOOL & funcPOOL,
			      const string & tb_name,
			      const string & tb_savenm,
			      const string & tb_rowid_vn,
			      const string & tb_noread,
			      const string & tb_crp_name  ) ;

		void   tbmod( int & RC,
			      fPOOL & funcPOOL,
			      const string & tb_name,
			      const string & tb_namelst,
			      const string & tb_order ) ;

		void   tbput( int & RC,
			      fPOOL & funcPOOL,
			      const string & tb_name,
			      const string & tb_namelst,
			      const string & tb_order ) ;

		void   tbquery( int & RC,
				fPOOL & funcPOOL,
				const string & tb_name,
				const string & tb_keyn,
				const string & tb_varn,
				const string & tb_rownn,
				const string & tb_keynn,
				const string & tb_namenn,
				const string & tb_crpn,
				const string & tb_sirn,
				const string & tb_lstn,
				const string & tb_condn,
				const string & tb_dirn ) ;

		void   tbsarg( int & RC,
			       fPOOL & funcPOOL,
			       const string & tb_name,
			       const string & tb_namelst,
			       const string & tb_next_prev,
			       const string & tb_cond_pairs ) ;

		void   tbskip( int & RC,
			       fPOOL & funcPOOL,
			       const string & tb_name,
			       int num,
			       const string & tb_varname,
			       const string & tb_rowid_vn,
			       const string & tb_rowid,
			       const string & tb_noread,
			       const string & tb_crp_name ) ;

		void   tbscan( int & RC,
			       fPOOL & funcPOOL,
			       const string & tb_name,
			       const string & tb_namelst,
			       const string & tb_savenm,
			       const string & tb_rowid_vn,
			       const string & tb_next_prev,
			       const string & tb_read,
			       const string & tb_crp_name,
			       const string & tb_condlst ) ;

		void   tbtop( int & RC,
			      const string & tb_name ) ;

		void   tbvclear( int & RC,
				 fPOOL & funcPOOL,
				 const string & tb_name ) ;

		map<string, Table> tables ;
		friend class pApplication ;
} ;
