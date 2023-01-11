//==============================================================================
/// @file MediaManager.h
/// A class providing support for audio/midi playback.

#pragma once

#include "MediaManagerData.h"
#include "MidiPlaybackThread.h"

struct MediaManager :
  public MidiPlaybackThread::MidiPlaybackClient,
  public ValueTree::Listener,
  public Timer {

  MediaManager();
  
  ~MediaManager();

  MediaManagerData& getManagerData() {return managerData;}

 private:

  //============================================================================
  // ValueTree::Listener overrides (only valueTreePropertyChanged() is used)

  void valueTreePropertyChanged(ValueTree& tree, const Identifier& ident) final override;

  void valueTreeChildAdded(ValueTree& tree, ValueTree& child) final override {}
  void valueTreeChildRemoved(ValueTree& tree, ValueTree& child, int index) final override {}
  void valueTreeChildOrderChanged(ValueTree& tree, int oldc, int newc) final override {}
  void valueTreeParentChanged(ValueTree& tree) final override {}

  //============================================================================
  // Timer overrides

  void timerCallback() override;
  
  //============================================================================
  // Generic Media Support

  void openMediaFile();

  void openMediaInfoDialog();

  const String getMediaInfo();

  //==============================================================================
  // Audio playback support
 
  void openAudioSettings();

  const String getAudioInfo();

  void playAudio();

  void pauseAudio();

  void setAudioGain(double gain);

  void setAudioTempo(double tempo);

  void rewindAudio();

  void setAudioPlaybackPosition(double pos);

  void scrollAudioPlaybackPosition();

  void loadAudioFile(File file) ;

  void loadIntoTransport(AudioFormatReader* reader) ;

  void clearAudioPlaybackState();
  
  //============================================================================
  // MIDI playback support
  
  void playMidi();

  void pauseMidi();

  void setMidiGain(double gain);

  void setMidiTempo(double tempo);

  void setMidiPlaybackPosition(double positoin);

  void scrollMidiPlaybackPosition();

  void rewindMidi();

  const String getMidiInfo();

  bool isInternalSynthAvailable();

  void openMidiOutput(int dev);

  void closeMidiOutput();

  void loadMidiFile(File file);

  void loadIntoPlayer(MidiFile& midifile);

  void clearMidiPlaybackState();

  void playInternalSynth(const MidiMessage& message);

  void sendAllSoundsOff();

  void sendMessage(const MidiMessage& message);

  //==============================================================================
  // MidiPlaybackClient overrides

  void addMidiPlaybackMessages(MidiPlaybackThread::MidiMessageQueue &queue,
                               MidiPlaybackThread::PlaybackPosition &position) override;

  void handleMessage(const MidiMessage& midiMessage) override;
  
   
  //==============================================================================
  // Audio member data

  AudioDeviceManager audioDeviceManager;

  AudioFormatManager formatManager;

  AudioSourcePlayer audioSourcePlayer;

  AudioTransportSource transportSource;

  std::unique_ptr<juce::AudioFormatReaderSource> audioFileReaderSource;

  //==============================================================================
  // MIDI member data.

  std::unique_ptr<MidiOutput> midiOutputDevice;

  std::unique_ptr<MidiPlaybackThread> playbackThread;

  MediaManagerData managerData;

  int midiFileNumTracks {0};

  int midiFileTimeFormat {0};

  int midiFileLength {0};

  double midiFileDuration {0.0};

  MidiMessageSequence sequence;

  CriticalSection sendLock;
  
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MediaManager)
};

