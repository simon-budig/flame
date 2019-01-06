/*
 * Flaming Torch  (c) 2013-2019 Simon Budig <simon@budig.de>
 */

#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PIN_BUTTON   3  // Input pin für Button
#define PIN_LED      2  // Output pin für Led-Strip
#define NUM_PIXELS (5 * 64)

#define NUM_MODES 6

// Framebuffer-Dimensions. Depends on the tube radius
#define FLAME_WIDTH  12
#define FLAME_HEIGHT 27

// Intensity buffer for flames and sparks
static uint16_t flamebuffer[FLAME_HEIGHT][FLAME_WIDTH] = { { 0, }, };
static uint16_t sparkbuffer[FLAME_WIDTH] = { 0, };

// Gamma-Lookup-Table
static uint8_t glut[256];

// Neo-Pixel Framebuffer
Adafruit_NeoPixel pixels = Adafruit_NeoPixel (NUM_PIXELS, PIN_LED,
                                              NEO_GRB | NEO_KHZ800);

// Function to show the torch flame
void
render_flame ()
{
  uint16_t i, val;
  uint8_t x, y;

  // Random values at the bottom end, random seeded sparks
  for (x = 0; x < FLAME_WIDTH; x++)
    {
      val = rand() & 0xff;
      val = (val * val) >> 8;
      flamebuffer[FLAME_HEIGHT-1][x] = val;

      if (sparkbuffer[x] == 0)
        {
          if (rand() % 512 == 0)
            sparkbuffer[x] = FLAME_HEIGHT-1;
        }
      else
        {
          sparkbuffer[x] -= 1;
        }
    }

  // propagate enegy and blur. Damping is a fiddle factor.
  for (y = 0; y < FLAME_HEIGHT-1; y++)
    {
      for (x = 0; x < FLAME_WIDTH; x++)
        {
          val  = flamebuffer[(y+1) % FLAME_HEIGHT][x];
          val += flamebuffer[(y+1) % FLAME_HEIGHT][(x+1) % FLAME_WIDTH];
          val += flamebuffer[(y+1) % FLAME_HEIGHT][(x+FLAME_WIDTH-1) % FLAME_WIDTH];
          val += flamebuffer[(y+2) % FLAME_HEIGHT][x];
          val <<= 5;
          val /= 140;

          flamebuffer[y][x] = val;
          if (sparkbuffer[x] && sparkbuffer[x] == y)
            flamebuffer[y][x] = 255;
        }
    }

  for (i = 0; i < NUM_PIXELS; i++)
    {
      val = flamebuffer[i / FLAME_WIDTH][i % FLAME_WIDTH];
      val = MIN (255, val * 3);

      pixels.setPixelColor (i,
                            glut[val],
                            glut[val * 3 / 4],
                            glut[val * 3 / 8]);
    }
}


void
render_blueyellow (const uint16_t t)
{
  uint16_t i;
  uint8_t pos;

  for (i = 0; i < NUM_PIXELS; i++)
    {
      pos = (t + i) % 64;
      if (pos < 32)
        pixels.setPixelColor (i, 255, 200, 0);
      else
        pixels.setPixelColor (i, 0, 0, 255);
    }
}


void
render_rainbow (const uint16_t t)
{
  uint16_t i;
  uint8_t pos, pos2;

  for (i = 0; i < NUM_PIXELS; i++)
    {
      pos = (t + i) % 255;
      pos2 = (pos % 85) * 3;
      if (pos < 85)
        pixels.setPixelColor (i, glut[pos2], glut[0], glut[255 - pos2]);
      else if (pos < 170)
        pixels.setPixelColor (i, glut[255 - pos2], glut[pos2], glut[0]);
      else
        pixels.setPixelColor (i, glut[0], glut[255 - pos2], glut[pos2]);
    }
}


