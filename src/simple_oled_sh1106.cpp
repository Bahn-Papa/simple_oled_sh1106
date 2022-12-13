//##########################################################################
//#
//#		simple_oled_sh1106.cpp
//#
//#-------------------------------------------------------------------------
//#
//#	This class is used to control an OLED display with the sh1106 chipset
//#	via the I²C bus.
//#	Supported are only simple text output and some auxiliary functions,
//#	e.g.: clear display, clear line, position cursor, etc.
//#
//#-------------------------------------------------------------------------
//#
//#	Diese Klasse dient der Ansteuerung eines OLED Display mit dem
//#	Chipsatz sh1106 über den I²C Bus.
//#	Unterstütz werden nur simple Textausgaben und einige Hilfsfunktionen,
//#	wie: Display löschen, Zeile löschen, Cursor positionieren usw.
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.04	Date: 13.12.2022
//#
//#	Implementation:
//#		-	move function PrintChar() to the public section
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.03	Date: 30.01.2022
//#
//#	Implementation:
//#		-	add functions to print text that is stored in PROGMEM
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.02	Date: 28.01.2022
//#
//#	Implementation:
//#		-	change void Init( void ) to uint8_t Init( uint8_t address )
//#			the function will now check, if the given address is valid
//#			and if a display is connected
//#			if all is okay then '0' is returned
//#			otherwise an error code is retruned
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.01	Date: 21.01.2022
//#
//#	Implementation:
//#		-	Add function ClearLine( uint8_t usLineToClear )
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.0	Date: 18.01.2022
//#
//#	Implementation:
//#		-	First implementation of the class 'SimpleDisplayClass'.
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include <avr/pgmspace.h>
#include <Wire.h>

#include "simple_oled_sh1106.h"
#include "font.h"


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define DISPLAY_LINES					64
#define DISPLAY_COLUMNS					132

#define	TEXT_LINES						8
#define TEXT_COLUMNS					16

#define PIXELS_CHAR_HEIGHT				8
#define PIXELS_CHAR_WIDTH				8

#define DISPLAY_LINE_OFFSET_MAX			63
#define DISPLAY_LINE_OFFSET_MIN			0
#define DISPLAY_LINE_OFFSET_DEFAULT		0

#define DISPLAY_COLUMN_OFFSET_MAX		3
#define DISPLAY_COLUMN_OFFSET_MIN		0
#define DISPLAY_COLUMN_OFFSET_DEFAULT	2

//----	Print Modes  -------------------------------------------------------
#define PM_OVERWRITE_SAME_LINE			1
#define PM_OVERWRITE_NEXT_LINE			2
#define PM_SCROLL_LINE					3


//--------------------------------------------------------------------------
//	Definitions for I²C protocol
//

//----	Prefix Codes  --------------------------------------------------
#define	PREFIX_NEXT_COMMAND				0x80
#define PREFIX_LAST_COMMAND				0x00
#define PREFIX_DATA						0x40

//----	Command Codes  -------------------------------------------------
#define	OPC_COLUMN_ADDRESS_LOW			0x00
#define OPC_COLUMN_ADDRESS_HIGH			0x10
#define OPC_SEG_ROTATION_RIGHT			0xA0
#define OPC_SEG_ROTATION_LEFT			0xA1
#define	OPC_ENTIRE_DISPLAY_NORMAL		0xA4
#define	OPC_ENTIRE_DISPLAY_ON			0xA5
#define OPC_MODE_NORMAL					0xA6
#define OPC_MODE_INVERSE				0xA7
#define OPC_DISPLAY_OFF					0xAE
#define OPC_DISPLAY_ON					0xAF
#define	OPC_PAGE_ADDRESS				0xB0
#define OPC_OUTPUT_SCAN_NORMAL			0xC0
#define OPC_OUTPUT_SCAN_INVERSE			0xC8
#define OPC_DISPLAY_LINE_OFFSET			0xD3

