/*
 * Copyright (c) 2018 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>

#include "logs.h"

#include "media-v4l2/mediactl.h"
#include "media-v4l2/v4l2subdev.h"
#include "media-v4l2/v4l2videodev.h"

#include "aml_isp_api.h"

#include "./sensor/imx290/imx290_api.h"
//#define WDR_ENABLE
enum {
    WDR_MODE_NONE,
    WDR_MODE_2To1_LINE,
    WDR_MODE_2To1_FRAME,
    SDR_DDR_MODE,
    ISP_SDR_DCAM_MODE,
};

#define NB_BUFFER                4

#define V4L2_META_AML_ISP_CONFIG    v4l2_fourcc('A', 'C', 'F', 'G') /* Aml isp config */
#define V4L2_META_AML_ISP_STATS     v4l2_fourcc('A', 'S', 'T', 'S') /* Aml isp statistics */

int image_width = 640;
int image_height = 480;

#ifndef ANDROID
#define RTSP 0
#endif

typedef struct pipe_info{
    char * media_dev_name ;
    char * sensor_ent_name ;
    char * csiphy_ent_name;
    char * adap_ent_name;
    char * isp_ent_name;
    char * video_ent_name;
    char * video_stats_name;
    char * video_param_name;
} pipe_info_t;

struct pipe_info pipe_0 = {
   .media_dev_name  = "/dev/media0",
   .sensor_ent_name = "imx290-0",//"ov08a10-1",//"imx290-0",//
   .csiphy_ent_name = "isp-csiphy",
   .adap_ent_name   = "isp-adapter",
   .isp_ent_name   = "isp-core",
   .video_ent_name  = "isp-output0",
   .video_stats_name  = "isp-stats",
   .video_param_name  = "isp-param",
};

struct pipe_info pipe_1 = {
   .media_dev_name  = "/dev/media0",
   .sensor_ent_name = "imx290-0",
   .csiphy_ent_name = "isp-csiphy",
   .adap_ent_name   = "isp-adapter",
   .isp_ent_name   = "isp-core",
   .video_ent_name  = "isp-output3",
   .video_stats_name  = "isp-stats",
   .video_param_name  = "isp-param",
};


/*struct pipe_info pipe_1 = {
    .media_dev_name  = "/dev/media1",
    .sensor_ent_name = "imx290-1",
    .csiphy_ent_name = "t7-csi2phy-1",
    .adap_ent_name   = "t7-adapter-1",
    .video_ent_name  = "t7-video-1-0"
};*/

enum {
    ARM_V4L2_TEST_STREAM_OUTPUT0,
    ARM_V4L2_TEST_STREAM_RAW,
    ARM_V4L2_TEST_STREAM_STATS,
    ARM_V4L2_TEST_STREAM_MAX
};

typedef struct camera_configuration{
    struct aml_format format;
} camera_configuration_t;

typedef struct media_stream {
    char media_dev_name[64];
    char sensor_ent_name[32];
    char csiphy_ent_name[32];
    char adap_ent_name[32];
    char isp_ent_name[32];
    char video_ent_name[32];
    char video_stats_name[32];
    char video_param_name[32];

    struct media_device  * media_dev;

    struct media_entity  * sensor_ent;
    struct media_entity  * csiphy_ent;
    struct media_entity  * adap_ent;
    struct media_entity  * isp_ent;
    struct media_entity  * video_ent;
    struct media_entity  * video_stats;
    struct media_entity  * video_param;

} media_stream_t;

struct thread_info {
    pthread_t p_id;
    uint32_t status;//1: run 0:stop
    sem_t p_sem;

    aisp_calib_info_t calib;
    AML_ALG_CTX_S pstAlgCtx;
};

pthread_t tid[ARM_V4L2_TEST_STREAM_MAX];

/* config parameters */
struct thread_param {
    /* v4l2 variables */
    struct media_stream         v4l2_media_stream;
    void                        *v4l2_mem_param;
    void                        *v4l2_mem[NB_BUFFER];
    int                          param_buf_length;
    int                          stats_buf_length;

    char                        *mediadevname;
    /* video device info */
    char                        *devname;

    /* display device info */
    char                        *fbp;
    struct fb_var_screeninfo    vinfo;
    struct fb_fix_screeninfo    finfo;

    /* format info */
    uint32_t                    width;
    uint32_t                    height;
    uint32_t                    pixformat;
    uint32_t                    fmt_code;
    uint32_t                    wdr_mode;
    uint32_t                    exposure;

    /* for snapshot stream (non-zsl implementation) */
    int32_t                     capture_count;
    int32_t                     gdc_ctrl;
    int                         videofd;
    uint32_t                    c_width;
    uint32_t                    c_height;
    uint32_t                    a_ctrl;
    int                         fps;
    int                         pipe_idx;

    struct thread_info          info;
};

