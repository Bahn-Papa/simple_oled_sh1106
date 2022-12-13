## Simple OLED sh1106 library

| Version | Date | Description |
| --- | --- | --- |
| 1.04 | 13. Dec 2022 | move function PrintChar() to public section |
| 1.03 | 30. Jan 2022 | add functions to print text that is stored in PROGMEM |
| 1.02 | 28. Jan 2022 | change void Init( void ) to uint8_t Init( uint8_t address )<br>
the function will now check, if the given address is valid and if a display is connected<br>
if all is okay then '0' is returned otherwise an error code is retruned |
| 1.01 | 21. Jan 2022 | add function ClearLine( uint8_t usLineToClear ) |
| 1.00 | 18. Jan 2022 | first implementation of the class 'SimpleDisplayClass' |