//----	Masks to prepare commands  -------------------------------------
#define MASK_PAGE_ADDRESS				0x0F
#define	MASK_COLUMN_ADDRESS_LOW			0x0F
#define MASK_COLUMN_ADDRESS_HIGH		0xF0

//----	Idx into command buffer  ---------------------------------------
#define	IDX_PAGE_ADDRESS				1
#define IDX_COLUMN_ADDRESS_LOW			3
#define IDX_COLUMN_ADDRESS_HIGH			5


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

SimpleDisplayClass	g_clDisplay	= SimpleDisplayClass();

uint8_t		g_usDisplayColumnOffset	= DISPLAY_COLUMN_OFFSET_DEFAULT;
uint8_t		g_arusPositionCommandBuffer[] =
	{
		PREFIX_NEXT_COMMAND,
		OPC_PAGE_ADDRESS,
		PREFIX_NEXT_COMMAND,
		OPC_COLUMN_ADDRESS_LOW,
		PREFIX_LAST_COMMAND,
		OPC_COLUMN_ADDRESS_HIGH
	};


////////////////////////////////////////////////////////////////////////////
//
//	CLASS: SimpleDisplayClass
//


//**************************************************************************
//	Constructor
//--------------------------------------------------------------------------
//	description
//
SimpleDisplayClass::SimpleDisplayClass()
{
}


//**************************************************************************
//	Init
//--------------------------------------------------------------------------
//	The Function initializes the class, sets the display in default
//	operation mode, switches the display 'on', clears the display and
//	sets the cursor to home position (top left corner).
//
uint8_t SimpleDisplayClass::Init( uint8_t address )
{
	uint8_t	usError;


	//------------------------------------------------------------------
	//	set initial values for internal variables
	//
	m_usTextLine	= 0;
	m_usTextColumn	= 0;
	m_usPrintMode	= PM_SCROLL_LINE;
	m_usLineOffset	= 0;
	m_bInverse		= false;

	//------------------------------------------------------------------
	//	Check the given address
	//
	if( !(DISPLAY_ADDRESS == address) || (SECOND_DISPLAY_ADDRESS == address) )
	{
		//----------------------------------------------------------
		//	no valid address
		//
		return( 1 );
	}

	Wire.begin();

	//------------------------------------------------------------------
	//	Check if Display can be connected under the given address
	//
	Wire.beginTransmission( address );
	usError = Wire.endTransmission();

	if( 0 == usError )
	{
		//----------------------------------------------------------
		//	YES the display can be connected with the given address
		//	so initialize the display
		//
		m_usAddress	= address;

		SendCommand( OPC_DISPLAY_LINE_OFFSET, 0 );
		SendCommand( OPC_ENTIRE_DISPLAY_NORMAL );
		SendCommand( OPC_DISPLAY_ON );
		SendCommand( OPC_SEG_ROTATION_RIGHT );
		SendCommand( OPC_OUTPUT_SCAN_NORMAL );
		Clear();
	}
	
	return( usError );
}


//**************************************************************************
//	MaxTextLines
//--------------------------------------------------------------------------
//	The function returns the maximum number of text lines
//	that are possible on the display.
//
uint8_t SimpleDisplayClass::MaxTextLines( void )
{
	return( TEXT_LINES );
}


//**************************************************************************
//	MaxTextColumns
//--------------------------------------------------------------------------
//	The function returns the maximum number of text columns
//	that are possible on the display.
//
uint8_t SimpleDisplayClass::MaxTextColumns( void )
{
	return( TEXT_COLUMNS );
}


