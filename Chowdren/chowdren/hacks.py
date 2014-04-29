from chowdren.key import convert_key

is_knytt = False
is_avgn = False
is_anne = False
is_knytt_japan = False

def init(converter):
    name = converter.info_dict.get('name').lower()
    global is_knytt
    global is_knytt_japan
    global is_avgn
    global is_anne
    is_knytt = 'knytt' in name
    is_avgn = 'angry video game' in name
    is_anne = 'ane' in name
    is_knytt_japan = 'japan' in name

    if is_avgn:
        # hack to set default keyboard keys
        # 72 - 82
        values = converter.game.globalValues.items
        for i in xrange(71, 82):
            values[i] = convert_key(values[i])

object_checks = {
}

def write_pre(converter, writer, group):
    pass

def use_simple_or(converter):
    return is_knytt

def use_iteration_index(converter):
    return is_avgn

alterable_int_objects = [
    'MenuMainMapObject_',
    'MiniMapObject_',
    'MenuMainController'
]

def use_global_int(expression):
    if not is_avgn:
        return False
    index = expression.data.loader.value
    return index in (0, 1)

def use_alterable_int(expression):
    if not is_anne:
        return False
    object_info, object_type = expression.get_object()
    name = expression.converter.get_object_name(object_info)
    for check in alterable_int_objects:
        if name.startswith(check):
            return True
    return False

def use_global_instances(converter):
    return True

def write_defines(converter, writer):
    if is_anne:
        writer.putln('#define CHOWDREN_SNES_CONTROLLER')
        writer.putln('#define CHOWDREN_FORCE_REMOTE')
    if is_anne or is_avgn:
        writer.putln('#define CHOWDREN_QUICK_SCALE')
    if is_knytt_japan:
        writer.putln('#define CHOWDREN_TEXT_USE_UTF8')
        writer.putln('#define CHOWDREN_TEXT_JAPANESE')
        writer.putln('#define CHOWDREN_BIG_FONT_OFFY 1')
    if not is_knytt:
        writer.putln('#define CHOWDREN_USE_COLTREE')
    if is_avgn:
        writer.putln('#define CHOWDREN_STARTUP_WINDOW')
    if not is_knytt or is_knytt_japan:
        writer.putln('#define CHOWDREN_USE_DYNTREE')