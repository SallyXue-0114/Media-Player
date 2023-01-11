#include "Transport.h"

/// Constructor.
Transport::Transport(const MediaManagerData& mmd) {
    //Set the local managerData member to the copy passed in
    //and add this transport as a listener
    

    managerData = mmd;
    managerData.addListener(this);
    
    drawPlayButton(playPauseButton);
    playPauseButton.setClickingTogglesState(true);
    addAndMakeVisible(playPauseButton);
    playPauseButton.addListener(this);
    
    drawGoToStartButton(goToStartButton);
    addAndMakeVisible(goToStartButton);
    goToStartButton.addListener(this);
    
    double gain = managerData.getTransportGain();
    drawGainButton(gainButton, gain);
    addAndMakeVisible(gainButton);
    gainButton.addListener(this);
    
    
    gainSlider.setRange(0.0, 1.0);
    gainSlider.setValue(managerData.getTransportGain(), dontSendNotification);
    addAndMakeVisible(gainSlider);
    gainSlider.addListener(this);
    
    addAndMakeVisible(tempoSlider);
    tempoSlider.setSliderStyle(Slider::LinearBar);
    tempoSlider.setTextValueSuffix(" bpm");
    tempoSlider.setRange(40, 208, 1);
    tempoSlider.setValue(managerData.getTransportTempo());
    tempoSlider.addListener(this);
    
    drawCurrentTimeLabel();
    currentTimeLabel.setJustificationType(Justification::centredRight);
    currentTimeLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(currentTimeLabel);
    currentTimeLabel.setText("00:00", dontSendNotification);
    
    
    drawEndTimeLabel();
    endTimeLabel.setJustificationType(Justification::centredLeft);
    endTimeLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(endTimeLabel);
    endTimeLabel.setText("100:00", dontSendNotification);
    
    positionSlider.setRange(0.0, 1.0);
    addAndMakeVisible(positionSlider);
    positionSlider.addListener(this);
    
    
    setVisible(true);
    
    
}
    

/// Destructor.
Transport::~Transport() {
}


void Transport::paint(Graphics& gr) {
    gr.setColour(Colours::grey);
    gr.drawRoundedRectangle(0, 0, 250, 78, 8, 2);
    
}

void Transport::resized() {
    auto bounds = getLocalBounds(); //the size of the window
    //从左上角移出一个高度为24 * 64的小方块（即那个button）
    auto p = 6;
    bounds.removeFromTop(p);
    bounds.removeFromBottom(p);
    bounds.removeFromLeft(p);
    bounds.removeFromRight(p);
    
    
    //playPauseButton
    auto line = bounds.removeFromTop(36);
    line = line.withSizeKeepingCentre(36, 36);
    playPauseButton.setBounds(line);
    

    //top buttons other than playPauseButton
    auto bounds2 = getLocalBounds();
    bounds2.removeFromBottom(p);
    bounds2.removeFromLeft(p);
    bounds2.removeFromRight(p);
    
    bounds2.removeFromTop((36-24)/2 + p);
    auto line2 = bounds2.removeFromTop(24);
    
    tempoSlider.setBounds(line2.removeFromLeft(getWidth()/2 - p - 36/2 - 24 - 6));
    line2.removeFromLeft(6);
    goToStartButton.setBounds(line2.removeFromLeft(24));
    line2.removeFromLeft(36);
    gainButton.setBounds(line2.removeFromLeft(24));
    gainSlider.setBounds(line2.removeFromLeft(getWidth()/2 - p - 36/2 - 24));
    

    //bottom buttons
    auto bounds3 = getLocalBounds();
    bounds3.removeFromBottom(p);
    bounds3.removeFromLeft(p);
    bounds3.removeFromRight(p);
    auto line3 = bounds3.removeFromBottom(24);
    currentTimeLabel.setBounds(line3.removeFromLeft(44));
    endTimeLabel.setBounds(line3.removeFromRight(44));
    positionSlider.setBounds(line3);

    
    setVisible(true);
}

//============================================================================
// JUCE Component Callbacks

void Transport::buttonClicked(juce::Button* button) {
    if(button == &playPauseButton){
        //std::cout << "playPauseButton clicked" << std::endl;
        
        bool state = !managerData.getTransportPlaying();
        managerData.setTransportPlaying(state, this);
        
        
    } else if (button == &goToStartButton){
        //std::cout << "goToStartButton clicked" << std::endl;
        
        positionSlider.setValue(0.0, dontSendNotification);
        managerData.setTransportRewind();
        
    } else if (button == &gainButton){
        //std::cout << "gainButton clicked" << std::endl;
        
        auto g = (gainSlider.getValue() > 0.0)? 0.0 : 0.5;
        gainSlider.setValue(g, dontSendNotification);
    }
}


