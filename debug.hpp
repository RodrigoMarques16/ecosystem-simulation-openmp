#pragma once

#include <stdio.h>
#include <iostream>
#include <string>

#ifdef DEBUG

namespace dbg {

FILE *LOGFILE = stdout;

#define LOGVAR(X)                                   \
    if (DEBUG) {                          \
        fprintf(dbg::LOGFILE, "%s: %d\n", #X, (X)); \
        fflush(dbg::LOGFILE);                       \
    }

void LOG(std::string s) {
    if (DEBUG) {
        fputs(s.c_str(), LOGFILE);
        fflush(LOGFILE);
    }
}

template <typename... Ts>
void LOG(std::string fmt, const Ts &... args) {
    if (DEBUG) {
        fprintf(LOGFILE, fmt.c_str(), args...);
        fflush(LOGFILE);
    }
}

template <typename... Ts>
void LOGLN(std::string fmt, const Ts &... args) {
    if (DEBUG) {
        LOG(fmt, args...);
        fputc('\n', LOGFILE);
    }
}

} // namespace dbg

#endif