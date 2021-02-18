#!/bin/sh
./ffmpeg.exe -f gdigrab -i desktop  -c:v h264_qsv -b:v 4M  -r 8 -vcodec h264_qsv -f h264 udp://224.3.3.10:9999
