// ILI9163C library example
// Analog clock with true color clock face
// (c) 2019 Pawel A. Hernik
// YT video: https://youtu.be/Xr-dxPhePhY

/*
Pinout (header on the top, from left):
  LED   -> 3.3V
  SCK   -> D13
  SDA   -> D11/MOSI
  A0/DC -> D8  or any digital
  RESET -> D9  or any digital
  CS    -> D10 or any digital
  GND   -> GND
  VCC   -> 3.3V
*/

#define SCR_WD 128
#define SCR_HT 128

#define TFT_CS 10
#define TFT_DC  8
#define TFT_RST 9
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ILI9163C_Fast.h>
Arduino_ILI9163C lcd = Arduino_ILI9163C(TFT_DC, TFT_RST, TFT_CS);

#include "sans.h"
//#include "gold.h"
//#include "red.h"
//#include "roman.h"
//#include "roman2.h"
//#include "roman3.h"
//#include "old.h"
//#include "white.h"
//#include "metal.h"
//#include "black.h"

//#include "small4x6_font.h"

char buf[80];

uint8_t txt2num(const char* p) 
{
  return 10*(*p-'0') + *(p+1)-'0';
}


// configuration

int cx = SCR_WD/2, cy = SCR_HT/2;  // clock center
int style = 3;
uint16_t hHandCol = RGBto565(40,40,40);
uint16_t mHandCol = RGBto565(80,80,80);
uint16_t sHandCol = RED;
uint16_t cirCol = YELLOW;
int cirSize = 1;
int hHandL = 25, hHandW = 3;
int mHandL = 34, mHandW = 3;
int sHandL = 42, sHandW = 2;


int sDeg,mDeg,hDeg;
int sDegOld,mDegOld,hDegOld;
unsigned long styleTime, ms;
uint8_t hh = txt2num(__TIME__+0);
uint8_t mm = txt2num(__TIME__+3);
uint8_t ss = txt2num(__TIME__+6);
uint8_t start = 1;

// ----------------------------------------------------------------

uint16_t palette[16];
uint16_t line[128+2];

void imgLineH(int x, int y, int w)
{
  uint8_t v,*img = (uint8_t*)clockface+16*2+6+(y*128+x)/2;
  int ww = (x&1)?w+1:w;
  for(int i=0;i<ww;i+=2) {
    v = pgm_read_byte(img++);
    line[i+0] = palette[v>>4];
    line[i+1] = palette[v&0xf];
  }
  lcd.drawImage(x,y,w,1,(x&1)?line+1:line);
}

void imgRect(int x, int y, int w, int h)
{
  for(int i=y;i<y+w;i++) imgLineH(x,i,w);
}
// ----------------------------------------------------------------

