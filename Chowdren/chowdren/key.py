VK_TO_GLFW = {
    0x1 : ['GLFW_MOUSE_BUTTON_LEFT'],
    0x2 : ['GLFW_MOUSE_BUTTON_RIGHT'],
    0x4 : ['GLFW_MOUSE_BUTTON_MIDDLE'],
    0x8 : ['GLFW_KEY_BACKSPACE'], # VK_BACK
    0x9 : ['GLFW_KEY_TAB'], # VK_TAB
    0xC : ['GLFW_KEY_KP_5'], # VK_CLEAR
    0xD : ['GLFW_KEY_ENTER', 'GLFW_KEY_KP_ENTER'], # VK_RETURN
    0x10 : ['GLFW_KEY_LSHIFT', 'GLFW_KEY_RSHIFT'], # VK_SHIFT
    0x11 : ['GLFW_KEY_LCONTROL', 'GLFW_KEY_RCONTROL'], # VK_CONTROL
    0x12 : ['GLFW_KEY_RALT', 'GLFW_KEY_LALT'], # VK_MENU
    0x13 : ['GLFW_KEY_PAUSE'], # VK_PAUSE
    0x14 : ['GLFW_KEY_CAPS_LOCK'], # VK_CAPITAL
    0x1B : ['GLFW_KEY_ESC'], # VK_ESCAPE
    0x20 : ['GLFW_KEY_SPACE'], # VK_SPACE
    0x21 : ['GLFW_KEY_PAGEUP', 'GLFW_KEY_KP_9'], # VK_PRIOR
    0x22 : ['GLFW_KEY_PAGEDOWN', 'GLFW_KEY_KP_3'], # VK_NEXT
    0x23 : ['GLFW_KEY_END', 'GLFW_KEY_KP_1'], # VK_END
    0x24 : ['GLFW_KEY_HOME', 'GLFW_KEY_KP_7'], # VK_HOME
    0x25 : ['GLFW_KEY_LEFT', 'GLFW_KEY_KP_4'], # VK_LEFT
    0x26 : ['GLFW_KEY_UP', 'GLFW_KEY_KP_4'], # VK_UP
    0x27 : ['GLFW_KEY_RIGHT', 'GLFW_KEY_KP_8'], # VK_RIGHT
    0x28 : ['GLFW_KEY_DOWN', 'GLFW_KEY_KP_2'], # VK_DOWN
    0x2D : ['GLFW_KEY_INSERT', 'GLFW_KEY_KP_0'], # VK_INSERT
    0x2E : ['GLFW_KEY_DEL', 'GLFW_KEY_KP_DECIMAL'], # VK_DELETE
    0x30 : ["'0'"], # VK_0
    0x31 : ["'1'"], # VK_1
    0x32 : ["'2'"], # VK_2
    0x33 : ["'3'"], # VK_3
    0x34 : ["'4'"], # VK_4
    0x35 : ["'5'"], # VK_5
    0x36 : ["'6'"], # VK_6
    0x37 : ["'7'"], # VK_7
    0x38 : ["'8'"], # VK_8
    0x39 : ["'9'"], # VK_9
    0x41 : ["'A'"], # VK_A
    0x42 : ["'B'"], # VK_B
    0x43 : ["'C'"], # VK_C
    0x44 : ["'D'"], # VK_D
    0x45 : ["'E'"], # VK_E
    0x46 : ["'F'"], # VK_F
    0x47 : ["'G'"], # VK_G
    0x48 : ["'H'"], # VK_H
    0x49 : ["'I'"], # VK_I
    0x4A : ["'J'"], # VK_J
    0x4B : ["'K'"], # VK_K
    0x4C : ["'L'"], # VK_L
    0x4D : ["'M'"], # VK_M
    0x4E : ["'N'"], # VK_N
    0x4F : ["'O'"], # VK_O
    0x50 : ["'P'"], # VK_P
    0x51 : ["'Q'"], # VK_Q
    0x52 : ["'R'"], # VK_R
    0x53 : ["'S'"], # VK_S
    0x54 : ["'T'"], # VK_T
    0x55 : ["'U'"], # VK_U
    0x56 : ["'V'"], # VK_V
    0x57 : ["'W'"], # VK_W
    0x58 : ["'X'"], # VK_X
    0x59 : ["'Y'"], # VK_Y
    0x5A : ["'Z'"], # VK_Z
    0x5B : ['GLFW_KEY_LSUPER'], # VK_LWIN
    0x5C : ['GLFW_KEY_RSUPER'], # VK_RWIN
    0x5D : ['GLFW_KEY_MENU'], # VK_APPS
    0x60 : ["'0'"], # VK_NUMPAD0
    0x61 : ["'1'"], # VK_NUMPAD1
    0x62 : ["'2'"], # VK_NUMPAD2
    0x63 : ["'3'"], # VK_NUMPAD3
    0x64 : ["'4'"], # VK_NUMPAD4
    0x65 : ["'5'"], # VK_NUMPAD5
    0x66 : ["'6'"], # VK_NUMPAD6
    0x67 : ["'7'"], # VK_NUMPAD7
    0x68 : ["'8'"], # VK_NUMPAD8
    0x69 : ["'9'"], # VK_NUMPAD9
    0x6A : ['GLFW_KEY_KP_MULTIPLY'], # VK_MULTIPLY
    0x6B : ['GLFW_KEY_KP_ADD'], # VK_ADD
    0x6D : ['GLFW_KEY_KP_SUBTRACT'], # VK_SUBTRACT
    0x6E : ['GLFW_KEY_KP_DECIMAL'], # VK_DECIMAL
    0x6F : ['GLFW_KEY_KP_DIVIDE'], # VK_DIVIDE
    0x70 : ['GLFW_KEY_F1'], # VK_F1
    0x71 : ['GLFW_KEY_F2'], # VK_F2
    0x72 : ['GLFW_KEY_F3'], # VK_F3
    0x73 : ['GLFW_KEY_F4'], # VK_F4
    0x74 : ['GLFW_KEY_F5'], # VK_F5
    0x75 : ['GLFW_KEY_F6'], # VK_F6
    0x76 : ['GLFW_KEY_F7'], # VK_F7
    0x77 : ['GLFW_KEY_F8'], # VK_F8
    0x78 : ['GLFW_KEY_F9'], # VK_F9
    0x79 : ['GLFW_KEY_F10'], # VK_F10
    0x7A : ['GLFW_KEY_F11'], # VK_F11
    0x7B : ['GLFW_KEY_F12'], # VK_F12
    0x7C : ['GLFW_KEY_F13'], # VK_F13
    0x7D : ['GLFW_KEY_F14'], # VK_F14
    0x7E : ['GLFW_KEY_F15'], # VK_F15
    0x7F : ['GLFW_KEY_F16'], # VK_F16
    0x80 : ['GLFW_KEY_F17'], # VK_F17
    0x81 : ['GLFW_KEY_F18'], # VK_F18
    0x82 : ['GLFW_KEY_F19'], # VK_F19
    0x83 : ['GLFW_KEY_F20'], # VK_F20
    0x84 : ['GLFW_KEY_F21'], # VK_F21
    0x85 : ['GLFW_KEY_F22'], # VK_F22
    0x86 : ['GLFW_KEY_F23'], # VK_F23
    0x87 : ['GLFW_KEY_F24'], # VK_F24
    0x90 : ['GLFW_KEY_KP_NUM_LOCK'], # VK_NUMLOCK
    0x91 : ['GLFW_KEY_SCROLL_LOCK'], # VK_SCROLL
    0xA0 : ['GLFW_KEY_LSHIFT'], # VK_LSHIFT
    0xA1 : ['GLFW_KEY_RSHIFT'], # VK_RSHIFT
    0xA2 : ['GLFW_KEY_LCONTROL'], # VK_LCONTROL
    0xA3 : ['GLFW_KEY_RCONTROL'], # VK_RCONTROL
    0xA4 : ['GLFW_KEY_LALT'], # VK_LMENU
    0xA5 : ['GLFW_KEY_RALT'] # VK_RMENU
}