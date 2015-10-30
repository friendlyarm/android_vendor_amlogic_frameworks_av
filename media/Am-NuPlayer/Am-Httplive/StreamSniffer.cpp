/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 //#define LOG_NDEBUG 0
#define LOG_TAG "StreamSniffer"
#include <utils/Log.h>

#include "StreamSniffer.h"

#include <cutils/properties.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/Utils.h>

namespace android {

#define kHTTPSourceSizeDefault 1024

#define EXTM3U "#EXTM3U"

StreamSniffer::StreamSniffer(const char * url) {
    if (url) {
        mURL.setTo(url);
    }
}

StreamSniffer::~StreamSniffer() {
}

int32_t StreamSniffer::isBOMHeader(ABitReader * br) {
    int32_t ret;
    ret = br->getBits(8);
    if ((ret == 0xEF) && (br->getBits(8) == 0xBB) && (br->getBits(8) == 0xBF)) {
        ALOGV("This m3u8 is encoded with UTF-8 !\n");
        return br->getBits(8);
    } else if ((ret == 0xFF) && (br->getBits(8) == 0xFE)) {
        ALOGV("This m3u8 is encoded with UTF-16LE !\n");
        return br->getBits(8);
    } else if ((ret == 0XFE) && (br->getBits(8) == 0xFF)) {
        ALOGV("This m3u8 is encoded with UTF-16BE !\n");
        return br->getBits(8);
    }
    return ret;
}

status_t StreamSniffer::tryHLSParser(ABitReader * br) {
    char line[1024] = {0};
    char * ptr = line;
    int32_t tp;
    for (;;) {
        tp = isBOMHeader(br);
        if (tp < 0) {
            return UNKNOWN_ERROR;
        }
        if (tp == '\n' || tp == '\0') {
            if (ptr > line && ptr[-1] == '\r') {
                ptr--;
            }
            *ptr = '\0';
            break;
        } else {
            if ((ptr - line) < 1023) {
                *ptr++ = tp;
            }
        }
    }

    if (!strncmp(line, EXTM3U, strlen(EXTM3U))) {
        return OK;
    }

    return UNKNOWN_ERROR;
}

}
