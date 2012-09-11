from chowdren.writers.events import (ActionWriter, ConditionWriter, 
    ExpressionWriter, ComparisonWriter)
from chowdren.common import get_method_name, to_c

# conditions

class CreateObject(ActionWriter):
    def write(self, writer):
        is_shoot = False # action_name == 'Shoot'
        details = self.convert_index(0)
        x = str(details['x'])
        y = str(details['y'])
        parent = details.get('parent', None)
        if parent and not is_shoot:
            x = '%s.x + %s' % (parent, x)
            y = '%s.y + %s' % (parent, y)
            # arguments.append('parent = %s' % details['parent'])
            # if details.get('use_action_point', False):
            #     arguments.append('use_action = True')
            # if details.get('set_direction', False):
            #     arguments.append('use_direction = True')
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
        writer.put('%s = %s(new %s(%s)); // %s' % (
            list_name, create_method, create_object, 
            ', '.join(arguments), details))
        self.converter.has_selection[
            self.parameters[0].loader.objectInfo] = list_name
        if False:#action_name == 'DisplayText':
            paragraph = parameters[1].loader.value
            if paragraph != 0:
                raise NotImplementedError

class CompareAlterableValue(ComparisonWriter):
    value = 'values->get(%s)'

# actions

class StartLoop(ActionWriter):
    def write(self, writer):
        real_name = self.convert_index(0)
        name = get_method_name(real_name)
        times = self.convert_index(1)
        writer.putln('for (int i = 0; i < %s; i++) {' % times, 
            indent = False)
        writer.indent()
        writer.putln('loop_indexes[%s] = i;' % real_name)
        writer.putln('if (!loop_%s()) break;' % name)
        writer.end_brace()

# expressions

class ValueExpression(ExpressionWriter):
    def get_string(self):
        return to_c('%r', self.data.loader.value)

class ConstantExpression(ExpressionWriter):
    def get_string(self):
        return self.value

class EndParenthesis(ConstantExpression):
    value = ')'

class PlusExpression(ConstantExpression):
    value = '+'

class MinusExpression(ConstantExpression):
    value = '-'

class MultiplyExpression(ConstantExpression):
    value = '*'

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

# method writers

class ActionMethodWriter(ActionWriter):
    def write(self, writer):
        parameters = [str(self.convert_parameter(item)) 
            for item in self.parameters]
        out = '%s(%s);' % (self.method, ', '.join(parameters))
        writer.put(out)

class ConditionMethodWriter(ConditionWriter):
    def write(self, writer):
        parameters = [str(self.convert_parameter(item)) 
            for item in self.parameters]
        out = '%s(%s)' % (self.method, ', '.join(parameters))
        writer.put(out)

class ExpressionMethodWriter(ExpressionWriter):
    def get_string(self):
        return '%s(' % self.method

def make_table(method_writer, table):
    new_table = {}
    for k, v in table.iteritems():
        if isinstance(v, str):
            class NewWriter(method_writer):
                method = v
            v = NewWriter
        new_table[k] = v
    return new_table

actions = make_table(ActionMethodWriter, {
    'CreateObject' : CreateObject,
    'StartLoop' : StartLoop,
    'SetX' : 'set_x',
    'SetY' : 'set_y',
    'SetAlterableValue' : 'values->set'
})

conditions = make_table(ConditionMethodWriter, {
    'CompareAlterableValue' : CompareAlterableValue
})

expressions = make_table(ExpressionMethodWriter, {
    'String' : ValueExpression,
    'Long' : ValueExpression,
    'Double' : ValueExpression,
    'EndParenthesis' : EndParenthesis,
    'Plus' : PlusExpression,
    'Multiply' : MultiplyExpression,
    'Minus' : MinusExpression,
    'Virgule' : VirguleExpression,
    'Random' : 'randrange',
    'AlterableValue' : AlterableValueExpression
})