/*
  Copyright (c) 2017 Daniel John Erdos

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

class pedrxm1 : public TSOENV
{
	public:
		pedrxm1() ;
		void application() ;
		void isredit( const string& ) ;

	private:
		miblock* mibptr ;

		void start_rexx() ;
		void macroError() ;
		void macroError( miblock* ) ;

		void showErrorScreen( const string&,
				      string = "" ) ;

	friend RexxObjectPtr RexxEntry editServiceHandler( RexxExitContext*,
							   RexxStringObject,
							   RexxStringObject ) ;
	friend bool is_pgmmacro( miblock* ) ;
} ;
