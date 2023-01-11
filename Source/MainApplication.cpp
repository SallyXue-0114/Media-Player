//==============================================================================

#include "MainApplication.h"
#include "MainWindow.h"
#include "MainComponent.h"

//==============================================================================
// MainApplication members

MainApplication& MainApplication::getApp() {
  MainApplication* const app = dynamic_cast<MainApplication*>(JUCEApplication::getInstance());
  jassert(app != nullptr);
  return *app;
}

void MainApplication::closeAllAlertAndDialogWindows() {
}

//==============================================================================
// JUCEApplication overrides

MainApplication::MainApplication() {
}

const String MainApplication::getApplicationName() {
  return ProjectInfo::projectName;
}

const String MainApplication::getApplicationVersion() {
  return ProjectInfo::versionString;
}

bool MainApplication::moreThanOneInstanceAllowed() {
  return false;
}

void MainApplication::initialise(const String& commandLine) {
    //do mediaManager first!
    //create MediaManager
    mediaManager = std::make_unique<MediaManager>();

    //create MainWindow
    mainWindow = std::make_unique<MainWindow>(getApplicationName());

}

void MainApplication::shutdown() {
    mainWindow = nullptr;
    mediaManager = nullptr;
}

void MainApplication::systemRequestedQuit() {
    closeAllAlertAndDialogWindows();
    quit();
}

void MainApplication::anotherInstanceStarted(const String& commandLine) {
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MainApplication)
