#pragma once

#include <functional>

// Starts the GLib portal hotkey listener on a background thread.
// on_toggle is called (from that thread) when "toggle-proxy" is activated.
void wayland_hotkey_start(std::function<void()> on_toggle);

// Signals the background GLib loop to quit and joins the thread.
void wayland_hotkey_stop();
