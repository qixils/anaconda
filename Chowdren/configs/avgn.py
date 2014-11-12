from chowdren.key import convert_key

def init(converter):
    # hack to set default keyboard keys
    # 72 - 82
    values = converter.game.globalValues.items
    for i in xrange(71, 82):
        values[i] = convert_key(values[i])

    converter.add_define('CHOWDREN_FORCE_REMOTE')
    converter.add_define('CHOWDREN_QUICK_SCALE')
    converter.add_define('CHOWDREN_STARTUP_WINDOW')
    converter.add_define('CHOWDREN_POINT_FILTER')
    converter.add_define('CHOWDREN_PERSISTENT_FIXED_STRING')
    converter.add_define('CHOWDREN_LAYER_WRAP')
    converter.add_define('CHOWDREN_RESTORE_ANIMATIONS')
    converter.add_define('CHOWDREN_WIIU_USE_COMMON')
    converter.add_define('CHOWDREN_SCREEN2_WIDTH', 240)
    converter.add_define('CHOWDREN_SCREEN2_HEIGHT', 180)


alterable_int_objects = [
    'FireShark',
    'Cog',
]

def use_global_int(converter, expression):
    index = expression.data.loader.value
    return index in (0, 1)

def use_alterable_int(converter, expression):
    obj = expression.get_object()
    name = expression.converter.get_object_name(obj)
    for check in alterable_int_objects:
        if name.startswith(check):
            return True
    return False

def use_safe_division(converter):
    return False

def use_safe_create(converter):
    return True

def use_image_flush(converter, frame):
    # for 3DS primarily (maybe Vita as well)
    return frame.name in ('Level Select',)

def use_image_preload(converter):
    return converter.platform_name == '3ds'

def get_depth(converter, layer):
    if converter.platform_name != '3ds':
        return None
    if layer.name in ('HUD', 'Untitled'):
        return 0.0
    coeff = layer.xCoefficient
    if coeff == 0.0:
        return 1.0
    depth = 1.0 - coeff
    depth = 0.15 + depth * (1.0 - 0.15)
    return max(0.0, min(1.0, depth))