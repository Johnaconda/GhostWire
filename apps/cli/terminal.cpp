#include "terminal.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace ghostwire::cli {
bool interactive_terminal() noexcept {
#ifdef _WIN32
    DWORD input_mode{}, output_mode{};
    return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &input_mode) != 0 &&
           GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &output_mode) != 0;
#else
    return ::isatty(STDIN_FILENO) == 1 && ::isatty(STDOUT_FILENO) == 1;
#endif
}
} // namespace ghostwire::cli
