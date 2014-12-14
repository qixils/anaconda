from mmfparser.gperf import get_hash_function
from chowdren.codewriter import CodeWriter

def get_string_int_map(map_func, hash_func, string_map, case_sensitive=True):
    strings = list(string_map.iterkeys())
    hash_data = get_hash_function(hash_func, strings, case_sensitive)
    writer = CodeWriter()
    writer.putlnc(hash_data.code)
    writer.putmeth('int %s' % map_func, 'const std::string & in')
    writer.putlnc('unsigned int hash = %s(&in[0], in.size());',
                  hash_func)
    hashes = dict((v, k) for (k, v) in hash_data.strings.iteritems())
    writer.putlnc('switch (hash) {')
    writer.indent()
    for i in xrange(hash_data.max_hash_value + 1):
        value = hashes.get(i, None)
        if value is None:
            value = '-1'
        else:
            value = string_map[value]
        writer.putlnc('case %s: return %s;', i, value)
    writer.end_brace()
    writer.putlnc('return -1;')
    writer.end_brace()
    return writer.get_data()