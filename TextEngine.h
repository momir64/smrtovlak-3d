#pragma once
#include "WindowManager.h"
#include "DataClasses.h"
#include <GL/glew.h>
#include "Shader.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>

class TextEngine {
    struct Glyph {
        int bearingX, bearingY;
        int advance;
        GLuint tex;
        int w, h;
    };

    std::vector<uint32_t> utf16_decode(const std::wstring& s);
    void loadFont(const std::string& path, int glyphResolution);

    std::map<uint32_t, Glyph> glyphs;
    GLuint vao = 0, vbo = 0;
    WindowManager& window;
    Shader shader;

public:
    TextEngine(WindowManager& window, const std::string& path, int glyphResolution);
    void draw(const std::wstring& text, Bounds bounds);
};
