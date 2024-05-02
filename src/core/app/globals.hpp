#pragma once

using cpu_word_size = unsigned int;

namespace globals {
    const long ROWS = 60;
    const long COLS = 60;
    const long NUM_BUTTON_LAYERS = 5;

    const long CHUNK_LENGTH = 8;
    const long CHUNK_AREA = CHUNK_LENGTH * CHUNK_LENGTH;

    const long TEXTBOXHEIGHT = 17;
    const long TEXTBOXWIDTH = 25;
    const long REGBOXWIDTH = 8;
    const long REGBOXHEIGHT = 1;
};  // namespace globals
