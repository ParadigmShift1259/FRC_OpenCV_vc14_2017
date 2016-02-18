#include <opencv2/opencv.hpp>
#include <networktables/NetworkTable.h>
#include <algorithm>
#include <vector>
#include <iostream>


using namespace cv;
using namespace std;


const string videoStreamAddress = "rtsp://FRC:frc@axis-camera.local:554/axis-media/media.amp?videocodec=h264";

const Scalar red = Scalar(0, 0, 255);
const Scalar orange = Scalar(0, 128, 255);
const Scalar yellow = Scalar(0, 255, 255);
const Scalar greenyellow = Scalar(0, 255, 128);
const Scalar green = Scalar(0, 255, 0);
const Scalar springgreen = Scalar(128, 255, 0);
const Scalar cyan = Scalar(255, 255, 0);
const Scalar lightblue = Scalar(255, 128, 0);
const Scalar blue = Scalar(255, 0, 0);
const Scalar electricindigo = Scalar(255, 0, 128);
const Scalar magenta = Scalar(255, 0, 255);
const Scalar flushorange = Scalar(128, 0, 255);
const Scalar white = Scalar(255, 255, 255);
const Rect bounds = Rect(280, 180, 80, 40);
const Point2f center = (bounds.tl() + bounds.br())*0.5;

void myContours(Mat src_gray, Mat image, shared_ptr<NetworkTable> netTable, int thresh = 100, int max_thresh = 255);


bool rectCompare(Rect l, Rect r) {
	//biggest area is closest
	if (l.area() > r.area())
		return true;
	if (l.area() < r.area())
		return false;
	//nearest to the center of the bounds is closest
	Point2f lcenter = ((l.tl() + l.br())*0.5);
	Point2f rcenter = ((r.tl() + r.br())*0.5);
	Point2f ldist = lcenter - center;
	Point2f rdist = rcenter - center;
	if (ldist.dot(ldist) < rdist.dot(rdist))
		return true;
	if (ldist.dot(ldist) > rdist.dot(rdist))
		return false;
	//lower down is closest
	if (l.y > r.y)
		return true;
	if (l.y < r.y)
		return false;
	//closer to frame right (robot left) is closest
	if (l.x > r.x)
		return true;
	return false;
}


int main(int argc, char* argv[])
{
	VideoCapture videostream;
	Mat streamimage, inrangeimage;

	NetworkTable::SetClientMode();
	//NetworkTable::SetTeam(1259);
	//NetworkTable::SetIPAddress("10.12.59.1");
	NetworkTable::SetIPAddress("roboRIO-1259-FRC.local");
	shared_ptr<NetworkTable> netTable = NetworkTable::GetTable("OpenCV");
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);

	//open the video stream and make sure it's opened
	if (videostream.open(videoStreamAddress))
	{
		while (true)
		{
			// read an image from the stream
			if (videostream.read(streamimage))
			{
				//imshow("Output Window", image);
				// filter the image
				inRange(streamimage, Scalar(128, 128, 0), Scalar(255, 255, 54), inrangeimage);
				myContours(inrangeimage, streamimage, netTable);
//				namedWindow("Contours", CV_WINDOW_AUTOSIZE);
//				imshow("Contours", streamimage);
				if (waitKey(1) != -1)
					break;
			}
			else
			{
				cout << "Error:  Unable to read frame from stream" << endl;
			}
		}
	}
	else
	{
		cout << "Error:  Unable to open video stream" << endl;
		return 1;
	}
	return 0;
}


void myContours(Mat inrangeimage, Mat streamimage, shared_ptr<NetworkTable> netTable, int thresh, int max_thresh)
{
	vector<vector<Point>> contours;
	//vector<Point> hull;
	//vector<Rect> rects;
	vector<Vec4i> hierarchy;
	vector<double> rectsprops[8];
	bool(*compare) (Rect, Rect) = rectCompare;
	map<Rect, RotatedRect, bool(*)(Rect, Rect)> rects2 (compare);
	
	for (size_t i = 0; i < 8; i++)
		rectsprops[i] = vector<double>(0);

	findContours(inrangeimage, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	for (size_t i = 0; (i < contours.size()) && (rects2.size() < 8); i++)
	{
		if (hierarchy[i][3] < 0)
		{
			//convexHull(contours[i], hull);
			RotatedRect a = minAreaRect(contours[i]);
			if (a.size.area() > 300)
			{
				rects2[boundingRect(contours[i])] = a;
			}
		}
	}

	int j = 0;
	for (map<Rect, RotatedRect, bool(*)(Rect, Rect)>::iterator i = rects2.begin(); i != rects2.end(); i++, j++)
	{
		//cout << "i " << i << endl;
		rectsprops[0].push_back(i->first.area());
		rectsprops[1].push_back(i->first.x);
		rectsprops[2].push_back(i->first.y);
		rectsprops[3].push_back(i->first.width);
		rectsprops[4].push_back(i->first.height);
		Rect unionrect = i->first | bounds;
		double x = unionrect.x;
		x -= bounds.x;
		double y = unionrect.y; 
		y -= bounds.y;
		double w = unionrect.width;
		w -= bounds.width;
		double h = unionrect.height; 
		h -= bounds.height;
		double xpos = w + x;
		if ((w != 0) && (x != 0) && (abs(xpos) < 10))
			xpos = 0;
		rectsprops[5].push_back(xpos);
		double ypos = h + y;
		if ((h != 0) && (y != 0) && (abs(ypos) < 10))
			ypos = 0;
		rectsprops[6].push_back(ypos);
		rectsprops[7].push_back(i->second.angle);

		Scalar color;
		switch (j % 12)
		{
		case 0 :
			color = red;
			break;
		case 1 :
			color = yellow;
			break;
		case 2:
			color = green;
			break;
		case 3:
			color = cyan;
			break;
		case 4:
			color = blue;
			break;
		case 5:
			color = magenta;
			break;
		case 6 :
			color = greenyellow;
			break;
		case 7:
			color = lightblue;
			break;
		case 8:
			color = orange;
			break;
		case 9:
			color = electricindigo;
			break;
		case 10 :
			color = springgreen;
			break;
		case 11 :
			color = flushorange;
			break;
		}
		//rectangle(streamimage, rotrects[i], color, 2);
		rectangle(streamimage, i->first, color, 3);
	}

	//cout << "1" << endl;

	if (rects2.size() > 0)
	{
		netTable->PutNumber("area", rectsprops[0][0]);
		netTable->PutNumber("x", rectsprops[1][0]);
		netTable->PutNumber("y", rectsprops[2][0]);
		netTable->PutNumber("width", rectsprops[3][0]);
		netTable->PutNumber("height", rectsprops[4][0]);
		netTable->PutNumber("xpos", rectsprops[5][0]);
		netTable->PutNumber("ypos", rectsprops[6][0]);
		netTable->PutNumber("angle", rectsprops[7][0]);
	}

	rectangle(streamimage, bounds, white, 3);

	// Show in a window
	imshow("Contours", streamimage);
}
