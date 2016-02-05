#include <opencv2/opencv.hpp>
#include <networktables/NetworkTable.h>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	int retVal=0;
    VideoCapture vcap;
    Mat image;
    NetworkTable::SetClientMode();
    //NetworkTable::SetTeam(1259);
    NetworkTable::SetIPAddress("10.12.59.1");
    shared_ptr<NetworkTable> netTable = NetworkTable::GetTable("OpenCV");

    // This works on a D-Link CDS-932L
    const string videoStreamAddress = "rtsp://FRC:frc@axis-camera.local:554/axis-media/media.amp?videocodec=h264";

    //open the video stream and make sure it's opened
    if (!vcap.open(videoStreamAddress)) {
        cout << "Error opening video stream or file" << std::endl;
        retVal=-1;
    } else
    {
	   int frame = 0;
	   for (;;) {
			if (!vcap.read(image)) {
				cout << "No frame" << endl;
				waitKey();
			}

			imshow("Output Window", image);
			netTable->GetNumber("OpenCV Frame",frame);
			frame++;
			if (waitKey(1) >= 0) break;
		}
    }
  if (retVal == -1)
  {
	  image = imread(argc == 2 ? argv[1] : "lena.jpg", 1);
	  if (image.empty())
	  {
		cout << "Cannot open image!" << endl;
		retVal=-2;
	  } else
	  {
			netTable->GetString("OpenCV Failure","failure");
		  imshow("Output Window", image);
		  waitKey(0);
	  }
  }

  return retVal;
}
