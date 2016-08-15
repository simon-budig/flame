![Nicht verbrennen!](/images/fackel.jpg)
# flame.ino

This is a small arduino sketch to drive a string of WS2812 LEDs wound around
a tube, to simulate a flaming torch.

I have carried around the torch on various events and it has proven quite
popular and served as inspiration for other similiar builds, see for example
the work of Lukas, who expanded on the idea and added message scrolling:
https://github.com/plan44/messagetorch

This code is built for 5m LED strip with 64 led/m. The circumference of the
tube is about 17-20 cm, so that after 12 leds one winding is complete.

This is an arduino sketch, however, there are two small issues with this:

It needs a patched Adafruit-Neopixel library, where I extended the
constructor with a pointer to pre-allocated memory. That way I get some
control on where in the memory the framebuffer is residing.

Also, the memory in the arduino leonardo for 320 LEDs is tight. It tends
to interfere with the bootloader activation since the "Catarina"
Bootloader relies on some values in very specific memory locations to
decide if it starts the main application or not. This value sometimes
gets clobbered by the flame rendering code, in extreme cases it might be
necessary to reflash your leonardo via ISP.

I tried to keep the code clean and have the most important values as
defines, so that it can be adjusted to different led layouts.

Have fun!
        Simon
