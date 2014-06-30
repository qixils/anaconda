from mmfparser.data.chunkloaders.objectinfo import SEMITRANSPARENT_EFFECT
from chowdren.writers.events import (ActionWriter, ConditionWriter,
    ExpressionWriter, ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table,
    make_expression, make_comparison, EmptyAction, FalseCondition)
from chowdren.common import (get_method_name, to_c, make_color,
                             parse_direction, get_flag_direction,
                             get_list_type, get_iter_type)
from chowdren.writers.objects import ObjectWriter
from chowdren import shader
from collections import defaultdict
from chowdren import hacks
from chowdren.key import convert_key
from mmfparser.bitdict import BitDict
from chowdren.idpool import get_id
from chowdren.shader import INK_EFFECTS, NATIVE_SHADERS

def get_loop_running_name(name):
    return 'loop_%s_running' % get_method_name(name)

def get_loop_index_name(name):
    return 'loop_%s_index' % get_method_name(name)

def get_repeat_name(group):
    return 'repeat_%s' % group.global_id

def get_restrict_name(group):
    return 'restrict_%s' % group.global_id

PROFILE_LOOPS = set([])

class SystemObject(ObjectWriter):
    def __init__(self, converter):
        self.converter = converter
        self.data = None

    def write_frame(self, writer):
        self.write_group_activated(writer)
        self.write_loops(writer)
        self.write_repeats(writer)
        self.write_restrict_for(writer)
        # self.write_collisions(writer)

    def write_start(self, writer):
        for container, names in self.group_activations.iteritems():
            for name in names:
                writer.putln(to_c('%s = true;', name))
        for name in self.loops.keys():
            running_name = get_loop_running_name(name)
            index_name = get_loop_index_name(name)
            writer.putln('%s = false;' % running_name)
            writer.putln('%s = 0;' % index_name)
        if not self.dynamic_loops:
            return
        writer.putln('static bool loops_initialized = false;')
        writer.putln('if (!loops_initialized) {')
        writer.indent()
        for loop in self.dynamic_loops:
            loop_method = 'loop_wrapper_' + get_method_name(loop)
            running_name = get_loop_running_name(loop)
            index_name = get_loop_index_name(loop)
            writer.putlnc('loops[%r].set(&%s, &%s, &%s);',
                          loop, loop_method, running_name, index_name)
        writer.putln('loops_initialized = true;')
        for name in self.repeats:
            writer.putlnc('%s = 0;', name)
        for name in self.restrict:
            writer.putlnc('%s = frame_time;', name)
        writer.end_brace()

    def write_restrict_for(self, writer):
        self.restrict = []
        for group in self.converter.always_groups_dict['RestrictFor']:
            name = get_restrict_name(group)
            writer.add_member('float ' + name)
            self.restrict.append(name)

    def write_repeats(self, writer):
        self.repeats = []
        for group in self.converter.always_groups_dict['Repeat']:
            name = get_repeat_name(group)
            writer.add_member('int ' + name)
            self.repeats.append(name)

    def write_group_activated(self, writer):
        self.group_activations = defaultdict(list)
        for group in self.converter.always_groups_dict['OnGroupActivation']:
            cond = group.conditions[0]
            container = cond.container
            check_name = cond.get_group_check()
            writer.add_member('bool %s' % check_name)
            self.group_activations[container].append(check_name)

    def write_collisions(self, writer):
        converter = self.converter

        # XXX group based on saved collision need?
        groups = defaultdict(list)
        for group in self.get_conditions('OnCollision'):
            cond = group.conditions[0]
            data = cond.data
            object_info = (data.objectInfo, data.objectType)
            other_info = (data.items[0].loader.objectInfo,
                          data.items[0].loader.objectType)
            key = (object_info, other_info)
            key = tuple(sorted(key))
            key = key + (cond.has_collisions(group),)
            groups[key].append(group)

        writer.putmeth('void test_collisions')
        for key, groups in groups.iteritems():
            object_info, other_info, has_col = key
            selected_name = converter.get_list_name(converter.get_object_name(
                object_info))
            other_selected = converter.get_list_name(converter.get_object_name(
                other_info))
            self.converter.begin_events()
            cond = group.conditions[0]
            end_name = 'col_end_%s' % get_id(cond)
            func_name = 'check_overlap_save'
            writer.putlnc('if (!%s(%s, %s, %s, %s)) goto %s;', func_name,
                          converter.get_object(object_info, True),
                          converter.get_object(other_info, True),
                          selected_name, other_selected, end_name)
            for group in groups:
                cond.add_collision_objects(object_info, other_info)

                # converter.set_list(object_info, selected_name)
                # converter.set_list(other_info, other_selected)
                converter.write_event(writer, group, True)
            writer.put_label(end_name)
        writer.end_brace()

    def write_loops(self, writer):
        self.loop_names = set()
        loops = self.loops = defaultdict(list)
        self.dynamic_loops = set()
        for loop_group in self.get_conditions('OnLoop'):
            parameter = loop_group.conditions[0].data.items[0]
            items = parameter.loader.items
            name = self.converter.convert_static_expression(items)
            if name is None:
                name = self.converter.convert_parameter(parameter)
                print 'dynamic "on loop" not implemented:', name
                continue

            if name == 'Clear Filter':
                # KU-specific hack
                continue
            self.loop_names.add(name.lower())
            loops[name].append(loop_group)

        if not loops:
            return

        for name in loops.keys():
            running_name = get_loop_running_name(name)
            index_name = get_loop_index_name(name)
            writer.add_member('bool %s' % running_name)
            writer.add_member('int %s' % index_name)

        for start_loop in self.converter.action_dict['StartLoop']:
            names = start_loop.get_loop_names(loops.keys())
            if names is None:
                continue
            self.dynamic_loops.update(names)

        self.converter.begin_events()
        for name, groups in loops.iteritems():
            profile = name in PROFILE_LOOPS
            loop_name = get_method_name(name)
            writer.putmeth('bool loop_%s' % loop_name)
            self.converter.current_loop_name = name

            if profile:
                writer.putln('double profile_time, profile_dt;')

            for index, group in enumerate(groups):
                if profile:
                    writer.putln('profile_time = platform_get_time();')
                self.converter.write_event(writer, group, True)
                if profile:
                    writer.putln('profile_dt = platform_get_time() '
                                              '- profile_time;')
                    writer.putln('if (profile_dt > 0.0001)')
                    writer.indent()
                    writer.putln(
                        ('std::cout << "Event %s took " << '
                         'profile_dt << std::endl;') % group.global_id)
                    writer.dedent()
            writer.putln('return true;')
            writer.end_brace()

        for name in self.dynamic_loops:
            loop_name = get_method_name(name)
            writer.putmeth('static bool loop_wrapper_%s' % loop_name,
                           'void * frame')
            writer.putlnc('return ((%s*)frame)->loop_%s();',
                          self.converter.frame_class, loop_name)
            writer.end_brace()


