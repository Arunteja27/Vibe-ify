#ifndef AUDIONODE_H
#define AUDIONODE_H

#include <string>
#include <iostream>

using namespace std;

// Abstract base class for every element in the audio pipeline.
// Defines the polymorphic interface that the AudioEngine dispatches through.
class AudioNode {
public:
    virtual ~AudioNode() {}

    // Process audio: fill 'buffer' with 'numFrames' samples (interleaved stereo).
    // Returns the number of frames actually written (0 when finished).
    virtual int process(float* buffer, int numFrames) = 0;

    // Human-readable node type (e.g. "Track", "Playlist", "VolumeEffect")
    virtual const char* getType() const = 0;

    // Deep-copy the node (caller owns the returned pointer)
    virtual AudioNode* clone() const = 0;

    // Reset playback position to the beginning
    virtual void reset() = 0;

    // Print node metadata to an output stream
    virtual void print(ostream& os) const = 0;

    friend ostream& operator<<(ostream& os, const AudioNode& node) {
        node.print(os);
        return os;
    }
};

#endif
