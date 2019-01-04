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

Originally there were some funky problems since for larger numbers of LEDs the
rendering code tends to interfere with the bootloader activation since the
"Catarina" Bootloader relies on some values in very specific memory locations
to decide if it starts the main application or not. This value was prone to get
clobbered by the flame rendering code, in extreme cases it even required to
reflash your leonardo via ISP.

I now have implemented a somewhat ugly workaround. By strategically dis- and
re-enabling interrupts and checking for the bootloader-entering-condition at
the beginning of loop() the code now makes entering the bootloader via the IDE
a lot more reliable.

I tried to keep the code clean and have the most important values as
defines, so that it can be adjusted to different led layouts.

Have fun!
        Simon
