// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"
#include <fcntl.h>

ScreenRecorder::ScreenRecorder(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("ScreenRecorder");

    subComponents = vector<HardwareComponent *> {
        
        &muxer
    };
    
    muxer.setDescription("RecMuxer");
}

bool
ScreenRecorder::hasFFmpeg()
{
    return getSizeOfFile(ffmpegPath()) > 0;
}

void
ScreenRecorder::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
ScreenRecorder::_dump()
{
    msg("%s:%s installed\n", ffmpegPath(), hasFFmpeg() ? "" : " not");
    msg("Video pipe:%s created\n", videoPipe != -1 ? "" : " not");
    msg("Audio pipe:%s created\n", audioPipe != -1 ? "" : " not");
}
    
bool
ScreenRecorder::startRecording(int x1, int y1, int x2, int y2,
                               long bitRate,
                               long aspectX,
                               long aspectY)
{
    if (isRecording()) return false;

    // Create pipes
    debug(REC_DEBUG, "Creating pipes...\n");

    unlink(videoPipePath());
    unlink(audioPipePath());
    if (mkfifo(videoPipePath(), 0666) == -1) return false;
    if (mkfifo(audioPipePath(), 0666) == -1) return false;
        
    debug(REC_DEBUG, "Pipes created\n");
    dump();
    
    synchronized {
        
        // Make sure the screen dimensions are even
        if ((x2 - x1) % 2) x2--;
        if ((y2 - y1) % 2) y2--;
        cutout.x1 = x1;
        cutout.x2 = x2;
        cutout.y1 = y1;
        cutout.y2 = y2;
        debug("Recorded area: (%d,%d) - (%d,%d)\n", x1, y1, x2, y2);
        
        //
        // Assemble the command line arguments for the video encoder
        //
        
        char cmd1[512]; char *ptr = cmd1;
        
        // Path to the FFmpeg executable
        ptr += sprintf(ptr, "%s -nostdin", ffmpegPath());
        
        // Verbosity
        ptr += sprintf(ptr, " -loglevel %s", loglevel());
        
        // Input stream format
        ptr += sprintf(ptr, " -f:v rawvideo -pixel_format rgba");
        
        // Frame rate
        ptr += sprintf(ptr, " -r %d", frameRate);
        
        // Frame size (width x height)
        ptr += sprintf(ptr, " -s:v %dx%d", x2 - x1, y2 - y1);
        
        // Input source (named pipe)
        ptr += sprintf(ptr, " -i %s", videoPipePath());
        
        // Output stream format
        ptr += sprintf(ptr, " -f mp4 -pix_fmt yuv420p");
        
        // Bit rate
        ptr += sprintf(ptr, " -b:v %ldk", bitRate);
        
        // Aspect ratio
        ptr += sprintf(ptr, " -bsf:v ");
        ptr += sprintf(ptr, "\"h264_metadata=sample_aspect_ratio=");
        ptr += sprintf(ptr, "%ld/%ld\"", aspectX, 2*aspectY);
        
        // Output file
        ptr += sprintf(ptr, " -y %s", videoStreamPath());
        
        
        //
        // Assemble the command line arguments for the audio encoder
        //
        
        char cmd2[512]; ptr = cmd2;
        
        // Path to the FFmpeg executable
        ptr += sprintf(ptr, "%s -nostdin", ffmpegPath());
        
        // Verbosity
        ptr += sprintf(ptr, " -loglevel %s", loglevel());
        
        // Audio format and number of channels
        ptr += sprintf(ptr, " -f:a f32le -ac 2");
        
        // Sampling rate
        ptr += sprintf(ptr, " -sample_rate %d", sampleRate);
        
        // Input source (named pipe)
        ptr += sprintf(ptr, " -i %s", audioPipePath());
        
        // Output stream format
        ptr += sprintf(ptr, " -f mp4");
        
        // Output file
        ptr += sprintf(ptr, " -y %s", audioStreamPath());
        
        //
        // Launch FFmpeg instances
        //
        
        assert(videoFFmpeg == NULL);
        assert(audioFFmpeg == NULL);
        
        msg("\nStarting video encoder with options:\n%s\n", cmd1);
        videoFFmpeg = popen(cmd1, "w");
        msg(videoFFmpeg ? "Success\n" : "Failed to launch\n");
        
        msg("\nStarting audio encoder with options:\n%s\n", cmd2);
        audioFFmpeg = popen(cmd2, "w");
        msg(audioFFmpeg ? "Success\n" : "Failed to launch\n");
        
        // Open pipes
        videoPipe = open(videoPipePath(), O_WRONLY);
        audioPipe = open(audioPipePath(), O_WRONLY);
        
        recording = videoFFmpeg && audioFFmpeg && videoPipe != -1 && audioPipe != -1;
    }

    if (isRecording()) {
        messageQueue.put(MSG_RECORDING_STARTED);
        return true;
    }
    
    return false;
}

