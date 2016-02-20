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

/* Execute exec against member */

/* Re-direct say ouput and error messages to a temporary file or it'll mess up lspf output */
"echo x > /tmp/porexx2.say"
"echo x > /tmp/porexx2.err"
"rm /tmp/porexx2.say"
"rm /tmp/porexx2.err"
.output~destination(.stream~new("/tmp/porexx2.say"))
.error~destination(.stream~new("/tmp/porexx2.err"))
/**********************************************************************************************/

exit 

Zfile = .stream~new(arg(1))
Zfile~open("read")

i = 0
data. = ""
do while Zfile~lines > 0
   i = i + 1
   data.i = Zfile~linein
end
data.0 = i
Zfile~close

do i = 1 to data.0
   if UPPER(word(data.i,1)) = "FIELD" | UPPER(word(data.i,1)) = "LITERAL" | UPPER(word(data.i,1)) = "DYNAREA" then
     do
        row = word(data.i,2)
        col = word(data.i,3)
        if datatype(row,"W") then row = row + 1
        if datatype(col,"W") then col = col + 1
        wi2 = wordindex(data.i,2)
        wi3 = wordindex(data.i,3)
        wi4 = wordindex(data.i,4)
        l2  = wi3 - wi2
        l3 = wi4 - wi3
        data.i = substr(data.i, 1, wi2-1) || left( row, l2 ) || left( col, l3 ) || substr( data.i, wi4)
     end
end

do i = 1 to data.0
   if UPPER(word(data.i,1)) = "TBFIELD" then
     do
        col = word(data.i,2)
        if datatype(col,"W") then col = col + 1
        wi2 = wordindex(data.i,2)
        wi3 = wordindex(data.i,3)
        l2  = wi3 - wi2
        data.i = substr(data.i, 1, wi2-1) || left( col, l2 ) || substr( data.i, wi3)
     end
end


Zfile = .stream~new(arg(1))
Zfile~open("write replace")

i = 0
do i = 1 to data.0
   Zfile~lineout(data.i)
   say data.i
end
data.0 = i
Zfile~close
