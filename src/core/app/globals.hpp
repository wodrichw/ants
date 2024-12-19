#pragma once

#include <mutex>

using cpu_word_size = unsigned int;

namespace globals {
    const long ROWS = 60;
    const long COLS = 60;
    const long NUM_BUTTON_LAYERS = 5;

    const long CHUNK_LENGTH = 8;
    const long CHUNK_AREA = CHUNK_LENGTH * CHUNK_LENGTH;

    const long MIN_SECTION_LENGTH = CHUNK_LENGTH * 4;
    const long MAX_SECTION_LENGTH = CHUNK_LENGTH * 16;

    const long WORLD_LENGTH = MAX_SECTION_LENGTH * 2;
    const long MAX_LEVEL_DEPTH = 10;

    const long TEXTBOXHEIGHT = 17;
    const long TEXTBOXWIDTH = 25;
    const long REGBOXWIDTH = 8;
    const long REGBOXHEIGHT = 1;

    extern std::mutex threadpool_mutex;
};  // namespace globals
