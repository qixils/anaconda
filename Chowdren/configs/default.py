def init(converter):
    pass

def init_container(converter, container):
    pass

def write_pre(converter, writer, group):
    pass

def write_frame_post(converter, writer):
    pass

def use_simple_or(converter):
    return False

def use_iteration_index(converter):
    return True

def use_global_alterables(converter, obj):
    return False

def use_single_global_alterables(converter, obj):
    return True

def use_global_int(converter, expression):
    return False

def use_alterable_int(converter, expression):
    return False

def use_safe_division(converter):
    return True

def get_startup_instances(converter, instances):
    return instances

def use_safe_create(converter):
    return False

def use_global_instances(converter):
    return True

def use_update_filtering(converter):
    return False

def use_image_flush(converter, frame):
    return True

def use_image_preload(converter):
    return False

def add_defines(converter):
    pass

def get_frames(converter, game, frames):
    return frames

def get_depth(converter, layer):
    return None

def get_object_depth(converter, obj):
    return None

def get_loop_name(converter, parameter):
    return None

def get_loop_call_names(converter, name):
    return None

def get_fonts(converter):
    return ()