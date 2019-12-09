/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "WavefrontObjParser.h"
#include "Shape.h"
#include "Textures.h"
#include "Uniforms.h"
#include "Attributes.h"


//==============================================================================

class GLComponent : public Component, private OpenGLRenderer, public Slider::Listener
{
public:
    GLComponent(MidiKeyboardState &mKeybState, GlmidipluginEditor *par) :
                        //allwaysDisplayKeysButton("Always Display Keys"),
                        rotation (0.0f), scale (0.5f), rotationSpeed (0.1f),
        textureToUse (nullptr), lastTexture (nullptr)
    {
        parent = par;
        
        midiKeyboardState = &mKeybState;
        Array<ShaderPreset> shaders = getPresets();
        if (shaders.size() > 0)
        {
            newVertexShader = shaders.getReference (0).vertexShader;
            newFragmentShader = shaders.getReference (0).fragmentShader;
        }
        
        lastTexture = textureToUse = new BuiltInTexture ("Portmeirion", BinaryData::portmeirion_jpg, BinaryData::portmeirion_jpgSize);
        
        openGLContext.setComponentPaintingEnabled (false);
        openGLContext.setContinuousRepainting (true);
            
        openGLContext.setRenderer (this);
        openGLContext.attachTo (*this);
    }
    
    ~GLComponent()
    {
        openGLContext.detach();
        openGLContext.setRenderer (nullptr);
        
        if (lastTexture != nullptr)
            delete lastTexture;
    }
    
    bool drawPianoKeys = true;
private:
    
    
    void newOpenGLContextCreated() override
    {
        freeAllContextObjects();
    }
    
    void renderOpenGL() override
    {
        jassert (OpenGLHelpers::isContextActive());
        
        const float desktopScale = (float) openGLContext.getRenderingScale();
        OpenGLHelpers::clear (Colours::burlywood);
        
        updateShader();   // Check whether we need to compile a new shader
        
        if (shader == nullptr)
            return;
        
        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        openGLContext.extensions.glActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);
        
        glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
        texture.bind();
        
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        shader->use();
        
        if (uniforms->projectionMatrix != nullptr)
            uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
        
        if (uniforms->texture != nullptr)
            uniforms->texture->set ((GLint) 0);
        
        if (uniforms->lightPosition != nullptr)
            uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);
        
        
        for(int i = 0; i < 128; i++)
        {
            // TODO could add automatic MidiChannel Sensing
                if (uniforms->viewMatrix != nullptr)
                uniforms->viewMatrix->setMatrix4 (getViewMatrix(i).mat, 1, false);
            
            if(midiKeyboardState->isNoteOn(1, i)){
                if(drawPianoKeys) shapePianoKey->draw (openGLContext, *attributes);
                else              shapeTeapot  ->draw (openGLContext, *attributes);
            }
            
        }
        
        // Reset the element buffers so child Components draw correctly
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
        openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
        
        rotation += (float) rotationSpeed;
    }
    
    void openGLContextClosing() override
    {
        freeAllContextObjects();
        
        if (lastTexture != nullptr)
            setTexture (lastTexture);
    }
    
    void paint(Graphics& g) override {}
    
    void setTexture (DemoTexture* t)
    {
        lastTexture = textureToUse = t;
    }
    
    void freeAllContextObjects()
    {
        shapePianoKey = nullptr;
        shader = nullptr;
        attributes = nullptr;
        uniforms = nullptr;
        texture.release();
    }
    
    Draggable3DOrientation draggableOrientation;
    float rotation;
    float scale, rotationSpeed;
    
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<Shape> shapePianoKey;
    ScopedPointer<Shape> shapeTeapot;
    
    ScopedPointer<Attributes> attributes;
    ScopedPointer<Uniforms> uniforms;
    
    OpenGLTexture texture;
    DemoTexture* textureToUse, *lastTexture;
    
    String newVertexShader, newFragmentShader;
    
    OpenGLContext openGLContext;
    
    Matrix3D<float> getProjectionMatrix() const
    {
        //auto w = 1.0f / (scale + 0.1f);
        auto w = 1.0f / (scale + 0.1f);
        auto h = w * getLocalBounds().toFloat().getAspectRatio (false);
        
        return Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
        //return Matrix3D<float>::fromFrustum (-1000.0f,-750.0f, -h, h, 4.0f, 30.0f);
    }
    Matrix3D<float> getViewMatrix(int i) const
    {
        int fundamentalPitch = i % 12;
        auto viewMatrix = draggableOrientation.getRotationMatrix()
        * Vector3D<float> (0.0f, 1.0f, -10.0f);
        
        float angle = 360.0f / 12.0f;
        float rotY = degreesToRadians(angle) * ((fundamentalPitch + 9) % 12);
        auto rotationMatrix = Matrix3D<float>::rotation ({ 1.55f, rotY, -1.55f });
        
        
        float cylinderRadius = 2.0f;
        //x2 = x1 + cylinderRadius * cos(degreesToRadians(fundamentalPitch * angle));
        //y2 = y1 + cylinderRadius * sin(degreesToRadians(fundamentalPitch * angle));
        
        
        float translateY = i * -.5f + 30.0f;
        
        float translateX = cylinderRadius * sin(degreesToRadians(fundamentalPitch * angle));
        float translateZ = cylinderRadius * cos(degreesToRadians(fundamentalPitch * angle));
        
        
        
        auto translationVector = Matrix3D<float>(Vector3D<float> ({ translateX, translateY, translateZ}));
                //Matrix3D<float> ({ 0, translateY, 0 });
        
        
        //auto translationMatrix2 = Matrix3D<float> ({ translateX, 0, translateZ});
        return  rotationMatrix * translationVector * viewMatrix;
        
    }
