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

/* Display contents of table file
   Table format
   0085 - 2 bytes for file type
   01   - table format V1
   xx   - header length
   vvv  - header
   xx   - Number of header fields (just SIR so should be 1)
   xx   - field length }
   vvv  - field value  }
   xx   - number of rows in the table
   x    - number of keys in the table
   x    - number of fields in the table
   xx   - value length        }
   vvv  - table column value  } x num of rows*(num of keys + num of flds)
*/


/* Display contents of the profile file
   Profile format 
   0084 - 2 bytes for file type
   01   - profile format V1
   xx   - header length
   vvv  - header
   xx   - name length        }
   vvv  - variable name      } Repeated until EOD
   xxxx - value length       }
   vvv  - variable value     }
*/

/* Re-direct say ouput and error messages to a temporary file or it'll mess up lspf output */
"echo x > /tmp/porexx2.say"
"echo x > /tmp/porexx2.err"
"rm /tmp/porexx2.say"
"rm /tmp/porexx2.err"
.output~destination(.stream~new("/tmp/porexx2.say"))
.error~destination(.stream~new("/tmp/porexx2.err"))
/**********************************************************************************************/

Zfile = .stream~new(arg(1))
Zfile~open("read")

data = ""
do while Zfile~chars > 0
   data = data || Zfile~charin
end
Zfile~close

if substr(data,1,2) = '0084'x then
  do
    say "Variable profile file found"
    Call ListVariables
  end
else
  do
    if substr(data,1,2) = '0085'x then
      do
        say "lspf table file found"
        call ListTable
      end
    else
      do
        say "File does not contain a variable profile or a lspf table"
        exit 0
      end
  end
EXIT 0


ListVariables:

ver = c2d(substr(data,3,1))
if ver \= 01 then
  do
    say "Profile version not supported.  Found" version
    exit 0
  end

say "Found version" ver "profile file"
say

hdrlen = c2d(substr(data,4,1))
say "Header length is" hdrlen
say "Header is "substr(data,5,hdrlen)
say

offset = hdrlen + 5
datal  = length(data)
errs   = 0
tot    = 0

do forever
    l      = c2d(substr(data,offset,1))
    if l > 8 then
      do
        errs = errs + 1
        say
        say "*ERROR*  Variable length greater than 8"
        say
      end
    offset = offset + 1
    var    = substr(data,offset,l)
    offset = offset + l
    l      = c2d(substr(data,offset,2))
    offset = offset + 2
    val    = substr(data,offset,l)
    say "Variable:" left(var,8) " Value:" val
    offset = offset + l
    tot = tot + 1
    if offset >= datal then leave
end

say
say "Total number of variables found is" tot
say "Total number of errors found is" errs
say
RETURN


ListTable:
ver = c2d(substr(data,3,1))

if ver \= 01 then
  do
    say "Table version not supported.  Found" version
    exit 0
  end

say "Found version" ver "table file"
say

hdrlen = c2d(substr(data,4,1))
say "Header length is" hdrlen
say "Header is "substr(data,5,hdrlen)
say

offset = hdrlen + 5
hdrflds = c2d(substr(data,offset,1))
say "Number of header fields is" hdrflds

do hdrflds
   offset = offset + 1
   l = c2d(substr(data,offset,1))
   offset = offset + 1
   fld    = substr(data,offset,l)
   offset = offset + l
   say "Header field. . . . . . . . . . .:" fld
end
say

rows = c2d(substr(data,offset,2))
say "Number of rows in the table. . . :" rows

offset = offset + 2
keys = c2d(substr(data,offset,1))
say "Number of keys in the table. . . :" keys

offset = offset + 1
flds = c2d(substr(data,offset,1))
say "Number of fields in the table. . :" flds
say
offset = offset + 1

datal  = length(data)
errs   = 0

name.  = ""
n      = 1

do keys
   klen   = c2d(substr(data,offset,1))
    if klen > 8 then
      do
        errs = errs + 1
        say
        say "*ERROR*  Variable length greater than 8"
        say
      end
   offset = offset + 1
   key    = substr(data,offset,klen)
   name.n = key
   max.n  = klen
   n = n + 1
   offset = offset + klen
   say "Key field . . . . . . . . . . . .:" key
end

do flds
   flen   = c2d(substr(data,offset,1))
    if flen > 8 then
      do
        errs = errs + 1
        say
        say "*ERROR*  Variable length greater than 8"
        say
      end
   offset = offset + 1
   fld    = substr(data,offset,flen)
   name.n = fld
   max.n  = flen
   n = n + 1
   offset = offset + flen
   say "NonKey field. . . . . . . . . . .:" fld
end
say

tot   = 0
val.  = ""
row   = 1
col   = 1
cols  = keys+flds

do forever
    l      = c2d(substr(data,offset,2))
    offset = offset + 2
    v      = substr(data,offset,l)
    offset = offset + l
    say right(row, 4) right(col, 2) "Value :" v
    val.row.col = v
    if length(v) > max.col then max.col = length(v)
    col = col + 1
    if col > cols then 
      do
        col = 1
        row = row + 1
        say
      end
    tot = tot + 1
    if offset >= datal then leave
end

say
say "Total values found in table is" Tot
if ( tot \= cols * rows ) then
  do
    say "Error.  Found "tot" values.  Expected " cols * rows
    err = err + 1
    say
  end
say "Total number of errors found is" errs
say
say "In table format:"

l = "      "
width = 0
do j = 1 to cols
   l = l left( name.j, max.j )
   width = width + max.j
end
say l
say copies("=",width+10)

l = ""
do i = 1 to rows
   do j = 1 to cols
      l = l left( val.i.j, max.j )
   end
   say right( i, 5 ) l
   l = ""
end
RETURN
