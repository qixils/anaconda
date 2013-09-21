def write_pre(converter, writer, group):
    if converter.info_dict.get('name') != 'ANNE':
        return
    if group.global_id != 209:
        return
    writer.putln('if (get_instances(BulletShock_60_type).empty()) %s' % (
        converter.event_break))
