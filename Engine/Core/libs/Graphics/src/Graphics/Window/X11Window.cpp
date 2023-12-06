//
// Created by innerviewer on 06/11/23.
//

#include <Graphics/Window/X11Window.h>

/*#include <xcb/xcb_ewmh.h>
#include <xcb/xcb.h>
#include <xcb/xcb_util.h>*/

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

namespace SR_GRAPH_NS {
    bool X11Window::Initialize(const std::string &name,
           const Utils::Math::IVector2 &position,
           const Utils::Math::UVector2 &size,
           bool fullScreen, bool resizable
   ) {
        SR_LOG("X11Window::Initialize() : creating X11 window...");

        Display* pDisplay = nullptr;

        pDisplay = XOpenDisplay(nullptr);
        if (!pDisplay) {
            SR_ERROR("X11Window::Initialize() : failed to create X11 display!");
            return false;
        }

        auto&& screen = DefaultScreen(pDisplay);
        auto&& window = XCreateWindow(pDisplay, RootWindow(pDisplay, screen), position.x, position.y, size.x, size.y,
                               0, 0, InputOutput, CopyFromParent, 0, nullptr);

        m_display = pDisplay;
        m_window = window;
        m_surfaceSize = size;
        m_size = size;

        Move(position.x,position.y);
        Resize(size.x, size.y);
        SetResizable(resizable);
        SetFullscreen(fullScreen);

        auto&& windowTypeProperty = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE", false);
        auto&& windowType = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_DESKTOP", false);

        //XChangeProperty(m_display, m_window, windowTypeProperty, XA_WM_CLASS, 8, PropModeReplace, (unsigned char*)windowType, 1);
        XChangeProperty(m_display, m_window, windowTypeProperty, XA_ATOM, 32, PropModeReplace, (unsigned char*)&windowType, 1);

        XSelectInput(pDisplay, window, ExposureMask | KeyPressMask | PointerMotionMask);
        XMapWindow(pDisplay, window);

        m_isValid = true;
        return true;
    }

    void* X11Window::GetHandle() const {
        return nullptr;
    }

    void X11Window::SetResizable(bool resizable) const {
        /*XSizeHints* pSizeHints = XAllocSizeHints();
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
        XFree(pSizeHints);*/
    }

    void X11Window::SetFullscreen(bool fullscreen) const {
        //TODO: (Linux) Finish implementing SetFullscreen() for X11.
    }

    void* X11Window::GetDisplay() const {
        return m_display;
    }

    SR_MATH_NS::IVector2 X11Window::GetScreenResolution() const {
        //TODO: (Linux) Finish implementing GetScreenResolution() for X11.

        return BasicWindowImpl::GetScreenResolution();
        //return SR_MATH_NS::IVector2(width, height);
    }

    void X11Window::PollEvents() {
        PollEventsHandler();
    }

    void X11Window::Close() {
        /*if (m_connection) {
            xcb_destroy_window(m_connection, m_window);
            xcb_disconnect(m_connection);

            m_connection = nullptr;
        }*/

        if (m_display) {
            XDestroyWindow(m_display, m_window);
            XCloseDisplay(m_display);

            m_display = nullptr;
        }

        BasicWindowImpl::Close();
    }

    void X11Window::PollEventsHandler() {
        XEvent event;
        XPeekEvent(m_display, &event);

        switch (event.type) {
            case MotionNotify: {
                SR_LOG("Motion ---------------");
                break;
            }
            case Expose: {

                SR_LOG("Exposure ---------------");
            }
            case KeyPress: {
                SR_LOG("KeyPress ---------------");
            }
            default:
                break;
        }
    }

    uint64_t X11Window::GetWindow() const {
        return m_window;
    }

    void X11Window::Maximize() {
        XClientMessageEvent ev = {};
        auto&& wmState = XInternAtom(m_display, "_NET_WM_STATE", False);
        auto&& maxH = XInternAtom(m_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
        auto&& maxV = XInternAtom(m_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

        if (!wmState) {
            return;
        }

        ev.type = ClientMessage;
        ev.format = 32;
        ev.window = m_window;
        ev.message_type = wmState;
        ev.data.l[0] = 1;
        ev.data.l[1] = maxH;
        ev.data.l[2] = maxV;
        ev.data.l[3] = 1;

        XFlush(m_display);

        BasicWindowImpl::Maximize();
    }
}