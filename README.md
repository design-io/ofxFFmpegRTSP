# ofxFFmpegRTSP
Wrapper for ffmpeg and rtsp stream server and client. Tested with OF v.11.2 on OSX, Windows 10 and Ubuntu 20.04.

A rtsp server is needed. 
rtsp-simpler-server is a good option. We have tested with v0.19.1. Download the binary for your file system from
https://github.com/aler9/rtsp-simple-server/releases/tag/v0.19.1

Run the server using `./rtsp-simple-server rtsp-simple-server.yml`

Compile and run the server example. 

Comment out `#define OFX_FFMPEG_RTSP_FROM_SOURCE` in ofxFFmpegRTSPUtils.h to load ffmpeg from system level.

FFmpeg 4.4.1 source.
https://github.com/FFmpeg/FFmpeg/releases/tag/n4.4.1

This software uses code of <a href=http://ffmpeg.org>FFmpeg</a> licensed under the <a href=http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>LGPLv2.1</a> and its source can be downloaded <a href=link_to_your_sources>here</a>.
