import string

COMPARISONS = [
    '==',
    '!=',
    '<=',
    '<',
    '>=',
    '>'
]

VALID_CHARACTERS = string.ascii_letters + string.digits
DIGITS = string.digits

def get_method_name(value, check_digits = False):
    new_name = ''
    add_underscore = False
    for c in value:
        if c.isupper():
            c = c.lower()
            add_underscore = True
        if c in VALID_CHARACTERS:
            if add_underscore:
                if new_name:
                    new_name += '_'
                add_underscore = False
            new_name += c
        else:
            add_underscore = True
    if check_digits:
        new_name = check_digits(new_name, 'meth_')
    return new_name

def get_class_name(value):
    new_name = ''
    go_upper = True
    for c in value:
        if c in VALID_CHARACTERS:
            if go_upper:
                c = c.upper()
                go_upper = False
            new_name += c
        else:
            go_upper = True
    return check_digits(new_name, 'Obj')

def check_digits(value, prefix):
    if not value:
        return value
    if value[0] in DIGITS:
        value = prefix + value
    return value

class StringWrapper(object):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return self.value

    def __repr__(self):
        return '"%s"' % self.value.replace('"', '')

def to_c(format_spec, *args):
    new_args = []
    for arg in args:
        if isinstance(arg, str):
            arg = StringWrapper(arg)
        elif isinstance(arg, bool):
            if arg:
                arg = 'true'
            else:
                arg = 'false'
        new_args.append(arg)
    return format_spec % tuple(new_args)

def get_image_name(value):
    return '&image%s' % value

def make_color(value):
    return 'Color(%s)' % ', '.join([str(item) for item in value])

ANIMATION_NAMES = [
    'STOPPED',
    'WALKING',
    'RUNNING',
    'APPEARING',
    'DISAPPEARING',
    'BOUNCING',
    'SHOOTING',
    'JUMPING',
    'FALLING',
    'CLIMBING',
    'CROUCH',
    'STAND',
    'USER_DEFINED_1',
    'USER_DEFINED_2',
    'USER_DEFINED_3',
    'USER_DEFINED_4'
]

def get_animation_name(index):
    return ANIMATION_NAMES[index]