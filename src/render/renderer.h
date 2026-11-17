#pragma once

#include "render.h"


struct Renderer {
    struct Context;
    static void Initialize(Asset_Manager& asset_manager, Render_Group& render_group);
    static void Render(Render_Group& group);
};
