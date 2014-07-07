VK_TO_SDL = {
    0x1 : ['SDL_BUTTON_LEFT'],
    0x2 : ['SDL_BUTTON_RIGHT'],
    # 0x3 : ['SDLK_BREAK'],
    0x4 : ['SDL_BUTTON_MIDDLE'],
    0x5 : ['SDL_BUTTON_X1'],
    0x6 : ['SDL_BUTTON_X2'],
    0x8 : ['SDLK_BACKSPACE'],
    0x9 : ['SDLK_TAB'],
    0xC : ['SDLK_CLEAR'],
    0xD : ['SDLK_RETURN', 'SDLK_KP_ENTER'],
    0x10 : ['SDLK_LSHIFT', 'SDLK_RSHIFT'],
    0x11 : ['SDLK_LCTRL', 'SDLK_RCTRL'],
    0x12 : ['SDLK_RALT', 'SDLK_LALT'],
    0x13 : ['SDLK_PAUSE'],
    0x14 : ['SDLK_CAPSLOCK'],
    0x1B : ['SDLK_ESCAPE'],
    0x20 : ['SDLK_SPACE'],
    0x21 : ['SDLK_PAGEUP', 'SDLK_KP_9'],
    0x22 : ['SDLK_PAGEDOWN', 'SDLK_KP_3'],
    0x23 : ['SDLK_END', 'SDLK_KP_1'],
    0x24 : ['SDLK_HOME', 'SDLK_KP_7'],
    0x25 : ['SDLK_LEFT', 'SDLK_KP_4'],
    0x26 : ['SDLK_UP', 'SDLK_KP_8'],
    0x27 : ['SDLK_RIGHT', 'SDLK_KP_6'],
    0x28 : ['SDLK_DOWN', 'SDLK_KP_2'],
    0x2D : ['SDLK_INSERT', 'SDLK_KP_0'],
    0x2E : ['SDLK_DELETE', 'SDLK_KP_DECIMAL'],
    0x30 : ['SDLK_0'], # VK_0
    0x31 : ['SDLK_1'], # VK_1
    0x32 : ['SDLK_2'], # VK_2
    0x33 : ['SDLK_3'], # VK_3
    0x34 : ['SDLK_4'], # VK_4
    0x35 : ['SDLK_5'], # VK_5
    0x36 : ['SDLK_6'], # VK_6
    0x37 : ['SDLK_7'], # VK_7
    0x38 : ['SDLK_8'], # VK_8
    0x39 : ['SDLK_9'], # VK_9
    0x41 : ['SDLK_a'], # VK_A
    0x42 : ['SDLK_b'], # VK_B
    0x43 : ['SDLK_c'], # VK_C
    0x44 : ['SDLK_d'], # VK_D
    0x45 : ['SDLK_e'], # VK_E
    0x46 : ['SDLK_f'], # VK_F
    0x47 : ['SDLK_g'], # VK_G
    0x48 : ['SDLK_h'], # VK_H
    0x49 : ['SDLK_i'], # VK_I
    0x4A : ['SDLK_j'], # VK_J
    0x4B : ['SDLK_k'], # VK_K
    0x4C : ['SDLK_l'], # VK_L
    0x4D : ['SDLK_m'], # VK_M
    0x4E : ['SDLK_n'], # VK_N
    0x4F : ['SDLK_o'], # VK_O
    0x50 : ['SDLK_p'], # VK_P
    0x51 : ['SDLK_q'], # VK_Q
    0x52 : ['SDLK_r'], # VK_R
    0x53 : ['SDLK_s'], # VK_S
    0x54 : ['SDLK_t'], # VK_T
    0x55 : ['SDLK_u'], # VK_U
    0x56 : ['SDLK_v'], # VK_V
    0x57 : ['SDLK_w'], # VK_W
    0x58 : ['SDLK_x'], # VK_X
    0x59 : ['SDLK_y'], # VK_Y
    0x5A : ['SDLK_z'], # VK_Z
    0x5B : ['SDLK_LGUI'], # VK_LWIN
    0x5C : ['SDLK_RGUI'], # VK_RWIN
    0x5D : ['SDLK_MENU'], # VK_APPS
    0x60 : ['SDLK_KP_0'], # VK_NUMPAD0
    0x61 : ['SDLK_KP_1'], # VK_NUMPAD1
    0x62 : ['SDLK_KP_2'], # VK_NUMPAD2
    0x63 : ['SDLK_KP_3'], # VK_NUMPAD3
    0x64 : ['SDLK_KP_4'], # VK_NUMPAD4
    0x65 : ['SDLK_KP_5'], # VK_NUMPAD5
    0x66 : ['SDLK_KP_6'], # VK_NUMPAD6
    0x67 : ['SDLK_KP_7'], # VK_NUMPAD7
    0x68 : ['SDLK_KP_8'], # VK_NUMPAD8
    0x69 : ['SDLK_KP_9'], # VK_NUMPAD9
    0x6A : ['SDLK_KP_MULTIPLY'], # VK_MULTIPLY
    0x6B : ['SDLK_KP_PLUS'], # VK_ADD
    0x6D : ['SDLK_KP_MINUS'], # VK_SUBTRACT
    0x6E : ['SDLK_KP_DECIMAL'], # VK_DECIMAL
    0x6F : ['SDLK_KP_DIVIDE'], # VK_DIVIDE
    0x70 : ['SDLK_F1'], # VK_F1
    0x71 : ['SDLK_F2'], # VK_F2
    0x72 : ['SDLK_F3'], # VK_F3
    0x73 : ['SDLK_F4'], # VK_F4
    0x74 : ['SDLK_F5'], # VK_F5
    0x75 : ['SDLK_F6'], # VK_F6
    0x76 : ['SDLK_F7'], # VK_F7
    0x77 : ['SDLK_F8'], # VK_F8
    0x78 : ['SDLK_F9'], # VK_F9
    0x79 : ['SDLK_F10'], # VK_F10
    0x7A : ['SDLK_F11'], # VK_F11
    0x7B : ['SDLK_F12'], # VK_F12
    0x7C : ['SDLK_F13'], # VK_F13
    0x7D : ['SDLK_F14'], # VK_F14
    0x7E : ['SDLK_F15'], # VK_F15
    0x7F : ['SDLK_F16'], # VK_F16
    0x80 : ['SDLK_F17'], # VK_F17
    0x81 : ['SDLK_F18'], # VK_F18
    0x82 : ['SDLK_F19'], # VK_F19
    0x83 : ['SDLK_F20'], # VK_F20
    0x84 : ['SDLK_F21'], # VK_F21
    0x85 : ['SDLK_F22'], # VK_F22
    0x86 : ['SDLK_F23'], # VK_F23
    0x87 : ['SDLK_F24'], # VK_F24
    0x90 : ['SDLK_NUMLOCKCLEAR'], # VK_NUMLOCK
    0x91 : ['SDLK_SCROLLLOCK'], # VK_SCROLL
    0xA0 : ['SDLK_LSHIFT'], # VK_LSHIFT
    0xA1 : ['SDLK_RSHIFT'], # VK_RSHIFT
    0xA2 : ['SDLK_LCTRL'], # VK_LCONTROL
    0xA3 : ['SDLK_RCTRL'], # VK_RCONTROL
    0xA4 : ['SDLK_LALT'], # VK_LMENU
    0xA5 : ['SDLK_RALT'], # VK_RMENU
    0xBD : ['SDLK_MINUS'],
    0xBB : ['SDLK_EQUALS'],
    0xDB : ['SDLK_LEFTBRACKET'],
    0xDD : ['SDLK_RIGHTBRACKET'],
    0xDC : ['SDLK_BACKSLASH'],
    0xBA : ['SDLK_SEMICOLON'],
    0xDE : ['SDLK_QUOTE'],
    0xC0 : ['SDLK_BACKQUOTE'],
    0xBC : ['SDLK_COMMA'],
    0xBE : ['SDLK_PERIOD'],
    0xBF : ['SDLK_SLASH'],
    0xE2 : ['SDLK_BACKSLASH']
}

