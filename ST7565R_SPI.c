/* ************************************************************************
 *
 *   driver functions for ST7565R compatible grafic displays
 *   - using SPI interface (4 and 5 line)
 *
 *   (c) 2015 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment
 *    /RES        LCD_RESET
 *    A0          LCD_A0
 *    SCL (DB6)   LCD_SCL
 *    SI (DB7)    LCD_SI
 *    /CS1        LCD_CS (optional)
 *  - max. SPI clock rate: 20MHz
 *  - write only
 *  - horizontal flip might require an offset of 4 dots
 *    (132 RAM dots - 128 real dots = 4)
 */

/* http://edeca.net/wp/electronics/the-st7565-display-controller/ */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_ST7565R_SPI


/*
 *  local constants
 */

/* source management */
#define LCD_DRIVER_C


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "ST7565R.h"          /* ST7565R specifics */

/* fonts */
#include "font_fixed_8x8.h"



/*
 *  derived constants
 */

/* pages/bytes required for character's height */
#define CHAR_PAGES       ((FONT_SIZE_Y + 7) / 8)

/* number of lines and characters per line */
#define LCD_CHAR_X       (LCD_DOTS_X / FONT_SIZE_X)
#define LCD_CHAR_Y       ((LCD_DOTS_Y / 8) / CHAR_PAGES)

/* segment driver (ADC) direction */
#ifdef LCD_FLIP_X
  #define ADC_MODE       FLAG_ADC_REVERSE
#else
  #define ADC_MODE       FLAG_ADC_NORMAL
#endif

/* common driver direction */
#ifdef LCD_FLIP_Y
  #define COMMON_MODE    FLAG_COM_REVERSE
#else
  #define COMMON_MODE    FLAG_COM_NORMAL
#endif



/* ************************************************************************
 *   low level functions for SPI interface
 * ************************************************************************ */


/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  /* set port pins to output mode */
  #ifdef LCD_CS
    /* including /CS1 */
    LCD_DDR = LCD_DDR | (1 << LCD_RESET) | (1 << LCD_A0) | (1 << LCD_SCL) | (1 << LCD_SI) | (1 << LCD_CS);
  #else
    /* excluding /CS1 */
    LCD_DDR = LCD_DDR | (1 << LCD_RESET) | (1 << LCD_A0) | (1 << LCD_SCL) | (1 << LCD_SI);
  #endif

  /*  set default levels:
   *  - /CS1 high, if pin available
   *  - SCL high
   */

  #ifdef LCD_CS
    /* including /CS1 */
    LCD_PORT = LCD_PORT | (1 << LCD_CS) | (1 << LCD_SCL);
  #else
    /* excluding /CS1 */
    LCD_PORT = LCD_PORT | (1 << LCD_SCL);
  #endif

  /* disable reset */
  LCD_PORT = LCD_PORT | (1 << LCD_RESET);    /* set /RES high */
}



/*
 *  send a byte (data or command) to the LCD
 *  - bit-bang 8 bits, MSB first, LSB last
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Send(unsigned char Byte)
{
  uint8_t           n = 0;         /* counter */

  /* select chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT = LCD_PORT & ~(1 << LCD_CS);    /* set /CS1 low */
  #endif

  /* bit-bang 8 bits */
  while (n < 8)               /* 8 bits */
  {
    /* get current MSB and set SI */
    if (Byte & 0b10000000)    /* 1 */
    {
      /* set SI high */
      LCD_PORT = LCD_PORT | (1 << LCD_SI);
    }
    else                      /* 0 */
    {
      /* set SI low */
      LCD_PORT = LCD_PORT & ~(1 << LCD_SI);
    }

    /* start clock cycle (falling edge) */
    LCD_PORT = LCD_PORT & ~(1 << LCD_SCL);

    /* end clock cycle (rising edge takes bit) */
    LCD_PORT = LCD_PORT |(1 << LCD_SCL); 

    Byte <<= 1;               /* shift bits one step left */

    n++;                      /* next bit */
  }

  /* deselect chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT = LCD_PORT | (1 << LCD_CS);     /* set /CS1 high */
  #endif
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */
 
