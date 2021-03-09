#pragma once
#pragma comment(lib, "OpenGL32.lib")
#include "../stdafx/stdafx.h"
#include <gl/GL.h>
#include "../mem/mem.h"
#include "../geometry/geometry.h"

namespace rgb
{
    /// <summary>
    /// Define colors
    /// </summary>
    const GLubyte red[3] = { 255,0,0 };
    const GLubyte green[3] = { 0,255,0 };
    const GLubyte gray[3] = { 55,55,55 };
    const GLubyte lightgray[3] = { 192,192,192 };
    const GLubyte black[3] = { 0,0,0 };
}


namespace GL
{
    /// <summary>
    /// Font Class
    /// </summary>
    class Font
    {
    public:

        /// <summary>
        /// If the font is built
        /// </summary>
        bool bBuilt{ false };

        /// <summary>
        /// Display Lists
        /// </summary>
        unsigned int base;

        /// <summary>
        /// Handle to Device Context
        /// </summary>
        HDC hdc{ nullptr };

        /// <summary>
        /// Height of font
        /// </summary>
        int height;

        /// <summary>
        /// Width of font
        /// </summary>
        int width;

        /// <summary>
        /// Build Font
        /// </summary>
        /// <param name="height">height of font</param>
        void Build(int height);

        /// <summary>
        /// Print Text
        /// </summary>
        /// <param name="x">x position</param>
        /// <param name="y">y position</param>
        /// <param name="color">text color</param>
        /// <param name="format">string format</param>
        /// <param name="">string params</param>
        void Print(float x, float y, const unsigned char color[3], const char* format, ...);

        /// <summary>
        /// Gets center of text for rectangle
        /// </summary>
        /// <param name="x">x positiong</param>
        /// <param name="y">y position</param>
        /// <param name="width">width of space</param>
        /// <param name="height">height of space</param>
        /// <param name="textWidth">text width</param>
        /// <param name="textHeight">text height</param>
        /// <returns></returns>
        geometry::vector3 centerText(float x, float y, float width, float height, float textWidth, float textHeight);

        /// <summary>
        /// Gets center of text
        /// </summary>
        /// <param name="x">x position</param>
        /// <param name="width">width of space</param>
        /// <param name="textWidth">text width</param>
        /// <returns></returns>
        float centerText(float x, float width, float textWidth);
    };

    /// <summary>
    /// Sets up screen so we can draw in 2D
    /// </summary>
    void SetupOrtho();

    /// <summary>
    /// Restores the overlay
    /// </summary>
    void RestoreGL();

    /// <summary>
    /// Draw a filled rectangle
    /// </summary>
    /// <param name="x">x position</param>
    /// <param name="y">y position</param>
    /// <param name="width">width of rectangle</param>
    /// <param name="height">height of rectangle</param>
    /// <param name="color">color</param>
    void DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3]);

    /// <summary>
    /// Draw an outline
    /// </summary>
    /// <param name="x">x position</param>
    /// <param name="y">y position</param>
    /// <param name="width">width of outline</param>
    /// <param name="height">height of outline</param>
    /// <param name="lineWidth">width of outline line</param>
    /// <param name="color">color</param>
    void DrawOutline(float x, float y, float width, float height, float lineWidth, const GLubyte color[3]);

    /// <summary>
    /// World to Screen
    /// </summary>
    /// <param name="position">3D coordinate</param>
    /// <param name="screenPos">position of screen to set</param>
    /// <param name="viewMatrix">view matrix</param>
    /// <param name="windowWidth">window width</param>
    /// <param name="windowHeight">window height</param>
    /// <returns></returns>
    bool WorldToScreen(geometry::vector3 position, geometry::vector3& screenPos, float viewMatrix[16], int windowWidth, int windowHeight);
}