static int media_stream_init(media_stream_t * stream, struct pipe_info *pipe_info_ptr){

    memset(stream, 0, sizeof(*stream));

    strncpy(stream->media_dev_name, pipe_info_ptr->media_dev_name, sizeof(stream->media_dev_name));

    strncpy(stream->sensor_ent_name, pipe_info_ptr->sensor_ent_name, sizeof(stream->sensor_ent_name));
    strncpy(stream->csiphy_ent_name, pipe_info_ptr->csiphy_ent_name, sizeof(stream->csiphy_ent_name));
    strncpy(stream->adap_ent_name,   pipe_info_ptr->adap_ent_name,   sizeof(stream->adap_ent_name));
    strncpy(stream->isp_ent_name,   pipe_info_ptr->isp_ent_name,   sizeof(stream->isp_ent_name));
    strncpy(stream->video_ent_name,  pipe_info_ptr->video_ent_name,  sizeof(stream->video_ent_name));
    strncpy(stream->video_stats_name,   pipe_info_ptr->video_stats_name,   sizeof(stream->video_stats_name));
    strncpy(stream->video_param_name,  pipe_info_ptr->video_param_name,  sizeof(stream->video_param_name));

    stream->media_dev = media_device_new(stream->media_dev_name);

    if (NULL == stream->media_dev) {
        ERR("new media dev fail\n");
        return -1;
    }

    if (0 != media_device_enumerate(stream->media_dev) ) {
        ERR("media_device_enumerate fail\n");
        return -1;
    }

    int node_num = media_get_entities_count(stream->media_dev);
    for (int ii = 0; ii <node_num; ++ii) {
        struct media_entity * ent = media_get_entity(stream->media_dev, ii);
        INFO("ent %d, name %s \n", ii, ent->info.name);
    }

    stream->sensor_ent = media_get_entity_by_name(stream->media_dev, stream->sensor_ent_name, strlen(stream->sensor_ent_name));
    if (NULL == stream->sensor_ent) {
        ERR("get  sensor_ent fail\n");
        return -1;
    }

    stream->csiphy_ent = media_get_entity_by_name(stream->media_dev, stream->csiphy_ent_name, strlen(stream->csiphy_ent_name));
    if (NULL == stream->csiphy_ent) {
        ERR("get  csiphy_ent fail\n");
        return -1;
    }

    stream->adap_ent = media_get_entity_by_name(stream->media_dev, stream->adap_ent_name, strlen(stream->adap_ent_name));
    if (NULL == stream->adap_ent) {
        ERR("get  adap_ent fail\n");
        return -1;
    }

    stream->isp_ent = media_get_entity_by_name(stream->media_dev, stream->isp_ent_name, strlen(stream->isp_ent_name));
    if (NULL == stream->isp_ent) {
        ERR("get  isp_ent fail\n");
        return -1;
    }

    stream->video_ent = media_get_entity_by_name(stream->media_dev, stream->video_ent_name, strlen(stream->video_ent_name));
    if (NULL == stream->video_ent) {
        ERR("get video_ent fail\n");
        return -1;
    }

    stream->video_stats = media_get_entity_by_name(stream->media_dev, stream->video_stats_name, strlen(stream->video_stats_name));
    if (NULL == stream->video_stats) {
        ERR("get  video_stats fail\n");
        return -1;
    }

    stream->video_param = media_get_entity_by_name(stream->media_dev, stream->video_param_name, strlen(stream->video_param_name));
    if (NULL == stream->video_param) {
        ERR("get  video_param fail\n");
        return -1;
    }

    MSG("media stream init success\n");
    return 0;
}


int createLinks(media_stream_t *camera)
{
    // 0 = sink; 1 = src
    const int sink_pad_idx = 0;
    const int src_pad_idx = 1;

    int rtn = -1;
    struct media_pad      *src_pad;
    struct media_pad      *sink_pad;

    int flag = MEDIA_LNK_FL_ENABLED;

    MSG("create link ++\n");

    if (camera->media_dev == NULL) {
        return 0;
    }

/*source:adap_ent sink:isp_ent*/
    sink_pad = (struct media_pad*) media_entity_get_pad(camera->isp_ent, sink_pad_idx);
    if (!sink_pad) {
        ERR("Failed to get isp sink pad[0]");
        return rtn;
    }

    src_pad = (struct media_pad*)media_entity_get_pad(camera->adap_ent, src_pad_idx);
    if (!src_pad) {
        ERR("Failed to get adap_ent src pad[1]");
        return rtn;
    }

    rtn = media_setup_link( camera->media_dev, src_pad, sink_pad, flag);

/*source:csiphy_ent sink:adap_ent*/
    sink_pad = (struct media_pad*) media_entity_get_pad(camera->adap_ent, sink_pad_idx);
    if (!sink_pad) {
        ERR("Failed to get adap sink pad[0]");
        return rtn;
    }

    src_pad = (struct media_pad*)media_entity_get_pad(camera->csiphy_ent, src_pad_idx);
    if (!src_pad) {
        ERR("Failed to get csiph src pad[1]");
        return rtn;
    }

    rtn = media_setup_link( camera->media_dev, src_pad, sink_pad, flag);
    if (0 != rtn) {
        ERR( "Failed to link adap with csiphy");
        return rtn;
    }

/*source:sensor_ent sink:csiphy_ent*/
    // sensor only has 1 pad
    src_pad =  (struct media_pad*)media_entity_get_pad(camera->sensor_ent, 0);
    if (!src_pad) {
        ERR("Failed to get sensor src pad[0]");
        return rtn;
    }

    sink_pad = (struct media_pad*)media_entity_get_pad(camera->csiphy_ent, sink_pad_idx);
    if (!sink_pad) {
        ERR("Failed to get csiph sink pad[1]");
        return rtn;
    }

    rtn = media_setup_link( camera->media_dev, src_pad, sink_pad, flag);
    if (0 != rtn) {
        ERR( "Failed to link sensor with csiphy");
        return rtn;
    }

    MSG("create link success \n");
    return rtn;
}

int media_set_WdrMode(media_stream_t *camera, uint32_t wdr_mode)
{
    int rtn = -1;

    MSG("%s ++ wdr_mode : %d \n", __func__,wdr_mode);

    // sensor wdr mode
    rtn = v4l2_subdev_set_wdr(camera->sensor_ent, wdr_mode);
    if (rtn < 0) {
        ERR("Failed to set sensor wdr mode");
        return rtn;
    }

    // sensor wdr mode
    rtn = v4l2_subdev_set_wdr(camera->adap_ent, wdr_mode);
    if (rtn < 0) {
        ERR("Failed to set adapter wdr mode");
        return rtn;
    }

    // sensor wdr mode
    rtn = v4l2_subdev_set_wdr(camera->isp_ent, wdr_mode);
    if (rtn < 0) {
        ERR("Failed to set isp wdr mode");
        return rtn;
    }

    MSG("%s success --\n", __func__);

    return rtn;
}


