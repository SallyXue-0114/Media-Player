//==============================================================================

#include "MainApplication.h"
#include "MainWindow.h"
#include "MainComponent.h"

MainWindow::MainWindow(String name)
: DocumentWindow(name, Colours::white, DocumentWindow::allButtons) {
    // Window title bar uses  the OS'es window style
    setUsingNativeTitleBar(true);
    
    // On windows or linux install the menubar in the window.
    // FIXME: on iOs or Android you will do something else!
  #if JUCE_WINDOWS || JUCE_LINUX
    setMenuBar(MainApplication::getApp().getMenuBarModel());
  #endif
    
    // add our content component. owned means that when the window is
    // deleted it will automatically delete our content component
    auto maincomp = std::make_unique<MainComponent>();
    setContentOwned(maincomp.release(), false);
    
    // On laptops put the window in the middle of the screen with size
    // determined by our content component.
  #if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX
    //not be resizable!
      setResizable(false, false);
      centreWithSize(392, 120);
  #else // JUCE_IOS || JUCE_ANDROID
       setFullScreen (true);
  #endif
    
    // make the window visible(inportant!)
    setVisible(true);
    
}

//==============================================================================
// DocumentWindow overrides

void MainWindow::closeButtonPressed() {
    MainApplication::getApp().systemRequestedQuit();
}

