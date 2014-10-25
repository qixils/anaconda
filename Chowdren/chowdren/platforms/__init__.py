from chowdren.platforms.generic import GenericPlatform

classes = {
    'generic': GenericPlatform
}

try:
    from extra import classes as extra_classes
    classes.update(extra_classes)
except ImportError:
    pass
