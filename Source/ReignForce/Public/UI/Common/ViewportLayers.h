// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


/**
 * Viewport z-orders of the game widgets: the HUD at the bottom, then its hints, the menus & their submenus, the loading screen on top
 */
namespace ViewportLayers
{
    /** Aim, stats, rounds & the other gameplay widgets */
    constexpr int32 HUD = 0;

    /** Controls reminder: over the HUD widgets (shares the top left corner with the skills counter), under the menus */
    constexpr int32 CONTROLS_HINT = 5;

    /** Pause & skills menus with their confirmation modals (added after the menu, so drawn over it) */
    constexpr int32 MENU = 10;

    /** Menus opened from a menu (settings, key bindings, gamepad scheme, skill upgrade confirmation) */
    constexpr int32 SUBMENU = 20;

    constexpr int32 LOADING_SCREEN = 1000;
}
