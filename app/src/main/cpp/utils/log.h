//
// Created by Felix on 2024/10/25.
//

#ifndef TESTOPENSL_LOG_H
#define TESTOPENSL_LOG_H

#include <android/log.h>

#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG, "opensl", FORMAT)
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR, "opensl", FORMAT)

#endif //TESTOPENSL_LOG_H
