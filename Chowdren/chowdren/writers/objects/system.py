from chowdren.writers.objects import ObjectWriter

from mmfparser.data.chunkloaders.objectinfo import (QUICKBACKDROP, BACKDROP, 
    ACTIVE, TEXT, QUESTION, SCORE, LIVES, COUNTER, RTF, SUBAPPLICATION)

from chowdren.common import get_image_name, get_animation_name

class Active(ObjectWriter):
    class_name = 'Active'

    def write_init(self, writer):
        common = self.common
        animations = common.animations.loadedAnimations
        for animation_index, animation in animations.iteritems():
            directions = animation.loadedDirections
            animation_name = get_animation_name(animation.index)
            for direction_index, direction in directions.iteritems():
                writer.putln('add_direction(%s, %s, %s, %s, %s, %s);' % (
                    animation_name, direction_index, direction.minSpeed,
                    direction.maxSpeed, direction.repeat, direction.backTo))
                for image in direction.frames:
                    writer.putln('add_image(%s, %s, &%s);' % (animation_name,
                        direction_index, get_image_name(image)))

class Backdrop(ObjectWriter):
    class_name = 'Backdrop'

    def get_parameters(self):
        return [get_image_name(self.common.image)]
        # objects_file.putln('const static int obstacle_type = %s;' % 
        #     common.getObstacleType())
        # objects_file.putdef('collision_mode', 
        #     common.getCollisionMode())
        # objects_file.putln('image = image%s' % common.image)

class Text(ObjectWriter):
    class_name = 'Text'

class Counter(ObjectWriter):
    class_name = 'Counter'

    def write_init(self, writer):
        counters = common.counters
        counter = common.counter
        if counters:
            display_type = counters.displayType
            if display_type == NUMBERS:
                writer.putln('frames = {')
                writer.indent()
                for char_index, char in enumerate(COUNTER_FRAMES):
                    writer.putln("%r : image%s," % (char,
                        counters.frames[char_index]))
                writer.dedent()
                writer.putln('}')
            elif display_type == HORIZONTAL_BAR:
                shape_object = counters.shape
                shape = shape_object.shape
                fill_type = shape_object.fillType
                if shape != RECTANGLE_SHAPE:
                    raise NotImplementedError
                writer.putdef('width', counters.width)
                writer.putdef('height', counters.height)
                if fill_type == GRADIENT_FILL:
                    writer.putdef('color1', shape_object.color1)
                    writer.putdef('color2', shape_object.color2)
                elif fill_type == SOLID_FILL:
                    writer.putdef('color1', shape_object.color1)
                else:
                    raise NotImplementedError
            else:
                raise NotImplementedError
        writer.putdef('initial', counter.initial)
        writer.putdef('minimum', counter.minimum)
        writer.putdef('maximum', counter.maximum)

system_objects = {
    TEXT : Text,
    ACTIVE : Active,
    BACKDROP : Backdrop
    # COUNTER : 'Counter',
    # SUBAPPLICATION : 'SubApplication',
}