//    Matrix3D<float> getProjectionMatrix() const
//    {
//        float w = 1.0f / (scale + 0.1f);
//        float h = w * getLocalBounds().toFloat().getAspectRatio (false);
//        return Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
//    }
//
//    Matrix3D<float> getViewMatrix() const
//    {
//        Matrix3D<float> viewMatrix = draggableOrientation.getRotationMatrix()
//        * Vector3D<float> (0.0f, 1.0f, -10.0f);
//
//        //Matrix3D<float> rotationMatrix = viewMatrix.rotated (Vector3D<float> (rotation, rotation, -0.3f));
//
//        return //rotationMatrix *
//            viewMatrix;
//    }
    
    void resized() override
    {
        draggableOrientation.setViewport (getLocalBounds());
    }
    
    //==============================================================================
    void updateShader()
    {
        if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
        {
            ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
            String statusText;
            
            if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (newVertexShader))
                && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (newFragmentShader))
                && newShader->link())
            {
                shapePianoKey = nullptr;
                shapeTeapot = nullptr;
                attributes = nullptr;
                uniforms = nullptr;
                
                shader = newShader;
                shader->use();
                
                shapePianoKey = new Shape (openGLContext, BinaryData::pianokey_rectangle_obj);
                shapeTeapot   = new Shape (openGLContext, BinaryData::teapot_obj);
                attributes = new Attributes (openGLContext, *shader);
                uniforms   = new Uniforms (openGLContext, *shader);
                
                statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
            }
            else
            {
                statusText = newShader->getLastError();
            }
            
            newVertexShader = String();
            newFragmentShader = String();
        }
    }
    
    //==============================================================================
    struct ShaderPreset
    {
        const char* name;
        const char* vertexShader;
        const char* fragmentShader;
    };
    
    static Array<ShaderPreset> getPresets()
    {
#define SHADER_DEMO_HEADER \
"/*  This is a live OpenGL Shader demo.\n" \
"    Edit the shader program below and it will be \n" \
"    compiled and applied to the model above!\n" \
"*/\n\n"
        
        ShaderPreset presets[] =
        {
            {
                "Spiral Array Shader",
                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 sourceColour;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "varying float xPos;\n"
                "varying float yPos;\n"
                "varying float zPos;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 v = vec4 (position);\n"
                "    xPos = clamp (v.x, 0.0, 1.0);\n"
                "    yPos = clamp (v.y, 0.0, 1.0);\n"
                "    zPos = clamp (v.z, 0.0, 1.0);\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}",
                
                SHADER_DEMO_HEADER
#if JUCE_OPENGL_ES
                "varying lowp vec4 destinationColour;\n"
                "varying lowp vec2 textureCoordOut;\n"
                "varying lowp float xPos;\n"
                "varying lowp float yPos;\n"
                "varying lowp float zPos;\n"
#else
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "varying float xPos;\n"
                "varying float yPos;\n"
                "varying float zPos;\n"
#endif
                "\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = vec4 (xPos, yPos, zPos, 1.0);\n"
                "}"
//                ,
//                "Simple Light",
//
//                SHADER_DEMO_HEADER
//                "attribute vec4 position;\n"
//                "attribute vec4 normal;\n"
//                "\n"
//                "uniform mat4 projectionMatrix;\n"
//                "uniform mat4 viewMatrix;\n"
//                "uniform vec4 lightPosition;\n"
//                "\n"
//                "varying float lightIntensity;\n"
//                "\n"
//                "void main()\n"
//                "{\n"
//                "    vec4 light = viewMatrix * lightPosition;\n"
//                "    lightIntensity = dot (light, normal);\n"
//                "\n"
//                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
//                "}\n",
//
//                SHADER_DEMO_HEADER
//#if JUCE_OPENGL_ES
//                "varying highp float lightIntensity;\n"
//#else
//                "varying float lightIntensity;\n"
//#endif
//                "\n"
//                "void main()\n"
//                "{\n"
//#if JUCE_OPENGL_ES
//                "   highp float l = lightIntensity * 0.25;\n"
//                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
//#else
//                "   float l = lightIntensity * 0.25;\n"
//                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
//#endif
//                "\n"
//                "    gl_FragColor = colour;\n"
//                "}\n"
            }
        };
        
        return Array<ShaderPreset> (presets, numElementsInArray (presets));
    }
    
    // INPUT HANDLING
    
    void sliderValueChanged (Slider*) override
    {
        scale = (float) parent->zoomSlider.getValue();
        //demo.rotationSpeed = (float) speedSlider.getValue();
    }
    
    void mouseDown (const MouseEvent& e) override
    {
        draggableOrientation.mouseDown (e.getPosition());
    }
    
    void mouseDrag (const MouseEvent& e) override
    {
        draggableOrientation.mouseDrag (e.getPosition());
    }
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& d) override
    {
        parent->zoomSlider.setValue(parent->zoomSlider.getValue() + d.deltaY);
    }
    MidiKeyboardState * midiKeyboardState;
    GlmidipluginEditor *parent;
};


