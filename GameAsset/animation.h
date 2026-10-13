#ifndef GAME_ANIMATION
#define GAME_ANIMATION

#include "GamePlatform.h"
#include "GameMath.h"

struct Joint_Pose {
    v3 translation;
    quaternion rotation;
    scale 
}

struct Animation {
    string name;
    uint32 duration;
    uint32 nBones;
    float* Content;
};

struct game_animator {
    game_animation* Animation;
    armature* Armature;
    uint32 CurrentFrame;
    bool Active;
    bool Loop;
};

void Update(game_animator* Animator) {
    game_animation* Animation = Animator->Animation;
    armature* Armature = Animator->Armature;

    float* Pointer = Animation->Content + Animator->CurrentFrame * Armature->nBones * 10;
    if(Animator->Active) {
        for (int i = 0; i < Armature->nBones; i++) {
            bone* Bone = &Armature->Bones[i];
            Bone->Transform.Translation.X = *Pointer++;
            Bone->Transform.Translation.Y = *Pointer++;
            Bone->Transform.Translation.Z = *Pointer++;
            Bone->Transform.Rotation.c = *Pointer++;
            Bone->Transform.Rotation.i = *Pointer++;
            Bone->Transform.Rotation.j = *Pointer++;
            Bone->Transform.Rotation.k = *Pointer++;
            Bone->Transform.Scale.X = *Pointer++;
            Bone->Transform.Scale.Y = *Pointer++;
            Bone->Transform.Scale.Z = *Pointer++;
        }

        Animator->CurrentFrame++;
        if (Animator->CurrentFrame >= Animation->nFrames - 1) {
            Animator->CurrentFrame = 0;
            if (!Animator->Loop) Animator->Active = false;
        }
    }
    else {
        for (int i = 0; i < Armature->nBones; i++) {
            bone* Bone = &Armature->Bones[i];
            Bone->Transform.Translation = V3(0,0,0);
            Bone->Transform.Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
            Bone->Transform.Scale = GetScale();
        }
    }
}

void GetAnimationSizes(void* Content, uint32* nFrames, uint32* nBones) {
    tokenizer Tokenizer = InitTokenizer(Content);
    token Token = RequireToken(Tokenizer, Token_Identifier);
    while (Token.Type == Token_Identifier) {
        if (Token == "nF")     *nFrames = Parseuint32(Tokenizer);
        else if (Token == "nB") *nBones = Parseuint32(Tokenizer);
        Token = GetToken(Tokenizer);
    }
}

game_animation LoadAnimation(Arena* Arena, void* FileContent) {
    game_animation Result = {};

    GetAnimationSizes(FileContent, &Result.nFrames, &Result.nBones);

    Result.Content = PushArray(Arena, Result.nFrames * Result.nBones * 10, float);

    tokenizer Tokenizer = InitTokenizer(FileContent);
    AdvanceUntilLine(Tokenizer, 2);

    float* pOut = Result.Content;
    for (uint32 i = 0; i < Result.nFrames; i++) {
        for (uint32 j = 0; j < Result.nBones; j++) {
            uint32 Frame = Parseuint32(Tokenizer);
            uint32 BoneID = Parseuint32(Tokenizer);
            Assert(Frame == i && BoneID == j);

            v3 Translation = ParseV3(Tokenizer);
            quaternion Rotation = ParseQuaternion(Tokenizer);
            v3 Scale = ParseV3(Tokenizer);

            *pOut++ = Translation.X;
            *pOut++ = Translation.Y;
            *pOut++ = Translation.Z;
            *pOut++ = Rotation.c;
            *pOut++ = Rotation.i;
            *pOut++ = Rotation.j;
            *pOut++ = Rotation.k;
            *pOut++ = Scale.X;
            *pOut++ = Scale.Y;
            *pOut++ = Scale.Z;
        }
    }

    return Result;
}

#endif