void imgTriangle(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t x3,int16_t y3, uint16_t c=0)
{
  int16_t t1x,t2x,y,minx,maxx,t1xp,t2xp;
  bool changed1 = false,changed2 = false;
  int16_t signx1,signx2,dx1,dy1,dx2,dy2;
  uint16_t e1,e2;

  if (y1>y2) { swap(y1,y2); swap(x1,x2); }
  if (y1>y3) { swap(y1,y3); swap(x1,x3); }
  if (y2>y3) { swap(y2,y3); swap(x2,x3); }

  t1x=t2x=x1; y=y1;   // Starting points

  dx1 = x2 - x1; if(dx1<0) { dx1=-dx1; signx1=-1; } else signx1=1;
  dy1 = y2 - y1;
 
  dx2 = x3 - x1; if(dx2<0) { dx2=-dx2; signx2=-1; } else signx2=1;
  dy2 = y3 - y1;
  
  if (dy1 > dx1) { swap(dx1,dy1); changed1 = true; }
  if (dy2 > dx2) { swap(dy2,dx2); changed2 = true; }
  
  e2 = dx2>>1;
  if(y1==y2) goto next;  // Flat top, just process the second half
  e1 = dx1>>1;
  
  for (uint16_t i = 0; i < dx1;) {
    t1xp=0; t2xp=0;
    if(t1x<t2x) { minx=t1x; maxx=t2x; }
    else    { minx=t2x; maxx=t1x; }
    // process first line until y value is about to change
    while(i<dx1) {
      i++;      
      e1 += dy1;
      while (e1 >= dx1) {
        e1 -= dx1;
               if (changed1) t1xp=signx1;//t1x += signx1;
        else          goto next1;
      }
      if (changed1) break;
      else t1x += signx1;
    }
  // Move line
  next1:
    // process second line until y value is about to change
    while (1) {
      e2 += dy2;    
      while (e2 >= dx2) {
        e2 -= dx2;
        if (changed2) t2xp=signx2;//t2x += signx2;
        else          goto next2;
      }
      if (changed2)     break;
      else              t2x += signx2;
    }
  next2:
    if(minx>t1x) minx=t1x; if(minx>t2x) minx=t2x;
    if(maxx<t1x) maxx=t1x; if(maxx<t2x) maxx=t2x;
    
    // line from min to max points found on the y
    if(c) lcd.drawFastHLine(minx, y, maxx-minx, c); else imgLineH(minx, y, maxx-minx); 
    // increase y
    if(!changed1) t1x += signx1;
    t1x+=t1xp;
    if(!changed2) t2x += signx2;
    t2x+=t2xp;
    y += 1;
    if(y==y2) break;
    
   }
  next:
  
  // Second half
  dx1 = x3 - x2; if(dx1<0) { dx1=-dx1; signx1=-1; } else signx1=1;
  dy1 = y3 - y2;
  t1x=x2;
 
  if (dy1 > dx1) { swap(dy1,dx1); changed1 = true; } else changed1=false;
  
  e1 = dx1>>1;
  
  for (uint16_t i = 0; i<=dx1; i++) {
    t1xp=0; t2xp=0;
    if(t1x<t2x) { minx=t1x; maxx=t2x; }
    else    { minx=t2x; maxx=t1x; }
      // process first line until y value is about to change
    while(i<dx1) {
        e1 += dy1;
          while (e1 >= dx1) {
        e1 -= dx1;
                if (changed1) { t1xp=signx1; break; }//t1x += signx1;
        else          goto next3;
      }
      if (changed1) break;
      else          t1x += signx1;
      if(i<dx1) i++;
    }
  next3:
    // process second line until y value is about to change
    while (t2x!=x3) {
      e2 += dy2;
          while (e2 >= dx2) {
        e2 -= dx2;
        if(changed2) t2xp=signx2;
        else          goto next4;
      }
      if (changed2)     break;
      else              t2x += signx2;
    }        
  next4:
    if(minx>t1x) minx=t1x; if(minx>t2x) minx=t2x;
    if(maxx<t1x) maxx=t1x; if(maxx<t2x) maxx=t2x;
    // line from min to max points found on the y
    if(c) lcd.drawFastHLine(minx, y, maxx-minx, c); else imgLineH(minx, y, maxx-minx); 
    // increase y
    if(!changed1) t1x += signx1;
    t1x+=t1xp;
    if(!changed2) t2x += signx2;
    t2x+=t2xp;
      y += 1;
    if(y>y3) return;
  }
}

// ------------------------------------------------
#define MAXSIN 255
const uint8_t sinTab[91] PROGMEM = {
0,4,8,13,17,22,26,31,35,39,44,48,53,57,61,65,70,74,78,83,87,91,95,99,103,107,111,115,119,123,
127,131,135,138,142,146,149,153,156,160,163,167,170,173,177,180,183,186,189,192,195,198,200,203,206,208,211,213,216,218,
220,223,225,227,229,231,232,234,236,238,239,241,242,243,245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
255
};

int fastSin(int i)
{
  while(i<0) i+=360;
  while(i>=360) i-=360;
  if(i<90)  return(pgm_read_byte(&sinTab[i])); else
  if(i<180) return(pgm_read_byte(&sinTab[180-i])); else
  if(i<270) return(-pgm_read_byte(&sinTab[i-180])); else
            return(-pgm_read_byte(&sinTab[360-i]));
}

int fastCos(int i)
{
  return fastSin(i+90);
}

// ------------------------------------------------

int px[5],py[5];

void drawHand(int deg, int style, int w, int l, int col=0)
{
  int i,num = 4;
  cirCol = YELLOW;
  cirSize = 1;
  switch(style) {
    default:
    case 0:  // tris 013, 230, 024, rect with triangle
      px[0]=-w, py[0]= l-5;
      px[1]=-w, py[1]=-10;
      px[2]= w, py[2]= l-5;
      px[3]= w, py[3]=-10;
      px[4]= 0, py[4]= l-5+7;
      num = 5;
      break;
    case 1:  // tris 013,023, peak style
      px[0]= 0, py[0]= l;
      px[1]=-w-1, py[1]= 0;
      px[2]= w+1, py[2]= 0;
      px[3]= 0, py[3]=-15;
      break;
    case 2:  // tris 013, 230, rect
      px[0]=-w, py[0]= l;
      px[1]=-w, py[1]=-10;
      px[2]= w, py[2]= l;
      px[3]= w, py[3]=-12;
      break;
    case 3:  // tris 013, 230, 024, rect with peak
      px[0]=-w-1, py[0]= l-15;
      px[1]=-w+1, py[1]=-5;
      px[2]= w+1, py[2]= l-15;
      px[3]= w-1, py[3]=-5;
      px[4]= 0, py[4]= l-15+17;
      num = 5;
      cirCol = RED;
      cirSize = 3;
      break;
  }
  int x[5],y[5];
  int cc = fastCos(deg+180);
  int ss = fastSin(deg+180);
  for(i=0;i<num;i++) {
    x[i] = px[i]*cc - py[i]*ss;
    y[i] = px[i]*ss + py[i]*cc;
    x[i] = cx + (x[i]+(x[i]>0?MAXSIN/2:-MAXSIN/2))/MAXSIN;
    y[i] = cy + (y[i]+(y[i]>0?MAXSIN/2:-MAXSIN/2))/MAXSIN;
  }
  imgTriangle(x[0],y[0], x[1],y[1], x[3],y[3], col);
  imgTriangle(x[2],y[2], x[3],y[3], x[0],y[0], col);
  if(num==5) imgTriangle(x[0],y[0], x[2],y[2], x[4],y[4], col);
}

