def write_pre(converter, writer, group):
    if converter.info_dict.get('name') != 'AnE_ALpha':
        return
    if group.global_id != 310:
        return
    writer.putln('if (get_instances(BulletShock_67_type).empty()) %s' % (
        converter.event_break))