int setSdFormat(media_stream_t *camera, camera_configuration_t *cfg)
{
    int rtn = -1;

    struct v4l2_mbus_framefmt mbus_format;

    mbus_format.width  = cfg->format.width;
    mbus_format.height = cfg->format.height;
    mbus_format.code   = cfg->format.code;

    int which = V4L2_SUBDEV_FORMAT_ACTIVE;

    MSG("%s ++\n", __func__);

    // sensor source pad fmt
    rtn = v4l2_subdev_set_format(camera->sensor_ent,
          &mbus_format, 0, which);
    if (rtn < 0) {
        ERR("Failed to set sensor format");
        return rtn;
    }

    cmos_set_sensor_entity(camera->sensor_ent, 0);

    // csiphy source & sink pad fmt
    rtn = v4l2_subdev_set_format(camera->csiphy_ent,
          &mbus_format, 0, which);
    if (rtn < 0) {
        ERR("Failed to set csiphy pad[0] format");
        return rtn;
    }

    rtn = v4l2_subdev_set_format(camera->csiphy_ent,
          &mbus_format, 1, which);
    if (rtn < 0) {
        ERR("Failed to set csiphy pad[1] format");
        return rtn;
    }

    // adap source & sink pad fmt
    rtn = v4l2_subdev_set_format(camera->adap_ent,
          &mbus_format, 0, which);

    if (rtn < 0) {
        ERR("Failed to set adap pad[0] format");
        return rtn;
    }

    rtn = v4l2_subdev_set_format(camera->adap_ent,
          &mbus_format, 1, which);
    if (rtn < 0) {
        ERR("Failed to set adap pad[1] format");
        return rtn;
    }

    // isp source & sink pad fmt
    rtn = v4l2_subdev_set_format(camera->isp_ent,
          &mbus_format, 0, which);

    if (rtn < 0) {
        ERR("Failed to set isp pad[0] format");
        return rtn;
    }

    MSG("%s success --\n", __func__);

    return rtn;
}

int setImgFormat(media_stream_t *camera, camera_configuration_t *cfg)
{
    int rtn = -1;
    struct v4l2_format          v4l2_fmt;

    MSG("%s ++\n", __func__);

    memset (&v4l2_fmt, 0, sizeof (struct v4l2_format));
    if (cfg->format.nplanes > 1) {
        ERR ("not supported yet!");
        //return -1;
    }

    v4l2_fmt.type                    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_fmt.fmt.pix_mp.width        = image_width;//cfg->format.width;
    v4l2_fmt.fmt.pix_mp.height       = image_height;//cfg->format.height;
    v4l2_fmt.fmt.pix_mp.pixelformat  = cfg->format.fourcc;
    v4l2_fmt.fmt.pix_mp.field        = V4L2_FIELD_ANY;

    rtn = v4l2_video_set_format( camera->video_ent,&v4l2_fmt);
    if (rtn < 0) {
        ERR("Failed to set video fmt, ret %d\n", rtn);
        return rtn;
    }

    MSG("%s success --\n", __func__);

    return rtn;
}

int setDataFormat(media_stream_t *camera, camera_configuration_t *cfg)
{
    int rtn = -1;
    struct v4l2_format          v4l2_fmt;

    MSG("%s ++\n", __func__);

    memset (&v4l2_fmt, 0, sizeof (struct v4l2_format));

    v4l2_fmt.type                    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_fmt.fmt.pix_mp.width        = cfg->format.width;
    v4l2_fmt.fmt.pix_mp.height       = cfg->format.height;
    v4l2_fmt.fmt.pix_mp.pixelformat  = V4L2_META_AML_ISP_STATS;
    v4l2_fmt.fmt.pix_mp.field        = V4L2_FIELD_ANY;

    rtn = v4l2_video_set_format( camera->video_stats,&v4l2_fmt);
    if (rtn < 0) {
        ERR("Failed to set video fmt, ret %d\n", rtn);
        return rtn;
    }

    MSG("%s success\n", __func__);

    return 0;
}

int setConfigFormat(media_stream_t *camera, camera_configuration_t *cfg)
{
    int rtn = -1;
    struct v4l2_format          v4l2_fmt;

    MSG("%s ++\n", __func__);

    memset (&v4l2_fmt, 0, sizeof (struct v4l2_format));

    v4l2_fmt.type                    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_fmt.fmt.pix_mp.width        = cfg->format.width;
    v4l2_fmt.fmt.pix_mp.height       = cfg->format.height;
    v4l2_fmt.fmt.pix_mp.pixelformat  = V4L2_META_AML_ISP_CONFIG;
    v4l2_fmt.fmt.pix_mp.field        = V4L2_FIELD_ANY;

    rtn = v4l2_video_set_format( camera->video_param,&v4l2_fmt);
    if (rtn < 0) {
        ERR("Failed to set video fmt, ret %d\n", rtn);
        return rtn;
    }

    MSG("%s success\n", __func__);

    return 0;
}

int media_stream_config(media_stream_t * stream, camera_configuration_t *cfg)
{
    int rtn = -1;

    MSG("%s ++\n", __func__);

    rtn = setSdFormat(stream, cfg);
    if (rtn < 0) {
        ERR("Failed to set subdev format");
        return rtn;
    }

    rtn = setImgFormat(stream, cfg);
    if (rtn < 0) {
        ERR("Failed to set image format");
        return rtn;
    }

    rtn = createLinks(stream);
    if (rtn) {
        ERR( "Failed to create links");
        return rtn;
    }

    MSG("Success to config media stream \n");

    return 0;
}


int media_stream_start(media_stream_t * stream, int type)
{
    return v4l2_video_stream_on(stream->video_ent, type);
}

int media_stream_stop(media_stream_t * stream, int type)
{
    return v4l2_video_stream_off(stream->video_ent, type);
}

/**********
 * helper functions
 */
uint64_t getTimestamp() {
    struct timespec ts;
    int rc;

    rc = clock_gettime(0x00, &ts);
    if (rc == 0) {
        return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    } else {
        return 0;
    }
}

int64_t GetTimeMsec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static int do_get_dma_buf_fd(int videofd, uint32_t index, uint32_t plane)
{
    struct v4l2_exportbuffer ex_buf;

    ex_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ex_buf.index = index;
    ex_buf.plane = plane;
    ex_buf.flags = 0;
    ex_buf.fd = -1;

    if (ioctl(videofd, VIDIOC_EXPBUF, &ex_buf))  {
        ERR("LIKE-0:Failed get dma buf fd\n");
    }

    return ex_buf.fd;
}


