/*
  Copyright (c) 2023 Daniel John Erdos

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


class lss
{
       public:
		static unsigned int ndataid ;
		static gls*         p_gls ;

		lss() {} ;
		~lss() ;

		static logger* lg ;

       private:
		void lmclose( errblock&,
			      fPOOL*,
			      const string& ) ;

		void lmddisp( errblock&,
			      fPOOL*,
			      const string& ) ;

		void lmdfree( errblock&,
			      fPOOL*,
			      const string& ) ;

		void lmdinit( errblock&,
			      fPOOL*,
			      const string&,
			      const string& ) ;

		void lminit( errblock&,
			     fPOOL*,
			     const string&,
			     string,
			     string,
			     LM_DISP ) ;

		void lmfree( errblock&,
			     fPOOL*,
			     const string& ) ;

		void lmget( errblock&,
			    fPOOL*,
			    const string&,
			    string&,
			    const string&,
			    size_t ) ;

		void lmmadd( errblock&,
			     fPOOL*,
			     const string&,
			     const string&,
			     bool = false ) ;

		void lmmdel( errblock&,
			     fPOOL*,
			     const string&,
			     const string& ) ;

		void lmmfind( errblock&,
			      fPOOL*,
			      const string&,
			      const string&,
			      const string& ) ;

		void lmmlist( errblock&,
			      fPOOL*,
			      const string&,
			      const string&,
			      const string&,
			      const string&,
			      const string& ) ;

		void lmopen( errblock&,
			     fPOOL*,
			     const string&,
			     const string&,
			     const string& ) ;

		void lmput( errblock&,
			    fPOOL*,
			    const string&,
			    const string& ) ;

		void lmquery( errblock&,
			      fPOOL*,
			      const string&,
			      const string&,
			      const string&,
			      const string&,
			      const string&,
			      const string& ) ;

		string get_dsn_dataid( const string& ) ;

		string get_dsn_listid( const string& ) ;

		void set_stats( errblock&,
				fPOOL*,
				const string&,
				const string& ) ;

		bool exists_dataid( errblock&,
				    fPOOL*,
				    map<string, lmentry>::iterator&,
				    const string& ) ;

		bool exists_listid( errblock&,
				    fPOOL*,
				    map<string, ldentry>::iterator&,
				    const string& ) ;

		string get_target( const string& ) ;

		string full_name( const string&,
				  const string& ) ;

		map<string, lmentry> lmentries ;

		map<string, ldentry> ldentries ;

		map<string, vector<string>*> lmmlist_entries ;

		map<vector<string>*, uint> lmmlist_posn ;

		string modname() { return "LSSERV" ; }

		friend class pApplication ;
} ;
