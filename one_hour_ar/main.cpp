#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>

#include <iostream>

using namespace std;
using namespace cv;

const Scalar blue(255, 0, 0);
const Scalar green(0, 255, 0);
const Scalar red(0, 0, 255);

const int MARKER_WIDTH = 200;

void draw_quad(Mat img, vector<Point2f> pts, Scalar color)
{
	for (int i = 0; i < 3; i++) {
		line(img, pts[i], pts[i + 1], color);
	}

	line(img, pts[3], pts[0], color);
}

void clockwise(vector<Point2f>& square)
{
	auto v1 = square[1] - square[0];
	auto v2 = square[2] - square[0];

	double o = (v1.x * v2.y) - (v1.y * v2.x);

	if (o < 0.0) {
		swap(square[1], square[3]);
	}
}

int main()
{
	Mat image;

	VideoCapture cap("video.mp4");

	if (!cap.isOpened()) {
		return -1;
	}

	while (cap.grab()) {
		cap.retrieve(image);

		Mat gray_img;
		cvtColor(image, gray_img, CV_RGB2GRAY);

		Mat blur_img;
		blur(gray_img, blur_img, Size(5, 5));

		Mat thresh_img;
		threshold(blur_img, thresh_img, 128.0, 255.0, THRESH_OTSU);

		vector<vector<Point>> contours;
		findContours(thresh_img, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

		vector<vector<Point2f>> squares;
		for (size_t i = 0; i < contours.size(); i++) {
			auto contour = contours[i];
			vector<Point> approx;
			approxPolyDP(contour, approx, arcLength(Mat(contour), true)*0.02, true);
			if (approx.size() == 4 && fabs(contourArea(Mat(approx))) > 1000 && isContourConvex(Mat(approx))) {
				vector<Point2f> square;
				for (int j = 0; j < 4; j++) {
					square.emplace_back(Point2f(approx[j].x, approx[j].y));
				}
				squares.push_back(square);
			}
		}

		auto square = squares[0];
		draw_quad(image, square, green);

		clockwise(square);

		Mat marker;
		vector<Point2f> marker_square;

		marker_square.push_back(Point(0, 0));
		marker_square.push_back(Point(MARKER_WIDTH - 1, 0));
		marker_square.push_back(Point(MARKER_WIDTH - 1, MARKER_WIDTH - 1));
		marker_square.push_back(Point(0, MARKER_WIDTH - 1));

		Mat transform = getPerspectiveTransform(square, marker_square);
		warpPerspective(gray_img, marker, transform, Size(MARKER_WIDTH, MARKER_WIDTH));
		threshold(marker, marker, 125, 255, THRESH_BINARY | THRESH_OTSU);

		vector<Point> direction_point = { {50,50}, {150,50}, {150,150}, {50,150} };
		int direction = 0;
		for (int i = 0; i < 4; i++) {
			auto p = direction_point[i];
			if (countNonZero(marker(Rect(p.x - 25, p.y - 25, MARKER_WIDTH / 4, MARKER_WIDTH / 4))) > 20) {
				direction = i;
				break;
			}
		}

		for (int i = 0; i < direction; i++) {
			rotate(square.begin(), square.begin() + 1, square.end());
		}

		circle(image, square[0], 5, red);

		FileStorage fs("calibrate/out_camera_data.xml", FileStorage::READ);
		Mat intrinsics, distortion;
		fs["Camera_Matrix"] >> intrinsics;
		fs["Distortion_Coefficients"] >> distortion;

		vector<Point3f> object_pts;
		object_pts.push_back(Point3f(-1, 1, 0));
		object_pts.push_back(Point3f(1, 1, 0));
		object_pts.push_back(Point3f(1, -1, 0));
		object_pts.push_back(Point3f(-1, -1, 0));
		Mat object_pts_mat(object_pts);
		Mat rvec, tvec;

		solvePnP(object_pts_mat, square, intrinsics, distortion, rvec, tvec);

		cout << "rvec: " << rvec << endl;
		cout << "tvec: " << tvec << endl;

		vector<Point3f> line3dx = { { 0, 0, 0 }, { 1, 0, 0 } };
		vector<Point3f> line3dy = { { 0, 0, 0 }, { 0, 1, 0 } };
		vector<Point3f> line3dz = { { 0, 0, 0 }, { 0, 0, 1 } };

		vector<Point2f> line2dx, line2dy, line2dz;

		projectPoints(line3dx, rvec, tvec, intrinsics, distortion, line2dx);
		projectPoints(line3dy, rvec, tvec, intrinsics, distortion, line2dy);
		projectPoints(line3dz, rvec, tvec, intrinsics, distortion, line2dz);

		line(image, line2dx[0], line2dx[1], red);
		line(image, line2dy[0], line2dy[1], blue);
		line(image, line2dz[0], line2dz[1], green);


		imshow("image", image);
		waitKey(100);
	}
}