void save_img(const char* prefix, unsigned char *buff, unsigned int size, int flag, int num)
{
    char name[60] = {'\0'};
    int fd = -1;

    if (buff == NULL || size == 0) {
        printf("%s:Error input param\n", __func__);
        return;
    }

    if (num > 100)
        return;

    if (num % 20 != 0)
        return;

    #ifdef ANDROID
    sprintf(name, "/sdcard/DCIM/ca_%s-%d_dump-%d.yuv", prefix, flag, num);
    #else
    sprintf(name, "/tmp/ca_%s-%d_dump-%d.yuv", prefix, flag, num);
    #endif

    fd = open(name, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        printf("%s:Error open file\n", __func__);
        return;
    }
    write(fd, buff, size);
    close(fd);
}

int get_file_size(char *f_name)
{
    int f_size = -1;
    FILE *fp = NULL;

    if (f_name == NULL) {
        printf("Error file name\n");
        return f_size;
    }

    fp = fopen(f_name, "rb");
    if (fp == NULL) {
        printf("Error open file %s\n", f_name);
        return f_size;
    }

    fseek(fp, 0, SEEK_END);

    f_size = ftell(fp);

    fclose(fp);

    printf("%s: size %d\n", f_name, f_size);

    return f_size;
}

void isp_param_init(struct media_stream v4l2_media_stream, struct thread_param *tparm)
{
    struct v4l2_requestbuffers  v4l2_rb;
    int rc, i;
    int total_mapped_mem=0;
    struct v4l2_buffer v4l2_buf;
    char alg_init[256*1024];

    camera_configuration_t     stream_config;
    stream_config.format.width = 1024;
    stream_config.format.height = 256;
    stream_config.format.nplanes   = 1;

    rc = setDataFormat(&v4l2_media_stream, &stream_config);
    if (rc < 0) {
        ERR("Failed to set stats format");
        return;
    }

    rc = setConfigFormat(&v4l2_media_stream, &stream_config);
    if (rc < 0) {
        ERR("Failed to set param format");
        return;
    }

    /* request buffers */
    memset (&v4l2_rb, 0, sizeof (struct v4l2_requestbuffers));
    v4l2_rb.count  = NB_BUFFER;
    v4l2_rb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_rb.memory = V4L2_MEMORY_MMAP;
    rc = v4l2_video_req_bufs(tparm->v4l2_media_stream.video_stats, &v4l2_rb);
    if (rc < 0) {
        printf("Error: request buffer.\n");
        return;
    }

    memset (&v4l2_rb, 0, sizeof (struct v4l2_requestbuffers));
    v4l2_rb.count  = 1;
    v4l2_rb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_rb.memory = V4L2_MEMORY_MMAP;
    rc = v4l2_video_req_bufs(v4l2_media_stream.video_param, &v4l2_rb);
    if (rc < 0) {
        ERR("Failed to req_bufs");
        return;
    }

    /* map stats buffers */
    for (i = 0; i < NB_BUFFER; i++) {
        memset (&v4l2_buf, 0, sizeof (struct v4l2_buffer));
        v4l2_buf.index   = i;
        v4l2_buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory  = V4L2_MEMORY_MMAP;
        rc = v4l2_video_query_buf(tparm->v4l2_media_stream.video_stats, &v4l2_buf);
        if (rc < 0) {
            printf("Error: query buffer %d.\n", rc);
            return;
        }

        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
            tparm->stats_buf_length = v4l2_buf.length;
            INFO("video capture. length: %u offset: %u\n", v4l2_buf.length, v4l2_buf.m.offset);
            tparm->v4l2_mem[i] = mmap (0, v4l2_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                tparm->v4l2_media_stream.video_stats->fd, v4l2_buf.m.offset);
            ++total_mapped_mem;
            INFO("Buffer[%d] mapped at address 0x%p total_mapped_mem:%d.\n", i, tparm->v4l2_mem[i], total_mapped_mem);
        }
        if (tparm->v4l2_mem[i] == MAP_FAILED) {
            ERR("Error: mmap buffers.\n");
            return;
        }
    }

    /* map buffers */
    memset (&v4l2_buf, 0, sizeof (struct v4l2_buffer));
    v4l2_buf.index   = 0;
    v4l2_buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory  = V4L2_MEMORY_MMAP;
    rc = v4l2_video_query_buf(v4l2_media_stream.video_param, &v4l2_buf);
    if (rc < 0) {
        ERR("Failed to query bufs");
        return;
    }

    if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        tparm->param_buf_length = v4l2_buf.length;
        INFO("[T#2] type video capture. length: %u offset: %u\n", v4l2_buf.length, v4l2_buf.m.offset);
        tparm->v4l2_mem_param = mmap (0, v4l2_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
            v4l2_media_stream.video_param->fd, v4l2_buf.m.offset);
        INFO("[T#2] Buffer[0] mapped at address 0x%p total_mapped_mem:%d.\n", tparm->v4l2_mem_param, total_mapped_mem);
    }
    if (tparm->v4l2_mem_param == MAP_FAILED) {
        ERR("[T#2] Error: mmap buffers.\n");
        return;
    }

    cmos_sensor_control_cb(&tparm->info.pstAlgCtx.stSnsExp);

    cmos_get_sensor_calibration(&tparm->info.calib);

    aisp_enable(0, &tparm->info.pstAlgCtx, &tparm->info.calib);
    memset(alg_init, 0, sizeof(alg_init));

    aisp_alg2user(0, alg_init);
    aisp_alg2kernel(0, tparm->v4l2_mem_param);

    /* queue buffers */
    DBG("[T#0] begin to Queue buf.\n");
    for (i = 0; i < NB_BUFFER; ++i) {
        memset (&v4l2_buf, 0, sizeof (struct v4l2_buffer));
        v4l2_buf.index   = i;
        v4l2_buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory  = V4L2_MEMORY_MMAP;
        rc = v4l2_video_q_buf( tparm->v4l2_media_stream.video_stats, &v4l2_buf );
        if (rc < 0) {
            ERR("Error: queue buffers, rc:%d i:%d\n",rc, i);
            return;
        }
    }

    memset (&v4l2_buf, 0, sizeof (struct v4l2_buffer));
    v4l2_buf.index   = 0;
    v4l2_buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory  = V4L2_MEMORY_MMAP;
    rc = v4l2_video_q_buf( v4l2_media_stream.video_param, &v4l2_buf );
    if (rc < 0) {
        ERR("Error: queue buffers, rc:%d\n", rc);
    }

    rc = v4l2_video_stream_on(tparm->v4l2_media_stream.video_stats, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    if (rc < 0) {
        ERR("[T#0] Error: streamon.\n");
        return;
    }

    rc = v4l2_video_stream_on(v4l2_media_stream.video_param, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    if (rc < 0) {
        ERR("[T#0] Error: streamon.\n");
        return;
    }

    DBG("[T#0] Finished alg_param_init.\n");
}

void * video_thread(void *arg)
{
    struct v4l2_capability      v4l2_cap;
    struct v4l2_format          v4l2_fmt;
    struct v4l2_requestbuffers  v4l2_rb;

    int                         v4l2_buf_length = 0;

    int                         dma_fd = -1;

    void                        *v4l2_mem[NB_BUFFER*VIDEO_MAX_PLANES];
    int                         v4l2_dma_fd[NB_BUFFER * VIDEO_MAX_PLANES] = {0};
    int                         total_mapped_mem=0;

    /* thread parameters */
    struct thread_param         *tparm = (struct thread_param *)arg;
    pthread_t                   cur_pthread = pthread_self();
    int                         stream_type = -1;

    /* condition & loop flags */
    int                         rc = 0;
    int                         i,j;

    uint32_t          v4l2_enum_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    uint64_t display_count = 0;
    int64_t start, end;
    struct pipe_info *pipe_ptr = NULL;

    /**************************************************
     * find thread id
     *************************************************/
    for (i = 0; i < ARM_V4L2_TEST_STREAM_MAX; i++) {
        if (cur_pthread == tid[i]) {
            stream_type = i;
            break;
        }
    }

    for (i = 0; i < NB_BUFFER * VIDEO_MAX_PLANES; i++) {
        v4l2_dma_fd[i] = -1;
    }
    if (stream_type == ARM_V4L2_TEST_STREAM_OUTPUT0)
        pipe_ptr = &pipe_0;
    else if(stream_type == ARM_V4L2_TEST_STREAM_RAW)
        pipe_ptr = &pipe_1;

    if (tparm->mediadevname ) {
        pipe_ptr->media_dev_name = tparm->mediadevname;
    }

    rc = media_stream_init(&tparm->v4l2_media_stream, pipe_ptr);
    if (0 != rc) {
        ERR("[T#%d] The %s device init fail.\n", stream_type, tparm->mediadevname);
        goto fatal; ;
    }
    INFO("[T#%d] The %s device was opened successfully. stream init ok\n", stream_type, tparm->mediadevname);

    /* check capability */
    memset (&v4l2_cap, 0, sizeof (struct v4l2_capability));
    rc = v4l2_video_get_capability(tparm->v4l2_media_stream.video_ent, &v4l2_cap);
    if (rc < 0) {
        ERR ("[T#%d] Error: get capability.\n", stream_type);
        goto fatal;
    }
    INFO("[T#%d] VIDIOC_QUERYCAP: cap.driver = %s, capabilities=0x%x, device_caps:0x%x\n",
        stream_type, v4l2_cap.driver, v4l2_cap.capabilities, v4l2_cap.device_caps);

    media_set_WdrMode(&tparm->v4l2_media_stream,tparm->wdr_mode);

    /* config & set format */
    camera_configuration_t     stream_config ;
    stream_config.format.width = tparm->width;
    stream_config.format.height = tparm->height;
    stream_config.format.fourcc = tparm->pixformat;
    stream_config.format.code   = tparm->fmt_code;
    stream_config.format.nplanes   = 1;

    rc = media_stream_config(&tparm->v4l2_media_stream, &stream_config);
    if (rc < 0) {
        ERR("[T#%d] Error: config stream %d.\n", stream_type, rc);
        goto fatal;
    }

    memset (&v4l2_fmt, 0, sizeof (struct v4l2_format));
    v4l2_fmt.type                    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_fmt.fmt.pix_mp.width        = stream_config.format.width;
    v4l2_fmt.fmt.pix_mp.height       = stream_config.format.height;
    v4l2_fmt.fmt.pix_mp.pixelformat  = stream_config.format.fourcc;
    v4l2_fmt.fmt.pix_mp.field        = V4L2_FIELD_ANY;

    rc = v4l2_video_get_format(tparm->v4l2_media_stream.video_ent, &v4l2_fmt);
    if (rc < 0) {
        ERR("[T#%d] Error: get video format %d.\n", stream_type, rc);
        goto fatal;
    }

    /* request buffers */
    memset (&v4l2_rb, 0, sizeof (struct v4l2_requestbuffers));
    v4l2_rb.count  = NB_BUFFER;
    v4l2_rb.type   = v4l2_enum_type;
    v4l2_rb.memory = V4L2_MEMORY_MMAP;
    rc = v4l2_video_req_bufs(tparm->v4l2_media_stream.video_ent, &v4l2_rb);
    if (rc < 0) {
        printf("[T#%d] Error: request buffer.\n", stream_type);
        goto fatal;
    }

    printf("[T#%d] request buf ok\n", stream_type);

    /* map buffers */
    for (i = 0; i < NB_BUFFER; i++) {
        struct v4l2_buffer v4l2_buf;
        struct v4l2_plane buf_planes[v4l2_fmt.fmt.pix_mp.num_planes];
        memset (&v4l2_buf, 0, sizeof (struct v4l2_buffer));
        v4l2_buf.index   = i;
        v4l2_buf.type    = v4l2_enum_type;
        v4l2_buf.memory  = V4L2_MEMORY_MMAP;
        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            v4l2_buf.m.planes=buf_planes;
            v4l2_buf.length = v4l2_fmt.fmt.pix_mp.num_planes;
        }
        rc = v4l2_video_query_buf(tparm->v4l2_media_stream.video_ent, &v4l2_buf);
        if (rc < 0) {
            printf("[T#%d] Error: query buffer %d.\n", stream_type, rc);
            goto fatal;
        }

        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
            v4l2_buf_length = v4l2_buf.length;
            INFO("[T#%d] type video capture. length: %u offset: %u\n", stream_type, v4l2_buf.length, v4l2_buf.m.offset);
            v4l2_mem[i] = mmap (0, v4l2_buf.length, PROT_READ, MAP_SHARED,
                tparm->v4l2_media_stream.video_ent->fd, v4l2_buf.m.offset);
            ++total_mapped_mem;
            INFO("[T#%d] Buffer[%d] mapped at address 0x%p total_mapped_mem:%d.\n", stream_type, i, v4l2_mem[i], total_mapped_mem);
        }
        else if(v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE){
            for (j=0;j<v4l2_fmt.fmt.pix_mp.num_planes;j++) {
                v4l2_buf_length = v4l2_buf.m.planes[j].length;
                dma_fd = do_get_dma_buf_fd(tparm->v4l2_media_stream.video_ent->fd, i, j);
                INFO("[T#%d] plane:%d multiplanar length: %u offset: %u, dma_fd:%d\n",
                    stream_type, j, v4l2_buf.m.planes[j].length, v4l2_buf.m.planes[j].m.mem_offset, dma_fd);
                v4l2_mem[i*v4l2_fmt.fmt.pix_mp.num_planes + j] = mmap (0, v4l2_buf.m.planes[j].length, PROT_READ, MAP_SHARED,
                    tparm->v4l2_media_stream.video_ent->fd, v4l2_buf.m.planes[j].m.mem_offset);
                v4l2_dma_fd[i*v4l2_fmt.fmt.pix_mp.num_planes + j] = dma_fd;
                ++total_mapped_mem;
                INFO("[T#%d] Buffer[%d] mapped at address %p total_mapped_mem:%d.\n", stream_type,i*v4l2_fmt.fmt.pix_mp.num_planes + j, v4l2_mem[i*v4l2_fmt.fmt.pix_mp.num_planes + j],total_mapped_mem);
            }
        }
        if (v4l2_mem[i] == MAP_FAILED) {
            ERR("[T#%d] Error: mmap buffers.\n", stream_type);
            goto fatal;
        }
        MSG("[T#%d] map  %d ok, 0x%p\n", stream_type, i, v4l2_mem[i]);
    }

    /* queue buffers */
    DBG("[T#%d] begin to Queue buf.\n", stream_type);

    for (i = 0; i < NB_BUFFER; ++i) {
        struct v4l2_buffer v4l2_buf;
        struct v4l2_plane buf_planes[v4l2_fmt.fmt.pix_mp.num_planes];
        memset (&v4l2_buf, 0, sizeof (struct v4l2_buffer));
        v4l2_buf.index   = i;
        v4l2_buf.type    = v4l2_enum_type;
        v4l2_buf.memory  = V4L2_MEMORY_MMAP;
        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            v4l2_buf.m.planes=buf_planes;
            v4l2_buf.length = v4l2_fmt.fmt.pix_mp.num_planes;
        }
        rc = v4l2_video_q_buf( tparm->v4l2_media_stream.video_ent, &v4l2_buf );
        if (rc < 0) {
            ERR("[T#%d] Error: queue buffers, rc:%d i:%d\n",stream_type, rc, i);
            goto fatal;;
        }
    }
    DBG("[T#%d] Queue buf done.\n", stream_type);

    isp_param_init(tparm->v4l2_media_stream, tparm);

    /**************************************************
     * V4L2 stream on, get buffers
     *************************************************/
    /* stream on */
    //int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int type = v4l2_enum_type;
    rc =  media_stream_start(&tparm->v4l2_media_stream, type); // v4l2_video_stream_on(v4l2_media_stream->video_ent, type);
    if (rc < 0) {
        ERR("[T#%d] Error: streamon.\n", stream_type);
        goto fatal;
    }

    INFO("[T#%d] Video stream is on.\n", stream_type);

    sem_post(&tparm->info.p_sem);

    start = GetTimeMsec();

    /* dequeue and display */
    do {
        struct v4l2_buffer v4l2_buf;

        int idx = -1;
        if (idx == 0)
            ;
        // dqbuf from video node
        memset (&v4l2_buf, 0, sizeof (struct v4l2_buffer));
        //v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.type = v4l2_enum_type;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            //v4l2_buf.m.planes=buf_planes;
            v4l2_buf.m.planes=malloc(v4l2_fmt.fmt.pix_mp.num_planes*sizeof(struct v4l2_plane));
            v4l2_buf.length = v4l2_fmt.fmt.pix_mp.num_planes;
        }
        //INFO("beg to  dq buffer.\n");
        rc = v4l2_video_dq_buf(tparm->v4l2_media_stream.video_ent, &v4l2_buf);
        if (rc < 0) {
            ERR ("Error: dequeue buffer.\n");
            if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
                free(v4l2_buf.m.planes);
            break;
        }
        idx = v4l2_buf.index;
        //INFO("[T#%d] dq buf ok, idx %d, mem 0x%p \n",stream_type, idx, v4l2_mem[idx]);
        save_img("mif",v4l2_mem[idx], image_width * image_height*3/2, stream_type, display_count);
