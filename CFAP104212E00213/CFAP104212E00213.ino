//=============================================================================
// "Arduino" example program for Crystalfontz ePaper. 
//
// This project is for the CFAP104212E0-0213:
//
//   https://www.crystalfontz.com/product/CFAP104212E00213
//
// It was written against a Seeduino v4.2 @3.3v. An Arduino UNO modified to
// operate at 3.3v should also work.
//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>
//=============================================================================
// Connecting the Arduino to the display
//
// ARDUINO |Adapter |Wire Color |Function
// --------+--------+-----------+--------------------
// D2      |19      |Yellow     |BS1 Not Used
// D3      |17      |Green      |Busy Line
// D4      |18      |Brown      |Reset Line
// D5      |15      |Purple     |Data/Command Line
// D10     |16      |Blue       |Chip Select Line
// D11     |14      |White      |MOSI
// D13     |13      |Orange     |Clock
// 3.3V    |5       |Red        |Power
// GND     |3       |Black      |Ground
//
// Short the following pins on the adapter board:
// GND  -> BS2
// RESE -> .47ohms
//=============================================================================
//Connecting the Arduino to the SD card
//
// ARDUINO  |Wire Color |Function
// ---------+-----------+--------------------
// D2       |Blue       |CS
// D3       |Green      |MOSI
// D4       |Brown      |CLK
// D5       |Purple     |MISO
//
//=============================================================================
// Creating image data arrays
//
// Bmp_to_epaper is code that will aid in creating bitmaps necessary from .bmp files.
// The code can be downloaded from the Crystalfontz website: https://www.Crystalfontz.com
// or it can be downloaded from github: https://github.com/crystalfontz/bmp_to_epaper
//=============================================================================

// The display is SPI, include the library header.
#include <SPI.h>
#include <SD.h>
#include <avr/io.h>

// Include LUTs
#include "LUTs_for_CFAP104212E00213.h"
#include "Images_for_CFAP104212E00213.h"

#define EPD_READY   3
#define EPD_RESET   4
#define EPD_DC      5
#define EPD_CS      10
#define SD_CS       8

#define ePaper_RST_0  (digitalWrite(EPD_RESET, LOW))
#define ePaper_RST_1  (digitalWrite(EPD_RESET, HIGH))
#define ePaper_CS_0   (digitalWrite(EPD_CS, LOW))
#define ePaper_CS_1   (digitalWrite(EPD_CS, HIGH))
#define ePaper_DC_0   (digitalWrite(EPD_DC, LOW))
#define ePaper_DC_1   (digitalWrite(EPD_DC, HIGH))


uint16_t HRES = 104;
uint16_t VRES = 212;

//=============================================================================
//this function will take in a byte and send it to the display with the 
//command bit low for command transmission
void writeCMD(uint8_t command)
{
  ePaper_DC_0;
  ePaper_CS_0;
  SPI.transfer(command);
  ePaper_CS_1;
}

//this function will take in a byte and send it to the display with the 
//command bit high for data transmission
void writeData(uint8_t data)
{
  ePaper_DC_1;
  ePaper_CS_0;
  SPI.transfer(data);
  ePaper_CS_1;
}

//===========================================================================
void setup(void)
{
  //Debug port / Arduino Serial Monitor (optional)
  Serial.begin(9600);
  Serial.println("setup started");
  // Configure the pin directions   
  pinMode(EPD_CS, OUTPUT);
  pinMode(EPD_RESET, OUTPUT);
  pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_READY, INPUT);
  pinMode(SD_CS, OUTPUT);

  digitalWrite(SD_CS, LOW);

  if (!SD.begin(SD_CS))
  {
    Serial.println("SD could not initialize");
  }
  //Set up SPI interface
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.begin();


  //reset driver
  ePaper_RST_0;
  delay(200);
  ePaper_RST_1;
  delay(200);

  initEPD();
  Serial.println("setup complete");
}

