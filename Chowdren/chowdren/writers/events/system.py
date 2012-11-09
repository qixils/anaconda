from chowdren.writers.events import (ActionWriter, ConditionWriter, 
    ExpressionWriter, ComparisonWriter, ActionMethodWriter, 
    ConditionMethodWriter, ExpressionMethodWriter, make_table,
    make_expression, make_comparison, EmptyAction)
from chowdren.common import get_method_name, to_c
from chowdren.writers.objects import ObjectWriter
from chowdren import shader
from collections import defaultdict

class SystemObject(ObjectWriter):
    def __init__(self, converter):
        self.converter = converter
        self.data = None

    def write_frame(self, writer):
        self.write_group_activated(writer)
        self.write_loops(writer)

    def write_start(self, writer):
        for container, names in self.group_activations.iteritems():
            for name in names:
                writer.putln(to_c('%s = true;', name))

    def write_group_activated(self, writer):
        self.group_activations = defaultdict(list)
        for group in self.converter.always_groups_dict['OnGroupActivation']:
            cond = group.conditions[0]
            container = cond.container
            writer.putln('bool %s;' % (cond.get_group_check()))
            self.group_activations[container].append(cond.get_group_check())

    def write_loops(self, writer):
        loops = defaultdict(list)
        for loop_group in self.get_conditions('OnLoop'):
            exp = loop_group.conditions[0].data.items[0].loader.items[0]
            name = exp.loader.value
            if name == 'Clear Filter':
                # KU-specific hack
                continue
            loops[name].append(loop_group)

        if not loops:
            return

        self.converter.begin_events()
        for name, groups in loops.iteritems():
            loop_name = get_method_name(name)
            writer.putmeth('bool loop_%s' % loop_name)
            self.converter.current_loop_name = name
            for index, group in enumerate(groups):
                self.converter.write_event(writer, group, True)
            writer.putln('return true;')
            writer.end_brace()

        writer.putmeth('bool call_dynamic_loop', 'std::string name')
        for name in loops.keys():
            writer.putln(to_c('if (name == %r) return loop_%s();', 
                name, get_method_name(name)))
        writer.putln('return false;')
        writer.end_brace()

# conditions

class IsOverlapping(ConditionWriter):
    has_object = False

    def write(self, writer):
        data = self.data
        negated = data.otherFlags['Not']
        object_info = data.objectInfo
        other_info = data.items[0].loader.objectInfo
        converter = self.converter
        if negated:
            writer.put(to_c(
                'check_not_overlap(%s, %s)',
                converter.get_object(object_info, True), 
                converter.get_object(other_info, True)))
        else:
            selected_name = converter.get_list_name(converter.get_object_name(
                object_info))
            other_selected = converter.get_list_name(converter.get_object_name(
                other_info))
            writer.put(to_c(
                'check_overlap(%s, %s, %s, %s)',
                converter.get_object(object_info, True), 
                converter.get_object(other_info, True), 
                selected_name, other_selected))
            converter.set_list(object_info, selected_name)
            converter.set_list(other_info, other_selected)

    def is_negated(self):
        return False

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

class TimerEquals(ConditionWriter):
    is_always = True

    def write(self, writer):
        seconds = self.parameters[0].loader.timer / 1000.0
        writer.put('frame_time >= %s' % seconds)

class OnGroupActivation(ConditionWriter):
    custom = True
    def write(self, writer):
        group_check = self.get_group_check()
        writer.putln('if (!%s) %s' % (group_check, self.converter.event_break))
        writer.putln('%s = false;' % group_check)

    def get_group_check(self):
        return 'group_check_%s' % id(self)

class NotAlways(ConditionWriter):
    custom = True
    def write(self, writer):
        event_break = self.converter.event_break
        name = 'not_always_%s' % id(self)
        writer.putln('static unsigned int %s = loop_count;' % name)
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
        name = 'once_condition_%s' % id(self)
        writer.putln('static Frame * %s = NULL;' % name)
        writer.putln('if (%s == this) %s' % (name, event_break))
        writer.putln('%s = this;' % (name))

class GroupActivated(ConditionWriter):
    def write(self, writer):
        container = self.converter.containers[
            self.parameters[0].loader.pointer]
        writer.put(container.code_name)

class PickRandom(ConditionWriter):
    custom = True
    def write(self, writer):
        object_info, object_type = self.get_object()
        converter = self.converter
        selected_name = converter.get_list_name(converter.get_object_name(
            object_info))
        if object_info not in converter.has_selection:
            get_list = converter.get_object(object_info, True)
            writer.putln('%s = %s;' % (selected_name, get_list))
            converter.set_list(object_info, selected_name)
        writer.putln('pick_random(%s);' % selected_name)

