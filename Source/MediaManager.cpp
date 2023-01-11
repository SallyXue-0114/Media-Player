#include "MediaManager.h"
#include "MainApplication.h"

MediaManager::MediaManager() {
    auto str = audioDeviceManager.initialise(0, 2, nullptr, true);
    jassert(str.isEmpty());
    playbackThread = std::make_unique<MidiPlaybackThread>(this, 100, 60.0);
    playbackThread->startThread();
    managerData.addListener(this);
    formatManager.registerBasicFormats();
    audioDeviceManager.addAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(&transportSource);
    
    //timer bug fixed!
    startTimer(50);
}

MediaManager::~MediaManager() {
    stopTimer();
    managerData.removeListener(this);
    playbackThread->stopThread(100); //!
    playbackThread = nullptr;
    sendAllSoundsOff();
    closeMidiOutput();
    transportSource.setSource(nullptr);
    audioSourcePlayer.setSource(nullptr);
    audioDeviceManager.removeAudioCallback(&audioSourcePlayer);
    
}

///=============================================================================
/// The ManagerData callbacks

void MediaManager::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& ident) {
    //std::cout << "MediaManager::valueTreePropertyChanged(" << ident.toString() << ")\n";
    
    auto mediaType = managerData.getLoadedMediaType();
    
    if(ident == MediaManagerData::LAUNCH_OPEN_MEDIA_DIALOG){
        openMediaFile();
        //std::cout << "  LAUNCH_OPEN_MEDIA_DIALOG\n";
        
    } else if (ident == MediaManagerData::LAUNCH_MEDIA_INFO_DIALOG){
        openMediaInfoDialog();
        //std::cout << "   LAUNCH_MEDIA_INFO_DIALOG\n";
        
    } else if (ident == MediaManagerData::TRANSPORT_PLAYING){
        //worked!
        //std::cout << "   MediaManagerData::TRANSPORT_PLAYING\n";
        if(managerData.getTransportPlaying()){
            switch(mediaType){
                case MediaManagerData::MEDIA_MIDI: playMidi(); break;
                case MediaManagerData::MEDIA_AUDIO: playAudio(); break;
            }
        } else {
            switch(mediaType){
                case MediaManagerData::MEDIA_MIDI: pauseMidi(); break;
                case MediaManagerData::MEDIA_AUDIO: pauseAudio(); break;
            }
        }
        
    } else if (ident == MediaManagerData::TRANSPORT_GAIN){
        auto val = managerData.getTransportGain();
        switch(mediaType){
            case MediaManagerData::MEDIA_MIDI: setMidiGain(val); break;
            case MediaManagerData::MEDIA_AUDIO: setAudioGain(val); break;
        }
        
    } else if (ident == MediaManagerData::TRANSPORT_TEMPO){
        auto t = managerData.getTransportTempo();
        switch(mediaType){
            case MediaManagerData::MEDIA_MIDI: setMidiTempo(t); break;
            case MediaManagerData::MEDIA_AUDIO: setAudioTempo(t); break;
        }
        
    } else if (ident == MediaManagerData::TRANSPORT_REWIND){
        if(managerData.getTransportPlaying()){
            managerData.clickPlayPause();
        }
        switch(mediaType){
            case MediaManagerData::MEDIA_MIDI: rewindMidi(); break;
            case MediaManagerData::MEDIA_AUDIO: rewindAudio(); break;
        }
        
    } else if (ident == MediaManagerData::TRANSPORT_POSITION){
        auto position = managerData.getTransportPosition();
        switch(mediaType){
            case MediaManagerData::MEDIA_MIDI: setMidiPlaybackPosition(position); break;
            case MediaManagerData::MEDIA_AUDIO: setAudioPlaybackPosition(position); break;
        }
        
    } else if (ident == MediaManagerData::MIDI_OUTPUT_OPEN_ID){
        auto id = managerData.getMidiOutputOpenID();
        if(id == 0){
            closeMidiOutput();
        } else {
            openMidiOutput(id-1);
        }
    }
}


///=============================================================================
// PlaybackPositionTimer callback

void MediaManager::timerCallback() {
    if(managerData.getTransportPlaying()){
        auto media_type = managerData.getLoadedMediaType();
        if(media_type == MediaManagerData::MEDIA_MIDI){
            scrollMidiPlaybackPosition();
        } else if (media_type == MediaManagerData::MEDIA_AUDIO){
            scrollAudioPlaybackPosition();
        }
    }
}

//==============================================================================
// Generic Media Support
//==============================================================================

