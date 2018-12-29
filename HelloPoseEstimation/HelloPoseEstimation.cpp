#include "pch.h"

#include <iostream>

#include "opencv2/opencv.hpp"
//#include "opencv/videoInput.h"

#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/public/session.h"
#include "Eigen/Dense"
#include "tensorflow/core/public/session.h"
#include "tensorflow/cc/ops/standard_ops.h"

#include "PoseEstimator.h"
#include "HelloPoseEstimation.h"

using namespace std;
using namespace cv;
using namespace tensorflow;

int main(int argc,  char* const argv[]) {
	if (argc < 2) {
		cout << "Graph path not specified" << "\n";
		return 1;
	}

	// videoInput vidInput = videoInput::getInstance();
	VideoCapture cap;
	boolean haveFile = argc > 2;
	if (argc > 2) {
		if (!cap.open(argv[2])) {
			cout << "File not found: " << argv[2] << "\n";
			return -1;
		}
	} else {
		if (!cap.open(0)) {
			cout << "Webcam not found: " << argv[2] << "\n";
			return 2;
		}
		cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
		cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
	}

	// TODO crop input mat so that resized image size is multiple of 16
	 const int image_size_factor = 2;
	const int heat_map_upsample_size = 4;
	// TODO explicit size and resize mat to it in order to have fixed cpu/gpu load and fixed quality
	PoseEstimator pose_estimator(argv[1], cap.get(cv::CAP_PROP_FRAME_WIDTH) / image_size_factor, cap.get(cv::CAP_PROP_FRAME_HEIGHT) / image_size_factor);
	error::Code loadGraphResult = pose_estimator.loadModel();
	if (loadGraphResult != error::Code::OK) return loadGraphResult;

	try {
		for (;;) {
			Mat image;
			cap >> image;
			if (image.empty()) break; // end of video stream

			vector<Human> humans;
			pose_estimator.inference(image, heat_map_upsample_size, humans);

			pose_estimator.draw_humans(image, humans);

			imshow(haveFile ? argv[2] : "this is you, smile! :)", image);
			if (waitKey(10) == 27) break; // stop capturing by pressing ESC 
		}
	}
	catch (std::exception& e) {
		cout << e.what();
	}

	cap.release();

	return 0;
}