bool IsStructType(debug_type Type) { return Type > 38 && Type < 51; }

struct debug_struct_member {
    const char* Name;
    debug_type StructType;
    debug_type MemberType;
    uint64 Size;
    uint64 Offset;
    int ArraySize;
    bool IsPointer;
};

const int STRUCT_MEMBERS_SIZE = 62;
debug_struct_member StructMembers[STRUCT_MEMBERS_SIZE] = {
    {"Translation", Debug_Type_transform, Debug_Type_v3, sizeof(v3), (uint64)(&((transform*)0)->Translation),0, false},
    {"Scale", Debug_Type_transform, Debug_Type_scale, sizeof(scale), (uint64)(&((transform*)0)->Scale),0, false},
    {"Rotation", Debug_Type_transform, Debug_Type_quaternion, sizeof(quaternion), (uint64)(&((transform*)0)->Rotation),0, false},
    {"Head", Debug_Type_segment3, Debug_Type_v3, sizeof(v3), (uint64)(&((segment3*)0)->Head),0, false},
    {"Tail", Debug_Type_segment3, Debug_Type_v3, sizeof(v3), (uint64)(&((segment3*)0)->Tail),0, false},
    {"ID", Debug_Type_game_animation, Debug_Type_game_animation_id, sizeof(game_animation_id), (uint64)(&((game_animation*)0)->ID),0, false},
    {"nFrames", Debug_Type_game_animation, Debug_Type_uint32, sizeof(uint32), (uint64)(&((game_animation*)0)->nFrames),0, false},
    {"nBones", Debug_Type_game_animation, Debug_Type_uint32, sizeof(uint32), (uint64)(&((game_animation*)0)->nBones),0, false},
    {"Content", Debug_Type_game_animation, Debug_Type_float, sizeof(float), (uint64)(&((game_animation*)0)->Content),0, true},
    {"Animation", Debug_Type_game_animator, Debug_Type_game_animation, sizeof(game_animation), (uint64)(&((game_animator*)0)->Animation),0, true},
    {"Armature", Debug_Type_game_animator, Debug_Type_armature, sizeof(armature), (uint64)(&((game_animator*)0)->Armature),0, true},
    {"CurrentFrame", Debug_Type_game_animator, Debug_Type_uint32, sizeof(uint32), (uint64)(&((game_animator*)0)->CurrentFrame),0, false},
    {"Active", Debug_Type_game_animator, Debug_Type_bool, sizeof(bool), (uint64)(&((game_animator*)0)->Active),0, false},
    {"Loop", Debug_Type_game_animator, Debug_Type_bool, sizeof(bool), (uint64)(&((game_animator*)0)->Loop),0, false},
    {"ID", Debug_Type_bone, Debug_Type_int, sizeof(int), (uint64)(&((bone*)0)->ID),0, false},
    {"Name", Debug_Type_bone, Debug_Type_string, sizeof(char), (uint64)(&((bone*)0)->Name),0, false},
    {"Segment", Debug_Type_bone, Debug_Type_segment3, sizeof(segment3), (uint64)(&((bone*)0)->Segment),0, false},
    {"Transform", Debug_Type_bone, Debug_Type_transform, sizeof(transform), (uint64)(&((bone*)0)->Transform),0, false},
    {"nBones", Debug_Type_armature, Debug_Type_uint32, sizeof(uint32), (uint64)(&((armature*)0)->nBones),0, false},
    {"Bones", Debug_Type_armature, Debug_Type_bone, sizeof(bone), (uint64)(&((armature*)0)->Bones),32, false},
    {"Name", Debug_Type_game_entity, Debug_Type_string, sizeof(char), (uint64)(&((game_entity*)0)->Name),0, false},
    {"ID", Debug_Type_game_entity, Debug_Type_int, sizeof(int), (uint64)(&((game_entity*)0)->ID),0, false},
    {"Parent", Debug_Type_game_entity, Debug_Type_game_entity, sizeof(game_entity), (uint64)(&((game_entity*)0)->Parent),0, true},
    {"Index", Debug_Type_game_entity, Debug_Type_int, sizeof(int), (uint64)(&((game_entity*)0)->Index),0, false},
    {"Type", Debug_Type_game_entity, Debug_Type_game_entity_type, sizeof(game_entity_type), (uint64)(&((game_entity*)0)->Type),0, false},
    {"Transform", Debug_Type_game_entity, Debug_Type_transform, sizeof(transform), (uint64)(&((game_entity*)0)->Transform),0, false},
    {"Velocity", Debug_Type_game_entity, Debug_Type_v3, sizeof(v3), (uint64)(&((game_entity*)0)->Velocity),0, false},
    {"Collider", Debug_Type_game_entity, Debug_Type_collider, sizeof(collider), (uint64)(&((game_entity*)0)->Collider),0, false},
    {"Collided", Debug_Type_game_entity, Debug_Type_bool, sizeof(bool), (uint64)(&((game_entity*)0)->Collided),0, false},
    {"Active", Debug_Type_game_entity, Debug_Type_bool, sizeof(bool), (uint64)(&((game_entity*)0)->Active),0, false},
    {"Hovered", Debug_Type_game_entity, Debug_Type_bool, sizeof(bool), (uint64)(&((game_entity*)0)->Hovered),0, false},
    {"HP", Debug_Type_stats, Debug_Type_uint32, sizeof(uint32), (uint64)(&((stats*)0)->HP),0, false},
    {"MaxHP", Debug_Type_stats, Debug_Type_uint32, sizeof(uint32), (uint64)(&((stats*)0)->MaxHP),0, false},
    {"Strength", Debug_Type_stats, Debug_Type_uint32, sizeof(uint32), (uint64)(&((stats*)0)->Strength),0, false},
    {"Defense", Debug_Type_stats, Debug_Type_uint32, sizeof(uint32), (uint64)(&((stats*)0)->Defense),0, false},
    {"Intelligence", Debug_Type_stats, Debug_Type_uint32, sizeof(uint32), (uint64)(&((stats*)0)->Intelligence),0, false},
    {"Wisdom", Debug_Type_stats, Debug_Type_uint32, sizeof(uint32), (uint64)(&((stats*)0)->Wisdom),0, false},
    {"Speed", Debug_Type_stats, Debug_Type_float, sizeof(float), (uint64)(&((stats*)0)->Speed),0, false},
    {"Precission", Debug_Type_stats, Debug_Type_float, sizeof(float), (uint64)(&((stats*)0)->Precission),0, false},
    {"Entity", Debug_Type_enemy, Debug_Type_game_entity, sizeof(game_entity), (uint64)(&((enemy*)0)->Entity),0, true},
    {"Stats", Debug_Type_enemy, Debug_Type_stats, sizeof(stats), (uint64)(&((enemy*)0)->Stats),0, false},
    {"Type", Debug_Type_enemy, Debug_Type_enemy_type, sizeof(enemy_type), (uint64)(&((enemy*)0)->Type),0, false},
    {"MeshID", Debug_Type_enemy, Debug_Type_game_mesh_id, sizeof(game_mesh_id), (uint64)(&((enemy*)0)->MeshID),0, false},
    {"TextureID", Debug_Type_enemy, Debug_Type_game_bitmap_id, sizeof(game_bitmap_id), (uint64)(&((enemy*)0)->TextureID),0, false},
    {"Type", Debug_Type_weapon, Debug_Type_weapon_type, sizeof(weapon_type), (uint64)(&((weapon*)0)->Type),0, false},
    {"Affinity", Debug_Type_weapon, Debug_Type_magic_affinity, sizeof(magic_affinity), (uint64)(&((weapon*)0)->Affinity),0, false},
    {"Color", Debug_Type_weapon, Debug_Type_color, sizeof(color), (uint64)(&((weapon*)0)->Color),0, false},
    {"Entity", Debug_Type_weapon, Debug_Type_game_entity, sizeof(game_entity), (uint64)(&((weapon*)0)->Entity),0, true},
    {"ParentBone", Debug_Type_weapon, Debug_Type_int, sizeof(int), (uint64)(&((weapon*)0)->ParentBone),0, false},
    {"SpellCasting", Debug_Type_weapon, Debug_Type_bool, sizeof(bool), (uint64)(&((weapon*)0)->SpellCasting),0, false},
    {"ID", Debug_Type_character_action, Debug_Type_character_action_id, sizeof(character_action_id), (uint64)(&((character_action*)0)->ID),0, false},
    {"AnimationID", Debug_Type_character_action, Debug_Type_game_animation_id, sizeof(game_animation_id), (uint64)(&((character_action*)0)->AnimationID),0, false},
    {"Loop", Debug_Type_character_action, Debug_Type_bool, sizeof(bool), (uint64)(&((character_action*)0)->Loop),0, false},
    {"Armature", Debug_Type_character, Debug_Type_armature, sizeof(armature), (uint64)(&((character*)0)->Armature),0, false},
    {"Stats", Debug_Type_character, Debug_Type_stats, sizeof(stats), (uint64)(&((character*)0)->Stats),0, false},
    {"Animator", Debug_Type_character, Debug_Type_game_animator, sizeof(game_animator), (uint64)(&((character*)0)->Animator),0, false},
    {"Entity", Debug_Type_character, Debug_Type_game_entity, sizeof(game_entity), (uint64)(&((character*)0)->Entity),0, true},
    {"LeftHand", Debug_Type_character, Debug_Type_weapon, sizeof(weapon), (uint64)(&((character*)0)->LeftHand),0, true},
    {"RightHand", Debug_Type_character, Debug_Type_weapon, sizeof(weapon), (uint64)(&((character*)0)->RightHand),0, true},
    {"Action", Debug_Type_character, Debug_Type_character_action, sizeof(character_action), (uint64)(&((character*)0)->Action),0, false},
    {"Class", Debug_Type_character, Debug_Type_character_class, sizeof(character_class), (uint64)(&((character*)0)->Class),0, false},
    {"KnownSpell", Debug_Type_character, Debug_Type_bool, sizeof(bool), (uint64)(&((character*)0)->KnownSpell),0, false},
};