void initEPD()
{

  //-----------------------------------------------------------------------------
  //more detail on the following commands and additional commands not used here	
  //can be found on the CFAP104212E0-0213 datasheet on the Crystalfontz website	
  //-----------------------------------------------------------------------------

  //Power Setting
  writeCMD(0x01);
  writeData(0x03);
  writeData(0x00);
  writeData(0x2B);
  writeData(0x2B);
  writeData(0x03);

  //Booster Soft Start
  writeCMD(0x06);
  writeData(0x17);
  writeData(0x17);
  writeData(0x17);

  //Power On
  writeCMD(0x04);
  Serial.println("before wait");
  //wait until powered on
  while (0 == digitalRead(EPD_READY));
  Serial.println("after wait");

  //Panel Setting 
  writeCMD(0x00);
  writeData(0x83);

  //PLL Control
  writeCMD(0x30);
  writeData(0x3a);

  //Resolution
  writeCMD(0x61);
  writeData(HRES);	
  writeData(VRES>>8);	
  writeData(VRES&0xff);

  //VCOM_DC Setting
  writeCMD(0x82);
  writeData(0x28);

  //Vcom and data interval setting
  writeCMD(0x50);
  writeData(0x87);

}

void setRegisterLUT()
{
  //set LUTs
  //The following block allows the LUTs to be changed.
  //In order for these LUTs to take effect, command 0x00 must have bit 5 set to "1"
  //set panel setting to call LUTs from the register
  writeCMD(0x00);
  writeData(0xb3);

  //VCOM_LUT_LUTC
  writeCMD(0x20);
  for (int i = 0; i < 44; i++)
  {
    writeData(pgm_read_byte(&VCOM_LUT_LUTC[i]));
  }
  //W2W_LUT_LUTWW
  writeCMD(0x21);
  for (int i = 0; i < 42; i++)
  {
    writeData(pgm_read_byte(&W2W_LUT_LUTWW[i]));
  }
  //B2W_LUT_LUTBW_LUTR
  writeCMD(0x22);
  for (int i = 0; i < 42; i++)
  {
    writeData(pgm_read_byte(&B2W_LUT_LUTBW_LUTR[i]));
  }
  //W2B_LUT_LUTWB_LUTW
  writeCMD(0x23);
  for (int i = 0; i < 42; i++)
  {
    writeData(pgm_read_byte(&W2B_LUT_LUTWB_LUTW[i]));
  }
  //B2B_LUT_LUTBB_LUTB
  writeCMD(0x24);
  for (int i = 0; i < 42; i++)
  {
    writeData(pgm_read_byte(&B2B_LUT_LUTBB_LUTB[i]));
  }
}

void setPartialRegisterLUT()
{
  //set LUTs
  //The following block allows the LUTs to be changed.
  //In order for these LUTs to take effect, command 0x00 must have bit 5 set to "1"
  //set panel setting to call LUTs from the register
  writeCMD(0x00);
  writeData(0xb3);

  //VCOM_LUT_LUTC
  writeCMD(0x20);
  for (int i = 0; i < 44; i++)
  {
    writeData(pgm_read_byte(&VCOM_LUT_LUTC_PARTIAL[i]));
  }
  //W2W_LUT_LUTWW
  writeCMD(0x21);
  for (int i = 0; i < 42; i++)
  {
    writeData(pgm_read_byte(&W2W_LUT_LUTWW_PARTIAL[i]));
  }
  //B2W_LUT_LUTBW_LUTR
  writeCMD(0x22);
  for (int i = 0; i < 42; i++)
  {
    writeData(pgm_read_byte(&B2W_LUT_LUTBW_LUTR_PARTIAL[i]));
  }
  //W2B_LUT_LUTWB_LUTW
  writeCMD(0x23);
  for (int i = 0; i < 42; i++)
  {
    writeData(pgm_read_byte(&W2B_LUT_LUTWB_LUTW_PARTIAL[i]));
  }
  //B2B_LUT_LUTBB_LUTB
  writeCMD(0x24);
  for (int i = 0; i < 42; i++)
  {
    writeData(pgm_read_byte(&B2B_LUT_LUTBB_LUTB_PARTIAL[i]));
  }
}

void setOTPLUT()
{
  //set panel setting to call LUTs from OTP
  //These are the original settings in the init block
  writeCMD(0x00);
  writeData(0x93); 
}

