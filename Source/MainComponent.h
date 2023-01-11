//==============================================================================
/// @file MainComponent.h
/// Defines the class representing our main window content component.

#pragma once

#include "MidiPlaybackThread.h"
#include "MediaManager.h"

struct MainComponent : public Component, public ComboBox::Listener,
public Button::Listener, public ValueTree::Listener {

  MainComponent();
  ~MainComponent();

private:

  //============================================================================
  // Component overrides.

  void paint (Graphics&) override;
  void resized() override;
  void comboBoxChanged(ComboBox* combo) override ;
  void buttonClicked(Button* button ) override ;

  //============================================================================
  // ValueTree::Listener overrides (only valueTreePropertyChanged() is used).
  
  void valueTreePropertyChanged(ValueTree& tree, const Identifier& ident) final override;
  void valueTreeChildAdded(ValueTree& tree, ValueTree& child) final override {}
  void valueTreeChildRemoved(ValueTree& tree, ValueTree& child, int index) final override {}
  void valueTreeChildOrderChanged(ValueTree& parent, int oldIndex, int newIndex) final override {}
  void valueTreeParentChanged(ValueTree& tree) final override {}

  /// A ComboBox that dynamically populates its menu with the available MIDI
  /// outputs. Invoked each time the user clicks on the menu.
  struct MidiOutputMenu : public ComboBox {
    MainComponent& parent;
    MidiOutputMenu(MainComponent& comp) : parent (comp) {}
    void mouseDown(const MouseEvent& me) {
      // save the current selection
      String sel = getSelectedItemText();
      // remove menu's existing midi port list (they could be stale...)
      clear(NotificationType::dontSendNotification);
      // get current midi outputs
      const StringArray devs (MidiOutput::getDevices());
      addItemList(devs, 1); // item id's are 1 based
      ComboBox::mouseDown(me);
      // if user didnt select a new port use the previous one
      if (!getSelectedId())
        selectItemWithText(sel);
    }
    String getSelectedItemText() {
      auto index = getSelectedItemIndex();
      return (index >= 0) ? getItemText(index) : "";
    }
    void selectItemWithText(const String text) {
      for (auto i = 0; i < getNumItems(); ++i)
        if (getItemText(i) == text)
          return setSelectedItemIndex(i, dontSendNotification);
    }
  };

    MediaManagerData managerData;
    std::unique_ptr<Transport> transport;
    TextButton openButton{"Open..."};
    TextButton infoButton{"Media Info..."};
    MidiOutputMenu midiOutputMenu;
  
  //============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
