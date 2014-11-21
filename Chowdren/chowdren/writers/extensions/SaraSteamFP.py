from chowdren.writers.objects import ObjectWriter

from chowdren.common import get_animation_name, to_c, make_color

from chowdren.writers.events import (ConditionMethodWriter,
    ActionMethodWriter, ExpressionMethodWriter, make_table,
    TrueCondition, FalseCondition, EmptyAction)

class SteamObject(ObjectWriter):
    static = True
    update = False

    def write_init(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
    0 : EmptyAction, # sarasteamfp_general_frame_update_0
    1 : EmptyAction, # sarasteamfp_leaderboards_uploading_find_boards_1
    2 : EmptyAction, # sarasteamfp_leaderboards_uploading_upload_crystals_2
    3 : EmptyAction, # sarasteamfp_leaderboards_uploading_upload_time_3
    6 : EmptyAction, # sarasteamfp_achievements_simple_unlock_6
    7 : EmptyAction, # sarasteamfp_achievements_simple_lock_7
    11 : EmptyAction, # sarasteamfp_steam_cloud_simple_upload_records_dat_11
    13 : EmptyAction, # sarasteamfp_steam_cloud_simple_upload_gamepad_cfg_13
    14 : EmptyAction, # sarasteamfp_steam_cloud_simple_upload_keyboard_cfg_14
    15 : EmptyAction, # sarasteamfp_steam_cloud_simple_download_records_dat_15
    17 : EmptyAction, # sarasteamfp_steam_cloud_simple_download_gamepad_cfg_17
    18 : EmptyAction, # sarasteamfp_steam_cloud_simple_download_keyboard_cfg_18
    27 : EmptyAction, # sarasteamfp_steam_cloud_simple_upload_adventure_27
    29 : EmptyAction # sarasteamfp_steam_cloud_simple_download_adventure_29

})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return SteamObject