//**************************************************************************
//	SetCursor
//--------------------------------------------------------------------------
//	The function sets the cursor to the given line and column
//	valid values are:
//		line:	0 -  7
//		column:	0 - 15
//
void SimpleDisplayClass::SetCursor( uint8_t usTextLine, uint8_t usTextColumn )
{
	uint8_t		usAddressLow;
	uint8_t		usAddressHigh;


	if( (TEXT_LINES > usTextLine) && (TEXT_COLUMNS > usTextColumn) )
	{
		//------------------------------------------------------------------
		//	store the new cursor position
		//
		m_usTextLine	= usTextLine;
		m_usTextColumn	= usTextColumn;

		//------------------------------------------------------------------
		//	take care of the display line shift
		//	and correct the text line accordingly
		//
		usTextLine += m_usLineOffset;

		if( TEXT_LINES <= usTextLine )
		{
			usTextLine -= TEXT_LINES;
		}

		//------------------------------------------------------------------
		//	preparation for the command that will be send to the display
		//
		usTextLine &= MASK_PAGE_ADDRESS;
		g_arusPositionCommandBuffer[ IDX_PAGE_ADDRESS ] = usTextLine | OPC_PAGE_ADDRESS;

		//------------------------------------------------------------------
		//	calculate bit column
		//	the calculated bit column is the start column of a character
		//
		usTextColumn <<= 3;		//	multiply by 8
		usTextColumn  += g_usDisplayColumnOffset;

		//------------------------------------------------------------------
		//	preparation for the commands that will be send to the display
		//
		usAddressLow	 = usTextColumn & MASK_COLUMN_ADDRESS_LOW;
		g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] = usAddressLow | OPC_COLUMN_ADDRESS_LOW;

		usAddressHigh	 = usTextColumn & MASK_COLUMN_ADDRESS_HIGH;
		usAddressHigh >>= 4;
		g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_HIGH ] = usAddressHigh | OPC_COLUMN_ADDRESS_HIGH;

		//------------------------------------------------------------------
		//	now send the commands to position the cursor to the display
		//
		Wire.beginTransmission( m_usAddress );
		Wire.write( g_arusPositionCommandBuffer, sizeof( g_arusPositionCommandBuffer ) );
		Wire.endTransmission();
	}
}


//**************************************************************************
//	PrintChar
//--------------------------------------------------------------------------
//	This function will print the given character on the display starting at
//	the actual cursor position.
//	If the text contains a new line character ('\n') then the output will
//	continue at the beginning of the next line.
//	If the text ouput reaches the end of the line then depending of the
//	PrintMode the cursor will be set to the beginning of the (next) line
//	and the text output will continue there.
//
void SimpleDisplayClass::PrintChar( uint8_t usCharIdx )
{
	uint16_t		uiHelper;
	const uint8_t *	pusActualColumn;
	uint8_t			usLetterColumn;

	if( '\n' == usCharIdx )
	{
		NextLine( true );
	}
	else if( (' ' <= usCharIdx) && (128 > usCharIdx) )
	{
		//--------------------------------------------------------------
		//	if we reached the end of the line then depending of the
		//	PrintMode continue in the 'next line'
		//
		if( TEXT_COLUMNS <= m_usTextColumn )
		{
			NextLine( false );
		}

		//--------------------------------------------------------------
		//	this is a printable character, so calculate the pointer
		//	into the font array to that position where the bitmap of
		//	this character starts
		//
		uiHelper   = usCharIdx - 32;
		uiHelper <<= 3;	//	mit 8 multiplizieren

		pusActualColumn = &font8x8_simple[ 0 ] + uiHelper;

		//--------------------------------------------------------------
		//	transmit the bitmap of the character to the display
		//
		Wire.beginTransmission( m_usAddress );

		Wire.write( PREFIX_DATA );

		for( uint8_t idx = 0 ; idx < PIXELS_CHAR_WIDTH ; idx++ )
		{
			usLetterColumn = pgm_read_byte( pusActualColumn );
			pusActualColumn++;

			if( m_bInverse )
			{
				usLetterColumn = ~usLetterColumn;
			}

			Wire.write( usLetterColumn );
		}

		Wire.endTransmission();

		//--------------------------------------------------------------
		//	one character printed, so move cursor
		//
		m_usTextColumn++;
	}
}


