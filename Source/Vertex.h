/*
 ==============================================================================
 
 Vertex.h
 Created: 8 Dec 2019 11:48:01pm
 Author:  Ross Hoyt
 
 ==============================================================================
 */

#pragma once

/** Vertex data to be passed to the shaders.
 Each has e a 3D position, a colour and a 2D texture co-ordinate
 */
struct Vertex
{
    float position[3];
    float normal[3];
    float colour[4];
    float texCoord[2];
};
