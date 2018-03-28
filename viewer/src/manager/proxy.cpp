/*
 * DIPlib 3.0 viewer
 * This file contains functionality for a rudamentary proxy window manager.
 *
 * (c)2018, Wouter Caarls
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diplib/viewer/proxy.h"

/// \file
/// \brief Defines the proxy interface of \ref viewer.

using namespace dip::viewer;

ProxyManager ProxyManager::instance_;

int proxyGetWidth(Window *window)
{
  return ProxyManager::instance()->proxyGetWidth(window);
}

int proxyGetHeight(Window *window)
{
  return ProxyManager::instance()->proxyGetHeight(window);
}

bool proxyGetDestroyed(Window *window)
{
  return ProxyManager::instance()->proxyGetDestroyed(window);
}

void proxyRelease(Window *window)
{
  ProxyManager::instance()->release(window);
}

void proxyDrawEvent(Window *window)
{
  ProxyManager::instance()->proxyDrawEvent(window);
}

void proxyIdleEvent(Window *window)
{
  ProxyManager::instance()->proxyIdleEvent(window);
}

void proxyReshapeEvent(Window *window, int width, int height)
{
  ProxyManager::instance()->proxyReshapeEvent(window, width, height);
}

void proxyVisibleEvent(Window *window, int vis)
{
  ProxyManager::instance()->proxyVisibleEvent(window, vis);
}

void proxyCreateEvent(Window *window)
{
  ProxyManager::instance()->proxyCreateEvent(window);
}

void proxyCloseEvent(Window *window)
{
  ProxyManager::instance()->proxyCloseEvent(window);
}

void proxyKeyEvent(Window *window, unsigned char k, int x, int y, int mods)
{
  ProxyManager::instance()->proxyKeyEvent(window, k, x, y, mods);
}

void proxyClickEvent(Window *window, int button, int state, int x, int y, int mods)
{
  ProxyManager::instance()->proxyClickEvent(window, button, state, x, y, mods);
}

void proxyMotionEvent(Window *window, int x, int y)
{
  ProxyManager::instance()->proxyMotionEvent(window, x, y);
}

void proxySetSwapBuffersCallback(Window *window, ProxySwapBuffersCallback cb)
{
  ProxyManager::instance()->setSwapBuffersCallback(window, cb);
}

void proxySetWindowTitleCallback(Window *window, ProxySetWindowTitleCallback cb)
{
  ProxyManager::instance()->setWindowTitleCallback(window, cb);
}

void proxySetRefreshWindowCallback(Window *window, ProxyRefreshWindowCallback cb)
{
  ProxyManager::instance()->setRefreshWindowCallback(window, cb);
}

void proxySetCreateWindowCallback(Window *window, ProxyCreateWindowCallback cb)
{
  ProxyManager::instance()->setCreateWindowCallback(window, cb);
}
