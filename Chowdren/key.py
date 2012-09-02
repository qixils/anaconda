table = [
                  # Dec |  Hex | Windows Virtual key
    'unknown',    #   0   0x00
    'unknown',    #   1   0x01   VK_LBUTTON          | Left mouse button
    'unknown',    #   2   0x02   VK_RBUTTON          | Right mouse button
    'Cancel',     #   3   0x03   VK_CANCEL           | Control-Break processing
    'unknown',    #   4   0x04   VK_MBUTTON          | Middle mouse button
    'unknown',    #   5   0x05   VK_XBUTTON1         | X1 mouse button
    'unknown',    #   6   0x06   VK_XBUTTON2         | X2 mouse button
    'unknown',    #   7   0x07   -- unassigned --
    'Backspace',  #   8   0x08   VK_BACK             | BackSpace key
    'Tab',        #   9   0x09   VK_TAB              | Tab key
    'unknown',    #  10   0x0A   -- reserved --
    'unknown',    #  11   0x0B   -- reserved --
    'Clear',      #  12   0x0C   VK_CLEAR            | Clear key
    'Return',     #  13   0x0D   VK_RETURN           | Enter key
    'unknown',    #  14   0x0E   -- unassigned --
    'unknown',    #  15   0x0F   -- unassigned --
    'Shift',      #  16   0x10   VK_SHIFT            | Shift key
    'Control',    #  17   0x11   VK_CONTROL          | Ctrl key
    'Alt',        #  18   0x12   VK_MENU             | Alt key
    'Pause',      #  19   0x13   VK_PAUSE            | Pause key
    'CapsLock',   #  20   0x14   VK_CAPITAL          | Caps-Lock
    'unknown',    #  21   0x15   VK_KANA / VK_HANGUL | IME Kana or Hangul mode
    'unknown',    #  22   0x16   -- unassigned --
    'unknown',    #  23   0x17   VK_JUNJA            | IME Junja mode
    'unknown',    #  24   0x18   VK_FINAL            | IME final mode
    'unknown',    #  25   0x19   VK_HANJA / VK_KANJI | IME Hanja or Kanji mode
    'unknown',    #  26   0x1A   -- unassigned --
    'Escape',     #  27   0x1B   VK_ESCAPE           | Esc key
    'unknown',    #  28   0x1C   VK_CONVERT          | IME convert
    'unknown',    #  29   0x1D   VK_NONCONVERT       | IME non-convert
    'unknown',    #  30   0x1E   VK_ACCEPT           | IME accept
    'Mode_switch',#  31   0x1F   VK_MODECHANGE       | IME mode change request
    'Space',      #  32   0x20   VK_SPACE            | Spacebar
    'PageUp',     #  33   0x21   VK_PRIOR            | Page Up key
    'PageDown',   #  34   0x22   VK_NEXT             | Page Down key
    'End',        #  35   0x23   VK_END              | End key
    'Home',       #  36   0x24   VK_HOME             | Home key
    'Left',       #  37   0x25   VK_LEFT             | Left arrow key
    'Up',         #  38   0x26   VK_UP               | Up arrow key
    'Right',      #  39   0x27   VK_RIGHT            | Right arrow key
    'Down',       #  40   0x28   VK_DOWN             | Down arrow key
    'Select',     #  41   0x29   VK_SELECT           | Select key
    'Printer',    #  42   0x2A   VK_PRINT            | Print key
    'Execute',    #  43   0x2B   VK_EXECUTE          | Execute key
    'Print',      #  44   0x2C   VK_SNAPSHOT         | Print Screen key
    'Insert',     #  45   0x2D   VK_INSERT           | Ins key
    'Delete',     #  46   0x2E   VK_DELETE           | Del key
    'Help',       #  47   0x2F   VK_HELP             | Help key
    '0',          #  48   0x30   (VK_0)              | 0 key
    '1',          #  49   0x31   (VK_1)              | 1 key
    '2',          #  50   0x32   (VK_2)              | 2 key
    '3',          #  51   0x33   (VK_3)              | 3 key
    '4',          #  52   0x34   (VK_4)              | 4 key
    '5',          #  53   0x35   (VK_5)              | 5 key
    '6',          #  54   0x36   (VK_6)              | 6 key
    '7',          #  55   0x37   (VK_7)              | 7 key
    '8',          #  56   0x38   (VK_8)              | 8 key
    '9',          #  57   0x39   (VK_9)              | 9 key
    'unknown',    #  58   0x3A   -- unassigned --
    'unknown',    #  59   0x3B   -- unassigned --
    'unknown',    #  60   0x3C   -- unassigned --
    'unknown',    #  61   0x3D   -- unassigned --
    'unknown',    #  62   0x3E   -- unassigned --
    'unknown',    #  63   0x3F   -- unassigned --
    'unknown',    #  64   0x40   -- unassigned --
    'A',          #  65   0x41   (VK_A)              | A key
    'B',          #  66   0x42   (VK_B)              | B key
    'C',          #  67   0x43   (VK_C)              | C key
    'D',          #  68   0x44   (VK_D)              | D key
    'E',          #  69   0x45   (VK_E)              | E key
    'F',          #  70   0x46   (VK_F)              | F key
    'G',          #  71   0x47   (VK_G)              | G key
    'H',          #  72   0x48   (VK_H)              | H key
    'I',          #  73   0x49   (VK_I)              | I key
    'J',          #  74   0x4A   (VK_J)              | J key
    'K',          #  75   0x4B   (VK_K)              | K key
    'L',          #  76   0x4C   (VK_L)              | L key
    'M',          #  77   0x4D   (VK_M)              | M key
    'N',          #  78   0x4E   (VK_N)              | N key
    'O',          #  79   0x4F   (VK_O)              | O key
    'P',          #  80   0x50   (VK_P)              | P key
    'Q',          #  81   0x51   (VK_Q)              | Q key
    'R',          #  82   0x52   (VK_R)              | R key
    'S',          #  83   0x53   (VK_S)              | S key
    'T',          #  84   0x54   (VK_T)              | T key
    'U',          #  85   0x55   (VK_U)              | U key
    'V',          #  86   0x56   (VK_V)              | V key
    'W',          #  87   0x57   (VK_W)              | W key
    'X',          #  88   0x58   (VK_X)              | X key
    'Y',          #  89   0x59   (VK_Y)              | Y key
    'Z',          #  90   0x5A   (VK_Z)              | Z key
    'Meta',       #  91   0x5B   VK_LWIN             | Left Windows  - MS Natural kbd
    'Meta',       #  92   0x5C   VK_RWIN             | Right Windows - MS Natural kbd
    'Menu',       #  93   0x5D   VK_APPS             | Application key-MS Natural kbd
    'unknown',    #  94   0x5E   -- reserved --
    'Sleep',      #  95   0x5F   VK_SLEEP
    '0',          #  96   0x60   VK_NUMPAD0          | Numeric keypad 0 key
    '1',          #  97   0x61   VK_NUMPAD1          | Numeric keypad 1 key
    '2',          #  98   0x62   VK_NUMPAD2          | Numeric keypad 2 key
    '3',          #  99   0x63   VK_NUMPAD3          | Numeric keypad 3 key
    '4',          # 100   0x64   VK_NUMPAD4          | Numeric keypad 4 key
    '5',          # 101   0x65   VK_NUMPAD5          | Numeric keypad 5 key
    '6',          # 102   0x66   VK_NUMPAD6          | Numeric keypad 6 key
    '7',          # 103   0x67   VK_NUMPAD7          | Numeric keypad 7 key
    '8',          # 104   0x68   VK_NUMPAD8          | Numeric keypad 8 key
    '9',          # 105   0x69   VK_NUMPAD9          | Numeric keypad 9 key
    'Asterisk',   # 106   0x6A   VK_MULTIPLY         | Multiply key
    'Plus',       # 107   0x6B   VK_ADD              | Add key
    'Comma',      # 108   0x6C   VK_SEPARATOR        | Separator key
    'Minus',      # 109   0x6D   VK_SUBTRACT         | Subtract key
    'Period',     # 110   0x6E   VK_DECIMAL          | Decimal key
    'Slash',      # 111   0x6F   VK_DIVIDE           | Divide key
    'F1',         # 112   0x70   VK_F1               | F1 key
    'F2',         # 113   0x71   VK_F2               | F2 key
    'F3',         # 114   0x72   VK_F3               | F3 key
    'F4',         # 115   0x73   VK_F4               | F4 key
    'F5',         # 116   0x74   VK_F5               | F5 key
    'F6',         # 117   0x75   VK_F6               | F6 key
    'F7',         # 118   0x76   VK_F7               | F7 key
    'F8',         # 119   0x77   VK_F8               | F8 key
    'F9',         # 120   0x78   VK_F9               | F9 key
    'F10',        # 121   0x79   VK_F10              | F10 key
    'F11',        # 122   0x7A   VK_F11              | F11 key
    'F12',        # 123   0x7B   VK_F12              | F12 key
    'F13',        # 124   0x7C   VK_F13              | F13 key
    'F14',        # 125   0x7D   VK_F14              | F14 key
    'F15',        # 126   0x7E   VK_F15              | F15 key
    'F16',        # 127   0x7F   VK_F16              | F16 key
    'F17',        # 128   0x80   VK_F17              | F17 key
    'F18',        # 129   0x81   VK_F18              | F18 key
    'F19',        # 130   0x82   VK_F19              | F19 key
    'F20',        # 131   0x83   VK_F20              | F20 key
    'F21',        # 132   0x84   VK_F21              | F21 key
    'F22',        # 133   0x85   VK_F22              | F22 key
    'F23',        # 134   0x86   VK_F23              | F23 key
    'F24',        # 135   0x87   VK_F24              | F24 key
    'unknown',    # 136   0x88   -- unassigned --
    'unknown',    # 137   0x89   -- unassigned --
    'unknown',    # 138   0x8A   -- unassigned --
    'unknown',    # 139   0x8B   -- unassigned --
    'unknown',    # 140   0x8C   -- unassigned --
    'unknown',    # 141   0x8D   -- unassigned --
    'unknown',    # 142   0x8E   -- unassigned --
    'unknown',    # 143   0x8F   -- unassigned --
    'NumLock',    # 144   0x90   VK_NUMLOCK          | Num Lock key
    'ScrollLock', # 145   0x91   VK_SCROLL           | Scroll Lock key
                  # Fujitsu/OASYS kbd --------------------
    None, # Jisho # 146   0x92   VK_OEM_FJ_JISHO     | 'Dictionary' key /
                  #              VK_OEM_NEC_EQUAL  = key on numpad on NEC PC-9800 kbd
    'Massyo',     # 147   0x93   VK_OEM_FJ_MASSHOU   | 'Unregister word' key
    'Touroku',    # 148   0x94   VK_OEM_FJ_TOUROKU   | 'Register word' key
    None, # Oyayubi_Left,#149   0x95  VK_OEM_FJ_LOYA  | 'Left OYAYUBI' key
    None, # Oyayubi_Right,#150  0x96  VK_OEM_FJ_ROYA  | 'Right OYAYUBI' key
    'unknown',    # 151   0x97   -- unassigned --
    'unknown',    # 152   0x98   -- unassigned --
    'unknown',    # 153   0x99   -- unassigned --
    'unknown',    # 154   0x9A   -- unassigned --
    'unknown',    # 155   0x9B   -- unassigned --
    'unknown',    # 156   0x9C   -- unassigned --
    'unknown',    # 157   0x9D   -- unassigned --
    'unknown',    # 158   0x9E   -- unassigned --
    'unknown',    # 159   0x9F   -- unassigned --
    'Shift',      # 160   0xA0   VK_LSHIFT           | Left Shift key
    'Shift',      # 161   0xA1   VK_RSHIFT           | Right Shift key
    'Control',    # 162   0xA2   VK_LCONTROL         | Left Ctrl key
    'Control',    # 163   0xA3   VK_RCONTROL         | Right Ctrl key
    'Alt',        # 164   0xA4   VK_LMENU            | Left Menu key
    'Alt',        # 165   0xA5   VK_RMENU            | Right Menu key
    'Back',       # 166   0xA6   VK_BROWSER_BACK     | Browser Back key
    'Forward',    # 167   0xA7   VK_BROWSER_FORWARD  | Browser Forward key
    'Refresh',    # 168   0xA8   VK_BROWSER_REFRESH  | Browser Refresh key
    'Stop',       # 169   0xA9   VK_BROWSER_STOP     | Browser Stop key
    'Search',     # 170   0xAA   VK_BROWSER_SEARCH   | Browser Search key
    'Favorites',  # 171   0xAB   VK_BROWSER_FAVORITES| Browser Favorites key
    'HomePage',   # 172   0xAC   VK_BROWSER_HOME     | Browser Start and Home key
    'VolumeMute', # 173   0xAD   VK_VOLUME_MUTE      | Volume Mute key
    'VolumeDown', # 174   0xAE   VK_VOLUME_DOWN      | Volume Down key
    'VolumeUp',   # 175   0xAF   VK_VOLUME_UP        | Volume Up key
    'MediaNext',  # 176   0xB0   VK_MEDIA_NEXT_TRACK | Next Track key
    'MediaPrevious',#177 0xB1   VK_MEDIA_PREV_TRACK | Previous Track key
    'MediaStop',  # 178   0xB2   VK_MEDIA_STOP       | Stop Media key
    'MediaPlay',  # 179   0xB3   VK_MEDIA_PLAY_PAUSE | Play/Pause Media key
    'LaunchMail', # 180   0xB4   VK_LAUNCH_MAIL      | Start Mail key
    'LaunchMedia',# 181   0xB5   VK_LAUNCH_MEDIA_SELECT Select Media key
    'Launch0',    # 182   0xB6   VK_LAUNCH_APP1      | Start Application 1 key
    'Launch1',    # 183   0xB7   VK_LAUNCH_APP2      | Start Application 2 key
    'unknown',    # 184   0xB8   -- reserved --
    'unknown',    # 185   0xB9   -- reserved --
    None,             # 186   0xBA   VK_OEM_1            | ';:' for US
    None,             # 187   0xBB   VK_OEM_PLUS         | '+' any country
    None,             # 188   0xBC   VK_OEM_COMMA        | '',' any country
    None,             # 189   0xBD   VK_OEM_MINUS        | '-' any country
    None,             # 190   0xBE   VK_OEM_PERIOD       | '.' any country
    None,             # 191   0xBF   VK_OEM_2            | '/?' for US
    None,             # 192   0xC0   VK_OEM_3            | '`~' for US
    'unknown',    # 193   0xC1   -- reserved --
    'unknown',    # 194   0xC2   -- reserved --
    'unknown',    # 195   0xC3   -- reserved --
    'unknown',    # 196   0xC4   -- reserved --
    'unknown',    # 197   0xC5   -- reserved --
    'unknown',    # 198   0xC6   -- reserved --
    'unknown',    # 199   0xC7   -- reserved --
    'unknown',    # 200   0xC8   -- reserved --
    'unknown',    # 201   0xC9   -- reserved --
    'unknown',    # 202   0xCA   -- reserved --
    'unknown',    # 203   0xCB   -- reserved --
    'unknown',    # 204   0xCC   -- reserved --
    'unknown',    # 205   0xCD   -- reserved --
    'unknown',    # 206   0xCE   -- reserved --
    'unknown',    # 207   0xCF   -- reserved --
    'unknown',    # 208   0xD0   -- reserved --
    'unknown',    # 209   0xD1   -- reserved --
    'unknown',    # 210   0xD2   -- reserved --
    'unknown',    # 211   0xD3   -- reserved --
    'unknown',    # 212   0xD4   -- reserved --
    'unknown',    # 213   0xD5   -- reserved --
    'unknown',    # 214   0xD6   -- reserved --
    'unknown',    # 215   0xD7   -- reserved --
    'unknown',    # 216   0xD8   -- unassigned --
    'unknown',    # 217   0xD9   -- unassigned --
    'unknown',    # 218   0xDA   -- unassigned --
    None,             # 219   0xDB   VK_OEM_4            | '[{' for US
    None,             # 220   0xDC   VK_OEM_5            | '\|' for US
    None,             # 221   0xDD   VK_OEM_6            | ']}' for US
    None,             # 222   0xDE   VK_OEM_7            | ''"' for US
    None,             # 223   0xDF   VK_OEM_8
    'unknown',    # 224   0xE0   -- reserved --
    'unknown',    # 225   0xE1   VK_OEM_AX           | 'AX' key on Japanese AX kbd
    'unknown',    # 226   0xE2   VK_OEM_102          | "<>" or "\|" on RT 102-key kbd
    'unknown',    # 227   0xE3   VK_ICO_HELP         | Help key on ICO
    'unknown',    # 228   0xE4   VK_ICO_00           | 00 key on ICO
    'unknown',    # 229   0xE5   VK_PROCESSKEY       | IME Process key
    'unknown',    # 230   0xE6   VK_ICO_CLEAR        |
    'unknown',    # 231   0xE7   VK_PACKET           | Unicode char as keystrokes
    'unknown',    # 232   0xE8   -- unassigned --
                      # Nokia/Ericsson definitions ---------------
    'unknown',    # 233   0xE9   VK_OEM_RESET
    'unknown',    # 234   0xEA   VK_OEM_JUMP
    'unknown',    # 235   0xEB   VK_OEM_PA1
    'unknown',    # 236   0xEC   VK_OEM_PA2
    'unknown',    # 237   0xED   VK_OEM_PA3
    'unknown',    # 238   0xEE   VK_OEM_WSCTRL
    'unknown',    # 239   0xEF   VK_OEM_CUSEL
    'unknown',    # 240   0xF0   VK_OEM_ATTN
    'unknown',    # 241   0xF1   VK_OEM_FINISH
    'unknown',    # 242   0xF2   VK_OEM_COPY
    'unknown',    # 243   0xF3   VK_OEM_AUTO
    'unknown',    # 244   0xF4   VK_OEM_ENLW
    'unknown',    # 245   0xF5   VK_OEM_BACKTAB
    'unknown',    # 246   0xF6   VK_ATTN             | Attn key
    'unknown',    # 247   0xF7   VK_CRSEL            | CrSel key
    'unknown',    # 248   0xF8   VK_EXSEL            | ExSel key
    'unknown',    # 249   0xF9   VK_EREOF            | Erase EOF key
    'Play',       # 250   0xFA   VK_PLAY             | Play key
    'Zoom',       # 251   0xFB   VK_ZOOM             | Zoom key
    'unknown',    # 252   0xFC   VK_NONAME           | Reserved
    'unknown',    # 253   0xFD   VK_PA1              | PA1 key
    'Clear',      # 254   0xFE   VK_OEM_CLEAR        | Clear key
    None
]

def key_to_qt(value):
    return table[value]