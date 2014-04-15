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
    # (4, 326) : 'BulletShock_67',
    # (4, 2556) : 'SAVEInventoryANNEPieces76100UNUSED_459',
    # (3, 186) : 'BulletShock_67',
    # (3, 2215) : 'SAVEInventoryANNEPieces76100UNUSED_459'
}

def write_pre(converter, writer, group):
    if not is_anne:
        return
    obj = object_checks.get((converter.current_frame_index, group.global_id),
                            None)
    if obj is None:
        return
    writer.putlnc('if (get_instances(%s_type).empty()) %s',
                  obj, converter.event_break)

def use_simple_or(converter):
    return is_knytt

def use_iteration_index(converter):
    return is_avgn or is_anne

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

def write_defines(converter, writer):
    if is_anne:
        writer.putln('#define CHOWDREN_SNES_CONTROLLER')
    if is_anne or is_avgn:
        writer.putln('#define CHOWDREN_QUICK_SCALE')
    if is_knytt_japan:
        writer.putln('#define CHOWDREN_TEXT_USE_UTF8')
        writer.putln('#define CHOWDREN_BIG_FONT_SIZE 23')
        writer.putln('#define CHOWDREN_BIG_FONT_OFFY 1')
    if not is_knytt:
        writer.putln('#define CHOWDREN_USE_COLTREE')
    if is_avgn:
        writer.putln('#define CHOWDREN_STARTUP_WINDOW')