VK_TO_NAME = {
    1 : 'Mouse Left',
    2 : 'Mouse Right',
    3 : 'Control-break',
    4 : 'Mouse Middle',
    5 : 'Mouse X1',
    6 : 'Mouse X2',
    8 : 'Backspace',
    9 : 'Tab',
    12 : 'Clear',
    13 : 'Return',
    16 : 'Shift',
    17 : 'Control',
    18 : 'Alt',
    19 : 'Break',
    20 : 'Capslock',
    21 : 'Hangul*',
    23 : 'Junja*',
    24 : 'Final*',
    25 : 'Hanja*',
    25 : 'Kanji*',
    28 : 'convert*',
    29 : 'nonconvert*',
    30 : 'accept*',
    31 : 'change request*',
    27 : 'Escape',
    32 : 'Space',
    33 : 'Page up',
    34 : 'Page down',
    35 : 'End',
    36 : 'Home',
    37 : 'Left',
    38 : 'Up',
    39 : 'Right',
    40 : 'Down',
    41 : 'Select',
    42 : 'Print',
    43 : 'Execute',
    44 : 'Snapshot',
    45 : 'Insert',
    46 : 'Delete',
    47 : 'Help',
    48 : '0',
    49 : '1',
    50 : '2',
    51 : '3',
    52 : '4',
    53 : '5',
    54 : '6',
    55 : '7',
    56 : '8',
    57 : '9',
    65 : 'A',
    66 : 'B',
    67 : 'C',
    68 : 'D',
    69 : 'E',
    70 : 'F',
    71 : 'G',
    72 : 'H',
    73 : 'I',
    74 : 'J',
    75 : 'K',
    76 : 'L',
    77 : 'M',
    78 : 'N',
    79 : 'O',
    80 : 'P',
    81 : 'Q',
    82 : 'R',
    83 : 'S',
    84 : 'T',
    85 : 'U',
    86 : 'V',
    87 : 'W',
    88 : 'X',
    89 : 'Y',
    90 : 'Z',
    91 : 'Left Window',
    92 : 'Right Window',
    93 : 'Menu',
    95 : 'Sleep',
    96 : '0 numeric',
    97 : '1 numeric',
    98 : '2 numeric',
    99 : '3 numeric',
    100 : '4 numeric',
    101 : '5 numeric',
    102 : '6 numeric',
    103 : '7 numeric',
    104 : '8 numeric',
    105 : '9 numeric',
    106 : '*',
    107 : '+',
    108 : 'Separator',
    109 : '-',
    110 : 'Decimal',
    111 : '/',
    112 : 'F1',
    113 : 'F2',
    114 : 'F3',
    115 : 'F4',
    116 : 'F5',
    117 : 'F6',
    118 : 'F7',
    119 : 'F8',
    120 : 'F9',
    121 : 'F10',
    122 : 'F11',
    123 : 'F12',
    124 : 'F13',
    125 : 'F14',
    126 : 'F15',
    127 : 'F16',
    128 : 'F17',
    129 : 'F18',
    130 : 'F19',
    131 : 'F20',
    132 : 'F21',
    133 : 'F22',
    134 : 'F23',
    135 : 'F24',
    144 : 'Numlock',
    145 : 'Scrollock',
    160 : 'Shift Left',
    161 : 'Shift Right',
    162 : 'Control Left',
    163 : 'Control Right',
    164 : 'Alt Left',
    165 : 'Alt Right',
}

