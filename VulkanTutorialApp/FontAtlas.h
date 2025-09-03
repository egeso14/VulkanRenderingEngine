#pragma once

// FontAtlas.h
#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct Glyph {
    float u0, v0, u1, v1;   // UVs in [0..1]
    float xOff, yOff;       // pixel offset from pen to top-left of quad
    float xAdvance;         // how much to advance the pen
    int   w, h;             // glyph bitmap size in pixels
};

struct FontAtlas {
    int texWidth = 0, texHeight = 0;
    std::vector<unsigned char> pixels;   // 8-bit alpha
    std::unordered_map<uint32_t, Glyph> glyphs;
    float ascent = 0.f, descent = 0.f, lineGap = 0.f; // in pixels
    float pixelHeight = 0.f; // chosen bake size
    // API-specific texture handle lives elsewhere (e.g., GLuint texture)
};

// Builds an atlas for a font at given pixelHeight, for codepoints [first, first+count)
inline bool BuildFontAtlas(const char* ttfPath,
    float pixelHeight,
    uint32_t firstCodepoint,
    int codepointCount,
    int atlasW, int atlasH,
    FontAtlas& outAtlas)
{
    // 1) Read font file
    FILE* f = fopen(ttfPath, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<unsigned char> fontData(size);
    fread(fontData.data(), 1, size, f);
    fclose(f);

    // 2) Init stb
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontData.data(), stbtt_GetFontOffsetForIndex(fontData.data(), 0)))
        return false;

    // 3) Create an empty atlas
    outAtlas.texWidth = atlasW;
    outAtlas.texHeight = atlasH;
    outAtlas.pixels.assign(atlasW * atlasH, 0);

    // 4) Use packer API to place glyphs
    stbtt_pack_context pc;
    if (!stbtt_PackBegin(&pc, outAtlas.pixels.data(), atlasW, atlasH, atlasW, /*padding*/ 2, nullptr))
        return false;

    // Better sharpness at small sizes:
    stbtt_PackSetOversampling(&pc, 2, 2);

    // Define a range to pack
    std::vector<stbtt_packedchar> packed(codepointCount);
    stbtt_PackFontRange(&pc,
        fontData.data(),
        /*font_index*/ 0,
        /*pixel_height*/ pixelHeight,
        /*first_codepoint*/ firstCodepoint,
        /*num_chars*/ codepointCount,
        packed.data());
    stbtt_PackEnd(&pc);

    // 5) Font vertical metrics (convert to pixels at this size)
    float scale = stbtt_ScaleForPixelHeight(&font, pixelHeight);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    outAtlas.ascent = ascent * scale;
    outAtlas.descent = descent * scale;
    outAtlas.lineGap = lineGap * scale;
    outAtlas.pixelHeight = pixelHeight;

    // 6) Copy glyph metrics/UVs
    for (int i = 0; i < codepointCount; ++i) {
        const stbtt_packedchar& p = packed[i];
        // If a glyph failed to pack, its w/h will be 0
        if (p.x1 <= p.x0 || p.y1 <= p.y0) continue;

        Glyph g;
        g.u0 = float(p.x0) / atlasW;
        g.v0 = float(p.y0) / atlasH;
        g.u1 = float(p.x1) / atlasW;
        g.v1 = float(p.y1) / atlasH;
        g.xOff = p.xoff;     // pixel offset from pen to quad
        g.yOff = p.yoff;     // (y is down in stb)
        g.xAdvance = p.xadvance; // already includes scale
        g.w = p.x1 - p.x0;
        g.h = p.y1 - p.y0;

        uint32_t cp = firstCodepoint + i;
        outAtlas.glyphs[cp] = g;
    }

    // 7) Optional: Precompute kerning function access (keep 'font' or copy info)
    // You can re-init a stbtt_fontinfo later for kerning queries.

    return true;
}