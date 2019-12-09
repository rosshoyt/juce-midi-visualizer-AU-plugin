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
#include "ShaderPresetManager.h"

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
    bool drawControlMesh = true;
    
    
    String newVertexShader, newFragmentShader;
private:
    
    void newOpenGLContextCreated() override
    {
        freeAllContextObjects();
        
    }
    
    void renderOpenGL() override
    {
        jassert (OpenGLHelpers::isContextActive());
        
        const float desktopScale = (float) openGLContext.getRenderingScale();
        OpenGLHelpers::clear (Colours::cadetblue);
        
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
            
            if(drawPianoKeys)
            {
                if(drawControlMesh)
                    shapePianoKey->drawControlMesh(openGLContext, *attributes);
                if(midiKeyboardState->isNoteOn(1, i))
                    shapePianoKey->draw (openGLContext, *attributes);
            }
            else
            {
                if(drawControlMesh)
                    shapeTeapot->drawControlMesh(openGLContext, *attributes);
                if(midiKeyboardState->isNoteOn(1, i))
                    shapeTeapot->draw(openGLContext, *attributes);
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
    
    //String newVertexShader, newFragmentShader;
    
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
        //const char* geometryShader;
    };
    
    static Array<ShaderPreset> getPresets()
    {

        
        ShaderPreset presets[] =
        {
            {
                "Spiral Array Shader",
                
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
                "//  gl_FragColor = vec4 (xPos, yPos, zPos, 1.0);\n"
                "    gl_FragColor = vec4 (xPos, yPos, zPos, 1.0);\n"
                "}",
            },
            {
                "Texture + Lighting",
                
                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "attribute vec4 sourceColour;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "varying float lightIntensity;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    destinationColour = sourceColour;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",
                
                SHADER_DEMO_HEADER
#if JUCE_OPENGL_ES
                "varying lowp vec4 destinationColour;\n"
                "varying lowp vec2 textureCoordOut;\n"
                "varying highp float lightIntensity;\n"
#else
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "varying float lightIntensity;\n"
#endif
                "\n"
                "uniform sampler2D demoTexture;\n"
                "\n"
                "void main()\n"
                "{\n"
#if JUCE_OPENGL_ES
                "   highp float l = max (0.3, lightIntensity * 0.3);\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
#else
                "   float l = max (0.3, lightIntensity * 0.3);\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
#endif
                "    gl_FragColor = colour * texture2D (demoTexture, textureCoordOut);\n"
                "}\n"
            },
            
            {
                "Textured",
                
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
                "void main()\n"
                "{\n"
                "    destinationColour = sourceColour;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",
                
                SHADER_DEMO_HEADER
#if JUCE_OPENGL_ES
                "varying lowp vec4 destinationColour;\n"
                "varying lowp vec2 textureCoordOut;\n"
#else
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
#endif
                "\n"
                "uniform sampler2D demoTexture;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = texture2D (demoTexture, textureCoordOut);\n"
                "}\n"
            },
            
            {
                "Flat Colour",
                
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
                "void main()\n"
                "{\n"
                "    destinationColour = sourceColour;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",
                
                SHADER_DEMO_HEADER
#if JUCE_OPENGL_ES
                "varying lowp vec4 destinationColour;\n"
                "varying lowp vec2 textureCoordOut;\n"
#else
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
#endif
                "\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = destinationColour;\n"
                "}\n"
            },
            
            {
                "Rainbow",
                
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
            },
            
            {
                "Changing Colour",
                
                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "    //(projectionMatrix + mat4(gl_InstanceID, 1, 1, 1))\n"
                "    int id = gl_InstanceID;\n"
                "}\n",
                
                SHADER_DEMO_HEADER
                "#define PI 3.1415926535897932384626433832795\n"
                "\n"
#if JUCE_OPENGL_ES
                "precision mediump float;\n"
                "varying lowp vec2 textureCoordOut;\n"
#else
                "varying vec2 textureCoordOut;\n"
#endif
                "uniform float bouncingNumber;\n"
                "\n"
                "void main()\n"
                "{\n"
                "   float b = bouncingNumber;\n"
                "   float n = b * PI * 2.0;\n"
                "   float sn = (sin (n * textureCoordOut.x) * 0.5) + 0.5;\n"
                "   float cn = (sin (n * textureCoordOut.y) * 0.5) + 0.5;\n"
                "\n"
                "   vec4 col = vec4 (b, sn, cn, 1.0);\n"
                "   gl_FragColor = col;\n"
                "}\n"
            },
            
            {
                "Simple Light",
                
                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying float lightIntensity;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",
                
                SHADER_DEMO_HEADER
#if JUCE_OPENGL_ES
                "varying highp float lightIntensity;\n"
#else
                "varying float lightIntensity;\n"
#endif
                "\n"
                "void main()\n"
                "{\n"
#if JUCE_OPENGL_ES
                "   highp float l = lightIntensity * 0.25;\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
#else
                "   float l = lightIntensity * 0.25;\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
#endif
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
            },
            
            {
                "Flattened",
                
                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying float lightIntensity;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    vec4 v = vec4 (position);\n"
                "    v.z = v.z * 0.1;\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * v;\n"
                "}\n",
                
                SHADER_DEMO_HEADER
#if JUCE_OPENGL_ES
                "varying highp float lightIntensity;\n"
#else
                "varying float lightIntensity;\n"
#endif
                "\n"
                "void main()\n"
                "{\n"
#if JUCE_OPENGL_ES
                "   highp float l = lightIntensity * 0.25;\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
#else
                "   float l = lightIntensity * 0.25;\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
#endif
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
            },
            
            {
                "Toon Shader",
                
                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying float lightIntensity;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",
                
                SHADER_DEMO_HEADER
#if JUCE_OPENGL_ES
                "varying highp float lightIntensity;\n"
#else
                "varying float lightIntensity;\n"
#endif
                "\n"
                "void main()\n"
                "{\n"
#if JUCE_OPENGL_ES
                "    highp float intensity = lightIntensity * 0.5;\n"
                "    highp vec4 colour;\n"
#else
                "    float intensity = lightIntensity * 0.5;\n"
                "    vec4 colour;\n"
#endif
                "\n"
                "    if (intensity > 0.95)\n"
                "        colour = vec4 (1.0, 0.5, 0.5, 1.0);\n"
                "    else if (intensity > 0.5)\n"
                "        colour  = vec4 (0.6, 0.3, 0.3, 1.0);\n"
                "    else if (intensity > 0.25)\n"
                "        colour  = vec4 (0.4, 0.2, 0.2, 1.0);\n"
                "    else\n"
                "        colour  = vec4 (0.2, 0.1, 0.1, 1.0);\n"
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
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
    // TOP MIDI KEYBOARD DISPLAY
    addAndMakeVisible(midiKeyboardComponent);
    
    // BOTTOM RIGHT - OPENGL DISPLAY
    glComponent = new GLComponent(midiKeyboardState, this);
    addAndMakeVisible (glComponent);
    
    // LEFT - OBJ FILE SELECTOR (RADIO BOX GROUP)
    radioButtonsObjSelector = new GroupComponent ("OBJ Selector", "Use Obj File:");
    addAndMakeVisible (radioButtonsObjSelector);
    togglePianoKeyObj = new ToggleButton("Pianokey_rectangle.obj");
    toggleTeapotObj   = new ToggleButton("Teapot.obj");
    togglePianoKeyObj->setRadioGroupId(ObjSelectorButtons);
    toggleTeapotObj  ->setRadioGroupId(ObjSelectorButtons);
    togglePianoKeyObj->onClick = [this] { updateToggleState (togglePianoKeyObj,   "Pianokey_rectangle.obj");   };
    toggleTeapotObj  ->onClick = [this] { updateToggleState (toggleTeapotObj, "Teapot.obj"); };
    addAndMakeVisible(togglePianoKeyObj);
    addAndMakeVisible(toggleTeapotObj);
    
    // LEFT - ZOOM SLIDER
    addAndMakeVisible (zoomSlider);
    zoomSlider.setRange (0.0, 1.0, 0.001);
    zoomSlider.addListener (glComponent);
    zoomSlider.setSliderStyle (Slider::LinearVertical);
    zoomLabel.attachToComponent (&zoomSlider, false);
    addAndMakeVisible (zoomLabel);
    
    // LEFT RADIO BOX: Draw Points
    toggleDrawControlMesh = new ToggleButton("Draw Points");
    toggleDrawControlMesh->onClick = [this] { updateToggleState (toggleDrawControlMesh, "Draw Points");   };
    addAndMakeVisible(toggleDrawControlMesh);
    
    
    addAndMakeVisible (shaderPresetBox);
    shaderPresetBox.onChange = [this] { selectShaderPreset (shaderPresetBox.getSelectedItemIndex()); };
    
    auto presets = ShaderPresetManager::getPresets();
    
    for (int i = 0; i < presets.size(); ++i)
        shaderPresetBox.addItem (presets[i].name, i + 1);
    
    addAndMakeVisible (shaderPresetBoxLabel);
    shaderPresetBoxLabel.attachToComponent (&shaderPresetBox, true);

    
    
    
    initialise();
    
    //setResizable(true, true);
    setSize (1000, 800);
}

GlmidipluginEditor::~GlmidipluginEditor()
{
    glComponent = nullptr;
}

void GlmidipluginEditor::initialise()
{
    togglePianoKeyObj->setToggleState(true, false);
    toggleDrawControlMesh->setToggleState(true, false);
    zoomSlider .setValue (0.5);
    //textureBox.setSelectedItemIndex (0);
    //presetBox .setSelectedItemIndex (0);
    
}

void GlmidipluginEditor::updateToggleState (Button* button, String name)
{
    if(name == "Draw Points") glComponent->drawControlMesh = button->getToggleState();
    else if(button->getToggleState())
    {
        if(name == "Pianokey_rectangle.obj") glComponent->drawPianoKeys = true;
        else if(name == "Teapot.obj") glComponent->drawPianoKeys = false;
    }
}

void GlmidipluginEditor::selectShaderPreset (int preset)
{
    const auto& p = ShaderPresetManager::getPresets()[preset];
    setShaderProgram(p.vertexShader, p.fragmentShader);
    //vertexDocument  .replaceAllContent (p.vertexShader);
    //fragmentDocument.replaceAllContent (p.fragmentShader);
    
    //startTimer (1);
}

void  GlmidipluginEditor::setShaderProgram (const String& vertexShader, const String& fragmentShader)
{
    DBG("In SetShaderProgram");
    glComponent->newVertexShader = vertexShader;
    glComponent->newFragmentShader = fragmentShader;
}
//==============================================================================

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
    // TODO Optimize Resizing (or skip resizeable
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
    togglePianoKeyObj->setBounds(getSubdividedRegion(radioObjSelectorRegion, 1, 3, Vertical));//
    toggleTeapotObj->setBounds (getSubdividedRegion(radioObjSelectorRegion, 2, 3, Vertical));
    
    int sliderRegionHeight = leftButtonToolbarArea.getHeight() / 4 + BUTTON_HEIGHT;
    auto sliderRegion1 = leftButtonToolbarArea.removeFromTop(sliderRegionHeight);
    zoomSlider.setBounds(sliderRegion1.removeFromBottom(sliderRegion1.getHeight() - BUTTON_HEIGHT));
    zoomSlider.setTextBoxStyle (Slider::TextBoxBelow, false, BUTTON_WIDTH, 20);
    
    toggleDrawControlMesh->setBounds(leftButtonToolbarArea.removeFromTop(radioObjSelectorRegion.getHeight()));
    
    shaderPresetBox.setBounds(leftButtonToolbarArea.removeFromTop(BUTTON_HEIGHT * 3));
    
}
