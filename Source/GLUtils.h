/*
 ==============================================================================

 Ross Hoyt
 Utility.h


 ==============================================================================
 */

#pragma once
#include <fstream>
#include <string>
#include <sstream>


static Image resizeImageToPowerOfTwo (Image image)
{
    if (! (isPowerOfTwo (image.getWidth()) && isPowerOfTwo (image.getHeight())))
        return image.rescaled (jmin (1024, nextPowerOfTwo (image.getWidth())),
                               jmin (1024, nextPowerOfTwo (image.getHeight())));

    return image;
}

struct ShaderPreset
{
    const char* name;
    const char* vertexShader;
    const char* fragmentShader;
};

static const char * SHADER_DISPLAY_NAME_FLAG = "#shader preset display name ";
static const char * SHADER_VERTEX_FLAG = "#shader vertex";
static const char * SHADER_FRAGMENT_FLAG = "#shader fragment";


static Array<ShaderPreset> getShaderPresets(const char* absoluteDirectory)
{
    DirectoryIterator dirIter(File(absoluteDirectory), false, "*.shader");

    enum ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1 };

    Array<ShaderPreset> shaderPresets;

    while(dirIter.next())
    {
        File file(dirIter.getFile());
        std::ifstream stream(file.getFullPathName().toRawUTF8());
        std::stringstream ss[2]; //
        ShaderPreset shaderPreset; // the shader preset to be added to array
        ShaderType type = NONE;
        std::string line;
        while(getline(stream, line))
        {
            if(line.find(SHADER_DISPLAY_NAME_FLAG) != std::string::npos)
                shaderPreset.name = String(line.c_str()).removeCharacters(SHADER_DISPLAY_NAME_FLAG).toRawUTF8();
            else if(line.find(SHADER_VERTEX_FLAG) != std::string::npos)
                type = VERTEX;
            else if(line.find(SHADER_FRAGMENT_FLAG) != std::string::npos)
                type = FRAGMENT;
            else
                ss[(int)type] << line << '\n';
        }
        shaderPreset.vertexShader = ss[(int)VERTEX].str().c_str();
        shaderPreset.fragmentShader = ss[(int)FRAGMENT].str().c_str();
        shaderPresets.add(shaderPreset);
    }
    return shaderPresets;
}
//static ShaderPreset readShader(const)

//static Array<ShaderPreset> getShaderPresets()
//{
//    ShaderPreset presets[] =
//    {
//
//    };
//
//    return Array<ShaderPreset> (presets, numElementsInArray (presets));
//}
