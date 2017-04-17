#include <iostream>
#include <opencv2/opencv.hpp>

// is selecting object
bool g_selecting_object = false;
// 0 for no object to track, 1 for has, -1 for not calculated
int g_track_object = 0;
// region selected using mouse
cv::Rect g_selection;
// video frame buffer
cv::Mat g_image;

void on_mouse(int mevent, int x, int y, int /*flag*/, void* /*param*/)
{
	static cv::Point origin;

	if (g_selecting_object) {
		g_selection.x = MIN(x, origin.x);
		g_selection.y = MIN(y, origin.y);
		g_selection.width = abs(x - origin.x);
		g_selection.height = abs(y - origin.y);

		g_selection &= cv::Rect(0, 0, g_image.cols, g_image.rows);
	}

	switch (mevent) {
	case CV_EVENT_LBUTTONDOWN:
		origin = cv::Point(x, y);
		g_selection = cv::Rect(x, y, 0, 0);
		g_selecting_object = true;
		break;

	case CV_EVENT_LBUTTONUP:
		g_selecting_object = false;
		if (g_selection.width > 0 && g_selection.height > 0) {
			g_track_object = -1;
		}
		break;

	default:
		break;
	}
}


int main()
{
#ifdef _WIN32
	cv::VideoCapture video(R"(C:\dev_shiyanlou\shiyanlou_cpp\camshift\out.ogv)");
#else
	cv::VideoCapture video(R"(out.ogv)");
#endif

	auto winname = "camshift";

	cv::namedWindow(winname);
	cv::setMouseCallback(winname, on_mouse);

	cv::Mat frame;
	cv::Mat hsv, hue, mask, hist, backproj;
	cv::Rect track_window;

	int hsize = 16;
	float hranges[] = { 0, 180 };
	const float* phranges = hranges;


	while (true) {
		video >> frame;
		if (frame.empty()) {
			break;
		}

		frame.copyTo(g_image);

		cv::cvtColor(g_image, hsv, cv::COLOR_BGR2HSV);

		if (g_track_object) {
			// only deal it when the pixel's H:0~180, S:30~256, V:10~256
			cv::inRange(hsv, cv::Scalar(0, 30, 10), cv::Scalar(180, 256, 256), mask);

			// distract channel H
			int ch[] = { 0,0 };
			hue.create(hsv.size(), hsv.depth());
			cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);

			if (g_track_object < 0) {
				// create rois of channel H and mask
				cv::Mat roi(hue, g_selection), maskroi(mask, g_selection);

				// calc hist of roi
				cv::calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
				
				// nomalize hist
				cv::normalize(hist, hist, 0, 255, CV_MINMAX);

				track_window = g_selection;
				g_track_object = 1;
			}

			cv::calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
			backproj &= mask;
			cv::RotatedRect track_box = cv::CamShift(backproj, track_window, cv::TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));

			if (track_window.area() <= 1) {
				int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
				track_window = cv::Rect(track_window.x - r, track_window.y - r,
										track_window.x + r, track_window.y + r)
					& cv::Rect(0, 0, cols, rows);


			}

			cv::ellipse(g_image, track_box, cv::Scalar(0, 0, 255), 3, CV_AA);
		}

		if (g_selecting_object && g_selection.width > 0 && g_selection.height > 0) {
			cv::Mat roi(g_image, g_selection); // region of interest
			cv::bitwise_not(roi, roi);
		}

		cv::imshow(winname, g_image);

		int key = cv::waitKey(1000 / 15);
		if (key == 27) {
			break;
		}
	}

	cv::destroyAllWindows();
	video.release();

	cv::waitKey();
}
