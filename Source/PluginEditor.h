/*
  ==============================================================================
Ross Hoyt
PluginEditor.h
The Plugin Window, and the GL Component are declared here.
They are defined PluginEditor.h

==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED


#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "GLUtils.h"


//==============================================================================
/**
*/
class GLComponent; // forward declared - implemented in CPP file

class PluginEditor  : public AudioProcessorEditor
{
public:
    PluginEditor (PluginProcessor&, MidiKeyboardState&);
    ~PluginEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    // SLIDERS FOR GL WINDOW PARAMS THAT NEED TO BE ACCESSED BY CHILDREN
    Slider zoomSlider;
    Slider rotationSlider;
    
    // SHADER SELECTOR
    ComboBox shaderPresetBox;
    
private:
    
    
    // RADIO BUTTONS:
    enum RadioButtonIds { ObjSelectorButtons = 1001 };
    // RB->OBJ SELECTOR RADIO BOX
    GroupComponent* radioButtonsObjSelector;
    ToggleButton* togglePianoKeyObj;
    ToggleButton* toggleTeapotObj;
    
    
    // OBJ SELECTOR RADIO BOX
    ToggleButton* toggleDrawControlMesh;
    //ToggleButton* toggleButton_TeapotObj;
    

    // LABELS
    Label zoomLabel  { {}, "Zoom:" };
    Label shaderPresetBoxLabel;
    //Label rotationLabel { {}, "Rotation:" };
    
    
    void initialise();
    
    void updateToggleState(Button*, String);
    
    void selectShaderPreset(int);
    void setShaderProgram(const String&, const String&);
    void updateShader();
    
    // class that created this class
    PluginProcessor& processor;
    

    MidiKeyboardComponent midiKeyboardComponent;
    MidiKeyboardState * midiKeyboardState;
    
    ScopedPointer<GLComponent> glComponent;
    
    // SHADERS
    const char * SHADERS_ABS_DIR_PATH = "/Users/RossHoyt/Downloads/GLPlugin/Source";
    Array<ShaderPreset> shaders;
    void loadShaders(const char*);
    
    // GUI constants
    static const int MARGIN = 4, MAX_WINDOW_HEIGHT = 800, MAX_WINDOW_WIDTH = 1200 + 2 * MARGIN,
    MAX_KEYB_WIDTH = 1200, MAX_KEYB_HEIGHT = 82, BUTTON_WIDTH = 50, BUTTON_HEIGHT = 30;
    Colour backgroundColor { 44, 54, 60 };
    
    // Display Helper Method
    enum SubdividedOrientation { Vertical, Horizontal};
    static Rectangle<int> getSubdividedRegion(const Rectangle<int>, int, int, SubdividedOrientation);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
