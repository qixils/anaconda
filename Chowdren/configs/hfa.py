def init(converter):
    converter.add_define('CHOWDREN_QUICK_SCALE')
    converter.add_define('CHOWDREN_STARTUP_WINDOW')
    converter.add_define('CHOWDREN_POINT_FILTER')
    converter.add_define('CHOWDREN_RESTORE_ANIMATIONS')
    converter.add_define('CHOWDREN_INI_FILTER_QUOTES')
    converter.add_define('CHOWDREN_INI_KEEP_ORDER')
    converter.add_define('CHOWDREN_FORCE_TRANSPARENT')
    converter.add_define('CHOWDREN_VSYNC')
    converter.add_define('CHOWDREN_IS_HFA')
    converter.add_define('CHOWDREN_OBSTACLE_IMAGE')

    # hack to turn on high-resolution lighting system images
    values = converter.game.globalValues.items
    values[191] = 1 # lights max resolution
    values[194] = 1 # turn off adaptive lights
    values[195] = 1 # lights min resolution
    values[196] = 1 # force small images off

# turn off debug stuff in HFA
deactivate_containers = [
    'BETA TESTER SHIT -- TURN OFF WITH FINAL RELEASE',
    'DEBUG -- turn off!'
]

def init_container(converter, container):
    for name in deactivate_containers:
        if container.name == name:
            container.inactive = True
            return

global_objects = [
    'Scrolling'
]

def use_global_alterables(converter, obj):
    for name in global_objects:
        if obj.data.name.startswith(name):
            return True
    return False

def use_single_global_alterables(converter, obj):
    return True

def use_global_int(converter, expression):
    index = expression.data.loader.value
    return index in (428,)

alterable_int_objects = [
    'MapMainObject',
    'CellFG',
    'CellBG'
]

def use_alterable_int(converter, expression):
    obj = expression.get_object()
    name = expression.converter.get_object_name(obj)
    for check in alterable_int_objects:
        if name.startswith(check):
            return True
    return False

def get_startup_instances(converter, instances):
    if converter.current_frame_index == 0:
        # bug in Text Blitter object, need to move them to front
        new_instances = []
        text_blitters = []
        for item in instances:
            frameitem = item[1]
            obj = (frameitem.handle, frameitem.objectType)
            writer = converter.get_object_writer(obj)
            if writer.class_name == 'TextBlitter':
                text_blitters.append(item)
            else:
                new_instances.append(item)

        new_instances += text_blitters
        return new_instances
    return instances

def write_frame_post(converter, writer):
    if converter.current_frame_index != 26:
        return
    for item in converter.startup_instances:
        frameitem = item[1]
        if frameitem.name == 'Active 4':
            break
    handle = (frameitem.handle, frameitem.objectType, converter.game_index)
    writer.putlnc('%s->move_front();', converter.get_object(handle))

def use_safe_create(converter):
    return True

def get_fonts(converter):
    return ('SmallFonts',)

def get_frames(converter, game, frames):
    new_frames = {}
    if game.index == 0:
        # indexes = (0, 1, 3, 4, 10, 15, 16, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        #            31, 32, 33, 35, 36, 37, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        #            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
        #            63, 64, 65, 66, 67, 68, 69, 70, 71, 72)
        indexes = (32,)
    else:
        indexes = (0, 1, 2, 3, 4, 5, 6)
    for index in indexes:
        new_frames[index] = frames[index]
    return new_frames
