
Unicode versions of the X11 "misc-fixed-*" fonts
------------------------------------------------

Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> -- 2008-04-21


This package contains the X Window System bitmap fonts

   -Misc-Fixed-*-*-*--*-*-*-*-C-*-ISO10646-1

These are Unicode (ISO 10646-1) extensions of the classic ISO 8859-1
X11 terminal fonts that are widely used with many X11 applications
such as xterm, emacs, etc.

COVERAGE
--------

None of these fonts covers Unicode completely. Complete coverage
simply would not make much sense here. Unicode 5.1 contains over
100000 characters, and the large majority of them are
Chinese/Japanese/Korean Han ideographs (~70000) and Korean Hangul
Syllables (~11000) that cannot adequately be displayed in the small
pixel sizes of the fixed fonts. Similarly, Arabic characters are
difficult to fit nicely together with European characters into the
fixed character cells and X11 lacks the ligature substitution
mechanisms required for using Indic scripts.

Therefore these fonts primarily attempt to cover Unicode subsets that
fit together with European scripts. This includes the Latin, Greek,
Cyrillic, Armenian, Georgian, and Hebrew scripts, plus a lot of
linguistic, technical and mathematical symbols. Some of the fixed
fonts now also cover Arabic, Thai, Ethiopian, halfwidth Katakana, and
some other non-European scripts.

We have defined 3 different target character repertoires (ISO 10646-1
subsets) that the various fonts were checked against for minimal
guaranteed coverage:

  TARGET1    617 characters
             Covers all characters of ISO 8859 part 1-5,7-10,13-16,
             CEN MES-1, ISO 6937, Microsoft CP1251/CP1252, DEC VT100
             graphics symbols, and the replacement and default
             character. It is intended for small bold, italic, and
             proportional fonts, for which adding block graphics
             characters would make little sense. This repertoire
             covers the following ISO 10646-1:2000 collections
             completely: 1-3, 8, 12.

  TARGET2    886 characters
             Adds to TARGET1 the characters of the Adobe/Microsoft
             Windows Glyph List 4 (WGL4), plus a selected set of
             mathematical characters (covering most of ISO 31-11
             high-school level math symbols) and some combining
             characters. It is intended to be covered by all normal
             "fixed" fonts and covers all European IBM, Microsoft, and
             Macintosh character sets. This repertoire covers the
             following ISO 10646-1:2000 (including Amd 1:2002)
             collections completely: 1-3, 8, 12, 33, 45.

  TARGET3    3282 characters

             Adds to TARGET2 all characters of all European scripts
             (Latin, Greek, Cyrillic, Armenian, Georgian), all
             phonetic alphabet symbols, many mathematical symbols
             (including all those available in LaTeX), all typographic
             punctuation, all box-drawing characters, control code
             pictures, graphical shapes and some more that you would
             expect in a very comprehensive Unicode 4.0 font for
             European users. It is intended for some of the more
             useful and more widely used normal "fixed" fonts. This
             repertoire is, with two exceptions, a superset of all
             graphical characters in CEN MES-3A and covers the
             following ISO 10646-1:2000 (including Amd 1:2002)
             collections completely: 1-12, 27, 30-31, 32 (only
             graphical characters), 33-42, 44-47, 63, 65, 70 (only
             graphical characters).

             [The two MES-3A characters deliberately omitted are the
             angle bracket characters U+2329 and U+232A. ISO and CEN
             appears to have included these into collection 40 and
             MES-3A by accident, because there they are the only
             characters in the Unicode EastAsianWidth "wide" class.]

CURRENT STATUS:

   6x13.bdf 8x13.bdf 9x15.bdf 9x18.bdf 10x20.bdf:

     Complete (TARGET3 reached and checked)

   5x7.bdf 5x8.bdf 6x9.bdf 6x10.bdf 6x12.bdf 7x13.bdf 7x14.bdf clR6x12.bdf:

     Complete (TARGET2 reached and checked)

   6x13B.bdf 7x13B.bdf 7x14B.bdf 8x13B.bdf 9x15B.bdf 9x18B.bdf:

     Complete (TARGET1 reached and checked)

   6x13O.bdf 7x13O.bdf 8x13O.bdf

     Complete (TARGET1 minus Hebrew and block graphics)

[None of the above fonts contains any character that has in Unicode
the East Asian Width Property "W" or "F" assigned. This way, the
desired combination of "half-width" and "full-width" glyphs can be
achieved easily. Most font mechanisms display a character that is not
covered in a font by using a glyph from another font that appears
later in a priority list, which can be arranged to be a "full-width"
font.]

The supplement package

  http://www.cl.cam.ac.uk/~mgk25/download/ucs-fonts-asian.tar.gz

