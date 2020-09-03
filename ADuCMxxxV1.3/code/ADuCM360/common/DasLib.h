/** 
@mainpage ADuCM360 Low Level Functions
@version  V0.4
@author   ADI
@date     February 2013 

@par Revision History:
   - V0.1, February 2012: initial version. 
   - V0.2, October 2012: 
            - addition of PwrLib, FeeLib and DmaLib.
            - added dma functions to SPI Library.
            - fixed baud rate generation inside UrtCfg.
            - fixed an issue with WdtClrInt.
            - Modified DioLib to include pin functions.
            - RstLib CMSIS compiant.
            - All libraries now Doxygen commented.
   - V0.3, November 2012:
            - Fixed dio comments.
            - SPI dma functions moved to DmaLib.
            - Modified FlashProtect example.
            - Changes to FeeLib.
   - V0.4, February 2013:
            - Fixed ClkLib comment.
            - Fixed functions in AdcLib and IexcLib.
            - Fixed examples.
            - Added more doxygen comments to Examples.

@section intro Introduction
The ADuCM360/361 is a fully integrated microcontroller with dual/single 24-bit ADCs for low power applications.
A set of code examples demonstrating the use of the ADuCM360/361 peripherals are provided. For example, thermocouple/RTD 
interfacing examples (CN0221,CN0300) and an example using the on-chip DAC to control a 4-20mA loop (CN0300) are provided.
All examples are based on low level functions, also described in this document.

- The Modules tab list the Low Level Functions. These are grouped per peripherals or features.
- The files tab shows the Low Level Functions files.
- The Examples tab list the code examples based on the Low Level Functions.

@section disclaimer Disclaimer
All files for ADuCM360/361 provided by ADI, including this file, are
provided  as is without warranty of any kind, either expressed or implied.
The user assumes any and all risk from the use of this code.
It is the responsibility of the person integrating this code into an application
to ensure that the resulting application performs as required and is safe.

@section notes Release notes
Functions and examples still work in progress. Check for updates regularly.

   @defgroup daslibs Low Level Functions
   @{
      @defgroup adc ADC
      @defgroup clk Clock
      @defgroup dac DAC
      @defgroup dio Digital IO
      @defgroup dma DMA
      @defgroup fee Flash
      @defgroup gpt General Purpose Timer
      @defgroup i2c I2C
      @defgroup iexc Excitation Current Source
      @defgroup int Interrupts
      @defgroup pwm PWM
      @defgroup pwr Power
      @defgroup rst Reset
      @defgroup spi SPI
      @defgroup urt UART
      @defgroup wdt Watchdong Timer
      @defgroup wut Wake Up Timer
      
      Low Level Peripheral functions
   @}

**/