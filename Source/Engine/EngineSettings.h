#pragma once

class Settings_t {
public:
    enum {
        JobQueueThreads			= 2,
    };

    struct FileSystem {
        enum {
            DataBuffer			= 10 * 1024 * 1024, //10MB

            PathReserve			= 128,
            DefaultLoadFlags	= 0,
        };
    };
    
    struct Scenes {
        static inline float GetSceneLoadTimeOut() { return 0.2f; }
    };
};
