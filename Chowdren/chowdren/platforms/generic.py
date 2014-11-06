from chowdren.platforms.common import Platform
from cStringIO import StringIO
from mmfparser.bytereader import ByteReader

class GenericPlatform(Platform):
    def get_image(self, image):
        temp = StringIO()
        image.save(temp, 'PNG')
        return temp.getvalue()

    def get_shader(self, vert, frag):
        writer = ByteReader()
        writer.writeInt(len(vert))
        writer.write(vert)
        writer.writeInt(len(frag))
        writer.write(frag)
        return str(writer)
