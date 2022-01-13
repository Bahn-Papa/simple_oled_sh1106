//##########################################################################
//#
//#		simple_oled_sh1106.cpp
//#
//#-------------------------------------------------------------------------
//#
//#	simple_oled_sh1106	Simple Lib to print plain text on an OLED display.
//#
//#	Copyright (C) 2022	Michael Pfeil
//#						Am Kuckhof 8, D - 52146 Wuerselen, GERMANY
//#
//#	This library is free software; you can redistribute it and/or
//#	modify it under the terms of the GNU Lesser General Public
//#	License as published by the Free Software Foundation; either
//#	version 2.1 of the License, or any later version.
//#
//#	This library is distributed in the hope that it will be useful,
//#	but WITHOUT ANY WARRANTY; without even the implied warranty of
//#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//#	See the GNU Lesser General Public License for more details.
//#
//#	You should have received a copy of the GNU Lesser General Public
//#	License along with this library; if not, write to the
//#	Free Software Foundation, Inc.,
//#	51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
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
//#	Version: 1.0	Date: 14.01.2022
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

#include <Wire.h>

#include "simple_oled_sh1106.h"
#include "font.h"


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define	DISPLAY_ADDRESS						60

#define DISPLAY_LINES						64
#define DISPLAY_COLUMNS						132

#define DISPLAY_LINE_OFFSET_MAX				63
#define DISPLAY_LINE_OFFSET_MIN				0
#define DISPLAY_LINE_OFFSET_DEFAULT			0

#define DISPLAY_COLUMN_OFFSET_MAX			3
#define DISPLAY_COLUMN_OFFSET_MIN			0
#define DISPLAY_COLUMN_OFFSET_DEFAULT		2

#define MASK_PAGE_ADDRESS			0x0F
#define	MASK_COLUMN_ADDRESS_LOW		0x0F
#define MASK_COLUMN_ADDRESS_HIGH	0xF0

//--------------------------------------------------------------------------
//	Definitions for I²C protocol
//
#define	PREFIX_NEXT_COMMAND		0x80
#define PREFIX_LAST_COMMAND		0x00
#define PREFIX_DATA				0x40

#define	OPC_COLUMN_ADDRESS_LOW		0x00
#define OPC_COLUMN_ADDRESS_HIGH		0x10
#define OPC_SEG_ROTATION_RIGHT		0xA0
#define OPC_SEG_ROTATION_LEFT		0xA1
#define	OPC_ENTIRE_DISPLAY_NORMAL	0xA4
#define	OPC_ENTIRE_DISPLAY_ON		0xA5
#define OPC_MODE_NORMAL				0xA6
#define OPC_MODE_INVERSE			0xA7
#define OPC_DISPLAY_OFF				0xAE
#define OPC_DISPLAY_ON				0xAF
#define	OPC_PAGE_ADDRESS			0xB0
#define OPC_OUTPUT_SCAN_NORMAL		0xC0
#define OPC_OUTPUT_SCAN_INVERSE		0xC8
#define OPC_DISPLAY_LINE_OFFSET		0xD3


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

SimpleDisplayClass	g_clDisplay	= SimpleDisplayClass();

uint8_t		g_ui8DisplayLineOffset		= DISPLAY_LINE_OFFSET_DEFAULT;
uint8_t		g_ui8DisplayColumnOffset	= DISPLAY_COLUMN_OFFSET_DEFAULT;


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
//	Die Funnktion initialisiert die Klasse.
//
void SimpleDisplayClass::Init( void )
{
	m_ui8TextLine	= 0;
	m_ui8TextColumn	= 0;
	m_bInverse	= false;

	Wire.begin();

	SendCommand( OPC_DISPLAY_LINE_OFFSET, 0 );
	SetOption( OPC_ENTIRE_DISPLAY_NORMAL );
	SetOption( OPC_DISPLAY_ON );
	Clear();
}


