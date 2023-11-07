//
// Created by innerviewer on 06/11/23.
//

#include <Graphics/Window/X11Window.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb_ewmh.h>

namespace SR_GRAPH_NS {
    bool X11Window::Initialize(const std::string &name,
           const Utils::Math::IVector2 &position,
           const Utils::Math::UVector2 &size,
           bool fullScreen, bool resizable
   ) {
        SR_LOG("X11Window::Initialize() : creating X11 window...");

        Display* pDisplay = XOpenDisplay(nullptr);
        if (!pDisplay)
        {
            SR_ERROR("X11Window::Initialize() : failed to create X11 display!");
            return false;
        }

        xcb_connection_t* pConnection = XGetXCBConnection(pDisplay);
        if (!pConnection)
        {
            SR_ERROR("X11Window::Initialize() : failed to create X11 connection!");
            return false;
        }

        const xcb_setup_t* pSetup = xcb_get_setup(pConnection);
        xcb_screen_t* pScreen = (xcb_setup_roots_iterator(pSetup)).data;
        xcb_window_t window = xcb_generate_id (pConnection);

        xcb_create_window (pConnection, 0, window, pScreen->root, position.x, position.y, size.x, size.y,
                           0, InputOutput, pScreen->root_visual, 0, nullptr);

        m_connection = pConnection;
        m_display = pDisplay;
        m_window = window;

        SetResizable(resizable);
        SetFullscreen(fullScreen);

        return true;
    }

    void* X11Window::GetHandle() const {
        return nullptr;
    }

    xcb_connection_t* X11Window::GetConnection() const {
        return m_connection;
    }

    uint32_t X11Window::GetWindow() const {
        return m_window;
    }

    void X11Window::SetResizable(bool resizable) const {
        XSizeHints* pSizeHints = XAllocSizeHints();
        pSizeHints->flags = resizable ? 0L : PMinSize | PMaxSize;
        if(!resizable) {
            XWindowAttributes windowAttributes;
            XGetWindowAttributes((Display*)GetDisplay(), GetWindow(), &windowAttributes);
            pSizeHints->min_width = windowAttributes.width;
            pSizeHints->max_width = windowAttributes.width;
            pSizeHints->min_height = windowAttributes.height;
            pSizeHints->max_height = windowAttributes.height;
        }

        XSetWMNormalHints((Display*)GetDisplay(), GetWindow(), pSizeHints);
        XFree(pSizeHints);
    }

    void X11Window::SetFullscreen(bool fullscreen) const {
        //TODO: (Linux) Finish implementing fullscreen mode for X11.
    }

    void* X11Window::GetDisplay() const {
        return m_display;
    }
}