//**************************************************************************
//	Print
//--------------------------------------------------------------------------
//	This function will print the given text on the display starting at
//	the actual cursor position.
//	If the text contains a new line character ('\n') then the output will
//	continue at the beginning of the next line.
//	If the text ouput reaches the end of the line then depending of the
//	PrintMode the cursor will be set to the beginning of the (next) line
//	and the text output will continue there.
//
void SimpleDisplayClass::Print( const __FlashStringHelper* cstrText )
{
	PGM_P	pText		= reinterpret_cast<PGM_P>( cstrText );
	uint8_t	usCharIdx	= pgm_read_byte( pText++ );

	while( 0x00 != usCharIdx )
	{
		PrintChar( usCharIdx );

		usCharIdx = pgm_read_byte( pText++ );
	}
}


//**************************************************************************
//	Print
//--------------------------------------------------------------------------
//	This function will print the given text on the display starting at
//	the actual cursor position and then sets the cursor to the beginning
//	of the next line.
//	If the text contains a new line character ('\n') then the output will
//	continue at the beginning of the next line.
//	If the text ouput reaches the end of the line then depending of the
//	PrintMode the cursor will be set to the beginning of the (next) line
//	and the text output will continue there.
//
void SimpleDisplayClass::PrintLn( const __FlashStringHelper* cstrText )
{
	Print( cstrText );
	NextLine( true );
}


//**************************************************************************
//	Print
//--------------------------------------------------------------------------
//	This function will print the given text on the display starting at
//	the actual cursor position.
//	If the text contains a new line character ('\n') then the output will
//	continue at the beginning of the next line.
//	If the text ouput reaches the end of the line then depending of the
//	PrintMode the cursor will be set to the beginning of the (next) line
//	and the text output will continue there.
//
void SimpleDisplayClass::Print( char* strText )
{
	uint8_t	usCharIdx	= *strText++;

	while( 0x00 != usCharIdx )
	{
		PrintChar( usCharIdx );

		usCharIdx = *strText++;
	}
}


//**************************************************************************
//	Print
//--------------------------------------------------------------------------
//	This function will print the given text on the display starting at
//	the actual cursor position and then sets the cursor to the beginning
//	of the next line.
//	If the text contains a new line character ('\n') then the output will
//	continue at the beginning of the next line.
//	If the text ouput reaches the end of the line then depending of the
//	PrintMode the cursor will be set to the beginning of the (next) line
//	and the text output will continue there.
//
void SimpleDisplayClass::PrintLn( char* strText )
{
	Print( strText );
	NextLine( true );
}


//**************************************************************************
//	Clear
//--------------------------------------------------------------------------
//	The function deletes all text shown on the display.
//
void SimpleDisplayClass::Clear( void )
{
	for( m_usTextLine = 0 ; m_usTextLine < TEXT_LINES ; m_usTextLine++ )
	{
		ClearLine();
	}

	//----------------------------------------------------------------------
	//	Set the display line offset back to the default value '0'.
	//	That means beginn to display the display with the top line.
	//
	m_usLineOffset = 0;

	SendCommand( OPC_DISPLAY_LINE_OFFSET, 0 );

	//----------------------------------------------------------------------
	//	set the cursor to home position
	//
	SetCursor( 0, 0 );
}


