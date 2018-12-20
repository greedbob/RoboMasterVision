//
// Created by greedbob on 18-12-19.
//

#include "FlowerDetector.hpp"
using namespace cv;

void FlowerDetector::setImage() {
    Mat hsv;
    max_color = cv::Mat(src.size(), CV_8UC1, cv::Scalar(0));
    threshold(src, src_red, 155, 255, THRESH_BINARY);

//    cvtColor(src, hsv, CV_BGR2HSV);
//    for (int i = 0; i < hsv.rows; i++) {
//        for (int j = 0; j < hsv.cols; j++) {
//            if ((((hsv.ptr<Vec3b>(i)[j][0] > 0) and (hsv.ptr<Vec3b>(i)[j][0] < 8)) or ((hsv.ptr<Vec3b>(i)[j][0] > 120) and (hsv.ptr<Vec3b>(i)[j][0] < 180)))) {
//                max_color.ptr<Vec3b>(i)[j] = 255;
//            }
//        }
//    }
//    imshow("hsv", hsv);


    medianBlur(src, src, 1);
    int total_pixel = src.cols * src.rows;
    const uchar * ptr_src = src.data;
    const uchar * ptr_src_end = src.data + total_pixel * 3;
    uchar *ptr_max_color = max_color.data;
    for (; ptr_src != ptr_src_end; ++ptr_src, ++ptr_max_color)	{
        uchar b = *ptr_src;
        uchar g = *(++ptr_src);
        uchar r = *(++ptr_src);
        if (r > 200 and (g < 230 and b < 230))
            *ptr_max_color = 255;
    }

    Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
    morphologyEx(max_color, max_color, MORPH_OPEN, element);
//    Mat splitChannels[3];
//    split(src, splitChannels);
//    src_channel_r = splitChannels[2];
//    src_channel_g = splitChannels[1];
//    src_channel_b = splitChannels[0];
//    imshow("src_channel_r", src_channel_r);
    imshow("src", src);
    imshow("max_color", max_color);
}


void FlowerDetector::findAll() {
    RotatedRect rect;
//    Mat img_rects = Mat(max_color.size(), CV_8UC1, cv::Scalar(0));
//    output = Mat::zeros(max_color.size(), CV_8UC3);
    vector<vector<Point2i>> contours;
    vector<Vec4i> hierarchy;
    findContours(max_color, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    cout << contours.size() << endl;
    drawContours(output, contours, -1, cvScalarAll(255), 1, 8, hierarchy);
//    vector<vector<Point2i> >::const_iterator it = contours.begin();
    for (int i = 0; i < contours.size(); ++i){
        rect = minAreaRect(contours[i]);
        rects.push_back(rect);
    }
//    imshow("img_rects", img_rects);
//    imshow("output", output);
};


void FlowerDetector::showRects(vector<RotatedRect> & rects){
//    Mat img_rects = Mat(src.size(), CV_8UC3, cv::Scalar(0));
    Mat img_rects = src;
//    const uchar * ptr_img_rects = img_rects.data;
    for (int i = 0; i < src.cols * src.rows; ++i){
        for (int j = 0; j < rects.size(); ++j){
            Point2f vertices[4];
            rects[j].points(vertices);
            for (int k = 0; k < 4; k++) {
                line(img_rects, vertices[k], vertices[(k + 1) % 4], Scalar(0, 0, 0), 5);
//                Rect brect = rects[j].boundingRect();
//                rectangle(img_rects, brect, Scalar(255, 0, 0));
            }
        }
    }
    imshow("img_rects", img_rects);
}

void FlowerDetector::outPut() {
    setImage();
    findAll();
    cout << "rects size is " << rects.size() << endl;
    showRects(rects);
}