void
render_redblue (const uint16_t t)
{
  uint16_t i;
  uint8_t pos;

  for (i = 0; i < NUM_PIXELS; i++)
    {
      pos = (t + i) % 400;
      if (pos < 85)
        pixels.setPixelColor (i, 0, 0, glut[pos * 3]);
      else if (pos < 136)
        pixels.setPixelColor (i, glut[255 - ((pos - 85) * 5)], 0, 0);
      else
        pixels.setPixelColor (i, 0, 0, 0);
    }
}


void
render_kitt (const uint16_t t)
{
  static uint8_t basecolor = 0;
  uint16_t i;
  uint16_t pos, pos2, p;
  uint8_t *pixdata = pixels.getPixels ();

  pos = (t*2) % (NUM_PIXELS * 2 - 2);
  
  for (i = 0; i < NUM_PIXELS * 3; i++)
    {
      pixdata[i] = (((uint16_t) pixdata[i]) * 7) / 8;
    }

  if (pos >= NUM_PIXELS)
    p = 2 * NUM_PIXELS - 2 - pos;
  else
    p = pos;
    
  basecolor = basecolor + 1 + 0 * ((rand() % 12) + 249) & 0xff;
  pos = (t*1) % 256;
  pos = basecolor % (85*3);
  pos2 = (pos % 85) * 3;
  if (pos < 85)
    {
      pixels.setPixelColor (p,   glut[pos2], glut[0], glut[255 - pos2]);
      pixels.setPixelColor (p+1, glut[pos2], glut[0], glut[255 - pos2]);
    }
  else if (pos < 170)
    {
      pixels.setPixelColor (p,   glut[255 - pos2], glut[pos2], glut[0]);
      pixels.setPixelColor (p+1, glut[255 - pos2], glut[pos2], glut[0]);
    }
  else
    {
      pixels.setPixelColor (p,   glut[0], glut[255 - pos2], glut[pos2]);
      pixels.setPixelColor (p+1, glut[0], glut[255 - pos2], glut[pos2]);
    }
}


void
render_rgbsparks (const uint16_t t)
{
  uint8_t x, y;

  // Random values: factor 3 differentiates between R/G/B

  x = rand() % FLAME_WIDTH;
  y = rand() % FLAME_HEIGHT;
  flamebuffer[y][x] = rand() % (255 * 3);

  for (y = FLAME_HEIGHT; y > 0; )
    {
      y--;

      for (x = 0; x < FLAME_WIDTH; x++)
        {
          switch (flamebuffer[y][x] % 3)
            {
              case 0:
                pixels.setPixelColor (y * FLAME_WIDTH + x,
                                      glut[flamebuffer[y][x] / 3], 0, 0);
                break;
              case 1:
                pixels.setPixelColor (y * FLAME_WIDTH + x,
                                      0, glut[flamebuffer[y][x] / 3], 0);
                break;
              case 2:
                pixels.setPixelColor (y * FLAME_WIDTH + x,
                                      0, 0, glut[flamebuffer[y][x] / 3]);
                break;
            }

          // Deal with multiples of three, this ensures the same base color
          // the condition here is false always, if enabed this makes the
          // colorful sparks go up.
          if (t % 6 == 7)
            {
              if (y > 1)
                flamebuffer[y][x] = MAX (9, flamebuffer[y-1][x]) - 9;
              else
                flamebuffer[y][x] = 0;
            }
          else
            {
              flamebuffer[y][x] = MAX (9, flamebuffer[y][x]) - 9;
            }
        }
    }
}


// Arduino init.

void
setup ()
{
  uint16_t i;
  uint8_t state;
  float rf;

  // calculate Gamma-Table
  for (i = 0; i < 256; i++)
    {
      rf = i / 255.0;
      rf = pow (rf, 2.2);
      glut[i] = 255.0 * rf;
    }

  // Button Pin Input, internal Pullup
  pinMode (PIN_BUTTON, INPUT_PULLUP);

  // initial button test to make it possible
  // to skip modes taking too much power (--> reset

   if (!digitalRead (PIN_BUTTON))
     {
       state = EEPROM.read(0);
       state = (state + 1) % NUM_MODES;
       EEPROM.write (0, state);
     }

  // initialize Neopixel library
  pixels.begin();
}


