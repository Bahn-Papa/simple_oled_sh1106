//##########################################################################
//#
//#		PrintMode.ino
//#
//#	This program will write Text on an OLED display and demonstrates
//#	the different PrintModes:
//#		- Overwrite Next Line (default mode)
//#		- Overwrite Same Line
//#		- Scroll Line
//#
//#-------------------------------------------------------------------------
//#
//#		Description of the PrintModes
//#
//#	Overwrite Next Line
//#		Whenever the output of a text comes to the end of a line and there
//#		is still some text to print then the cursor is set to the beginning
//#		of the next line. The output of the text will continue starting
//#		from this new position. If there is old text in this new line then
//#		it will be overwriten.
//#		If the output of a text comes to the end of the last line of the
//#		display then the output of the text will continue starting at the
//#		top left corner (home position) of the display. Again old text will
//#		be overwriten.
//#
//#	Overwrite Same Line
//#		Whenever the output of a text comes to the end of a line and there
//#		is still some text to print then the cursor is set to the beginning
//#		of the actual line. The output of the text will continue starting
//#		from this position. The old text in this line will be overwriten.
//#		As long as the Print function without '\n' in the text is used the
//#		output of text will go into the same line.
//#		To change the line use function SetCursor, function PrintLn or
//#		use the character '\n' within the text.
//#		If the function PrintLn or the character '\n' is used in the last
//#		line of the display then the output of the text will continue
//#		starting at the home position of the display. Again old text will
//#		be overwriten.
//#
//#	Scroll Line
//#		Basically it is the same behavior as 'Overwrite Next Line' mode.
//#		But if the output of the text reaches the end of the last line of
//#		the display then all lines on the display will be shifted up by
//#		one line. The last line of the display will be cleared and the
//#		cursor will be set to the beginning of that line.
//#		So if just the functions Print and PrintLn or the character '\n'
//#		are used all new text will be printed into the last
//#		line of the display.
//#		If the output of a text should go into another line then the
//#		function SetCursor must be used.
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

char g_strPrintModes[]			= "PrintModes Demo";
char g_strOverwriteNextLine[]	= " Overwrite Next Line ";
char g_strOverwriteSameLine[]	= " Overwrite Same Line ";
char g_strScrollLine[]			= " Scroll Line (default mode) ";
char g_buffer[]					= " ";
bool g_bInverseFont				= false;


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
	char	*pchLetter;

	//------------------------------------------------------------------
	//	start with PrintMode 'Scroll Line' (the default mode)
	//
	g_clDisplay.Print( g_strPrintModes );

	delay( 2000 );

	g_clDisplay.SetPrintModeScrollLine();
	g_clDisplay.SetCursor( 2, 0 );
	g_clDisplay.PrintLn( g_strScrollLine );

	for( uint8_t count = 0 ; count < 7 ; count++ )
	{
		pchLetter		= g_strScrollLine;
		g_bInverseFont	= !g_bInverseFont;

		g_clDisplay.SetInverseFont( g_bInverseFont );
		
		while( '\0' != *pchLetter )
		{
			g_buffer[ 0 ] = *pchLetter++;
			
			g_clDisplay.Print( g_buffer );
			
			delay( 100 );
		}
	}

	g_bInverseFont = false;
	g_clDisplay.SetInverseFont( false );
	g_clDisplay.Clear();


	//------------------------------------------------------------------
	//	next PrintMode is 'Overwrite Next Line'
	//
	g_clDisplay.Print( g_strPrintModes );

	delay( 2000 );

	g_clDisplay.SetPrintModeOverwriteNextLine();
	g_clDisplay.SetCursor( 2, 0 );
	g_clDisplay.PrintLn( g_strOverwriteNextLine );
	
	for( uint8_t count = 0 ; count < 7 ; count++ )
	{
		pchLetter		= g_strOverwriteNextLine;
		g_bInverseFont	= !g_bInverseFont;

		g_clDisplay.SetInverseFont( g_bInverseFont );
		
		while( '\0' != *pchLetter )
		{
			g_buffer[ 0 ] = *pchLetter++;
			
			g_clDisplay.Print( g_buffer );
			
			delay( 100 );
		}
	}
	
	g_bInverseFont = false;
	g_clDisplay.SetInverseFont( false );
	g_clDisplay.Clear();


	//------------------------------------------------------------------
	//	the last PrintMode is 'Overwrite Same Line'
	//
	g_clDisplay.Print( g_strPrintModes );

	delay( 2000 );

	g_clDisplay.SetCursor( 2, 0 );
	g_clDisplay.PrintLn( g_strOverwriteSameLine );
	g_clDisplay.SetPrintModeOverwriteSameLine();

	for( uint8_t count = 0 ; count < 5 ; count++ )
	{
		pchLetter		= g_strOverwriteSameLine;
		g_bInverseFont	= !g_bInverseFont;

		g_clDisplay.SetInverseFont( g_bInverseFont );
		
		while( '\0' != *pchLetter )
		{
			g_buffer[ 0 ] = *pchLetter++;
			
			g_clDisplay.Print( g_buffer );
			
			delay( 100 );
		}
	}

	g_bInverseFont = false;
	g_clDisplay.SetInverseFont( false );
	g_clDisplay.Clear();
}
