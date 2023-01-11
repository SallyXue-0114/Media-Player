//==============================================================================

#include "MainComponent.h"
#include "MainApplication.h"

//==============================================================================
// Component overrides

MainComponent::MainComponent() : midiOutputMenu(*this) {
    //Set our managerData member variable to the MediaManager's managerData and add this component as a listener of our local copy.
    managerData = MainApplication::getApp().getMediaManager()->getManagerData();
    managerData.addListener(this);
    
    addAndMakeVisible(openButton);
    openButton.addListener(this);
    
    midiOutputMenu.setTextWhenNothingSelected("MIDI Outputs");
    addAndMakeVisible(midiOutputMenu);
    midiOutputMenu.addListener(this);
    
    addAndMakeVisible(infoButton);
    infoButton.setEnabled(false);
    infoButton.addListener(this);
    
    //Create the Transport giving it our local managerData and make it visible. The transport's initial state should disabled and our component should be its listener.
    transport = std::make_unique<Transport>(managerData);
    transport->setEnabled(false);
    addAndMakeVisible(transport.get());
    
    
    setVisible(true);
    
}

MainComponent::~MainComponent() {
    managerData.removeListener(this);
}

void MainComponent::paint (Graphics& gr) {
    gr.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

}

void MainComponent::resized() {
    auto bounds = getLocalBounds(); //the size of the window
    //从左上角移出一个高度为24 * 64的小方块（即那个button）
    auto p = 8;
    bounds.removeFromTop(p);
    bounds.removeFromBottom(p);
    bounds.removeFromLeft(p);
    bounds.removeFromRight(p);
    auto h = 24;
    
    auto line = bounds.removeFromTop(h); //width = 24
    openButton.setBounds(line.removeFromLeft(90)); //length = 120
    infoButton.setBounds(line.removeFromRight(90));
    
    line = line.withSizeKeepingCentre(180, 24);
    midiOutputMenu.setBounds(line);
    
    //not suppose to be here actually...
    bounds.removeFromTop(8);
    bounds = bounds.withSizeKeepingCentre(250, 78);
    transport->setBounds(bounds);
    
    setVisible(true);
}


//==============================================================================
// JUCE Listener overrides

void MainComponent::buttonClicked(Button* button) {
    //std::cout << "MainComponent::buttonClicked()\n";
    if(button == &openButton){
        //std::cout << "MainComponent::openButton()\n";
        managerData.launchOpenMediaDialog();
        
    } else if(button == &infoButton){
        //std::cout << "MainComponent::infoButton()\n";
        managerData.launchMediaInfoDialog();
    }
}

void MainComponent::comboBoxChanged(ComboBox* menu) {
    //std::cout << "MainComponent::comboBoxChanged()\n";
    if(menu == &midiOutputMenu){
        //std::cout << "MainComponent::midiOutputMenu()\n";
        int id = midiOutputMenu.getSelectedId();
        managerData.setMidiOutputOpenID(id);
    }
}

//==============================================================================
// ValueTree::Listener overrides

void MainComponent::valueTreePropertyChanged(juce::ValueTree& tree,
					     const juce::Identifier& ident) {
    
    //std::cout << "\n--------\nMainComponent::valueTreePropertyChanged!(" << ident.toString() << ")\n";
    
    if(ident == MediaManagerData::TRANSPORT_PLAYING){
        //not showing up!
        //std::cout << "MediaManagerData::TRANSPORT_PLAYING" << std::endl;
        
        auto pausing = !managerData.getTransportPlaying();
        auto mediaType = managerData.getLoadedMediaType();
        openButton.setEnabled(pausing);
        
        if(mediaType == MediaManagerData::MEDIA_MIDI){
            midiOutputMenu.setEnabled(pausing);
        }
            
    } else if (ident == MediaManagerData::LOADED_MEDIA_TYPE){
        //std::cout << "\n--------\nMainComponent::LOADED_MEDIA_TYPE!(" << ident.toString() << ")\n";
        
        auto mediaType = managerData.getLoadedMediaType();
        auto midiOpen = managerData.getMidiOutputOpenID();
        
        switch(mediaType){
            case MediaManagerData::MEDIA_AUDIO:
                infoButton.setEnabled(true);
                midiOutputMenu.setEnabled(false);
                managerData.setTransportEnabled(true);
                managerData.setTransportTempoEnabled(false);
                break;
            
            case MediaManagerData::MEDIA_MIDI:
                infoButton.setEnabled(true);
                midiOutputMenu.setEnabled(true);
                managerData.setTransportEnabled(true);
                managerData.setTransportTempoEnabled(true);
                break;
            
            case MediaManagerData::MEDIA_NONE:
                infoButton.setEnabled(false);
                midiOutputMenu.setEnabled(false);
                managerData.setTransportEnabled(false);
                managerData.setTransportTempoEnabled(false);
                break;
            
            default: jassert(false);
        }
        
    } else if (ident == MediaManagerData::MIDI_OUTPUT_OPEN_ID){
        
        auto mediaType = managerData.getLoadedMediaType();
        auto midiOpen = managerData.getMidiOutputOpenID();
        if(mediaType == MediaManagerData::MEDIA_MIDI){
            managerData.setTransportEnabled(midiOpen);
            managerData.setTransportTempoEnabled(midiOpen);
        }
    }
}
