def init(converter):
    converter.add_define('CHOWDREN_IS_FP')
    converter.add_define('CHOWDREN_QUICK_SCALE')
    converter.add_define('CHOWDREN_POINT_FILTER')
    converter.add_define('CHOWDREN_OBSTACLE_IMAGE')

def get_frames(converter, game, frames):
    new_frames = {}
    # indexes = (0, 1, 2, 3, 7, 8, 14, 83, 20, 21, 22, 23, 76, 82)
    indexes = (39,)
    for index in indexes:
        new_frames[index] = frames[index]
    return new_frames

def use_safe_create(converter):
    return True

def use_global_int(converter, expression):
    index = expression.data.loader.value
    # 3: LevelTimer
    return index in (3,)

def use_deferred_collisions(converter):
    return True

global_objects = [
    'GlobalInputData',
    'Player_Others',
    'Master_Level',
    'Controller.Music'
]

def use_global_alterables(converter, obj):
    check_name = obj.data.name
    for name in global_objects:
        if check_name.startswith(name):
            return True
    return False

def use_single_global_alterables(converter, obj):
    return True

alterable_int_objects = [
    ('FileSaveSlot', None),
    ('MinibossGolem_', [5]),
    ('GachaponWheel_', [0, 2, 3, 4, 5, 6, 7, 8]),
    ('BossGunshipTurbineLeft', None),
    ('BossGunshipTurbineRight', None),
    ('RecordLabel', None),
    ('CrystalCursor', None),
    ('GimmickRisingSwingPiece_', [0, 1, 8])
]

def use_alterable_int(converter, expression):
    obj = expression.get_object()
    name = expression.converter.get_object_name(obj)
    for (check_name, alts) in alterable_int_objects:
        if not name.startswith(check_name):
            continue
        if alts is None:
            return True
        index = expression.data.loader.value
        return index in alts
    return False

counter_int_objects = [
    'MapSelect'
]

def use_counter_int(converter, expression):
    obj = expression.get_object()
    name = expression.converter.get_object_name(obj)
    for check_name in counter_int_objects:
        if name.startswith(check_name):
            return True
    return False

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


def use_loop_selection_clear(converter):
    return False