void MediaManager::openMediaFile() {
    //Pass (wildcard) matches of all the supported midi and audio file extensions to a juce FileChooser. Midi wild types are "*.mid;*.midi" and audio file types are given by formatManager.getWildcardForAllFormats().
    
    //std::cout << "MediaManager::openMediaFile() !!\n";
    String midiFileTypes{"*.mid;*.midi"};
    String audioFileType(formatManager.getWildcardForAllFormats());
    
    //std::cout << "audioFileTypes: " << audioFileType << "\n";
    String allowableTypes =  midiFileTypes + ";" + audioFileType;
    
    FileChooser fc ("Open Media File");
    //Call browseForFileToOpen() and if it returns true then call loadMidiFile() if the file is a midi file else call loadAudioFile(). See: FileChooser.browserForFileToOpen().
    if(fc.browseForFileToOpen()){
        File file = fc.getResult();
        std::cout << "my file is:" << file.getFullPathName() << std::endl;
        
        if(midiFileTypes.contains(file.getFileExtension())){
            loadMidiFile(file);
        } else if (audioFileType.contains(file.getFileExtension())){
            loadAudioFile(file);
        }
    }
    
}

void MediaManager::openMediaInfoDialog() {
    //pop up the mediaInfo window!
    //need to call other functions
    //std::cout << "MediaManager::openMediaInfoDialog() !!\n";

    //Call getMediaInfo() to get information about the loaded media file and if the string is empty return. Otherwise do the following actions:
    auto text = getMediaInfo();
    if(text.isEmpty()){
        return;
    }

    //Create a TextEditor to hold the info. Configure the editor to be multiline, readonly, sized 400x200. Then give it the text string.
    TextEditor* editor = new TextEditor();
    editor->setMultiLine(true);
    editor->setReadOnly(true);
    editor->setSize(400, 200);
    editor->setText(text);
    
    
    //Allocate a juce::DialogWindow::LaunchOptions struct and fill it. The dialog's titlebar should be native, it should be resizable, the title should be "Media Info" and the background color our application's backgroundColor.
    juce::DialogWindow::LaunchOptions dw;
    dw.useNativeTitleBar = true;
    dw.resizable = true;
    dw.dialogTitle = "Media Info";
    dw.dialogBackgroundColour = LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    dw.content.setOwned(editor);
    dw.launchAsync();

    
}


const String MediaManager::getMediaInfo() {
    //Returns information about the current media file.

    //If the managerData's loaded media type is MEDIA_MIDI then return getMidiInfo() else if its MEDIA_AUDIO return getAudioInfo() else return a null string.
    auto type = managerData.getLoadedMediaType();
    if(type == MediaManagerData::MEDIA_NONE){
        return "";
    }
    
    if(type == MediaManagerData::MEDIA_AUDIO){
        return getAudioInfo();
    } else {
        return getMidiInfo();
    }
}

//==============================================================================
// Audio playback support

void MediaManager::openAudioSettings() {
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(audioDeviceManager, 0,0,2,2,false, true, true, false);
    selector->setSize(500, 270);
    DialogWindow::LaunchOptions dialog;
    dialog.useNativeTitleBar = true;
    dialog.dialogTitle = "Audio Settings";
    dialog.dialogBackgroundColour = LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    dialog.content.setOwned(selector.release());
    dialog.launchAsync();
}

const String MediaManager::getAudioInfo() {
    //!
    //std::cout << "MediaManager::getAudioInfo" << std::endl;
    
    auto audioReader = audioFileReaderSource->getAudioFormatReader();
    
    //important part!!!
    auto audioFile = managerData.getLoadedMediaFile();
    if(audioFile == File()) return "";
    if(audioFileReaderSource == nullptr) return "";
    if(audioReader == nullptr) return "";
    
    String floatingPointData = "";
    if(audioReader->usesFloatingPointData == true){
        floatingPointData = "yes";
    } else {
        floatingPointData = "no";
    }
    
    String info;
    
    //look at AudioFormatReader!!!
    info
    << "Audio file: " << audioFile.getFullPathName() << "\n"
    << "File size: " << audioFile.getSize() << "\n"
    << "Audio format: " << audioReader->getFormatName() << "\n"
    << "Channels: " << (String) audioReader->numChannels << "\n"
    << "Sample rate: " << audioReader->sampleRate << "\n"
    << "Sample frames: " << audioReader->lengthInSamples << "\n"
    << "Bits per sample: " << (String) audioReader->bitsPerSample << "\n"
    << "Floating point data: " << floatingPointData << "\n";
    
    return info;
}



