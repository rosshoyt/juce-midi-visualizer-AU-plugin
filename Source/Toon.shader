#shader preset display name Toon Shader
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
    float intensity = lightIntensity * 0.5;
    vec4 colour;

    if (intensity > 0.95)
        colour = vec4 (1.0, 0.5, 0.5, 1.0);
    else if (intensity > 0.5)
        colour  = vec4 (0.6, 0.3, 0.3, 1.0);
    else if (intensity > 0.25)
        colour  = vec4 (0.4, 0.2, 0.2, 1.0);
    else
        colour  = vec4 (0.2, 0.1, 0.1, 1.0);

    gl_FragColor = colour;
}
