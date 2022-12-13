## Simple OLED sh1106 library

| Version | Date | Description |
| --- | --- | --- |
| 1.04 | 13.12.2022 | Move function PrintChar() to public section. |
| 1.03 | 30.01.2022 | Add functions to print text that is stored in PROGMEM. |
| 1.02 | 28.01.2022 | Change void Init( void ) to uint8_t Init( uint8_t address ). The function will now check, if the given address is valid and if a display is connected. If all is okay then '0' is returned otherwise an error code is retruned. |
| 1.01 | 21.01.2022 | Add function ClearLine( uint8_t usLineToClear ). |
| 1.00 | 18.01.2022 | First implementation of the class 'SimpleDisplayClass'. |
