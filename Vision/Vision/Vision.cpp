// Vision.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

int main()
{
    std::string control = "Control";
    std::string checks = "Checks";

    std::string image_path = "C:\\Users\\User\\Downloads\\VisionTemplate.png";

    cv::VideoCapture cam(0);
    cv::Mat frame;
    cv::Mat hsv;
    cv::Mat rangehsv;
    cv::Mat rangeN;
    cv::Mat threshold;
    cv::Mat canny;
    cv::Mat gray;
    cv::Mat filtered;

    cv::Scalar HSVlow = { 0, 0, 0 };
    cv::Scalar HSVhigh = { 255, 255, 255 };

    int low[] = { 51, 97, 171 };
    int high[] = { 83, 255, 255 };
    
    cv::Scalar lineColors[5];
    lineColors[0] = { 255, 0, 0 };
    lineColors[1] = { 0, 255, 0 };
    lineColors[2] = { 0, 0, 255 };
    lineColors[3] = { 255, 0, 255 };
    lineColors[4] = { 0, 255, 255 };

    int errosionSize = 3;

    cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
        cv::Size(2 * errosionSize + 1, 2 * errosionSize + 1),
        cv::Point(errosionSize, errosionSize));

    int i_alpha = 1;
    double d_alpha;
    double beta;

    //Checks Variables
    double maxArea;
    double minArea;
    double maxSolid;
    double minSolid;
    double maxRatio;
    double minRatio;

    int i_maxArea = 20000;
    int i_minArea = 0;
    int i_maxSolid = 100;
    int i_minSolid = 0;
    int i_maxRatio = 100;
    int i_minRatio = 0;

    while (true)
    {
        //Taskbars
        cv::namedWindow(control, cv::WINDOW_AUTOSIZE);
        cv::createTrackbar("lowH", control, &low[0], 255);
        cv::createTrackbar("highH", control, &high[0], 255);
        cv::createTrackbar("lowS", control, &low[1], 255);
        cv::createTrackbar("highS", control, &high[1], 255);
        cv::createTrackbar("lowV", control, &low[2], 255);
        cv::createTrackbar("highV", control, &high[2], 255);
        cv::createTrackbar("elementS", control, &errosionSize, 100);

        element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
            cv::Size(2 * errosionSize + 1, 2 * errosionSize + 1),
            cv::Point(errosionSize, errosionSize));

        cv::namedWindow(checks, cv::WINDOW_AUTOSIZE);
        cv::createTrackbar("minArea", checks, &i_minArea, 30000);
        cv::createTrackbar("maxArea", checks, &i_maxArea, 30000);
        cv::createTrackbar("minSolid", checks, &i_minSolid, 100);
        cv::createTrackbar("maxSolid", checks, &i_maxSolid, 100);
        cv::createTrackbar("minRatio", checks, &i_minRatio, 10);
        cv::createTrackbar("maxRatio", checks, &i_maxRatio, 10);

        minArea = i_minArea;
        maxArea = i_maxArea;
        minSolid = i_minSolid / 100.0;
        maxSolid = i_maxSolid / 100.0;
        minRatio = i_minRatio;
        maxRatio = i_maxRatio;

        for (int i = 0; i < HSVlow.channels - 1; i++)
        {
            HSVlow[i] = low[i];
            HSVhigh[i] = high[i];
        }

        frame = cv::imread(image_path);
        filtered = frame;
        //cam >> frame;

        if (frame.empty())
        {
            std::cout << "Failed to Read file" << std::endl;
            return 1;
        }

        cv::imshow("Default", frame);

        /*cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::imshow("Gray", gray);*/

        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        //cv::imshow("hsv", hsv);

        //cv::inRange(frame, HSVlow, HSVhigh, rangeN);
        cv::inRange(hsv, HSVlow, HSVhigh, rangehsv);

        //cv::imshow("RangeN", rangeN);
        cv::imshow("Rangehsv", rangehsv);

        cv::threshold(rangehsv, threshold, 180, 255, cv::THRESH_BINARY);
        cv::erode(threshold, threshold, element);
        cv::dilate(threshold, threshold, element);
        //cv::imshow("Threshold", threshold);

        cv::Canny(threshold, canny, 0, 0);
        cv::imshow("Canny", canny);

        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        cv::Mat matCon = cv::Mat::zeros(canny.rows, canny.cols, CV_8UC3);

        for (size_t i = 0; i < contours.size(); i++)
        {
            //CHECKS

            //Contour Area
            double contourArea = cv::contourArea(contours[i]);
            if (contourArea > maxArea || contourArea < minArea)
            {
                std::cout << "Area: " << "contourArea = " << contourArea << " minArea = " << minArea << " maxArea = "
                    << maxArea << std::endl;
                continue;
            }

            //Solidarity
            cv::Rect boundRect = cv::boundingRect(contours[i]);
            double solid = cv::contourArea(contours[i]) / (1.0 * boundRect.width * boundRect.height);
            if (solid > maxSolid || solid < minSolid)
            {
                std::cout << "Solidarity: " << "solid = " << solid << " minSolid = " << minSolid << " maxSolid = "
                    << maxSolid << std::endl;
                continue;
            }

            //Aspect Ratio
            double ratio = (double)boundRect.width / boundRect.height;
            //maxRatio = 2.2 + 0.10;
            //minRatio = 2.2 - 0.10;
            if (ratio > maxRatio || ratio < minRatio)
            {
                std::cout << "Ratio: " << "ratio = " << ratio << " minRatio = " << minRatio << " maxRatio = "
                    << maxRatio << std::endl;
                continue;
            }
            
            double centerX = boundRect.x + (double)(boundRect.width / 2);
            double centerY = boundRect.y + (double)(boundRect.height / 2);
            cv::Moments moment = cv::moments(contours[i]);
            cv::Point center = cv::Point2f(moment.m10 / moment.m00, moment.m01 / moment.m00);

            cv::line(matCon, cv::Point(center.x, 0), cv::Point(center.x, matCon.size().height), lineColors[i], 2);
            cv::line(matCon, cv::Point(0, center.y), cv::Point(matCon.size().width, center.y), lineColors[i], 2);
            cv::line(matCon, cv::Point(centerX, centerY - boundRect.height / 2), cv::Point(centerX, centerY + boundRect.height / 2), lineColors[i+1], 2);
            cv::line(matCon, cv::Point(centerX - boundRect.width / 2, centerY), cv::Point(centerX + boundRect.width / 2, centerY), lineColors[i+1], 2);

            cv::rectangle(filtered, boundRect, lineColors[i], 2);

            /*cv::line(filtered, cv::Point(center.x, 0), cv::Point(center.x, matCon.size().height), lineColors[i+1], 2);
            cv::line(filtered, cv::Point(0, center.y), cv::Point(matCon.size().width, center.y), lineColors[i+1], 2);*/
            cv::line(filtered, cv::Point(centerX, centerY - boundRect.height/2), cv::Point(centerX, centerY+boundRect.height/2 ), lineColors[i], 2);
            cv::line(filtered, cv::Point(centerX - boundRect.width/2, centerY), cv::Point(centerX + boundRect.width/2, centerY), lineColors[i], 2);

            cv::drawContours(matCon, contours, i, cv::Scalar(255, 0, 255), 3);
        }

        cv::imshow("Contours", matCon);
        cv::imshow("Filtered", filtered);

        cv::waitKey(1);
    }
}