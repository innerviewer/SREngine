//
// Created by Monika on 17.11.2021.
//

#ifndef GAMEENGINE_THREAD_H
#define GAMEENGINE_THREAD_H

#include <Debug.h>

namespace SR_HTYPES_NS {
    class Thread {
    public:
        Thread()
            : m_thread(std::thread())
        { }

        explicit Thread(std::thread thread)
            : m_thread(std::move(thread))
        { }

        explicit Thread(const std::function<void()>& fn)
            : m_thread(fn)
        { }

        template<class Functor, typename... Args> explicit Thread(Functor&& fn, Args&&... args)
            : m_thread(fn, std::forward<Args>(args)...)
        { }

        Thread(Thread&& thread)  noexcept {
            m_thread.swap(thread.m_thread);
        }

        Thread& operator=(Thread&& thread) noexcept {
            m_thread.swap(thread.m_thread);
            return *this;
        }

        ~Thread() {
            SRAssert(!Joinable());
        }

    public:
        SR_NODISCARD bool Joinable() const { return m_thread.joinable(); }
        void Join() { m_thread.join(); }
        bool TryJoin() {
            if (Joinable()) {
                Join();
                return true;
            }

            return false;
        }
        void Detach() { m_thread.detach(); }

        static void Sleep(uint64_t milliseconds);

    private:
        std::thread m_thread;

    };
}

#endif //GAMEENGINE_THREAD_H
