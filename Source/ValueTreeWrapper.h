//==============================================================================
/// @file ValueTreeWrapper.h
/// Class wrapper enables getter and setter methods to be defined on ValueTrees
/// rather than using the more clunky JUCE getProperty() & setProperty() APIs.

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/** A base class for wrapping a value tree. Subclass this to
    define a value tree that provides getters and setters rather
    than calling getProperty() and setProperty() to access data.
    For each property in your tree provide juce Identifiers for
    property names and one for your subclass's type id. Example:
    @code
    struct MyData : public ValueTreeWapper {
      static const juce::Identifier MY_TREE_TYPEID ("MyData");
      static const juce::Identifier GLYPH_FONT_SIZE ("glyphSize");
      MyData()
        : ValueTreeWrapper(juce::ValueTree(MY_TREE_TYPEID)) {
      }
      int getFontSize() const {
        return data.getProperty(GLYPH_FONT_SIZE_PROPID, 12);
      }
    };
    @endcode
 */

struct ValueTreeWrapper {
  mutable juce::ValueTree data;
  ValueTreeWrapper(const juce::Identifier& id) : data(id) {
  }
  ValueTreeWrapper(const juce::ValueTree& data) : data(data) {
  }
  /// add a listener
  void addListener(juce::ValueTree::Listener* listener) {
    data.addListener(listener);
  }
  /// remove a listener
  void removeListener(juce::ValueTree::Listener* listener) {
    data.removeListener(listener);
  }
  /// conversion operator
  operator juce::ValueTree() {
	  return data;
  }
  /// conversion operator
  operator juce::ValueTree() const {
	  return data;
  }
  /// Returns true if the data is a valid ValueTree (has a TYPEID).
  bool isValid() const {
	  return data.isValid();
  }
  /// Removes all properties and subtrees.
  void clear () {
    data.removeAllProperties(nullptr);
    data.removeAllChildren(nullptr);
  }
};
