/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "WavefrontObjParser.h"

//==============================================================================
struct OpenGLDemoClasses
{
    /** Vertex data to be passed to the shaders.
     For the purposes of this demo, each vertex will have a 3D position, a colour and a
     2D texture co-ordinate. Of course you can ignore these or manipulate them in the
     shader programs but are some useful defaults to work from.
     */
    struct Vertex
    {
        float position[3];
        float normal[3];
        float colour[4];
        float texCoord[2];
    };
    
    //==============================================================================
    // This class just manages the attributes that the demo shaders use.
    struct Attributes
    {
        Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
        {
            position      = createAttribute (openGLContext, shader, "position");
            normal        = createAttribute (openGLContext, shader, "normal");
            sourceColour  = createAttribute (openGLContext, shader, "sourceColour");
            texureCoordIn = createAttribute (openGLContext, shader, "texureCoordIn");
        }
        
        void enable (OpenGLContext& openGLContext)
        {
            if (position != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
                openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
            }
            
            if (normal != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
                openGLContext.extensions.glEnableVertexAttribArray (normal->attributeID);
            }
            
            if (sourceColour != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
                openGLContext.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
            }
            
            if (texureCoordIn != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (texureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
                openGLContext.extensions.glEnableVertexAttribArray (texureCoordIn->attributeID);
            }
        }
        
        void disable (OpenGLContext& openGLContext)
        {
            if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
            if (normal != nullptr)         openGLContext.extensions.glDisableVertexAttribArray (normal->attributeID);
            if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
            if (texureCoordIn != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (texureCoordIn->attributeID);
        }
        
        ScopedPointer<OpenGLShaderProgram::Attribute> position, normal, sourceColour, texureCoordIn;
        
    private:
        static OpenGLShaderProgram::Attribute* createAttribute (OpenGLContext& openGLContext,
                                                                OpenGLShaderProgram& shader,
                                                                const char* attributeName)
        {
            if (openGLContext.extensions.glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
                return nullptr;
            
            return new OpenGLShaderProgram::Attribute (shader, attributeName);
        }
    };
    
    //==============================================================================
    // This class just manages the uniform values that the demo shaders use.
    struct Uniforms
    {
        Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
        {
            projectionMatrix = createUniform (openGLContext, shader, "projectionMatrix");
            viewMatrix       = createUniform (openGLContext, shader, "viewMatrix");
            texture          = createUniform (openGLContext, shader, "demoTexture");
            lightPosition    = createUniform (openGLContext, shader, "lightPosition");
        }
        
        ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, texture, lightPosition;
        
    private:
        static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
                                                            OpenGLShaderProgram& shader,
                                                            const char* uniformName)
        {
            if (openGLContext.extensions.glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
                return nullptr;
            
            return new OpenGLShaderProgram::Uniform (shader, uniformName);
        }
    };
    
    struct DemoTexture
    {
        virtual ~DemoTexture() {}
        virtual bool applyTo (OpenGLTexture&) = 0;
        
        String name;
    };
    
    struct BuiltInTexture   : public DemoTexture
    {
        BuiltInTexture (const char* nm, const void* imageData, size_t imageSize)
        : image (resizeImageToPowerOfTwo (ImageFileFormat::loadFrom (imageData, imageSize)))
        {
            name = nm;
        }
        
        Image image;
        
        bool applyTo (OpenGLTexture& texture) override
        {
            texture.loadImage (image);
            return false;
        }
    };
    
    static Image resizeImageToPowerOfTwo (Image image)
    {
        if (! (isPowerOfTwo (image.getWidth()) && isPowerOfTwo (image.getHeight())))
            return image.rescaled (jmin (1024, nextPowerOfTwo (image.getWidth())),
                                   jmin (1024, nextPowerOfTwo (image.getHeight())));
        
        return image;
    }
    
    //==============================================================================
    /** This loads a 3D model from an OBJ file and converts it into some vertex buffers
     that we can draw.
     */
    struct Shape
    {
        Shape (OpenGLContext& openGLContext, const String& objFileContent)
        {
            if (shapeFile.load (objFileContent).wasOk())
                for (int i = 0; i < shapeFile.shapes.size(); ++i)
                    vertexBuffers.add (new VertexBuffer (openGLContext, *shapeFile.shapes.getUnchecked(i)));
            
        }
        
        void draw (OpenGLContext& openGLContext, Attributes& attributes)
        {
            for (int i = 0; i < vertexBuffers.size(); ++i)
            {
                VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
                vertexBuffer.bind();
                
                attributes.enable (openGLContext);
                glDrawElements (GL_TRIANGLES, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
                attributes.disable (openGLContext);
            }
        }
        
    private:
        struct VertexBuffer
        {
            VertexBuffer (OpenGLContext& context, WavefrontObjFile::Shape& shape) : openGLContext (context)
            {
                numIndices = shape.mesh.indices.size();
                
                openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
                
                Array<Vertex> vertices;
                createVertexListFromMesh (shape.mesh, vertices, Colours::green);
                
                openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER, vertices.size() * (int) sizeof (Vertex),
                                                       vertices.getRawDataPointer(), GL_STATIC_DRAW);
                
                openGLContext.extensions.glGenBuffers (1, &indexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * (int) sizeof (juce::uint32),
                                                       shape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
            }
            
            ~VertexBuffer()
            {
                openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
                openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
            }
            
            void bind()
            {
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            }
            
            GLuint vertexBuffer, indexBuffer;
            int numIndices;
            OpenGLContext& openGLContext;
            
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
        };
        
        WavefrontObjFile shapeFile;
        OwnedArray<VertexBuffer> vertexBuffers;
        
        static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Colour colour)
        {
            const float scale = 0.2f;
            WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
            WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };
            
            for (int i = 0; i < mesh.vertices.size(); ++i)
            {
                const WavefrontObjFile::Vertex& v = mesh.vertices.getReference (i);
                
                const WavefrontObjFile::Vertex& n
                = i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;
                
                const WavefrontObjFile::TextureCoord& tc
                = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTexCoord;
                
                Vertex vert =
                {
                    { scale * v.x, scale * v.y, scale * v.z, },
                    { scale * n.x, scale * n.y, scale * n.z, },
                    { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                    { tc.x, tc.y }
                };
                
                list.add (vert);
            }
        }
    };
    
};

class GLComponent : public Component, private OpenGLRenderer
{
public:
    GLComponent(MidiKeyboardState &mKeybState) :
                        //allwaysDisplayKeysButton("Always Display Keys"),
                        rotation (0.0f), scale (0.5f), rotationSpeed (0.1f),
        textureToUse (nullptr), lastTexture (nullptr)
    {
        
        
        midiKeyboardState = &mKeybState;
        Array<ShaderPreset> shaders = getPresets();
        if (shaders.size() > 0)
        {
            newVertexShader = shaders.getReference (0).vertexShader;
            newFragmentShader = shaders.getReference (0).fragmentShader;
        }
        
        lastTexture = textureToUse = new OpenGLDemoClasses::BuiltInTexture ("Portmeirion", BinaryData::portmeirion_jpg, BinaryData::portmeirion_jpgSize);
        
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
        OpenGLHelpers::clear (Colours::black);
        
        updateShader();   // Check whether we need to compile a new shader
        
        if (shader == nullptr)
            return;
        
        // Having used the juce 2D renderer, it will have messed-up a whole load of GL state, so
        // we need to initialise some important settings before doing our normal GL 3D drawing..
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
        for(int i = 0; i < 128; i++){
            
            if (uniforms->projectionMatrix != nullptr)
                uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
            
            if (uniforms->viewMatrix != nullptr)
                uniforms->viewMatrix->setMatrix4 (getViewMatrix(i).mat, 1, false);
            
            if (uniforms->texture != nullptr)
                uniforms->texture->set ((GLint) 0);
            
            if (uniforms->lightPosition != nullptr)
                uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);
            
            // TODO could add automatic MidiChannel Sensing
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
    
    void setTexture (OpenGLDemoClasses::DemoTexture* t)
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
    ScopedPointer<OpenGLDemoClasses::Shape> shapePianoKey;
    ScopedPointer<OpenGLDemoClasses::Shape> shapeTeapot;
    
    ScopedPointer<OpenGLDemoClasses::Attributes> attributes;
    ScopedPointer<OpenGLDemoClasses::Uniforms> uniforms;
    
    OpenGLTexture texture;
    OpenGLDemoClasses::DemoTexture* textureToUse, *lastTexture;
    
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
    const float DegreesToRadians = 3.14159265358f/180.f;
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
                
                shapePianoKey = new OpenGLDemoClasses::Shape (openGLContext, BinaryData::pianokey_rectangle_obj);
                shapeTeapot   = new OpenGLDemoClasses::Shape (openGLContext, BinaryData::teapot_obj);
                attributes = new OpenGLDemoClasses::Attributes (openGLContext, *shader);
                uniforms   = new OpenGLDemoClasses::Uniforms (openGLContext, *shader);
                
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
        scale += d.deltaY;
    }
    MidiKeyboardState * midiKeyboardState;
    
};


//==============================================================================
GlpluginAudioProcessorEditor::GlpluginAudioProcessorEditor (GlpluginAudioProcessor& p, MidiKeyboardState& midiKeyboardState)
: AudioProcessorEditor (&p), processor (p), midiKeyboardComponent(midiKeyboardState, MidiKeyboardComponent::horizontalKeyboard)
{
    glComponent = new GLComponent(midiKeyboardState);
    
    radioButtonsObjSelector = new GroupComponent ("OBJ Selector", "Use Obj File:");
    addAndMakeVisible (radioButtonsObjSelector);
    toggleButton_PianoKeyRectObj = new ToggleButton("Pianokey_rectangle.obj");
    toggleButton_TeapotObj = new ToggleButton("Teapot.obj");
    toggleButton_PianoKeyRectObj->setRadioGroupId(ObjSelectorButtons);
    toggleButton_TeapotObj      ->setRadioGroupId(ObjSelectorButtons);
    addAndMakeVisible(toggleButton_PianoKeyRectObj);
    addAndMakeVisible(toggleButton_TeapotObj);
    
    
    toggleButton_PianoKeyRectObj->onClick = [this] { updateToggleState (toggleButton_PianoKeyRectObj,   "Pianokey_rectangle.obj");   };
    toggleButton_TeapotObj      ->onClick = [this] { updateToggleState (toggleButton_TeapotObj, "Teapot.obj"); };
    
    toggleButton_PianoKeyRectObj->setToggleState(true, false);
    
    addAndMakeVisible (glComponent);
    addAndMakeVisible(midiKeyboardComponent);
    

    setSize (1000, 800);
}

GlpluginAudioProcessorEditor::~GlpluginAudioProcessorEditor()
{
    glComponent = nullptr;
}
void GlpluginAudioProcessorEditor::updateToggleState (Button* button, String name)
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
void GlpluginAudioProcessorEditor::paint (Graphics& g)
{}

Rectangle<int> GlpluginAudioProcessorEditor::getSubdividedRegion(Rectangle<int> region, int numer, int denom)
{
    int newHeight = region.getHeight() / denom;
    return Rectangle<int>(region.getX(), region.getY() + numer * newHeight, region.getWidth(), newHeight);
}

void GlpluginAudioProcessorEditor::resized()
{
    Rectangle<int> r = getLocalBounds();
    float resizedKeybWidth = r.getWidth() - MARGIN * 2, resizedKeybHeight = r.getHeight() - 5;
    float keybWidth = resizedKeybWidth > MAX_KEYB_WIDTH ? MAX_KEYB_WIDTH : resizedKeybWidth;
    float keybHeight = resizedKeybHeight > MAX_KEYB_HEIGHT ? MAX_KEYB_HEIGHT : resizedKeybHeight;
    midiKeyboardComponent.setBounds (MARGIN, MARGIN, keybWidth, keybHeight );
    //midiKeyboardComponent.setBounds(r.removeFromTop())
    //btn.setBounds (r.removeFromTop (r.getHeight() >> 1));
    //glComponent->setBounds (0, r.getY() - keybHeight, r.getWidth(), r.getHeight());
    auto areaBelowKeyboard = r.removeFromBottom(r.getHeight() - (keybHeight + MARGIN));
    
    int leftToolbarWidth = r.getWidth() / 7;
    
    auto glArea = areaBelowKeyboard.removeFromRight(r.getWidth() - leftToolbarWidth);
    glComponent->setBounds(glArea);
    
    auto leftButtonToolbarArea = areaBelowKeyboard.removeFromLeft(glArea.getWidth());
    auto radioObjSelectorRegion = leftButtonToolbarArea.removeFromTop((BUTTON_HEIGHT * 2 + MARGIN*3));
    radioObjSelectorRegion.translate(0, MARGIN);
    radioButtonsObjSelector->setBounds (radioObjSelectorRegion);//MARGIN, keybHeight + MARGIN, 220, 140);
    toggleButton_PianoKeyRectObj->setBounds(getSubdividedRegion(radioObjSelectorRegion, 1, 3));//
    toggleButton_TeapotObj->setBounds (getSubdividedRegion(radioObjSelectorRegion, 2, 3));
    //auto toggleButtonArea = radioObjSelectorRegion.withTrimmedBottom(radioObjSelectorRegion.getHeight() /2);
    //toggleButton_PianoKeyRectObj->setBounds(toggleButtonArea.removeFromTop(MARGIN));//radioObjSelectorRegion.removeFromTop(MARGIN));
    //toggleButtonArea.setTop(toggleButtonArea.getY() + BUTTON_HEIGHT);
    //toggleButton_TeapotObj->setBounds (toggleButtonArea);//radioObjSelectorRegion.withTrimmedTop(BUTTON_HEIGHT + 2 * MARGIN));
}
