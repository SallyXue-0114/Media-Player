//==============================================================================
/// MainApplication.h
/// This file defines the class representing our application instance.

#pragma once

#include "MediaManager.h"

class MainWindow;


class MainApplication : public JUCEApplication {
public:

  MainApplication();

  //============================================================================
  // JUCEApplication overrides

  const String getApplicationName() override;
  const String getApplicationVersion() override;
  bool moreThanOneInstanceAllowed() override;

  void initialise (const String& commandLine) override;

  void shutdown() override;

  void systemRequestedQuit() override;

  void anotherInstanceStarted (const String& commandLine) override;

  static MainApplication& getApp();

  MediaManager* getMediaManager(){return mediaManager.get();}

private:

  void closeAllAlertAndDialogWindows();
  std::unique_ptr<MediaManager> mediaManager;
  std::unique_ptr<MainWindow> mainWindow;
};