class NumberOfObjects(ComparisonWriter):
    iterate_objects = False

    def get_comparison_value(self):
        object_info, object_type = self.get_object()
        return '%s.size()' % self.converter.get_object(object_info, True)

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

# actions

class CreateObject(ActionWriter):
    def write(self, writer):
        is_shoot = False # action_name == 'Shoot'
        details = self.convert_index(0)
        x = str(details['x'])
        y = str(details['y'])
        parent = details.get('parent', None)
        if parent and not is_shoot:
            if details.get('use_action_point', False):
                parent_x = 'get_action_x()'
                parent_y = 'get_action_y()'
            else:
                parent_x = 'x'
                parent_y = 'y'
            if details.get('set_direction', False):
                raise NotImplementedError
                # arguments.append('use_direction = True')
            if details.get('transform_position_direction', False):
                raise NotImplementedError
            if details.get('use_direction', False):
                raise NotImplementedError
            x = '%s->%s + %s' % (parent, parent_x, x)
            y = '%s->%s + %s' % (parent, parent_y, y)
            layer = '%s->layer_index' % parent
        else:
            layer = details['layer']
        if is_shoot:
            create_object = details['shoot_object']
        else:
            create_object = details['create_object']
        arguments = [x, y]
        list_name = self.converter.get_list_name(create_object)
        if is_shoot:
            create_method = '%s->shoot' % self.get_object(
                action.objectInfo)
            arguments.append(str(details['shoot_speed']))
        else:
            create_method = 'create_object'
        writer.put('%s = %s(new %s(%s), %s); // %s' % (
            list_name, create_method, create_object, 
            ', '.join(arguments), layer, details))
        self.converter.has_selection[
            self.parameters[0].loader.objectInfo] = list_name
        if False:#action_name == 'DisplayText':
            paragraph = parameters[1].loader.value
            if paragraph != 0:
                raise NotImplementedError

class SetPosition(ActionWriter):
    def write(self, writer):
        details = self.convert_index(0)
        x = str(details['x'])
        y = str(details['y'])
        parent = details.get('parent', None)
        if parent:
            if details.get('use_action_point', False):
                parent_x = 'get_action_x()'
                parent_y = 'get_action_y()'
            else:
                parent_x = 'x'
                parent_y = 'y'
            x = '%s->%s + %s' % (parent, parent_x, x)
            y = '%s->%s + %s' % (parent, parent_y, y)
            if details.get('set_direction', False):
                raise NotImplementedError
            if details.get('transform_position_direction', False):
                raise NotImplementedError
            if details.get('use_direction', False):
                raise NotImplementedError
        arguments = [x, y]
        writer.put('set_position(%s); // %s' % (
            ', '.join(arguments), details))

class MoveInFront(ActionWriter):
    def write(self, writer):
        object_info = self.parameters[0].loader.objectInfo
        if object_info in self.converter.multiple_instances:
            raise NotImplementedError
        writer.put('move_front(%s);' % (self.converter.get_object(object_info)))

class StartLoop(ActionWriter):
    def write(self, writer):
        real_name = None
        try:
            exp, = self.parameters[0].loader.items[:-1]
            real_name = exp.loader.value
            name = get_method_name(real_name)
            func_call = 'loop_%s()' % name
        except ValueError:
            func_call = 'call_dynamic_loop(name)'
        if real_name == 'Clear Filter':
            self.converter.clear_selection()
            return
        comparison = None
        times = None
        try:
            exp, = self.parameters[1].loader.items[:-1]
            if exp.getName() == 'Long':
                times = exp.loader.value
                if times == -1:
                    comparison = ''
        except ValueError:
            pass
        if times is None:
            times = self.convert_index(1)
        is_infinite = comparison is not None
        if not is_infinite:
            comparison = 'i < times'
        writer.start_brace()
        writer.putln('std::string name = %s;' % self.convert_index(0))
        writer.putln('running_loops[name] = true;')
        if not is_infinite:
            writer.putln('int times = int(%s);' % times)
        writer.putln('for (int i = 0; %s; i++) {' % comparison)
        writer.indent()
        writer.putln('loop_indexes[name] = i;')
        writer.putln('%s;' % func_call)
        writer.putln('if (!running_loops[name]) break;')
        writer.end_brace()
        writer.end_brace()
        self.converter.write_container_check(self.group, writer)
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
        container = self.get_deactivated_container()
        if container in self.converter.container_tree:
            writer.putln('goto %s;' % container.end_label)
        elif container in self.container.tree:
            pass

class StopLoop(ActionWriter):
    def write(self, writer):
        exp, = self.parameters[0].loader.items[:-1]
        name = exp.loader.value
        writer.putln(to_c('running_loops[%r] = false;', name))