# conditions

class CollisionCondition(ConditionWriter):
    save_collision = False
    is_always = True

    def has_collisions(self, group=None):
        if not self.save_collision:
            return False
        group = group or self.converter.current_group
        actions = group.actions
        for action in actions:
            if action.data.getName() in ('Stop', 'Bounce'):
                return True
        return False

    def add_collision_objects(self, *handles):
        for handle in handles:
            infos = self.converter.resolve_qualifier(handle)
            self.converter.collision_objects.update(infos)

class IsOverlapping(CollisionCondition):
    has_object = False
    custom = True

    def write(self, writer):
        data = self.data
        negated = data.otherFlags['Not']
        object_info = (data.objectInfo, data.objectType)
        other_info = (data.items[0].loader.objectInfo,
                      data.items[0].loader.objectType)
        converter = self.converter
        if negated:
            condition = to_c('check_not_overlap(%s, %s)',
                             converter.get_object(object_info, True),
                             converter.get_object(other_info, True))
        else:
            selected_name = converter.create_list(object_info, writer)
            other_selected = converter.create_list(other_info, writer)
            if self.has_collisions():
                func_name = 'check_overlap<true>'
                self.add_collision_objects(object_info, other_info)
            else:
                func_name = 'check_overlap<false>'
            condition = to_c('%s(%s, %s)', func_name, selected_name,
                             other_selected)
        writer.putlnc('if (!%s) %s', condition, self.converter.event_break)

    def is_negated(self):
        return False

class OnCollision(IsOverlapping):
    save_collision = True
    # is_always = False

class OnBackgroundCollision(CollisionCondition):
    # is_always = False
    save_collision = True

    def write(self, writer):
        obj = self.get_object()
        if self.has_collisions():
            func_name = 'overlaps_background_save'
            self.add_collision_objects(obj)
        else:
            func_name = 'overlaps_background'
        writer.put('%s()' % func_name)

class ObjectInvisible(ConditionWriter):
    def write(self, writer):
        writer.put('flags & VISIBLE')

    def is_negated(self):
        return True

class MouseOnObject(ConditionWriter):
    def get_object(self):
        data = self.data.items[0].loader
        return data.objectInfo, data.objectType

    def write(self, writer):
        writer.put('mouse_over()')

class Always(ConditionWriter):
    custom = True

    def write(self, writer):
        pass

class MouseClicked(ConditionWriter):
    is_always = True

    def write(self, writer):
        writer.put('is_mouse_pressed_once(%s)' % self.convert_index(0))

class ObjectClicked(ConditionWriter):
    is_always = True

    def get_object(self):
        data = self.data.items[1].loader
        return data.objectInfo, data.objectType

    def write(self, writer):
        writer.put('mouse_over() && '
                   'is_mouse_pressed_once(%s)' % self.convert_index(0))

KEY_FLAGS = BitDict(
    'Up',
    'Down',
    'Left',
    'Right',
    'Button1',
    'Button2',
    'Button3',
    'Button4'
)

JOYSTICK_CONDITIONS = {
    'Button1' : 'CHOWDREN_BUTTON_A',
    'Button2' : 'CHOWDREN_BUTTON_B',
    'Button3' : 'CHOWDREN_BUTTON_X',
    'Button4' : 'CHOWDREN_BUTTON_Y'
}

class PlayerKeyCondition(ConditionWriter):
    method = None

    def write(self, writer):
        player = self.data.objectInfo + 1
        controls = self.converter.game.header.controls.items[player-1]
        keys = []
        flag_value = self.parameters[0].loader.value
        is_keyboard = controls.getControlType() == 'Keyboard'
        if not is_keyboard and flag_value & 15:
            direction = flag_value & 15
            key = 'test_joystick_direction_flags(%s, %s)' % (player,
                                                             direction)
            keys.append(key)
            flag_value &= ~15

        flags = KEY_FLAGS.copy()
        flags.setFlags(flag_value)
        for k, v in flags.iteritems():
            if not v:
                continue
            if is_keyboard:
                key = getattr(controls.keys, k.lower())
                key = convert_key(key.getValue())
                key = '%s(%s)' % (self.key_method, key)
            else:
                key = JOYSTICK_BUTTONS[k]
                key = '%s(%s, %s)' % (self.joystick_method, player, key)
            keys.append(key)

        # true if nothing is pressed
        if not keys:
            if is_keyboard:
                for k in flags.keys.keys():
                    key = getattr(controls.keys, k.lower())
                    key = convert_key(key.getValue())
                    key = '!%s(%s)' % (self.key_method, key)
                    keys.append(key)
            else:
                raise NotImplementedError()

        writer.put(' && '.join(keys))

class PlayerKeyDown(PlayerKeyCondition):
    key_method = 'is_key_pressed'
    joystick_method = 'is_joystick_pressed'

class PlayerKeyPressed(PlayerKeyCondition):
    is_always = True
    key_method = 'is_key_pressed_once'
    joystick_method = 'is_joystick_pressed_once'

class TimerEquals(ConditionWriter):
    is_always = True

    def write(self, writer):
        seconds = self.parameters[0].loader.timer / 1000.0
        writer.put('frame_time >= %s' % seconds)

class TimerGreater(ConditionWriter):
    is_always = True

    def write(self, writer):
        seconds = self.parameters[0].loader.timer / 1000.0
        writer.put('frame_time >= %s' % seconds)

class TimerLess(ConditionWriter):
    is_always = True

    def write(self, writer):
        seconds = self.parameters[0].loader.timer / 1000.0
        writer.put('frame_time <= %s' % seconds)

class TimerEvery(ConditionWriter):
    is_always = True
    custom = True

    def write(self, writer):
        seconds = self.parameters[0].loader.delay / 1000.0
        name = 'every_%s' % get_id(self)
        name2 = '%s_frame' % name
        writer.putln('static float %s = 0.0f;' % name)
        writer.putlnc('static unsigned int %s = frame_iteration;', name2)
        writer.putlnc('if (%s != frame_iteration) {', name2)
        writer.indent()
        writer.putlnc('%s = frame_iteration;', name2)
        writer.putlnc('%s = 0.0f;', name)
        writer.end_brace()
        event_break = self.converter.event_break
        writer.putln('%s += float(manager->fps_limit.dt);' % name)
        writer.putln('if (%s < %s) %s' % (name, seconds, event_break))
        writer.putln('%s -= %s;' % (name, seconds))

class AnimationFinished(ConditionWriter):
    is_always = True

    def write(self, writer):
        writer.put('is_animation_finished(%s)' % self.convert_index(0))

