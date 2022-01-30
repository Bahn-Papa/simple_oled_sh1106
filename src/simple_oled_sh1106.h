
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

#include <stdint.h>


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define	DISPLAY_ADDRESS					60
#define SECOND_DISPLAY_ADDRESS			61


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

		uint8_t Init( uint8_t address = DISPLAY_ADDRESS );

		uint8_t MaxTextLines( void );
		uint8_t MaxTextColumns( void );

		void Print(   const __FlashStringHelper* cstrText );
		void PrintLn( const __FlashStringHelper* cstrText );

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
		void ClearLine( uint8_t usLineToClear );
		inline void ClearLine( void )
		{
			ClearLine( m_usTextLine );
		};

		void SetCursor( uint8_t usTextLine, uint8_t usTextColumn );

		inline void Home( void )
		{
			SetCursor( 0, 0 );
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

		void SetDisplayColumnOffset( uint8_t usOffset );


	private:
		uint8_t		m_usAddress;
		uint8_t		m_usTextLine;
		uint8_t		m_usTextColumn;
		uint8_t		m_usPrintMode;
		uint8_t		m_usLineOffset;
		bool		m_bInverse;

		void NextLine( bool bShiftLine );
		void PrintChar( uint8_t usCharIdx );
		void SendCommand( uint8_t usOpCode );
		void SendCommand( uint8_t usOpCode, uint8_t usParameter );
		void ShiftDisplayOneLine( void );
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern SimpleDisplayClass	g_clDisplay;