//**************************************************************************
//	Print
//--------------------------------------------------------------------------
//	Die Funnktion gibt den angegebenen Text im Display aus.
//	Dabei werden die Sonderzeichen CR and LF beachtet.
//
void SimpleDisplayClass::Print( char* strText )
{
	uint8_t		ui8CharIdx			= *strText++;
	uint16_t	ui16Helper;
	uint8_t *	pui8ActualColumn;
	uint8_t		ui8LetterColumn;

	while( 0x00 != ui8CharIdx )
	{
		if( '\n' == ui8CharIdx )
		{
			NextLine();
		}
		else if( (' ' <= ui8CharIdx) && (128 > ui8CharIdx) )
		{
			ui16Helper = ui8CharIdx - 32;
			ui16Helper <<= 3;	//	mit 8 multiplizieren
			pui8ActualColumn = &font8x8_simple[ 0 ] + ui16Helper;

			Wire.beginTransmission( DISPLAY_ADDRESS );

			Wire.write( PREFIX_DATA );

			for( uint8_t idx = 0 ; idx < 8 ; idx++ )
			{
				ui8LetterColumn = *pui8ActualColumn++;

				if( m_bInverse )
				{
					ui8LetterColumn = ~ui8LetterColumn;
				}

				Wire.write( ui8LetterColumn );
			}

			Wire.endTransmission();
		}

		ui8CharIdx = *strText++;
	}
}


//**************************************************************************
//	Print
//--------------------------------------------------------------------------
//	Die Funktion gibt den angegebenen Text im Display aus und wechselt
//	die Cursor-Position auf den Anfang der nächsten Zeile.
//
void SimpleDisplayClass::PrintLn( char* strText )
{
	Print( strText );
	NextLine();
}


//**************************************************************************
//	Clear
//--------------------------------------------------------------------------
//	Die Funktion löscht allen Text auf dem Display.
//
void SimpleDisplayClass::Clear( void )
{
	for( m_ui8TextLine = 0 ; m_ui8TextLine < MAX_TEXT_LINES ; m_ui8TextLine++ )
	{
		ClearLine();
	}

	Home();
}


//**************************************************************************
//	ClearLine
//--------------------------------------------------------------------------
//	Die Funktion löscht die Zeile in der der Cursor gerade steht.
//
void SimpleDisplayClass::ClearLine( void )
{
	SetCursor( m_ui8TextLine, 0, false );

	for( uint8_t idx1 = 0 ; idx1 < 11 ; idx1++ )
	{
		Wire.beginTransmission( DISPLAY_ADDRESS );

		Wire.write( PREFIX_DATA );

		for( uint8_t idx2 = 0 ; idx2 < 12 ; idx2++ )
		{
			Wire.write( 0x00 );
		}

		Wire.endTransmission();
	}
	
	SetCursor( m_ui8TextLine, 0, true );
}


//**************************************************************************
//	SetInverse
//--------------------------------------------------------------------------
//	Die Funktion invertiert das Display, so dass der Hintergrund hell ist
//	und die Schrift dunkel.
//
void SimpleDisplayClass::SetInverse( bool bInverse )
{
	if( bInverse )
	{
		SetOption( OPC_MODE_INVERSE );
	}
	else
	{
		SetOption( OPC_MODE_NORMAL );
	}
}


//**************************************************************************
//	Flip
//--------------------------------------------------------------------------
//	Die Funktion dreht die Ausgabe auf dem Display um 180 Grad.
//
void SimpleDisplayClass::Flip( bool bFlip )
{
	if( bFlip )
	{
		SetOption( OPC_SEG_ROTATION_LEFT );
		SetOption( OPC_OUTPUT_SCAN_INVERSE );
	}
	else
	{
		SetOption( OPC_SEG_ROTATION_RIGHT );
		SetOption( OPC_OUTPUT_SCAN_NORMAL );
	}
	
	Clear();
}


//**************************************************************************
//	SetPrintMode
//--------------------------------------------------------------------------
//	Die Funktion
//
void SimpleDisplayClass::SetPrintMode( uint8_t ui8Mode )
{
	if( (PM_OVERWRITE_SAME_LINE <= ui8Mode) && (PM_SCROLL_LINE >= ui8Mode) )
	{
		m_ui8PrintMode = ui8Mode;
	}
}