// Arduino Loop function. Repeats continuously

void
loop ()
{
  uint16_t i;
  static uint16_t t = 0xffff;
  static uint8_t pressed = 0;
  static uint8_t state = 0xff;
  uint8_t delay_value = 0;

#ifdef MAGIC_KEY_POS
  // for atmega32u4 based Arduinos:
  //
  // check if the bootloader has been activated.
  // avoid doing any rendering to prevent the
  // MAGIC_KEY getting overridden which in turn
  // would prevent entering the bootloader properly.

  if (*((uint16_t *) MAGIC_KEY_POS) == MAGIC_KEY &&
      WDTCSR & (1 << WDE))
    {
      return;
    }
#endif
    
  if (state >= NUM_MODES)
    state = EEPROM.read(0);
  if (state >= NUM_MODES)
    state = 0;

#ifdef MAGIC_KEY_POS
  // Now, this is quite unfortunate:
  //
  // for the atmege32u4 based arduinos (Leonardo, pro micro etc.)
  // entering the bootloader is initiated in the USB interrupt
  // handler (i.e. can happen at any time).
  //
  // This does two things: writes MAGIC_KEY to MAGIC_KEY_POS and
  // enables the watchdog reset.
  //
  // If the watchdog fires the atmega32u4 resets and the bootloader
  // code checks for the MAGIC_KEY at MAGIC_KEY_POS. If it finds
  // the MAGIC_KEY it sticks in the bootloader mode.
  //
  // for larger LED strips it is quite likely that MAGIC_KEY_POS
  // resides in the middle of the framebuffer. And if the USB interrupt
  // happens while the code is rendering stuff to the framebuffer,
  // it then might happen that the MAGIC_KEY immediately gets overwritten
  // by the rendering code. This prevents that the bootloader gets
  // entered upon the watchdog reset. For some effects the AVR is mostly
  // rendering, making it basically impossible to enter the bootloader
  // via the IDE.
  //
  // As a workaround we disable all interrupts during the rendering code
  // which is quite a brute force method. This delays the writing of the
  // MAGIC_KEY to the point of the sei() (since this is now the point where
  // the USB interrupt gets handled), giving the MAGIC_KEY precedence over
  // the rendered effect.
  //
  // and since we basically avoid running loop() when the
  // bootloader-conditions are met (see above) the switch to the bootloader
  // now is more reliable again.

  cli ();
#endif

  switch (state)
    {
      case 0:
        render_flame ();
        break;

      case 1:
        render_blueyellow (t);
        delay_value = 10;
        break;

      case 2:
        render_rainbow (t);
        delay_value = 10;
        break;

      case 3:
        render_redblue (t);
        delay_value = 10;
        break;

      case 4:
        render_kitt (t);
        delay_value = 20;
        break;
        
      case 5:
        render_rgbsparks (t);
        delay_value = 10;
        break;

      default:
        render_flame ();
        break;
    }

#ifdef MAGIC_KEY_POS
  sei ();
#endif

  // the actual delay relies on interupts, hence
  // we have to do the per-frame-waiting after the sei();
  
  if (delay_value)
    {
      delay (delay_value);
    }

  // Time-Tick. Needed for moving stripes
  t--;

  // update Pixels
  pixels.show ();

  // Button-Handling  (inverted logic: 0 = button pressed)
  if (!digitalRead (PIN_BUTTON))
    {
      // for software-debouncing on a pressed button we count down to 0
      // for each frame.
      pressed = MAX (pressed, 1) - 1;

      // at 1 we clear the framebuffer and switch mode.
      if (pressed == 1)
        {
          memset (flamebuffer, 0x00, sizeof (flamebuffer));
          memset (sparkbuffer, 0x00, sizeof (sparkbuffer));
          state = (state + 1) % NUM_MODES;
          EEPROM.write (0, state);
          for (i = 0; i < NUM_PIXELS; i++)
            {
              pixels.setPixelColor (i, 0, 0, 0);
            }
        }
    }
  else
    {
      pressed = 5;
    }
}