void
ScreenRecorder::stopRecording()
{
    debug(REC_DEBUG, "stopRecording()\n");
    
    if (!isRecording()) return;
    
    synchronized {
        recording = false;
        recordCounter++;
    }

    // Close pipes
    close(videoPipe);
    close(audioPipe);
    videoPipe = -1;
    audioPipe = -1;
     
    // Shut down encoders
    pclose(videoFFmpeg);
    pclose(audioFFmpeg);
    videoFFmpeg = NULL;
    audioFFmpeg = NULL;

    debug(REC_DEBUG, "Recording has stopped\n");
    messageQueue.put(MSG_RECORDING_STOPPED);
}

bool
ScreenRecorder::exportAs(const char *path)
{
    if (isRecording()) return false;
    
    //
    // Assemble the command line arguments for the video encoder
    //
    
    char cmd[512]; char *ptr = cmd;
    
    // Path to the FFmpeg executable
    ptr += sprintf(ptr, "%s", ffmpegPath());
    
    // Verbosity
    ptr += sprintf(ptr, " -loglevel %s", loglevel());

    // Input streams
    ptr += sprintf(ptr, " -i %s", videoStreamPath());
    ptr += sprintf(ptr, " -i %s", audioStreamPath());

    // Don't reencode
    ptr += sprintf(ptr, " -c:v copy -c:a copy");
    
    // Output file
    ptr += sprintf(ptr, " -y %s", path);
    
    //
    // Launch FFmpeg
    //
    
    msg("\nMerging video and audio stream with options:\n%s\n", cmd);
    FILE *ffmpeg = popen(cmd, "w");

    if (!ffmpeg) {
        msg("Failed to launch\n");
        return false;
    }
    
    // Wait for FFmpeg to finish
    fclose(ffmpeg);
    
    return true;
}

void
ScreenRecorder::vsyncHandler(Cycle target)
{
    if (!isRecording()) return;
    
    // debug("vsyncHandler\n");
    assert(videoFFmpeg != NULL);
    assert(audioFFmpeg != NULL);
    
    synchronized {
        
        //
        // Video
        //
        
        ScreenBuffer buffer = denise.pixelEngine.getStableBuffer();
        
        int width = sizeof(u32) * (cutout.x2 - cutout.x1);
        int height = cutout.y2 - cutout.y1;
        int offset = cutout.y1 * HPIXELS + cutout.x1 + HBLANK_MIN * 4;
        u8 *data = new u8[width * height];
        u8 *src = (u8 *)(buffer.data + offset);
        u8 *dst = data;
        for (int y = 0; y < height; y++, src += 4 * HPIXELS, dst += width) {
            memcpy(dst, src, width);
        }
        
        // Feed the video pipe
        assert(videoPipe != -1);
        write(videoPipe, data, (size_t)(width * height));
        
        //
        // Audio
        //
        
        // Clone Paula's muxer contents
        muxer.sampler[0] = paula.muxer.sampler[0];
        muxer.sampler[1] = paula.muxer.sampler[1];
        muxer.sampler[2] = paula.muxer.sampler[2];
        muxer.sampler[3] = paula.muxer.sampler[3];
        assert(muxer.sampler[0].r == paula.muxer.sampler[0].r);
        assert(muxer.sampler[0].w == paula.muxer.sampler[0].w);
        
        // Synthesize audio samples for this frame
        if (audioClock == 0) audioClock = target-1;
        muxer.synthesize(audioClock, target, samplesPerFrame);
        audioClock = target;
        
        // Copy samples to buffer
        float *samples = new float[2 * samplesPerFrame];
        muxer.copyInterleaved(samples, samplesPerFrame);
        
        // Feed the audio pipe
        assert(audioPipe != -1);
        write(audioPipe, (u8 *)samples, (size_t)(2 * sizeof(float) * samplesPerFrame));
    }
}