//==============================================================================
GlmidipluginEditor::GlmidipluginEditor (GlmidipluginProcessor& p, MidiKeyboardState& midiKeyboardState)
: AudioProcessorEditor (&p), processor (p), midiKeyboardComponent(midiKeyboardState, MidiKeyboardComponent::horizontalKeyboard)
{
    // MIDI KEYBOARD DISPLAY
    addAndMakeVisible(midiKeyboardComponent);
    
    // OPENGL DISPLAY
    glComponent = new GLComponent(midiKeyboardState, this);
    addAndMakeVisible (glComponent);
    
    // OBJ FILE SELECTOR (RADIO BOX)
    radioButtonsObjSelector = new GroupComponent ("OBJ Selector", "Use Obj File:");
    addAndMakeVisible (radioButtonsObjSelector);
    toggleButton_PianoKeyRectObj = new ToggleButton("Pianokey_rectangle.obj");
    toggleButton_TeapotObj = new ToggleButton("Teapot.obj");
    toggleButton_PianoKeyRectObj->setRadioGroupId(ObjSelectorButtons);
    toggleButton_TeapotObj      ->setRadioGroupId(ObjSelectorButtons);
    toggleButton_PianoKeyRectObj->onClick = [this] { updateToggleState (toggleButton_PianoKeyRectObj,   "Pianokey_rectangle.obj");   };
    toggleButton_TeapotObj      ->onClick = [this] { updateToggleState (toggleButton_TeapotObj, "Teapot.obj"); };
    addAndMakeVisible(toggleButton_PianoKeyRectObj);
    addAndMakeVisible(toggleButton_TeapotObj);
    
    // ZOOM SLIDER
    addAndMakeVisible (zoomSlider);
    zoomSlider.setRange (0.0, 1.0, 0.001);
    zoomSlider.addListener (glComponent);
    zoomSlider.setSliderStyle(Slider::LinearVertical);
    zoomLabel.attachToComponent (&zoomSlider, false);
    addAndMakeVisible (zoomLabel);
    
    // ROTATION SLIDER
    //addAndMakeVisible(rotationSlider);
    
    
    
    initialise();
    
    setResizable(true, true);
    setSize (1000, 800);
}