//**************************************************************************
//	ClearLine
//--------------------------------------------------------------------------
//	The function deletes the text line at the given cursor position and
//	sets the cursor to the bginning of that line.
//
void SimpleDisplayClass::ClearLine( uint8_t usLineToClear )
{
	//------------------------------------------------------------------
	//	at the end of the function the cursor will be positioned to
	//	the beginning of the line that will be cleared
	//
	m_usTextLine	= usLineToClear;
	m_usTextColumn	= 0;

	//------------------------------------------------------------------
	//	take care of the display line shift
	//	and correct the line to clear accordingly
	//
	usLineToClear += m_usLineOffset;

	if( TEXT_LINES <= usLineToClear )
	{
		usLineToClear -= TEXT_LINES;
	}

	//------------------------------------------------------------------
	//	preparation for the command that will be send to the display
	//		set cursor to actual line first column
	//
	usLineToClear &= MASK_PAGE_ADDRESS;
	g_arusPositionCommandBuffer[ IDX_PAGE_ADDRESS ] = usLineToClear | OPC_PAGE_ADDRESS;
	g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] = OPC_COLUMN_ADDRESS_LOW;
	g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_HIGH ] = OPC_COLUMN_ADDRESS_HIGH;

	//------------------------------------------------------------------
	//	now send the commands to position the cursor to the display
	//
	Wire.beginTransmission( m_usAddress );
	Wire.write( g_arusPositionCommandBuffer, sizeof( g_arusPositionCommandBuffer ) );
	Wire.endTransmission();

	for( uint8_t idx1 = 0 ; idx1 < 6 ; idx1++ )
	{
		Wire.beginTransmission( m_usAddress );

		Wire.write( PREFIX_DATA );

		for( uint8_t idx2 = 0 ; idx2 < 22 ; idx2++ )
		{
			Wire.write( 0x00 );
		}

		Wire.endTransmission();
	}

	//------------------------------------------------------------------
	//	set cursor to first text position of this line
	//
	g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] =	  OPC_COLUMN_ADDRESS_LOW
																| g_usDisplayColumnOffset;

	Wire.beginTransmission( m_usAddress );
	Wire.write( g_arusPositionCommandBuffer, sizeof( g_arusPositionCommandBuffer ) );
	Wire.endTransmission();
}


//**************************************************************************
//	SetInverse
//--------------------------------------------------------------------------
//	This function inverses the display, means every OLED pixel that is 'on'
//	will be turned 'off' and vice versa.
//
void SimpleDisplayClass::SetInverse( bool bInverse )
{
	if( bInverse )
	{
		SendCommand( OPC_MODE_INVERSE );
	}
	else
	{
		SendCommand( OPC_MODE_NORMAL );
	}
}


//**************************************************************************
//	Flip
//--------------------------------------------------------------------------
//	This function will turn the output on the display by 180 degree
//	and clears the display.
//
void SimpleDisplayClass::Flip( bool bFlip )
{
	if( bFlip )
	{
		SendCommand( OPC_SEG_ROTATION_LEFT );
		SendCommand( OPC_OUTPUT_SCAN_INVERSE );
	}
	else
	{
		SendCommand( OPC_SEG_ROTATION_RIGHT );
		SendCommand( OPC_OUTPUT_SCAN_NORMAL );
	}
	
	Clear();
}


//**************************************************************************
//	SetPrintModeOverwriteSameLine
//--------------------------------------------------------------------------
//	This function sets the PrintMode to overwrite same line.
//	Overwrite same line means:
//	if the text output comes to the end of a line then continue with the
//	output in the same line and overwrite an existing text.
//
void SimpleDisplayClass::SetPrintModeOverwriteSameLine( void )
{
	m_usPrintMode = PM_OVERWRITE_SAME_LINE;
}


//**************************************************************************
//	SetPrintModeOverwriteNextLine
//--------------------------------------------------------------------------
//	This function sets the PrintMode to overwrite next line.
//	Overwrite next line means:
//	if the text output comes to the end of a line then continue with the
//	output in the next line and perhaps overwrite an existing text.
//	if it was the last line of the display then jumpt to the first line and
//	continue the output there.
//
void SimpleDisplayClass::SetPrintModeOverwriteNextLine( void )
{
	m_usPrintMode = PM_OVERWRITE_NEXT_LINE;
}


//**************************************************************************
//	SetPrintModeScrollLine
//--------------------------------------------------------------------------
//	This function sets the PrintMode to scroll line.
//	Scroll line means:
//	if the text output comes to the end of a line then continue with the
//	output in the next line.
//	If it was the last line of the display then scroll all lines one up
//	discarding the first line, clear the last line and continue the output
//	in the cleared last line.
//
void SimpleDisplayClass::SetPrintModeScrollLine( void )
{
	m_usPrintMode = PM_SCROLL_LINE;
}


