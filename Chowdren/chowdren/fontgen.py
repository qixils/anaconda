import sys
sys.path.append('..')
import freetype
from ctypes import byref
from mmfparser.bytereader import ByteReader

"""
XXX use kerning information

Format:

struct Glyph
{
    uint32 charcode
    float x1, y1, x2, y2
    float advance_x, advance_y
    float corner_x, corner_y
    uint32 width, height
    uint8 data[width * height];
};

struct Font
{
    uint32 size;
    float width;
    float height;
    float ascender;
    float descender;
    uint32 num_glyphs;
    Glyph glyphs[num_glyphs];
};

struct FontBank
{
    uint32 font_count;
    Font fonts[font_count];
};
"""

class Glyph(object):
    def __init__(self, generator, char):
        self.char = char
        self.font = generator.font
        self.glyph = glyph = generator.font.glyph
        self.advance = (glyph.advance.x / 64.0, glyph.advance.y / 64.0)
        box = self.get_cbox()
        self.x1 = box.xMin / 64.0
        self.y1 = box.yMin / 64.0
        self.x2 = box.xMax / 64.0
        self.y2 = box.yMax / 64.0
        real_glyph = glyph.get_glyph()
        bitmap_glyph = real_glyph.to_bitmap(freetype.FT_RENDER_MODE_NORMAL, 0,
                                            True)
        self.corner = (bitmap_glyph.left, bitmap_glyph.top)
        bitmap = bitmap_glyph.bitmap
        self.width = bitmap.width
        self.height = bitmap.rows
        self.buf = bitmap.buffer
        # if (stroke) {
        #     FT_Stroker stroker;
        #     FT_Stroker_New(*FTLibrary::Instance().GetLibrary(), &stroker);
        #     FT_Stroker_Set(stroker,
        #         180,
        #         FT_STROKER_LINECAP_ROUND,
        #         FT_STROKER_LINEJOIN_ROUND,
        #         0
        #     );
        #     FT_Glyph_StrokeBorder(&actual_glyph, stroker, 0, 1);
        #     FT_Stroker_Done(stroker);
        # }

    def get_cbox(self):
        outline = self.glyph.outline
        bbox = freetype.FT_BBox()
        freetype.FT_Outline_Get_CBox(byref(outline._FT_Outline), byref(bbox))
        return freetype.BBox(bbox)

    def write(self, writer):
        writer.writeInt(ord(self.char), True)
        writer.writeFloat(self.x1)
        writer.writeFloat(self.y1)
        writer.writeFloat(self.x2)
        writer.writeFloat(self.y2)
        writer.writeFloat(self.advance[0])
        writer.writeFloat(self.advance[1])
        writer.writeFloat(self.corner[0])
        writer.writeFloat(self.corner[1])
        writer.writeInt(self.width)
        writer.writeInt(self.height)
        buf = ''
        for c in self.buf:
            buf += chr(c)
        writer.write(buf)


RESOLUTION = 96


class Font(object):
    def __init__(self, name, size, charset):
        self.size = size
        size = int(size * 64)
        font = self.font = freetype.Face(name)

        font.set_char_size(size, size, RESOLUTION, RESOLUTION)
        self.width = self.get_width()
        self.height = self.get_height()
        metrics = self.font.size
        self.ascender = metrics.ascender / 64.0
        self.descender = metrics.descender / 64.0
        self.glyphs = []
        self.charmap = {}
        charcode, agindex = font.get_first_char()
        while agindex != 0:
            self.charmap[agindex] = charcode
            charcode, agindex = font.get_next_char(charcode, agindex)
        for c in charset:
            self.load_char(c)

        # FT_HAS_KERNING((*ftFace)) != 0

    def get_width(self):
        size = self.font.size
        if self.font.is_scalable:
            bbox = self.font.bbox
            return (bbox.xMax - bbox.xMin) * (float(size.x_ppem) /
                                              self.font.units_per_EM)
        else:
            return size.max_advance / 64.0

    def get_height(self):
        size = self.font.size
        if self.font.is_scalable:
            bbox = self.font.bbox
            return (bbox.yMax - bbox.yMin) * (float(size.y_ppem) /
                                              self.font.units_per_EM)
        else:
            return size.height / 64.0

    def load_char(self, c):
        self.font.load_char(c, freetype.FT_LOAD_FORCE_AUTOHINT)
        glyph = Glyph(self, c)
        if (glyph.x1 == 0.0 and glyph.y1 == 0.0 and
            glyph.x2 == 0.0 and glyph.y2 == 0.0):
            return
        self.glyphs.append(glyph)

    def write(self, writer):
        writer.writeInt(self.size)
        writer.writeFloat(self.width)
        writer.writeFloat(self.height)
        writer.writeFloat(self.ascender)
        writer.writeFloat(self.descender)
        writer.writeInt(len(self.glyphs))
        for glyph in self.glyphs:
            glyph.write(writer)

    def convert_monospace(self):
        # create a monospace-like version of the font.
        # this is useful for japanese fonts where you don't want variations
        # on font widths
        max_width = 0
        for glyph in self.glyphs:
            max_width = max(glyph.advance[0], max_width)
        print 'Max width used for advance:', max_width
        for glyph in self.glyphs:
            glyph.advance = (max_width, glyph.advance[1])

def generate_font(filename, charset, out, sizes):
    fonts = []
    for size in sizes:
        fonts.append(Font(filename, size, charset))
    write_fonts(out, fonts)

def write_fonts(out, fonts):
    fp = open(out, 'wb')
    writer = ByteReader(fp)
    writer.writeInt(len(fonts))
    for font in fonts:
        font.write(writer)
    fp.close()