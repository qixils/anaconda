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
        self.write_loops(writer)

    def write_loops(self, writer):
        loops = defaultdict(list)
        for loop_group in self.get_conditions('OnLoop'):
            exp = loop_group.conditions[0].data.items[0].loader.items[0]
            name = exp.loader.value
            loops[name].append(loop_group)

        if not loops:
            return

        for name, groups in loops.iteritems():
            if self.converter.debug and name not in (
                'Create Inventory X', 
                'Create Inventory Y',
                'Create Ambiance Emitter'):
                continue
            loop_name = get_method_name(name)
            writer.putmeth('bool loop_%s' % loop_name)
            self.converter.current_loop_name = name
            for group in groups:
                self.converter.write_event(writer, group, True)
            writer.putln('return true;')
            writer.end_brace()

        if self.converter.debug:
            return

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
        selected_name = converter.get_list_name(converter.get_object_name(
            object_info))
        other_selected = converter.get_list_name(converter.get_object_name(
            other_info))
        writer.put(to_c(
            'check_overlap(%s, %s, %s, %s, %s)',
            converter.get_object(object_info, True), 
            converter.get_object(other_info, True), 
            selected_name, other_selected,
            negated))
        converter.set_list(object_info, selected_name)
        converter.set_list(other_info, other_selected)

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

# actions

class CreateObject(ActionWriter):
    def write(self, writer):
        is_shoot = False # action_name == 'Shoot'
        details = self.convert_index(0)
        x = str(details['x'])
        y = str(details['y'])
        parent = details.get('parent', None)
        if parent and not is_shoot:
            x = '%s->x + %s' % (parent, x)
            y = '%s->y + %s' % (parent, y)
            layer = '%s->layer_index' % parent
            # arguments.append('parent = %s' % details['parent'])
            # if details.get('use_action_point', False):
            #     arguments.append('use_action = True')
            # if details.get('set_direction', False):
            #     arguments.append('use_direction = True')
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
            x = '%s->x + %s' % (parent, x)
            y = '%s->y + %s' % (parent, y)
            # arguments.append('parent = %s' % details['parent'])
            # if details.get('use_action_point', False):
            #     arguments.append('use_action = True')
            # if details.get('set_direction', False):
            #     arguments.append('use_direction = True')
        arguments = [x, y]
        writer.put('set_position(%s); // %s' % (
            ', '.join(arguments), details))

class StartLoop(ActionWriter):
    def write(self, writer):
        try:
            exp, = self.parameters[0].loader.items[:-1]
            real_name = exp.loader.value
            name = get_method_name(real_name)
            func_call = 'loop_%s()' % name
        except ValueError:
            func_call = 'call_dynamic_loop(name)'
        times = self.convert_index(1)
        writer.start_brace()
        writer.putln('std::string name = %s;' % self.convert_index(0))
        writer.putln('running_loops[name] = true;')
        writer.putln('for (int i = 0; i < %s; i++) {' % times)
        writer.indent()
        writer.putln('loop_indexes[name] = i;')
        writer.putln('%s;' % func_call)
        writer.putln('if (!running_loops[name]) break;')
        writer.end_brace()
        writer.end_brace()

class DeactivateGroup(ActionWriter):
    def write(self, writer):
        container = self.converter.containers[
            self.parameters[0].loader.pointer]
        writer.put('%s = false;' % container.code_name)

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

actions = make_table(ActionMethodWriter, {
    'CreateObject' : CreateObject,
    'StartLoop' : StartLoop,
    'StopLoop' : StopLoop,
    'SetX' : 'set_x',
    'SetY' : 'set_y',
    'SetAlterableValue' : 'values->set',
    'SetAlterableString' : 'strings->set',
    'AddCounterValue' : 'add',
    'SetGlobalString' : 'global_strings->set',
    'SetGlobalValue' : 'global_values->set',
    'AddGlobalValue' : 'global_values->add',
    'SetString' : 'set_string',
    'Hide' : 'set_visible(false)',
    'Show' : 'set_visible(true)',
    'SetParagraph' : 'set_paragraph(%s-1)',
    'LockChannel' : 'media->lock(%s-1)',
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
    'FullscreenMode' : 'set_fullscreen(true)',
    'NextFrame' : '.next_frame = index + 1;',
    'MoveToLayer' : 'set_layer',
    'JumpToFrame' : JumpToFrame,
    'SetAlphaCoefficient' : 'blend_color.set_alpha(%s)',
    'SetXScale' : 'set_x_scale({0})',
    'SetYScale' : 'set_y_scale({0})',
    'ForceAnimation' : 'force_animation',
    'SetEffect' : SetEffect,
    'AddToDebugger' : EmptyAction
})

conditions = make_table(ConditionMethodWriter, {
    'CompareAlterableValue' : make_comparison('values->get(%s)'),
    'CompareAlterableString' : make_comparison('strings->get(%s)'),
    'CompareGlobalValue' : make_comparison('global_values->get(%s)'),
    'CompareGlobalString' : make_comparison('global_strings->get(%s)'),
    'CompareY' : make_comparison('y'),
    'Compare' : make_comparison('%s'),
    'IsOverlapping' : IsOverlapping,
    'ObjectVisible' : '.visible',
    'WhileMousePressed' : 'is_mouse_pressed',
    'MouseOnObject' : MouseOnObject,
    'Always' : Always,
    'MouseClicked' : MouseClicked,
    'ObjectClicked' : ObjectClicked,
    'KeyDown' : 'is_key_pressed'
})

expressions = make_table(ExpressionMethodWriter, {
    'String' : StringExpression,
    'ToNumber' : 'string_to_double',
    'ToInt' : 'int',
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
    'Modulus' : ModulusExpression,
    'Random' : 'randrange',
    'ApplicationPath' : 'get_app_path()',
    'AlterableValue' : AlterableValueExpression,
    'AlterableString' : AlterableStringExpression,
    'GlobalString' : GlobalStringExpression,
    'GlobalValue' : GlobalValueExpression,
    'YPosition' : '.y',
    'XPosition' : '.x',
    'ActionX' : '.get_image()->action_x',
    'GetParagraph' : 'get_paragraph',
    'CurrentParagraphIndex' : 'get_index()+1',
    'LoopIndex' : 'get_loop_index',
    'CurrentText' : '.text',
    'XMouse' : 'get_mouse_x()',
    'Min' : 'std::min<double>',
    'Max' : 'std::max<double>',
    'Sin' : 'sin_deg',
    'GetAngle' : 'get_angle()',
    'FrameHeight' : '.height',
    'StringLength' : 'string_size',
    'Find' : 'string_find',
    'LowerString' : 'lowercase_string',
    'MidString' : 'mid_string'
})