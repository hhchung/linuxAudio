#include <stdio.h>
#include <stdlib.h>
#include "audio.h"
int main(int argc, char *argv[]){
    unsigned long fileSize;
    FILE *fout = fopen("test.wav", "wb");
    if(fout == NULL){
        perror("fopen");
        exit(-1);
    }
    fileSize = writeWAVData(fout, 44100, 5);
    writeWAVHeader(fout, 44100, fileSize); 
    fclose(fout);

    FILE *fin = fopen("test.wav", "rb");
    if(fin == NULL){
        perror("fopen");
        exit(-1);
    }
    playWAVData(fin, 44100, 5);
    return 0;
}
