//##########################################################################
//#
//#		PrintText.ino
//#
//#	This program will write Text on an OLED display and demonstrates
//#	some options of the simple_oled_sh1106 library, like
//#		- Cursor positioning
//#		- normal and inverse text font
//#
//#-------------------------------------------------------------------------
//#
//#	Dieses Programm schreibt Text auf ein OLED Display und zeigt dabei
//#	einige Möglichkeiten aus der simple_OLED_sh1106 Library:
//#		- Cursor positionieren
//#		- normale und inverse Textausgabe
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
	g_clDisplay.Print( F( "Print Text Demo" ) );

	delay( 2000 );

	//----------------------------------------------------------------------
	//	positioning the cursor to the beginning of the third line
	//	print text in normal font mode
	//
	g_clDisplay.SetCursor( 2, 0 );
	g_clDisplay.PrintLn( F( "Normal Text\n" ) );

	delay( 1000 );

	//----------------------------------------------------------------------
	//	now print some text in inverse font mode
	//
	g_clDisplay.SetInverseFont( true );
	g_clDisplay.Print( F( "Inverse Font" ) );

	delay( 3000 );

	//----------------------------------------------------------------------
	//	some additional text to demonstrate how to clear a line
	//
	g_clDisplay.SetCursor( 6, 0 );
	g_clDisplay.Print( F( "Clear this Line" ) );
	g_clDisplay.SetInverseFont( false );

	delay( 2000 );

	//----------------------------------------------------------------------
	//	this will clear the line where the cursor is in
	//
	g_clDisplay.ClearLine();

	delay( 1000 );

	//----------------------------------------------------------------------
	//	and now clear the hole display
	//	first some text ...
	//
	g_clDisplay.Print( F( "Clear Display" ) );

	delay( 2000 );

	//----------------------------------------------------------------------
	//	... and now clear the display
	//
	g_clDisplay.Clear();

	delay( 500 );

	//----------------------------------------------------------------------
	//	another example for positioning the cursor and writing some text
	//
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

	//----------------------------------------------------------------------
	//	In this example you can see how to print text with numbers in.
	//	First prepare the text (in this case with function sprintf).
	//	Then print the text.
	for( uint8_t idx = 0 ; idx < g_clDisplay.MaxTextLines() ; idx++ )
	{
		g_clDisplay.SetCursor( idx, 4 );

		sprintf( g_buffer, "Zeile %d", idx );
		g_clDisplay.Print( g_buffer );

		delay( 250 );
	}

	delay( 2000 );

	g_clDisplay.Clear();

	delay( 500 );

	//----------------------------------------------------------------------
	//	the last example for printing text.
	//
	g_clDisplay.SetCursor( 4, 0 );
	g_clDisplay.Print( F( "Column" ) );
	g_clDisplay.SetCursor( 3, 0 );
	
	for( uint8_t idx = 0 ; idx < g_clDisplay.MaxTextColumns() ; idx++ )
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
