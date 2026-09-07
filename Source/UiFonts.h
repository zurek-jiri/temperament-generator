// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#pragma once
#include <juce_graphics/juce_graphics.h>

namespace ui
{
inline juce::String sansFont()
{
   #if JUCE_WINDOWS
    return "Segoe UI";
   #else
    return juce::Font::getDefaultSansSerifFontName();
   #endif
}
inline juce::String monoFont()
{
   #if JUCE_WINDOWS
    return "Consolas";
   #else
    return juce::Font::getDefaultMonospacedFontName();
   #endif
}
}
