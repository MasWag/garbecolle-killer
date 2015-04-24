#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <opencv2/opencv.hpp>
#include <iostream>


void ImageFromDisplay(std::vector<uint8_t>& Pixels, int& Width, int& Height, int& BitsPerPixel)
{
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);

    XWindowAttributes attributes = {0};
    XGetWindowAttributes(display, root, &attributes);

    Width = attributes.width;
    Height = attributes.height;

    XImage* img = XGetImage(display, root, 0, 0 , Width, Height, AllPlanes, ZPixmap);
    BitsPerPixel = img->bits_per_pixel;
    Pixels.resize(Width * Height * 4);

    memcpy(&Pixels[0], img->data, Pixels.size());

    XFree(img);
    XCloseDisplay(display);
}

using namespace cv;


int main()
{
    int Width = 0;
    int Height = 0;
    int Bpp = 0;
    std::vector<std::uint8_t> Pixels;

    cv::Scalar cols[7];
    cols[0] = cv::Scalar(0,0,255);
    cols[1] = cv::Scalar(0,255,255);
    cols[2] = cv::Scalar(255,0,255);
    cols[3] = cv::Scalar(255,0,0);
    cols[4] = cv::Scalar(0,255,0);
    cols[5] = cv::Scalar(255,255,0);
    cols[6] = cv::Scalar(255,255,255);

    Mat nerd = imread ("nerd.png");
    cvtColor (nerd,nerd,CV_RGB2GRAY);

    Mat gomi = imread ("gomi.png");
    cvtColor (gomi,gomi,CV_RGB2GRAY);

    ImageFromDisplay(Pixels, Width, Height, Bpp);

    if (Width && Height)
      {
        Mat img = Mat(Height, Width, Bpp > 24 ? CV_8UC4 : CV_8UC3, Pixels.data());

        cvtColor (img,img,CV_RGB2GRAY);

        Mat result;
        matchTemplate(img, nerd, result, CV_TM_CCOEFF_NORMED);

        // 最大のスコアの場所を探す
        cv::Point max_pt;
        double maxVal;
        cv::minMaxLoc(result, NULL, &maxVal, NULL, &max_pt);

        // 一定スコア以下の場合は処理終了
        if ( maxVal < 0.5 ) return 0;
 
        std::cout << max_pt + Point {nerd.cols/2,nerd.rows/2} << std::endl;

        matchTemplate(img, gomi, result, CV_TM_CCOEFF_NORMED);
        cv::Point gomi_max_pt;
        cv::minMaxLoc(result, NULL, &maxVal, NULL, &gomi_max_pt);

        std::cout << gomi_max_pt + Point {gomi.cols/2,gomi.rows/2} << std::endl;
#ifdef DEBUG
        cvtColor (img,img,CV_GRAY2RGB);
        // 探索結果の場所に矩形を描画
        cv::rectangle(img, max_pt,max_pt + Point {nerd.cols,nerd.rows} ,cols[0], 3);

        cv::rectangle(img, gomi_max_pt,gomi_max_pt + Point {gomi.cols,gomi.rows} ,cols[3], 3);
        imshow("Display window", img);
#endif        
        

        waitKey(0);
    }
    return 0;
}