class PathFinished(ConditionMethodWriter):
    is_always = True
    method = 'get_movement()->is_path_finished'

class NodeReached(ConditionMethodWriter):
    is_always = True
    method = 'get_movement()->is_node_reached'

class OnGroupActivation(ConditionWriter):
    custom = True

    def write(self, writer):
        group_check = self.get_group_check()
        writer.putln('if (!%s) %s' % (group_check, self.converter.event_break))
        writer.putln('%s = false;' % group_check)

    def get_group_check(self):
        return 'group_check_%s' % get_id(self)

class RepeatCondition(ConditionWriter):
    custom = True

    def write(self, writer):
        count = self.convert_index(0)
        name = get_repeat_name(self.converter.current_group)
        writer.putlnc('if (%s >= %s) %s', name, count,
                      self.converter.event_break)
        writer.putlnc('%s++;', name)

class RestrictFor(ConditionWriter):
    custom = True

    def write(self, writer):
        # XXX reset on frame start
        seconds = self.parameters[0].loader.timer / 1000.0
        name = get_restrict_name(self.converter.current_group)
        writer.putlnc('if (frame_time - %s < %s) %s', name, seconds,
                      self.converter.event_break)
        writer.putlnc('%s = frame_time;', name)

class NotAlways(ConditionWriter):
    custom = True

    def write(self, writer):
        event_break = self.converter.event_break
        name = 'not_always_%s' % get_id(self)
        name2 = '%s_frame' % name
        writer.putln('static unsigned int %s = loop_count;' % name)
        writer.putln('static unsigned int %s = frame_iteration;' % name2)
        writer.putln('if (%s != frame_iteration) {' % name2)
        writer.indent()
        writer.putln('%s = frame_iteration;' % name2)
        writer.putln('%s = loop_count;' % name)
        writer.end_brace()
        writer.putln('if (%s > loop_count) {' % (name))
        writer.indent()
        writer.putln('%s = loop_count + 2;' % name)
        writer.putln(event_break)
        writer.end_brace()
        writer.putln('%s = loop_count + 2;' % name)

class OnceCondition(ConditionWriter):
    custom = True
    def write(self, writer):
        event_break = self.converter.event_break
        name = 'once_condition_%s' % get_id(self)
        writer.putln('static unsigned int %s = (int)(-1);' % name)
        writer.putln('if (%s == frame_iteration) %s' % (name, event_break))
        writer.putln('%s = frame_iteration;' % (name))

class GroupActivated(ConditionWriter):
    def write(self, writer):
        container = self.converter.containers[
            self.parameters[0].loader.pointer]
        writer.put(self.converter.get_container_check(container))

class PickRandom(ConditionWriter):
    custom = True
    def write(self, writer):
        obj = self.get_object()
        converter = self.converter
        selected_name = converter.create_list(obj, writer)
        writer.putlnc('if (!pick_random(%s)) %s', selected_name,
                      self.converter.event_break)

class NumberOfObjects(ComparisonWriter):
    has_object = False
    iterate_objects = False

    def get_comparison_value(self):
        obj = (self.data.objectInfo, self.data.objectType)
        return '%s.size()' % self.converter.get_object_list(obj)

class CompareObjectsInZone(ComparisonWriter):
    has_object = False
    iterate_objects = False

    def get_parameters(self):
        return self.parameters[1:]

    def get_comparison_value(self):
        zone = self.parameters[0].loader
        obj = (self.data.objectInfo, self.data.objectType)
        obj_list = self.converter.get_object_list(obj)
        return 'objects_in_zone(%s, %s, %s, %s, %s)' % (obj_list, zone.x1,
            zone.y1, zone.x2, zone.y2)

class PickObjectsInZone(ConditionWriter):
    custom = True
    def write(self, writer):
        objs = set()
        zone = self.parameters[0].loader
        for action in self.converter.current_group.actions:
            objs.add(action.get_object())
        for obj in objs:
            obj_list = self.converter.create_list(obj, writer)
            writer.putlnc('pick_objects_in_zone(%s, %s, %s, %s, %s);',
                          obj_list, zone.x1, zone.y1, zone.x2, zone.y2)

class CompareFixedValue(ConditionWriter):
    custom = True
    def write(self, writer):
        obj = self.get_object()
        converter = self.converter

        end_label = 'fixed_%s_end' % get_id(self)
        value = self.convert_index(0)
        comparison = self.get_comparison()
        is_equal = comparison == '=='
        has_selection = obj in converter.has_selection
        is_instance = value.endswith('get_fixed()')
        test_all = has_selection or not is_equal or not is_instance
        if is_instance:
            instance_value = value.replace('->get_fixed()', '')
        else:
            instance_value = 'get_object_from_fixed(%s)' % value

        fixed_name = 'fixed_test_%s' % get_id(self)
        writer.putln('FrameObject * %s = %s;' % (fixed_name, instance_value))
        if is_equal:
            event_break = converter.event_break
        else:
            event_break = 'goto %s;' % end_label

        if not is_instance:
            writer.putln('if (%s == NULL) %s' % (fixed_name, event_break))
        if test_all:
            list_name = converter.create_list(obj, writer)
            with converter.iterate_object(obj, writer, copy=False):
                writer.putlnc('if (!((*it) %s %s)) it.deselect();', comparison,
                              fixed_name)
            if not is_equal:
                writer.put_label(end_label)
            writer.putlnc('if (!%s.has_selection()) %s', list_name,
                          converter.event_break)
        else:
            converter.set_object(obj, fixed_name)

class AnyKeyPressed(ConditionMethodWriter):
    is_always = True
    method = 'is_any_key_pressed_once'

class FacingInDirection(ConditionWriter):
    def write(self, writer):
        parameter = self.parameters[0].loader
        if parameter.isExpression:
            name = 'test_direction'
            value = self.convert_index(0)
        else:
            name = 'test_directions'
            value = parameter.value
        writer.put('%s(%s)' % (name, value))

class InsidePlayfield(ConditionMethodWriter):
    method = 'outside_playfield'

    def is_negated(self):
        return not ConditionMethodWriter.is_negated(self)

# actions

class CollisionAction(ActionWriter):
    def write(self, writer):
        obj = self.get_object()
        is_col = obj in self.converter.collision_objects
        writer.put(to_c(self.method + ';', is_col))

class StopAction(CollisionAction):
    method = 'get_movement()->stop(%s)'

class BounceAction(CollisionAction):
    method = 'get_movement()->bounce(%s)'

