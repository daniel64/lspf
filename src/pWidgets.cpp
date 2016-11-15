/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modifyf
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

#undef  MOD_NAME
#undef  LOGOUT
#define MOD_NAME pWidgets
#define LOGOUT   aplog


int field::field_init( int MAXW, int MAXD, string line )
{
	// Format of field entry in panels (FORMAT 1 VERSION 1 )
	// FIELD row col len cuaAttr opts field_name
	// w1    w2  w3  w4  w5      w6   w7

	// FIELD 3  14  90  NEF CAPS(On),pad('_'),just(left),numeric(off),skip(on) ZCMD

	int RC    ;
	int row   ;
	int col   ;
	int len   ;
	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string w6 ;
	string w7 ;
	cuaType fType   ;

	w2  = word( line, 2 ) ;
	w3  = word( line, 3 ) ;
	w4  = word( line, 4 ) ;
	w5  = word( line, 5 ) ;
	w6  = word( line, 6 ) ;
	w7  = word( line, 7 ) ;

	row = ds2d( w2 ) ;

	if ( row > MAXD )
	{
		log( "E", "Field outside panel area.  row=" << row << " ZSCRMAXD=" << MAXD << endl ; )
		return 20 ;
	}

	if ( isnumeric( w3 ) )                   { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                  { col = MAXW       ; }
	else if ( substr( w3, 1, 4 ) == "MAX-" ) { col = MAXW - ds2d( substr( w3, 5 ) ) ; }
	else                                     { return 20        ; }

	if ( isnumeric( w4 ) )                   { len = ds2d( w4 ) ; }
	else if ( w4 == "MAX" )                  { len = MAXW - col + 1 ; }
	else if ( substr( w4, 1, 4 ) == "MAX-" ) { len = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                     { return 20        ; }

	if ( w5 == "PWD" )
	{
		fType     = NEF  ;
		field_pwd = true ;
	}
	else
	{
		if ( cuaAttrName.find( w5 ) != cuaAttrName.end() )
		{
			fType = cuaAttrName[ w5 ] ;
		}
		else
		{
			log( "E", "Unknown field CUA attribute type " << w5 << endl ; )
			return 20 ;
		}
	}

	field_cua    = fType ;
	field_prot   = cuaAttrProt [ fType ] ;
	field_row    = row-1 ;
	field_col    = col-1 ;
	field_length = len   ;
	field_cole   = field_col + field_length ;
	field_input  = ( fType == CEF || fType == NEF || fType == DATAIN ) ;

	fieldOptsParse( RC, w6, field_caps, field_just, field_numeric, field_padchar, field_skip ) ;
	if ( RC > 0 )
	{
	       log( "E", "Error parsing options for field " << w7 << ". Entry is " << w6 << endl ) ;
	}

	return RC ;
}


int dynArea::dynArea_init( int MAXW, int MAXD, string line )
{
	// Format of OPTION entry in panels (FORMAT 1 VERSION 1 )
	// DYNAREA row col width depth   A-name S-name DATAIN() DATAOUT() USERMOD() DATAMOD()

	// w1      w2         w3   w4    w5     w6     w7      <-----------------keywords------------------>
	// DYNAREA MAX-10 MAX-20   MAX   MAX-6  ZAREA  ZSHADOW DATAIN(01) DATAOUT(02) USERMOD(03) DATAMOD(04)

	// if width=MAX  width=MAXW-col+1
	// if depth=MAX  depth=MAXD-row+1

	// UserMod - field touched
	// DataMod - field changed

	// UserMod but not DataMod - Attr changed to UserMod even if the field was changed
	// DataMod but not UserMod - Attr changed to DataMod only if the field value has changed
	// UserMod and DataMod     - Attr changed to UserMod if no change to field value, else DataMod
	// Neither                 - Attr unchanged

	int row   ;
	int col   ;
	int width ;
	int depth ;
	int p1    ;
	int p2    ;
	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string w7 ;
	string t  ;

	w2  = word( line, 2 ) ;
	w3  = word( line, 3 ) ;
	w4  = word( line, 4 ) ;
	w5  = word( line, 5 ) ;
	w7  = word( line, 7 ) ;

	row = ds2d( w2 ) ;
	col = ds2d( w3 ) ;

	if ( isnumeric( w2 ) )                   { row = ds2d( w2  ) ; }
	else if ( w2 == "MAX" )                  { row = MAXD        ; }
	else if ( substr( w2, 1, 4 ) == "MAX-" ) { row = MAXD - ds2d( substr( w2, 5 ) ) ; }
	else                                     { return 20         ; }

	if ( isnumeric( w3 ) )                   { col = ds2d( w3 )  ; }
	else if ( w3 == "MAX" )                  { col = MAXW        ; }
	else if ( substr( w3, 1, 4 ) == "MAX-" ) { col = MAXW - ds2d( substr( w3, 5 ) ) ; }
	else                                     { return 20         ; }

	if ( isnumeric( w4 ) )                   { width = ds2d( w4  )    ; }
	else if ( w4 == "MAX" )                  { width = MAXW - col + 1 ; }
	else if ( substr( w4, 1, 4 ) == "MAX-" ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                     { return 20              ; }

	if ( isnumeric( w5 ) )                   { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                  { depth = MAXD - row + 1 ; }
	else if ( substr( w5, 1, 4 ) == "MAX-" ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else                                     { return 20              ; }


	if ( row > MAXD )
	{
		log( "E", "Dynamic area outside panel area.  row=" << row << " MAXD=" << MAXD ) ;
		return  20 ;
	}
	if ( width > (MAXW - col+1) ) { width = (MAXW - col+1) ; }
	if ( depth > (MAXD - row+1) ) { depth = (MAXD - row+1) ; }

	if ( p1 = pos( "DATAIN(", line ) )
	{
		p2 = pos( ")", line, p1 ) ;
		if ( p2 == 0 ) { return 20 ; }
		t  = strip( substr( line, (p1 + 7), (p2 - (p1 + 7)) ) ) ;
		dynArea_DataInsp = true ;
		if      ( t.size() == 1 ) { dynArea_DataIn = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_DataIn = xs2cs( t )[0] ; }
		else                      { return 20                      ; }
	}
	if ( p1 = pos( "DATAOUT(", line ) )
	{
		p2 = pos( ")", line, p1 ) ;
		if ( p2 == 0 ) { return 20 ; }
		t  = strip( substr( line, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		dynArea_DataOutsp = true ;
		if      ( t.size() == 1 ) { dynArea_DataOut = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_DataOut = xs2cs( t )[0] ; }
		else                      { return 20                       ; }
	}
	if ( p1 = pos( "USERMOD(", line ) )
	{
		p2 = pos( ")", line, p1 ) ;
		if ( p2 == 0 ) { return 20 ; }
		t  = strip( substr( line, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		dynArea_UserModsp = true ;
		if      ( t.size() == 1 ) { dynArea_UserMod = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_UserMod = xs2cs( t )[0] ; }
		else                      { return 20                       ; }
	}
	if ( p1 = pos( "DATAMOD(", line ) )
	{
		p2 = pos( ")", line, p1 ) ;
		if ( p2 == 0 ) { return 20 ; }
		t  = strip( substr( line, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		dynArea_DataModsp = true ;
		if      ( t.size() == 1 ) { dynArea_DataMod = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_DataMod = xs2cs( t )[0] ; }
		else                      { return 20                       ; }
	}

	if ( w7 == "" ) { return 20 ; }
	if ( (dynArea_UserModsp || dynArea_DataModsp) && !dynArea_DataInsp ) { return 20 ; }

	dynArea_row         = row-1 ;
	dynArea_col         = col-1 ;
	dynArea_width       = width ;
	dynArea_depth       = depth ;
	dynArea_shadow_name = w7    ;

	if ( dynArea_DataInsp  ) { dynArea_FieldIn.push_back( dynArea_DataIn  ) ; }
	if ( dynArea_UserModsp ) { dynArea_FieldIn.push_back( dynArea_UserMod ) ; }
	if ( dynArea_DataModsp ) { dynArea_FieldIn.push_back( dynArea_DataMod ) ; }

	dynArea_Field = dynArea_FieldIn ;
	if ( dynArea_DataOutsp ) { dynArea_Field.push_back( dynArea_DataOut   ) ; }

	return 0 ;
}


bool field::edit_field_insert( WINDOW * win, char ch, int col, bool Isrt, bool snulls )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_DataInsp is true and
	// there is an input attribute byte at the start of the field

	// Use the null character (x'00') to fill fields when the cursor is past the end if the field value
	// These are then removed before further processing by the application

	uint pos ;

	int p1  ;
	int p2  ;

	dynArea * da ;
	const char nulls(0x00) ;

	pos = (col - field_col ) ;
	if ( pos >= field_value.size() )
	{
		field_value.resize( pos+1, nulls ) ;
	}

	if ( Isrt )
	{
		if ( field_dynArea )
		{
			da = field_dynArea_ptr ;
			p2 = field_value.find_first_of( da->dynArea_Field, pos ) ;
			if ( p2 == string::npos ) { p2 = field_value.size()-1 ; }
			else                      { p2--                      ; }
			if ( field_value[ p2 ] != ' '  &&
			     field_value[ p2 ] != nulls ) { return false ; }
			field_value.erase( p2, 1 ) ;
			field_shadow_value.erase( p2, 1 ) ;
			p1 = field_value.find_last_of( da->dynArea_FieldIn, pos ) ;
			if ( da->dynArea_DataModsp )
			{
				if ( field_value[ p1 ] != da->dynArea_DataMod )
				{
					if ( ch != ' ' )
					{
						field_value[ p1 ] = da->dynArea_DataMod ;
					}
					else if ( field_value.substr( pos, p2-pos ) != string( p2-pos, ' ' ) )
					{
						field_value[ p1 ] = da->dynArea_DataMod ;
					}
					else if ( da->dynArea_UserModsp )
					{
						field_value[ p1 ] = da->dynArea_UserMod ;
					}
				}
			}
			else if ( da->dynArea_UserModsp )
			{
				field_value[ p1 ] = da->dynArea_UserMod ;
			}
			if ( field_value[ pos ] == nulls )
			{
				field_value[ pos ]        = ch       ;
				field_shadow_value[ pos ] = B_YELLOW ;
			}
			else
			{
				field_value.insert( pos, 1, ch ) ;
				field_shadow_value.insert( pos, 1, B_YELLOW ) ;
			}
		}
		else
		{
			if ( field_length == field_value.size() ) { return false ; }
			if ( field_value[ pos ] == nulls )
			{
				field_value[ pos ] = ch ;
			}
			else
			{
				field_value.insert( pos, 1, ch ) ;
			}
		}
	}
	else
	{
		if ( field_dynArea )
		{
			if ( field_value[ pos ] != nulls && !isprint( field_value[ pos ] ) ) { return false ; }
			da = field_dynArea_ptr ;
			field_shadow_value[ pos ] = B_YELLOW ;
			p1 = field_value.find_last_of( da->dynArea_FieldIn, pos ) ;
			if ( da->dynArea_DataModsp )
			{
				if ( field_value[ p1 ] != da->dynArea_DataMod )
				{
					if ( field_value[ pos ] != ch )
					{
						field_value[ p1 ] = da->dynArea_DataMod ;
					}
					else if ( da->dynArea_UserModsp )
					{
						field_value[ p1 ] = da->dynArea_UserMod ;
					}
				}
			}
			else if ( da->dynArea_UserModsp )
			{
				field_value[ p1 ] = da->dynArea_UserMod ;
			}

		}
		field_value[ pos ] = ch ;
	}

	display_field( win, snulls ) ;
	field_changed = true ;
	return true ;
}


void field::edit_field_delete( WINDOW * win, int col, bool snulls )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_DataInsp is true
	// and there is an input attribute byte at the start of the field.

	uint pos ;

	int p1 ;
	int p2 ;

	const char nulls(0x00) ;

	dynArea * da ;

	pos = col - field_col ;
	if ( pos > field_value.size() ) { return ; }
	if ( field_value[ pos ] != nulls && !isprint( field_value[ pos ] ) ) { return ; }

	if ( field_dynArea )
	{
		da = field_dynArea_ptr ;
		p1 = field_value.find_last_of( da->dynArea_FieldIn, pos ) ;
		p2 = field_value.find_first_of( da->dynArea_Field, pos )  ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		if ( da->dynArea_DataModsp )
		{
			if ( field_value[ p1 ] != da->dynArea_DataMod )
			{
				if ( field_value.substr( pos, p2-pos ) != string( p2-pos, ' ' ) )
				{
					field_value[ p1 ] = da->dynArea_DataMod ;
				}
				else if ( da->dynArea_UserModsp )
				{
					field_value[ p1 ] = da->dynArea_UserMod ;
				}
			}
		}
		else if ( da->dynArea_UserModsp )
		{
			field_value[ p1 ] = da->dynArea_UserMod ;
		}
		field_value.insert( p2, 1, ' ' ) ;
		field_shadow_value.insert( p2, 1, 0xFF ) ;
		field_shadow_value.erase( pos, 1 ) ;
	}

	field_value.erase( pos, 1 ) ;
	display_field( win, snulls ) ;
	field_changed = true ;
}


int field::edit_field_backspace( WINDOW * win, int col, bool snulls )
{
	// If this is a dynamic area, we know it is an input field so pos > 0 (to allow for the input attribute byte)

	int pos ;

	pos = col - field_col ;
	if ( pos > field_value.size() ) { return --col ; }

	if ( field_dynArea )
	{
		if ( field_dynArea_ptr->dynArea_Field.find_first_of( field_value[ pos-1 ] ) != string::npos )
		{
			return col ;
		}
	}

	col-- ;
	edit_field_delete( win, col, snulls ) ;
	return col ;
}


void field::field_erase_eof( WINDOW * win, uint col, bool snulls )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_DataInsp is true,
	// and there is an input attribute byte at the start of the field

	int pos ;
	int p1  ;
	int p2  ;

	dynArea * da ;

	if ( ( field_col + field_value.size() ) < col ) return ;
	pos = (col - field_col) ;

	if ( field_dynArea )
	{
		da = field_dynArea_ptr ;
		p1 = field_value.find_last_of( da->dynArea_FieldIn, pos ) ;
		p2 = field_value.find_first_of( da->dynArea_Field, pos ) ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		if ( da->dynArea_DataModsp )
		{
			if ( field_value[ p1 ] != da->dynArea_DataMod )
			{
				if ( field_value.substr( pos, p2-pos ) != string( p2-pos, ' ' ) )
				{
					field_value[ p1 ] = da->dynArea_DataMod ;
				}
				else if ( da->dynArea_UserModsp )
				{
					field_value[ p1 ] = da->dynArea_UserMod ;
				}
			}
		}
		else if ( da->dynArea_UserModsp )
		{
			field_value[ p1 ] = da->dynArea_UserMod ;
		}
		field_value.replace( pos, p2-pos, p2-pos, ' ' )  ;
		field_shadow_value.replace( pos, p2-pos, p2-pos, 0xFF ) ;
	}
	else
	{
		field_blank( win ) ;
		field_value.erase( pos+1 ) ;
	}

	display_field( win, snulls ) ;
	field_changed = true ;
}


void field::field_blank( WINDOW * win )
{
	string blank( field_length, field_padchar ) ;
	mvwaddstr( win, field_row, field_col, blank.c_str() ) ;
}


void field::field_remove_nulls()
{
	// Remove all nulls from the field value

	// For dynamic areas, remove nulls from input fields that have been touched or changed.
	// Trailing nulls are changed to blanks.  Keep field size constant by adding blanks at
	// the end of the input field when removing nulls.

	// If a field that has only been touched, changes, change DataIn to DataMod (if specified)

	// Keep shadow variable in sync

	// BUG: Nulls not removed from a touched/changed field with no USERMOD/DATAMOD parameter specified
	// (as we cannot determine if the field has been touched/changed without these)

	int p1 ;
	int p2 ;
	int p3 ;
	int pattr ;

	bool changed ;

	const char nulls(0x00) ;

	dynArea * da ;

	p1 = 0 ;
	if ( field_dynArea )
	{
		da = field_dynArea_ptr ;
		if ( !da->dynArea_DataInsp ) { return ; }
		while ( true )
		{
			p1 = field_value.find_first_of( da->dynArea_FieldIn, p1 ) ;
			if ( p1 == string::npos ) { return ; }
			pattr = p1 ;
			p1++ ;
			p3 = field_value.find_first_of( da->dynArea_Field, p1 ) ;
			if ( p3 == string::npos ) { p3 = field_value.size() - 1 ; }
			else                      { p3--                        ; }
			p2 = field_value.find_last_not_of( nulls, p3 ) ;
			if ( p2 == string::npos )
			{
				field_value.replace( p1, p3-p1+1, p3-p1+1, ' ' ) ;
				field_shadow_value.replace( p1, p3-p1+1, p3-p1+1, 0xFF ) ;
				if ( da->dynArea_DataModsp )
				{
					field_value[ pattr ] = da->dynArea_DataMod ;
				}
				break ;
			}
			changed = false ;
			if ( p3 > p2 )
			{
				field_value.replace( p2+1, p3-p2, p3-p2, ' ' ) ;
				field_shadow_value.replace( p2+1, p3-p2, p3-p2, 0xFF ) ;
				changed = true ;
			}
			while ( true )
			{
				if ( p1 >= p2 ) { break ; }
				if ( field_value[ p1 ] == nulls )
				{
					changed = true ;
					field_value.erase( p1, 1 ) ;
					field_value.insert( p3, 1, ' ' ) ;
					field_shadow_value.erase( p1, 1 ) ;
					field_shadow_value.insert( p3, 1, 0xFF ) ;
					p2-- ;
					continue ;
				}
				p1++ ;
			}
			if ( changed && da->dynArea_DataModsp )
			{
				field_value[ pattr ] = da->dynArea_DataMod ;
			}
		}
	}
	else
	{
		while ( true )
		{
			p1 = field_value.find_first_of( nulls, p1 ) ;
			if ( p1 == string::npos ) { break ; }
			field_value.erase( p1, 1 ) ;
		}
	}
}


void field::field_clear( WINDOW * win )
{
	field_value = ""     ;
	field_blank( win )   ;
	field_changed = true ;
}


int field::end_of_field( WINDOW * win, uint col )
{
	// If this is a dynamic area, we know at this point this is an input field.

	// Strip trailing nulls if not a dynamic area

	int pos ;
	int p1  ;
	int p2  ;

	string padc ;

	padc.clear()           ;
	padc.push_back( 0x00 ) ;
	padc.push_back( 0x20 ) ;

	const char nulls(0x00) ;

	if ( field_dynArea )
	{
		pos = (col - field_col) ;
		p2  = field_value.find_first_of( field_dynArea_ptr->dynArea_Field, pos ) ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		p1  = field_value.find_last_not_of( padc, p2-1 ) ;
		if ( p1 == p2 - 1 ) { return p1 + field_col     ; }
		else                { return p1 + field_col + 1 ; }
	}
	else
	{
		pos = field_value.find_last_not_of( nulls ) ;
		if ( pos == string::npos )
		{
			field_value = "" ;
			return field_col ;
		}
		else
		{
			if ( pos < field_value.size()-1 ) { field_value.erase( pos+1 ) ; }
		}
		if ( field_value.size() == field_length )
		{
			return ( field_col + field_value.size()-1 ) ;
		}
		else
		{
			return ( field_col + field_value.size() ) ;
		}
	}
	return 0 ;
}


bool field::field_dyna_input( uint col )
{
	// When this routine is called, we only know the field is a dynamic area.
	// Check if we are on a field attribute byte
	// Find the previous field attribute.
	// If found, test to see if it is an input attribute

	uint pos ;
	int p1   ;

	dynArea * da = field_dynArea_ptr ;

	if ( !da->dynArea_DataInsp ) { return false ; }

	pos = (col - field_col) ;

	p1 = da->dynArea_Field.find_first_of( field_value[ pos ] ) ;
	if ( p1 != string::npos ) { return false ; }

	p1 = field_value.find_last_of( da->dynArea_Field, pos ) ;
	if ( p1 == string::npos ) { return false ; }

	p1 = da->dynArea_FieldIn.find_first_of( field_value[ p1 ] ) ;
	if ( p1 != string::npos ) { return true  ; }
	else                      { return false ; }
}


int field::field_dyna_input_offset( uint col )
{
	// When this routine is called, we know the field is a dynamic area

	uint pos ;
	int  p1  ;

	pos = ( col < field_col ) ? 0 : (col - field_col) ;

	p1 = field_value.find_first_of( field_dynArea_ptr->dynArea_FieldIn, pos ) ;
	if ( p1 == string::npos ) { return -1 ; }
	return p1+1 ;
}


int field::field_attr( string attrs )
{
	// Format:
	// TYPE(CUA) (change to/from PS not allowed)
	// COLOUR(RED/GREEN/YELLOW/BLUE/MAGENTA/TURQ/WHITE) INTENSE(LOW/HIGH) HILITE(NONE/BLINK/REVERSE/USCORE)
	// For NON-CUA field_colour, default to the current CUA value in case only one attribute is changed
	// Hex format for field_colour:
	// 00 X0 00 00   - X is the INTENSITY
	// 00 0X 00 00   - X is the HILITE
	// 00 00 XX 00   - X is the COLOUR

	int p1 ;
	int p2 ;
	string cua    ;
	string col    ;
	string hilite ;
	string intens ;

	cua    = "" ;
	col    = "" ;
	intens = "" ;
	hilite = "" ;

	p1 = pos( "TYPE(", attrs ) ;
	if ( p1 > 0 )
	{
		p2     = pos( ")", attrs, p1 ) ;
		if ( p2 == 0 ) { return 20 ; }
		cua    = strip( substr( attrs, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
		cua    = strip( cua, 'B', '"' ) ;
		attrs  = delstr( attrs, p1, (p2 - p1 + 1) ) ;
		if ( strip( attrs ) != "" ) { return 20 ; }
		if ( cuaAttrName.find( cua ) == cuaAttrName.end() ) { return 20 ; }
		if ( cua == "PS" || field_cua == PS ) { return 20 ; }
		field_cua = cuaAttrName[ cua ] ;
		field_usecua = true ;
		return 0 ;
	}
	if ( field_usecua ) { field_colour = cuaAttr[ field_cua ] ; }
	p1 = pos( "COLOUR(", attrs ) ;
	if ( p1 > 0 )
	{
		p2     = pos( ")", attrs, p1 ) ;
		if ( p2 == 0 ) { return 20 ; }
		col    = strip( substr( attrs, (p1 + 7), (p2 - (p1 + 7)) ) ) ;
		col    = strip( col, 'B', '"' ) ;
		attrs  = delstr( attrs, p1, (p2 - p1 + 1) ) ;
		field_colour = field_colour & 0XFFFF00FF ;
		if      ( col == "RED"     ) { field_colour = field_colour | RED     ; }
		else if ( col == "GREEN"   ) { field_colour = field_colour | GREEN   ; }
		else if ( col == "YELLOW"  ) { field_colour = field_colour | YELLOW  ; }
		else if ( col == "BLUE"    ) { field_colour = field_colour | BLUE    ; }
		else if ( col == "MAGENTA" ) { field_colour = field_colour | MAGENTA ; }
		else if ( col == "TURQ"    ) { field_colour = field_colour | TURQ    ; }
		else if ( col == "WHITE"   ) { field_colour = field_colour | WHITE   ; }
		else                         { return 20                             ; }
	}
	p1 = pos( "INTENSE(", attrs ) ;
	if ( p1 > 0 )
	{
		field_colour = field_colour & 0XFF0FFFFF ;
		p2     = pos( ")", attrs, p1 ) ;
		if ( p2 == 0 ) { return 20 ; }
		intens = strip( substr( attrs, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		intens = strip( intens, 'B', '"' ) ;
		attrs  = delstr( attrs, p1, (p2 - p1 + 1) ) ;
		if      ( intens == "HIGH" ) { field_colour = field_colour | A_BOLD   ; }
		else if ( intens == "LOW"  ) { field_colour = field_colour | A_NORMAL ; }
		else                         { return 20                              ; }
	}
	p1 = pos( "HILITE(", attrs ) ;
	if ( p1 > 0 )
	{
		field_colour = field_colour & 0XFFF0FFFF ;
		p2     = pos( ")", attrs, p1 ) ;
		if ( p2 == 0 ) { return 20 ; }
		hilite = strip( substr( attrs, (p1 + 7), (p2 - (p1 + 7)) ) ) ;
		hilite = strip( hilite, 'B', '"' ) ;
		attrs  = delstr( attrs, p1, (p2 - p1 + 1) ) ;
		if      ( hilite == "NONE"    ) {}
		else if ( hilite == "BLINK"   ) { field_colour = field_colour | A_BLINK     ; }
		else if ( hilite == "REVERSE" ) { field_colour = field_colour | A_REVERSE   ; }
		else if ( hilite == "USCORE"  ) { field_colour = field_colour | A_UNDERLINE ; }
		else                            { return 20                                 ; }
	}
	if ( strip( attrs ) != "" ) { return 20 ; }
	field_usecua = false ;
	return 0 ;
}


void field::field_attr()
{
	// Reset field attribute to use the CUA value

	field_usecua = true ;
}


void field::field_DataMod_to_UserMod( string * darea, int offset )
{
	// Reset any dynArea_DataMod attributes to dynArea_UserMod or field_dynDataIn, for dynamic fields that
	// have not changed. This is for fields that have been changed, then changed back to the original.
	// (This has to show as touched/input attr, not changed).

	// When this is called, we know dynDataIn and dynDataMod have been specified (but not necessarily dynUserMod,
	// or dynDataOut).  If no change to the field, attr=dynUserMod if specified, else dynDataIn

	int p1 ;
	int p2 ;

	dynArea * da = field_dynArea_ptr ;

	p1 = 0 ;
	while ( true )
	{
		p1 = field_value.find_first_of( da->dynArea_DataMod, p1 ) ;
		if ( p1 == string::npos ) { break ; }
		p1++ ;
		p2 = field_value.find_first_of( da->dynArea_Field, p1 ) ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		if ( field_value.compare( p1, p2-p1, (*darea), offset+p1, p2-p1 ) == 0 )
		{
			if ( da->dynArea_UserModsp ) { field_value[ p1-1 ] = da->dynArea_UserMod ; }
			else                         { field_value[ p1-1 ] = da->dynArea_DataIn  ; }
		}
		p1 = p2 ;
	}
}


void field::display_field( WINDOW * win, bool snulls )
{
	// For non-dynamic area fields: if an input field, truncate if value size > field size else for output fields
	// display field size bytes and leave field value unchanged (necessary for table display fields)

	// Display the null character as the field pad character and pad the field with the same character

	// Call ncurses touchline() for the field row as the update does not always appear without it

	uint i     ;
	char nullc ;
	string t   ;

	const char nulls(0x00) ;

	string::iterator it1 ;
	string::iterator it2 ;

	dynArea * da ;

	nullc = snulls ? '.' : ' ' ;

	if ( field_dynArea )
	{
		da = field_dynArea_ptr ;
		if ( da->dynArea_DataInsp || da->dynArea_DataOutsp || da->dynArea_UserModsp || da->dynArea_DataModsp )
		{
			it2  = field_shadow_value.begin() ;
			i    = 0 ;
			for ( it1 = field_value.begin() ; it1 != field_value.end() ; it1++, it2++, i++ )
			{
				wattrset( win, usrAttr[ (*it2) ] ) ;
				if (  (*it1) == nulls )
				{
					mvwaddch( win, field_row, field_col+i, nullc ) ;
				}
				else if (  da->dynArea_Field.find_first_of( (*it1) ) != string::npos )
				{
					mvwaddch( win, field_row, field_col+i, ' ' ) ;
				}
				else if ( isprint( (*it1) ) )
				{
					mvwaddch( win, field_row, field_col+i, (*it1) ) ;
				}
				else
				{
					mvwaddch( win, field_row, field_col+i, '.' ) ;
				}
			}
		}
		else
		{
			it2  = field_shadow_value.begin() ;
			i    = 0 ;
			for ( it1 = field_value.begin() ; it1 != field_value.end() ; it1++, it2++, i++ )
			{
				wattrset( win, usrAttr[ (*it2) ] ) ;
				if (  (*it1) == nulls )
				{
					mvwaddch( win, field_row, field_col+i, nullc ) ;
				}
				else if ( isprint( (*it1) ) )
				{
					mvwaddch( win, field_row, field_col+i, (*it1) ) ;
				}
				else
				{
					mvwaddch( win, field_row, field_col+i, '.' )    ;
				}
				wattroff( win, usrAttr[ (*it2) ] ) ;
			}
		}
	}
	else
	{
		if ( field_usecua ) { wattrset( win, cuaAttr[ field_cua ] ) ; }
		else                { wattrset( win, field_colour)          ; }
		if ( field_input )
		{
			if ( field_value.size() > field_length )
			{
				field_value = field_value.substr( 0, field_length-1 ) ;
			}
			t = field_value ;
		}
		else
		{
			if ( field_value.size() > field_length ) { t = field_value.substr( 0, field_length ) ; }
			else                                     { t = field_value                           ; }
		}
		for ( it1 = t.begin() ; it1 != t.end() ; it1++ )
		{
			if      ( (*it1) == nulls   ) { (*it1) = field_padchar ; }
			else if ( !isprint( (*it1)) ) { (*it1) = '.'           ; }
		}
		if ( field_pwd )
		{
			mvwaddstr( win, field_row, field_col, string( field_value.size(), '*').c_str() ) ;
		}
		else
		{
			t.resize( field_length, field_padchar )           ;
			mvwaddstr( win, field_row, field_col, t.c_str() ) ;
		}
	}
	touchline( win, field_row, 1 ) ;
}


bool field::cursor_on_field( uint row, uint col )
{
	if ( field_row != row) { return false ; } ;
	if ( col >= field_col && col < field_cole ) { return true ; }
	return false ;
}


int literal::literal_init( int MAXW, int MAXD, int & opt_field, string line )
{
	// Format of literal entry in panels (FORMAT 1 VERSION 1 )
	// LITERAL row col cuaAttr (EXPAND) value
	// w1      w2  w3  w4      w5
	// LITERAL 3   1   OUTPUT  "COMMAND ===> "
	// OR
	// w1      w2  w3  w4      w5     w6
	// LITERAL 3   1   OUTPUT  EXPAND "-"
	// Use EXPAND to repeat the value to end of the screen (MAXW)

	int row   ;
	int col   ;
	int l     ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;

	cuaType fType ;

	w2 = word( line, 2 ) ;
	w3 = word( line, 3 ) ;
	w4 = word( line, 4 ) ;
	w5 = word( line, 5 ) ;

	if ( isnumeric( w2 ) )                   { row = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                  { row = MAXD       ; }
	else if ( substr( w2, 1, 4 ) == "MAX-" ) { row = MAXD - ds2d( substr( w2, 5 ) ) ; }
	else                                     { return 20        ; }

	if ( isnumeric( w3 ) )                   { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                  { col = MAXW       ; }
	else if ( substr( w3, 1, 4 ) == "MAX-" ) { col = MAXW - ds2d( substr( w3, 5 ) ) ; }
	else                                     { return 20        ; }

	if ( cuaAttrName.find( w4 ) != cuaAttrName.end() )
	{
		fType = cuaAttrName[ w4 ] ;
	}
	else
	{
		log( "E", "Unknown field CUA attribute type " << w4 << endl ; )
		return 20 ;
	}

	if ( row > MAXD )
	{
		log( "E", "Literal outside panel area.  row=" << row << " ZSCRMAXD=" << MAXD << endl ; )
		return 20 ;
	}
	if ( col > MAXW )
	{
		log( "E", "Literal outside panel area.  col=" << col << " ZSCRMAXW=" << MAXW << endl ; )
		return 20 ;
	}

	literal_cua = fType   ;
	literal_row = row - 1 ;
	literal_col = col - 1 ;
	if ( w5 == "EXPAND" )
	{
		literal_value  = strip( strip( subword( line, 6 ) ), 'B', '"' ) ;
		l = (MAXW - col) / literal_value.size() + 1 ;
		literal_value  = substr( copies( literal_value, l ), 1, MAXW-col+1 ) ;
	}
	else
	{
		literal_value  = strip( strip( subword( line, 5 ) ), 'B', '"' ) ;
	}
	literal_cole = literal_col + literal_value.size() ;
	if ( fType == PS )
	{
		literal_name = "ZPS01" + right( d2ds(++opt_field), 3, '0') ;
	}
	return 0 ;
}


void literal::literal_display( WINDOW * win, const string & s  )
{
	wattrset( win, cuaAttr[ literal_cua ] ) ;
	mvwaddstr( win, literal_row, literal_col, s.c_str() ) ;
	wattroff( win, cuaAttr[ literal_cua ] ) ;
}


bool literal::cursor_on_literal( uint row, uint col )
{
	if ( literal_row != row) { return false ; } ;
	if ( col >= literal_col && col < literal_cole ) { return true ; }
	return false ;
}


void dynArea::setsize( int row, int col, int width, int depth )
{
	dynArea_row   = row - 1 ;
	dynArea_col   = col - 1 ;
	dynArea_width = width   ;
	dynArea_depth = depth   ;
}


void abc::add_pdc( string name, string run, string parm, string unavail )
{
	pdc m_pdc ;

	m_pdc.pdc_name    = name    ;
	m_pdc.pdc_run     = run     ;
	m_pdc.pdc_parm    = parm    ;
	m_pdc.pdc_unavail = unavail ;

	++abc_maxh ;
	if ( abc_maxw < name.size() ) { abc_maxw = name.size() ; }

	pdcList.push_back( m_pdc ) ;
}


void abc::display_abc_sel( WINDOW * win )
{
	wattrset( win, cuaAttr[ AB ] ) ;
	mvwaddstr( win, 0, abc_col, abc_name.c_str() ) ;
	wattroff( win, cuaAttr[ AB ] ) ;
}


void abc::display_abc_unsel( WINDOW * win )
{
	wattrset( win, cuaAttr[ ABU ] ) ;
	mvwaddstr( win, 0, abc_col, abc_name.c_str() ) ;
	wattroff( win, cuaAttr[ ABU ] ) ;
}


void abc::display_pd( uint prow, uint pcol )
{
	string t ;
	int    i ;

	if ( !pd_created )
	{
		win = newwin( abc_maxh + 2, abc_maxw + 10, prow+1, pcol+abc_col ) ;
		wattrset( win, cuaAttr[ AB ] ) ;
		box( win, 0, 0 ) ;
		panel = new_panel( win ) ;
		for ( i = 0 ; i < pdcList.size() ; i++ )
		{
			t = d2ds( i+1 ) + ". " + pdcList.at( i ).pdc_name ;
			wattrset( win, cuaAttr[ PAC ] ) ;
			mvwaddstr( win, i + 1, 4 , t.c_str() ) ;
			wattroff( win, cuaAttr[ PAC ] ) ;
		}
		wattroff( win, cuaAttr[ AB ] ) ;
		pd_created = true ;
	}
	else
	{
		mvwin( win, prow+1, pcol+abc_col ) ;
		show_panel( panel ) ;
	}
}


void abc::hide_pd()
{
	hide_panel( panel ) ;
}


pdc abc::retrieve_pdChoice( uint row, uint col )
{
	uint y ;

	hide_pd()   ;
	y = row - 2 ;

	if ( y > (pdcList.size() - 1) ) { return pdc() ; }
	if ( (col < (abc_col + 2 )) || (col > ( abc_col + abc_maxw + 6 )) ) { return pdc() ; }

	return pdcList.at( y ) ;
}


int Box::box_init( int MAXW, int MAXD, string line )
{
	// Format of BOX entry in panels (FORMAT 1 VERSION 1 )
	// BOX  row col width depth cuaAttr  B-title
	// w1   w2  w3  w4    w5    w6       w8
	// BOX  7   7   41    22    N_WHITE  "Test Dynamic Area 1"

	int row    ;
	int col    ;
	int width  ;
	int depth  ;
	int colour ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string w6 ;

	string title ;

	w2 = word( line, 2 ) ;
	w3 = word( line, 3 ) ;
	w4 = word( line, 4 ) ;
	w5 = word( line, 5 ) ;
	w6 = word( line, 6 ) ;

	title  = strip( strip( subword( line, 7 ) ), 'B', '"' ) ;

	row   = ds2d( w2 ) ;
	col   = ds2d( w3 ) ;
	width = ds2d( w4 ) ;
	depth = ds2d( w5 ) ;

	if ( row > (MAXD - 2) )
	{
		log( "E", "Box outside panel area" << endl ) ;
		return 20 ;
	}

	if ( width > (MAXW - col+1) ) { width = (MAXW - col+1) ; } ;
	if ( depth > (MAXD - row+1) ) { depth = (MAXD - row+1) ; } ;

	colour = wordpos( w6, usrAttrNames ) ;
	if ( colour == 0 ) colour = B_GREEN  ;

	box_row    = row - 1 ;
	box_col    = col - 1 ;
	box_width  = width   ;
	box_depth  = depth   ;
	box_colour = usrAttr[ colour ] ;

	if ( title.size() > ( width - 4) ) title = substr( title, 1, ( width - 4 ) ) ;
	box_title        = " " + title + " "  ;
	box_title_offset = ( box_width - box_title.size() ) / 2 ;
	return 0 ;
}


void Box::display_box( WINDOW * win )
{
	wattrset( win, box_colour ) ;

	mvwaddch( win, box_row, box_col, ACS_ULCORNER ) ;
	mvwaddch( win, box_row, box_col + box_width - 1, ACS_URCORNER ) ;

	mvwaddch( win, box_row + box_depth - 1, box_col, ACS_LLCORNER ) ;
	mvwaddch( win, box_row + box_depth - 1, box_col + box_width - 1, ACS_LRCORNER ) ;

	mvwhline( win, box_row, (box_col + 1), ACS_HLINE, (box_width - 2) ) ;
	mvwhline( win, (box_row + box_depth - 1), (box_col + 1), ACS_HLINE, (box_width - 2) ) ;

	mvwvline( win, (box_row + 1), box_col, ACS_VLINE, (box_depth - 2) ) ;
	mvwvline( win, (box_row + 1), (box_col + box_width - 1 ), ACS_VLINE, (box_depth - 2) ) ;

	mvwaddstr( win, box_row, (box_col + box_title_offset), box_title.c_str() ) ;

	wattroff( win, box_colour ) ;
}
