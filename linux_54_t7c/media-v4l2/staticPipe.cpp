
#define LOG_TAG "staticPipe"

#include <cstdlib>
#include <vector>

#include "staticPipe.h"


namespace android {

int staticPipe::fetchPipeMaxResolution(media_stream_t *stream, uint32_t& width, uint32_t &height) {
    auto cfg = matchSensorConfig(stream);
    if (cfg) {
        width = cfg->sensorWidth;
        height = cfg->sensorHeight;
        printf("find matched sensor configs %dx%d", width, height);
        return 0;
    }
    printf("do not find matched sensor configs");
    return -1;
}

int staticPipe::fetchSensorFormat(media_stream_t *stream, int hdrEnable) {
    auto cfg = matchSensorConfig(stream);
    if (cfg) {
        return hdrEnable ? cfg->wdrFormat : cfg->sdrFormat;
    }
    printf("do not find matched");
    return -1;
}
sensorType staticPipe::fetchSensorType(media_stream_t * stream) {
    auto cfg = matchSensorConfig(stream);
    if (cfg) {
        return cfg->type;
    }
    return sensor_NULL;
}
}
