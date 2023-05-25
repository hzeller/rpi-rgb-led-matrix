import os, time, logging
from math import pi, sin, cos, sqrt, atan2, asin
from PIL import Image

path = os.path.dirname(__file__) + "/"
log = logging.getLogger(__name__)

day = path + "images/day.png" 
night = path + "images/night.png"

blur = 10.          # blur angle for terminator
phong = False       # enable Phong shading
shad_div = 260.     # shading intensity dividend (higher value -> brighter shading)
diff_int = 1.       # diffuse intensity
spec_exp = 4        # specular reflection exponent (0 = diffuse only; > 50 = metallic)
res = (64, 32)

tpi = 2 * pi
degs = 180 / pi
rads = pi / 180

def init():
    t = time.gmtime(time.time())
    y = t[0]
    m = t[1]
    d = t[2]
    h = t[3]
    mins = t[4]

    h = h + mins / 60.
    return y, m, d, h

def FNday(y, m, d, h):
    days = 367 * y - 7 * (y + (m + 9) // 12) // 4 + 275 * m // 9 + d - 730530 + h / 24.
    return float(days)

def rev(x):
    rv = x - int(x / 360) * 360
    if rv < 0:
        rv += 360
    return rv

def calc_ra_dec(y, m, d, h):
    global L

    d = FNday(y, m, d, h)

    w = 282.9404 + 4.70935E-5 * d
    a = 1.000000
    e = 0.016709 - 1.151E-9 * d
    M = 356.0470 + 0.9856002585 * d
    M = rev(M)

    oblecl = 23.4393 - 3.563E-7 * d
    L = rev(w + M)

    E = M + degs * e * sin(M * rads) * (1 + e * cos(M * rads))

    x = cos(E * rads) - e
    y = sin(E * rads) * sqrt(1 - e * e)
    r = sqrt(x * x + y * y)
    v = atan2(y, x) * degs
    lon = rev(v + w)

    xequat = r * cos(lon * rads)
    yequat = r * sin(lon * rads) * cos(oblecl * rads)
    zequat = r * sin(lon * rads) * sin(oblecl * rads)

    RA = atan2(yequat, xequat) * degs / 15
    Decl = asin(zequat / r) * degs

    return RA, Decl

def calc_alt(RA, Decl, lat, long, h):
    GMST0 = (L * rads + 180 * rads) / 15 * degs
    SIDTIME = GMST0 + h + long / 15
    HA = rev((SIDTIME - RA)) * 15

    x = cos(HA * rads) * cos(Decl * rads)
    y = sin(HA * rads) * cos(Decl * rads)
    z = sin(Decl * rads)

    xhor = x * sin(lat * rads) - z * cos(lat * rads)
    yhor = y
    zhor = x * cos(lat * rads) + z * sin(lat * rads)

    altitude = atan2(zhor, sqrt(xhor * xhor + yhor * yhor)) * degs

    return altitude

def xy2ll(x, y, res):
    lat = 90. - float(y) / res[1] * 180.
    lon = float(x) / res[0] * 360. - 180.
    return lat, lon

def mixp(a, b, x):
    c = []
    for ai, bi in zip(a, b):
        c.append(int((1 - x) * ai + x * bi))
    return tuple(c)

def mul_tup(a, x):
    b = []
    for i in a:
        b.append(int(x * i))
    return tuple(b)

def plot(x, y, alt, width):
    ix = 3 * int(y * width + x)
    if alt > blur and not phong:
        odat[ix:ix + 3] = ddat[ix:ix + 3]
    elif alt > blur:
        dc = ddat[ix:ix + 3]
        nc = ndat[ix:ix + 3]
        i = sin(rads * alt)
        shad_int = min(2., max(1., shad_div / float(100. + dc[0] + dc[1] + dc[2])))
        shad_int *= (shad_int - .98) ** .2  # reduce brightness in deserts
        odat[ix:ix + 3] = mul_tup(dc, 1. + .5 * (diff_int * i + i ** spec_exp) * shad_int)
    elif alt < -blur:
        odat[ix:ix + 3] = ndat[ix:ix + 3]
    else:
        dc = ddat[ix:ix + 3]
        nc = ndat[ix:ix + 3]
        odat[ix:ix + 3] = mixp(nc, dc, (alt + blur) / blur / 2.)

def calculate_image():
    global ddd, nnn, ddat, ndat, odat, res

    ddd = Image.open(day)
    nnn = Image.open(night)

    ddat = list(ddd.tobytes())
    ndat = list(nnn.tobytes())
    odat = [0] * (3 * res[0] * res[1])

    y, m, d, h = init()
    ra, dec = calc_ra_dec(y, m, d, h)
    hx = res[0] / 2
    hy = res[1] / 2

    for y in range(int(res[1])):
        for x in range(res[0]):
            lat, lon = xy2ll(x, y, res)
            alt = calc_alt(ra, dec, lat, lon, h)
            plot(x, y, alt, res[0])
    for n in range(len(odat)):
        odat[n] = max(0, min(255, odat[n]))

    img = Image.new('RGB', res)
    img.putdata([(odat[i], odat[i + 1], odat[i + 2]) for i in range(0, len(odat), 3)])
    return img


class Earth:
    def __init__(self, offscreen_canvas):
        self.framerate = 1
        self.offscreen_canvas = offscreen_canvas
        self.offscreen_canvas.SetImage(calculate_image())

    def get_framerate(self):
        return self.framerate

    def show(self, matrix):
        self.offscreen_canvas = matrix.SwapOnVSync(self.draw())

    def draw(self):
        self.offscreen_canvas.SetImage(calculate_image())
        return self.offscreen_canvas
