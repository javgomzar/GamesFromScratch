import bpy # type: ignore
import os
from dataclasses import dataclass
from mathutils import Vector # type: ignore


@dataclass
class Bone:
    id: int
    name: str
    head: Vector
    tail: Vector


def collect_bones(armature) -> dict[str, Bone]:
    result = {}
    exclude = ['Knee', 'Wrist', 'Elbow', 'Heel']
    id = 0
    for bone_name, bone in armature.data.bones.items():
        if bone_name.split("_")[0] not in exclude:
            result[bone_name] = Bone(id, bone_name, bone.head_local, bone.tail_local)
            id += 1
    return result

def export_animation(filepath):
    scene = bpy.context.scene
    armature = bpy.context.object  # Assumes the active object is the armature
    
    if armature is None or armature.type != 'ARMATURE':
        raise Exception("Please select an armature object.")
    
    bones = collect_bones(armature)
    
    start_frame = scene.frame_start
    end_frame = scene.frame_end
    
    lines = []

    lines.append(f"nF {end_frame + 1} nB {len(bones)}")
    
    for frame in range(start_frame, end_frame + 1):
        scene.frame_set(frame)
        
        for pose_bone in armature.pose.bones:
            if pose_bone.bone.name in bones:
                id = bones[pose_bone.bone.name].id

                bone_matrix = pose_bone.matrix.copy() @ pose_bone.bone.matrix_local.inverted()
                translation = bone_matrix.translation
                rotation = bone_matrix.to_quaternion()
                scale = bone_matrix.to_scale()
                
                lines.append(f"{frame} {id} {translation.x} {translation.z} {translation.y} {rotation.w} {rotation.x} {rotation.z} {rotation.y} {scale.x} {scale.z} {scale.y}")        
    
    with open(filepath, 'w') as f:
        f.write("\n".join(lines))
    
    print(f"Animation exported to {filepath} succesfully!")

# Set the output file path
output_path = os.path.join(bpy.path.abspath('//'), ".anim")
export_animation(output_path)