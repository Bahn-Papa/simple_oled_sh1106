Simple OLED sh1106 library

This library can print simple text on an OLED display that is driven by a sh1106 chip.

Why another OLED library ?

I was searching for a simple OLED library to print out some infos (e.g.: debug infos).
But all I found were libraries with much more capabilities that I want to have.
Therefore these libraries need many resources (e.g.: flash and RAM memory).
So I decide to write my own simple library.

Features:

The library provides a global variable 'g_clDisplay' of class 'SimpleOledDisplay'
to work with the OLED display.
The class has the following function set:

Print			Prints the text in the current line on the display
PrintLn			Prints the text in the current line on the display and
				sets the cursor to the beginning of the next line

Clear			clears the hole display and sets the cursor to the top left position of the display
ClearLine		clears just the current line and sets the cursor to the beginning of the line

Home			sets the cursor to the top left position of the display
SetCursor		sets the cursor to the given line and column of the display

SetInverseFont	sets the print mode to white background with black characters
				or black background with white characters

SetInverse		inverts the entire display
				each white pixel will get black and each black pixel will get white
Flip			turns the output to the display by 180 degree