void MediaManager::loadAudioFile(File audioFile) {
    //std::cout << "MediaManager::loadAudioFile(" << audioFile.getFileName() << ")\n";
    
    if(AudioFormatReader* reader = formatManager.createReaderFor(audioFile)){
        //have a valid audiofile now, load it into transport
        loadIntoTransport(reader);
        
    } else {
        //open alertWindow - audioFile unsupported format
        String msg("The file ");
        msg << audioFile.getFileName() << " is an unsupported audio format.";
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Open Media File", msg);
        audioFile = File(); //zero out audio file
    }
    
    //set the mediaManager's media file to the loaded audio file and the media type to MEDIA_AUDIO (or to File() and and MEDIA_NONE if the file was not loaded)
    auto mediaType = (audioFile == File()) ? MediaManagerData::MEDIA_NONE
    :  MediaManagerData::MEDIA_AUDIO;
    
    managerData.setLoadedMediaFile(audioFile);
    managerData.setLoadedMediaType(mediaType);
    
}



void MediaManager::loadIntoTransport(AudioFormatReader* reader) {
    //std::cout << "MediaManager::loadIntoTransport()\n";
    
    //Call the managerData method to stop the transport playing.
    managerData.setTransportPlaying(false);
    
    
    //Call the managerData method to set the transport's position to 0.
    managerData.setTransportPosition(0);
    
    //Call clearAudioPlaybackState() to zero out the MediaManager's playback data.
    clearAudioPlaybackState();
    
    //Create a new AudioFormatReaderSource for the reader.
    audioFileReaderSource.reset(new AudioFormatReaderSource(reader, true));
    
    
    //Pass that reader source to the transportSource using its setSource() method (you will also need to pass it the reader's sampleRate value.)
    transportSource.setSource(audioFileReaderSource.get(), 0,0, reader->sampleRate);
    
    
    //Call transportSource.getLengthInSeconds() and pass that value to the managerData method that sets the total duration of playback.
    managerData.setPlaybackDuration(transportSource.getLengthInSeconds());
    
}


void MediaManager::clearAudioPlaybackState() {
    transportSource.setSource(nullptr);
    audioFileReaderSource = nullptr;
}



//==============================================================================
// MIDI transport callbacks

void MediaManager::playMidi() {
  //std::cout << "MediaManager::playMidi()\n";
    if (playbackThread->isPlaybackAtEnd()) {
        playbackThread->setPlaybackPosition(0.0,0);
    }
    playbackThread->setTempo(managerData.getTransportTempo());
    playbackThread->setGain(managerData.getTransportGain());
    playbackThread->setPaused(false);
}


void MediaManager::pauseMidi() {
  //std::cout << "MediaManager::pauseMidi()\n";
    playbackThread->setPaused(true);
}

void MediaManager::setMidiGain(double gain) {
  //std::cout << "MediaManager::setMidiGain(" << gain << ")\n";
    playbackThread->setGain(gain);
}

void MediaManager::setMidiTempo(double tempo) {
  //std::cout << "MediaManager::setMidiTempo(" << tempo << ")\n";
    playbackThread->setTempo(tempo);
}

void MediaManager::rewindMidi() {
  //std::cout << "MediaManager::rewindMidi()\n";
    playbackThread->setPlaybackPosition(0.0, 0); //！
}

void MediaManager::setMidiPlaybackPosition(double position) {
  //std::cout << "MediaManager::setMidiPlaybackPosition("<< position <<")\n";
    double time = position * midiFileDuration;
    int index = sequence.getNextIndexAtTime(time);
    
    if(position >= midiFileDuration){
        index = midiFileLength; //don't go over
    }
    
    //pause playback thread if its playing
    auto playing = managerData.getTransportPlaying();
    if(playing){
        playbackThread->pause();
    }
    playbackThread->clear();
    
    //rewind
    playbackThread->setPlaybackPosition(time, index);
    
    //if thread is running, start it running again
    if(playing){
        playbackThread->play();
    }
}

void MediaManager::scrollMidiPlaybackPosition() {
  //std::cout << "MediaManager::scrollMidiPlaybackPosition()\n";
    double sec = playbackThread->getPlaybackBeat();
    double dur = managerData.getPlaybackDuration();
    double pos = sec/dur;
    managerData.setTransportPosition(pos, this);
    
    //std::cout << "pbPos="<<sec<<", pbDur="<<dur<<", pbrat="<<pos<<"\n";
    if(pos >= 1.0){
        managerData.clickPlayPause();
    }
    
}




/// Audio transport callbacks

