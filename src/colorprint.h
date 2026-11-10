#ifndef COLORPRINT_H
#define COLORPRINT_H

#include <stdio.h>

#define COLOR_BLACK  "\033[0;30m"
#define COLOR_RED    "\033[0;31m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE   "\033[0;34m"
#define COLOR_PURPLE "\033[0;35m"
#define COLOR_CYAN   "\033[0;36m"
#define COLOR_WHITE  "\033[0;37m"

#define COLOR_BLACK_BOLD  "\033[1;30m"
#define COLOR_RED_BOLD    "\033[1;31m"
#define COLOR_GREEN_BOLD  "\033[1;32m"
#define COLOR_YELLOW_BOLD "\033[1;33m"
#define COLOR_BLUE_BOLD   "\033[1;34m"
#define COLOR_PURPLE_BOLD "\033[1;35m"
#define COLOR_CYAN_BOLD   "\033[1;36m"
#define COLOR_WHITE_BOLD  "\033[1;37m"

#define BOLD "\033[1m"

#define COLOR_RESET  "\033[0m"

#define color_printf(fg_color, fmt_string, ...) \
    printf(fg_color fmt_string COLOR_RESET, ##__VA_ARGS__)

#define color_println(fg_color, fmt_string, ...) \
    printf(fg_color fmt_string COLOR_RESET "\n", ##__VA_ARGS__)

#define color_fprintf(dest, fg_color, fmt_string, ...) \
    fprintf(dest, fg_color fmt_string COLOR_RESET, ##__VA_ARGS__)

#define color_fprintln(dest, fg_color, fmt_string, ...) \
    fprintf(dest, fg_color fmt_string COLOR_RESET "\n", ##__VA_ARGS__)

#endif
