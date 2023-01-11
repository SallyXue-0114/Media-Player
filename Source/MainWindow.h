//==============================================================================
// @file MainWindow.h
// Defines the app's main window instance.

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/// The main application window.
class MainWindow  : public DocumentWindow {
public:
  
  MainWindow(String name);

  //============================================================================
  // DocumentWindow overrides

  void closeButtonPressed() override;
  
private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};