void partialUpdateSolid(uint8_t x1, uint16_t y1, uint8_t x2, uint16_t y2, uint8_t color1, uint8_t color2)
{
  //turn on partial update mode
  writeCMD(0x91);

  writeCMD(0x90);
  writeData(((x1 / 8) << 1) | 0x07);	//x1
  writeData(((x2 / 8) << 1));	//x2
  writeData(y1 >> 8);	  //1st half y1
  writeData(y1 & 0xff);	//2nd half y1
  writeData(y2 >> 8);	  //1st half y2
  writeData(y2 & 0xff);	//2nd half y2
  writeData(0x01);

  int i;
  int h;
  //send black and white information
  writeCMD(0x10);
  for (h = 0; h < y2 - y1; h++)
  {
    for (i = 0; i < (x2 - x1) / 8; i++)
    {
      writeData(color1);
    }
  }
  //send yellow information
  writeCMD(0x13); 
  for (h = 0; h < y2 - y1; h++)
  {
    for (i = 0; i < (x2 - x1) / 8; i++)
    {
      writeData(color2);
    }
  }

  //partial refresh of the same area as the partial update
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));

  //turn off partial update mode
  writeCMD(0x92);
}

//================================================================================
void Load_Flash_Image_To_Display_RAM(uint16_t width_pixels,
  uint16_t height_pixels,
  const uint8_t *BW_image,
  const uint8_t *Y_image)
{
  //Index into *image, that works with pgm_read_byte()
  uint16_t
    index;
  index = 0;

  //Get width_bytes from width_pixel, rounding up
  uint8_t
    width_bytes;
  width_bytes = (width_pixels + 7) >> 3;

  //Make sure the display is not busy before starting a new command.
  while (0 == digitalRead(EPD_READY));
  //Select the controller   
  ePaper_CS_0;

  //Aim at the command register
  ePaper_DC_0;
  //Write the command: DATA START TRANSMISSION 1 (DTM2) (R13H)
  //  Display Start transmission 1
  //  (DTM1, BW Data)
  //
  // This command starts transmitting data and write them into SRAM. To complete
  // data transmission, command DSP (Data transmission Stop) must be issued. Then
  // the chip will start to send data/VCOM for panel.
  //  * In B/W mode, this command writes “OLD” data to SRAM.
  //  * In B/W/Yellow mode, this command writes “BW” data to SRAM.
  SPI.transfer(0x10);
  //Pump out the BW data.
  ePaper_DC_1;
  index = 0;
  for (uint16_t y = 0; y<height_pixels; y++)
  {
    for (uint8_t x = 0; x<width_bytes; x++)
    {
      SPI.transfer(pgm_read_byte(&BW_image[index]));
      index++;
    }
  }

  //Aim at the command register
  ePaper_DC_0;
  //Write the command: DATA START TRANSMISSION 2 (DTM2) (R13H)
  //  Display Start transmission 2
  //  (DTM2, Yellow Data)
  //
  // This command starts transmitting data and write them into SRAM. To complete
  // data transmission, command DSP (Data transmission Stop) must be issued. Then
  // the chip will start to send data/VCOM for panel.
  //  * In B/W mode, this command writes “NEW” data to SRAM.
  //  * In B/W/Yellow mode, this command writes “Yellow” data to SRAM.
  SPI.transfer(0x13);
  //Pump out the Yellow data.
  ePaper_DC_1;
  index = 0;
  for (uint16_t y = 0; y<height_pixels; y++)
  {
    for (uint8_t x = 0; x<width_bytes; x++)
    {
      SPI.transfer(pgm_read_byte(&Y_image[index]));
      index++;
    }
  }

  //Aim back at the command register
  ePaper_DC_0;
  //Write the command: DATA STOP (DSP) (R11H)
  SPI.transfer(0x11);
  //Write the command: Display Refresh (DRF)   
  SPI.transfer(0x12);
  //Deslect the controller   
  ePaper_CS_1;
}

