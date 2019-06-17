#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>

QImage MainWindow::cvMat2QImage(const Mat& mat)
{
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    else if(mat.type() == CV_8UC3)
    {
        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        return QImage();
    }
}

void MainWindow::ShowImage(const Mat& mat, QLabel* lpLabel)
{
    QImage qimg = cvMat2QImage(mat);
    lpLabel->setPixmap(QPixmap::fromImage(qimg).scaled(lpLabel->size()));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    cameraThread(&MainWindow::CameraLoop, this),
    faceThread(&MainWindow::FaceScan, this),
    camshiftThread(&MainWindow::MyCamShift, this)
{
    ui->setupUi(this);
    String face_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml";
    String eyes_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
    face_cascade.load(face_cascade_name);
    eyes_cascade.load(eyes_cascade_name);
    std::string videoStreamAddress = "http://192.168.0.11:8080/video";
    //cap = VideoCapture(0);
    cap.open(videoStreamAddress);
    //QObject::connect(ui->openButton, SIGNAL(clicked()), this, SLOT(CameraLoop()), Qt::AutoConnection);
}

MainWindow::~MainWindow()
{
    exitSignel = true;
    cap.release();
    delete ui;
}

void MainWindow::CameraLoop()
{
    while(!exitSignel)
    {
        usleep(30);
        if(cap.isOpened())
            cap.read(cameraMat);
    }
}
void MainWindow::MyCamShfit()
{
    int _vmin = 100, _vmax = 100;//use bar to adjust
    cv::Mat hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    cvtColor(cameraMat,hsv, COLOR_BGR2HSV);
    inRange(hsv, Scalar(0, smin, MIN(_vmin, _vmax)), Scalar(180, 256, MAX(_vmin, _vmax)), mask);
    int ch[] = {0, 0};
    hue.create(hsv)

}

void MainWindow::FaceScan()
{
    while(!exitSignel)
    {
        usleep(30);
        if(cameraMat.empty())
            continue;
        Mat frame_gray;
        cvtColor(cameraMat, frame_gray, CV_BGR2GRAY);
        equalizeHist(frame_gray, frame_gray);
        face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30));
        qDebug() << faces.size();
        for(size_t i = 0; i < faces.size(); i++)
        {
            Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5);
            ellipse(cameraMat, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0);
        }
        ShowImage(cameraMat, ui->faceLabel);
    }
/*  for( size_t i = 0; i < faces.size(); i++ )
    {
      Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
      ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
      Mat faceROI = frame_gray( faces[i] );
      std::vector<Rect> eyes;
      eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );
      for( size_t j = 0; j < eyes.size(); j++ )
       {
         Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
         int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
         circle( frame, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
       }
    }
    ShowImage(frame, ui->faceLabel);*/
}