class CreateBase(ActionWriter):
    custom = True
    def write(self, writer):
        details = self.convert_index(0)
        if details is None:
            return
        end_name = 'create_%s_end' % get_id(self)
        is_shoot = self.is_shoot
        x = str(details['x'])
        y = str(details['y'])
        parent_info = details.get('parent', None)
        direction = None
        use_direction = details.get('use_direction', False)
        use_action_point = details.get('use_action_point', False)

        object_info = self.parameters[0].loader.objectInfo
        object_info = (object_info, self.converter.object_types[object_info])
        if is_shoot:
            create_object = details['shoot_object']
            if use_action_point and not use_direction:
                direction = parse_direction(details['direction'])
            else:
                direction = '-1'
        else:
            create_object = details['create_object']

        action_index = self.converter.current_group.actions.index(self)

        if object_info != parent_info:
            has_selection = object_info in self.converter.has_selection
            last_info, last_index = self.converter.current_group.last_created
            was_last = (object_info == last_info and
                        action_index - last_index > 1)
            if not has_selection or was_last:
                list_name = self.converter.get_object_list(object_info)
                writer.putlnc('%s.empty_selection();', list_name)
                self.converter.set_list(object_info, list_name)

        self.converter.current_group.last_created = (object_info, action_index)

        single_parent = self.converter.get_single(parent_info)
        if single_parent:
            parent = single_parent
        elif parent_info is not None:
            self.converter.start_object_iteration(parent_info, writer, 'p_it',
                                                  copy=False)
            writer.putlnc('FrameObject * parent = *p_it;')
            parent = 'parent'
        writer.start_brace()
        if parent_info is not None and not is_shoot:
            if use_action_point:
                parent_x = 'get_action_x() - %s->layer->off_x' % parent
                parent_y = 'get_action_y() - %s->layer->off_y' % parent
            else:
                parent_x = 'x'
                parent_y = 'y'
            if details.get('transform_position_direction', False):
                writer.putln('int x_off; x_off = %s;' % x)
                writer.putln('int y_off; y_off = %s;' % y)
                writer.putlnc('transform_pos(x_off, y_off, %s);', parent)
                x = 'x_off'
                y = 'y_off'
            if use_direction:
                direction = '%s->direction' % parent
            x = '%s->%s + %s' % (parent, parent_x, x)
            y = '%s->%s + %s' % (parent, parent_y, y)
            layer = '%s->layer' % parent
        elif is_shoot:
            layer = '%s->layer' % parent
        else:
            layer = details['layer']
        writer.putlnc('FrameObject * new_obj; // %s', details)
        self.converter.create_object(create_object, x, y, layer, 'new_obj',
                                     writer)
        if is_shoot:
            writer.putlnc('%s->shoot(new_obj, %s, %s);', parent,
                          details['shoot_speed'], direction)
            # object_class = self.converter.get_object_class(
            #     object_info=parent_info, star=False)
            # print object_class
            # if object_class == 'Active':
            #     parent = '((Active*)%s)' % parent
            #     writer.putlnc('if (%s->has_animation(SHOOTING))', parent)
            #     writer.indent()
            #     writer.putlnc('%s->force_animation(SHOOTING);', parent)
            #     writer.dedent()

        elif direction:
            writer.putln('new_obj->set_direction(%s);' % direction)
        writer.end_brace()
        if parent_info is not None and not single_parent:
            self.converter.end_object_iteration(parent_info, writer, 'p_it',
                                                copy=False)
        if False: # action_name == 'DisplayText':
            paragraph = parameters[1].loader.value
            if paragraph != 0:
                raise NotImplementedError

class CreateObject(CreateBase):
    is_shoot = False

class ShootObject(CreateBase):
    is_shoot = True

class SetPosition(ActionWriter):
    custom = True

    def write(self, writer):
        object_info = self.get_object()

        end_name = 'pos_end_%s' % get_id(self)

        single = self.converter.get_single(object_info)
        if single:
            obj = single
        else:
            self.converter.start_object_iteration(object_info, writer,
                                                  copy=False)
            obj = '(*it)'

        details = self.convert_index(0)
        x = str(details['x'])
        y = str(details['y'])
        parent = details.get('parent', None)
        if parent is not None:
            parent = self.converter.get_object(parent)
            writer.putln('FrameObject * parent = %s;' % parent)
            writer.putlnc('if (parent == NULL) goto %s;', end_name)
            if details.get('use_action_point', False):
                parent_x = 'get_action_x()'
                parent_y = 'get_action_y()'
            else:
                parent_x = 'get_x()'
                parent_y = 'get_y()'
            if details.get('transform_position_direction', False):
                writer.putln('int x_off; x_off = %s;' % x)
                writer.putln('int y_off; y_off = %s;' % y)
                writer.putln('transform_pos(x_off, y_off, parent);')
                x = 'x_off'
                y = 'y_off'
            if details.get('use_direction', False):
                writer.putln('(*it)->set_direction(parent->direction);')
            x = 'parent->%s + %s' % (parent_x, x)
            y = 'parent->%s + %s' % (parent_y, y)
        arguments = [x, y]
        writer.putlnc('%s->set_global_position(%s); // %s', obj,
                      ', '.join(arguments), details)
        writer.put_label(end_name)
        if not single:
            self.converter.end_object_iteration(object_info, writer,
                                                copy=False)

class LookAt(ActionWriter):
    def write(self, writer):
        object_info = self.get_object()
        instance = self.converter.get_object(object_info)
        details = self.convert_index(0)
        x = str(details['x'])
        y = str(details['y'])
        parent = details.get('parent', None)
        if not parent:
            raise NotImplementedError()
        parent = self.converter.get_object(parent)
        x = '%s->x + %s' % (parent, x)
        y = '%s->y + %s' % (parent, y)
        writer.put('set_direction(get_direction_int(%s->x, %s->y, %s, %s));'
            % (instance, instance, x, y))

class MoveInFront(ActionWriter):
    def write(self, writer):
        object_info = (self.parameters[0].loader.objectInfo,
                       self.parameters[0].loader.objectType)
        writer.put('move_front(%s);' % (self.converter.get_object(object_info)))

class MoveBehind(ActionWriter):
    def write(self, writer):
        object_info = (self.parameters[0].loader.objectInfo,
                       self.parameters[0].loader.objectType)
        writer.put('move_back(%s);' % (self.converter.get_object(object_info)))

