/*
  Copyright (c) 2024 Daniel John Erdos

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

enum RR_PARMS
{
	RR_PLU,
	RR_PLD
} ;


class plfhist1 : public pApplication
{
	public:
		plfhist1() ;
		void application() ;

	private:
		void openTableRO( const string& ) ;
		void openTableUP( const string& ) ;
		void closeTable( const string& ) ;
		void endTable( const string& ) ;

		void addFieldHistoryEntry( const string&,
					   const string&,
					   const string& ) ;
		void getHistoryEntries( const string&,
					const string&,
					const string&,
					const string&,
					const string&,
					const string& ) ;
		void createDefaultTable( const string& ) ;

		void wait_enqueue() ;
		void release_enqueue() ;

		void set_shared_var( const string&,
				     const string& ) ;

		string qname ;
		string rname ;

		map<string, RR_PARMS> pgm_parms = { { "PLU", RR_PLU },
						    { "PLD", RR_PLD } } ;
} ;
