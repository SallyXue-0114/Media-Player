//==============================================================================
/// @file Transport.h
/// A generic audio transport that can be used for MIDI and audio playback.

#pragma once

#include "MediaManagerData.h"

struct Transport : public Component, public Button::Listener,
public Slider::Listener, public ValueTree::Listener {

  Transport(const MediaManagerData& mmd);

  ~Transport();

  MediaManagerData managerData;

private:

  void resized() override;
  void paint(Graphics& gr) override;
  
  //============================================================================
  // JUCE component callbacks.

  void buttonClicked(Button* button) override;
  void sliderValueChanged(Slider* slider) override;
 
  //============================================================================
  // JUCE ValueTree callbacks (only valueTreePropertyChanged() is used).

  void valueTreePropertyChanged(ValueTree& tree, const Identifier& ident) final override;
  void valueTreeChildAdded(ValueTree& parent, ValueTree& child) final override {}
  void valueTreeChildRemoved(ValueTree& parent, ValueTree& child, int index) final override {}
  void valueTreeChildOrderChanged(ValueTree& parent, int iOld, int iNew) final override {}
  void valueTreeParentChanged(ValueTree& tree) final override {}

  //============================================================================
  // Subcomponent drawing.

  void drawPlayButton(DrawableButton& button);
  void drawGoToStartButton(DrawableButton& b);
  void drawGainButton(DrawableButton& button, double gain);
  void drawCurrentTimeLabel();
  void drawEndTimeLabel();
  static const String toFormattedTimeString(const double seconds);
  
  //============================================================================
  // Member variables.

  Colour iconColor {Colours::white};
  DrawableButton playPauseButton {"", DrawableButton::ImageFitted};
  DrawableButton goToStartButton {"", DrawableButton::ImageFitted};
  DrawableButton gainButton {"", DrawableButton::ImageFitted};
  Slider gainSlider {Slider::LinearHorizontal, Slider::NoTextBox};
  Slider tempoSlider {Slider::LinearHorizontal, Slider::TextBoxLeft};
  Slider positionSlider {Slider::LinearHorizontal, Slider::NoTextBox};
    Label currentTimeLabel;
    Label endTimeLabel;

  //============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Transport)
};