class StartLoop(ActionWriter):
    custom = True

    def get_name(self):
        parameter = self.parameters[0]
        items = parameter.loader.items
        real_name = self.converter.convert_static_expression(items)
        return real_name

    def get_dynamic_name(self):
        return self.convert_index(0)

    def get_loop_names(self, loops):
        if self.get_name() is not None:
            return
        strings = self.converter.get_string_expressions(self.parameters[0])
        strings = [v.lower() for v in strings]
        loop_names = set()
        for loop in loops:
            loop_check = loop.lower()
            for substring in strings:
                if substring not in loop_check:
                    continue
                loop_names.add(loop)
                break
        return loop_names

    def write(self, writer):
        real_name = self.get_name()
        if real_name is None:
            func_call = 'dyn_loop.callback(this)'
        else:
            loop_names = self.converter.system_object.loop_names
            if real_name.lower() not in loop_names:
                if real_name == 'Clear Filter':
                    self.converter.clear_selection()
                print 'Could not find loop %r' % real_name
                return
            name = get_method_name(real_name)
            func_call = 'loop_%s()' % name
        comparison = None
        times = None
        try:
            exp, = self.parameters[1].loader.items[:-1]
            if exp.getName() == 'Long':
                times = exp.loader.value
                if times == -1:
                    comparison = 'true'
        except ValueError:
            pass
        if times is None:
            times = self.convert_index(1)
        is_infinite = comparison is not None
        is_dynamic = real_name is None
        if is_dynamic:
            running_name = '(*dyn_loop.running)'
            index_name = '(*dyn_loop.index)'
        else:
            running_name = get_loop_running_name(real_name)
            index_name = get_loop_index_name(real_name)
        if not is_infinite:
            comparison = '%s < times' % index_name
        writer.start_brace()
        if is_dynamic:
            writer.putlnc('DynamicLoop & dyn_loop = loops[%s];',
                          self.convert_index(0))
        writer.putln('%s = true;' % running_name)
        if not is_infinite:
            writer.putln('int times = int(%s);' % times)
        writer.putln('%s = 0;' % index_name)
        writer.putln('while (%s) {' % comparison)
        writer.indent()
        writer.putln('%s;' % func_call)
        writer.putln('if (!%s) break;' % running_name)
        writer.putln('%s++;' % index_name)
        writer.end_brace()
        writer.end_brace()

        # since we have cleared the object selection list, we need to
        # remove and insert a new scope
        writer.end_brace()
        writer.start_brace()

        self.converter.clear_selection()

class DeactivateGroup(ActionWriter):
    deactivated_container = None
    def write(self, writer):
        container = self.get_deactivated_container()
        writer.putln('%s = false;' % container.code_name)

    def get_deactivated_container(self):
        if self.deactivated_container is None:
            self.deactivated_container = self.converter.containers[
                self.parameters[0].loader.pointer]
        return self.deactivated_container

    def write_post(self, writer):
        pass
        # container = self.get_deactivated_container()
        # if container in self.converter.container_tree:
        #     writer.putln('goto %s;' % container.end_label)
        # elif self.container and container in self.container.tree:
        #     pass

class StopLoop(ActionWriter):
    def write(self, writer):
        parameter = self.parameters[0]
        items = parameter.loader.items
        name = self.converter.convert_static_expression(items)
        if name is None:
            name = self.converter.convert_parameter(parameter)
            print 'Could not convert stop loop name for', name
            return
        running_name = get_loop_running_name(name)
        writer.putln('%s = false;' % running_name)

class SetLoopIndex(ActionWriter):
    def write(self, writer):
        exp, = self.parameters[0].loader.items[:-1]
        name = exp.loader.value
        index_name = get_loop_index_name(name)
        value = self.convert_index(1)
        writer.putln('%s = %s;' % (index_name, value))

class ActivateGroup(ActionWriter):
    custom = True

    def write(self, writer):
        container = self.converter.containers[
            self.parameters[0].loader.pointer]
        writer.putlnc('if (!%s) {', container.code_name)
        writer.indent()
        writer.putlnc('%s = true;', container.code_name)
        check_names = set()
        group_activations = self.converter.system_object.group_activations
        for child in ([container] + container.get_all_children()):
            check_names.update(group_activations[child])
        for name in check_names:
            writer.putln('%s = true;' % name)
        writer.end_brace()


class CenterDisplayX(ActionWriter):
    def write(self, writer):
        writer.put('set_display_center(%s, -1);' % self.convert_index(0))

class CenterDisplayY(ActionWriter):
    def write(self, writer):
        writer.put('set_display_center(-1, %s);' % self.convert_index(0))

class CenterDisplay(ActionWriter):
    custom = True

    def write(self, writer):
        writer.start_brace()

        details = self.convert_index(0)
        x = str(details['x'])
        y = str(details['y'])
        parent = details.get('parent', None)
        if parent is not None:
            parent = self.converter.get_object(parent)
            writer.putln('FrameObject * parent = %s;' % parent)
            if details.get('use_action_point', False):
                parent_x = 'get_action_x()'
                parent_y = 'get_action_y()'
            else:
                parent_x = 'get_x()'
                parent_y = 'get_y()'
            if details.get('transform_position_direction', False):
                writer.putln('int x_off = %s;' % x)
                writer.putln('int y_off = %s;' % y)
                writer.putln('transform_pos(x_off, y_off, parent);')
                x = 'x_off'
                y = 'y_off'
            x = 'parent->%s + %s' % (parent_x, x)
            y = 'parent->%s + %s' % (parent_y, y)
        writer.putlnc('set_display_center(%s, %s);', x, y)
        writer.end_brace()

class EndApplication(ActionWriter):
    def write(self, writer):
        writer.put('has_quit = true;')

class SetFrameAction(ActionWriter):
    def set_frame(self, writer, value):
        writer.put('next_frame = %s;' % value)
        writer.putln('')
        fade = self.converter.current_frame.fadeOut
        if not fade:
            return
        color = fade.color
        writer.putln(to_c('manager->set_fade(%s, %s);', make_color(
            color), 1.0 / (fade.duration / 1000.0)))

class JumpToFrame(SetFrameAction):
    def write(self, writer):
        try:
            frame = self.parameters[0].loader
            if frame.isExpression:
                value = '%s-1' % self.convert_index(0)
            else:
                value = str(self.converter.game.frameHandles[frame.value])
            self.set_frame(writer, value)
        except IndexError:
            pass

class RestartFrame(SetFrameAction):
    def write(self, writer):
        self.set_frame(writer, 'index')

class NextFrame(SetFrameAction):
    def write(self, writer):
        self.set_frame(writer, 'index + 1')

class PreviousFrame(SetFrameAction):
    def write(self, writer):
        self.set_frame(writer, 'index - 1')

class SetInkEffect(ActionWriter):
    def write(self, writer):
        ink_effect = self.parameters[0].loader.value1
        ink_value = self.parameters[0].loader.value2

        if ink_effect in INK_EFFECTS:
            name = NATIVE_SHADERS[INK_EFFECTS[ink_effect]]
            if name is not None:
                writer.put('set_shader(%s);' % name)
        elif ink_effect == SEMITRANSPARENT_EFFECT:
            # XXX maybe also set shader
            writer.put('blend_color.set_semi_transparency(%s);' % ink_value)
        else:
            print 'unknown set ink effect:', ink_effect

