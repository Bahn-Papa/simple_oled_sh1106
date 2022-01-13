
#pragma once

//##########################################################################
//#
//#		simple_oled_sh1106.h
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

#include <stdint.h>


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define	MAX_TEXT_LINES				8
#define MAX_TEXT_COLUMNS			16


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

//extern uint8_t	gc_ui8MaxTextLines;
//extern uint8_t	gc_ui8MaxTextColumns;


//==========================================================================
//
//		C L A S S   D E F I N I T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////////
//	CLASS: SimpleDisplayClass
//
class SimpleDisplayClass
{
	public:
		SimpleDisplayClass();

		void Init( void );

		void Print(   char* strText );
		void PrintLn( char* strText );

		inline void Print( const char* strText )
		{
			Print( (char *)strText );
		}

		inline void PrintLn( const char* strText )
		{
			PrintLn( (char *)strText );
		}


		void Clear( void );
		void ClearLine( void );

		inline void SetCursor( uint8_t ui8TextLine, uint8_t ui8TextColumn )
		{
			SetCursor( ui8TextLine, ui8TextColumn, true );
		}

		inline void Home( void )
		{
			SetCursor( 0, 0, true );
		}

		void SetInverse( bool bInverse );
		void Flip( bool bFlip );

		inline void SetInverseFont( bool bInverse )
		{
			m_bInverse = bInverse;
		}

		void SetDisplayLineOffset( uint8_t ui8Offset );
		void SetDisplayColumnOffset( uint8_t ui8Offset );


	private:
		uint8_t		m_ui8TextLine;
		uint8_t		m_ui8TextColumn;
		bool		m_bInverse;

		void SetCursor( uint8_t ui8TextLine, uint8_t ui8TextColumn, bool bWithOffset );
		void NextLine( void );
		void SendCommand( uint8_t ui8OpCode, uint8_t ui8Parameter );
		void SetOption( uint8_t ui8OpCode );
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern SimpleDisplayClass	g_clDisplay;
