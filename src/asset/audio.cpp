#include "pch.h"
#include "audio.h"

/*
    TODO:
        - Mixer
        - Some effects (low pass, high pass)
        - Volume control
        - Implement circular buffer with VirtualAlloc2 and MapViewOfFile3
*/


bool LoadSound(void* content, Sound* sound) {
    unsigned long* pointer = (unsigned long*)content;

    unsigned long chunk_type = *pointer++;
    if (chunk_type != 'FFIR') {
        return false;
    }

    unsigned long riff_chunk_size = *pointer++;
    unsigned long file_type = *pointer++;
    if (file_type != 'EVAW') {
        return false;
    }

    chunk_type = *pointer++;
    if (chunk_type != ' tmf') {
        return false;
    }
    unsigned long chunk_size = *pointer++;
    sound->wave_format = *(Wave_Format*)pointer;

    pointer += 4;
    chunk_type = *pointer++;
    if (chunk_type != 'atad') {
        return false;
    }
    chunk_size = *pointer++;

    sound->sample_count = chunk_size / 2;
    sound->sample_out = (int16*)pointer;

    return true;
}

float WriteSineWave(Sound_Buffer* sound_buffer, float hz, float initial_phase) {
    uint32 sample_count = sound_buffer->size;
    float sine_wave_period = (float)(sound_buffer->samples_per_second) / hz;
    float phase_increment = TAU / (float)sine_wave_period;

    float phase = initial_phase;
    int16* sample_out = sound_buffer->sample_out;
    for (uint32 sample_index = 0; sample_index < sample_count; sample_index++) {
        float sine_value = sin(phase);
        phase += phase_increment;
        while (phase > TAU) {
            phase -= TAU;
        }
        uint16 SampleValue = (uint16)(sine_value * AMPLITUDE);
        *sample_out++ = SampleValue; // LEFT
        *sample_out++ = SampleValue; // RIGHT
    }
    return phase;
}

void FadeIn(Sound_Buffer* sound_buffer, uint8 transition_count) {
    uint32 sample_count = sound_buffer->size;
    uint16 half_life = (uint16)(sample_count / 2);
    int16* sample_out = sound_buffer->sample_out;
    for (uint32 sample_index = 0; sample_index < sample_count; sample_index++) {
        float exp = (1 - expf(-(float)((sample_index + transition_count * sample_count) / (float)half_life)));
        float left_value = (float)*sample_out * exp;
        *sample_out = (uint16)(left_value); // LEFT
        sample_out++;
        float right_value = (float)*sample_out * exp;
        *sample_out = (uint16)(right_value); // RIGHT
        sample_out++;
    }
}

void FadeOut(Sound_Buffer* sound_buffer, uint8 transition_count) {
    uint32 sample_count = sound_buffer->size;
    uint16 half_life = (uint16)(sample_count / 2);
    int16* sample_out = sound_buffer->sample_out;
    for (uint32 sample_index = 0; sample_index < sample_count; sample_index++) {
        float exp = expf(-(float)((sample_index + transition_count * sample_count) / (float)half_life));
        float left_value = (float)*sample_out * exp;
        *sample_out = (uint16)(left_value); // LEFT
        sample_out++;
        float right_value = (float)*sample_out * exp;
        *sample_out = (uint16)(right_value); // RIGHT
        sample_out++;
    }
}

void Silence(Sound_Buffer* sound_buffer) {
    uint32 sample_count = sound_buffer->size;

    int16* sample_out = sound_buffer->sample_out;
    for (uint32 sample_index = 0; sample_index < sample_count; sample_index++) {
        *sample_out++ = 0; // LEFT
        *sample_out++ = 0; // RIGHT
    }
}

//void DebugPlotSoundBuffer(game_offscreen_buffer* pScreenBuffer, game_sound_buffer* sound_buffer, v3 PlotOrigin) {
//    uint32 sample_count = sound_buffer->size;
//    uint16 PlotHeight = 200;
//    uint16 PlotWidth = 400;
//    uint16 Amplitude = 4000;
//
//    int16* sample_out = sound_buffer->sample_out;
//    for (int i = 0; i < sample_count; i++) {
//        v3 Position = { PlotOrigin.X + (i * PlotWidth / sample_count), (uint32)((*sample_out * PlotHeight / Amplitude) + PlotOrigin.Y), 0 };
//        Plot(pScreenBuffer, Position, White);
//        sample_out++;
//        sample_out++;
//    }
//}

void PlayGameSound(Sound* sound, Sound_Buffer* sound_buffer) {
    if (sound->played + sound_buffer->size > sound->sample_count) {
        sound->played = 0;
    }

    uint32 sample_count = sound_buffer->size;
    int16* sample_out = sound_buffer->sample_out;
    int16* sample_in = sound->sample_out + sound->played;
    for (uint32 sample_index = 0; sample_index < sample_count; sample_index++) {
        *sample_out++ = *sample_in++; // LEFT
        *sample_out++ = *sample_in++; // RIGHT
    }

    sound->played += 2 * sound_buffer->size;
}