class SetEffect(ActionWriter):
    def write(self, writer):
        name = self.parameters[0].loader.value
        if name == '':
            shader_name = 'NULL'
        else:
            shader_name = shader.get_name(name)
        writer.put('set_shader(%s);' % shader_name)

class SpreadValue(ActionWriter):
    custom = True
    def write(self, writer):
        alt = self.convert_index(0)
        start = self.convert_index(1)
        obj = self.get_object()
        object_list = self.converter.get_object(obj, True)
        writer.putln('spread_value(%s, %s, %s);' % (object_list, alt, start))

class Destroy(ActionMethodWriter):
    method = 'destroy'
    ignore_static = True

# expressions

class ValueExpression(ExpressionWriter):
    def get_string(self):
        return to_c('%r', self.data.loader.value)

class ConstantExpression(ExpressionWriter):
    def get_string(self):
        return self.value

class StringExpression(ExpressionWriter):
    def get_string(self):
        # self.converter.start_clauses -= self.data.loader.value.count('(')
        # self.converter.end_clauses -= self.data.loader.value.count(')')
        return self.converter.intern_string(self.data.loader.value)

class EndParenthesis(ConstantExpression):
    value = ')'

class PlusExpression(ConstantExpression):
    value = '+'

class MinusExpression(ConstantExpression):
    value = '-'

class MultiplyExpression(ConstantExpression):
    value = '*'

class DivideExpression(ConstantExpression):
    value = '/'

    def get_string(self):
        if hacks.use_safe_division(self.converter):
            return '/math_helper/'
        return self.value

class ModulusExpression(ConstantExpression):
    value = '%'

class ParenthesisExpression(ConstantExpression):
    value = '('

class VirguleExpression(ExpressionWriter):
    def get_string(self):
        out = ''
        if self.converter.last_out[-1] == '(':
            out += ')'
        out += ', '
        return out

class AlterableValueExpression(ExpressionWriter):
    def get_string(self):
        if hacks.use_alterable_int(self):
            func = 'alterables->values.get_int'
        else:
            func = 'alterables->values.get'
        return '%s(%s)' % (func, self.data.loader.value)

class AlterableValueIndexExpression(ExpressionWriter):
    def get_string(self):
        return 'alterables->values.get('

class AlterableStringExpression(ExpressionWriter):
    def get_string(self):
        return 'alterables->strings.get(%s)' % self.data.loader.value

class GlobalValueExpression(ExpressionWriter):
    def get_string(self):
        if hacks.use_global_int(self):
            func = 'global_values->get_int'
        else:
            func = 'global_values->get'
        return '%s(%s)' % (func, self.data.loader.value)

class GlobalStringExpression(ExpressionWriter):
    def get_string(self):
        return 'global_strings->get(%s)' % self.data.loader.value

class ObjectCount(ExpressionWriter):
    has_object = False

    def get_string(self):
        obj = (self.data.objectInfo, self.data.objectType)
        instances = self.converter.get_object_list(obj, allow_single=True)
        return '%s.size()' % instances

class ToString(ExpressionWriter):
    def get_string(self):
        converter = self.converter
        next = converter.expression_items[converter.item_index + 1].getName()
        if next == 'FixedValue':
            return 'std::string('
        return 'number_to_string('

class GetLoopIndex(ExpressionWriter):
    def get_string(self):
        converter = self.converter
        items = converter.expression_items
        last_exp = items[converter.item_index + 2]
        if last_exp.getName() != 'EndParenthesis':
            name = converter.convert_static_expression(items,
                                                       converter.item_index+1)
            if name is None:
                return 'get_loop_index('
            size = len(items) - 1
        else:
            size = 2
            next_exp = items[converter.item_index + 1]
            name = next_exp.loader.value
        converter.item_index += size
        index_name = get_loop_index_name(name)
        return index_name