# not used yet, but will be useful in the future
VK_TO_CTRLX = {
    -47 : 'Right menu',
    -31 : 'Left shift',
    -19 : 'Right shift',
    -15 : 'Left menu',
    -13 : 'Left control',
    -6 : 'Right control',
    8 : 'Backspace',
    9 : 'Tab',
    13 : 'Return',
    16 : 'Shift',
    17 : 'Control',
    18 : 'none',
    19 : 'Break',
    20 : 'Capslock',
    27 : 'Escape',
    32 : 'Space',
    33 : 'Page up',
    34 : 'Page down',
    35 : 'End',
    36 : 'Home',
    37 : 'Left',
    38 : 'Up',
    39 : 'Right',
    40 : 'Down',
    41 : 'Select',
    42 : 'Print',
    43 : 'Execute',
    44 : 'Snapshot',
    45 : 'Insert',
    46 : 'Delete',
    47 : 'Help',
    48 : '0',
    49 : '1',
    50 : '2',
    51 : '3',
    52 : '4',
    53 : '5',
    54 : '6',
    55 : '7',
    56 : '8',
    57 : '9',
    65 : 'A',
    66 : 'B',
    67 : 'C',
    68 : 'D',
    69 : 'E',
    70 : 'F',
    71 : 'G',
    72 : 'H',
    73 : 'I',
    74 : 'J',
    75 : 'K',
    76 : 'L',
    77 : 'M',
    78 : 'N',
    79 : 'O',
    80 : 'P',
    81 : 'Q',
    82 : 'R',
    83 : 'S',
    84 : 'T',
    85 : 'U',
    86 : 'V',
    87 : 'W',
    88 : 'X',
    89 : 'Y',
    90 : 'Z',
    91 : 'Left Window',
    92 : 'Right Window',
    93 : 'Menu',
    96 : '0 on numeric',
    97 : '1 on numeric',
    98 : '2 on numeric',
    99 : '3 on numeric',
    100 : '4 on numeric',
    101 : '5 on numeric',
    102 : '6 on numeric',
    103 : '7 on numeric',
    104 : '8 on numeric',
    105 : '9 on numeric',
    106 : '*',
    108 : 'Separator',
    110 : 'Decimal',
    111 : '/',
    112 : 'F1',
    113 : 'F2',
    114 : 'F3',
    115 : 'F4',
    116 : 'F5',
    117 : 'F6',
    118 : 'F7',
    119 : 'F8',
    120 : 'F9',
    121 : 'F10',
    122 : 'F11',
    123 : 'F12',
    144 : 'Numlock',
    145 : 'Scrollock',
    186 : '\xbf',
    187 : '+',
    188 : ',',
    189 : '-',
    190 : '.',
    191 : "'",
    219 : '\xa6',
    220 : '\xba',
    226 : '<'
}