void drawHandS(int deg, int style, int w, int l, int col=0)
{
  int i,num = 4;
  cirCol = YELLOW;
  cirSize = 1;
  switch(style) {
    default:
    case 0:  // tris 013, 230, 024, rect with triangle
      px[0]=-w+1, py[0]= l-5;
      px[1]=-w+1, py[1]=-10;
      px[2]= w-1, py[2]= l-5;
      px[3]= w-1, py[3]=-10;
      px[4]=   0, py[4]= l-5+7;
      num = 5;
      break;
    case 1:  // tris 013,023, peak style
      px[0]= 0, py[0]= l;
      px[1]=-w, py[1]= 0;
      px[2]= w, py[2]= 0;
      px[3]= 0, py[3]=-15;
      break;
    case 2:  // tris 013, 230, rect
      px[0]=-w+1, py[0]= l;
      px[1]=-w+1, py[1]=-10;
      px[2]= w-1, py[2]= l;
      px[3]= w-1, py[3]=-12;
      break;
    case 3:  // tris 013, 230, rect with peak, sec thin long
      px[0]=-w+1, py[0]= l;
      px[1]=-w+1, py[1]=-15;
      px[2]= w-1, py[2]= l;
      px[3]= w-1, py[3]=-15;
      cirCol = RED;
      cirSize = 3;
      break;
  }
  int x[5],y[5];
  int cc = fastCos(deg+180);
  int ss = fastSin(deg+180);
  for(i=0;i<num;i++) {
    x[i] = px[i]*cc - py[i]*ss;
    y[i] = px[i]*ss + py[i]*cc;
    x[i] = cx + (x[i]+(x[i]>0?MAXSIN/2:-MAXSIN/2))/MAXSIN;
    y[i] = cy + (y[i]+(y[i]>0?MAXSIN/2:-MAXSIN/2))/MAXSIN;
  }
  imgTriangle(x[0],y[0], x[1],y[1], x[3],y[3], col);
  imgTriangle(x[2],y[2], x[3],y[3], x[0],y[0], col);
  if(num==5) imgTriangle(x[0],y[0], x[2],y[2], x[4],y[4], col);
}


void nextHandStyle()
{
  if(millis()-styleTime<15000) return;
  styleTime = millis();
  drawHand(hDegOld,style,hHandW,hHandL);
  drawHand(mDegOld,style,mHandW,mHandL);
  drawHandS(sDegOld,style,sHandW,sHandL);
  if(++style>3) style=0;
  start = 1;
}


void clockUpdate() 
{
  if(millis()-ms>=1000 || start) 
  {
    ms = millis();
    if(++ss>59) {
      ss=0;
      if(++mm>59) {
        mm=0;
        if(++hh>23) hh=0;
      }
    }

    sDeg = ss*6;

    if(ss==0 || start) {
      start = 0;
      mDeg = mm*6+sDeg/60;
      hDeg = hh*30+mDeg/12;
      drawHand(hDegOld,style,hHandW,hHandL);
      drawHand(mDegOld,style,mHandW,mHandL);
      mDegOld = mDeg;
      hDegOld = hDeg;
    }
    
    drawHandS(sDegOld,style,sHandW,sHandL);
    drawHand(hDeg,style,hHandW,hHandL,hHandCol);
    drawHand(mDeg,style,mHandW,mHandL,mHandCol);
    drawHandS(sDeg,style,sHandW,sHandL,sHandCol);
    sDegOld = sDeg;

    lcd.fillCircle(cx,cy, cirSize, cirCol);

    //snprintf(buf,20,"%02d:%02d:%02d",hh,mm,ss);
    //lcd.setCursor(0,0); lcd.setTextColor(BLACK);
    //lcd.fillRect(0,0,6*7+8,8,WHITE);  lcd.print(buf);
  }
}

void setup() 
{
  Serial.begin(115200);
  lcd.init();
  lcd.fillScreen(BLACK);

  uint8_t *pal = (uint8_t*)clockface+6;
  for(int i=0;i<16;i++) {
    palette[i] = pgm_read_byte(&pal[i*2+0]);
    palette[i] = palette[i]*256 + pgm_read_byte(&pal[i*2+1]);
    //Serial.print(i); Serial.print(","); Serial.println(palette[i],HEX);
  }
  imgRect(0,0,128,128);
  ms = millis(); 
}

void loop()
{
  nextHandStyle();
  clockUpdate();
}