class ActivateGroup(ActionWriter):
    def write(self, writer):
        container = self.converter.containers[
            self.parameters[0].loader.pointer]
        writer.put('%s = true;' % container.code_name)
        for name in self.converter.system_object.group_activations[container]:
            writer.putln('%s = true;' % name)

class CenterDisplayY(ActionWriter):
    def write(self, writer):
        writer.put('set_display_center(-1, %s);' % self.convert_index(0))

class EndApplication(ActionWriter):
    def write(self, writer):
        writer.put('has_quit = true;')

class JumpToFrame(ActionWriter):
    def write(self, writer):
        frame = self.parameters[0].loader
        if frame.isExpression:
            value = '%s-1' % self.convert_index(0)
        else:
            value = str(self.converter.game.frameHandles[frame.value])
        writer.put('next_frame = %s;' % value)

class SetEffect(ActionWriter):
    def write(self, writer):
        name = self.parameters[0].loader.value
        writer.put('set_shader(&%s);' % shader.get_name(name))

# expressions

class ValueExpression(ExpressionWriter):
    def get_string(self):
        return to_c('%r', self.data.loader.value)

class ConstantExpression(ExpressionWriter):
    def get_string(self):
        return self.value

class StringExpression(ExpressionWriter):
    def get_string(self):
        return to_c('std::string(%r)', self.data.loader.value)

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
        return 'values->get(%s)' % self.data.loader.value

class AlterableStringExpression(ExpressionWriter):
    def get_string(self):
        return 'strings->get(%s)' % self.data.loader.value

class GlobalValueExpression(ExpressionWriter):
    def get_string(self):
        return 'global_values->get(%s)' % self.data.loader.value

class GlobalStringExpression(ExpressionWriter):
    def get_string(self):
        return 'global_strings->get(%s)' % self.data.loader.value

# class NumberOfObjects(ExpressionWriter):
#     has_object = False
#     def get_string(self):
#         return '%s.size()' % self.converter.get_object(
#             self.data.loader.objectInfo)

actions = make_table(ActionMethodWriter, {
    'CreateObject' : CreateObject,
    'StartLoop' : StartLoop,
    'StopLoop' : StopLoop,
    'SetX' : 'set_x',
    'SetY' : 'set_y',
    'SetAlterableValue' : 'values->set',
    'AddToAlterable' : 'values->add',
    'SubtractFromAlterable' : 'values->sub',
    'SetAlterableString' : 'strings->set',
    'AddCounterValue' : 'add',
    'SetCounterValue' : 'set',
    'SetGlobalString' : 'global_strings->set',
    'SetGlobalValue' : 'global_values->set',
    'AddGlobalValue' : 'global_values->add',
    'SetString' : 'set_string',
    'Hide' : 'set_visible(false)',
    'Show' : 'set_visible(true)',
    'SetParagraph' : 'set_paragraph(%s-1)',
    'LockChannel' : 'media->lock(%s-1)',
    'StopChannel' : 'media->stop_channel(%s-1)',
    'SetChannelPosition' : 'media->set_channel_position(%s-1, %s)',
    'SetChannelPan' : 'media->set_channel_pan(%s-1, %s)',
    'SetChannelVolume' : 'media->set_channel_volume(%s-1, %s)',
    'PlayLoopingChannelFileSample' : 'media->play(%s, %s-1, %s)',
    'SetChannelFrequency' : 'media->set_channel_frequency(%s-1, %s) ',
    'SetDirection' : 'set_direction',
    'SetRGBCoefficient' : 'set_blend_color',
    'SetAngle' : 'set_angle',
    'DeactivateGroup' : DeactivateGroup,
    'ActivateGroup' : ActivateGroup,
    'CenterDisplayY' : CenterDisplayY,
    'EndApplication' : EndApplication,
    'SetPosition' : SetPosition,
    'ExecuteEvaluatedProgram' : 'open_process',
    'HideCursor' : 'set_cursor_visible(false)',
    'ShowCursor' : 'set_cursor_visible(true)',
    'FullscreenMode' : 'set_fullscreen(true)',
    'NextFrame' : '.next_frame = index + 1;',
    'MoveToLayer' : 'set_layer(%s-1)',
    'JumpToFrame' : JumpToFrame,
    'SetAlphaCoefficient' : 'blend_color.set_alpha_coefficient(%s)',
    'SetXScale' : 'set_x_scale({0})',
    'SetYScale' : 'set_y_scale({0})',
    'SetScale' : 'set_scale({0})',
    'ForceAnimation' : 'force_animation',
    'ForceFrame' : 'force_frame',
    'ForceSpeed' : 'force_speed',
    'RestoreFrame' : 'restore_frame',
    'SetEffect' : SetEffect,
    'AddToDebugger' : EmptyAction,
    'SetFrameRate' : 'manager->set_framerate(%s)',
    'Destroy' : 'destroy',
    'BringToBack' : 'move_back',
    'BringToFront' : 'move_front',
    'DeleteAllCreatedBackdrops' : 'layers[%s-1]->destroy_backgrounds()',
    'DeleteCreatedBackdrops' : 'layers[%s-1]->destroy_backgrounds(%s, %s, %s)',
    'SetEffectParameter' : 'set_shader_parameter',
    'SetFrameBackgroundColor' : 'set_background_color',
    'AddBackdrop' : 'paste',
    'PasteActive' : 'paste',
    'MoveInFront' : MoveInFront,
    'ForceDirection' : 'force_direction',
    'StopAnimation' : 'stop_animation',
    'StartAnimation' : 'start_animation',
    'SetMainVolume' : 'media->set_main_volume',
    'StopAllSamples' : 'media->stop_samples',
    'NextParagraph' : 'next_paragraph',
    'PauseApplication' : 'pause'
})

