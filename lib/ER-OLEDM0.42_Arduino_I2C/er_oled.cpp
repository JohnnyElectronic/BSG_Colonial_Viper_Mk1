/***************************************************
//Web: http://www.buydisplay.com
EastRising Technology Co.,LTD
****************************************************/

#include <Wire.h>
#include "er_oled.h"
#include "Arduino.h"

void I2C_Write_Byte(uint8_t value, uint8_t Cmd)
{
  uint8_t Addr = SSD1306_I2C_ADDRESS;
  Wire.beginTransmission(Addr);
  Wire.write(Cmd);
  Wire.write(value);
  Wire.endTransmission();
}

void er_oled_begin()
{
    if (0) {
        /* Only needed for SPI connections */
        pinMode(OLED_RST, OUTPUT);      
        digitalWrite(OLED_RST, HIGH);
        delay(10);
        digitalWrite(OLED_RST, LOW);
        delay(10);
        digitalWrite(OLED_RST, HIGH);
    }

    command(SSD1306_DISPLAYOFF);//--turn off oled panel
	
    command(SSD1306_SETDISPLAYCLOCKDIV);//--set display clock divide ratio/oscillator frequency
    command(0x80);//--set divide ratio

    command(SSD1306_SETMULTIPLEX);//--set multiplex ratio
    command(0x27);//--1/40 duty

    command(SSD1306_SETDISPLAYOFFSET);//-set display offset
    command(0x00);//-not offset

    command(SSD1306_SETIREF);//--Internal IREF Setting	
    command(SSD1306_INTIREF30);//--

    command(SSD1306_CHARGEPUMP);//--set Charge Pump enable/disable
    command(SSD1306_CHARGEPUMP7_5);//--Enable CP, set(0x10) disable


    command(SSD1306_SETSTARTLINE);//--set start line address

    command(SSD1306_NORMALDISPLAY);//--set normal display

    command(SSD1306_DISPLAYALLON_RESUME);//Disable Entire Display On

    command(SSD1306_SEGREMAPON);//--set segment re-map 128 to 0

    command(SSD1306_COMSCANDEC);//--Set COM Output Scan Direction 64 to 0

    command(SSD1306_SETCOMPINS);//--set com pins hardware configuration
    command(SSD1306_ALTCOM);//--Reset, Alt COM pin

    command(SSD1306_SETCONTRAST);//--set contrast control register
    command(0xaf);//-- contrast value

    command(SSD1306_SETPRECHARGE);//--set pre-charge period
    command(0x22);

    command(SSD1306_SETVCOMDETECT);//--set vcomh
    command(SSD1306_VCOMH77);

    command(SSD1306_DISPLAYON);//--turn on oled panel
    
}

void er_oled_clear(uint8_t* buffer)
{
	int i;
	for(i = 0;i < SSD1306_WIDTH * SSD1306_HEIGHT_ER / 8;i++)
	{
		buffer[i] = 0;
	}
}

void er_oled_pixel(int x, int y, char color, uint8_t* buffer)
{
    if(x > SSD1306_WIDTH || y > SSD1306_HEIGHT_ER)return ;
    if(color)
        buffer[x+(y/8)*SSD1306_WIDTH] |= 1<<(y%8);
    else
        buffer[x+(y/8)*SSD1306_WIDTH] &= ~(1<<(y%8));
}

void er_oled_char1616(uint8_t x, uint8_t y, uint8_t chChar, uint8_t* buffer)
{
	uint8_t i, j;
	uint8_t chTemp = 0, y0 = y, chMode = 0;

	for (i = 0; i < 32; i++) {
		chTemp = pgm_read_byte(&Font1612[chChar - 0x30][i]);
		for (j = 0; j < 8; j++) {
			chMode = chTemp & 0x80? 1 : 0; 
			er_oled_pixel(x, y, chMode, buffer);
			chTemp <<= 1;
			y++;
			if ((y - y0) == 16) {
				y = y0;
				x++;
				break;
			}
		}
	}
}