#if RTSP
        lib_put_frame(v4l2_mem[idx], image_width * image_height * 3 / 2);
#endif
        //INFO("[T#%d] todo: do something with buf. capture count %d \n ", stream_type, tparm->capture_count);
        usleep(1000*10);

        rc = v4l2_video_q_buf(tparm->v4l2_media_stream.video_ent,  & v4l2_buf);
        if (rc < 0) {
            ERR ("[T#%d] Error: queue buffer.\n", stream_type);
            break;
        }
        //INFO("[T#%d] q buf back idx %d \n", stream_type, v4l2_buf.index);

        display_count++;
        if ((display_count % 100 == 0)) {
            end = GetTimeMsec();
            end = end - start;
            #ifdef ANDROID
            INFO("[T#%d] stream port %s fps is : %ld\n",stream_type, "raw", (100 * 1000) /end);
            #else
            INFO("[T#%d] stream port %s fps is : %ld\n",stream_type, "raw", (100 * 1000) /end);
            #endif
            start = GetTimeMsec();
        }

        if (tparm->capture_count > 0)
            tparm->capture_count--;

    } while (tparm->capture_count > 0);


    INFO("[T#%d] media stream stop \n",stream_type);

    /* stream off */
    rc = media_stream_stop(&tparm->v4l2_media_stream, type);

    /* unmap buffers */
    for (i = 0; i < NB_BUFFER; i++) {
        munmap (v4l2_mem[i], v4l2_buf_length);
        if (v4l2_dma_fd[i] >= 0)
            close(v4l2_dma_fd[i]);
    }


    MSG("[T#%d]  close success. thread exit ...\n", stream_type);
    return 0;