conditions = make_table(ConditionMethodWriter, {
    'CompareAlterableValue' : make_comparison('values->get(%s)'),
    'CompareAlterableString' : make_comparison('strings->get(%s)'),
    'CompareGlobalValue' : make_comparison('global_values->get(%s)'),
    'CompareGlobalString' : make_comparison('global_strings->get(%s)'),
    'CompareX' : make_comparison('x'),
    'CompareY' : make_comparison('y'),
    'Compare' : make_comparison('%s'),
    'IsOverlapping' : IsOverlapping,
    'ObjectVisible' : '.visible',
    'WhileMousePressed' : 'is_mouse_pressed',
    'MouseOnObject' : MouseOnObject,
    'Always' : Always,
    'MouseClicked' : MouseClicked,
    'ObjectClicked' : ObjectClicked,
    'KeyDown' : 'is_key_pressed',
    'KeyPressed' : 'is_key_pressed_once(%s)',
    'OnGroupActivation' : OnGroupActivation,
    'FacingInDirection' : FacingInDirection,
    'AnimationPlaying' : 'test_animation',
    'Chance' : 'random_chance',
    'CompareFixedValue' : make_comparison('get_fixed()'),
    'OutsidePlayfield' : 'outside_playfield',
    'IsObstacle' : 'test_background_collision',
    'IsOverlappingBackground' : 'overlaps_background',
    'PickRandom' : PickRandom,
    'NumberOfObjects' : NumberOfObjects,
    'GroupActivated' : GroupActivated,
    'NotAlways' : NotAlways,
    'AnimationFrame' : make_comparison('get_frame()'),
    'ChannelNotPlaying' : '!media->is_channel_playing(%s-1)',
    'Once' : OnceCondition,
    'TimerEquals' : TimerEquals
})

expressions = make_table(ExpressionMethodWriter, {
    'String' : StringExpression,
    'ToNumber' : 'string_to_double',
    'ToInt' : 'int',
    'Abs' : 'abs',
    'ToString' : 'number_to_string',
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
    'Random' : 'randrange',
    'ApplicationPath' : 'get_app_path()',
    'AlterableValue' : AlterableValueExpression,
    'AlterableValueIndex' : 'values->get',
    'AlterableStringIndex' : 'strings->get',
    'AlterableString' : AlterableStringExpression,
    'GlobalString' : GlobalStringExpression,
    'GlobalValue' : GlobalValueExpression,
    'YPosition' : '.y',
    'XPosition' : '.x',
    'ActionX' : 'get_action_x()',
    'ActionY' : 'get_action_y()',
    'GetParagraph' : 'get_paragraph',
    'CurrentParagraphIndex' : 'get_index()+1',
    'LoopIndex' : 'get_loop_index',
    'CurrentText' : '.text',
    'XMouse' : 'get_mouse_x()',
    'YMouse' : 'get_mouse_y()',
    'Min' : 'std::min<double>',
    'Max' : 'std::max<double>',
    'Sin' : 'sin_deg',
    'Cos' : 'cos_deg',
    'GetAngle' : 'get_angle()',
    'FrameHeight' : '.height',
    'FrameWidth' : '.width',
    'StringLength' : 'string_size',
    'Find' : 'string_find',
    'ReverseFind' : 'string_rfind',
    'LowerString' : 'lowercase_string',
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
    'EffectParameter' : 'get_shader_parameter',
    'Floor' : 'floor',
    'Round' : 'int_round',
    'AnimationNumber' : 'get_animation',
    'Ceil' : 'ceil',
    'GetMainVolume' : 'media->get_main_volume()',
    'GetChannelPosition' : '.media->get_channel_position(-1 +',
    'NewLine' : '.newline_character'
})