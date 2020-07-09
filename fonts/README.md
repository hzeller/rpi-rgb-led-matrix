## Provided fonts
These are BDF fonts, a simple bitmap font-format that can be created
by many font tools. Given that these are bitmap fonts, they will look good on
very low resolution screens such as the LED displays.

Fonts in this directory (except tom-thumb.bdf) are public domain (see the [README](./README)) and
help you to get started with the font support in the API or the `text-util`
from the utils/ directory.

Tom-Thumb.bdf is included in this directory under [MIT license](http://vt100.tarunz.org/LICENSE). Tom-thumb.bdf was created by [@robey](http://twitter.com/robey) and originally published at https://robey.lag.net/2010/01/23/tiny-monospace-font.html

The texguire-27.bdf font was created using the [otf2bdf] tool from the TeX Gyre font.
```
otf2bdf -v -o texgyre-27.bdf -r 72 -p 27 texgyreadventor-regular.otf
```

## Create your own

Fonts are in a human readable and editbable `*.bdf` format, but unless you
like reading and writing pixels in hex, generating them is probably easier :)

You can use any font-editor to generate a BDF font or use the conversion
tool [otf2bdf] to create one from some other font format.

Here is an example how you could create a 30pixel high BDF font from some
TrueType font:

```bash
otf2bdf -v -o myfont.bdf -r 72 -p 30 /path/to/font-Bold.ttf
```

## Getting otf2bdf

Installing the tool should be fairly straight-foward

```
sudo apt-get install otf2bdf
```

## Compiling otf2bdf

If you like to compile otf2bdf, you might notice that the configure script
uses some old way of getting the freetype configuration. There does not seem
to be much activity on the mature code, so let's patch that first:

```
sudo apt-get install -y libfreetype6-dev pkg-config autoconf
git clone https://github.com/jirutka/otf2bdf.git   # check it out
cd otf2bdf
patch -p1 <<"EOF"
--- a/configure.in
+++ b/configure.in
@@ -5,8 +5,8 @@ AC_INIT(otf2bdf.c)
 AC_PROG_CC

 OLDLIBS=$LIBS
-LIBS="$LIBS `freetype-config --libs`"
-CPPFLAGS="$CPPFLAGS `freetype-config --cflags`"
+LIBS="$LIBS `pkg-config freetype2 --libs`"
+CPPFLAGS="$CPPFLAGS `pkg-config freetype2 --cflags`"
 AC_CHECK_LIB(freetype, FT_Init_FreeType, LIBS="$LIBS -lfreetype",[
              AC_MSG_ERROR([Can't find Freetype library! Compile FreeType first.])])
 AC_SUBST(LIBS)
EOF

autoconf       # rebuild configure script
./configure    # run configure
make           # build the software
sudo make install   # install it
```

[otf2bdf]: https://github.com/jirutka/otf2bdf
