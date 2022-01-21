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

#define	DISPLAY_ADDRESS					60

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

uint8_t		g_ui8DisplayColumnOffset	= DISPLAY_COLUMN_OFFSET_DEFAULT;
uint8_t		g_arui8PositionCommandBuffer[] =
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
void SimpleDisplayClass::Init( void )
{
	m_ui8TextLine	= 0;
	m_ui8TextColumn	= 0;
	m_ui8PrintMode	= PM_SCROLL_LINE;
	m_ui8LineOffset	= 0;
	m_bInverse		= false;

	Wire.begin();

	SendCommand( OPC_DISPLAY_LINE_OFFSET, 0 );
	SendCommand( OPC_ENTIRE_DISPLAY_NORMAL );
	SendCommand( OPC_DISPLAY_ON );
	Clear();
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
void SimpleDisplayClass::SetCursor( uint8_t ui8TextLine, uint8_t ui8TextColumn )
{
	uint8_t		ui8AddressLow;
	uint8_t		ui8AddressHigh;


	if( (TEXT_LINES > ui8TextLine) && (TEXT_COLUMNS > ui8TextColumn) )
	{
		//------------------------------------------------------------------
		//	store the new cursor position
		//
		m_ui8TextLine	= ui8TextLine;
		m_ui8TextColumn	= ui8TextColumn;

		//------------------------------------------------------------------
		//	take care of the display line shift
		//	and correct the text line accordingly
		//
		ui8TextLine += m_ui8LineOffset;

		if( TEXT_LINES <= ui8TextLine )
		{
			ui8TextLine -= TEXT_LINES;
		}

		//------------------------------------------------------------------
		//	preparation for the command that will be send to the display
		//
		ui8TextLine &= MASK_PAGE_ADDRESS;
		g_arui8PositionCommandBuffer[ IDX_PAGE_ADDRESS ] = ui8TextLine | OPC_PAGE_ADDRESS;

		//------------------------------------------------------------------
		//	calculate bit column
		//	the calculated bit column is the start column of a character
		//
		ui8TextColumn <<= 3;		//	multiply by 8
		ui8TextColumn  += g_ui8DisplayColumnOffset;

		//------------------------------------------------------------------
		//	preparation for the commands that will be send to the display
		//
		ui8AddressLow	 = ui8TextColumn & MASK_COLUMN_ADDRESS_LOW;
		g_arui8PositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] = ui8AddressLow | OPC_COLUMN_ADDRESS_LOW;

		ui8AddressHigh	 = ui8TextColumn & MASK_COLUMN_ADDRESS_HIGH;
		ui8AddressHigh >>= 4;
		g_arui8PositionCommandBuffer[ IDX_COLUMN_ADDRESS_HIGH ] = ui8AddressHigh | OPC_COLUMN_ADDRESS_HIGH;

		//------------------------------------------------------------------
		//	now send the commands to position the cursor to the display
		//
		Wire.beginTransmission( DISPLAY_ADDRESS );
		Wire.write( g_arui8PositionCommandBuffer, sizeof( g_arui8PositionCommandBuffer ) );
		Wire.endTransmission();
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
void SimpleDisplayClass::Print( char* strText )
{
	uint8_t			ui8CharIdx			= *strText++;
	uint16_t		ui16Helper;
	const uint8_t *	pui8ActualColumn;
	uint8_t			ui8LetterColumn;

	while( 0x00 != ui8CharIdx )
	{
		if( '\n' == ui8CharIdx )
		{
			NextLine( true );
		}
		else if( (' ' <= ui8CharIdx) && (128 > ui8CharIdx) )
		{
			//--------------------------------------------------------------
			//	if we reached the end of the line then depending of the
			//	PrintMode continue in the 'next line'
			//
			if( TEXT_COLUMNS <= m_ui8TextColumn )
			{
				NextLine( false );
			}

			//--------------------------------------------------------------
			//	this is a printable character, so calculate the pointer
			//	into the font array to that position where the bitmap of
			//	this character starts
			//
			ui16Helper = ui8CharIdx - 32;
			ui16Helper <<= 3;	//	mit 8 multiplizieren
			pui8ActualColumn = &font8x8_simple[ 0 ] + ui16Helper;

			//--------------------------------------------------------------
			//	transmit the bitmap of the character to the display
			//
			Wire.beginTransmission( DISPLAY_ADDRESS );

			Wire.write( PREFIX_DATA );

			for( uint8_t idx = 0 ; idx < PIXELS_CHAR_WIDTH ; idx++ )
			{
				ui8LetterColumn = pgm_read_byte( pui8ActualColumn );
				pui8ActualColumn++;

				if( m_bInverse )
				{
					ui8LetterColumn = ~ui8LetterColumn;
				}

				Wire.write( ui8LetterColumn );
			}

			Wire.endTransmission();

			//--------------------------------------------------------------
			//	one character printed, so move cursor
			//
			m_ui8TextColumn++;
		}

		ui8CharIdx = *strText++;
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
	for( m_ui8TextLine = 0 ; m_ui8TextLine < TEXT_LINES ; m_ui8TextLine++ )
	{
		ClearLine();
	}

	//----------------------------------------------------------------------
	//	Set the display line offset back to the default value '0'.
	//	That means beginn to display the display with the top line.
	//
	m_ui8LineOffset = 0;

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
void SimpleDisplayClass::ClearLine( uint8_t ui8LineToClear )
{
	//------------------------------------------------------------------
	//	at the end of the function the cursor will be positioned to
	//	the beginning of the line that will be cleared
	//
	m_ui8TextLine	= ui8LineToClear;
	m_ui8TextColumn	= 0;

	//------------------------------------------------------------------
	//	take care of the display line shift
	//	and correct the line to clear accordingly
	//
	ui8LineToClear += m_ui8LineOffset;

	if( TEXT_LINES <= ui8LineToClear )
	{
		ui8LineToClear -= TEXT_LINES;
	}

	//------------------------------------------------------------------
	//	preparation for the command that will be send to the display
	//		set cursor to actual line first column
	//
	ui8LineToClear &= MASK_PAGE_ADDRESS;
	g_arui8PositionCommandBuffer[ IDX_PAGE_ADDRESS ] = ui8LineToClear | OPC_PAGE_ADDRESS;
	g_arui8PositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] = OPC_COLUMN_ADDRESS_LOW;
	g_arui8PositionCommandBuffer[ IDX_COLUMN_ADDRESS_HIGH ] = OPC_COLUMN_ADDRESS_HIGH;

	//------------------------------------------------------------------
	//	now send the commands to position the cursor to the display
	//
	Wire.beginTransmission( DISPLAY_ADDRESS );
	Wire.write( g_arui8PositionCommandBuffer, sizeof( g_arui8PositionCommandBuffer ) );
	Wire.endTransmission();

	for( uint8_t idx1 = 0 ; idx1 < 6 ; idx1++ )
	{
		Wire.beginTransmission( DISPLAY_ADDRESS );

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
	g_arui8PositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] =	  OPC_COLUMN_ADDRESS_LOW
																| g_ui8DisplayColumnOffset;

	Wire.beginTransmission( DISPLAY_ADDRESS );
	Wire.write( g_arui8PositionCommandBuffer, sizeof( g_arui8PositionCommandBuffer ) );
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
	m_ui8PrintMode = PM_OVERWRITE_SAME_LINE;
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
	m_ui8PrintMode = PM_OVERWRITE_NEXT_LINE;
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
	m_ui8PrintMode = PM_SCROLL_LINE;
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
void SimpleDisplayClass::SetDisplayColumnOffset( uint8_t ui8Offset )
{
	if( (DISPLAY_COLUMN_OFFSET_MIN <= ui8Offset) && (DISPLAY_COLUMN_OFFSET_MAX >= ui8Offset) )
	{
		g_ui8DisplayColumnOffset = ui8Offset;

		SendCommand( OPC_DISPLAY_LINE_OFFSET, ui8Offset );
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
void SimpleDisplayClass::SendCommand( uint8_t ui8OpCode )
{
	Wire.beginTransmission( DISPLAY_ADDRESS );

	Wire.write( PREFIX_LAST_COMMAND );
	Wire.write( ui8OpCode );

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
void SimpleDisplayClass::SendCommand( uint8_t ui8OpCode, uint8_t ui8Parameter )
{
	Wire.beginTransmission( DISPLAY_ADDRESS );

	Wire.write( PREFIX_LAST_COMMAND );
	Wire.write( ui8OpCode );
	Wire.write( ui8Parameter );

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
	m_ui8TextColumn	= 0;

	if( PM_SCROLL_LINE == m_ui8PrintMode )
	{
		//------------------------------------------------------------------
		//	PrintMode is scroll line
		//	if		the cursor is in the last line of the display,
		//	then	stay there and shift all other lines one up
		//	else	set cursor to the next line
		//
		if( (TEXT_LINES - 1) == m_ui8TextLine )
		{
			ShiftDisplayOneLine();
		}
		else
		{
			m_ui8TextLine++;
		}
	}
	else if( bShiftLine || (PM_OVERWRITE_NEXT_LINE == m_ui8PrintMode) )
	{
		//------------------------------------------------------------------
		//	if PrintMode is overwrite next line or bShiftLine is 'true'
		//	set cursor to the next line
		//
		m_ui8TextLine++;
	}

	//----------------------------------------------------------------------
	//	check if the cursor is set to the allowed line range
	//
	if( TEXT_LINES <= m_ui8TextLine )
	{
		m_ui8TextLine = 0;
	}

	//----------------------------------------------------------------------
	//	now set the cursor to the new position and
	//	if required clear the line
	//
	SetCursor( m_ui8TextLine, m_ui8TextColumn );

	if( PM_OVERWRITE_NEXT_LINE < m_ui8PrintMode )
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
	m_ui8LineOffset++;
	
	if( TEXT_LINES <= m_ui8LineOffset )
	{
		m_ui8LineOffset = 0;
	}

	SendCommand( OPC_DISPLAY_LINE_OFFSET, (m_ui8LineOffset << 3) );
}
