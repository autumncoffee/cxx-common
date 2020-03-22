#pragma once

#include <memory>
#include <utility>

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

        void TriggerFd(int fd);

        class TNode {
        public:
            TNode(
                int ident,
                int filter = MUHEV_FILTER_NONE,
                int flags = MUHEV_FLAG_NONE
            )
                : EvIdent(ident)
                , EvFilter(filter)
                , EvFlags(flags)
            {
            }

            virtual ~TNode() {
            }

            virtual void Cb(int filter, int flags) = 0;

            int GetEvIdent() const {
                return EvIdent;
            }

            int GetEvFilter() const {
                return EvFilter;
            }

            int GetEvFlags() const {
                return EvFlags;
            }

            virtual bool IsAlive() const {
                return true;
            }

        protected:
            void Drain();

        protected:
            int EvIdent = 0;
            int EvFilter = MUHEV_FILTER_NONE;
            int EvFlags = MUHEV_FLAG_NONE;
        };

        class TTriggerNodeBase : public TNode {
        public:
            TTriggerNodeBase(int* fds)
                : TNode(fds[1], MUHEV_FILTER_READ)
                , TriggerFd_(fds[0])
            {
            }

            ~TTriggerNodeBase();

            void Trigger() const {
                TriggerFd(TriggerFd_);
            }

        private:
            int TriggerFd_;
        };

        template<typename TCb>
        class TTriggerNode : public TTriggerNodeBase {
        public:
            TTriggerNode(int* fds, TCb&& cb)
                : TTriggerNodeBase(fds)
                , Cb_(cb)
            {
            }

            void Cb(int, int) override {
                Drain();
                Cb_();
            }

        private:
            TCb Cb_;
        };

        class TLoop {
        private:
            int QueueId;
            std::unique_ptr<TTriggerNodeBase> WakeupNode;

        public:
            TLoop();
            ~TLoop();

        public:
            void AddEvent(TNode& node, bool mod = true);
            void RemoveEvent(TNode& node);

            void Wake();
            bool Wait(const size_t capacity = 100);

            template<typename TCb>
            std::unique_ptr<TTriggerNodeBase> NewTrigger(TCb&& cb) {
                int fds[2];
                MakeFds(fds);

                auto out = std::unique_ptr<TTriggerNodeBase>(new TTriggerNode<TCb>(fds, std::forward<TCb>(cb)));

                AddEvent(*out, /* mod = */false);

                return out;
            }

        private:
            static void MakeFds(int* out);
        };
    }
}