contains the following additional square fonts with Han characters for
East Asian users:

   12x13ja.bdf:

     Covers TARGET2, JIS X 0208, Hangul, and a few more. This font is
     primarily intended to provide Japanese full-width Hiragana,
     Katakana, and Kanji for applications that take the remaining
     ("halfwidth") characters from 6x13.bdf. The Greek lowercase
     characters in it are still a bit ugly and will need some work.

  18x18ja.bdf:

     Covers all JIS X 0208, JIS X 0212, GB 2312-80, KS X 1001:1992,
     ISO 8859-1,2,3,4,5,7,9,10,15, CP437, CP850 and CP1252 characters,
     plus a few more, where priority was given to Japanese han style
     variants. This font should have everything needed to cover the
     full ISO-2022-JP-2 (RFC 1554) repertoire. This font is primarily
     intended to provide Japanese full-width Hiragana, Katakana, and
     Kanji for applications that take the remaining ("halfwidth")
     characters from 9x18.bdf.

  18x18ko.bdf:

     Covers the same repertoire as 18x18ja plus full coverage of all
     Hangul syllables and priority was given to Hanja glyphs in the
     unified CJK area as they are used for writing Korean.

The 9x18 and 6x12 fonts are recommended for use with overstriking
combining characters.

Bug reports, suggestions for improvement, and especially contributed
extensions are very welcome!

INSTALLATION
------------

You install the fonts under Unix roughly like this (details depending
on your system of course):

System-wide installation (root access required):

  cd submission/
  make
  su
  mv -b *.pcf.gz /usr/lib/X11/fonts/misc/
  cd /usr/lib/X11/fonts/misc/
  mkfontdir
  xset fp rehash

Alternative: Installation in your private user directory:

  cd submission/
  make
  mkdir -p ~/local/lib/X11/fonts/
  mv *.pcf.gz ~/local/lib/X11/fonts/
  cd ~/local/lib/X11/fonts/
  mkfontdir
  xset +fp ~/local/lib/X11/fonts   (put this last line also in ~/.xinitrc)

Now you can have a look at say the 6x13 font with the command

  xfd -fn '-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-iso10646-1'

If you want to have short names for the Unicode fonts, you can also
append the fonts.alias file to that in the directory where you install
the fonts, call "mkfontdir" and "xset fp rehash" again, and then you
can also write

  xfd -fn 6x13U

Note: If you use an old version of xfontsel, you might notice that it
treats every font that contains characters >0x00ff as a Japanese JIS
font and therefore selects inappropriate sample characters for display
of ISO 10646-1 fonts. An updated xfontsel version with this bug fixed
comes with XFree86 4.0 / X11R6.8 or newer.

If you use the Exceed X server on Microsoft Windows, then you will
have to convert the BDF files into Microsoft FON files using the
"Compile Fonts" function of Exceed xconfig. See the file exceed.txt
for more information.

There is one significant efficiency problem that X11R6 has with the
sparsely populated ISO10646-1 fonts. X11 transmits and allocates 12
bytes with the XFontStruct data structure for the difference between
the lowest and the highest code value found in a font, no matter
whether the code positions in between are used for characters or not.
Even a tiny font that contains only two glyphs at positions 0x0000 and
0xfffd causes 12 bytes * 65534 codes = 786 kbytes to be requested and
stored by the client. Since all the ISO10646-1 BDF files provided in
this package contain characters in the U+00xx (ASCII) and U+ffxx
(ligatures, etc.) range, all of them would result in 786 kbyte large
XCharStruct arrays in the per_char array of the corresponding
XFontStruct (even for CharCell fonts!) when loaded by an X client.
Until this problem is fixed by extending the X11 font protocol and
implementation, non-CJK ISO10646-1 fonts that lack the (anyway not
very interesting) characters above U+31FF seem to be the best
compromise. The bdftruncate.pl program in this package can be used to
deactivate any glyphs above a threshold code value in BDF files. This
way, we get relatively memory-economic ISO10646-1 fonts that cause
"only" 150 kbyte large XCharStruct arrays to be allocated. The
deactivated glyphs are still present in the BDF files, but with an
encoding value of -1 that causes them to be ignored.

The ISO10646-1 fonts can not only be used directly by Unicode aware
software, they can also be used to create any 8-bit font. The
ucs2any.pl Perl script converts a ISO10646-1 BDF font into a BDF font
file with some different encoding. For instance the command

  perl ucs2any.pl 6x13.bdf MAPPINGS/8859-7.TXT ISO8859-7

will generate the file 6x13-ISO8859-7.bdf according to the 8859-7.TXT
Latin/Greek mapping table, which available from
<ftp://ftp.unicode.org/Public/MAPPINGS/>. [The shell script
./map_fonts automatically generates a subdirectory derived-fonts/ with
many *.bdf and *.pcf.gz 8-bit versions of all the
-misc-fixed-*-iso10646-1 fonts.]

