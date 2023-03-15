#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/photo/cuda.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/cudacodec.hpp>
#include <sys/stat.h>
#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvDeviceU3V.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvBuffer.h>
#include <PvBufferWriter.h>
#include <list>
#include <PvDisplayWnd.h>
#include <map>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <thread>
#include <sl/Camera.hpp>
#include <fstream>
#include <mntent.h>
#include <dirent.h>

using namespace cv;
using namespace cuda;
using namespace sl;
using namespace std;

typedef list<PvBuffer *> BufferList;

typedef struct {
    cv::Mat frame;
    int BlockID;
} EnumeratedFrame;

typedef struct {
    PvStream *aStream;
    int stream_index;
    queue<EnumeratedFrame *> Frames;
} StreamInfo;

struct VideoConfig {
    short FPS = 15, exposure_rgb = 500, exposure_800 = 1000, exposure_975 = 3000;
    short file_index = -1;
    int64_t width = 1536, height = 2048;
    bool output_fsi = false, output_rgb = false, output_800 = false, output_975 = false, output_svo = false;
    bool view = false;
    string output_dir = string("/home/mic-730ai/Desktop/JAI_Results");
};

struct AcquisitionParameters {
    StreamInfo *MyStreamInfos[3];
    PvDevice *lDevice = nullptr;
    BufferList lBufferLists[3];
    thread jai_t0, jai_t1, jai_t2, zed_t, merge_t;
    Camera zed;
    VideoConfig *video_conf;
    pthread_cond_t GrabEvent = PTHREAD_COND_INITIALIZER;
    pthread_cond_t MergeFramesEvent[3];
    pthread_mutex_t grab_mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t acq_start_mtx = PTHREAD_MUTEX_INITIALIZER;
    VideoWriter mp4_FSI, mp4_BGR, mp4_800, mp4_975;
    bool is_connected, is_running;
};

bool SelectDeviceLocally(PvString *aConnectionID);

PvDevice *ConnectToDevice(const PvString &aConnectionID);

PvStream *OpenStream(const PvString &aConnectionID);

void ConfigureStream(PvDevice *&aDevice, PvStream *aStream, int channel);

void CreateStreamBuffers(PvDevice *&aDevice, PvStream *aStream, BufferList *aBufferList);

void FreeStreamBuffers(BufferList *aBufferList);

VideoConfig * parse_args(int fps, int exposure_rgb, int exposure_800, int exposure_975, string output_dir,
                         bool output_fsi, bool output_rgb, bool output_800, bool output_975, bool output_svo,
                         bool view);

bool setup_JAI(AcquisitionParameters &acq);

void MP4CreateFirstTime(AcquisitionParameters &acq);

string gs_sink_builder(int file_index, const string& output_type_name, const string& output_dir);

bool exists(char path[100]);

bool connect_ZED(AcquisitionParameters &acq);

void GrabThread(int stream_index, AcquisitionParameters &acq);

void ZedThread(AcquisitionParameters &acq);

void MergeThread(AcquisitionParameters &acq);

bool connect_cameras(AcquisitionParameters &acq);

bool start_acquisition(AcquisitionParameters &acq);

void stop_acquisition(AcquisitionParameters &acq);

void disconnect_cameras(AcquisitionParameters &acq);

#ifndef JAI_ACQUISITION_MAIN_H
#define JAI_ACQUISITION_MAIN_H

#endif //JAI_ACQUISITION_MAIN_H
