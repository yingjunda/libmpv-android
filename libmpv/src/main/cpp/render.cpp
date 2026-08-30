#include <jni.h>

#include <mpv/client.h>

#include "jni_utils.h"
#include "log.h"
#include "globals.h"

extern "C" {
    jni_func(void, nativeAttachSurface, jlong instance, jobject surface);
    jni_func(void, nativeDetachSurface, jlong instance);
}

jni_func(void, nativeAttachSurface, jlong instance, jobject surface) {
    auto mpv_instance = reinterpret_cast<MPVInstance*>(instance);

    if (mpv_instance->surface)
        env->DeleteGlobalRef(mpv_instance->surface);
    mpv_instance->surface = env->NewGlobalRef(surface);
    if (!mpv_instance->surface) {
        die(env, "invalid surface provided");
        return;
    }

    // mpv 的 wid 是一个既作为 option 又作为运行时 property 的属性。在
    // mpv_initialize() 之后，必须使用 mpv_set_property 才能正确同步到 VO 的
    // WinID（mpv_set_option 仅 startup 生效，init 后调用不会触发 VO 重建，
    // 会导致 vo_mediacodec_embed 在 preinit 时读到 WinID=0 而断言崩溃）。
    int64_t wid = reinterpret_cast<intptr_t>(mpv_instance->surface);
    int result = mpv_set_property(mpv_instance->mpv, "wid", MPV_FORMAT_INT64, &wid);
    if (result < 0)
        ALOGE("mpv_set_property(wid) returned error %s", mpv_error_string(result));
    else
        ALOGI("mpv_set_property(wid) succeeded wid=%lld", (long long)wid);
}

jni_func(void, nativeDetachSurface, jlong instance) {
    auto mpv_instance = reinterpret_cast<MPVInstance*>(instance);
    int64_t wid = 0;
    int result = mpv_set_property(mpv_instance->mpv, "wid", MPV_FORMAT_INT64, &wid);
    if (result < 0)
        ALOGE("mpv_set_property(wid) returned error %s", mpv_error_string(result));

    env->DeleteGlobalRef(mpv_instance->surface);
    mpv_instance->surface = nullptr;
}
