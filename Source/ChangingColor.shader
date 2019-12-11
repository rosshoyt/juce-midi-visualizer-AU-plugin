#shader preset display name Changing Color
#shader vertex

attribute vec4 position;
attribute vec2 textureCoordIn;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

varying vec2 textureCoordOut;

void main()
{
    textureCoordOut = textureCoordIn;
    gl_Position = projectionMatrix * viewMatrix * position;
    //(projectionMatrix + mat4(gl_InstanceID, 1, 1, 1))
    int id = gl_InstanceID;
}

#shader fragment

#define PI 3.1415926535897932384626433832795

varying vec2 textureCoordOut;

uniform float bouncingNumber;

void main()
{
   float b = bouncingNumber;
   float n = b * PI * 2.0;
   float sn = (sin (n * textureCoordOut.x) * 0.5) + 0.5;
   float cn = (sin (n * textureCoordOut.y) * 0.5) + 0.5;

   vec4 col = vec4 (b, sn, cn, 1.0);
   gl_FragColor = col;
}
