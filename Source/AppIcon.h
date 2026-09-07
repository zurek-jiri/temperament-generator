// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AppAssets.h"

inline const juce::Image& appIcon()
{
    static const juce::Image icon = [] {
        juce::Image image(juce::Image::ARGB, 512, 512, true);
        if (auto drawable = juce::Drawable::createFromImageData(AppAssets::TuningFork_svg, AppAssets::TuningFork_svgSize))
        {
            juce::Graphics graphics(image);
            drawable->drawWithin(graphics, image.getBounds().toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }
        return image;
    }();
    return icon;
}
