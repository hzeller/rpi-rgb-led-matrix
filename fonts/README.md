## Provided fonts
These are BDF fonts, a simple bitmap font-format that can be created
by many font tools. Given that these are bitmap fonts, they will look good on
very low resolution screens such as the LED displays.

Fonts in this directory (except tom-thumb.bdf) are public domain (see the [README](./README)) and
help you to get started with the font support in the API or the `text-util`
from the utils/ directory.

Tom-Thumb.bdf is included in this directory under [MIT license](http://vt100.tarunz.org/LICENSE). Tom-thumb.bdf was created by [@robey](http://twitter.com/robey) and originally published at https://robey.lag.net/2010/01/23/tiny-monospace-font.html

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

[otf2bdf]: https://github.com/jirutka/otf2bdf
