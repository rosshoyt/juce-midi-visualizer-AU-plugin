/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   ChangingColor_shader;
    const int            ChangingColor_shaderSize = 776;

    extern const char*   FlatColor_shader;
    const int            FlatColor_shaderSize = 551;

    extern const char*   Flattened_shader;
    const int            Flattened_shaderSize = 595;

    extern const char*   Rainbow_shader;
    const int            Rainbow_shaderSize = 735;

    extern const char*   SimpleLight_shader;
    const int            SimpleLight_shaderSize = 555;

    extern const char*   SpiralArray_shader;
    const int            SpiralArray_shaderSize = 747;

    extern const char*   Textured_shader;
    const int            Textured_shaderSize = 605;

    extern const char*   TextureLighting_shader;
    const int            TextureLighting_shaderSize = 908;

    extern const char*   Toon_shader;
    const int            Toon_shaderSize = 815;

    extern const char*   portmeirion_jpg;
    const int            portmeirion_jpgSize = 145904;

    extern const char*   pianokey_rectangle_obj;
    const int            pianokey_rectangle_objSize = 442;

    extern const char*   teapot_obj;
    const int            teapot_objSize = 95004;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 12;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
