#include "muhev.hpp"

// #include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifdef __linux__
    #include <sys/epoll.h>
    #include <sys/ioctl.h>
    #include <utility>
#else
    #include <sys/types.h>
    #include <sys/event.h>
#endif

namespace NAC {
    namespace NMuhEv {
        namespace {
#ifndef __linux__
            static inline void AddEventKqueueImpl(int queueId, int filter, const TEvSpec& spec) {
                TInternalEvStruct event;

                EV_SET(
                    &event,
                    spec.Ident,
                    filter,
                    EV_ADD | EV_ENABLE,
                    0,
                    0,
                    0
                );

                event.udata = spec.Ctx;

                while (kevent(
                    queueId,
                    &event,
                    1,
                    nullptr,
                    0,
                    nullptr
                ) != 0) {
                    if (errno != EINTR) {
                        perror("kevent");
                        abort();
                    }
                }
            }

            static inline void RemoveEventKqueueImpl(int queueId, int filter, int ident) {
                TInternalEvStruct event;

                EV_SET(
                    &event,
                    ident,
                    filter,
                    EV_DELETE,
                    0,
                    0,
                    0
                );

                while (kevent(
                    queueId,
                    &event,
                    1,
                    nullptr,
                    0,
                    nullptr
                ) != 0) {
                    if (errno == ENOENT) {
                        return;
                    }

                    if (errno != EINTR) {
                        perror("kevent");
                        abort();
                    }
                }
            }
#endif
        }

        TLoop::TLoop() {
#ifdef __linux__
            QueueId = epoll_create(0x1);
#else
            QueueId = kqueue();
#endif

            if (QueueId == -1) {
                perror("kqueue");
                abort();
            }
        }

        TLoop::~TLoop() {
            if (close(QueueId) == -1) {
                perror("close");
            }
        }

        void TLoop::AddEvent(const TEvSpec& spec, bool mod) {
#ifdef __linux__
            TInternalEvStruct event = { 0 };
            event.events = 0;
            event.data.ptr = spec.Ctx;
            event.data.fd = spec.Ident;

            if (spec.Filter & MUHEV_FILTER_READ) {
                event.events |= EPOLLIN;
            }

            if (spec.Filter & MUHEV_FILTER_WRITE) {
                event.events |= EPOLLOUT;
            }

            if (
                (epoll_ctl(QueueId, (mod ? EPOLL_CTL_MOD : EPOLL_CTL_ADD), spec.Ident, &event) != 0)
                && (
                    (
                        (errno == (mod ? ENOENT : EEXIST))
                        && (epoll_ctl(QueueId, (mod ? EPOLL_CTL_ADD : EPOLL_CTL_MOD), spec.Ident, &event) != 0)
                    )
                    || (errno != (mod ? ENOENT : EEXIST))
                )
            ) {
                perror("epoll_ctl");
                abort();
            }

#else
            if (spec.Filter & MUHEV_FILTER_READ) {
                AddEventKqueueImpl(QueueId, EVFILT_READ, spec);
            }

            if (spec.Filter & MUHEV_FILTER_WRITE) {
                AddEventKqueueImpl(QueueId, EVFILT_WRITE, spec);
            }
#endif
        }

        bool TLoop::Wait(std::vector<TEvSpec>& out) {
            std::vector<TInternalEvStruct> list;

            if (out.capacity() > 0) {
                list.reserve(out.capacity());

            } else {
                list.reserve(100);
            }

            auto rawData = list.data();

            while (true) {
#ifdef __linux__
                // NUtils::cluck(1, "wait()");
                int triggeredCount = epoll_wait(
                    QueueId,
                    rawData,
                    list.capacity(),
                    24 * 60 * 60
                );

#else
                int triggeredCount = kevent(
                    QueueId,
                    nullptr,
                    0,
                    rawData,
                    list.capacity(),
                    nullptr
                );
#endif

                if (triggeredCount < 0) {
                    if (errno != EINTR) {
                        perror("kevent");
                        abort();
                    }

                    return false;

                } else {
                    for (size_t i = 0; i < triggeredCount; ++i) {
                        const auto& event = rawData[i];

                        TEvSpec node {
#ifdef __linux__
                            .Ident = (uintptr_t)event.data.fd,
                            .Ctx = event.data.ptr,

#else
                            .Ident = event.ident,
                            .Ctx = event.udata,
#endif

                            .Filter = MUHEV_FILTER_NONE,
                            .Flags = MUHEV_FLAG_NONE,
                        };

#ifdef __linux__
                        if (event.events & EPOLLIN) {
                            node.Filter |= MUHEV_FILTER_READ;
                        }

                        if (event.events & EPOLLOUT) {
                            node.Filter |= MUHEV_FILTER_WRITE;
                        }

#else
                        switch (event.filter) {
                            case EVFILT_READ:
                                node.Filter = MUHEV_FILTER_READ;
                                break;

                            case EVFILT_WRITE:
                                node.Filter = MUHEV_FILTER_WRITE;
                                break;

                            default:
                                node.Flags |= MUHEV_FLAG_ERROR;
                        }

                        if (event.flags & EV_ERROR) {
                            node.Flags |= MUHEV_FLAG_ERROR;
                        }

                        if (event.flags & EV_EOF) {
                            node.Flags |= MUHEV_FLAG_EOF;
                        }
#endif

                        out.emplace_back(std::move(node));
                    }

                    return true;
                }
            }
        }

        void TLoop::RemoveEvent(uintptr_t ident) {
#ifdef __linux__
            TInternalEvStruct event = { 0 };

            if (
                (epoll_ctl(QueueId, EPOLL_CTL_DEL, ident, &event) != 0)
                && (errno != ENOENT)
            ) {
                perror("epoll_ctl");
                abort();
            }

#else
            RemoveEventKqueueImpl(QueueId, EVFILT_READ, ident);
            RemoveEventKqueueImpl(QueueId, EVFILT_WRITE, ident);
#endif
        }
    }
}
