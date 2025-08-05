#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/dnn.hpp>
#include <opencv4/opencv2/objdetect.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <memory>

namespace imagepro
{
    class AgeDetection
    {
    private:
        AgeDetection()
        {
        }
        
        AgeDetection(const AgeDetection&) = delete;

        AgeDetection& operator=(const AgeDetection&) = delete;

        std::unique_ptr<cv::CascadeClassifier> face_cascade = nullptr;
        std::unique_ptr<cv::dnn::Net> age_net = nullptr;
        std::unique_ptr<cv::dnn::Net> faceNet = nullptr;

        const cv::Scalar MODEL_MEAN_VALUES = cv::Scalar(104.0, 177.0, 123.0);

        const std::vector<std::string> AGE_GROUPS = {
            "(0-4)", "(5-12)", "(13-17)", "(19-24)", "(25-37)", "(38-43)", "(44-53)", "(54-100)"
        };

        int currImageAdult = -1;

    public:
        static AgeDetection& GetInstance()
        {
            static AgeDetection instance; // Guaranteed to be thread-safe in C++11 and later
            return instance;
        }

        bool isCurrImageAdultPerson()
        {
            return currImageAdult != 0;
        }

        void registerCurrImage(const cv::Mat& img)
        {
            this->currImageAdult = isAdultPerson(img);
        }

        int isAdultPerson(const cv::Mat& img)
        {
            if (img.empty())
            {
                std::cerr << "Input image is empty.\n";
                return -1;
            }

            // Load face cascade
            if (face_cascade == nullptr)
            {
                face_cascade = std::make_unique<cv::CascadeClassifier>("haarcascade_frontalface_default.xml");
                if (!face_cascade->load("haarcascade_frontalface_default.xml"))
                {
                    std::cerr << "Failed to load face cascade.\n";
                    return -1;
                }
            }

            // Load age estimation model
            if (age_net == nullptr)
            {
                age_net = std::make_unique<cv::dnn::Net>(cv::dnn::readNetFromCaffe("deploy_age.prototxt", "age_net.caffemodel"));
                if (age_net->empty())
                {
                    std::cerr << "Failed to load age detection model.\n";
                    return -1;
                }
            }

            if (faceNet == nullptr)
            {
                faceNet = std::make_unique<cv::dnn::Net>(cv::dnn::readNet("res10_300x300_ssd_iter_140000.caffemodel", "facedeploy.prototxt"));
            }

            int result = -1;

            // first pass method 1
            {
                // Face detection
                std::vector<cv::Rect> faces;
                cv::Mat gray;
                cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
                face_cascade->detectMultiScale(gray, faces);

                for (const auto& face : faces)
                {
                    cv::Mat face_roi = img(face);
                    cv::Mat blob = cv::dnn::blobFromImage(face_roi, 1.0, cv::Size(227, 227),
                        cv::Scalar(78.4263377603, 87.7689143744, 114.895847746), false);

                    age_net->setInput(blob);
                    cv::Mat age_pred = age_net->forward();
                    int max_idx = std::max_element(age_pred.begin<float>(), age_pred.end<float>()) - age_pred.begin<float>();

                    std::string predicted_age = AGE_GROUPS[max_idx];

                    if (max_idx < 3)
                    {
                        result = 0;
                    }
                }
            }

            // second pass method 2
            {
                int imgHeight = img.rows;
                int imgWidth = img.cols;

                cv::Mat inputBlob = cv::dnn::blobFromImage(img, 1.0, cv::Size(300, 300), MODEL_MEAN_VALUES, false, false);
                faceNet->setInput(inputBlob, "data");
                cv::Mat detection = faceNet->forward("detection_out");

                cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

                // Iterate over detected faces
                for (int i = 0; i < detectionMat.rows; i++)
                {
                    float confidence = detectionMat.at<float>(i, 2);

                    if (confidence > 0.7)
                    {
                        int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * imgWidth);
                        int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * imgHeight);
                        int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * imgWidth);
                        int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * imgHeight);

                        // Ensure bounding box is within image boundaries
                        x1 = std::max(0, x1);
                        y1 = std::max(0, y1);
                        x2 = std::min(imgWidth - 1, x2);
                        y2 = std::min(imgHeight - 1, y2);

                        // Extract the face region
                        cv::Rect faceBox(x1, y1, x2 - x1, y2 - y1);
                        cv::Mat face = img(faceBox);

                        // Prepare the face for age prediction
                        cv::Mat ageBlob = cv::dnn::blobFromImage(face, 1.0, cv::Size(227, 227), MODEL_MEAN_VALUES, false, false);
                        age_net->setInput(ageBlob);
                        cv::Mat agePreds = age_net->forward();

                        int max_idx = 0;
                        for (int j = 1; j < agePreds.cols; j++)
                        {
                            if (agePreds.at<float>(0, j) > agePreds.at<float>(0, max_idx))
                            {
                                max_idx = j;
                            }
                        }

                        if (max_idx < 3)
                        {
                            result = 0;
                        }
                    }
                }
            }

            return result;
        }
    };

}