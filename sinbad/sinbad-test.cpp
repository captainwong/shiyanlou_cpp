#include <iostream>

#include <OGRE/Ogre.h>
#include <OIS/OIS.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>


// Ogre general variables
Ogre::Root* root;
OIS::InputManager* im;
OIS::Keyboard* keyboard;

// Ogre background variables
Ogre::PixelBox mPixelBox;
Ogre::TexturePtr mTexture;

// Ogre scene variables
Ogre::SceneNode* ogreNode;
Ogre::AnimationState *baseAnim, *topAnim;


cv::VideoCapture TheVideoCapturer;
cv::Mat image;
cv::Mat rvec, tvec;


using namespace cv;
using namespace std;

const int marker_width = 200;

Scalar blue(255, 0, 0);
Scalar green(0, 255, 0);
Scalar red(0, 0, 255);

void drawQuad(Mat image, vector<Point2f> points, Scalar color) {
	line(image, points[0], points[1], color);
	line(image, points[1], points[2], color);
	line(image, points[2], points[3], color);
	line(image, points[3], points[0], color);
}

int ogreGetPoseParameters(cv::Mat rvec, cv::Mat tvec, double position[3], double orientation[4]);
int initOgreAR(unsigned char* buffer, cv::Mat intrinsics, cv::Mat distortion, int width, int height);

void clockwise(vector<Point2f>& square) {
	Point2f v1 = square[1] - square[0];
	Point2f v2 = square[2] - square[0];

	double o = (v1.x * v2.y) - (v1.y * v2.x);

	if (o < 0.0) {
		std::swap(square[1], square[3]);
	}
}

