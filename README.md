# Simple libasound use
+ Advanced linux sound architectur
+ like command: arecord, aplay
+ `arecord -D plughw:1,0 -d 5 -f dat test.wav`
+ `aplay test.wav`

## Usage
+ `sudo apt install libasound2-dev`
+ `make all`
+ `./audio.o`  //will record 5 seconds
+ `aplay test.wav`
