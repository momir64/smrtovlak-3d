#pragma once
#include "WindowManager.h"
#include "DataClasses.h"
#include <GL/glew.h>
#include "Shader.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>

class Text {
    struct Glyph {
        int bearingX, bearingY;
        int advance;
        GLuint tex;
        int w, h;
    };

    struct QuadVertex {
        float x, y, u, v;
    };

    static void loadFont(const std::string& path, int glyphResolution, std::map<uint32_t, Glyph>& glyphs);
    static std::vector<uint32_t> utf16_decode(const std::wstring& s);

    static std::map<uint32_t, Glyph> glyphs;
    std::vector<QuadVertex> vertices;
    GLuint VAO = 0, VBO = 0;
    WindowManager& window;
    std::wstring text;
    Shader shader;
    Bounds bounds;

    void prepareVertices();

public:
    Text(WindowManager& window, const std::wstring& text, Bounds bounds);
    ~Text();

    void draw();
};