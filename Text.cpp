#include "Text.h"
#include <freetype/config/ftheader.h>
#include "DataClasses.h"
#include FT_FREETYPE_H
#include "GL/glew.h"
#include <cstdint>
#include <wchar.h>
#include <vector>
#include <string>

std::map<uint32_t, Text::Glyph> Text::glyphs;

Text::Text(WindowManager& window, const std::wstring& text, Bounds bounds) :
	window(window), shader("shaders/text.vert", "shaders/text.frag"), text(text), bounds(bounds) {
	
	loadFont("assets/fonts/jersey.ttf", 256, glyphs);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(float), 0);
	glBindVertexArray(0);

	prepareVertices();
}

Text::~Text() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void Text::prepareVertices() {
	vertices.clear();

	float invW = 1.0f / float(window.getWidth());
	float invH = 1.0f / float(window.getHeight());
	float scaleX = bounds.width / 100.f / float(window.getWidth());
	float scaleY = bounds.width / 100.f / float(window.getHeight());

	auto cps = utf16_decode(text);

	float cursorX = bounds.x * invW;
	float cursorY = 1 - (bounds.y * invH);

	for (uint32_t cp : cps) {
		auto it = glyphs.find(cp);
		if (it == glyphs.end())
			continue;

		const auto& g = it->second;

		float w = g.w * scaleX;
		float h = g.h * scaleY;

		float xpos = cursorX + g.bearingX * scaleX;
		float ypos = cursorY - (g.h - g.bearingY) * scaleY;

		vertices.push_back({ xpos,     ypos,     0, 1 });
		vertices.push_back({ xpos + w, ypos,     1, 1 });
		vertices.push_back({ xpos + w, ypos + h, 1, 0 });

		vertices.push_back({ xpos,     ypos,     0, 1 });
		vertices.push_back({ xpos + w, ypos + h, 1, 0 });
		vertices.push_back({ xpos,     ypos + h, 0, 0 });

		cursorX += (g.advance >> 6) * scaleX;
	}
}

void Text::draw() {
	shader.use();
	shader.setInt("uTex", 0);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	auto cps = utf16_decode(text);
	size_t vertexIndex = 0;

	for (uint32_t cp : cps) {
		auto it = glyphs.find(cp);
		if (it == glyphs.end())
			continue;

		const auto& g = it->second;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g.tex);

		glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), &vertices[vertexIndex], GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		vertexIndex += 6;
	}
}

void Text::loadFont(const std::string& path, int glyphResolution, std::map<uint32_t, Glyph>& glyphs) {
	glyphs.clear();

	FT_Library ft;
	FT_Init_FreeType(&ft);

	FT_Face face;
	FT_New_Face(ft, path.c_str(), 0, &face);
	FT_Set_Pixel_Sizes(face, 0, glyphResolution);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	uint32_t ranges[] = {
		0x0020, 0x00FF,
		0x0100, 0x017F,
		0x0400, 0x04FF
	};

	for (int r = 0; r < 3; r++) {
		uint32_t start = ranges[r * 2];
		uint32_t end = ranges[r * 2 + 1];

		for (uint32_t cp = start; cp <= end; cp++) {
			if (FT_Load_Char(face, cp, FT_LOAD_RENDER))
				continue;

			GLuint tex;
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);

			auto& bm = face->glyph->bitmap;

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bm.width, bm.rows, 0,
				GL_RED, GL_UNSIGNED_BYTE, bm.buffer);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			Glyph g{};
			g.tex = tex;
			g.w = bm.width;
			g.h = bm.rows;
			g.bearingX = face->glyph->bitmap_left;
			g.bearingY = face->glyph->bitmap_top;
			g.advance = face->glyph->advance.x;

			glyphs[cp] = g;
		}
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

std::vector<uint32_t> Text::utf16_decode(const std::wstring& s) {
	std::vector<uint32_t> out;
	size_t i = 0;

	while (i < s.size()) {
		uint32_t w = s[i];

#if WCHAR_MAX == 0xFFFF
		if (w >= 0xD800 && w <= 0xDBFF && i + 1 < s.size()) {
			uint32_t w2 = s[i + 1];
			if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
				uint32_t cp = 0x10000 +
					(((w - 0xD800) << 10) |
						(w2 - 0xDC00));
				out.push_back(cp);
				i += 2;
				continue;
			}
		}
#endif
		out.push_back(w);
		i++;
	}
	return out;
}