fatal:

    MSG("[T#%d]  fatal terminated ...\n", stream_type);

    return NULL;
}

void * stats_thread(void *arg)
{
    /* thread parameters */
    struct thread_param         *tparm = (struct thread_param *)arg;
    pthread_t                   cur_pthread = pthread_self();
    int                         stream_type = -1;

    /* condition & loop flags */
    int                         rc = 0;
    int                         i;

    uint32_t          v4l2_enum_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    uint64_t display_count = 0;
    int64_t start, end;

    /**************************************************
     * find thread id
     *************************************************/
    for (i = 0; i < ARM_V4L2_TEST_STREAM_MAX; i++) {
        if (cur_pthread == tid[i]) {
            stream_type = i;
            break;
        }
    }

    struct pipe_info *pipe_ptr = &pipe_0;
    if (tparm->pipe_idx == 1 ) {
        pipe_ptr = &pipe_1;
    }

    if (tparm->mediadevname ) {
        pipe_ptr->media_dev_name = tparm->mediadevname;
    }

    sem_wait(&tparm->info.p_sem);

    INFO("[T#%d] Video stream is on.\n", stream_type);

    start = GetTimeMsec();

    /* dequeue and display */
    do {
        struct v4l2_buffer v4l2_buf;
        struct v4l2_buffer v4l2_buf_param;

        int idx = -1;

        // dqbuf from video node
        memset (&v4l2_buf, 0, sizeof (struct v4l2_buffer));
        v4l2_buf.type = v4l2_enum_type;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        rc = v4l2_video_dq_buf(tparm->v4l2_media_stream.video_stats, &v4l2_buf);
        if (rc < 0) {
            ERR ("Error: dequeue buffer.\n");
            usleep(10000);
            continue;
        }

        memset (&v4l2_buf_param, 0, sizeof (struct v4l2_buffer));
        v4l2_buf_param.type = v4l2_enum_type;
        v4l2_buf_param.memory = V4L2_MEMORY_MMAP;
        rc = v4l2_video_dq_buf(tparm->v4l2_media_stream.video_param, &v4l2_buf_param);
        if (rc < 0) {
            ERR ("Error: video param dequeue buffer.\n");
            rc = v4l2_video_q_buf(tparm->v4l2_media_stream.video_stats, &v4l2_buf);
            usleep(10000);
            continue;
        }

        idx = v4l2_buf.index;

        aisp_alg2user(0, tparm->v4l2_mem[idx]);
        aisp_alg2kernel(0, tparm->v4l2_mem_param);

        usleep(1000*10);

        rc = v4l2_video_q_buf(tparm->v4l2_media_stream.video_stats,  &v4l2_buf);
        if (rc < 0) {
            ERR ("[T#%d] Error: queue buffer.\n", stream_type);
            break;
        }
        rc = v4l2_video_q_buf(tparm->v4l2_media_stream.video_param,  &v4l2_buf_param);

        display_count++;
        if ((display_count % 100 == 0)) {
            end = GetTimeMsec();
            end = end - start;
            #ifdef ANDROID
            INFO("[T#%d] stream port %s fps is : %ld\n",stream_type, "stats", (100 * 1000) /end);
            #else
            INFO("[T#%d] stream port %s fps is : %ld\n",stream_type, "stats", (100 * 1000) /end);
            #endif
            start = GetTimeMsec();
        }

        if (tparm->capture_count > 0)
            tparm->capture_count--;

    } while (tparm->capture_count > 0);


    INFO("[T#%d] media stream stop \n",stream_type);

    /* stream off */
    rc = v4l2_video_stream_off(tparm->v4l2_media_stream.video_stats, v4l2_enum_type);

    /* unmap buffers */
    for (i = 0; i < NB_BUFFER; i++) {
        munmap (tparm->v4l2_mem[i], tparm->stats_buf_length);
    }

    /* stream off */
    rc = v4l2_video_stream_off(tparm->v4l2_media_stream.video_param, v4l2_enum_type);

    /* unmap buffers */
    munmap (tparm->v4l2_mem_param, tparm->param_buf_length);

    MSG("[T#%d]  close success. thread exit ...\n", stream_type);

    return 0;
}


