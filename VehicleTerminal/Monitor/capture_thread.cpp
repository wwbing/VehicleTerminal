/******************************************************************
Copyright © Deng Zhimao Co., Ltd. 2021-2030. All rights reserved.
* @projectName   video_server
* @brief         capture_thread.cpp
* @author        Deng Zhimao
* @email         dengzhimao@alientek.com
* @link          www.openedv.com
* @date          2021-11-19
*******************************************************************/
#include "capture_thread.h"

void CaptureThread::run()
{
    /* 下面的代码请参考正点原子C应用编程V4L2章节，摄像头编程，这里不作解释 */
#ifdef linux
#ifndef __arm__
    return;
#endif
    int video_fd = -1;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req_bufs;
    static struct v4l2_buffer buf;
    int n_buf;
    struct buffer_info bufs_info[VIDEO_BUFFER_COUNT];
    enum v4l2_buf_type type;

    //打开设备文件
    video_fd = open(VIDEO_DEV, O_RDWR);
    if (0 > video_fd) {
        printf("ERROR: failed to open video device %s\n", VIDEO_DEV);
        return;
    }

    //设置视频格式
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 1024;
    fmt.fmt.pix.height      = 600;
    fmt.fmt.pix.colorspace  = V4L2_COLORSPACE_SRGB;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;


    if (0 > ioctl(video_fd, VIDIOC_S_FMT, &fmt)) {
        printf("ERROR: failed to VIDIOC_S_FMT\n");
        close(video_fd);
        return;
    }

    // 请求内核缓冲区
    req_bufs.count  = VIDEO_BUFFER_COUNT;
    req_bufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_bufs.memory = V4L2_MEMORY_MMAP;

    if (0 > ioctl(video_fd, VIDIOC_REQBUFS, &req_bufs)) {
        printf("ERROR: failed to VIDIOC_REQBUFS\n");
        return;
    }

    // 设置缓冲区类型和内存类型
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    // 查询缓冲区
    for (n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {
        buf.index = n_buf;
        if (0 > ioctl(video_fd, VIDIOC_QUERYBUF, &buf)) {
            printf("ERROR: failed to VIDIOC_QUERYBUF\n");
            return;
        }

        bufs_info[n_buf].length = buf.length;
        // 映射缓冲区到用户空间
        bufs_info[n_buf].start  = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, video_fd, buf.m.offset);
        if (MAP_FAILED == bufs_info[n_buf].start) {
            printf("ERROR: failed to mmap video buffer, size 0x%x\n", buf.length);
            return;
        }
    }

    // 将缓冲区放入队列
    for (n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {
        buf.index = n_buf;
        if (0 > ioctl(video_fd, VIDIOC_QBUF, &buf)) {
            printf("ERROR: failed to VIDIOC_QBUF\n");
            return;
        }
    }

    // 开始流
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 > ioctl(video_fd, VIDIOC_STREAMON, &type)) {
        printf("ERROR: failed to VIDIOC_STREAMON\n");
        return;
    }

    // 循环读取缓冲区
    while (startFlag) {
        for (n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {
            buf.index = n_buf;

            if (0 > ioctl(video_fd, VIDIOC_DQBUF, &buf)) {
                printf("ERROR: failed to VIDIOC_DQBUF\n");
                return;
            }

            // 将缓冲区转换为QImage
            QImage qImage((unsigned char*)bufs_info[n_buf].start, fmt.fmt.pix.width, fmt.fmt.pix.height,
                          QImage::Format_RGB16);

            /* 是否开启本地显示*/
            if (startLocalDisplay) emit imageReady(qImage);

            // 将缓冲区放回队列
            if (0 > ioctl(video_fd, VIDIOC_QBUF, &buf)) {
                printf("ERROR: failed to VIDIOC_QBUF\n");
                return;
            }
        }
    }

    // 休眠
    msleep(800);  // at lease 650

    // 释放缓冲区
    for (int i = 0; i < VIDEO_BUFFER_COUNT; i++) {
        munmap(bufs_info[i].start, buf.length);
    }

    close(video_fd);
#endif
}