void MediaManager::playAudio() {
  //std::cout << "MediaManager::playAudio()\n";
  // If the transportSource has finished playing set its position to 0.0
  if (transportSource.hasStreamFinished()) {
    transportSource.setPosition(0.0);
  }
  // Set the transportSource's gain to the managerData's gain.
  transportSource.setGain(managerData.getTransportGain());
  // Start the transport source.
  transportSource.start();
}

void MediaManager::pauseAudio() {
  //std::cout << "MediaManager::pauseAudio()\n";
  // Stop the transportSource.
  transportSource.stop();
}

void MediaManager::setAudioGain(double gain) {
  //std::cout << "MediaManager::setAudioGain(" << gain << ")\n";
  // Set the transportSource's gain to the managerData's gain.
  //transportSource.setGain(managerData.getTransportGain());
    transportSource.setGain(gain);

}

void MediaManager::setAudioTempo(double tempo) {
  //std::cout << "MediaManager::setAudioTempo(" << tempo << ")\n";
  // nothing to do!
}

void MediaManager::rewindAudio() {
  //std::cout << "MediaManager::rewindAudio()\n";
  // set the transportSource's position back to 0.0.
  transportSource.setPosition(0.0);
}

/// Sets the audio transport player's position.
void MediaManager::setAudioPlaybackPosition(double pos) {
  auto playing = managerData.getTransportPlaying();
  //std::cout << "media manager: receiving position:" << pos
  //<< ", playing:" << managerData.getTransportPlaying() << "\n" ;
  if (pos == 0.0) {
    // setSource(0) stops popping on rewind and play
    transportSource.setSource(0);
    transportSource.setSource(audioFileReaderSource.get());
    if (playing) transportSource.start();
  }
  else {
    // std::cout << "transport position=" << position << "\n";
    if (playing) transportSource.stop(); // not sure why this is necessary!
    transportSource.setPosition(pos * transportSource.getLengthInSeconds());
    if (playing) transportSource.start();
  }
}

void MediaManager::scrollAudioPlaybackPosition() {
  double sec = transportSource.getCurrentPosition();
  double dur = transportSource.getLengthInSeconds();
  double pos = sec/dur;
  //std::cout << "pbPos="<<sec<<", pbDur="<<dur<<", pbrat="<<pos<<"\n";
  managerData.setTransportPosition(pos, this);
  // auto-pause if at end-of-file
  if (pos >= 1.0)
    managerData.clickPlayPause();
}



///==============================================================================

void MediaManager::openMidiOutput(int dev) {
  // Call MidiOutput::openDevice and reset the midiOutputDevice to it.
  midiOutputDevice = MidiOutput::openDevice(dev);
  jassert(midiOutputDevice != nullptr);
}

void MediaManager::closeMidiOutput() {
  // Set the midiOutputDevice to nullptr.
  midiOutputDevice.reset(nullptr);
}

bool MediaManager::isInternalSynthAvailable() {
  return false;
}


///==============================================================================
/// MidiFile Functions

const String MediaManager::getMidiInfo() {
    //std::cout << "MediaManager::getMidiInfo" << std::endl;
    
    //important part!!!
    auto midiFile = managerData.getLoadedMediaFile();
    if(midiFile == File()) return "";
    
    String info;
    
    int level = 0;
    
    //look at AudioFormatReader!!!
    info
    << "Midi file: " << managerData.getLoadedMediaFile().getFullPathName() << "\n"
    << "File size: " << managerData.getLoadedMediaFile().getSize() << "\n"
    << "MIDI file format: level " << static_cast<int>(midiFileNumTracks > 1) << "\n"
    << "Number of Tracks: " << midiFileNumTracks << "\n"
    << "Duration: " << midiFileDuration << "\n"
    << "Number of Messages: " << midiFileLength << "\n";

    return info;
}


void MediaManager::clearMidiPlaybackState() {
    midiFileDuration = 0;
    midiFileLength = 0;
    midiFileNumTracks = 0;
    midiFileTimeFormat = 0;
    sequence.clear();
    playbackThread->setPlaybackPosition(0.0, 0);
}


