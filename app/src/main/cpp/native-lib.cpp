#include <jni.h>
#include <string>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "utils/log.h"

static SLObjectItf objectItf = nullptr;

SLEngineItf CreateSL() {
    SLresult re;
    SLEngineItf engineItf;
    re = slCreateEngine(&objectItf, 0, nullptr, 0, nullptr, nullptr);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    re = (*objectItf)->Realize(objectItf, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    re = (*objectItf)->GetInterface(objectItf, SL_IID_ENGINE, &engineItf);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    return engineItf;
}

void PcmCallback(SLAndroidSimpleBufferQueueItf queueItf, void *context) {
    LOGE("PcmCallback");
    static FILE *fp = nullptr;
    static char *buf = nullptr;
    if (!fp) {
        fp = fopen("/sdcard/audio.pcm", "rb");
    }
    if (!fp) {
        LOGE("Open Pcm file error");
        return;
    }
    if (feof(fp) == 0) {
        size_t len = fread(buf, 1, 1024, fp);
        if (len > 0) {
            (*queueItf)->Enqueue(queueItf, buf, len);
        }
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_felix_test_opensl_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

    //创建引擎
    SLresult re;
    SLEngineItf engineItf = CreateSL();
    if (engineItf) {
        LOGE("Create Success");
        //创建混音器
        SLObjectItf mix = nullptr;
        re = (*engineItf)->CreateOutputMix(engineItf, &mix, 0, nullptr, nullptr);
        if (re != SL_RESULT_SUCCESS) {
            LOGE("CreateOutputMix Failed");
        }
        re = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
        if (re != SL_RESULT_SUCCESS) {
            LOGE("mix Realize Failed");
        }
        SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mix};
        SLDataSink audioSink = {&outputMix, nullptr};

        //配置音频信息
        SLDataLocator_AndroidSimpleBufferQueue queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                        10};
        SLDataFormat_PCM pcm = {
                SL_DATAFORMAT_PCM,
                2,//声道
                SL_SAMPLINGRATE_44_1,
                SL_PCMSAMPLEFORMAT_FIXED_16,
                SL_PCMSAMPLEFORMAT_FIXED_16,
                SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                SL_BYTEORDER_LITTLEENDIAN, //字节序 小端模式
        };

        SLDataSource ds = {&queue, &pcm};

        //创建播放器
        SLObjectItf player = nullptr;
        const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
        const SLboolean req[] = {SL_BOOLEAN_TRUE};
        re = (*engineItf)->CreateAudioPlayer(engineItf, &player, &ds, &audioSink,
                                             sizeof(ids) / sizeof(SLInterfaceID), ids,
                                             req);
        if (re != SL_RESULT_SUCCESS) {
            LOGE("CreateAudioPlayer Failed");
        } else {
            LOGE("CreateAudioPlayer Success");
            (*player)->Realize(player, SL_BOOLEAN_FALSE);

            //获取接口
            SLPlayItf playerImpl = nullptr;
            SLAndroidSimpleBufferQueueItf pcmQueue = nullptr;
            re = (*player)->GetInterface(player, SL_IID_PLAY, &playerImpl);
            if (re != SL_RESULT_SUCCESS) {
                LOGE("playerImpl GetInterface SL_IID_PLAY Failed");
            }
            re = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQueue);
            if (re != SL_RESULT_SUCCESS) {
                LOGE("playerImpl GetInterface SL_IID_BUFFERQUEUE Failed");
            }

            //设置回调函数，在播放队列空的时候调用
            (*pcmQueue)->RegisterCallback(pcmQueue, PcmCallback, nullptr);

            //播放
            (*playerImpl)->SetPlayState(playerImpl, SL_PLAYSTATE_PLAYING);

            //启动队列回调
            (*pcmQueue)->Enqueue(pcmQueue, "", 1);
        }
    } else {
        LOGE("Create Failed");
    }


    return env->NewStringUTF(hello.c_str());
}