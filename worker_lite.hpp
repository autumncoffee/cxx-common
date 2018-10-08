#pragma once

#include <stdlib.h>
#include <pthread.h>
#include <functional>
#include <memory>

namespace NAC {
    namespace NBase {
        class TWorkerLite {
        public:
            TWorkerLite() {
            }

            virtual void Start() {
                Thread = std::make_shared<pthread_t>();
                ThreadAttr = std::make_shared<pthread_attr_t>();
                RunArgs = std::make_shared<TRunArgs>();

                RunArgs->Cb = [this](){
                    Run();
                };

                pthread_attr_init(ThreadAttr.get());
                pthread_attr_setstacksize(ThreadAttr.get(), 8 * 1024 * 1024);

                pthread_create(Thread.get(), nullptr, RunImpl, (void*)RunArgs.get());
            }

            virtual ~TWorkerLite() {
                if(Thread) {
                    pthread_join(*Thread, nullptr);
                }

                if(ThreadAttr) {
                    pthread_attr_destroy(ThreadAttr.get());
                }
            }

            virtual void Run() = 0;

        private:
            std::shared_ptr<pthread_t> Thread;
            std::shared_ptr<pthread_attr_t> ThreadAttr;
            struct TRunArgs {
                std::function<void()> Cb;
            };
            std::shared_ptr<TRunArgs> RunArgs;

            static void* RunImpl(void* _args) {
                ((TRunArgs*)_args)->Cb();
                return nullptr;
            }
        };
    }
}
