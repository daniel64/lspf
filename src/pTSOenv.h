/*
  Copyright (c) 2022 Daniel John Erdos

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

class TSOENV : public pApplication
{
	public:
		TSOENV() ;
		~TSOENV() ;

		string outtrap( const string&,
				int = -1 ) ;

		int drop( RexxCallContext*,
			  const string& ) ;

		int listdsi( RexxCallContext*,
			     const string&,
			     const string& ) ;

		bool  builtin( const string&,
			       RexxExitContext* = nullptr ) ;

		bool  builtin( const string&,
			       vector<string>&,
			       RexxExitContext* = nullptr ) ;

		string sysvar( const string& ) ;

		virtual string sysexec() ;

	private:

		bool outtrapOn ;
		bool outtrapValid ;

		int  outtrapLines ;
		int  outtrapIndex ;

		string outtrapVar ;

		void clear_sysvars( RexxCallContext* ) ;

		int TSOEntry( RexxExitContext*,
			      RexxStringObject command ) ;

		int Drop( RexxExitContext*,
			  const string& ) ;

		int Execio( RexxExitContext*,
			    const string& ) ;

		void altlib( const string&,
			     vector<string>& ) ;

		void set_shared_var( const string&,
				     const string& ) ;

		void issueLineMessage( int,
				       const string& = "" ) ;

		map<string, int(TSOENV::*)( RexxExitContext*, const string& )>commandfn = { { "DROP",    &TSOENV::Drop    },
											    { "EXECIO",  &TSOENV::Execio  } } ;

		friend RexxObjectPtr RexxEntry TSOServiceHandler( RexxExitContext* context,
								  RexxStringObject address,
								  RexxStringObject command ) ;

} ;