void get_rvec_tvec(Mat image, Mat intrinsics, Mat distortion, Mat& rvec, Mat& tvec)
{
	Mat grayImage;
	cvtColor(image, grayImage, CV_RGB2GRAY);
	Mat blurredImage;
	blur(grayImage, blurredImage, Size(5, 5));
	Mat threshImage;
	threshold(blurredImage, threshImage, 128.0, 255.0, THRESH_OTSU);

	vector<vector<Point> > contours;
	findContours(threshImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	vector<vector<Point2f> > squares;
	for (int i = 0; i < contours.size(); i++) {
		vector<Point> contour = contours[i];
		vector<Point> approx;
		approxPolyDP(contour, approx, arcLength(Mat(contour), true)*0.02, true);
		if (approx.size() == 4 &&
			fabs(contourArea(Mat(approx))) > 1000 &&
			isContourConvex(Mat(approx))) {
			vector<Point2f> square;

			for (int i = 0; i < 4; ++i) {
				square.push_back(Point2f(approx[i].x, approx[i].y));
			}
			squares.push_back(square);
		}
	}
	vector<Point2f> square = squares[0];

	clockwise(square);

	Mat marker;
	vector<Point2f> marker_square;
	drawQuad(image, square, green);

	marker_square.push_back(Point(0, 0));
	marker_square.push_back(Point(marker_width - 1, 0));
	marker_square.push_back(Point(marker_width - 1, marker_width - 1));
	marker_square.push_back(Point(0, marker_width - 1));


	Mat transform = getPerspectiveTransform(square, marker_square);
	warpPerspective(grayImage, marker, transform, Size(marker_width, marker_width));
	threshold(marker, marker, 125, 255, THRESH_BINARY | THRESH_OTSU);

	vector<Point> direction_point = { {50, 50} ,{150, 50},{150, 150},{50,150} };
	int direction;
	for (int i = 0; i < 4; ++i) {
		Point p = direction_point[i];
		if (countNonZero(marker(Rect(p.x - 25, p.y - 25, marker_width / 4, marker_width / 4))) > 20) {
			direction = i;
			break;
		}
	}
	for (int i = 0; i < direction; ++i) {
		rotate(square.begin(), square.begin() + 1, square.end());
	}

	vector<Point3f> objectPoints;
	objectPoints.push_back(Point3f(-1, -1, 0));
	objectPoints.push_back(Point3f(-1, 1, 0));
	objectPoints.push_back(Point3f(1, 1, 0));
	objectPoints.push_back(Point3f(1, -1, 0));

	circle(image, square[0], 5, red);
	Mat objectPointsMat(objectPoints);


	solvePnP(objectPointsMat, square, intrinsics, distortion, rvec, tvec);
}



int main(int argc, char** argv)
{
	//calibrate/out_camera_data.xml intrin.xml
	cv::FileStorage fs("calibrate/out_camera_data.xml", cv::FileStorage::READ);
	cv::Mat intrinsics, distortion;
	int video_width, video_height;
	fs["Camera_Matrix"] >> intrinsics;
	fs["Distortion_Coefficients"] >> distortion;
	fs["image_Width"] >> video_width;
	fs["image_Height"] >> video_height;

	TheVideoCapturer.open(argv[1]);


	// CAPTURE FIRST FRAME
	TheVideoCapturer.grab();
	TheVideoCapturer.retrieve(image);


	// INIT OGRE
	initOgreAR(image.ptr<uchar>(0), intrinsics, distortion, video_width, video_height);


	while (TheVideoCapturer.grab()) {
		TheVideoCapturer.retrieve(image);

		double position[3], orientation[4];
		get_rvec_tvec(image, intrinsics, distortion, rvec, tvec);

		//TheMarkers[0].OgreGetPoseParameters(position, orientation);

		ogreGetPoseParameters(rvec, tvec, position, orientation);

		// UPDATE BACKGROUND IMAGE
		mTexture->getBuffer()->blitFromMemory(mPixelBox);

		ogreNode->setPosition(position[0], position[1], position[2] / 2);
		ogreNode->setOrientation(orientation[0], orientation[1], orientation[2], orientation[3]);
		ogreNode->setVisible(true);
		cout << "position:" << position[0] << ", " << position[1] << ", " << position[2] << endl;
		cout << "orientation:" << orientation[0] << ", " << orientation[1] << ", " << orientation[2] << "," << orientation[3] << endl;
		// Update animation
		double deltaTime = 1.2*root->getTimer()->getMilliseconds() / 1000.;

		baseAnim->addTime(deltaTime);
		topAnim->addTime(deltaTime);

		root->getTimer()->reset();

		// RENDER FRAME
		if (root->renderOneFrame() == false) break;
		Ogre::WindowEventUtilities::messagePump();

		// KEYBOARD INPUT
		keyboard->capture();
		if (keyboard->isKeyDown(OIS::KC_ESCAPE)) break;

		cv::waitKey(10);
	}

	im->destroyInputObject(keyboard);
	im->destroyInputSystem(im);
	im = 0;

	delete root;
	return 0;
}

/**
*
*/
int initOgreAR(unsigned char* buffer, cv::Mat intrinsics, cv::Mat distortion, int width, int height)
{
	// INIT OGRE FUNCTIONS
	root = new Ogre::Root("plugins.cfg", "ogre.cfg");
	if (!root->showConfigDialog()) return -1;
	Ogre::SceneManager* smgr = root->createSceneManager(Ogre::ST_GENERIC);

	// CREATE WINDOW, CAMERA AND VIEWPORT
	Ogre::RenderWindow* window = root->initialise(true);
	Ogre::Camera *camera;
	Ogre::SceneNode* cameraNode;
	camera = smgr->createCamera("camera");
	camera->setNearClipDistance(0.01f);
	camera->setFarClipDistance(10.0f);
	camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
	camera->setPosition(0, 0, 0);
	camera->lookAt(0, 0, 0);

	Ogre::Matrix4 PM2(-intrinsics.at<double>(0, 0) / width * 2, 0, (intrinsics.at<double>(0, 2) - (width / 2)) / (width / 2) - 0.1, 0,
					  0, intrinsics.at<double>(1, 1) / height * 2, (intrinsics.at<double>(1, 2) - (height / 2)) / (height / 2) - 0.2, 0,
					  0, 0, 1.01, -0.1,
					  0, 0, 1, 0);

	cout << -intrinsics.at<double>(0, 0) / width * 2 << ", " << (intrinsics.at<double>(0, 2) - (width / 2)) / (width / 2) << ", " << intrinsics.at<double>(1, 1) / height * 2 << ", " << (intrinsics.at<double>(1, 2) - (height / 2)) / (height / 2) << endl;
	camera->setCustomProjectionMatrix(true, PM2);
	camera->setCustomViewMatrix(true, Ogre::Matrix4::IDENTITY);
	window->addViewport(camera);
	cameraNode = smgr->getRootSceneNode()->createChildSceneNode("cameraNode");
	cameraNode->attachObject(camera);

	// create background camera image
	mPixelBox = Ogre::PixelBox(width, height, 1, Ogre::PF_R8G8B8, buffer);
	// Create Texture
	mTexture = Ogre::TextureManager::getSingleton().createManual("CameraTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																 Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

	//Create Camera Material
	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create("CameraMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique *technique = material->createTechnique();
	technique->createPass();
	material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
	material->getTechnique(0)->getPass(0)->createTextureUnitState("CameraTexture");

	Ogre::Rectangle2D* rect = new Ogre::Rectangle2D(true);
	rect->setCorners(-1.0, 1.0, 1.0, -1.0);
	rect->setMaterial("CameraMaterial");

	// Render the background before everything else
	rect->setRenderQueueGroup(Ogre::RENDER_QUEUE_BACKGROUND);

	// Hacky, but we need to set the bounding box to something big, use infinite AAB to always stay visible
	Ogre::AxisAlignedBox aabInf;
	aabInf.setInfinite();
	rect->setBoundingBox(aabInf);

	// Attach background to the scene
	Ogre::SceneNode* node = smgr->getRootSceneNode()->createChildSceneNode("Background");
	node->attachObject(rect);

	// CREATE SIMPLE OGRE SCENE
	// add sinbad.mesh
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Sinbad.zip", "Zip", "Popular");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	Ogre::String entityName = "Marker";
	Ogre::Entity* ogreEntity = smgr->createEntity(entityName, "Sinbad.mesh");
	Ogre::Real offset = ogreEntity->getBoundingBox().getHalfSize().y;
	ogreNode = smgr->getRootSceneNode()->createChildSceneNode();
	// add entity to a child node to correct position (this way, entity axis is on feet of sinbad)
	Ogre::SceneNode *ogreNodeChild = ogreNode->createChildSceneNode();
	ogreNodeChild->attachObject(ogreEntity);
	// Sinbad is placed along Y axis, we need to rotate to put it along Z axis so it stands up over the marker
	// first rotate along X axis, then add offset in Z dir so it is over the marker and not in the middle of it 
	ogreNodeChild->rotate(Ogre::Vector3(1, 0, 0), Ogre::Radian(Ogre::Degree(90)));

	ogreNodeChild->translate(0, 0, -offset, Ogre::Node::TS_PARENT);
	// mesh is too big, rescale!
	const float scale = 0.24f;
	ogreNode->setScale(scale, scale, scale);

	// Init animation
	ogreEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
	baseAnim = ogreEntity->getAnimationState("RunBase");
	topAnim = ogreEntity->getAnimationState("RunTop");
	baseAnim->setLoop(true);
	topAnim->setLoop(true);
	baseAnim->setEnabled(true);
	topAnim->setEnabled(true);

	// KEYBOARD INPUT READING
	size_t windowHnd = 0;
	window->getCustomAttribute("WINDOW", &windowHnd);
	im = OIS::InputManager::createInputSystem(windowHnd);
	keyboard = static_cast<OIS::Keyboard*>(im->createInputObject(OIS::OISKeyboard, true));

	return 1;
}

int ogreGetPoseParameters(cv::Mat rvec, cv::Mat tvec, double position[3], double orientation[4])
{
	bool invalid = false;
	for (int i = 0; i < 3 && !invalid; i++) {
		if (tvec.at< double >(i, 0) != -999999)
			invalid |= false;
		if (rvec.at< double >(i, 0) != -999999)
			invalid |= false;
	}

	if (invalid) return -1;

	// calculate position vector
	position[0] = -tvec.ptr< double >(0)[0];
	position[1] = -tvec.ptr< double >(0)[1];
	position[2] = +tvec.ptr< double >(0)[2];

	// now calculare orientation quaternion
	cv::Mat Rot(3, 3, CV_32FC1);
	cv::Rodrigues(rvec, Rot);

	// calculate axes for quaternion
	double stAxes[3][3];
	// x axis
	stAxes[0][0] = -Rot.at< double >(0, 0);
	stAxes[0][1] = -Rot.at< double >(1, 0);
	stAxes[0][2] = +Rot.at< double >(2, 0);
	// y axis
	stAxes[1][0] = -Rot.at< double >(0, 1);
	stAxes[1][1] = -Rot.at< double >(1, 1);
	stAxes[1][2] = +Rot.at< double >(2, 1);
	// for z axis, we use cross product
	stAxes[2][0] = stAxes[0][1] * stAxes[1][2] - stAxes[0][2] * stAxes[1][1];
	stAxes[2][1] = -stAxes[0][0] * stAxes[1][2] + stAxes[0][2] * stAxes[1][0];
	stAxes[2][2] = stAxes[0][0] * stAxes[1][1] - stAxes[0][1] * stAxes[1][0];

	// transposed matrix
	double axes[3][3];
	axes[0][0] = stAxes[0][0];
	axes[1][0] = stAxes[0][1];
	axes[2][0] = stAxes[0][2];

	axes[0][1] = stAxes[1][0];
	axes[1][1] = stAxes[1][1];
	axes[2][1] = stAxes[1][2];

	axes[0][2] = stAxes[2][0];
	axes[1][2] = stAxes[2][1];
	axes[2][2] = stAxes[2][2];

	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".
	double fTrace = axes[0][0] + axes[1][1] + axes[2][2];
	double fRoot;

	if (fTrace > 0.0) {
		// |w| > 1/2, may as well choose w > 1/2
		fRoot = sqrt(fTrace + 1.0); // 2w
		orientation[0] = 0.5 * fRoot;
		fRoot = 0.5 / fRoot; // 1/(4w)
		orientation[1] = (axes[2][1] - axes[1][2]) * fRoot;
		orientation[2] = (axes[0][2] - axes[2][0]) * fRoot;
		orientation[3] = (axes[1][0] - axes[0][1]) * fRoot;
	} else {
		// |w| <= 1/2
		static unsigned int s_iNext[3] = { 1, 2, 0 };
		unsigned int i = 0;
		if (axes[1][1] > axes[0][0])
			i = 1;
		if (axes[2][2] > axes[i][i])
			i = 2;
		unsigned int j = s_iNext[i];
		unsigned int k = s_iNext[j];

		fRoot = sqrt(axes[i][i] - axes[j][j] - axes[k][k] + 1.0);
		double *apkQuat[3] = { &orientation[1], &orientation[2], &orientation[3] };
		*apkQuat[i] = 0.5 * fRoot;
		fRoot = 0.5 / fRoot;
		orientation[0] = (axes[k][j] - axes[j][k]) * fRoot;
		*apkQuat[j] = (axes[j][i] + axes[i][j]) * fRoot;
		*apkQuat[k] = (axes[k][i] + axes[i][k]) * fRoot;
	}
	return 1;
}




