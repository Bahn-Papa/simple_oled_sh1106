
#pragma once

//##########################################################################
//#
//#		simple_oled_sh1106.h
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

#include <stdint.h>


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

		uint8_t MaxTextLines( void );
		uint8_t MaxTextColumns( void );

		void Print(   char* strText );
		void PrintLn( char* strText );

		inline void Print( const char* strText )
		{
			Print( (char *)strText );
		};

		inline void PrintLn( const char* strText )
		{
			PrintLn( (char *)strText );
		};


		void Clear( void );
		void ClearLine( void );

		inline void SetCursor( uint8_t ui8TextLine, uint8_t ui8TextColumn )
		{
			SetCursor( ui8TextLine, ui8TextColumn, true );
		}

		inline void Home( void )
		{
			SetCursor( 0, 0, true );
		};

		void SetInverse( bool bInverse );
		void Flip( bool bFlip );

		inline void SetInverseFont( bool bInverse )
		{
			m_bInverse = bInverse;
		};

		void SetPrintModeOverwriteSameLine( void );
		void SetPrintModeOverwriteNextLine( void );
		void SetPrintModeScrollLine( void );

		void SetDisplayColumnOffset( uint8_t ui8Offset );


	private:
		uint8_t		m_ui8TextLine;
		uint8_t		m_ui8TextColumn;
		uint8_t		m_ui8PrintMode;
		uint8_t		m_ui8LineOffset;
		bool		m_bInverse;

		void SetCursor( uint8_t ui8TextLine, uint8_t ui8TextColumn, bool bWithOffset );
		void NextLine( bool bShiftLine );
		void SendCommand( uint8_t ui8OpCode );
		void SendCommand( uint8_t ui8OpCode, uint8_t ui8Parameter );
		void ShiftDisplayOneLine( void );
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern SimpleDisplayClass	g_clDisplay;
