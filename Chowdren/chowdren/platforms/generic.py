from chowdren.platforms.common import Platform
from cStringIO import StringIO

class GenericPlatform(Platform):
    def get_image(self, image):
        temp = StringIO()
        image.save(temp, 'PNG')
        return temp.getvalue()