//================================================================================
void show_BMPs_in_root(void)
{
  File
    root_dir;
  root_dir = SD.open("/");
  if (0 == root_dir)
  {
    Serial.println("show_BMPs_in_root: Can't open \"root\"");
    return;
  }


  File
    bmp_file;

  while (1)
  {
    bmp_file = root_dir.openNextFile();
    Serial.println(bmp_file.name());
    if (0 == bmp_file)
    {
      // no more files, break out of while()
      // root_dir will be closed below.
      break;
    }
    //Skip directories (what about volume name?)
    if (0 == bmp_file.isDirectory())
    {
      //The file name must include ".BMP"
      if (0 != strstr(bmp_file.name(), ".BMP"))
      {
        //The BMP must be exactly 36918 long
        //(this is correct for182x96, 24-bit)
        uint32_t size = bmp_file.size();

        Serial.println(size);
        if (HRES*VRES * 3 <= size)
        {
          Serial.println("made it");


          //Make sure the display is not busy before starting a new command.
          while (0 == digitalRead(EPD_READY));
          //Select the controller   
          ePaper_CS_0;

          writeCMD(0x10);
          //Jump over BMP header
          bmp_file.seek(54);
          //grab one row of pixels from the SD card at a time
          static uint8_t one_line[104 * 3];
          for (int line = 0; line < 212; line++)
          {

            //Set the LCD to the left of this line. BMPs store data
            //to have the image drawn from the other end, uncomment the line below

            //read a line from the SD card
            bmp_file.read(one_line, 104 * 3);

            //send the line to the display
            send_pixels_BW(104 * 3, one_line);
          }
          Serial.println("halfway");

          writeCMD(0x13);
          //Jump over BMP header
          bmp_file.seek(54);
          //grab one row of pixels from the SD card at a time
          for (int line = 0; line < 212; line++)
          {

            //Set the LCD to the left of this line. BMPs store data
            //to have the image drawn from the other end, uncomment the line below

            //read a line from the SD card
            bmp_file.read(one_line, 104 * 3);

            //send the line to the display
            send_pixels_Y(104 * 3, one_line);
          }

          Serial.println("refreshing......");
          //Give a bit to let them see it
          //Make sure the display is not busy before starting a new command.
          //Aim back at the command register
          ePaper_DC_0;
          //Write the command: DATA STOP (DSP) (R11H)
          SPI.transfer(0x11);
          //Write the command: Display Refresh (DRF)   
          SPI.transfer(0x12);
          //Deslect the controller   
          ePaper_CS_1;
          while (0 == digitalRead(EPD_READY));
          delay(30000);
        }
      }
    }
    //Release the BMP file handle
    bmp_file.close();
  }
  //Release the root directory file handle
  root_dir.close();
}

//================================================================================
void send_pixels_BW(uint16_t byteCount, uint8_t *dataPtr)
{
  uint8_t data;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  while (byteCount != 0)
  {
    uint8_t data = 0;
    red = *dataPtr;
    dataPtr++;
    byteCount--;
    green = *dataPtr;
    dataPtr++;
    byteCount--;
    blue = *dataPtr;
    dataPtr++;
    byteCount--;

    //check to see if pixel should be set as black or white
    if (127 < ((red*.21) + (green*.72) + (blue*.07)))
    {
      data = data | 0x01;
    }

    for (uint8_t i = 0; i < 7; i++)
    {
      red = *dataPtr;
      dataPtr++;
      byteCount--;
      green = *dataPtr;
      dataPtr++;
      byteCount--;
      blue = *dataPtr;
      dataPtr++;
      byteCount--;
      data = data << 1;

      //check to see if pixel should be set as black or white
      if (127 < ((red*.21) + (green*.72) + (blue*.07)))
      {
        data = data | 0x01;
      }
      else
      {
        data = data & 0xFE;
      }
    }
    writeData(data);
  }
}

//================================================================================
void send_pixels_Y(uint8_t byteCount, uint8_t *dataPtr)
{
  uint8_t data;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  while (byteCount != 0)
  {
    uint8_t data = 0;
    blue = *dataPtr;
    dataPtr++;
    byteCount--;
    green = *dataPtr;
    dataPtr++;
    byteCount--;
    red = *dataPtr;
    dataPtr++;
    byteCount--;

    //check to see if pixel should be set as yellow
    if ((228 < red) && (208 < green) && (blue < 250))
    {
      //data |= 0x01;
    }

    for (uint8_t i = 0; i < 7; i++)
    {
      blue = *dataPtr;
      dataPtr++;
      byteCount--;
      green = *dataPtr;
      dataPtr++;
      byteCount--;
      red = *dataPtr;
      dataPtr++;
      byteCount--;
      data = data << 1;

      //check to see if pixel should be set as yellow
      if ((228 < red) && (208 < green) && (blue < 250))
      {
        //data |= 0x01;
      }
      else
      {
        //data &= 0xFE;
      }
    }
    writeData(data);
  }
}

