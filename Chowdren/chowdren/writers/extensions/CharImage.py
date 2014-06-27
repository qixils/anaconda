from chowdren.writers.objects import ObjectWriter
from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)
from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class Alias(object):
    def __init__(self, name, characters):
        self.name = name
        self.characters = characters

class Character(object):
    def __init__(self, image, external, clip, size):
        self.image = image
        self.external = external
        self.rect = clip
        self.width, self.height = size

class CharImage(ObjectWriter):
    class_name = 'CharacterImageObject'
    filename = 'charimage'

    def write_init(self, writer):
        data = self.get_data()
        width = data.readInt(True)
        height = data.readInt(True)
        data.skipBytes(60) # logfont?
        data.skipBytes(4) # lastLogFontiPointSize
        data.skipBytes(4) # lastLogFontColor
        data.skipBytes(4) # get page size from object
        pageSize = (data.readInt(), data.readInt())
        text = data.readString(3072)
        lineBreaks = data.readInt() != 0
        wordWrap = data.readInt() != 0
        characters = [data.read(1)
            for _ in xrange(128)]
        characterCount = characterCount = data.readShort()
        data.skipBytes(2)
        aliasCount = data.readInt()
        data.skipBytes(4)
        aliases = []
        for _ in xrange(aliasCount):
            images = []
            external = []
            end = data.tell() + 128 * 2
            for _ in xrange(characterCount):
                images.append(data.readShort())
            data.seek(end)
            end = data.tell() + 128
            for _ in xrange(characterCount):
                external.append(data.readByte() != 0)
            data.seek(end)
            data.skipBytes(4)
            data.skipBytes(4 * 128)
            end = data.tell() + 128 * (4 + 4 + 4 + 4)
            clip = []
            for _ in xrange(characterCount):
                left = data.readInt()
                top = data.readInt()
                right = data.readInt()
                bottom = data.readInt()
                clip.append((left, top, right, bottom))
            data.seek(end)
            data.skipBytes(128 * 4)
            end = data.tell() + 128 * (4 + 4)
            sizes = []
            for _ in xrange(characterCount):
                sizes.append((data.readInt(), data.readInt()))
            data.seek(end)
            name = data.readString(128)
            characters = []
            for i in xrange(characterCount):
                characters.append(Character(
                    images[i], external[i], clip[i], sizes[i]))
            aliases.append(Alias(name, characters))


actions = make_table(ActionMethodWriter, {
    0 : 'set_text'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    1 : '.height'
})

def get_object():
    return CharImage
