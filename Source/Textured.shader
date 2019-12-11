#shader preset display name Textured
#shader vertex



attribute vec4 position;
attribute vec4 sourceColour;
attribute vec2 textureCoordIn;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

varying vec4 destinationColour;
varying vec2 textureCoordOut;

void main()
{
    destinationColour = sourceColour;
    textureCoordOut = textureCoordIn;
    gl_Position = projectionMatrix * viewMatrix * position;
}

#shader fragment

varying vec4 destinationColour;
varying vec2 textureCoordOut;


uniform sampler2D demoTexture;

void main()
{
    gl_FragColor = texture2D (demoTexture, textureCoordOut);
}
