// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"
#include "Muxer.h"

class ScreenRecorder : public AmigaComponent {

    //
    // Constants
    //
    
    // Path to the FFmpeg executable
    static const char *ffmpegPath() { return "/usr/local/bin/ffmpeg"; }

    // Path to the two named input pipes
    static const char *videoPipePath() { return "/tmp/videoPipe"; }
    static const char *audioPipePath() { return "/tmp/audioPipe"; }

    // Path to the two temporary output files
    static const char *videoStreamPath() { return "/tmp/video.mp4"; }
    static const char *audioStreamPath() { return "/tmp/audio.mp4"; }

    // Audio sample frequency in the output stream
    static const int frameRate = 50;
    static const int sampleRate = 44100;
    static const int samplesPerFrame = sampleRate / frameRate;

    // Log level passed to FFmpef
    static const char *loglevel() { return REC_DEBUG ? "verbose" : "warning"; }
    
    
    //
    // Sub components
    //
    
    // Audio muxer for synthesizing the audio track
    Muxer muxer = Muxer(amiga);
    

    //
    // Handles
    //
    
    // File handles to access FFmpeg
    FILE *videoFFmpeg = nullptr;
    FILE *audioFFmpeg = nullptr;

    // Video and audio pipe
    int videoPipe = -1;
    int audioPipe = -1;

    
    //
    // Recording status
    //
    
    // Indicates if a video is being recorded
    bool recording = false;

    // Number of records that have been made
    long recordCounter = 0;
    
    // Audio has been recorded up to this cycle
    Cycle audioClock = 0;

    
    //
    // Recording parameters
    //
    
    // The texture cutout that is going to be recorded
    struct { int x1; int y1; int x2; int y2; } cutout;

    // Bitrate passed to FFmpeg
    long bitRate = 0;
    
    // Pixel aspect ratio
    long aspectX;
    long aspectY;
            
    
    //
    // Initializing
    //
    
public:
    
    ScreenRecorder(Amiga& ref);
    
    const char *getDescription() const override { return "ScreenRecorder"; }

    bool hasFFmpeg() const;
    
    void _reset(bool hard) override;

    
    //
    // Analyzing
    //

private:
    
    void _dump() const override;
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Preparing to record a video stream
    //
    
public:
    
    // Sets the target file name
    bool setPath(const char *path);
    
    
    //
    // Starting and stopping a video stream
    //
    
public:
        
    // Checks whether the screen is currently recorded
    bool isRecording() { return recording; }
    
    // Returns the record counter
    long getRecordCounter() { return recordCounter; }
    
    // Starts the screen recorder
    bool startRecording(int x1, int y1, int x2, int y2,
                        long bitRate,
                        long aspectX,
                        long aspectY);

    // Stops the screen recorder
    void stopRecording();

    // Exports the recorded video
    bool exportAs(const char *path);
    
    
    //
    // Recording a video stream
    //

public:
    
    // Writes an audio sample to the temporary buffer
    // void addSample(float left, float right);
    
    // Records a single frame
    void vsyncHandler(Cycle target);
};
