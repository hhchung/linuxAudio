#include "audio.h"

void writeWAVHeader(FILE* fout, int32_t sampleRate, int32_t dataSize){
    WAVHeader header;
    int16_t bitsPerSample = 16; //1 sample contain 2byte; 1 frame contain 2 sample
    int16_t numChannels = 2;

    fseek(fout,0, SEEK_SET); 
    strncpy(header.chunkID, "RIFF", 4);
    header.chunkSize = dataSize+36;
    strncpy(header.format, "WAVE", 4);
    
    strncpy(header.subChunk1ID, "fmt ",4);
    header.subChunk1Size = 16; 
    header.audioFormat = 1;  //PCM
    
    header.numChannels = numChannels; //sterio
    header.sampleRate = sampleRate; //CD quality
    header.byteRate = sampleRate * numChannels * bitsPerSample/8;
    header.blockAlign = numChannels * bitsPerSample/8; //1 sample size
    header.bitsPerSample = bitsPerSample;  
   
    strncpy(header.subchunk2ID, "data", 4);
    header.subchunk2Size = dataSize;
    fwrite(&header, 1 , sizeof(header), fout );
}


unsigned long recordRaw(FILE* fout, int32_t sampleRate, unsigned long duration){
    long loops;
    int ret;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buf;
    unsigned long fileSize = 0;
    
    /* Open PCM device for recording (capture). */
    ret = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
    if (ret < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(ret));
        exit(1);
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params,SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, 2);

    val = sampleRate;
    snd_pcm_hw_params_set_rate_near(handle, params,&val, &dir);

    //1 period contain 32 frame
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle,params, &frames, &dir);

    /* Write the parameters to the driver */
    ret = snd_pcm_hw_params(handle, params);
    if (ret < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n",snd_strerror(ret));
        exit(1);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params,&frames, &dir);
    size = frames * 4; //1frame contain 2sample, 1sample contain 2byte
    buf = (char *) malloc(size);

    //get period of time unit: micro second
    snd_pcm_hw_params_get_period_time(params,&val, &dir);
     
    loops = duration *1000000 / val;
    while (loops > 0) {
        loops--;
        ret = snd_pcm_readi(handle, buf, frames);
        if (ret == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        }else if(ret < 0) {
            fprintf(stderr,"error from read: %s\n",snd_strerror(ret));
        }else if(ret != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", ret);
        }
        //ret = write(fd, buffer, size);
        ret = fwrite(buf, 1, size, fout);
        fileSize += ret;
        if (ret != size)
            fprintf(stderr, "short write: wrote %d bytes\n", ret);
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buf);
    return fileSize;
}



unsigned long writeWAVData(FILE *fout, int32_t sampleRate, unsigned long duration){
    fseek(fout,44, SEEK_SET); //move to the data position
    return recordRaw(fout, sampleRate ,duration);
}

void playWAVData(FILE *fin, int32_t sampleRate, unsigned long duration){
    fseek(fin, 44, SEEK_SET);
    playRaw(fin, sampleRate, duration);
}

void playRaw(FILE *fin, int32_t sampleRate, unsigned long duration){
    long loops;
    int ret;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;
    int fd;
    /* Open PCM device for playback. */
    ret = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(ret));
        exit(1);
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, 2);

    val = sampleRate;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

    /* Write the parameters to the driver */
    ret = snd_pcm_hw_params(handle, params);
    if (ret < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(ret));
        exit(1);
    }
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);

    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    //loops = 5000000 / ;
    
    loops = duration * 1000000 / val;
    while (loops > 0) {
        loops--;
        ret = fread(buffer, 1, size, fin);
        if(ret == 0){
            //fprintf(stderr, "end of file on input\n");
            //printf("End of file input\n");
            break;
        }else if(ret != size) {
            fprintf(stderr, "short read: read %d bytes\n", ret);
        }
        ret = snd_pcm_writei(handle, buffer, frames);
        if(ret == -EPIPE){
            /* EPIPE means underrun */
            fprintf(stderr, "underrun occurred\n");
            snd_pcm_prepare(handle);
        }else if (ret < 0){
            fprintf(stderr, "error from writei: %s\n", snd_strerror(ret));
        }else if (ret != (int)frames) {
            fprintf(stderr, "short write, write %d frames\n", ret);
        }
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);


}