//=============================================================================
#define SHUTDOWN_BETWEEN_UPDATES (0)
#define splashscreen 1
#define white 1
#define black 1
#define yellow 1
#define checkerboard 1
#define partialUpdate 1
#define showBMPs 0
void loop()
{

#if splashscreen
  Serial.println("top of loop");
  //load an image to the display
  Load_Flash_Image_To_Display_RAM(104, 212, Splash_Mono_1BPP, Splash_Yellow_1BPP);


  Serial.print("refreshing . . . ");
  while (0 == digitalRead(EPD_READY));
  Serial.println("refresh complete");
  //wait for 20sec before refreshing again
  delay(20000);
#endif

#if white
  //start data transmission 1
  writeCMD(0x10);
  for (int i = 0; i < VRES*HRES/8; i++)
  {
    writeData(0x00);
  }
  //start data transmission 2
  writeCMD(0x13);
  for (int i = 0; i < VRES*HRES / 8; i++)
  {
    writeData(0x00);
  }
  //refresh the display
  Serial.println("before refresh wait");
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  delay(2000);
#endif


#if black


  //start data transmission 1
  writeCMD(0x10);
  for (int i = 0; i < VRES*HRES / 8; i++)
  {
    writeData(0xff);
  }
  //start data transmission 2
  writeCMD(0x13);
  for (int i = 0; i < VRES*HRES / 8; i++)
  {
    writeData(0x00);
  }
  Serial.println("before refresh wait");
  //refresh the display
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  delay(2000);
#endif

#if yellow
  //start data transmission 1
  writeCMD(0x10);
  for (int i = 0; i < VRES*HRES / 8; i++)
  {
    writeData(0x00);
  }
  //start data transmission 2
  writeCMD(0x13);
  for (int i = 0; i < VRES*HRES / 8; i++)
  {
    writeData(0xff);
  }
  Serial.println("before refresh wait");
  //refresh the display
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  delay(20000);
#endif

#if checkerboard
  //color1 is for the bw data, color2 is for the yellow data
  uint8_t color1 = 0x00;
  uint8_t color2 = 0xff;

  //start data transmission 1
  writeCMD(0x10);
  for (int i = 0; i < 212; i++)
  {
    color1 = ~color1;
    //every 8 rows, change the color
    if (i % 8 == 0)
    {
      color1 = ~color1;
    }
    //run for 13 loops to complete a row --- 104 / 8 = 13
    for (uint8_t j = 0; j < 13; j++)
    {
      writeData(color1);
      //every 8 pixels, change the color
      color1 = ~color1;
    }
  }
  writeCMD(0x13);
  for (int i = 0; i < 212; i++)
  {
    color2 = ~color2;
    if (i % 8 == 0)
    {
      color2 = ~color2;
    }
    for (uint8_t j = 0; j < 13; j++)
    {
      writeData(color2);
      color2 = ~color2;
    }
}

  Serial.println("before refresh wait");
  //refresh the display
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  delay(20000);
#endif

#if partialUpdate
  partialUpdateSolid(50, 24, 100, 100, 0xff, 0x00);
  while (0 == digitalRead(EPD_READY));
  delay(1000);

  partialUpdateSolid(50, 24, 100, 100, 0x00, 0x00);
  while (0 == digitalRead(EPD_READY));
  delay(1000);

  partialUpdateSolid(50, 24, 100, 100, 0xff, 0xff);
  while (0 == digitalRead(EPD_READY));
  delay(1000);

  partialUpdateSolid(50, 24, 100, 100, 0x00, 0xff);
  while (0 == digitalRead(EPD_READY));
  delay(1000);
  #endif

#if showBMPs


  //Panel Setting 
  //writeCMD(0x00);
  //writeData(0x8b);

  show_BMPs_in_root();
  delay(30000);
  //Panel Setting 
  //writeCMD(0x00);
  //writeData(0x83);

#endif

}
//=============================================================================