//**************************************************************************
//	SetDisplayColumnOffset
//--------------------------------------------------------------------------
//	Die Funktion
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
//	SetCursor
//--------------------------------------------------------------------------
//	Die Funktion setzt den Cursor auf die angegebene Zeile und Spalte.
//	Gültige Werte sind:
//		Zeile:	0 -  7
//		Spalte:	0 - 15
//
void SimpleDisplayClass::SetCursor( uint8_t ui8TextLine, uint8_t ui8TextColumn, bool bWithOffset )
{
	uint8_t		ui8AddressLow;
	uint8_t		ui8AddressHigh;
	

	if( (MAX_TEXT_LINES > ui8TextLine) && (MAX_TEXT_COLUMNS > ui8TextColumn) )
	{
		m_ui8TextLine	= ui8TextLine;
		m_ui8TextColumn	= ui8TextColumn;

		ui8TextLine			&= MASK_PAGE_ADDRESS;
		ui8TextLine			|= OPC_PAGE_ADDRESS;

		ui8TextColumn	  <<= 3;		//	mit 8 multiplizieren

		//------------------------------------------------------------------
		//	Normally the offset is used.
		//	Only for clearing a row the offset is NOT used.
		//
		if( bWithOffset )
		{
			ui8TextColumn += g_ui8DisplayColumnOffset;
		}

		ui8AddressLow	 = ui8TextColumn & MASK_COLUMN_ADDRESS_LOW;
		ui8AddressLow	|= OPC_COLUMN_ADDRESS_LOW;
	
		ui8AddressHigh	 = ui8TextColumn & MASK_COLUMN_ADDRESS_HIGH;
		ui8AddressHigh >>= 4;
		ui8AddressHigh	|= OPC_COLUMN_ADDRESS_HIGH;
	
		Wire.beginTransmission( DISPLAY_ADDRESS );
	
		Wire.write( PREFIX_NEXT_COMMAND );
		Wire.write( ui8TextLine );
		Wire.write( PREFIX_NEXT_COMMAND );
		Wire.write( ui8AddressLow );
		Wire.write( PREFIX_LAST_COMMAND );
		Wire.write( ui8AddressHigh );
	
		Wire.endTransmission();
	}
}


//**************************************************************************
//	SendCommand
//--------------------------------------------------------------------------
//	Die Funktion sendet ein Kommando an das Display.
//	Ein Kommando besteht aus
//		-	1 Byte Prefix
//		-	1 Byte Commando Code
//		-	1 Byte Parameter
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
//	SetOption
//--------------------------------------------------------------------------
//	Die Funktion sendet ein 1 Byte Commando an das Display
//
void SimpleDisplayClass::SetOption( uint8_t ui8OpCode )
{
	Wire.beginTransmission( DISPLAY_ADDRESS );

	Wire.write( PREFIX_LAST_COMMAND );
	Wire.write( ui8OpCode );

	Wire.endTransmission();
}


//**************************************************************************
//	NextLine
//--------------------------------------------------------------------------
//	Die Funktion setzt den Cursor an den Anfang der nächsten Zeile.
//	Sollte das Ende des Displays erreicht werden, dann wird der Cursor
//	in die erste Zeile gesetzt.
//
void SimpleDisplayClass::NextLine( void )
{
	m_ui8TextLine++;
	m_ui8TextColumn	= 0;

	if( MAX_TEXT_LINES <= m_ui8TextLine )
	{
		m_ui8TextLine = 0;
	}

	SetCursor( m_ui8TextLine, m_ui8TextColumn, true );
}


//**************************************************************************
//	SetDisplayLineOffset
//--------------------------------------------------------------------------
//	Die Funktion
//
void SimpleDisplayClass::SetDisplayLineOffset( uint8_t ui8Offset )
{
	if( (DISPLAY_LINE_OFFSET_MIN <= ui8Offset) && (DISPLAY_LINE_OFFSET_MAX >= ui8Offset) )
	{
		g_ui8DisplayLineOffset = ui8Offset;

		SendCommand( OPC_DISPLAY_LINE_OFFSET, ui8Offset );
	}
}