GlmidipluginEditor::~GlmidipluginEditor()
{
    glComponent = nullptr;
}

void GlmidipluginEditor::initialise()
{
    toggleButton_PianoKeyRectObj->setToggleState(true, false);
    zoomSlider .setValue (0.5);
    //textureBox.setSelectedItemIndex (0);
    //presetBox .setSelectedItemIndex (0);
    
}

void GlmidipluginEditor::updateToggleState (Button* button, String name)
{

    //auto stateOn = button->getToggleState();
    if(button->getToggleState())
    {
        if(name == "Pianokey_rectangle.obj") glComponent->drawPianoKeys = true;
        else if(name == "Teapot.obj") glComponent->drawPianoKeys = false;
    }
    
    //String stateString = state ? "ON" : "OFF";
    //Logger::outputDebugString (name + " Button changed to " + stateString);
}
//==============================================================================
void GlmidipluginEditor::paint (Graphics& g)
{
    g.fillAll(backgroundColor);
}

Rectangle<int> GlmidipluginEditor::getSubdividedRegion(Rectangle<int> region, int numer, int denom, SubdividedOrientation orientation)
{
    int x, y, height, width;
    if(orientation == Vertical)
    {
        x = region.getX();
        width = region.getWidth();
        height = region.getHeight() / denom;
        y = region.getY() + numer * height;
    }
    else
    {
        y = region.getY();
        height = region.getHeight();
        width = region.getWidth() / denom;
        x = region.getX() + numer * width;
    }
    return Rectangle<int>(x, y, width, height);
}

void GlmidipluginEditor::resized()
{
    Rectangle<int> r = getLocalBounds();
    float resizedKeybWidth = r.getWidth() - MARGIN * 2, resizedKeybHeight = r.getHeight() - 5;
    float keybWidth = resizedKeybWidth > MAX_KEYB_WIDTH ? MAX_KEYB_WIDTH : resizedKeybWidth;
    float keybHeight = resizedKeybHeight > MAX_KEYB_HEIGHT ? MAX_KEYB_HEIGHT : resizedKeybHeight;
    midiKeyboardComponent.setBounds (MARGIN, MARGIN, keybWidth, keybHeight );
    
    auto areaBelowKeyboard = r.removeFromBottom(r.getHeight() - (keybHeight + MARGIN));
    
    int leftToolbarWidth = r.getWidth() / 7;
    
    auto glArea = areaBelowKeyboard.removeFromRight(r.getWidth() - leftToolbarWidth);
    glComponent->setBounds(glArea);
    
    auto leftButtonToolbarArea = areaBelowKeyboard.removeFromLeft(glArea.getWidth());
    auto radioObjSelectorRegion = leftButtonToolbarArea.removeFromTop((BUTTON_HEIGHT * 2 + MARGIN*3));
    radioObjSelectorRegion.translate(0, MARGIN);
    radioButtonsObjSelector->setBounds (radioObjSelectorRegion);//MARGIN, keybHeight + MARGIN, 220, 140);
    toggleButton_PianoKeyRectObj->setBounds(getSubdividedRegion(radioObjSelectorRegion, 1, 3, Vertical));//
    toggleButton_TeapotObj->setBounds (getSubdividedRegion(radioObjSelectorRegion, 2, 3, Vertical));
    
    int sliderRegionHeight = leftButtonToolbarArea.getHeight() / 4 + BUTTON_HEIGHT;
    auto sliderRegion1 = leftButtonToolbarArea.removeFromTop(sliderRegionHeight);
    zoomSlider.setBounds(sliderRegion1.removeFromBottom(sliderRegion1.getHeight() - BUTTON_HEIGHT));
    zoomSlider.setTextBoxStyle (Slider::TextBoxBelow, false, BUTTON_WIDTH, 20);
    
}
