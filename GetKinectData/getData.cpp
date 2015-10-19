/*
 * getData.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: hudson
 */




#include "libfreenect.h"
#include "libfreenect_sync.h"

#include <cv.hpp>
#include "opencv2/imgcodecs.hpp"
#include <math.h>
#include <stdio.h>

using namespace cv;

const int CAMERA_W = 640;
const int CAMERA_H = 480;

uint16_t* depth2D;

Mat image2D;
Mat image1D;
Mat lut(1, 256, CV_8U);

uint16_t depth[CAMERA_W];
float map[1024][1024];

class DepthCamera
{
public:
	//DepthCamera();

	int init(void (*depth_cb_F)(freenect_device*, void*, uint32_t))
	{
		// Initialize libfreenect.
		//freenect_context* fn_ctx;
		int ret = freenect_init(&fn_ctx, NULL);
		if (ret < 0)
			return ret;

		// Show debug messages and use camera only.
		freenect_set_log_level(fn_ctx, FREENECT_LOG_DEBUG);
		freenect_select_subdevices(fn_ctx, FREENECT_DEVICE_CAMERA);

		// Find out how many devices are connected.
		int num_devices = ret = freenect_num_devices(fn_ctx);
		if (ret < 0)
			return ret;
		if (num_devices == 0)
		{
			printf("No device found!\n");
			freenect_shutdown(fn_ctx);
			return 1;
		}

		// Open the first device.
		//freenect_device* fn_dev;
		ret = freenect_open_device(fn_ctx, &fn_dev, 0);
		if (ret < 0)
		{
			freenect_shutdown(fn_ctx);
			return ret;
		}

		// Set depth and video modes.
		ret = freenect_set_depth_mode(fn_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_MM));
		if (ret < 0)
		{
			freenect_shutdown(fn_ctx);
			return ret;
		}
	//	ret = freenect_set_video_mode(fn_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
	//	if (ret < 0)
	//	{
	//		freenect_shutdown(fn_ctx);
	//		return ret;
	//	}

		// Set frame callbacks.
		freenect_set_depth_callback(fn_dev, depth_cb_F);// apparently, i can't feed it a function that is part of a class... it has to be global?
	//	freenect_set_video_callback(fn_dev, video_cb);

//		void* (DepthCamera::)(_freenect_device*, void*, unsigned int)
//		void (*)(_freenect_device*, void*, unsigned int)
	}
	int startStream()
	{
		// Start depth and video.
		int ret = freenect_start_depth(fn_dev);
		if (ret < 0)
		{
			freenect_shutdown(fn_ctx);
			return ret;
		}
	//	ret = freenect_start_video(fn_dev);
	//	if (ret < 0)
	//	{
	//		freenect_shutdown(fn_ctx);
	//		return ret;
	//	}

		running = true;
		// Run until interruption or failure.
		while (running && freenect_process_events(fn_ctx) >= 0)
		{

		}
		stopStream();
		return 0;
	}
	int stopStream()
	{
		running = false;

		printf("Shutting down\n");

		// Stop everything and shutdown.
		freenect_stop_depth(fn_dev);
	//	freenect_stop_video(fn_dev);
		freenect_close_device(fn_dev);
		freenect_shutdown(fn_ctx);

		printf("Done!\n");

		return 0;
	}

	uint16_t* getDepth()
	{
		return depth;
	}

	void depth_cb(freenect_device *dev, void* data, uint32_t timestamp) // origonal callback function, not working because it's part of a class?
	{
		//printf("Received depth frame at %d\n", timestamp);

		depth = (uint16_t*)data;
		//int offset = 640 * (480 / 2);
		int offset = 0;
		int i;

		//char value[10] = {' ', '`', '.', '-', '~', '+', '*', '&', '@', '#'};
		char value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

		printf("Cross Section of Middle:\n");
		for (i=0; i<640/**480*/; i=i+8)	// Single line pure data
		{
	//		if(i == 0)
	//		{
	//			printf("[%d", depth[i+offset]);
	//		} else if(i == 639)
	//		{
	//			printf(", %d]\n", depth[i+offset]);
	//		} else
	//		{
	//			printf(", %d", depth[i+offset]);
	//		}
			printf("%c", value[(int)((float)depth[i+(640*(480/2))] / (float)(4096/9))]);
		}

		int x;
		int y;

		printf("\nDepth 2D Image:\n");

		for (y=0; y<480; y=y+24)
		{
			printf("|");
			for (x=0; x<640; x=x+8)
			{
				printf("%c", value[(int)((float)depth[y*480+x] / (float)(4096/9))]);
				//printf("%d, ", depth[y*480+x]);
				//printf("%c", value[1]);
			}
			printf("|\n");
		}
		cbFunc(depth);
	}
	void set_cb(DepthCamera* cam, void(*func)(uint16_t*))
	{
		cbFunc = (*func);
	}
	void updateDepth(uint16_t* ndepth)
	{
		depth = ndepth;
	}
	void gotDepth_cb();
