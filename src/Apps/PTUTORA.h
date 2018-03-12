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

class PTUTORA : public pApplication
{
	public:
		void application() ;

	private:
		void read_file( string ) ;
		void fill_dynamic_area() ;

		int  firstLine ;
		int  startCol  ;

		uint maxLines  ;
		uint maxCol    ;

		vector<string> data ;

		string ZROW1 ;
		string ZROW2 ;
		string ZCOL1 ;
		string ZAREA ;
		string ZSHADOW ;
		string ZAREAT  ;
		int    ZAREAW  ;
		int    ZAREAD  ;

		string fileType ;

		string zcmd  ;
		string zverb ;

		string ZSCROLLA  ;
		int    ZSCROLLN  ;

		string mh ;
		string fh ;
		string ph ;
		string ah ;
		string sh ;
		string kh ;
		string ps ;
		string help     ;
		string helplst  ;
		char   helptype ;
};
