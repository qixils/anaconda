def init(converter):
    name = converter.info_dict.get('name').lower()

    if 'japan' in name:
        converter.add_define('CHOWDREN_TEXT_USE_UTF8')
        converter.add_define('CHOWDREN_TEXT_JAPANESE')
        converter.add_define('CHOWDREN_BIG_FONT_OFFY 1')

def use_simple_or(converter):
    return True

def use_iteration_index(converter):
    return False

def use_safe_division(converter):
    return False