/**********
 * raw capture functions
 */
 int prepareOutput0Capture(struct thread_param * tparam) {

    return pthread_create(&tid[ARM_V4L2_TEST_STREAM_OUTPUT0], NULL, &video_thread, tparam);
}
int prepareRawCapture(struct thread_param * tparam) {

    return pthread_create(&tid[ARM_V4L2_TEST_STREAM_RAW], NULL, &video_thread, tparam);
}

int prepareStatsCapture(struct thread_param * tparam) {

    return pthread_create(&tid[ARM_V4L2_TEST_STREAM_STATS], NULL, &stats_thread, tparam);
}

void finishOutput0Capture(struct thread_param * tparam) {
    INFO("join and wait for output subthread to exit...... \n");

    pthread_join(tid[ARM_V4L2_TEST_STREAM_OUTPUT0], NULL);
    tparam->capture_count = 0;
}


void finishRawCapture(struct thread_param * tparam) {
    INFO("join and wait for raw subthread to exit...... \n");

    pthread_join(tid[ARM_V4L2_TEST_STREAM_RAW], NULL);
    tparam->capture_count = 0;
}

void finishStatsCapture(struct thread_param * tparam) {
    INFO("join and wait for stats subthread to exit...... \n");

    pthread_join(tid[ARM_V4L2_TEST_STREAM_STATS], NULL);
    tparam->capture_count = 0;
}

