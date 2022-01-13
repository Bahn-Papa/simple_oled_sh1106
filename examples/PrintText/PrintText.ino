//##########################################################################
//#
//#		PrintText.ino
//#
//#	Dieses Programm schreibt Text auf ein OLED Display.
//#	Dabei wird die Funktionalität aus der simple_OLED_sh1106 Library
//#	genutzt.
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include <stdint.h>
#include <simple_oled_sh1106.h>


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

char g_buffer[ 20 ];


//**************************************************************************
//	setup
//--------------------------------------------------------------------------
//	description
//
void setup()
{
	g_clDisplay.Init();
}


//**************************************************************************
//	loop
//--------------------------------------------------------------------------
//	description
//
void loop()
{
	g_clDisplay.Print( "Print Text Demo" );

	delay( 2000 );

	g_clDisplay.SetCursor( 2, 0 );
	g_clDisplay.PrintLn( "Normal Text\n" );

	delay( 1000 );

	g_clDisplay.SetInverseFont( true );
	g_clDisplay.Print( "Inverse Font" );

	delay( 3000 );

	g_clDisplay.SetCursor( 6, 0 );
	g_clDisplay.Print( "Clear this Line" );
	g_clDisplay.SetInverseFont( false );

	delay( 2000 );

	g_clDisplay.ClearLine();

	delay( 1000 );

	g_clDisplay.Print( "Clear Display" );

	delay( 2000 );

	g_clDisplay.Clear();

	delay( 500 );

	g_clDisplay.SetInverseFont( true );
	g_clDisplay.SetCursor( 0, 0 );
	g_clDisplay.Print( "*" );
	g_clDisplay.SetCursor( 0, 15 );
	g_clDisplay.Print( "*" );
	g_clDisplay.SetCursor( 7, 0 );
	g_clDisplay.Print( "*" );
	g_clDisplay.SetCursor( 7, 15 );
	g_clDisplay.Print( "*" );
	g_clDisplay.SetInverseFont( false );

	delay( 500 );

	for( uint8_t idx = 0 ; idx < MAX_TEXT_LINES ; idx++ )
	{
		g_clDisplay.SetCursor( idx, 4 );

		sprintf( g_buffer, "Zeile %d", idx );
		g_clDisplay.Print( g_buffer );

		delay( 250 );
	}

	delay( 2000 );

	g_clDisplay.Clear();

	delay( 500 );

	g_clDisplay.SetCursor( 4, 0 );
	g_clDisplay.Print( "Column" );
	g_clDisplay.SetCursor( 3, 0 );
	
	for( uint8_t idx = 0 ; idx < MAX_TEXT_COLUMNS ; idx++ )
	{
		if( 9 < idx )
		{
			g_clDisplay.SetCursor( 2, idx );
			g_clDisplay.Print( "1" );

			sprintf( g_buffer, "%d", idx - 10 );
			g_clDisplay.SetCursor( 3, idx );
		}
		else
		{
			sprintf( g_buffer, "%d", idx );
		}

		g_clDisplay.Print( g_buffer );
		
		delay( 250 );
	}

	delay( 2000 );

	g_clDisplay.Clear();

	delay( 2000 );
}