actions = make_table(ActionMethodWriter, {
    'CreateObject' : CreateObject,
    'Shoot' : ShootObject,
    'StartLoop' : StartLoop,
    'StopLoop' : StopLoop,
    'SetX' : 'set_x',
    'SetY' : 'set_y',
    'SetAlterableValue' : 'alterables->values.set',
    'AddToAlterable' : 'alterables->values.add',
    'SpreadValue' : SpreadValue,
    'SubtractFromAlterable' : 'alterables->values.sub',
    'SetAlterableString' : 'alterables->strings.set',
    'AddCounterValue' : 'add',
    'SubtractCounterValue' : 'subtract',
    'SetCounterValue' : 'set',
    'SetMaximumValue' : 'set_max',
    'SetMinimumValue' : 'set_min',
    'SetGlobalString' : 'global_strings->set',
    'SetGlobalValue' : 'global_values->set',
    'AddGlobalValue' : 'global_values->add',
    'SubtractGlobalValue' : 'global_values->sub',
    'SetString' : 'set_string',
    'SetBold' : 'set_bold',
    'Hide' : 'set_visible(false)',
    'Show' : 'set_visible(true)',
    'SetParagraph' : 'set_paragraph(%s-1)',
    'LockChannel' : 'media->lock(%s-1)',
    'StopChannel' : 'media->stop_channel(%s-1)',
    'ResumeChannel' : 'media->resume_channel(%s-1)',
    'PauseChannel' : 'media->pause_channel(%s-1)',
    'SetChannelPosition' : 'media->set_channel_position(%s-1, %s)',
    'SetChannelPan' : 'media->set_channel_pan(%s-1, %s)',
    'SetChannelVolume' : 'media->set_channel_volume(%s-1, %s)',
    'PlayLoopingChannelFileSample' : 'media->play(%s, %s-1, %s)',
    'PlayChannelFileSample' : 'media->play(%s, %s-1)',
    'PlayChannelSample' : 'media->play_name("%s", %s-1)',
    'PlayLoopingChannelSample' : 'media->play_name("%s", %s-1, %s)',
    'PlayLoopingSample' : 'media->play_name("%s", -1, %s)',
    'PlaySample' : 'media->play_name("%s", -1, 1)',
    'SetChannelFrequency' : 'media->set_channel_frequency(%s-1, %s) ',
    'SetDirection' : 'set_direction',
    'SetRGBCoefficient' : 'set_blend_color',
    'SetAngle' : 'set_angle',
    'DeactivateGroup' : DeactivateGroup,
    'ActivateGroup' : ActivateGroup,
    'CenterDisplayX' : CenterDisplayX,
    'CenterDisplayY' : CenterDisplayY,
    'CenterDisplay' : CenterDisplay,
    'EndApplication' : EndApplication,
    'RestartApplication' : 'restart',
    'LookAt' : LookAt,
    'SetPosition' : SetPosition,
    'ExecuteEvaluatedProgram' : 'open_process',
    'HideCursor' : 'set_cursor_visible(false)',
    'ShowCursor' : 'set_cursor_visible(true)',
    'FullscreenMode' : 'set_fullscreen(true)',
    'NextFrame' : NextFrame,
    'PreviousFrame' : PreviousFrame,
    'MoveToLayer' : 'set_layer(%s-1)',
    'JumpToFrame' : JumpToFrame,
    'RestartFrame' : RestartFrame,
    'SetAlphaCoefficient' : 'blend_color.set_alpha_coefficient(%s)',
    'SetSemiTransparency' : 'blend_color.set_semi_transparency(%s)',
    'SetXScale' : 'set_x_scale({0})',
    'SetYScale' : 'set_y_scale({0})',
    'SetScale' : 'set_scale({0})',
    'ForceAnimation' : 'force_animation',
    'RestoreAnimation' : 'restore_animation',
    'ForceFrame' : 'force_frame',
    'ForceSpeed' : 'force_speed',
    'RestoreFrame' : 'restore_frame',
    'SetInkEffect' : SetInkEffect,
    'SetEffect' : SetEffect,
    'AddToDebugger' : EmptyAction,
    'SetFrameRate' : 'manager->set_framerate(%s)',
    'Destroy' : Destroy,
    'BringToBack' : 'move_back',
    'BringToFront' : 'move_front',
    'DeleteAllCreatedBackdrops' : 'layers[%s-1]->destroy_backgrounds()',
    'DeleteCreatedBackdrops' : 'layers[%s-1]->destroy_backgrounds(%s, %s, %s)',
    'SetEffectParameter' : 'set_shader_parameter',
    'SetFrameBackgroundColor' : 'set_background_color',
    'AddBackdrop' : 'paste',
    'PasteActive' : 'paste',
    'MoveInFront' : MoveInFront,
    'MoveBehind' : MoveBehind,
    'ForceDirection' : 'force_direction',
    'RestoreDirection' : 'restore_direction',
    'StopAnimation' : 'stop_animation',
    'StartAnimation' : 'start_animation',
    'RestoreSpeed' : 'restore_speed',
    'SetMainVolume' : 'media->set_main_volume',
    'StopAllSamples' : 'media->stop_samples',
    'PauseAllSounds' : 'media->pause_samples',
    'ResumeAllSounds' : 'media->resume_samples',
    'StopSample' : 'media->stop_sample("%s")',
    'SetSamplePan' : 'media->set_sample_volume("%s", %s)',
    'SetSamplePosition' : 'media->set_sample_position("%s", %s)',
    'SetSampleVolume' : 'media->set_sample_volume("%s", %s)',
    'SetSampleFrequency' : 'media->set_sample_frequency("%s", %s)',
    'NextParagraph' : 'next_paragraph',
    'PauseApplication' : 'pause',
    'SetRandomSeed' : 'set_random_seed',
    'SetTimer' : 'set_timer((%s) / 1000.0)',
    'SetLoopIndex' : SetLoopIndex,
    'IgnoreControls' : EmptyAction, # XXX fix
    'RestoreControls' : EmptyAction, # XXX fix,
    'ChangeControlType' : EmptyAction, # XXX fix,
    'FlashDuring' : 'flash((%s) / 1000.0)',
    'SetMaximumSpeed' : 'get_movement()->set_max_speed',
    'SetSpeed' : 'get_movement()->set_speed',
    'Bounce' : BounceAction,
    'Start' : 'get_movement()->start()',
    'Stop': StopAction,
    'SetDirections' : 'get_movement()->set_directions',
    'GoToNode' : 'get_movement()->set_node',
    'SelectMovement' : 'set_movement(%s)',
    'EnableFlag' : 'alterables->flags.enable',
    'DisableFlag' : 'alterables->flags.disable',
    'ToggleFlag' : 'alterables->flags.toggle',
    'Reverse' : 'get_movement()->reverse()',
    'ReplaceColor' : EmptyAction, # XXX fix,
    'SetLives' : 'set_lives',
    'SetScore' : 'set_score',
    'SubtractLives' : 'set_lives(manager->lives - (%s))',
    'AddLives' : 'set_lives(manager->lives + (%s))',
    'EnableVsync' : 'set_vsync(true)',
    'DisableVsync' : 'set_vsync(false)',
    'SetGravity' : 'get_movement()->set_gravity',
    'LoadActiveFrame' : 'load',
    'SetClipboard' : EmptyAction,
    'SetFrameEffect' : EmptyAction, # XXX fix
    'SetFrameEffectParameter' : EmptyAction, # XXX fix
    'SetFrameAlphaCoefficient' : EmptyAction, # XXX fix
    # XXX implement this
    'JumpSubApplicationFrame' : EmptyAction,
})

conditions = make_table(ConditionMethodWriter, {
    'CompareAlterableValue' : make_comparison('alterables->values.get(%s)'),
    'CompareAlterableString' : make_comparison('alterables->strings.get(%s)'),
    'CompareGlobalValue' : make_comparison('global_values->get(%s)'),
    'CompareGlobalString' : make_comparison('global_strings->get(%s)'),
    'CompareCounter' : make_comparison('value'),
    'CompareX' : make_comparison('get_x()'),
    'CompareY' : make_comparison('get_y()'),
    'Compare' : make_comparison('%s'),
    'IsOverlapping' : IsOverlapping,
    'OnCollision' : OnCollision,
    'ObjectVisible' : '.flags & VISIBLE',
    'ObjectInvisible' : ObjectInvisible,
    'WhileMousePressed' : 'is_mouse_pressed',
    'MouseOnObject' : MouseOnObject,
    'Always' : Always,
    'MouseClicked' : MouseClicked,
    'ObjectClicked' : ObjectClicked,
    'PlayerKeyDown' : PlayerKeyDown,
    'PlayerKeyPressed' : PlayerKeyPressed,
    'KeyDown' : 'is_key_pressed',
    'KeyPressed' : 'is_key_pressed_once(%s)',
    'OnGroupActivation' : OnGroupActivation,
    'FacingInDirection' : FacingInDirection,
    'AnimationPlaying' : 'test_animation',
    'Chance' : 'random_chance',
    'CompareFixedValue' : CompareFixedValue,
    'InsidePlayfield' : InsidePlayfield,
    'OutsidePlayfield' : 'outside_playfield',
    'IsObstacle' : 'test_background_collision',
    'IsOverlappingBackground' : 'overlaps_background',
    'OnBackgroundCollision' : OnBackgroundCollision,
    'PickRandom' : PickRandom,
    'ObjectsInZone' : CompareObjectsInZone,
    'PickObjectsInZone' : PickObjectsInZone,
    'NumberOfObjects' : NumberOfObjects,
    'GroupActivated' : GroupActivated,
    'NotAlways' : NotAlways,
    'AnimationFrame' : make_comparison('get_frame()'),
    'ChannelNotPlaying' : '!media->is_channel_playing(%s-1)',
    'SampleNotPlaying' : '!media->is_sample_playing("%s")',
    'Once' : OnceCondition,
    'Every' : TimerEvery,
    'TimerEquals' : TimerEquals,
    'TimerGreater' : TimerGreater,
    'TimerLess' : TimerLess,
    'IsBold' : 'get_bold',
    'IsItalic' : 'get_italic',
    'MovementStopped' : 'get_movement()->is_stopped()',
    'PathFinished' : PathFinished,
    'NodeReached' : NodeReached,
    'CompareSpeed' : make_comparison('get_movement()->get_speed()'),
    'FlagOn' : 'alterables->flags.is_on',
    'FlagOff' : 'alterables->flags.is_off',
    'NearWindowBorder' : 'is_near_border',
    'AnimationFinished' : AnimationFinished,
    'StartOfFrame' : '.loop_count <= 1',
    'Never' : '.false',
    'NumberOfLives' :  make_comparison('manager->lives'),
    'AnyKeyPressed' : AnyKeyPressed,
    'Repeat' : RepeatCondition,
    'RestrictFor' : RestrictFor,
    # XXX implement this
    'SubApplicationFinished' : FalseCondition,
    'OnLoop' : FalseCondition # if not a generated group, this is always false
})