void MediaManager::loadMidiFile(File midiFile) {
    //std::cout << "MediaManager::loadMidiFile" << std::endl;
    //Create a local variable holding a FileInputStream for the file.
    FileInputStream inputstream(midiFile);
    
    //Create a local variable holding a juce MidiFile.
    MidiFile midifile;
    
    
    //If the FileInputStream opened ok:
    //use MidiFile::readFrom() to see if the input is a valid MidiFile, if it was, and if the MidiFile's time format is greater than 0 then:
    if(inputstream.openedOk() && midifile.readFrom(inputstream) && midifile.getTimeFormat() > 0){
        //pass the midi file to loadIntoPlayer().
        loadIntoPlayer(midifile);
        
    } else {
        //Otherwise (the previous step failed):
        //open an alert window to tell the user that the the midi file could not be loaded.
        //pass a null File to managerData.setLoadMediaFile().
        //pass MEDIA_NONE to managerData.setLoadedMediaType().
        
        String msg("The file ");
        msg << midiFile.getFileName() << " is an unsupported midi format.";
        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Open Media File", msg);
        
        return;
    }
    
    managerData.setLoadedMediaFile(midiFile);
    auto mediaType = midiFile == File() ? MediaManagerData::MEDIA_NONE : MediaManagerData::MEDIA_MIDI;
    managerData.setLoadedMediaType(mediaType);

    
}


void MediaManager::loadIntoPlayer(MidiFile& midifile) {
    //std::cout << "MediaManager::loadIntoPlayer" << std::endl;
    //Push the stop and rewind buttons to halt playback and clear any pending messages
    managerData.setTransportPlaying(false);
    managerData.setTransportPosition(0.0);
    
    clearMidiPlaybackState();
    
    //Set midiFileNumTracks and midiFileTimeFormat the values from the midifile.
    midiFileNumTracks = midifile.getNumTracks();
    midiFileTimeFormat = midifile.getTimeFormat();
    
    //Convert the beat-based midi file into time in seconds.
    midifile.convertTimestampTicksToSeconds();
    
    
    //Iterate track index from 0 below MidiFile::getNumTracks() and merge them into our empty playback sequence:
    for(int track = 0; track < midiFileNumTracks; track++){
        //get the current track using MidiFile::getTrack()
        const juce::MidiMessageSequence* seq = midifile.getTrack(track);
        
        //call Sequence::addSequence() to merge the current track into our single playback sequence. For the end time add 1 to the value of getEndTime() to ensure that everything is added!
        sequence.addSequence(*seq, 0.0, 0.0, seq->getEndTime()+1);
        
        //all updateMatchedPairs() on the sequence to attach noteOns to noteOffs.
        sequence.updateMatchedPairs();
    }
    
    //Set midiFileLenfth to the number of events in the sequence
    midiFileLength = sequence.getNumEvents();
    //Set file playback duration to the very last note event
    midiFileDuration = sequence.getEndTime();
    //Set the playbackThread's playback limit to the file duration and file length
    playbackThread->setPlaybackLimit(midiFileDuration, midiFileLength);
    //Set the transport's playback range to the file duration
    managerData.setPlaybackDuration(midiFileDuration);
    
}

///==============================================================================
/// MidiMessage Functions

void MediaManager::sendMessage(const MidiMessage& message) {
  ScopedLock sl (sendLock);
  if (midiOutputDevice) {
    midiOutputDevice->sendMessageNow(message);
  }
  else if (isInternalSynthAvailable()) {
    playInternalSynth(message);
  }
}

void MediaManager::playInternalSynth(const MidiMessage& message) {
}

void MediaManager::sendAllSoundsOff() {
  std::cout << "Sending allSoundsOff, output port is "
  << (midiOutputDevice.get() ? "OPEN" : "CLOSED") << "\n";
  for (int i = 1; i <= 16; i++) {
    juce::MidiMessage msg = juce::MidiMessage::allSoundOff(i);
    sendMessage(msg);
  }
}

//==============================================================================
// MidiPlaybackClient callbacks

void MediaManager::handleMessage (const MidiMessage &message) {
  sendMessage(message);
}

void MediaManager::addMidiPlaybackMessages(MidiPlaybackThread::MidiMessageQueue& queue,
                                           MidiPlaybackThread::PlaybackPosition& position) {
  int index = position.index;
  for (; index < position.length; index++) {
    juce::MidiMessageSequence::MidiEventHolder* ev = sequence.getEventPointer(index);
    // skip over non-channel messages
    if (ev->message.getChannel() < 1)
      continue;
    // skip over noteOffs because we add by pairs with noteOns
    if (ev->message.isNoteOff())
      continue;
    // add every message that is at or earlier (e.g. late) than the current time
    if (ev->message.getTimeStamp() <= position.beat) {
      queue.addMessage(new juce::MidiMessage(ev->message));
      if (ev->noteOffObject) {
        queue.addMessage(new juce::MidiMessage(ev->noteOffObject->message));
      }
    }
    else
      break;
  }
  // index is now the index of the next (future) event or length
  position.index = index;
  if (position.isAtEnd())
    std::cout << "Midi playback at end!\n";
}

