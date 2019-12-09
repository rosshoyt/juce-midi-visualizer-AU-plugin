/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class GLComponent;

class GlpluginAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    GlpluginAudioProcessorEditor (GlpluginAudioProcessor&, MidiKeyboardState&);
    ~GlpluginAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    // SLIDERS FOR GL WINDOW PARAMS THAT NEED TO BE ACCESSED BY CHILDREN
    Slider zoomSlider;
    Slider rotationSlider;
private:
    // OBJ SELECTOR RADIO BOX
    GroupComponent* radioButtonsObjSelector;
    ToggleButton* toggleButton_PianoKeyRectObj;
    ToggleButton* toggleButton_TeapotObj;
    enum RadioButtonIds { ObjSelectorButtons = 1001 };
    
    // SLIDER LABELS
    Label zoomLabel  { {}, "Zoom:" };
    Label rotationLabel { {}, "Rotation:" };
    
    
    void initialise();
    
    void updateToggleState(Button*, String);
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GlpluginAudioProcessor& processor;
    

    MidiKeyboardComponent midiKeyboardComponent;
    MidiKeyboardState * midiKeyboardState;
    
    ScopedPointer<GLComponent> glComponent;
    
    // Display Constants
    static const int MARGIN = 4, MAX_WINDOW_HEIGHT = 800, MAX_WINDOW_WIDTH = 1200 + 2 * MARGIN,
    MAX_KEYB_WIDTH = 1200, MAX_KEYB_HEIGHT = 82, BUTTON_WIDTH = 50, BUTTON_HEIGHT = 30;
    
    // Display Helper Method
    enum SubdividedOrientation { Vertical, Horizontal};
    static Rectangle<int> getSubdividedRegion(const Rectangle<int>, int, int, SubdividedOrientation);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlpluginAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