void Transport::sliderValueChanged(juce::Slider* slider) {
    //std::cout << "Transport::sliderValueChanged()\n";
    if(slider == &positionSlider){
        //std::cout << "Transport::positionSlider\n";
        auto position = slider->getValue();
        managerData.setTransportPosition(position, this);
        drawCurrentTimeLabel();
        
    } else if (slider == &gainSlider){
        //std::cout << "Transport::gainSlider\n";
        auto val = slider->getValue();
        managerData.setTransportGain(val, this);
        drawGainButton(gainButton, val);
        
        
    } else if (slider == &tempoSlider){
        //std::cout << "Transport::tempoSlider\n";
        auto position = slider->getValue();
        managerData.setTransportTempo(position, this);
        
    }
    
}

//============================================================================
// JUCE ValueTree callbacks (listening to the managerData changes)

void Transport::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& ident) {
    //std::cout << "\n---------\nTransport::valueTreePropertyChanged(" << ident.toString() << ")\n";
    
    if(ident == MediaManagerData::TRANSPORT_ENABLED){
        setEnabled(managerData.getTransportEnabled());
        
    } else if(ident == MediaManagerData::TRANSPORT_TEMPO_ENABLED){
        tempoSlider.setEnabled(managerData.getTransportTempoEnabled());
        
    } else if(ident == MediaManagerData::TRANSPORT_GAIN){
        gainSlider.setValue(managerData.getTransportGain(), dontSendNotification);
        
    } else if(ident == MediaManagerData::TRANSPORT_TEMPO){
        tempoSlider.setValue(managerData.getTransportTempo(), dontSendNotification);
        
    } else if(ident == MediaManagerData::TRANSPORT_CLICK_PLAYPAUSE){
        playPauseButton.triggerClick();
        
    } else if(ident == MediaManagerData::TRANSPORT_POSITION){
        positionSlider.setValue(managerData.getTransportPosition(), dontSendNotification);
        drawCurrentTimeLabel();
        
    } else if(ident == MediaManagerData::TRANSPORT_PLAYBACK_DURATION){
        drawEndTimeLabel();
    }
}

//============================================================================
// Icon Button Drawing

void Transport::drawPlayButton(juce::DrawableButton& button) {
  juce::DrawablePath triangleDrawable, rectangleDrawable;
  juce::Path trianglePath, rectanglePath;
    trianglePath.addTriangle(0, 0, 0, 100, 86.6, 50);
    rectanglePath.addRectangle(0.0, 0.0, 42, 150.0);
    rectanglePath.addRectangle(100, 0.0, 42, 150.0);
    
    triangleDrawable.setPath(trianglePath);
    rectangleDrawable.setPath(rectanglePath);
    
    juce::FillType fill(iconColor);
    rectangleDrawable.setFill(fill);
    triangleDrawable.setFill(fill);
    
    button.setImages(&triangleDrawable, nullptr, nullptr, nullptr, &rectangleDrawable);
}

void Transport::drawGoToStartButton(juce::DrawableButton& b) {
  // Juce path drawing done in percentage (100x100)
  juce::DrawablePath image;
  juce::Path path;
    path.addTriangle(100, 0, 100, 100, 13.4, 50);
    path.addRectangle(13.4, 0, 13.4, 100);
    image.setPath(path);
    juce::FillType fill(iconColor);
    image.setFill(fill);
  b.setImages(&image);
}

void Transport::drawGainButton(juce::DrawableButton& button, double gain) {
  // Juce path drawing done in percentage (100x100)
  juce::DrawablePath drawable;
  juce::Path p;
  
  // speaker rect from 0 to 30
  p.addRectangle(0, 30, 30, 35);
  // speaker cone from 0 to 45
  p.addTriangle(0, 50, 40, 0, 40, 100);
  // waves start at x=55 spaced 15 apart
  if (gain > 0.1)
    p.addCentredArc(55, 50, 6, 20,  0, 0, 3.14159f, true);
  if (gain > 0.4)
    p.addCentredArc(70, 50, 5, 35,  0, 0, 3.14159f, true);
  if (gain > 0.7)
    p.addCentredArc(85, 50, 5, 50,  0, 0, 3.14159f, true);
  // this makes button image width 100 no matter how many arcs added
  p.startNewSubPath(100, 0);
  drawable.setPath(p);
  drawable.setFill(iconColor);
  button.setImages(&drawable);
}

void Transport::drawCurrentTimeLabel() {
    float currValue = positionSlider.getValue();
    float duration = managerData.getPlaybackDuration();
    String duration_s = toFormattedTimeString(duration * currValue);
    currentTimeLabel.setText(duration_s, NotificationType::dontSendNotification);
}

void Transport::drawEndTimeLabel() {
    float duration = managerData.getPlaybackDuration();
    String duration_s = toFormattedTimeString(duration);
    endTimeLabel.setText(duration_s, NotificationType::dontSendNotification);
    
}

const juce::String Transport::toFormattedTimeString(const double seconds) {
    int second_int = roundToIntAccurate(seconds);
    int minute = second_int/60;
    int second = second_int % 60;
    String out = "0" + std::to_string(minute) + ":";
    if(second < 10){
        out += "0";
    }
    String temp = std::to_string(second);
    out += temp;
    return out;
}