expressions = make_table(ExpressionMethodWriter, {
    'Speed' : 'get_movement()->get_speed()',
    'String' : StringExpression,
    'ToNumber' : 'string_to_double',
    'ToInt' : 'int',
    'Abs' : 'get_abs',
    'ToString' : ToString,
    'GetRGB' : 'make_color_int',
    'Long' : ValueExpression,
    'Double' : ValueExpression,
    'EndParenthesis' : EndParenthesis,
    'Plus' : PlusExpression,
    'Multiply' : MultiplyExpression,
    'Divide' : DivideExpression,
    'Minus' : MinusExpression,
    'Virgule' : VirguleExpression,
    'Parenthesis' : ParenthesisExpression,
    'Modulus' : '.%math_helper%',
    'AND' : '.&math_helper&',
    'OR' : '.|math_helper|',
    'XOR' : '.^math_helper^',
    'Random' : 'randrange',
    'ApplicationPath' : 'get_app_path()',
    'AlterableValue' : AlterableValueExpression,
    'AlterableValueIndex' : AlterableValueIndexExpression,
    'AlterableStringIndex' : 'alterables->strings.get',
    'AlterableString' : AlterableStringExpression,
    'GlobalString' : GlobalStringExpression,
    'GlobalValue' : GlobalValueExpression,
    'GlobalValueExpression' : '.global_values->get(-1 + ',
    'YPosition' : 'get_y()',
    'XPosition' : 'get_x()',
    'ActionX' : 'get_action_x()',
    'ActionY' : 'get_action_y()',
    'GetParagraph' : 'get_paragraph',
    'ParagraphCount' : 'get_count()',
    'CurrentParagraphIndex' : 'get_index()+1',
    'LoopIndex' : GetLoopIndex,
    'CurrentText' : '.text',
    'XMouse' : 'get_mouse_x()',
    'YMouse' : 'get_mouse_y()',
    'Min' : 'std::min<double>',
    'Max' : 'std::max<double>',
    'Sin' : 'sin_deg',
    'Cos' : 'cos_deg',
    'Exp' : 'get_exp',
    'Log' : 'get_log10',
    'GetAngle' : 'get_angle()',
    'FrameHeight' : '.height',
    'FrameWidth' : '.width',
    'StringLength' : 'string_size',
    'Find' : 'string_find',
    'ReverseFind' : 'string_rfind',
    'LowerString' : 'lowercase_string',
    'UpperString' : 'uppercase_string',
    'RightString' : 'right_string',
    'MidString' : 'mid_string',
    'LeftString' : 'left_string',
    'FixedValue' : 'get_fixed()',
    'AnimationFrame' : 'get_frame()',
    'ObjectLeft' : 'get_box_index(0)',
    'ObjectRight' : 'get_box_index(2)',
    'ObjectTop' : 'get_box_index(1)',
    'ObjectBottom' : 'get_box_index(3)',
    'GetDirection' : 'get_direction()',
    'GetXScale' : '.x_scale',
    'GetYScale' : '.y_scale',
    'Power' : '.*math_helper*',
    'SquareRoot' : 'sqrt',
    'Atan2' : 'atan2d',
    'AlphaCoefficient' : 'blend_color.get_alpha_coefficient()',
    'SemiTransparency' : 'blend_color.get_semi_transparency()',
    'EffectParameter' : 'get_shader_parameter',
    'Floor' : 'get_floor',
    'Round' : 'int_round',
    'AnimationNumber' : 'get_animation()',
    'Ceil' : 'ceil',
    'GetMainVolume' : 'media->get_main_volume()',
    'GetChannelPosition' : '.media->get_channel_position(-1 +',
    'GetSamplePosition' : 'media->get_sample_position',
    'GetSampleDuration' : 'media->get_sample_duration',
    'GetChannelVolume' : '.media->get_channel_volume(-1 +',
    'GetChannelDuration' : '.media->get_channel_duration(-1 + ',
    'ObjectLayer' : '.layer->index+1',
    'NewLine' : '.newline_character',
    'XLeftFrame' : 'frame_left()',
    'XRightFrame' : 'frame_right()',
    'YBottomFrame' : 'frame_bottom()',
    'YTopFrame' : 'frame_top()',
    'ObjectCount' : ObjectCount,
    'CounterMaximumValue' : '.maximum',
    'ApplicationDirectory' : 'get_app_dir()',
    'ApplicationDrive' : 'get_app_drive()',
    'TimerValue' : '.(frame_time * 1000.0)',
    'TimerHundreds' : '.(int(frame_time * 100) % 100)',
    'CounterValue': '.value',
    'CurrentFrame' : '.(index+1)',
    'GetFlag' : 'alterables->flags.get',
    'GetCommandItem' : 'get_command_arg',
    # 1 (standard), 2 (DirectDraw), 4 (Direct3D8) 8 (Direct3D9)
    'DisplayMode' : '.8',
    'GetClipboard' : '.empty_string',
    'TotalObjectCount' : 'get_instance_count()',
    'FrameRate' : '.manager->fps_limit.framerate',
    'TemporaryPath' : 'get_temp_path()',
})
