#include <opencv2/opencv.hpp>
#include <networktables/NetworkTable.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <Windows.h>
#include "resource2.h"

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

const Rect bounds = Rect(275, 95, 90, 145);
const Point2f center = (bounds.tl() + bounds.br())*0.5;


void myContours(Mat src_gray, Mat image, int thresh = 100, int max_thresh = 255);


bool rectCompare(Rect l, Rect r);


Mat bmp2mat(LPCWSTR name);


int main(int argc, char* argv[])
{
	
	VideoCapture videostream;
	Mat failimage, streamimage, inrangeimage;
	failimage = bmp2mat(MAKEINTRESOURCE(IDB_BITMAP1));

	NetworkTable::SetClientMode();
	//NetworkTable::SetTeam(1259);
	//NetworkTable::SetIPAddress("10.12.59.1");
	NetworkTable::SetIPAddress("roboRIO-1259-FRC.local");
	//namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", failimage);

	//open the video stream and make sure it's opened
	while (true)
	{
		if (videostream.isOpened()||videostream.open(videoStreamAddress))
		{
//			cout << "while" << endl;
			// read an image from the stream
			if (videostream.read(streamimage))
			{
				//imshow("Output Window", image);
				// filter the image
//				cout << "inrange" << endl;
				inRange(streamimage, Scalar(128, 128, 0), Scalar(255, 255, 54), inrangeimage);
//				cout << "afer inrange" << endl;
				myContours(inrangeimage, streamimage);
//				cout << "afer contours" << endl;
				//	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
//				imshow("Contours", streamimage);
//				cout << "waitkey" << endl;
				if (waitKey(1) != -1)
					break;
			}
			else
			{
				cerr << "Error:  Unable to read frame from stream" << endl;
				if (waitKey(200) != -1)
					break;
			}
		}
		else
		{
			cerr << "Error:  Unable to open video stream" << endl;
			if (waitKey(200) != -1)
				break;
		}
	}
	return 0;
}


void myContours(Mat inrangeimage, Mat streamimage, int thresh, int max_thresh)
{
	shared_ptr<NetworkTable> netTable = NetworkTable::GetTable("OpenCV");
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	vector<double> rectsprops[8];
	bool(*compare) (Rect, Rect) = rectCompare;
	map<Rect, RotatedRect, bool(*)(Rect, Rect)> rects2 (compare);

	for (size_t i = 0; i < 8; i++)
		rectsprops[i] = vector<double>(0);

	findContours(inrangeimage, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	for (size_t i = 0; (i < contours.size()) && (rects2.size() < 8); i++)
	{
		if (hierarchy[i][3] < 0)
		{
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
		double w = unionrect.br().x;
		w -= bounds.br().x;
		double h = unionrect.br().y; 
		h -= bounds.br().y;
		double xpos = w + x;
		if ((w != 0) && (x != 0) && (abs(xpos) < 10))
			xpos = 0;
		rectsprops[5].push_back(xpos);
		double ypos = h + y;
		if ((h != 0) && (y != 0) && (abs(ypos) < 10))
			ypos = 0;
		rectsprops[6].push_back(ypos);
		rectsprops[7].push_back(i->second.angle);
//		cout << count << " " << j << " rectsprops" << endl;

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
//		cout << count << " " << j << " rectangle" << endl;
	}

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
//	cout << count << " putnumber" << endl;

	rectangle(streamimage, bounds, white, 3);
//	cout << count << " rectangle 2" << endl;

	// Show in a window
	imshow("Contours", streamimage);
	j = 0;
	for (vector<vector<Point> >::iterator i = contours.begin(); i != contours.end(); i++, j++)
	{
			//cout << j << " " << i->size() << endl;
			i->clear();
	}
	contours.clear();
//	cout << count << " show" << endl;
//	count++;
}


bool rectCompare(Rect l, Rect r) {
	//biggest area is closest
	if (l.width > r.width)
		return true;
	if (l.width < r.width)
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
	//farther up is closest
	if (l.y < r.y)
		return true;
	if (l.y > r.y)
		return false;
	//closer to frame right (robot left) is closest
	if (l.x > r.x)
		return true;
	return false;
}


Mat bmp2mat(LPCWSTR name) {

	Mat src;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	HDC hDC = CreateCompatibleDC(NULL);
	// Find the bitmap resource
	HRSRC hResInfo = FindResource(hInstance, name, RT_BITMAP);

	// Load the bitmap resource
	HGLOBAL hMemBitmap = LoadResource(hInstance, hResInfo);

	// Lock the resource and access the entire bitmap image
	PBYTE pBitmapImage = (BYTE*)LockResource(hMemBitmap);
	if (pBitmapImage == NULL)
	{
		FreeResource(hMemBitmap);
	}

	// Store the width and height of the bitmap
	BITMAPINFO* pBitmapInfo = (BITMAPINFO*)pBitmapImage;
	int m_iWidth = (int)pBitmapInfo->bmiHeader.biWidth;
	int m_iHeight = (int)pBitmapInfo->bmiHeader.biHeight;

	src.create(m_iHeight, m_iWidth, CV_8UC3);

	const PBYTE pTempBits = pBitmapImage + pBitmapInfo->bmiHeader.biSize +
		pBitmapInfo->bmiHeader.biClrUsed * sizeof(RGBQUAD);
	CopyMemory(src.data, pTempBits, pBitmapInfo->bmiHeader.biSizeImage);

	Mat dst;
	flip(src, dst, 0);

	UnlockResource(hMemBitmap);
	FreeResource(hMemBitmap);
	DeleteObject(hDC);

	return dst;
}