void LCD_Cmd(unsigned char Cmd)
{
  /* indicate command mode */
  LCD_PORT = LCD_PORT & ~(1 << LCD_A0);      /* set A0 low */

  LCD_Send(Cmd);              /* send command */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Data(unsigned char Data)
{
  /* indicate data mode */
  LCD_PORT = LCD_PORT | (1 << LCD_A0);       /* set A0 high */

  LCD_Send(Data);             /* send data */
}



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  set LCD character position
 *  - since we can't read the LCD and don't use a RAM buffer
 *    we have to move page-wise in y direction
 *  - top left: 0/0
 *
 *  requires:
 *  - x:  horizontal position (1-)
 *  - y:  vertical position (1-)
 */

void LCD_Pos(uint8_t x, uint8_t y)
{
  uint8_t           Temp;

  /* update UI */
  UI.CharPos_X = x;
  UI.CharPos_Y = y;

  /* vertical position */
  y--;                               /* pages start at 0 */
  y *= CHAR_PAGES;                   /* offset for character */
  LCD_Cmd(CMD_PAGE | y);             /* set page */

  /* horizontal position */
  x--;                               /* columns starts at 0 */
  x *= FONT_SIZE_X;                  /* offset for character */
  #ifdef LCD_OFFSET_X
  x += 0;                            /* offset of 0 dots */
  #endif
  Temp = x;
  Temp &= 0b00001111;                /* filter lower nibble */
  LCD_Cmd(CMD_COLUMN_L | Temp);      /* set lower nibble */
  Temp = x;
  Temp >>= 4;                        /* shift upper nibble to lower */
  Temp &= 0b00001111;                /* filter nibble */
  LCD_Cmd(CMD_COLUMN_H | Temp);      /* set upper nibble */
}



/*
 *  clear one single character line
 *
 *  requires:
 *  - Line: line number (1-)
 */ 

void LCD_ClearLine(uint8_t Line)
{
  uint8_t           MaxPage;       /* page limit */
  uint8_t           n;

  Line--;                               /* pages start at 0 */
  Line *= CHAR_PAGES;                   /* offset for char */
  MaxPage = Line + CHAR_PAGES;          /* end page + 1 */

  /* clear line */
  while (Line < MaxPage)
  {
    /* move to start of current page */
    LCD_Cmd(CMD_PAGE | Line);           /* set page */
    LCD_Cmd(CMD_COLUMN_L);              /* reset column */
    LCD_Cmd(CMD_COLUMN_H);

    /* clear page */
    n = 0;
    while (n <= 132)          /* internal RAM size */
    {
      LCD_Data(0);            /* send empty byte */
      n++;                    /* next byte */
    }

    Line++;                   /* next page */
  }
}



/*
 *  clear the display 
 */ 

void LCD_Clear(void)
{
  uint8_t           n = 1;         /* counter */

  /* since there's no clear command we have to clear all dots manually */
  while (n <= LCD_CHAR_Y)          /* for all lines */
  {
    LCD_ClearLine(n);              /* clear line */
    n++;                           /* next line */
  }

  LCD_Pos(1, 1);         /* reset character position */
}



/*
 *  set contrast
 *  required:
 *  - value: 0-63
 */

void LCD_Contrast(uint8_t Contrast)
{

  if (Contrast <= 63)              /* limit value */
  {
    /* set contrast */
    LCD_Cmd(CMD_V0_MODE);
    LCD_Cmd(CMD_V0_REG | Contrast);

    NV.Contrast = Contrast;        /* update value */
  }
}



/*
 *  initialize LCD
 *  - for a single 3.3V supply
 */
 
void LCD_Init(void)
{
  /* reset display */
  LCD_PORT = LCD_PORT & ~(1 << LCD_RESET);   /* set /RES low */
  wait1us();                                 /* wait 1µs */
  LCD_PORT = LCD_PORT | (1 << LCD_RESET);    /* set /RES high */
  wait1us();                                 /* wait 1µs */

  /* set start line: user defined value (default 0) */
  LCD_Cmd(CMD_START_LINE | LCD_START_Y);

  /* set segment driver direction (ADC) */
  LCD_Cmd(CMD_SEGMENT_DIR | ADC_MODE);

  /* set common driver direction */
  LCD_Cmd(CMD_COMMON_DIR | COMMON_MODE);

  /* set LCD bias to 1/9 (duty 1/65) */
  LCD_Cmd(CMD_LCD_BIAS | FLAG_BIAS_19);

  /* set power mode: all on */
  LCD_Cmd(CMD_POWER_MODE | FLAG_FOLOWER_ON | FLAG_REGULATOR_ON | FLAG_BOOSTER_ON);

  /* set booster ratio to 4x */
  LCD_Cmd(CMD_BOOSTER_MODE);
  LCD_Cmd(CMD_BOOSTER_REG | FLAG_BOOSTER_234);

  /* set contrast: resistor ratio 6.5 */
  LCD_Cmd(CMD_V0_RATIO | FLAG_RATIO_65);

  /* set contrast: user defined value (default 22) */
  LCD_Contrast(LCD_CONTRAST);

  /* no indicator */
  LCD_Cmd(CMD_INDICATOR_MODE);
  LCD_Cmd(CMD_INDICATOR_REG | FLAG_INDICATOR_OFF);

  /* switch display on */
  LCD_Cmd(CMD_DISPLAY | FLAG_DISPLAY_ON);

  /* update maximums */
  UI.CharMax_X = LCD_CHAR_X;       /* characters per line */
  UI.CharMax_Y = LCD_CHAR_Y;       /* lines */
  UI.MaxContrast = 63;             /* LCD contrast */

  LCD_Clear();                /* clear display */
}



/*
 *  display a single character
 *
 *  requires:
 *  - Char: character to display
 */

void LCD_Char(unsigned char Char)
{
  uint8_t           *Table;        /* pointer to table */
  uint8_t           Index;         /* font index */
  uint16_t          Offset;        /* address offset */
  uint8_t           x = 1;         /* bitmap x byte counter */
  uint8_t           y = 1;         /* bitmap y byte counter */

  /* get font index number from lookup table */
  Table = (uint8_t *)&FontTable;        /* start address */
  Table += Char;                        /* add offset for character */
  Index = pgm_read_byte(Table);         /* get index number */
  if (Index == 0xff) return;            /* no character bitmap available */

  /* calculate start address of character bitmap */
  Table = (uint8_t *)&FontData;        /* start address of font data */
  Offset = FONT_BYTES_N * Index;       /* offset for character */
  Table += Offset;                     /* address of character data */

  /* calculate vertical start position */
  Index = CHAR_PAGES;                  /* pages/bytes per character */
  Index *= (UI.CharPos_Y - 1);         /* offset for character */

  /* read data and send it to display */
  while (y <= FONT_BYTES_Y)
  {
    if (y > 1)                /* multi-page bitmap */
    {
      /* set byte position */
      LCD_Pos(UI.CharPos_X, 0);         /* set x pos */
      LCD_Cmd(CMD_PAGE | Index);        /* set y pos (page) */
    }

    /* read and send all bytes for this row */
    while (x <= FONT_BYTES_X)
    {
      Index = pgm_read_byte(Table);     /* read byte */
      LCD_Data(Index);                  /* send byte */
      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    Index++;                            /* next page */
    y++;                                /* next row */
  }

  UI.CharPos_X++;             /* update character position */
}



/*
 *  set cursor
 *
 *  required:
 *  - Mode: cursor mode
 *    0: cursor on
 *    1: cursor off
 */

void LCD_Cursor(uint8_t Mode)
{
  LCD_Pos(LCD_CHAR_X, LCD_CHAR_Y);      /* move to bottom right */

  if (Mode)              /* cursor on */
  {
    LCD_Char('>');
  }
  else                   /* cursor off */
  {
    LCD_Char(' ');
  }
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef LCD_DRIVER_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */
