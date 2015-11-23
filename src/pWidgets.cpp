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
		log( "E", "Field outside panel area.  row = " << row << " ZSCRMAXD " << MAXD << endl ; )
		return 20 ;
	}

	if ( isnumeric( w3 ) )                   { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                  { col = MAXW       ; }
	else if ( substr( w3, 1, 4 ) == "MAX-" ) { col = MAXW - ds2d( substr( w3, 5 ) ) ; }
	else 				         { return 20        ; }

	if ( isnumeric( w4 ) )                   { len = ds2d( w4 ) ; }
	else if ( w4 == "MAX" )                  { len = MAXW - col + 1 ; }
	else if ( substr( w4, 1, 4 ) == "MAX-" ) { len = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else 				         { return 20        ; }

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

	field_cua    = fType   ;
	field_prot   = cuaAttrProt [ fType ] ;
        field_row    = row - 1 ;
	field_col    = col - 1 ;
	field_length = len     ;

	if ( ( fType == CEF) || ( fType == NEF) || ( fType == DATAIN) ) { field_input = true ; }

	fieldOptsParse( RC, w6, field_caps, field_just, field_numeric, field_padchar, field_skip ) ;
	if ( RC > 0 ) { log( "E", "Error parsing options for field " << w7 << ". Options entry is " << w6 << endl ) ; }
	
	return RC ;
}


int dynArea::dynArea_init( int MAXW, int MAXD, string line )
{
	// Format of OPTION entry in panels (FORMAT 1 VERSION 1 )
	// DYNAREA row col width depth  A-name S-name  DATAIN() DATAOUT() USERMOD() DATAMOD()
	// w1      w2  w3  w4    w5     w6     w7      <------------keywords---------------->
	// DYNAREA 6   1   MAX   MAX-6  ZAREA  ZSHADOW DATAIN(01) DATAOUT(02) USERMOD(03) DATAMOD(04)

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

	if ( isnumeric( w4 ) )                   { width = ds2d( w4  )    ; }
	else if ( w4 == "MAX" )                  { width = MAXW - col + 1 ; }
	else if ( substr( w4, 1, 4 ) == "MAX-" ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else 				         { return 20              ; }

	if ( isnumeric( w5 ) )                   { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                  { depth = MAXD - row + 1 ; }
	else if ( substr( w5, 1, 4 ) == "MAX-" ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else 				         { return 20              ; }


	if ( row > MAXD )
	{
		log( "E", "Dynamic area outside panel area.  row = " << row << " MAXD " << MAXD ) ;
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

	if ( (dynArea_UserModsp || dynArea_DataModsp) && !dynArea_DataInsp ) { return 20 ; }

	dynArea_row         = row -1 ;
	dynArea_col         = col -1 ;
	dynArea_width       = width  ;
	dynArea_depth       = depth  ;
	dynArea_shadow_name = w7     ;

	return 0 ;
}


bool field::edit_field_insert( WINDOW * win, char ch, int col, bool Isrt )
{
	// If this is a dynamic area, we know at this point this is an input field, so field_dynDataInsp is true and there is an input attribute
	// byte at the start of the field
	//
	uint pos ;

	int i   ;
	int j   ;
	int l   ;

	pos = (col - field_col) ;
	if ( pos > field_value.size() )
	{
		j = pos - field_value.size() ;
		if ( field_value == "" )
		{
			field_value.append( j, ' ' ) ;
		}
		else
		{
			field_value.append( j, field_padchar ) ;
		}
	}

	if ( Isrt )
	{
		if ( field_dynArea )
		{
			l = field_value.size() ;
			j = 0                  ;
			for ( i = pos ; i < l ; i++ )
			{
				if ( (field_value[ i ]                       == field_dynDataIn )  ||
				     (field_dynDataOutsp && field_value[ i ] == field_dynDataOut)  ||
				     (field_dynUserModsp && field_value[ i ] == field_dynUserMod)  ||
				     (field_dynDataModsp && field_value[ i ] == field_dynDataMod )) { j = i ; break ; }
			}
			if ( j == 0 ) { j = l ; }
			j-- ;
			if ( field_value[ j ] == ' ' ) { field_value.erase( j, 1 ) ; }
			else                           { return false ;              }
			for ( i = pos ; i >= 0 ; i-- )
			{
				if ( (field_value[ i ]                       == field_dynDataIn)  || 
				     (field_dynUserModsp && field_value[ i ] == field_dynUserMod) ||
				     (field_dynDataModsp && field_value[ i ] == field_dynDataMod) ) { break ; }
			}
			if ( field_dynDataModsp )
			{
				if ( field_value[ i ] != field_dynDataMod )
				{
					if ( ch != ' ' ) { field_value[ i ] = field_dynDataMod ; }
					else if ( field_value.substr( pos, j-pos ) != string( j-pos, ' ' ) ) { field_value[ i ] = field_dynDataMod ; }
					else if ( field_dynUserModsp )                                       { field_value[ i ] = field_dynUserMod ; }
				}
			}
			else if ( field_dynUserModsp )
			{
				field_value[ i ] = field_dynUserMod ;
			}
		}
		else
		{
			if ( field_length == field_value.size() ) { return false ; }
		}
		field_value.insert( pos, 1, ch ) ;
	}
	else
	{
		if ( field_dynArea )
		{
			for ( i = pos ; i >= 0 ; i-- )
			{
				if ( (field_value[ i ]                       == field_dynDataIn)  || 
				     (field_dynUserModsp && field_value[ i ] == field_dynUserMod) ||
				     (field_dynDataModsp && field_value[ i ] == field_dynDataMod) ) { break ; }
			}
			if ( field_dynDataModsp )
			{
				if ( field_value[ i ] != field_dynDataMod )
				{
					if ( field_value[ pos ] != ch ) { field_value[ i ] = field_dynDataMod ; }
					else if ( field_dynUserModsp )  { field_value[ i ] = field_dynUserMod ; }
				}
			}
			else if ( field_dynUserModsp )
			{
				field_value[ i ] = field_dynUserMod ;
			}
				
		}
		field_value.replace( pos, 1, 1, ch ) ;
	}

	display_field( win ) ;
	field_changed = true ;
	return true ;
}


void field::edit_field_delete( WINDOW * win, int col )
{
	// If this is a dynamic area, we know at this point this is an input field, so field_dynDataInsp is true and there is an input attribute
	// byte at the start of the field
	//

	uint pos ;

	int i   ;
	int j   ;
	int l   ;
	
	pos = col - field_col ;
	if ( pos > field_value.size() ) return ;

	if ( field_dynArea )
	{
		l = field_value.size() ;
		j = 0                  ;
		for ( i = pos ; i < l ; i++ )
		{
			if ( (field_value[ i ]                       == field_dynDataIn  )  ||
			     (field_dynDataOutsp && field_value[ i ] == field_dynDataOut )  ||
			     (field_dynUserModsp && field_value[ i ] == field_dynUserMod )  ||
			     (field_dynDataModsp && field_value[ i ] == field_dynDataMod )  ) { j = i ; break ; }
		}
		if ( j == 0 ) { j = l ; }
		for ( i = pos ; i >= 0 ; i-- )
		{
			if ( (field_value[ i ]                       == field_dynDataIn)  || 
			     (field_dynUserModsp && field_value[ i ] == field_dynUserMod) ||
			     (field_dynDataModsp && field_value[ i ] == field_dynDataMod) ) { break ; }
		}
		if ( field_dynDataModsp )
		{
			if ( field_value[ i ] != field_dynDataMod )
			{
				if ( field_value.substr( pos, j-pos ) != string( j-pos, ' ' ) ) { field_value[ i ] = field_dynDataMod ; }
				else if ( field_dynUserModsp )                                  { field_value[ i ] = field_dynUserMod ; }
			}
		}
		else if ( field_dynUserModsp ) 
		{
			field_value[ i ] = field_dynUserMod ;
		}
		field_value.insert( j, 1, ' ' ) ;
	}

	field_value.erase( pos, 1 ) ;
	display_field( win ) ;
	field_changed = true ;
}


int field::edit_field_backspace( WINDOW * win, int col )
{
	// If this is a dynamic area, we know at this point this is an input field, so field_dynDataInsp is true and there is an input attribute
	// byte at the start of the field
	//

	uint pos ;

	int i   ;
	int j   ;
	int l   ;
	
	pos = col - field_col ;
	if ( (pos > field_value.size()) ) return --col ;

	if ( field_dynArea )
	{
		if ( (field_value[ pos-1 ]                       == field_dynDataIn  ) ||
		     (field_dynDataOutsp && field_value[ pos-1 ] == field_dynDataOut ) ||
		     (field_dynUserModsp && field_value[ pos-1 ] == field_dynUserMod ) ||
		     (field_dynDataModsp && field_value[ pos-1 ] == field_dynDataMod ) ) { return col ; }
		l = field_value.size() ;
		j = 0                  ;
		for ( i = pos ; i < l ; i++ )
		{
			if ((field_value[ i ]                       == field_dynDataIn  ) ||
			    (field_dynDataOutsp && field_value[ i ] == field_dynDataOut ) ||
			    (field_dynUserModsp && field_value[ i ] == field_dynUserMod ) ||
			    (field_dynDataModsp && field_value[ i ] == field_dynDataMod ) ) { j = i ; break ; }
		}
		if ( j == 0 ) { j = l ; }
		for ( i = pos ; i >= 0 ; i-- )
		{
			if ( (field_value[ i ]                       == field_dynDataIn  ) || 
			     (field_dynUserModsp && field_value[ i ] == field_dynUserMod ) ||
			     (field_dynDataModsp && field_value[ i ] == field_dynDataMod ) ) { break ; }
		}
		if ( field_dynDataModsp )
		{
			if ( field_value[ i ] != field_dynDataMod )
			{
				if ( field_value.substr( pos-1, j-pos+1 ) != string( j-pos+1, ' ' ) ) { field_value[ i ] = field_dynDataMod ; }
				else if ( field_dynUserModsp )                                        { field_value[ i ] = field_dynUserMod ; }
			}
		}
		else if ( field_dynUserModsp ) 
		{
			field_value[ i ] = field_dynUserMod ;
		}
		field_value.insert( j, 1, ' ' ) ;
	}

	field_value.erase( pos - 1, 1 ) ;
	display_field( win ) ;
	field_changed = true ;

	return --col ;
}


void field::field_blank( WINDOW * win )
{
	string blank( field_length, field_padchar ) ;
	mvwaddstr( win, field_row, field_col, blank.c_str() ) ;
}


void field::field_clear( WINDOW * win )
{
	field_value = ""     ;
	field_blank( win )   ;
	field_changed = true ;
}


void field::field_erase_eof( WINDOW * win, uint col )
{
	// If this is a dynamic area, we know at this point this is an input field, so field_dynDataInsp is true and there is an input attribute
	// byte at the start of the field
	//

	int pos ;
	int i   ;
	int j   ;
	int l   ;
	
	if ( ( field_col + field_value.size() ) < col ) return ;
	pos = (col - field_col) ;

	if ( field_dynArea )
	{
		l = field_value.size() ;
		j = 0                  ;
		for ( i = pos ; i < l ; i++ )
		{
			if ((field_value[ i ]                       == field_dynDataIn  ) ||
			    (field_dynDataOutsp && field_value[ i ] == field_dynDataOut ) ||
			    (field_dynUserModsp && field_value[ i ] == field_dynUserMod ) ||
			    (field_dynDataModsp && field_value[ i ] == field_dynDataMod ) ) { j = i ; break ; }
		}
		if ( j == 0 ) { j = l ; }
		for ( i = pos ; i >= 0 ; i-- )
		{
			if ( (field_value[ i ]                       == field_dynDataIn  ) || 
			     (field_dynUserModsp && field_value[ i ] == field_dynUserMod ) ||
			     (field_dynDataModsp && field_value[ i ] == field_dynDataMod ) ) { break ; }
		}
		if ( field_dynDataModsp )
		{
			if ( field_value[ i ] != field_dynDataMod )
			{
				if ( field_value.substr( pos, j-pos ) != string( j-pos, ' ' ) ) { field_value[ i ] = field_dynDataMod ; }
				else if ( field_dynUserModsp )                                  { field_value[ i ] = field_dynUserMod ; }
			}
		}
		else if ( field_dynUserModsp ) 
		{
			field_value[ i ] = field_dynUserMod ;
		}
		field_value = field_value.replace( pos, j-pos, j-pos, ' ' ) ;

	}
	else
	{
		field_blank( win ) ;
		field_value = field_value.substr( 0 , pos ) ;
	}
	
	display_field( win ) ;
	field_changed = true ;
}


int field::end_of_field( WINDOW * win, uint col )
{
	// If this is a dynamic area, we know at this point this is an input field, so field_dynDataInsp is true and there is an input attribute
	// byte at the start of the field
	//

	int l   ;
	int i   ;
	int j   ;
	int pos ;

	if ( field_dynArea )
	{
		l = field_value.size()  ;
		j = 0                   ;
		pos = (col - field_col) ;
		for ( i = pos ; i < l ; i++ )
		{
			if ( (field_value[ i ]                       == field_dynDataIn  ) ||
			     (field_dynDataOutsp && field_value[ i ] == field_dynDataOut ) ||
			     (field_dynUserModsp && field_value[ i ] == field_dynUserMod ) ||
			     (field_dynDataModsp && field_value[ i ] == field_dynDataMod ) ) { j = i ; break ; }
		}
		if ( j == 0 ) { j = l ; }
		for ( i = j-1 ; i >= 0 ; i-- )
		{
			if ( (field_value[ i ]                       == field_dynDataIn  ) || 
			     (field_dynUserModsp && field_value[ i ] == field_dynUserMod ) ||
			     (field_dynDataModsp && field_value[ i ] == field_dynDataMod ) ) { return field_col + i + 1 ; }
			if ( field_value[ i ] != ' ' )
			{
				if ( ( i == j - 1 ) ) { return field_col + i     ; }
				else                  { return field_col + i + 1 ; }
			}
		}
	}
	else
	{
		if ( field_value.size() == field_length )
		{
			return ( field_col + field_value.size() - 1 ) ;
		}
		else
		{
			return ( field_col + field_value.size() ) ;
		}
	}
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

	if ( strip( attrs ) == "RESET" )
	{
		field_usecua = true ;
		return 0 ;
	}
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
		else                         { return 20              ; }
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


bool field::field_dyna_input( uint col )
{
	// When this routine is called, we only know the field is a dynamic area.  DATAIN, DATAOUT, USERMOD and DATAMOD all need to be tested

	uint pos ;
	int i    ;

	if ( !field_dynDataInsp ) return false ;

	pos = (col - field_col) ;
	if ( (field_value[ pos ]                       == field_dynDataIn ) ||
	     (field_dynUserModsp && field_value[ pos ] == field_dynUserMod) ||
	     (field_dynDataModsp && field_value[ pos ] == field_dynDataMod) ) { return false ; }

	for ( i = pos ; i >= 0 ; i-- )
	{
		if ( (field_value[ i ]                       == field_dynDataIn ) ||
		     (field_dynUserModsp && field_value[ i ] == field_dynUserMod) ||
		     (field_dynDataModsp && field_value[ i ] == field_dynDataMod) ) { return true  ; }
		if ( field_dynDataOutsp  && field_value[ i ] == field_dynDataOut  ) { return false ; }
	}
	return false ;
}


int field::field_dyna_input_offset( uint col )
{
	// When this routine is called, we know the field is a dynamic area and dynDataIn is true

	uint pos ;
	int i    ;

	if ( col < field_col ) { pos = 0 ;                 }
	else                   { pos = (col - field_col) ; }

	for ( i = pos ; i < field_value.size() ; i++ )
	{
		if ( (field_value[ i ]                       == field_dynDataIn ) ||
		     (field_dynUserModsp && field_value[ i ] == field_dynUserMod) ||
		     (field_dynDataModsp && field_value[ i ] == field_dynDataMod) ) { return i + 1 ; }
	}
	return -1 ;
}


void field::display_field( WINDOW * win )
{
	// For non-dynamic area fields: if an input field, truncate if value size > field size else for output fields
	// display field size bytes and leave field value unchanged (necessary for table display fields)

	uint i   ;
	string t ;

	if ( field_dynArea )
	{
		if ( field_dynDataInsp || field_dynDataOutsp || field_dynUserModsp || field_dynDataModsp )
		{
			for ( i = 0 ; i < field_value.size() ; i++ )
			{
				wattrset( win, usrAttr[ field_shadow_value [ i ] ] ) ;
				if ( ( field_dynDataInsp  && field_value[ i ] == field_dynDataIn  ) || 
				     ( field_dynDataOutsp && field_value[ i ] == field_dynDataOut ) ||
				     ( field_dynUserModsp && field_value[ i ] == field_dynUserMod ) ||
				     ( field_dynDataModsp && field_value[ i ] == field_dynDataMod ) ) { mvwaddch( win, field_row, field_col+i, ' ' ) ; }
				else if ( isprint( field_value[ i ] ) ) { mvwaddch( win, field_row, field_col+i, field_value[ i ] ) ; }
				else                                    { mvwaddch( win, field_row, field_col+i, '.' ) ;              }
				wattroff( win, usrAttr[ field_shadow_value [ i ] ] ) ;
			}
		}
		else
		{
			for ( i = 0 ; i < field_value.size() ; i++ )
			{
				wattrset( win, usrAttr[ field_shadow_value [ i ] ] ) ;
				if ( isprint( field_value[ i ] ) ) { mvwaddch( win, field_row, field_col+i, field_value[ i ] ) ; }
				else                               { mvwaddch( win, field_row, field_col+i, '.' ) ;              }
				wattroff( win, usrAttr[ field_shadow_value [ i ] ] ) ;
			}
		}
	}
	else
	{
		field_blank( win ) ;
		if ( field_usecua ) { wattrset( win, cuaAttr[ field_cua ] ) ; }
		else                { wattrset( win, field_colour)          ; }
		if ( field_input )
		{
			if ( field_value.size() > field_length )
			{
				field_value = field_value.substr( 0, field_length-1 ) ;
			}
			for ( i = 0 ; i < field_value.size() ; i++ )
			{
				if ( !isprint( field_value[ i ] ) ) field_value.replace( i, 1, 1, '.' ) ;
			}
			if ( field_pwd ) { mvwaddstr( win, field_row, field_col, copies( "*", field_value.size()).c_str() ) ; }
			else		 { mvwaddstr( win, field_row, field_col, field_value.c_str() )                      ; }
		}
		else
		{
			if ( field_value.size() > field_length ) { t = field_value.substr( 0, field_length ) ; }
			else                                     { t = field_value                           ; }
			for ( i = 0 ; i < t.size() ; i++ )
			{
				if ( !isprint( t[ i ] ) ) t.replace( i, 1, 1, '.' ) ;
			}
			if ( field_pwd ) { mvwaddstr( win, field_row, field_col, copies( "*", t.size()).c_str() ) ; }
			else		 { mvwaddstr( win, field_row, field_col, t.c_str() )                      ; }
		}
		if ( field_usecua ) { wattroff( win, cuaAttr[ field_cua ] ) ; }
		else                { wattroff( win, field_colour )         ; }
	}
	//wrefresh( win ) ;
}


void literal::literal_display( WINDOW * win )
{
	wattrset( win, cuaAttr[ literal_cua ] ) ;
	mvwaddstr( win, literal_row, literal_col, literal_value.c_str() ) ;
	wattroff( win, cuaAttr[ literal_cua ] ) ;
	//wrefresh( win )  ;
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

	cuaType fType   ;

	w2 = word( line, 2 ) ;
	w3 = word( line, 3 ) ;
	w4 = word( line, 4 ) ;
	w5 = word( line, 5 ) ;

	if ( isnumeric( w2 ) )                   { row = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                  { row = MAXD       ; }
	else if ( substr( w2, 1, 4 ) == "MAX-" ) { row = MAXD - ds2d( substr( w2, 5 ) ) ; }
	else 				         { return 20        ; }

	if ( isnumeric( w3 ) )                   { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                  { col = MAXW       ; }
	else if ( substr( w3, 1, 4 ) == "MAX-" ) { col = MAXW - ds2d( substr( w3, 5 ) ) ; }
	else 				         { return 20        ; }

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
		literal_length = literal_value.size() ;
		l = (MAXW - col) / literal_length + 1 ;
		literal_value  = substr( copies( literal_value, l ), 1, MAXW-col+1 ) ;
	}
	else
	{
		literal_value  = strip( strip( subword( line, 5 ) ), 'B', '"' ) ;
	}
	literal_length = literal_value.size() ;
	if ( fType == PS )
	{
		literal_name = "ZPS01" + right( d2ds(++opt_field), 3, '0') ;
	}
	return 0 ;
}


void dynArea::setsize( int row, int col, int width, int depth )
{
	dynArea_row    = row - 1 ;
	dynArea_col    = col - 1 ;
	dynArea_width  = width   ;
	dynArea_depth  = depth   ;
}


void abc::add_pdc( string name, string run, string parm, string unavail )
{
	pdc m_pdc ;

	m_pdc.pdc_name    = name    ;
	m_pdc.pdc_run     = run     ;
	m_pdc.pdc_parm    = parm    ;
	m_pdc.pdc_unavail = unavail ;

	++abc_maxh ;
	if ( abc_maxw < name.size() ) abc_maxw = name.size() ;

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


void abc::display_pd()
{
	string t ;
	int    i ;

	if ( !pd_created )
	{
		win = newwin( abc_maxh + 2 , abc_maxw + 10, 1, abc_col ) ;
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
		update_panels()   ;
		pd_created = true ;
	}
	else
	{
		show_panel( panel ) ;
		update_panels()     ;
	}
}


void abc::hide_pd()
{
	hide_panel( panel ) ;
	update_panels()     ;
}


pdc abc::retrieve_pdc( uint row, uint col )
{
	uint y ;
	pdc t_pdc ;

	hide_pd()   ;
	y = row - 2 ;

	if ( y > (pdcList.size() - 1) ) return t_pdc ;
	if ( (col < (abc_col + 2 )) | (col > ( abc_col + abc_maxw + 6 )) ) return t_pdc ;

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


IFSTMNT::IFSTMNT( string s )
{
	// Format of the IF panel statement
	// IF ( &AAA=&BBBB )
	// IF ( &AAA = VALUE1,VALUE2 )
	// IF ( &AAA NE 'Hello','Goodbye' )
	// IF (.CURSOR = ZCMD)
	// IF (.MSG    = PSYS011)
	// IF ( &Z EQ .TRUE )
	// IF ( &Z EQ .FALSE ) .TRUE = "1" and .FALSE = "0"
	// rhs value lists only for EQ and NE (EQ only one needs to be true, NE all need to be true)


	int p1 ;
	int p2 ;

	bool  f_end ;

	string t    ;
	string comp ;

	if_RC    = 0     ;
	if_lhs   = ""    ;
	if_rhs.clear()   ;
	if_isvar.clear() ;
	if_stmnt = 0     ;
	if_true  = false ;
	if_else  = false ;
	if_istb  = false ;
	if_eq    = false ;
	if_ne    = false ;
	if_gt    = false ;
	if_lt    = false ;
	if_ge    = false ;
	if_le    = false ;
	if_ng    = false ;
	if_nl    = false ;

	p1 = s.find( '(' ) ;
	if ( p1 == string::npos ) { if_RC = 20 ; return ; }
	t = upper( strip( s.substr( 0, p1 ) ) ) ;
	if ( t != "IF" ) { if_RC = 20 ; return ; }
	s = strip( s.erase( 0, p1+1 ) ) ;

	p1 = s.find_first_of( "=><!" ) ;
	if ( p1 == string::npos )
	{
		p2 = s.find( ' ' ) ;
		if ( p2 == string::npos ) { if_RC = 20 ; return ; }
		if_lhs = upper( strip( s.substr( 0, p2 ) ) ) ;
		p1 = s.find_first_not_of( ' ', p2 ) ;
		if ( p1 == string::npos ) { if_RC = 20 ; return ; }
		p2 = s.find( ' ', p1 ) ;
		if ( p2 == string::npos ) { if_RC = 20 ; return ; }
	}
	else
	{		
		p2 = s.find_first_not_of( "=><!", p1 ) ;
		if ( p2 == string::npos ) { if_RC = 20 ; return ; }
		if_lhs = upper( strip( s.substr( 0, p1 ) ) ) ;
	}

	comp = s.substr( p1, p2-p1 ) ;
	s    = strip( s.erase( 0, p2 ) ) ;

	if ( words( if_lhs ) != 1 ) { if_RC = 20 ; return ; }
	if      ( if_lhs    == ".CURSOR" ) {}
	else if ( if_lhs    == ".MSG"    ) {}
	else if ( if_lhs[0] != '&' ) { if_RC = 20 ; return ; }
	else
	{
		if_lhs.erase( 0, 1 ) ;
		if ( !isvalidName( if_lhs ) ) { if_RC = 20 ; return ; }
	}

	if      ( comp == "="  ) { if_eq = true ; }
	else if ( comp == "EQ" ) { if_eq = true ; }
	else if ( comp == "!=" ) { if_ne = true ; }
	else if ( comp == "NE" ) { if_ne = true ; }
	else if ( comp == ">"  ) { if_gt = true ; }
	else if ( comp == "GT" ) { if_gt = true ; }
	else if ( comp == "<"  ) { if_lt = true ; }
	else if ( comp == "LT" ) { if_lt = true ; }
	else if ( comp == ">=" ) { if_ge = true ; }
	else if ( comp == "=>" ) { if_ge = true ; }
	else if ( comp == "GE" ) { if_ge = true ; }
	else if ( comp == "<=" ) { if_le = true ; }
	else if ( comp == "=<" ) { if_le = true ; }
	else if ( comp == "LE" ) { if_le = true ; }
	else if ( comp == "!>" ) { if_ng = true ; }
	else if ( comp == "NG" ) { if_ng = true ; }
	else if ( comp == "!<" ) { if_nl = true ; }
	else if ( comp == "NL" ) { if_nl = true ; }
	else                     { if_RC = 20 ; return ; }

	f_end = false ;

	while ( true )
	{
		if ( s[ 0 ] == '&' )
		{
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { if_RC = 20 ; return ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			t.erase( 0, 1 ) ;
			if ( !isvalidName( t ) ) { if_RC = 20 ; return ; }
			if_isvar.push_back( true ) ;
		}
		else if ( s[ 0 ]  == '\'' )
		{
			s.erase( 0, 1 ) ;
			p1 = s.find( '\'' ) ;
			if ( p1 == string::npos ) { if_RC = 20 ; return ; }
			t  = s.substr( 0, p1 ) ;
			s  = strip( s.erase( 0, p1+1 ) ) ;
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { if_RC = 20 ; return ; }
				f_end = true ;
			}
			if_isvar.push_back( false ) ;
		}
		else if ( s[ 0 ]  == '.' )
		{
			s.erase( 0, 1 ) ;
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { if_RC = 20 ; return ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			if      ( t == "TRUE" )  { t = "1" ; }
			else if ( t == "FALSE" ) { t = "0" ; }
			else    { if_RC = 20 ; return ; }
			if_isvar.push_back( false ) ;
		}
		else
		{
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { if_RC = 20 ; return ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			if ( t == "" || (t.find_first_of( " )" ) != string::npos) ) { if_RC = 20 ; return ; }
			if_isvar.push_back( false ) ;
		}
		if_rhs.push_back( t ) ;
		s = strip( s.erase( 0, p1+1 ) ) ;
		if ( s == "" && !f_end )  { if_RC = 20 ; return ; }
		if ( f_end ) { break ; }
	}
	if ( s != "" )  { if_RC = 20 ; return ; }
	if ( (!if_eq && !if_ne) && if_rhs.size() > 1 ) { if_RC = 20 ; return ; }
}


ASSGN::ASSGN( string s )
{
	// Format of the assignment panel statement
	// &AAA = &BBBB
	// &AAA = VALUE
	// &AAA = 'Quoted Value'
	// &AAA = .TRAIL | .HELP | .MSG | .CURSOR
	// .HELP | .MSG | .CURSOR = &BBB | VALUE | 'Quoted Value'
	// &AAA = UPPER( ABC )
	// &AAA = LENGTH( ABC )
	// &AAA = WORDS( ABC )  Number of words in the value of ABC
	// &A   = EXISTS( ABC ) True if file/directory in variable ABC exists
	// &A   = FILE( ABC )   True if file in variable ABC exists
	// &A   = DIR( ABC )    True if directory in variable ABC exists

	int p  ;
	int p1 ;
	
	as_RC      = 0     ;
	as_lhs     = ""    ;
	as_rhs     = ""    ;
	as_isvar   = false ;
	as_isattr  = false ;
	as_istb    = false ;
	as_retlen  = false ;
	as_upper   = false ;
	as_words   = false ;
	as_chkexst = false ;
	as_chkdir  = false ;
	as_chkfile = false ;
	
	p = s.find( '=' ) ;
	if ( p == string::npos ) { as_RC = 20 ; return ; }
	as_lhs = upper( strip( s.substr( 0, p ) ) ) ;
	if ( words( as_lhs ) != 1 ) { as_RC = 20 ; return ; }
	if      ( as_lhs == ".AUTOSEL" ) {}
	else if ( as_lhs == ".CURSOR" )  {}
	else if ( as_lhs == ".CSRROW" )  {}
	else if ( as_lhs == ".HELP" )    {}
	else if ( as_lhs == ".MSG" )     {}
	else if ( as_lhs == ".NRET" )    {}
	else if ( as_lhs.substr( 0, 6 ) == ".ATTR(" )
	{
		p1 = as_lhs.find( ')' ) ;
		if ( p1 == string::npos ) { as_RC = 20 ; return ; }
		as_lhs = strip( as_lhs.substr( 6, p1-6 ) ) ;
		if ( !isvalidName( as_lhs ) ) { as_RC = 20 ; return ; }
		as_isattr = true ;
	}
	else if ( as_lhs[0] == '&' )
	{
		as_lhs.erase( 0, 1 ) ;
		if ( !isvalidName( as_lhs ) ) { as_RC = 20 ; return ; }
	}
	else  { as_RC = 20 ; return ; }
	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s[ 0 ] == '&' )
	{

		if ( words( s ) != 1 ) { as_RC = 20 ; return ; }
		s.erase( 0, 1 ) ;
		s = upper( s )  ;
		if ( !isvalidName( s ) ) { as_RC = 20 ; return ; }
		as_rhs   = s   ;
		as_isvar = true ;
	}
	else if ( s[ 0 ]  == '\'' )
	{
		s.erase( 0, 1 ) ;
		p = s.find( '\'' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = s.substr( 0, p ) ;
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
	}
	else if ( upper( s.substr( 0, 4 ) ) == "DIR(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 4 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar  = true ;
		as_chkdir = true ;
	}
	else if ( upper( s.substr( 0, 7 ) ) == "EXISTS(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 7 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar   = true ;
		as_chkexst = true ;
	}
	else if ( upper( s.substr( 0, 5 ) ) == "FILE(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 5 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar   = true ;
		as_chkfile = true ;
	}
	else if ( upper( s.substr( 0, 7 ) ) == "LENGTH(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 7 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar  = true ;
		as_retlen = true ;
	}
	else if ( upper( s.substr( 0, 6 ) ) == "WORDS(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 6 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar = true ;
		as_words = true ;
	}
	else if ( upper( s.substr( 0, 6 ) ) == "UPPER(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 6 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar = true ;
		as_upper = true ;
	}
	else
	{
		if ( words( s ) != 1 ) { as_RC = 20 ; return ; }
		s = upper( s ) ;
		if ( s[ 0 ]  == '.' && ( s != ".TRAIL" && s != ".HELP" && s != ".MSG" && s != ".CURSOR" ) ) { as_RC = 20 ; return ; }
		as_rhs = s ;
	}
}


VPUTGET::VPUTGET( string s )
{
	int i  ;
	int j  ;
	int ws ;

	string w    ;
	string w1   ;
	string w2   ;
	string vars ;

	vpg_RC   = 0     ;
	vpg_vput = false ;
	vpg_vget = false ;
	vpg_pool = ASIS  ;


	s = upper( s )      ;
	w1 = word( s, 1 )   ;
	w2 = word( s, 2 )   ;
	w = subword( s, 2 ) ;

	if ( substr( w, 1, 1) == "(" )
	{
		i = pos( ")", w ) ;
		if ( i == 0 )  { vpg_RC = 20 ; return ; }
		vars = substr( w, 2, i-2 ) ;
		w    = strip( substr( w, i+1 ) ) ;
		ws   = words( vars ) ;
	}
	else
	{
		vars = w2 ;
		w    = subword( s, 3 ) ;
		ws   = 1  ;
	}
	for( j = 1; j <= ws ; j++ )
	{
		if ( !isvalidName( word( vars, j ) ) )  { vpg_RC = 20 ; return ; }
	}
	if ( w1 == "VPUT" ) { vpg_vput = true ; }
	else		    { vpg_vget = true ; }
	vpg_vars = vars ;

	if ( w == "ASIS" || w == "" ) { vpg_pool  = ASIS     ; }
	else if ( w == "SHARED" )     { vpg_pool  = SHARED   ; }
	else if ( w == "PROFILE" )    { vpg_pool  = PROFILE  ; }
	else                          { vpg_RC = 20 ; return ; }
}


VERIFY::VERIFY( string s )
{
	// VER (&VAR LIST A B C D)
	// VER (&VAR,LIST,A,B,C,D)
	// VER (&VAR NB LIST A B C D)
	// VER (&VAR NONBLANK LIST A B C D)
	// VER (&VAR PICT ABCD)
	// VER (&VAR HEX)
	// VER (&VAR OCT)

	int i     ;
	int p     ;
	string w  ;
	string w1 ;
	string w2 ;

	ver_RC      = 0     ;
	ver_nblank  = false ;
	ver_numeric = false ;
	ver_list    = false ;
	ver_pict    = false ;
	ver_hex     = false ;
	ver_octal   = false ;
	ver_tbfield = false ;

	s  = upper( s )      ;
	s  = subword( s, 2 ) ;
	w1 = word( s, 1 ) ;

	p = 0 ;
	while ( true )
	{
		p = s.find( ',', p ) ;
		if ( p == string::npos ) { break ; }
		s.replace( p, 1, 1, ' ' ) ;
		p++ ;
	}

	if ( w1.size() == 0 ) { ver_RC = 20 ; return ; }
	else if ( w1.size() == 1 )
	{
		if ( w1 != "(" ) { ver_RC = 20 ; return ; }
		s = subword( s, 2 )  ;
		if ( s[ 0 ] != '&' ) { ver_RC = 20 ; return ; }
		s = substr( s, 2 )  ;
	}
	else
	{
		if ( w1.substr( 0, 2 ) != "(&" ) { ver_RC = 20 ; return ; }
		s = substr( s, 3 ) ;
	}
	if ( s.back() != ')' ) { ver_RC = 20 ; return ; }
	s = substr( s, 1, s.size()-1 ) ;

	ver_field = word( s, 1 ) ;
	if ( !isvalidName( ver_field ) ) { ver_RC = 20 ; return ; }
	
	w2 = word( s, 2 ) ;
	
	if ( w2 == "NB" || w2 == "NONBLANK" ) { ver_nblank = true ; i = 3 ; }
	else                                  { i = 2 ;                     }

	while ( true )
	{
		w = word( s, i ) ;
		if ( w == "" ) break ;
		if      ( w == "NUM" )  { ver_numeric = true  ; }
		else if ( w == "LIST" ) { ver_list    = true  ; }
		else if ( w == "PICT" ) { ver_pict    = true  ; }
		else if ( w == "HEX" )  { ver_hex     = true  ; }
		else if ( w == "OCT" )  { ver_octal   = true  ; }
		else if ( substr( w, 1, 4 ) == "MSG=" ) { ver_msgid = substr( w, 5 ) ; }
		else if ( ver_pict ) { if ( ver_value != "" ) { ver_RC = 20 ; return ; } else ver_value = w ; }
		else if ( ver_list ) { ver_value = ver_value + " " + w ; }
		else    { ver_RC = 20 ; return ; }
		i++ ;
	}
	if ( !ver_nblank && !ver_numeric && !ver_list && !ver_pict && !ver_hex && !ver_octal )
	{
		ver_RC = 20 ;
		return      ;
	}
	if ( (ver_list || ver_pict) && ver_value == "" )
	{
		ver_RC = 20 ;
		return      ;
	}
}

TRUNC::TRUNC( string s )
{
	// Format of the TRUNC panel statement
	// &AAA = TRUNC( &BBB,'.' )
	// &AAA = TRUNC( &BBB, 3  )

	int    p ;
	string t ;

	trnc_RC   = 0   ;
	trnc_char = ' ' ;
	trnc_len  = 0   ;
	
	s = upper( s )    ;
	p = s.find( '=' ) ;
	if ( p == string::npos ) { trnc_RC = 20 ; return ; }
	trnc_field1 = strip( s.substr( 0, p ) ) ;
	if ( trnc_field1 == "" )     { trnc_RC = 20 ; return ; }
	if ( trnc_field1[0] != '&' ) { trnc_RC = 20 ; return ; }
	trnc_field1.erase( 0, 1 ) ;

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s.substr( 0, 6 ) != "TRUNC(" ) { trnc_RC = 20 ; return ; }
	s = strip( s.erase( 0, 6 ) ) ;
	if ( s == "" ) { trnc_RC = 20 ; return ; }
	p = s.find( ',' ) ;
	if ( p == string::npos ) { trnc_RC = 20 ; return ; }
	trnc_field2 = strip( s.substr( 0, p ) ) ;

	if ( trnc_field2 == "" )     { trnc_RC = 20 ; return ; }
	if ( trnc_field2[0] != '&' ) { trnc_RC = 20 ; return ; }
	trnc_field2.erase( 0, 1 ) ;
	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s == "" ) { trnc_RC = 20 ; return ; }

	if ( s[0] == '\'' )
	{
		if ( s.size() < 4 ) { trnc_RC = 20 ; return ; }
		trnc_char = s[ 1 ] ;
		if ( s[ 2 ] != '\'' ) { trnc_RC = 20 ; return ; }
		s = strip( s.erase( 0, 3 ) ) ;
		if ( s != ")" ) { trnc_RC = 20 ; return ; }
	}
	else
	{
		if ( s.size() < 2 ) { trnc_RC = 20 ; return ; }
		p = s.find( ')' ) ;
		if ( p == string::npos ) { trnc_RC = 20 ; return ; }
		t = strip( s.substr( 0, p ) ) ;
		s = strip( s.erase( 0, p ) )  ;
		if ( !datatype( t, 'W' ) ) { trnc_RC = 20 ; return ; }
		trnc_len = ds2d( t ) ;
		if ( trnc_len <= 0 ) { trnc_RC = 20 ; return ; }
		if ( s != ")" ) { trnc_RC = 20 ; return ; }
	}
	if ( !isvalidName( trnc_field1 ) || !isvalidName( trnc_field2 ) ) { trnc_RC = 20 ; return ; }

	return ;
}


TRANS::TRANS( string s )
{
	// Format of the TRANS panel statement ( change val1 to val2, * is everything else )
	// &AAA = TRANS( &BBB  val1,val2 ...  *,* )
	// &AAA = TRANS( &BBB  val1,val2 ...  *,'?' )
	// &AAA = TRANS( &BBB  val1,val2 ...  ) non-matching results in &AAA being set to null

	int    p  ;
	int    p1 ;
	int    p2 ;
	int    i  ;
	int    j  ;
	string v1 ;
	string v2 ;

	trns_RC   = 0   ;

	s = upper( s )    ;
	p = s.find( '=' ) ;
	if ( p == string::npos ) { trns_RC = 20 ; return ; }
	trns_field1 = strip( s.substr( 0, p ) ) ;
	if ( trns_field1 == "" )     { trns_RC = 20 ; return ; }
	if ( trns_field1[0] != '&' ) { trns_RC = 20 ; return ; }
	trns_field1.erase( 0, 1 ) ;

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s.substr( 0, 6 ) != "TRANS(" ) { trns_RC = 20 ; return ; }
	s = strip( s.erase( 0, 6 ) ) ;
	if ( s == "" ) { trns_RC = 20 ; return ; }
	p = s.find( ' ' ) ;
	if ( p == string::npos ) { trns_RC = 20 ; return ; }
	trns_field2 = strip( s.substr( 0, p ) ) ;

	if ( trns_field2 == "" )     { trns_RC = 20 ; return ; }
	if ( trns_field2[0] != '&' ) { trns_RC = 20 ; return ; }
	trns_field2.erase( 0, 1 ) ;

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s == "" ) { trns_RC = 20 ; return ; }
	if ( s.back() != ')' ) { trns_RC = 20 ; return ; }
	s = strip( s.erase( s.size()-1, 1 ) ) ;

	j = countc( s, ',' ) ;
	if ( j == 0 )  { trns_RC = 20 ; return ; }

	for ( i = 1 ; i <= j ; i++ )
	{
		p1 = s.find( ',' ) ;
		v1 = strip( s.substr( 0, p1 ) ) ;
		s  = strip( s.erase( 0, p1+1 ) ) ;
		p2 = s.find( ' ' ) ;
		if ( p2 == string::npos )
		{
			v2 = s  ;
			s  = "" ;
		}
		else
		{
			v2 = strip( s.substr( 0, p2 ) ) ;
			s  = strip( s.erase( 0, p2+1 ) ) ;
		}
		if ( tlst.find( v1 ) != tlst.end() ) { trns_RC = 20 ; return ; }
		if ( words( v1 ) != 1 ) { trns_RC = 20 ; return ; }
		if ( words( v2 ) != 1 ) { trns_RC = 20 ; return ; }
		tlst[ v1 ] = v2 ;
	}
	if ( strip( s ) != "" ) { trns_RC = 20 ; return ; }
	if ( !isvalidName( trns_field1 ) || !isvalidName( trns_field2 ) ) { trns_RC = 20 ; return ; }
}


pnts::pnts( string s )
{
	// Format of the PNTS panel entry (point-and-shoot entries)
	// FIELD(fld) VAR(var) VAL(value)
	// fld and var must be defined in the panel

	int p1 ;
	int p2 ;
	
	pnts_RC = 0 ;

	s  = upper( s )         ;
	p1 = pos( "FIELD(", s ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", s, p1 ) ;
		if ( p2 == 0 ) { pnts_RC = 20 ; return ; }
		pnts_field = strip( substr( s, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
		if ( !isvalidName( pnts_field ) ) {  pnts_RC = 20 ; return ; }
		s = delstr( s, p1, (p2 - p1 + 1) ) ;
	}

	p1 = pos( "VAR(", s ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", s, p1 ) ;
		if ( p2 == 0 ) { pnts_RC = 20 ; return ; }
		pnts_var = strip( substr( s, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
		if ( !isvalidName( pnts_var ) ) {  pnts_RC = 20 ; return ; }
		s = delstr( s, p1, (p2 - p1 + 1) ) ;
	}

	p1 = pos( "VAL(", s ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", s, p1 ) ;
		if ( p2 == 0 ) { pnts_RC = 20 ; return ; }
		pnts_val = strip( substr( s, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
		s = strip( delstr( s, p1, (p2 - p1 + 1) ) ) ;
	}

	if ( s != "" || pnts_field == "" || pnts_var == "" || pnts_val == "" ) { pnts_RC = 20 ; return ; }
}