void usage(char * prog){
    printf("%s\n", prog);
    printf("usage:\n");
    printf(" example   : ./v4l2_test_raw  -n 100 m /dev/media0 -p 0 \n");
    //printf("    f : fmt       : 0: rgb24  1:nv12 \n");
    printf("    m : media dev name: /dev/media0 or /dev/media1 \n");
    printf("    p : pipe selection  : 0 1 default 0\n");
    //printf("    e : exposure value	  : min 1, max 4, default is 1\n");
    //printf("    b : fbdev			 : default: /dev/fb0\n");
    printf("    v : videodev		 : default: /dev/video0\n");
    printf("    n : frame count \n");
}

/**********
 * main function
*/

void parse_fmt_res(uint8_t fmt, int res, uint32_t wdr_mode_prm, uint32_t exposure_prm, void *param)
{
    struct thread_param *t_param = NULL;

    uint32_t pixel_format = 0;
    uint32_t width        = 0;
    uint32_t height       = 0;
    uint32_t wdr_mode     = 0;
    uint32_t exposure     = 0;

    if (param == NULL) {
        ERR("Error input param\n");
        return;
    }

    t_param = param;

    switch (fmt) {
    case 0:
        pixel_format = V4L2_PIX_FMT_RGB24;
        break;
    case 1:
        pixel_format = V4L2_PIX_FMT_NV12;
        break;
    case 2:
        pixel_format = V4L2_PIX_FMT_SBGGR16;
        break;
    default:
        ERR("Invalid FR_OUT fmt %d !\n", fmt);
        break;
    }

    switch (res) {
    case 0:
        width = 3840;
        height = 2160;
        break;
    case 1:
        width = 1920;
        height = 1080;
        break;
    case 2:
        width = 1280;
        height = 720;
        break;
    case 3:
        width = 640;
        height = 480;
        break;
    case 4:
        width = 1280;
        height = 960;
        break;
    case 5:
        width = 320;
        height = 240;
        break;
    case 6:
        width = 228;
        height = 228;
        break;
    default:
        ERR("Invalid resolution %d !\n", res);
        break;
    }

    switch (wdr_mode_prm) {
    case 0:
        wdr_mode = 0;
        break;
    case 1:
        wdr_mode = 1;
        break;
    case 2:
        wdr_mode = 2;
        break;
    default:
        ERR("Invalid wdr mode %d !\n", wdr_mode_prm);
        break;
    }

    switch (exposure_prm) {
    case 1:
        exposure = 1;
        break;
    case 2:
        exposure = 2;
        break;
    case 3:
        exposure = 3;
        break;
    case 4:
        exposure = 4;
        break;
    default:
        ERR("Invalid exposure %d !\n", exposure_prm);
        break;
    }

    t_param->pixformat = pixel_format;
    t_param->width     = width;
    t_param->height    = height;
    t_param->wdr_mode  = wdr_mode;
    t_param->exposure  = exposure;

    ERR("\n parse_fmt_res: pixel fmt 0x%x, width %d, height %d, wdr_mode %d, exposure %d\n",
        pixel_format, width, height, wdr_mode, exposure);
}

int main(int argc, char *argv[])
{

    //char *fbdevname = "/dev/fb0";
    //char *v4ldevname = "/dev/video60";
    char *v4ldevname = "/dev/video3";
    char *v4l2mediadevname = "/dev/media0";

    int count = -100;
    int pipe_idx = 0;

    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }

    //int optind = 0;
    int c;

    while (optind < argc) {
        if ((c = getopt (argc, argv, "c:p:F:f:D:R:r:d:N:n:w:e:b:v:t:x:g:I:W:H:Y:Z:a:M:L:A:G:S:K:m:s:")) != -1) {
            switch (c) {
            case 'n':
                count = atoi(optarg);
                break;
            case 'b':
                //fbdevname = optarg;
                break;
            case 'v':
                v4ldevname = optarg;
                break;
            case 'm':
                v4l2mediadevname = optarg;
                break;
            case 'p':
                pipe_idx = atoi(optarg);
                break;
            case '?':
                usage(argv[0]);
                exit(1);
            }
        }else{
            MSG("Invalid argument %s\n",argv[optind]);
            usage(argv[0]);
            exit(1);
        }
    }

#if RTSP
    lib_initialization(image_width, image_height);

    if (fork() == 0)
        lib_run();
#endif
    struct thread_param tparam_raw = {
        .mediadevname = v4l2mediadevname,
        .devname    = v4ldevname,
        .fbp        = 0,

        .width      = 3840,
        .height     = 2160,
        .pixformat  = V4L2_PIX_FMT_NV21, //V4L2_PIX_FMT_Y12,//V4L2_PIX_FMT_NV12,//V4L2_PIX_FMT_SRGGB12,//

#ifdef WDR_ENABLE
        .fmt_code   = MEDIA_BUS_FMT_SBGGR10_1X10,//MEDIA_BUS_FMT_SRGGB12_1X12,//
        .wdr_mode   = WDR_MODE_2To1_FRAME,//WDR_MODE_2To1_LINE,
#else
        .fmt_code   = MEDIA_BUS_FMT_SBGGR10_1X10,//MEDIA_BUS_FMT_SBGGR12_1X12,//
        .wdr_mode   = WDR_MODE_NONE,
#endif
        .exposure   = 0,
        .capture_count = count,
        .pipe_idx = pipe_idx
    };

    sem_init(&tparam_raw.info.p_sem, 0, 0);

    /* turn on stats capture stream */
    if (prepareStatsCapture(&tparam_raw) != 0) {
        ERR("Error: Can't start raw stream, cancelling capture.\n");
    }

    /* turn on output0 capture stream */
    if (prepareOutput0Capture(&tparam_raw) != 0) {
        ERR("Error: Can't start raw stream, cancelling capture.\n");
    }

    /* turn on raw capture stream */
    //if (prepareRawCapture(&tparam_raw) != 0) {
    //    ERR("Error: Can't start raw stream, cancelling capture.\n");
    //}
    sleep(1);

    /* wait raw capture stream to be turned off */
    finishOutput0Capture(&tparam_raw);
    //finishRawCapture(&tparam_raw);
    finishStatsCapture(&tparam_raw);

    /**************************************************
     * Terminating threads and process
     *************************************************/
    MSG("terminating all threads ...\n");


    MSG("terminating v4l2 test app, thank you ...\n");

    return 0;
}
