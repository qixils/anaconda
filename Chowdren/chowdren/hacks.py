from chowdren.key import convert_key

is_knytt = False
is_avgn = False
is_anne = False
is_knytt_japan = False
is_test = False
is_hfa = False

def init(converter):
    name = converter.info_dict.get('name').lower()
    global is_knytt
    global is_knytt_japan
    global is_avgn
    global is_anne
    global is_test
    global is_hfa
    is_knytt = 'knytt' in name
    is_avgn = 'angry video game' in name
    is_anne = 'ane' in name
    is_knytt_japan = 'japan' in name
    is_test = 'application' in name
    is_hfa = 'alicia' in name

    if is_avgn:
        # hack to set default keyboard keys
        # 72 - 82
        values = converter.game.globalValues.items
        for i in xrange(71, 82):
            values[i] = convert_key(values[i])

object_checks = {
}

def init_container(converter, container):
    if container.name != 'BETA TESTER SHIT -- TURN OFF WITH FINAL RELEASE':
        return
    container.inactive = True

def write_pre(converter, writer, group):
    pass

def use_simple_or(converter):
    return is_knytt

def use_iteration_index(converter):
    return is_avgn or is_test or is_hfa

alterable_int_objects = [
    'MenuMainMapObject_',
    'MiniMapObject_',
    'MenuMainController',
    'FireShark',
    'Cog'
]

def use_global_int(expression):
    if not is_avgn:
        return False
    index = expression.data.loader.value
    return index in (0, 1)

def use_alterable_int(expression):
    if not is_anne and not is_avgn:
        return False
    obj = expression.get_object()
    name = expression.converter.get_object_name(obj)
    for check in alterable_int_objects:
        if name.startswith(check):
            return True
    return False

def use_safe_division(converter):
    return is_hfa or is_test

def get_startup_instances(converter, instances):
    if not is_hfa:
        return instances
    # bug in Text Blitter object, need to move them to front
    new_instances = []
    text_blitters = []
    for item in instances:
        frameitem = item[1]
        obj = (frameitem.handle, frameitem.objectType)
        writer = converter.all_objects[obj]
        if writer.class_name == 'TextBlitter':
            text_blitters.append(item)
        else:
            new_instances.append(item)

    new_instances += text_blitters
    return new_instances


def use_global_instances(converter):
    return True

def use_update_filtering(converter):
    return is_anne

def write_defines(converter, writer):
    if is_anne:
        writer.putln('#define CHOWDREN_SNES_CONTROLLER')
    if is_avgn or is_anne:
        writer.putln('#define CHOWDREN_FORCE_REMOTE')
    if is_anne or is_avgn or is_hfa:
        writer.putln('#define CHOWDREN_QUICK_SCALE')
    if is_knytt_japan:
        writer.putln('#define CHOWDREN_TEXT_USE_UTF8')
        writer.putln('#define CHOWDREN_TEXT_JAPANESE')
        writer.putln('#define CHOWDREN_BIG_FONT_OFFY 1')
    if is_avgn or is_hfa:
        writer.putln('#define CHOWDREN_STARTUP_WINDOW')
        writer.putln('#define CHOWDREN_POINT_FILTER')
    if is_avgn:
        writer.putln('#define CHOWDREN_PERSISTENT_FIXED_STRING')
    if use_iteration_index(converter):
        writer.putln('#define CHOWDREN_ITER_INDEX')
    if is_avgn or is_test:
        writer.putln('#define CHOWDREN_LAYER_WRAP')
    if is_avgn or is_hfa:
        writer.putln('#define CHOWDREN_RESTORE_ANIMATIONS')
    if is_hfa or is_test:
        writer.putln('#define CHOWDREN_INI_FILTER_QUOTES')
        writer.putln('#define CHOWDREN_INI_KEEP_ORDER')
    if is_hfa:
        writer.putln('#define CHOWDREN_FORCE_TRANSPARENT')
    writer.putln('#define CHOWDREN_USE_DYNTREE')

def get_frames(converter, frames):
    if not is_hfa:
        return frames
    return {
        0: frames[0],
        4 : frames[4],
        21 : frames[21]
    }