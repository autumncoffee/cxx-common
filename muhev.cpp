#include "muhev.hpp"

// #include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __linux__
    #include <sys/epoll.h>
    #include <sys/ioctl.h>
    #include <utility>

    struct epoll_event;

#else
    #include <sys/event.h>

    struct kevent;
#endif

namespace NAC {
    namespace NMuhEv {
#ifdef __linux__
        using TInternalEvStruct = struct ::epoll_event;
#else
        using TInternalEvStruct = struct ::kevent;
#endif

        namespace {
#ifndef __linux__
            static inline void AddEventKqueueImpl(int queueId, int filter, TNode& node) {
                TInternalEvStruct event;

                EV_SET(
                    &event,
                    node.GetEvIdent(),
                    filter,
                    EV_ADD | EV_ENABLE,
                    0,
                    0,
                    0
                );

                event.udata = (void*)&node;

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

        void TLoop::MakeFds(int* out) {
            if (socketpair(PF_LOCAL, SOCK_STREAM, 0, out) == -1) {
                perror("socketpair");
                abort();
            }
        }

        TTriggerNodeBase::~TTriggerNodeBase() {
            close(EvIdent);
            close(TriggerFd_);
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

            WakeupNode = NewTrigger([](){});
        }

        TLoop::~TLoop() {
            RemoveEvent(*WakeupNode);

            if (close(QueueId) == -1) {
                perror("close");
            }
        }

        void TLoop::Wake() {
            WakeupNode->Trigger();
        }

        void TriggerFd(int fd) {
            while (true) {
                int rv = write(fd, "1", 1);

                if (rv > 0) {
                    break;

                } else if (rv == 0) {
                    continue;

                } else {
                    if (errno == EINTR) {
                        continue;
                    }

                    perror("write");
                    abort();
                }
            }
        }

        void TLoop::AddEvent(TNode& node, bool mod) {
#ifdef __linux__
            TInternalEvStruct event = { 0 };
            event.events = 0;
            event.data.ptr = (void*)&node;

            if (node.GetEvFilter() & MUHEV_FILTER_READ) {
                event.events |= EPOLLIN;
            }

            if (node.GetEvFilter() & MUHEV_FILTER_WRITE) {
                event.events |= EPOLLOUT;
            }

            if (
                (epoll_ctl(QueueId, (mod ? EPOLL_CTL_MOD : EPOLL_CTL_ADD), node.GetEvIdent(), &event) != 0)
                && (
                    (
                        (errno == (mod ? ENOENT : EEXIST))
                        && (epoll_ctl(QueueId, (mod ? EPOLL_CTL_ADD : EPOLL_CTL_MOD), node.GetEvIdent(), &event) != 0)
                    )
                    || (errno != (mod ? ENOENT : EEXIST))
                )
            ) {
                perror("epoll_ctl");
                abort();
            }

#else
            if (node.GetEvFilter() & MUHEV_FILTER_READ) {
                AddEventKqueueImpl(QueueId, EVFILT_READ, node);

            } else {
                RemoveEventKqueueImpl(QueueId, EVFILT_READ, node.GetEvIdent());
            }

            if (node.GetEvFilter() & MUHEV_FILTER_WRITE) {
                AddEventKqueueImpl(QueueId, EVFILT_WRITE, node);

            } else {
                RemoveEventKqueueImpl(QueueId, EVFILT_WRITE, node.GetEvIdent());
            }
#endif
        }

        bool TLoop::Wait(const size_t capacity) {
            TInternalEvStruct list[capacity];

            while (true) {
#ifdef __linux__
                // NUtils::cluck(1, "wait()");
                int triggeredCount = epoll_wait(
                    QueueId,
                    list,
                    capacity,
                    24 * 60 * 60
                );

#else
                int triggeredCount = kevent(
                    QueueId,
                    nullptr,
                    0,
                    list,
                    capacity,
                    nullptr
                );
#endif

                if (triggeredCount < 0) {
                    if (errno == EINTR) {
                        continue;
                    }

                    return false;

                } else {
                    for (size_t i = 0; i < triggeredCount; ++i) {
                        const auto& event = list[i];
                        int filter = MUHEV_FILTER_NONE;
                        int flags = MUHEV_FLAG_NONE;

#ifdef __linux__
                        auto node = (TNode*)event.data.ptr;
#else
                        auto node = (TNode*)event.udata;
#endif

#ifdef __linux__
                        if (event.events & EPOLLIN) {
                            filter |= MUHEV_FILTER_READ;
                        }

                        if (event.events & EPOLLOUT) {
                            filter |= MUHEV_FILTER_WRITE;
                        }

#else
                        switch (event.filter) {
                            case EVFILT_READ:
                                filter = MUHEV_FILTER_READ;
                                break;

                            case EVFILT_WRITE:
                                filter = MUHEV_FILTER_WRITE;
                                break;

                            default:
                                flags |= MUHEV_FLAG_ERROR;
                        }

                        if (event.flags & EV_ERROR) {
                            flags |= MUHEV_FLAG_ERROR;
                        }

                        if (event.flags & EV_EOF) {
                            flags |= MUHEV_FLAG_EOF;
                        }
#endif

                        if (node->IsAlive()) {
                            try {
                                node->Cb(filter, flags);

                            } catch (...) {
                            }
                        }

                        if (!node->IsAlive()) {
                            RemoveEvent(*node);
                        }
                    }

                    return true;
                }
            }
        }

        void TLoop::RemoveEvent(TNode& node) {

#ifdef __linux__
            TInternalEvStruct event = { 0 };

            if (
                (epoll_ctl(QueueId, EPOLL_CTL_DEL, node.GetEvIdent(), &event) != 0)
                && (errno != ENOENT)
            ) {
                perror("epoll_ctl");
                abort();
            }

#else
            RemoveEventKqueueImpl(QueueId, EVFILT_READ, node.GetEvIdent());
            RemoveEventKqueueImpl(QueueId, EVFILT_WRITE, node.GetEvIdent());
#endif
        }

        void TNode::Drain() {
            char dummy[128];
            int rv = recvfrom(
                EvIdent,
                dummy,
                128,
                MSG_DONTWAIT,
                NULL,
                0
            );

            if (rv < 0) {
                perror("recvfrom");
                abort();
            }
        }
    }
}
