/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify x
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
#include <string>
using namespace std ;

void addHilight( vector<string> &, string fileType, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW ) ;

void addPanelHilight( vector<string> &, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW ) ;

void addCppHilight( vector<string> &, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW ) ;

void addDefHilight( vector<string> &, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW ) ;

void addNoHilight( vector<string> &, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW ) ;

void addRexxHilight( vector<string> &, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW ) ;