When you do a "make" in the submission/ subdirectory as suggested in
the installation instructions above, this will generate exactly the
set of fonts that have been submitted to the XFree86 project for
inclusion into XFree86 4.0. These consists of all the ISO10646-1 fonts
processed with "bdftruncate.pl U+3200" plus a selected set of derived
8-bit fonts generated with ucs2any.pl.

Every font comes with a *.repertoire-utf8 file that lists all the
characters in this font.


CONTRIBUTING
------------

If you want to help me in extending or improving the fonts, or if you
want to start your own ISO 10646-1 font project, you will have to edit
BDF font files. This is most comfortably done with the gbdfed font
editor (version 1.3 or higher), which is available from

    http://crl.nmsu.edu/~mleisher/gbdfed.html

Once you are familiar with gbdfed, you will notice that it is no
problem to design up to 100 nice characters per hour (even more if
only placing accents is involved).

Information about other X11 font tools and Unicode fonts for X11 in
general can be found on

    http://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html

The latest version of this package is available from

    http://www.cl.cam.ac.uk/~mgk25/download/ucs-fonts.tar.gz

If you want to contribute, then get the very latest version of this
package, check which glyphs are still missing or inappropriate for
your needs, and send me whatever you had the time to add and fix. Just
email me the extended BDF-files back, or even better, send me a patch
file of what you changed. The best way of preparing a patch file is

  ./touch_id newfile.bdf
  diff -d -u -F STARTCHAR oldfile.bdf newfile.bdf >file.diff

which ensures that the patch file preserves information about which
exact version you worked on and what character each "hunk" changes.

I will try to update this packet on a daily basis. By sending me
extensions to these fonts, you agree that the resulting improved font
files will remain in the public domain for everyone's free use. Always
make sure to load the very latest version of the package immediately
before your start, and send me your results as soon as you are done,
in order to avoid revision overlaps with other contributors.

Please try to be careful with the glyphs you generate:

  - Always look first at existing similar characters in order to
    preserve a consistent look and feel for the entire font and
    within the font family. For block graphics characters and geometric
    symbols, take care of correct alignment.

  - Read issues.txt, which contains some design hints for certain
    characters.

  - All characters of CharCell (C) fonts must strictly fit into
    the pixel matrix and absolutely no out-of-box ink is allowed.

  - The character cells will be displayed directly next to each other,
    without any additional pixels in between. Therefore, always make
    sure that at least the rightmost pixel column remains white, as
    otherwise letters will stick together, except of course for
    characters -- like Arabic or block graphics -- that are supposed to
    stick together.

  - Place accents as low as possible on the Latin characters.

  - Try to keep the shape of accents consistent among each other and
    with the combining characters in the U+03xx range.

  - Use gbdfed only to edit the BDF file directly and do not import
    the font that you want to edit from the X server. Use gbdfed 1.3
    or higher.

  - The glyph names should be the Adobe names for Unicode characters
    defined at

      http://www.adobe.com/devnet/opentype/archives/glyph.html

    which gbdfed can set automatically. To make the Edit/Rename Glyphs/
    Adobe Names function work, you have to download the file

      http://www.adobe.com/devnet/opentype/archives/glyphlist.txt

    and configure its location either in Edit/Preferences/Editing Options/
    Adobe Glyph List, or as "adobe_name_file" in "~/.gbdfed".

  - Be careful to not change the FONTBOUNDINGBOX box accidentally in
    a patch.

You should have a copy of the ISO 10646 standard

  ISO/IEC 10646:2003, Information technology -- Universal
  Multiple-Octet Coded Character Set (UCS),
  International Organization for Standardization, Geneva, 2003.
  http://standards.iso.org/ittf/PubliclyAvailableStandards/

and/or the Unicode 5.0 book:

  The Unicode Consortium: The Unicode Standard, Version 5.0,
  Reading, MA, Addison-Wesley, 2006,
  ISBN 9780321480910.
  http://www.amazon.com/exec/obidos/ASIN/0321480910/mgk25

All these fonts are from time to time resubmitted to the X.Org
project, XFree86 (they have been in there since XFree86 4.0), and to
other X server developers for inclusion into their normal X11
distributions.

Starting with XFree86 4.0, xterm has included UTF-8 support. This
version is also available from

  http://dickey.his.com/xterm/xterm.html

Please make the developer of your favourite software aware of the
UTF-8 definition in RFC 2279 and of the existence of this font
collection. For more information on how to use UTF-8, please check out

  http://www.cl.cam.ac.uk/~mgk25/unicode.html
  ftp://ftp.ilog.fr/pub/Users/haible/utf8/Unicode-HOWTO.html

where you will also find information on joining the
linux-utf8@nl.linux.org mailing list.

A number of UTF-8 example text files can be found in the examples/
subdirectory or on 

  http://www.cl.cam.ac.uk/~mgk25/ucs/examples/

