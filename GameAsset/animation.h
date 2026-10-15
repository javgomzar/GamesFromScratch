#ifndef GAME_ANIMATION
#define GAME_ANIMATION

#include "GamePlatform.h"
#include "GameMath.h"

typedef int32 Joint_ID;

struct Joint {
    Joint_ID id;
    String name;
    Joint_ID parent;
    Matrix4 inverse_bind;
    Matrix4 rest_pose;
    Transform local_pose;
};

struct Armature {
    Asset_ID id;
    Joint_ID root_joint;
    array<Joint> joints;
    Matrix4 global_transform;
};

struct Animation {
    Asset_ID id;
    String name;
    uint32 duration;
    uint32 nBones;
    float* Content;
};

struct Animator {
    Animation* Animation;
    Armature* Armature;
    uint32 CurrentFrame;
    bool Active;
    bool Loop;
};

// void Update(Animator* animator) {
//     Animation* animation = Animator->Animation;
//     armature* armature = Animator->Armature;

//     float* Pointer = Animation->Content + Animator->CurrentFrame * Armature->nBones * 10;
//     if(Animator->Active) {
//         for (int i = 0; i < Armature->nBones; i++) {
//             bone* Bone = &Armature->Bones[i];
//             Bone->Transform.Translation.X = *Pointer++;
//             Bone->Transform.Translation.Y = *Pointer++;
//             Bone->Transform.Translation.Z = *Pointer++;
//             Bone->Transform.Rotation.c = *Pointer++;
//             Bone->Transform.Rotation.i = *Pointer++;
//             Bone->Transform.Rotation.j = *Pointer++;
//             Bone->Transform.Rotation.k = *Pointer++;
//             Bone->Transform.Scale.X = *Pointer++;
//             Bone->Transform.Scale.Y = *Pointer++;
//             Bone->Transform.Scale.Z = *Pointer++;
//         }

//         Animator->CurrentFrame++;
//         if (Animator->CurrentFrame >= Animation->nFrames - 1) {
//             Animator->CurrentFrame = 0;
//             if (!Animator->Loop) Animator->Active = false;
//         }
//     }
//     else {
//         for (int i = 0; i < Armature->nBones; i++) {
//             bone* Bone = &Armature->Bones[i];
//             Bone->Transform.Translation = V3(0,0,0);
//             Bone->Transform.Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
//             Bone->Transform.Scale = GetScale();
//         }
//     }
// }

// void GetAnimationSizes(void* Content, uint32* nFrames, uint32* nBones) {
//     Tokenizer tokenizer = Inittokenizer(Content);
//     token Token = RequireToken(tokenizer, Token_Identifier);
//     while (Token.Type == Token_Identifier) {
//         if (Token == "nF")     *nFrames = Parseuint32(tokenizer);
//         else if (Token == "nB") *nBones = Parseuint32(tokenizer);
//         Token = GetToken(tokenizer);
//     }
// }

// Animation LoadAnimation(Arena* arena, void* FileContent) {
//     Animation result = {};

//     GetAnimationSizes(FileContent, &result.nFrames, &result.nBones);

//     result.Content = PushArray(Arena, result.nFrames * result.nBones * 10, float);

//     Tokenizer tokenizer = Inittokenizer(FileContent);
//     AdvanceUntilLine(tokenizer, 2);

//     float* pOut = result.Content;
//     for (uint32 i = 0; i < result.nFrames; i++) {
//         for (uint32 j = 0; j < result.nBones; j++) {
//             uint32 Frame = Parseuint32(tokenizer);
//             uint32 BoneID = Parseuint32(tokenizer);
//             Assert(Frame == i && BoneID == j);

//             v3 Translation = ParseV3(tokenizer);
//             quaternion Rotation = ParseQuaternion(tokenizer);
//             v3 Scale = ParseV3(tokenizer);

//             *pOut++ = Translation.X;
//             *pOut++ = Translation.Y;
//             *pOut++ = Translation.Z;
//             *pOut++ = Rotation.c;
//             *pOut++ = Rotation.i;
//             *pOut++ = Rotation.j;
//             *pOut++ = Rotation.k;
//             *pOut++ = Scale.X;
//             *pOut++ = Scale.Y;
//             *pOut++ = Scale.Z;
//         }
//     }

//     return result;
// }

#endif