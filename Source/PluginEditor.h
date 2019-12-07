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
    GlpluginAudioProcessorEditor (GlpluginAudioProcessor&);
    ~GlpluginAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GlpluginAudioProcessor& processor;

    TextButton btn;
    ScopedPointer<GLComponent> glComponent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlpluginAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
