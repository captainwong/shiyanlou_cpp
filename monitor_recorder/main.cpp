#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <memory>
#include <ctime>
#include <thread>
#include <chrono>
#include <limits>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

const char* g_winname = "Monitor";

char g_record_name[128] = {};

Ptr<BackgroundSubtractor> pMOG = {};
Ptr<BackgroundSubtractor> pMOG2 = {};


void process_camera(bool show_window, unsigned int method, unsigned int unnormal = 10, unsigned int fps = 24)
{
	const int channels[] = { 0,1 };
	const float h_ranges[] = { 0, 256 };
	const float s_ranges[] = { 0, 180 };
	const float* ranges[] = { h_ranges, s_ranges };
	const int h_bins = 50, s_bins = 60;
	const int hist_size[] = { h_bins, s_bins };


	int g_keyboard = 0;
	VideoCapture capture(0);
	if (!capture.isOpened()) {
		cerr << "Unable to open camera!" << endl;
		exit(-1);
	}

	bool is_back_ground = true, need_record = false;

	Mat frame, hsv, fgmask;
	MatND base, cur;

	unsigned int unnormal_frames = 0;

	CvSize size(static_cast<int>(capture.get(CAP_PROP_FRAME_WIDTH)),
				static_cast<int>(capture.get(CAP_PROP_FRAME_HEIGHT)));

	VideoWriter writer(g_record_name, CV_FOURCC('D', 'I', 'V', 'X'), fps, size, 1);

	cout.precision(numeric_limits<double>::digits10);
	while (g_keyboard != 'q' && g_keyboard != 27) {
		if (!capture.grab()) {
			cerr << "Unable to read camera!" << endl;
			exit(-1);
		}

		capture >> frame;

		if (method == 0) {
			pMOG2->apply(frame, fgmask);
		} else if (method == 1) {
			pMOG->apply(frame, fgmask);
		} else {
			fgmask = frame;
		}

		if (is_back_ground) {
			is_back_ground = false;
			cvtColor(frame, hsv, CV_BGR2HSV);
			calcHist(&hsv, 1, channels, Mat(), base, 2, hist_size, ranges, true, false);
			normalize(cur, cur, 0, 1, NORM_MINMAX, -1, Mat());
			//writer.set(CAP_PROP_FRAME_COUNT, capture.get(CAP_PROP_FRAME_COUNT));
		}

		cvtColor(frame, hsv, CV_BGR2HSV);
		calcHist(&hsv, 1, channels, Mat(), cur, 2, hist_size, ranges, true, false);
		normalize(cur, cur, 0, 1, NORM_MINMAX, -1, Mat());

		double comp = compareHist(base, cur, 0);
		if (comp < 0.65) {
			unnormal_frames++;
		} else if (unnormal_frames > 0) {
			unnormal_frames--;
		}

		if (unnormal_frames > unnormal) {
			need_record = true;
		} else if (unnormal_frames <= 0) {
			unnormal_frames = 0;
			need_record = false;
		}

		cout << "comp: " << comp << " unnormal_frames: " << unnormal_frames << " need_record:" << boolalpha << need_record << endl;

		if (need_record) {
			writer.write(frame);
		}

		if (show_window && !frame.empty()) {
			imshow(g_winname, frame);
		}

		g_keyboard = waitKey(30);
	}
}

void help()
{
	cout
		<< "----------------------------------------------------------------------------\n"
		<< "Usage:                                                                      \n"
		<< " ./MonitorRecorder.exe [VIS] [MODE] [FPS] [THRESHOLD] [OUTPUTFILE]          \n"
		<< "   [VIS]  : use -vis to show the monitor window, or it will run background. \n"
		<< "   [MODE] : -src   shows the original frame;                                \n"
		<< "            -mog1       shows the MOG frame;                                \n"
		<< "            -mog2      shows the MOG2 frame.                                \n"
		<< "   [FPS]  : set the fps of record file, default is 24.                      \n"
		<< "   [THRESHOLD]                                                              \n"
		<< "          : set the number x that the monitor will start recording after    \n"
		<< "            x unnormal frames passed.                                       \n"
		<< "   [OUTPUTFILE]                                                             \n"
		<< "          : assign the output recording file. It must be .avi format.       \n"
		<< "                                                   designed by Forec        \n"
		<< "----------------------------------------------------------------------------\n";
}

int main(int argc, char** argv)
{
	bool show_window = true;
	unsigned int method = 1, unnormal = 10, fps = 24;

	if (argc > 6) {
		cerr << "Inivalid parameters, exit..." << endl;
		exit(-1);
	} 
	
	if (argc >= 2) {
		if (strcmp(argv[1], "-vis") == 0) {
			show_window = true;
		} else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			help();
			return 0;
		}
	} 
	
	if (argc >= 3) {
		if (strcmp(argv[2], "-mog2") == 0) {
			method = 0;
		} else if (strcmp(argv[2], "-mog1") == 0) {
			method = 1;
		} else if (strcmp(argv[2], "-src") == 0) {
			method = 2;
		}
	}

	if (argc >= 4) {
		int param = stoi(argv[3]);
		if (param <= 10) {
			fps = 24;
		} else {
			fps = param;
		}
	}

	if (argc >= 5) {
		int param = stoi(argv[4]);
		if (param <= 0) {
			unnormal = 10;
		} else {
			unnormal = param;
		}
	}

	if (argc >= 6) {
		strcpy(g_record_name, argv[5]);
	} else {
		sprintf(g_record_name, "%ld.avi", static_cast<long>(time(nullptr)));
	}

	cout << "start after 2s..." << endl;
	this_thread::sleep_for(chrono::seconds(2));

	if (show_window) {
		namedWindow(g_winname);
	}

	pMOG = createBackgroundSubtractorKNN();
	pMOG2 = createBackgroundSubtractorMOG2();

	process_camera(show_window, method, unnormal, fps);

}