//**************************************************************************
//	SetDisplayColumnOffset
//--------------------------------------------------------------------------
//	With this function it is possible to adjust the display in left right
//	direction in a small range.
//
//	Background:
//	The OLED display that I used during development of this library has
//	132 columns of OLED pixels. The font I use has 8 pixels per character.
//	So 128 pixels are used for one text line. This leads to a left over
//	of 4 pixels that can be used to adjust the text output on the display.
//
void SimpleDisplayClass::SetDisplayColumnOffset( uint8_t usOffset )
{
	if( (DISPLAY_COLUMN_OFFSET_MIN <= usOffset) && (DISPLAY_COLUMN_OFFSET_MAX >= usOffset) )
	{
		g_usDisplayColumnOffset = usOffset;

		SendCommand( OPC_DISPLAY_LINE_OFFSET, usOffset );
	}
}


//**************************************************************************
//	SendCommand (private)
//--------------------------------------------------------------------------
//	This function will send a one byte command to the display.
//	A one byte command exists of
//		-	1 byte prefix
//		-	1 byte command code
//
void SimpleDisplayClass::SendCommand( uint8_t usOpCode )
{
	Wire.beginTransmission( m_usAddress );

	Wire.write( PREFIX_LAST_COMMAND );
	Wire.write( usOpCode );

	Wire.endTransmission();
}


//**************************************************************************
//	SendCommand (private)
//--------------------------------------------------------------------------
//	This function will send a two byte command to the display.
//	A two byte command exists of
//		-	1 byte prefix
//		-	1 byte command code
//		-	1 byte parameter
//
void SimpleDisplayClass::SendCommand( uint8_t usOpCode, uint8_t usParameter )
{
	Wire.beginTransmission( m_usAddress );

	Wire.write( PREFIX_LAST_COMMAND );
	Wire.write( usOpCode );
	Wire.write( usParameter );

	Wire.endTransmission();
}


//**************************************************************************
//	NextLine (private)
//--------------------------------------------------------------------------
//	The function will set the cursor to the beginning of the 'next print
//	line'. Which will be the 'next print line' depends on the print mode
//	and the function parameter.
//
void SimpleDisplayClass::NextLine( bool bShiftLine )
{
	m_usTextColumn	= 0;

	if( PM_SCROLL_LINE == m_usPrintMode )
	{
		//------------------------------------------------------------------
		//	PrintMode is scroll line
		//	if		the cursor is in the last line of the display,
		//	then	stay there and shift all other lines one up
		//	else	set cursor to the next line
		//
		if( (TEXT_LINES - 1) == m_usTextLine )
		{
			ShiftDisplayOneLine();
		}
		else
		{
			m_usTextLine++;
		}
	}
	else if( bShiftLine || (PM_OVERWRITE_NEXT_LINE == m_usPrintMode) )
	{
		//------------------------------------------------------------------
		//	if PrintMode is overwrite next line or bShiftLine is 'true'
		//	set cursor to the next line
		//
		m_usTextLine++;
	}

	//----------------------------------------------------------------------
	//	check if the cursor is set to the allowed line range
	//
	if( TEXT_LINES <= m_usTextLine )
	{
		m_usTextLine = 0;
	}

	//----------------------------------------------------------------------
	//	now set the cursor to the new position and
	//	if required clear the line
	//
	SetCursor( m_usTextLine, m_usTextColumn );

	if( PM_OVERWRITE_NEXT_LINE < m_usPrintMode )
	{
		ClearLine();
	}
}


//**************************************************************************
//	SetDisplayLineOffset (private)
//--------------------------------------------------------------------------
//	Die Funktion
//
void SimpleDisplayClass::ShiftDisplayOneLine( void )
{
	m_usLineOffset++;
	
	if( TEXT_LINES <= m_usLineOffset )
	{
		m_usLineOffset = 0;
	}

	SendCommand( OPC_DISPLAY_LINE_OFFSET, (m_usLineOffset << 3) );
}