KEY_TO_NAME = {}
for vk, names in VK_TO_SDL.iteritems():
    string_name = VK_TO_NAME.get(vk, None)
    if string_name is None:
        continue
    for name in names:
        KEY_TO_NAME[name] = string_name

def convert_key(value):
    return VK_TO_SDL[value][0]

def main():
    # generate key definition files
    from chowdren.code import CodeWriter
    from chowdren.common import get_base_path
    import os

    writer = CodeWriter(os.path.join(get_base_path(), 'keyconv.cpp'))

    # write keys file
    writer.putln('#include <string>')
    writer.putln('#include "keydef.h"')
    writer.putln('#include "stringcommon.h"')
    writer.putln('')

    writer.putmeth('int translate_vk_to_key', 'int vk')
    writer.putln('switch (vk) {')
    writer.indent()
    for vk, name in VK_TO_SDL.iteritems():
        writer.putln('case %s: return %s;' % (vk, name[0]))
    writer.end_brace()
    writer.putln('return -1;')
    writer.end_brace()
    writer.putln('')

    writer.putmeth('int translate_string_to_key',
                   'const std::string & in_name')
    writer.putln('std::string name = in_name;')
    writer.putln('to_lower(name);')
    for vk, name in VK_TO_SDL.iteritems():
        string_name = VK_TO_NAME.get(vk, None)
        if string_name is None:
            continue
        string_name = string_name.lower()
        writer.putlnc('if (name.compare(0, %s, %r) == 0) return %s;',
                      len(string_name), string_name, name[0], cpp=False)
    writer.putln('return -1;')
    writer.end_brace()
    writer.putln('')

    writer.putmeth('std::string translate_vk_to_string',
                   'int vk')
    writer.putln('switch (vk) {')
    writer.indent()
    for vk, name in VK_TO_SDL.iteritems():
        string_name = VK_TO_NAME.get(vk, None)
        if string_name is None:
            continue
        writer.putlnc('case %s: return %r;', vk, string_name)
    writer.end_brace()
    writer.putln('return "";')
    writer.end_brace()
    writer.putln('')

    writer.putmeth('std::string translate_key_to_string',
                   'int key')
    writer.putln('switch (key) {')
    writer.indent()
    for name, string_name in KEY_TO_NAME.iteritems():
        writer.putlnc('case %s: return %r;', name, string_name)
    writer.end_brace()
    writer.putln('return "";')
    writer.end_brace()

    writer.close()

if __name__ == '__main__':
    main()