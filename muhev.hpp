#pragma once

#include <memory>
#include <vector>

#ifdef __linux__
    #include <unordered_map>
    struct epoll_event;

#else
    struct kevent;
#endif

namespace NAC {
    namespace NMuhEv {
        enum EEvFilter {
            MUHEV_FILTER_NONE = 0,
            MUHEV_FILTER_READ = 2,
            MUHEV_FILTER_WRITE = 4
        };

        enum EEvFlags {
            MUHEV_FLAG_NONE = 0,
            MUHEV_FLAG_ERROR = 2,
            MUHEV_FLAG_EOF = 4
        };

        struct TEvSpec {
            uintptr_t Ident;
            int Filter;
            int Flags;
            void* Ctx;
        };

#ifdef __linux__
        using TInternalEvStruct = struct ::epoll_event;
#else
        using TInternalEvStruct = struct ::kevent;
#endif

        class TLoop {
        private:
            int QueueId;

#ifdef __linux__
            std::unordered_map<int, void*> FdMap;
#endif

        public:
            TLoop();
            ~TLoop();

        public:
            void AddEvent(const TEvSpec& spec, bool mod = true);
            void RemoveEvent(uintptr_t ident);

            bool Wait(std::vector<TEvSpec>& list);
        };
    }
}