private:
	uint16_t* depth;

	void (*cbFunc)(uint16_t*);

	freenect_context* fn_ctx;
	freenect_device* fn_dev;

	bool running;
};

/*int main()
{
// Handle signals gracefully.
//	signal(SIGINT, signalHandler);
//	signal(SIGTERM, signalHandler);
//	signal(SIGQUIT, signalHandler);

	// Initialize libfreenect.
	freenect_context* fn_ctx;
	int ret = freenect_init(&fn_ctx, NULL);
	if (ret < 0)
		return ret;

	// Show debug messages and use camera only.
	freenect_set_log_level(fn_ctx, FREENECT_LOG_DEBUG);
	freenect_select_subdevices(fn_ctx, FREENECT_DEVICE_CAMERA);

	// Find out how many devices are connected.
	int num_devices = ret = freenect_num_devices(fn_ctx);
	if (ret < 0)
		return ret;
	if (num_devices == 0)
	{
		printf("No device found!\n");
		freenect_shutdown(fn_ctx);
		return 1;
	}

	// Open the first device.
	freenect_device* fn_dev;
	ret = freenect_open_device(fn_ctx, &fn_dev, 0);
	if (ret < 0)
	{
		freenect_shutdown(fn_ctx);
		return ret;
	}

	// Set depth and video modes.
	ret = freenect_set_depth_mode(fn_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_MM));
	if (ret < 0)
	{
		freenect_shutdown(fn_ctx);
		return ret;
	}
//	ret = freenect_set_video_mode(fn_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
//	if (ret < 0)
//	{
//		freenect_shutdown(fn_ctx);
//		return ret;
//	}

	// Set frame callbacks.
	freenect_set_depth_callback(fn_dev, depth_cb);
//	freenect_set_video_callback(fn_dev, video_cb);

	// Start depth and video.
	ret = freenect_start_depth(fn_dev);
	if (ret < 0)
	{
		freenect_shutdown(fn_ctx);
		return ret;
	}
//	ret = freenect_start_video(fn_dev);
//	if (ret < 0)
//	{
//		freenect_shutdown(fn_ctx);
//		return ret;
//	}

	// Run until interruption or failure.
	while (running && freenect_process_events(fn_ctx) >= 0)
	{

	}

	printf("Shutting down\n");

	// Stop everything and shutdown.
	freenect_stop_depth(fn_dev);
//	freenect_stop_video(fn_dev);
	freenect_close_device(fn_dev);
	freenect_shutdown(fn_ctx);

	printf("Done!\n");

	return 0;
}*/

void depth_cb_G(freenect_device* dev, void* data, uint32_t timestamp) // old, but working code
{
	//printf("Received depth frame at %d\n", timestamp);

	depth2D = (uint16_t*)data;
	//image.data = (uchar*)depth2D;
	image2D = Mat(CAMERA_H, CAMERA_W, CV_16UC1, depth2D);	// create a 16bit Mat
	image2D = image2D.mul(Scalar(32));	// convert from 11bit to 16bit
	image2D = image2D.mul(Scalar(1.0/256.0));	// convert from 16bit to 8bit
	image2D.convertTo(image2D, CV_8UC1);	// convert Mat type to 8bit
	//subtract(Scalar::all(255), image, image);
	LUT(image2D, lut, image2D);
	image1D = image2D(Range(CAMERA_H/2, CAMERA_H/2+1), Range(0, CAMERA_W));
	imshow("KinectDepth", image2D);
	imshow("KinectDepthSlice", image1D);

	if(waitKey(1) == 1048608)	// Press the space bar to save.
	{
		char string[50];
		sprintf(string, "image/pics/ScreenShot_%d.png", timestamp);
		imwrite(string, image2D);
		printf("\nSaved image to: %s\n", string);
	}

}

int main()
{
	float power = 5;

	// Create LookUpTable for drawing the frames.
	uchar* p = lut.data;
	for(int i = 0; i < 256; i++)
	{
		if(i > 0)
		{
			p[i] = 255 - i;
			//p[i] = 255 - ((log(i) * 255) / (log(255)));
			//p[i] = 255 - (pow((float)i, power) * 255) / (pow(255, power));
		} else
		{
			p[i] = 0;
		}
	}

	namedWindow("KinectDepth", WINDOW_NORMAL);
	namedWindow("KinectDepthSlice", WINDOW_NORMAL);
	//printf("tilt return: %d", freenect_sync_set_tilt_degs(0, 0));
	DepthCamera kinect;
	kinect.init(depth_cb_G);
	kinect.startStream();
	return 0;
}
