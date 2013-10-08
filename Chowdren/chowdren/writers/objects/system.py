from chowdren.writers.objects import ObjectWriter

from mmfparser.data.chunkloaders.objectinfo import (QUICKBACKDROP, BACKDROP, 
    ACTIVE, TEXT, QUESTION, SCORE, LIVES, COUNTER, RTF, SUBAPPLICATION)

from mmfparser.data.chunkloaders.objects import (COUNTER_FRAMES, 
    ANIMATION_NAMES, NUMBERS, HIDDEN, VERTICAL_BAR, HORIZONTAL_BAR, 
    VERTICAL_GRADIENT, HORIZONTAL_GRADIENT, RECTANGLE_SHAPE, SOLID_FILL,
    GRADIENT_FILL, FINE_COLLISION)

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

class Active(ObjectWriter):
    class_name = 'Active'

    def write_init(self, writer):
        common = self.common
        animations = common.animations.loadedAnimations
        writer.putln('static Animations * saved_animations = '
                     'new Animations(%s);' % (max(animations)+1))
        writer.putln('this->animations = saved_animations;')
        writer.putln('static bool initialized = false;')
        writer.putln('if (!initialized) {')
        writer.indent()
        writer.putln('initialized = true;')
        for animation_index, animation in animations.iteritems():
            writer.putln('init_anim_%s();' % animation_index)
        writer.putln('initialize_animations();')
        writer.end_brace()
        flags = common.newFlags
        writer.putln(to_c('collision_box = %s;', flags['CollisionBox']))
        # if flags['AutomaticRotation']:
        #     print 
        #     raise NotImplementedError
        writer.putln('animation = %s;' % get_animation_name(min(animations)))
        writer.putln('initialize_active();')

    def write_class(self, writer):
        animations = self.common.animations.loadedAnimations
        for animation_index, animation in animations.iteritems():
            writer.putmeth('void init_anim_%s' % animation_index)
            directions = animation.loadedDirections
            animation_name = get_animation_name(animation.index)
            for direction_index, direction in directions.iteritems():
                loop_count = direction.repeat
                if loop_count not in (0, 1):
                    print 'loop count not supported:', loop_count
                repeat = loop_count == 0
                writer.putln(to_c('add_direction(%s, %s, %s, %s, %s, %s);',
                    animation_name, direction_index, direction.minSpeed,
                    direction.maxSpeed, loop_count, direction.backTo))
                for image in direction.frames:
                    writer.putln('add_image(%s, %s, %s);' % (animation_name,
                        direction_index, get_image_name(image)))
            writer.end_brace()

    def get_images(self):
        images = set()
        common = self.common
        animations = common.animations.loadedAnimations
        for animation in animations.values():
            directions = animation.loadedDirections
            for direction in directions.values():
                images.update(direction.frames)
        return images

class Backdrop(ObjectWriter):
    class_name = 'Backdrop'

    def initialize(self):
        obstacle = self.common.getObstacleType()
        if obstacle not in ('None', 'Solid'):
            print 'obstacle type', obstacle, 'not supported'
            raise NotImplementedError

    def write_init(self, writer):
        image = get_image_name(self.common.image, False)
        writer.putln('image = new Image(%s);' % image)
        writer.putln('image->hotspot_x = image->hotspot_y = 0;')
        if self.data.name.endswith('(DRC)'):
            writer.putraw('#ifdef CHOWDREN_IS_WIIU')
            writer.putln('remote = CHOWDREN_REMOTE_TARGET;')
            writer.putraw('#endif')
        if self.common.getObstacleType() != 'Solid':
            return
        writer.putln('width = image->width;')
        writer.putln('height = image->height;')
        if self.common.collisionMode == FINE_COLLISION:
            writer.putln('collision = new SpriteCollision(this, image);')
        else:
            writer.putln('collision = new InstanceBox(this);')

    def get_images(self):
        return [self.common.image]

