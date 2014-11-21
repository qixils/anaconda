def init(converter):
    converter.add_define('CHOWDREN_IS_FP')
    converter.add_define('CHOWDREN_RESTORE_ANIMATIONS')
    converter.add_define('CHOWDREN_QUICK_SCALE')

def get_frames(converter, game, frames):
    new_frames = {}
    indexes = (0, 1, 2, 3, 20, 76, 82)
    for index in indexes:
        new_frames[index] = frames[index]
    return new_frames

LOOP_NAMES = (
    'Player01DetectSensorMain',
    'Player01DetectSensorTop',
    'Player01DetectSensorBottom',
    'Player01DetectSensorLeft',
    'Player01DetectSensorRight',
    'Player01DetectSensorSlope',
    'Player01DetectSensorEdges',
    'Player01DetectSensorAngles'
)

def get_loop_name(converter, parameter):
    alt = parameter.loader.items[0]
    if alt.getName() != 'AlterableString':
        raise NotImplementedError()
    return LOOP_NAMES[alt.loader.value]

LOOP_CALLS = {
    'Player01AllSensors' : ("Player01DetectSensorMain",
                            "Player01DetectSensorLeft",
                            "Player01DetectSensorRight",
                            "Player01DetectSensorTop",
                            "Player01DetectSensorBottom",
                            "Player01DetectSensorSlope",
                            "Player01DetectSensorAngles",
                            "Player01DetectSensorEdges"),
    'Player01WallCollision' : ("Player01DetectSensorLeft",
                               "Player01DetectSensorRight"),
    'Player01Up' : ("Player01DetectSensorTop",),
    'Player01WallCollision' : ('Player01DetectSensorLeft',
                               'Player01DetectSensorRight'),
    'Player01Down' : ('Player01DetectSensorBottom',),
    'Player01DetectAngle' : ('Player01DetectSensorAngles',),
    'Player01SlopesDown' : ('Player01DetectSensorMain',
                            'Player01DetectSensorSlope'),
    'Player01SlopesUp' : ('Player01DetectSensorMain',)
}

def get_loop_call_names(converter, name):
    return LOOP_CALLS.get(name, None)

