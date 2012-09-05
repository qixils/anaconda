from collections import defaultdict, Counter
from chowdrencommon import COMPARISONS

class BaseWriter(object):
    def __init__(self, converter, data):
        self.converter = converter
        self.data = data

        self.convert_parameter = converter.convert_parameter

    def convert_index(self, index):
        return self.convert_parameter(self.parameters[index])

class ACBase(BaseWriter):
    def __init__(self, *arg, **kw):
        BaseWriter.__init__(self, *arg, **kw)
        self.parameters = self.data.items

    iterate_objects = None

class ActionWriter(ACBase):
    def write(self, writer):
        raise NotImplementedError()

class ConditionWriter(ACBase):
    negate = False
    def write(self, writer):
        raise NotImplementedError()

class ExpressionWriter(BaseWriter):
    def get_string(self):
        raise NotImplementedError()

# helper writers

class ComparisonWriter(ConditionWriter):
    def write(self, writer):
        comparison = COMPARISONS[self.parameters[-1].loader.comparison]
        parameters = [str(self.convert_parameter(parameter))
            for parameter in self.parameters]
        if len(parameters) == 1:
            value1 = self.value
            value2, = parameters[0]
        elif len(parameters) == 2:
            value1 = self.value % parameters[0]
            value2 = parameters[1]
        else:
            raise NotImplementedError
        writer.put('%s %s %s' % (value1, comparison, value2))

# debug writers

def print_parameters(ace):
    print [parameter.getName() for parameter in ace.items]

def write_default(self, name, type, writer):
    if name not in self.checked:
        print ('%s %r not implemented' % (type, name)),
        print_parameters(self.data)
    self.checked[name] += 1
    debug_parameters = [str(self.convert_parameter(parameter))
        for parameter in self.parameters]
    debug_name = '%s(%s)' % (name, ', '.join(debug_parameters))
    writer.put(debug_name)

class DefaultAction(ActionWriter):
    checked = Counter()
    def write(self, writer):
        name = self.converter.get_action_name(self.data)
        write_default(self, name, 'action', writer)

class DefaultCondition(ConditionWriter):
    checked = Counter()
    def write(self, writer):
        name = self.converter.get_condition_name(self.data)
        write_default(self, name, 'condition', writer)

class DefaultExpression(ExpressionWriter):
    checked = Counter()
    def get_string(self):
        name = self.converter.get_expression_name(self.data)
        if name not in self.checked:
            print 'expression not implemented:', name, self.data.loader
        self.checked[name] += 1
        return '%s(' % name

default_writers = {
    'actions' : DefaultAction,
    'conditions' : DefaultCondition,
    'expressions' : DefaultExpression
}