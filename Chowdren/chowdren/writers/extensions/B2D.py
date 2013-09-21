from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ConditionMethodWriter, 
    ActionMethodWriter, ExpressionMethodWriter, make_table, EmptyAction)

def read_vec2(reader):
    return (reader.readFloat(), reader.readFloat())

class Box2D(ObjectWriter):
    class_name = 'Box2D'
    includes = ['box2d/box2dext.h']

    def write_init(self, writer):
        data = self.get_data()
        writer.putln('maxBodies = %s;' % data.readInt())
        writer.putln('maxJoints = %s;' % data.readInt())
        writer.putln('maxBodyDefs = %s;' % data.readInt())
        writer.putln('maxShapeDefs = %s;' % data.readInt())
        writer.putln('maxJointDefs = %s;' % data.readInt())
        writer.putln('gravity = b2Vec2%s;' % (read_vec2(data),))

        upper_bound = read_vec2(data)
        lower_bound = read_vec2(data)
        scale = data.readFloat()

        writer.putln('scale = %s;' % scale)
        writer.putln('bounds.upperBound = (1.0f / scale) * b2Vec2%s;' % 
            (upper_bound,))
        writer.putln('bounds.lowerBound = (1.0f / scale) * b2Vec2%s;' % 
            (lower_bound,))

        writer.putln('allowSleep = %s;' % data.readInt())
        writer.putln('posIterations = %s;' % data.readInt())
        writer.putln('velIterations = %s;' % data.readInt())
        writer.putln('timestep = %s;' % data.readFloat())
        writer.putln('WarmStart = %s;' % data.readByte())
        writer.putln('PosCorrection = %s;' % data.readByte())
        writer.putln('CCD = %s;' % data.readByte())
        writer.putln('floatAngles = %s;' % data.readByte())
        writer.putln('autoUpdate = %s;' % data.readInt())
        writer.putln('settings.b2_maxPolygonVertices = %s;' % data.readInt())
        writer.putln('settings.b2_maxProxies = %s;' % data.readInt())
        writer.putln('settings.b2_maxPairs = %s;' % data.readInt())
        writer.putln('settings.b2_linearSlop = %s;' % data.readFloat())
        writer.putln('settings.b2_angularSlop = %s;' % data.readFloat())
        writer.putln('settings.b2_toiSlop = %s;' % data.readFloat())
        writer.putln('settings.b2_maxTOIContactsPerIsland = %s;' % 
                     data.readInt())
        writer.putln('settings.b2_maxTOIJointsPerIsland = %s;' % data.readInt())
        writer.putln('settings.b2_velocityThreshold = %s;' % data.readFloat())
        writer.putln('settings.b2_maxLinearCorrection = %s;' % data.readFloat())
        writer.putln('settings.b2_maxAngularCorrection = %s;' % data.readFloat())
        writer.putln('settings.b2_maxLinearVelocity = %s;' % data.readFloat())
        writer.putln('settings.b2_maxLinearVelocitySquared = %s;' % 
                     data.readFloat())
        writer.putln('settings.b2_maxAngularVelocity = %s;' % data.readFloat())
        writer.putln('settings.b2_maxAngularVelocitySquared = %s;' % 
                     data.readFloat())
        writer.putln('settings.b2_contactBaumgarte = %s;' % data.readFloat())
        writer.putln('settings.b2_timeToSleep = %s;' % data.readFloat())
        writer.putln('settings.b2_linearSleepTolerance = %s;' % 
                     data.readFloat())
        writer.putln('settings.b2_angularSleepTolerance = %s;' % 
                     data.readFloat())
        writer.putln('settings.b2_tableCapacity = %s;' % data.readInt())
        writer.putln('settings.b2_tableMask = %s;' % data.readInt())
        writer.putln('maxControllers = %s;' % data.readInt())
        writer.putln('initialize_box2d();')

class CreateBody(ActionMethodWriter):
    method = 'create_body'

    def __init__(self, *arg, **kw):
        ActionMethodWriter.__init__(self, *arg, **kw)
        self.parameters = self.parameters[:5]

actions = make_table(ActionMethodWriter, {
    1 : CreateBody,
    2 : 'create_box',
    3 : 'reset_world',
    11 : 'create_body',
    25 : 'remove_body',
    27 : 'create_shape',
    32 : 'create_edge_chain',
    72 : 'remove_shape',
    142 : 'create_mouse_joint',
    144 : 'remove_joint',
    145 : 'set_joint_target',
    158 : 'create_distance_joint',
    205 : 'set_auto_update',
    328 : 'create_shape',
    330 : 'set_linear_damping',
    354 : 'remove_joint'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    2 : '.lastJoint'
})

def get_object():
    return Box2D