void er_oled_char(unsigned char x, unsigned char y, char acsii, char size, char mode, uint8_t* buffer)
{
    unsigned char i, j, y0=y;
    char temp;
    unsigned char ch = acsii - ' ';

    for(i = 0;i<size;i++) {
        if(size == 12)
        {
            if(mode)temp = pgm_read_byte(&Font1206[ch][i]);
            else temp = ~pgm_read_byte(&Font1206[ch][i]);
        }
        else /*  16 */
        {
            if(mode)temp = pgm_read_byte(&Font1608[ch][i]);
            else temp = ~pgm_read_byte(&Font1608[ch][i]);
        }

        for(j =0;j<8;j++)
        {
            if(temp & 0x80) er_oled_pixel(x, y, 1, buffer);
            else er_oled_pixel(x, y, 0, buffer);
            temp <<= 1;
            y++;
            if((y-y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

void er_oled_string(uint8_t x, uint8_t y, const char *pString, uint8_t Size, uint8_t Mode, uint8_t* buffer)
{
    while (*pString != '\0') {       
        if (x > (SSD1306_WIDTH - Size / 2)) {
            x = 0;
            y += Size;
            if (y > (SSD1306_HEIGHT_ER - Size)) {
                y = x = 0;
            }
        }
        
        er_oled_char(x, y, *pString, Size, Mode, buffer);
        x += Size / 2;
        pString++;
    }
}

void er_oled_char3216(uint8_t x, uint8_t y, uint8_t chChar, uint8_t* buffer)
{
    uint8_t i, j;
    uint8_t chTemp = 0, y0 = y, chMode = 0;

    for (i = 0; i < 64; i++) {
        chTemp = pgm_read_byte(&Font3216[chChar - 0x30][i]);
        for (j = 0; j < 8; j++) {
            chMode = chTemp & 0x80? 1 : 0; 
            er_oled_pixel(x, y, chMode, buffer);
            chTemp <<= 1;
            y++;
            if ((y - y0) == 32) {
                y = y0;
                x++;
                break;
            }
        }
    }
}

void er_oled_bitmap(uint8_t x,uint8_t y,const uint8_t *pBmp, uint8_t chWidth, uint8_t chHeight, uint8_t* buffer)
{
	uint8_t i, j, byteWidth = (chWidth + 7)/8;
	for(j = 0;j < chHeight;j++){
		for(i = 0;i <chWidth;i++){
			if(pgm_read_byte(pBmp + j * byteWidth + i / 8) & (128 >> (i & 7))){
				er_oled_pixel(x + i,y + j, 1, buffer);
			}
		}
	}		
}

void er_oled_display(uint8_t* pBuf)
{    uint8_t page,i;   
    for (page = 0; page < SSD1306_PAGES; page++) {         
        command(SSD1306_PAGESTARTADDR + page);/* set page address */     
        command(SSD1306_SETLOWCOLUMN + 12);   /* set low column address */      
        command(SSD1306_SETHIGHCOLUMN + 1);   /* set high column address */  
        for(i = 0; i< SSD1306_WIDTH; i++ ) {
          data(pBuf[i+page*SSD1306_WIDTH]);// write data one
        }        
    }
}

/* When state is True (1) display is inverted */
void er_oled_invert(uint8_t state)
{   
    if (state) {
        /* Invert display */
        command(SSD1306_INVERTDISPLAY); 
    } else {
        /* Normal display */
        command(SSD1306_NORMALDISPLAY);
    }
}

// startscrollright
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void er_oled_startscrollright(uint8_t start, uint8_t stop)
{
    command(SSD1306_RIGHT_HORIZONTAL_SCROLL);
    command(SSD1306_DUMMYBYTE);
    command(start);
    command(0X00);
    command(stop);
    command(0X00);
    command(0XFF);
    command(SSD1306_ACTIVATE_SCROLL);
}

// startscrollleft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void er_oled_startscrollleft(uint8_t start, uint8_t stop)
{
    command(SSD1306_LEFT_HORIZONTAL_SCROLL);
    command(SSD1306_DUMMYBYTE);
    command(start);
    command(0X00);
    command(stop);
    command(0X00);
    command(0XFF);
    command(SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagright
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void er_oled_startscrolldiagright(uint8_t start, uint8_t stop)
{
    command(SSD1306_SET_VERTICAL_SCROLL_AREA);
    command(SSD1306_DUMMYBYTE);
    command(SSD1306_HEIGHT_ER);
    command(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
    command(0X00);
    command(start);
    command(0X00);
    command(stop);
    command(0X01);
    command(SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagleft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void er_oled_startscrolldiagleft(uint8_t start, uint8_t stop)
{
    command(SSD1306_SET_VERTICAL_SCROLL_AREA);
    command(SSD1306_DUMMYBYTE);
    command(SSD1306_HEIGHT_ER);
    command(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
    command(0X00);
    command(start);
    command(0X00);
    command(stop);
    command(0X01);
    command(SSD1306_ACTIVATE_SCROLL);
}

void er_oled_stopscroll(void)
{   
    command(SSD1306_DEACTIVATE_SCROLL);
}

// vertical scroll
uint8_t er_oled_vertscroll(uint8_t scrollLine, uint8_t rows)
{
    for (int i = 0; i < rows; ++i) {
         delay(50);
         command(SSD1306_SETSTARTLINE | scrollLine % 64);
         ++scrollLine;
    }

    delay (100);
    return scrollLine;
}

// Draw Vert/Horiz/Diag lines
void er_oled_Line(uint8_t* pBuf, int x1, int y1, int x2, int y2, uint8_t ucColor, bool bRender)
{
  int temp;
  int dx = x2 - x1;
  int dy = y2 - y1;
  int error;
  uint8_t *p, *pStart, mask, bOld, bNew;
  int xinc, yinc;
  int y, x;
  
  if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 > SSD1306_WIDTH || x2 > SSD1306_WIDTH || y1 > SSD1306_HEIGHT_ER || y2 > SSD1306_HEIGHT_ER)
     return;

  if(abs(dx) > abs(dy)) {
    // X major case
    if(x2 < x1) {
      dx = -dx;
      temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    }

    y = y1;
    dy = (y2 - y1);
    yinc = 1;
    if (dy < 0)
    {
      dy = -dy;
      yinc = -1;
    }

    if (dy == 0)
    {
      yinc = 0;
    }

    for(x=x1; x1 <= x2; x1++) {
      er_oled_pixel(x1, y, ucColor, pBuf);
      x = x1+1; // we've already written the byte at x1

      if (dy > 0)
      {
          if (x%2 == 0) y += yinc;

          if (x1 == (abs(dx)/2)) y += 1;
       //   if (x1 == (abs(dx)/4)) y += 1;
         // if (x1 == (x2 - (abs(dx)/4))) y += 1;
      }
    } // for x1    
  }
  else {
    // Y major case
    if(y1 > y2) {
      dy = -dy;
      temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    } 

    dx = (x2 - x1);
    xinc = 1;
    if (dx < 0)
    {
      dx = -dx;
      xinc = -1;
    }
    for(x = x1; y1 <= y2; y1++) {
      er_oled_pixel(x1, y1, ucColor, pBuf);
    } // for y
  } // y major case

  if (bRender) {
      er_oled_display(pBuf);
  }
} /* oledDrawLine() */

/* Draw an outline or filled rectangle */
void er_oled_Rectangle(uint8_t* pBuf, int x1, int y1, int x2, int y2, uint8_t ucColor, uint8_t bDividers, bool bRender)
{
    int iOff;
    int x, y;

    /* Top/Bottom Line */
    for (x=x1; x <= x2; x++) {
       er_oled_pixel(x, y1, ucColor, pBuf);
       er_oled_pixel(x, y2, ucColor, pBuf);
    }

    /* Left & Right Side */
    for (y=y1; y <= y2; y++)
    {
        er_oled_pixel(x1, y, ucColor, pBuf);
        er_oled_pixel(x2, y, ucColor, pBuf);

        if (bDividers) 
        {
            int xDivOffset;

            /* 1st Quarter */
            xDivOffset = x1 + ((x2-x1) / 4);
            er_oled_pixel(xDivOffset, y, ucColor, pBuf);


            xDivOffset = x1 + ((x2-x1) / 2);
            er_oled_pixel(xDivOffset, y, ucColor, pBuf);

            xDivOffset = x2 - ((x2-x1) / 4);
            er_oled_pixel(xDivOffset, y, ucColor, pBuf);
        }
    }

    if (bRender) {
        er_oled_display(pBuf);
    }
} /* er_oled_oledRectangle() */

/* fill rectangle bar graph */
void er_oled_bar_fill(uint8_t* pBuf, int xs, int y1, int xe, int y2, uint8_t ucColor, bool bRender)
{
    int x, y;

    /* BAR Graph Fill */
    for (x=xs; x <= xe; x++) {
       for (y=y1; y <= y2; y++)
       {
          er_oled_pixel(x, y, ucColor, pBuf);
       } /* end for y */
    } /* end for x */

    if (bRender) {
        er_oled_display(pBuf);
    }
} /* end bar fill() */


/* Draw a circle outline */
/*                    buffer to use       x & y centers     radius        0/1             0/1     */
void er_oled_Circle(uint8_t* pBuf, int16_t xc, int16_t yc, int16_t r, uint8_t ucColor, bool bRender) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  er_oled_pixel(xc, yc + r, ucColor, pBuf);
  er_oled_pixel(xc, yc - r, ucColor, pBuf);
  er_oled_pixel(xc + r, yc, ucColor, pBuf);
  er_oled_pixel(xc - r, yc, ucColor, pBuf);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    er_oled_pixel(xc + x, yc + y, ucColor, pBuf);
    er_oled_pixel(xc - x, yc + y, ucColor, pBuf);
    er_oled_pixel(xc + x, yc - y, ucColor, pBuf);
    er_oled_pixel(xc - x, yc - y, ucColor, pBuf);
    er_oled_pixel(xc + y, yc + x, ucColor, pBuf);
    er_oled_pixel(xc - y, yc + x, ucColor, pBuf);
    er_oled_pixel(xc + y, yc - x, ucColor, pBuf);
    er_oled_pixel(xc - y, yc - x, ucColor, pBuf);
  }

  if (bRender) {
      er_oled_display(pBuf);
  }
}

/* Draw an ellipse outline */
/*                    buffer to use  x & y centers    x & y radius        0/1             0/1    */
void er_oled_Ellipse(uint8_t* pBuf, int xc, int yc, int rx, int ry, uint8_t ucColor, bool bRender)
{
    float dx, dy, d1, d2, x, y;
    x = 0;
    y = ry;
 
    // Initial decision parameter of region 1
    d1 = (ry * ry) - (rx * rx * ry) +
                     (0.25 * rx * rx);
    dx = 2 * ry * ry * x;
    dy = 2 * rx * rx * y;
 
    // For region 1
    while (dx < dy)
    {
        // Print points based on 4-way symmetry
        er_oled_pixel(x + xc, y + yc, ucColor, pBuf);
        er_oled_pixel(-x + xc, y + yc, ucColor, pBuf);
        er_oled_pixel(x + xc, -y + yc, ucColor, pBuf);
        er_oled_pixel(-x + xc,-y + yc, ucColor, pBuf);

        // Checking and updating value of
        // decision parameter based on algorithm
        if (d1 < 0)
        {
            x++;
            dx = dx + (2 * ry * ry);
            d1 = d1 + dx + (ry * ry);
        }
        else
        {
            x++;
            y--;
            dx = dx + (2 * ry * ry);
            dy = dy - (2 * rx * rx);
            d1 = d1 + dx - dy + (ry * ry);
        }
    }
 
    // Decision parameter of region 2
    d2 = ((ry * ry) * ((x + 0.5) * (x + 0.5))) +
         ((rx * rx) * ((y - 1) * (y - 1))) -
          (rx * rx * ry * ry);
 
    // Plotting points of region 2
    while (y >= 0)
    {
 
        // Print points based on 4-way symmetry
        er_oled_pixel(x + xc, y + yc, ucColor, pBuf);
        er_oled_pixel(-x + xc, y + yc, ucColor, pBuf);
        er_oled_pixel(x + xc, -y + yc, ucColor, pBuf);
        er_oled_pixel(-x + xc, -y + yc, ucColor, pBuf);

        // Checking and updating parameter
        // value based on algorithm
        if (d2 > 0)
        {
            y--;
            dy = dy - (2 * rx * rx);
            d2 = d2 + (rx * rx) - dy;
        }
        else
        {
            y--;
            x++;
            dx = dx + (2 * ry * ry);
            dy = dy - (2 * rx * rx);
            d2 = d2 + dx - dy + (rx * rx);
        }
    }

    if (bRender) {
        er_oled_display(pBuf);
    }
}

