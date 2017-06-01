#ifndef AUDIO_H
#define AUDIO_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

/* All of the ALSA library API is defined
 * in this header */
#include <alsa/asoundlib.h>

typedef struct wavheader{
    
    //RIFF chunk descriptor
    char chunkID[4];
    int32_t chunkSize;
    char format[4];

    //fmt sub-chunk
    char subChunk1ID[4];
    int32_t subChunk1Size;
    int16_t audioFormat;
    int16_t numChannels;
    int32_t sampleRate;
    int32_t byteRate;
    int16_t blockAlign;
    int16_t bitsPerSample;
    
    //data sub-chunk
    char subchunk2ID[4];
    int32_t subchunk2Size;
       
}WAVHeader;


void writeWAVHeader(FILE* fout, int32_t sampleRate, int32_t dataSize);
unsigned long writeWAVData(FILE *fout, int32_t sampleRate, unsigned long duration);

unsigned long recordRaw(FILE* fout, int32_t sampleRate, unsigned long duration);
void playRaw(FILE* fout, int32_t sampleRate, unsigned long duration);
void playWAVData(FILE* fout, int32_t sampleRate, unsigned long duration);


#endif
