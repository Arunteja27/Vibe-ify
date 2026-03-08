CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall
LDFLAGS  = -lwinmm

TARGET   = vibeify

OBJECTS  = main.o Track.o Playlist.o TrackLibrary.o AudioEngine.o AudioEffect.o PlaybackController.o SpectrumAnalyzer.o

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

main.o: main.cc TrackLibrary.h AudioEngine.h PlaybackController.h
	$(CXX) $(CXXFLAGS) -c main.cc

Track.o: Track.cc Track.h AudioNode.h
	$(CXX) $(CXXFLAGS) -c Track.cc

Playlist.o: Playlist.cc Playlist.h AudioNode.h Track.h
	$(CXX) $(CXXFLAGS) -c Playlist.cc

TrackLibrary.o: TrackLibrary.cc TrackLibrary.h Track.h
	$(CXX) $(CXXFLAGS) -c TrackLibrary.cc

AudioEngine.o: AudioEngine.cc AudioEngine.h AudioNode.h AudioBuffer.h AudioEffect.h SpectrumAnalyzer.h
	$(CXX) $(CXXFLAGS) -c AudioEngine.cc

AudioEffect.o: AudioEffect.cc AudioEffect.h AudioNode.h
	$(CXX) $(CXXFLAGS) -c AudioEffect.cc

PlaybackController.o: PlaybackController.cc PlaybackController.h TrackLibrary.h Playlist.h AudioEngine.h
	$(CXX) $(CXXFLAGS) -c PlaybackController.cc

SpectrumAnalyzer.o: SpectrumAnalyzer.cc SpectrumAnalyzer.h
	$(CXX) $(CXXFLAGS) -c SpectrumAnalyzer.cc

clean:
	-del /Q $(TARGET).exe *.o 2>nul

.PHONY: clean