class QuickBackdrop(ObjectWriter):
    class_name = 'QuickBackdrop'

    def initialize(self):
        obstacle = self.common.getObstacleType()
        if obstacle not in ('Solid', 'None'):
            print 'obstacle type', obstacle, 'not supported'
            raise NotImplementedError
        shape = self.common.shape
        if shape.getShape() != 'Rectangle':
            print 'shape', shape.getShape(), 'not supported'
            raise NotImplementedError
        fill = shape.getFill()
        if fill not in ('Solid', 'Gradient'):
            print 'fill', shape.getFill(), 'not supported'
            raise NotImplementedError
        border = shape.borderColor
        color1 = shape.color1
        color2 = shape.color2
        has_color2 = color2 is not None
        border_size = shape.borderSize
        if border_size != 0 and (fill == 'None' or border != color1
                                 or (has_color2 and border != color2)):
            print fill, color1, color2, border
            print 'border size', shape.borderSize, 'not supported'
            raise NotImplementedError

    def write_init(self, writer):
        shape = self.common.shape
        color1 = shape.color1
        color2 = shape.color2

        writer.putln('width = %s;' % self.common.width)
        writer.putln('height = %s;' % self.common.height)
        writer.putln('color = %s;' % make_color(color1))
        fill = shape.getFill()
        if fill == 'Gradient':
            gradient = shape.getGradientType()
            if gradient == 'Horizontal':
                writer.putln('gradient_type = HORIZONTAL_GRADIENT;')
            else:
                writer.putln('gradient_type = VERTICAL_GRADIENT;')
            writer.putln('color2 = %s;' % make_color(color2))
        elif color2 is not None:
            raise NotImplementedError
        else:
            writer.putln('gradient_type = NONE_GRADIENT;')

        if self.common.getObstacleType() == 'Solid':
            writer.putln('collision = new InstanceBox(this);')

        # objects_file.putln('const static int obstacle_type = %s;' % 
        #     common.getObstacleType())
        # objects_file.putdef('collision_mode', 
        #     common.getCollisionMode())
        # objects_file.putln('image = image%s' % common.image)

class Text(ObjectWriter):
    class_name = 'Text'

    def initialize(self):
        pass
        # if self.is_background():
        #     raise NotImplementedError

    def write_init(self, writer):
        text = self.common.text
        lines = [paragraph.value for paragraph in text.items]
        for paragraph in text.items:
            writer.putln(to_c('add_line(%r);', paragraph.value))
        # objects_file.putln('font = font%s' % text.items[0].font)
        writer.putln('width = %s;' % text.width)
        writer.putln('height = %s;' % text.height)
        writer.putln('color = %s;' % make_color(text.items[0].color))
        font = text.items[0].font
        writer.putln('bold = font%s.bold;' % font)
        writer.putln('italic = font%s.italic;' % font)
        
        paragraph = text.items[0]
        flags = []
        if paragraph.flags['HorizontalCenter']:
            horizontal = 'ALIGN_HCENTER'
        elif paragraph.flags['RightAligned']:
            horizontal = 'ALIGN_RIGHT'
        else:
            horizontal = 'ALIGN_LEFT'
        if paragraph.flags['VerticalCenter']:
            vertical = 'ALIGN_VCENTER'
        elif paragraph.flags['BottomAligned']:
            vertical = 'ALIGN_BOTTOM'
        else:
            vertical = 'ALIGN_TOP'
        writer.putln('alignment = %s | %s;' % (horizontal, vertical))

    def is_background(self):
        return False

class RTFText(ObjectWriter):
    class_name = 'Text'

    def initialize(self):
        pass

    def write_init(self, writer):
        text = self.common.rtf
        writer.putln(to_c('add_line("");',))
        # objects_file.putln('font = font%s' % text.items[0].font)
        writer.putln('width = %s;' % text.width)
        writer.putln('height = %s;' % text.height)
        writer.putln('color = Color(0, 0, 0);')
        writer.putln('bold = false;')
        writer.putln('italic = false;')

        writer.putln('alignment = ALIGN_HCENTER | ALIGN_VCENTER;')

    def is_background(self):
        return False

class Counter(ObjectWriter):
    class_name = 'Counter'

    def write_init(self, writer):
        common = self.common
        counters = common.counters
        counter = common.counter
        if counters:
            display_type = counters.displayType
            if display_type == NUMBERS:
                for char_index, char in enumerate(COUNTER_FRAMES):
                    writer.putln("images[%s] = %s;" % (char_index,
                        get_image_name(counters.frames[char_index])))
            elif display_type == HORIZONTAL_BAR:
                print 'horizontal bar not implemented'
                return
                shape_object = counters.shape
                shape = shape_object.shape
                fill_type = shape_object.fillType
                if shape != RECTANGLE_SHAPE:
                    raise NotImplementedError
                writer.putln('width = %s;' % counters.width)
                writer.putln('height = %s;' % counters.height)
                if fill_type == GRADIENT_FILL:
                    writer.putdef('color1', shape_object.color1)
                    writer.putdef('color2', shape_object.color2)
                elif fill_type == SOLID_FILL:
                    writer.putdef('color1', shape_object.color1)
                else:
                    raise NotImplementedError
            else:
                raise NotImplementedError

    def get_parameters(self):
        counter = self.common.counter
        return [counter.initial, counter.minimum, counter.maximum]

    def get_images(self):
        common = self.common
        counters = common.counters
        counter = common.counter
        if counters:
            display_type = counters.displayType
            if display_type == NUMBERS:
                return counters.frames
        return []

system_objects = {
    TEXT : Text,
    ACTIVE : Active,
    BACKDROP : Backdrop,
    QUICKBACKDROP : QuickBackdrop,
    COUNTER : Counter,
    RTF : RTFText,
    # SUBAPPLICATION : 'SubApplication',
}