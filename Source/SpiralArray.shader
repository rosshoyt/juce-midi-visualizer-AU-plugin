#shader preset display name Spiral Array Shader
#shader vertex

attribute vec4 position;
attribute vec4 sourceColour;
attribute vec2 textureCoordIn;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

varying vec4 destinationColour;
varying vec2 textureCoordOut;

varying float xPos;
varying float yPos;
varying float zPos;

void main()
{
    vec4 v = vec4 (position);
    xPos = clamp (v.x, 0.0, 1.0);
    yPos = clamp (v.y, 0.0, 1.0);
    zPos = clamp (v.z, 0.0, 1.0);
    gl_Position = projectionMatrix * viewMatrix * position;
},

#shader fragment

varying vec4 destinationColour;
varying vec2 textureCoordOut;
varying float xPos;
varying float yPos;
varying float zPos;

void main()
{
    gl_FragColor = vec4 (xPos, yPos, zPos, 1.0);
}
