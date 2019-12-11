#shader preset display name Simple Light
#shader vertex

attribute vec4 position;
attribute vec4 normal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec4 lightPosition;

varying float lightIntensity;

void main()
{
    vec4 light = viewMatrix * lightPosition;
    lightIntensity = dot (light, normal);

    gl_Position = projectionMatrix * viewMatrix * position;
}

#shader fragment

varying float lightIntensity;

void main()
{
   float l = lightIntensity * 0.25;
   vec4 colour = vec4 (l, l, l, 1.0);


    gl_FragColor = colour;
}
