#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <iostream>

using namespace cv;
using namespace std;

// Function to preprocess the image for plate detection
Mat preprocessImage(Mat& image) {
    Mat gray, blurred, edgeDetected;
    cvtColor(image, gray, COLOR_BGR2GRAY);  // Convert to grayscale
    GaussianBlur(gray, blurred, Size(5, 5), 0);  // Apply Gaussian blur to reduce noise
    Canny(blurred, edgeDetected, 100, 200);  // Edge detection to highlight the contours
    return edgeDetected;
}

// Function to detect the license plate (basic approach)
Rect detectLicensePlate(Mat& edgeDetected) {
    vector<vector<Point>> contours;
    findContours(edgeDetected, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        Rect boundingRec = boundingRect(contour);

double perimeter = arcLength(contour , true);

// Filter potential license plate region by aspect ratio and area
        if (boundingRec.width > boundingRec.height && boundingRec.area() > 500 && (static_cast<double>(boundingRec.width) / boundingRec.height > 2)&&(perimeter > 100 && perimeter < 400)) {
            return boundingRec;  // Return the detected license plate-like region
        }
    }
    return Rect();  // Return an empty rect if no license plate is found
}

int main(int argc, char** argv) {
    // Load the image of the car
   // Mat carImage = imread("car.jpeg");
    if(argc>1)
            cout<< ">>" <<argv[1]<<"<<\n";
    Mat carImage = imread(argv[1]);

    if (carImage.empty()) {
        cout << "Error: Could not load image!" << endl;
        return -1;
    }

    // Preprocess the image
    Mat edgeDetected = preprocessImage(carImage);

    // Detect the license plate
    Rect licensePlateRegion = detectLicensePlate(edgeDetected);

    if (licensePlateRegion.area() == 0) {
        cout << "Error: No license plate detected!" << endl;
        return -1;
    }

    // Draw the rectangle on the original image
    rectangle(carImage, licensePlateRegion, Scalar(0,255,0),2);
    imshow("Detected License Plate",carImage);
    waitKey(0);
   
    //Extract the license plate region from the image
    Mat licensePlate = carImage(licensePlateRegion);
    //Save the image
    imwrite("extracted_license_plate.jpg", licensePlate);
   
   
    //Initialize Tesseract ocr
    tesseract :: TessBaseAPI ocr;
    if(ocr.Init(NULL, "eng", tesseract :: OEM_DEFAULT)){
            cout<<"Error: Could not initialize Tesseract!"<<endl;
            return -1;
}          
//Set tesseract to work with license plate image
ocr.SetImage(licensePlate.data, licensePlate.cols, licensePlate.rows, 3, licensePlate.step);

// Optional: Set a character whitelist and page segmentaion mode
ocr.SetVariable("tessedit_char_whitelist","ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
ocr.SetPageSegMode(tesseract::PSM_SINGLE_LINE);

//Perform OCR to extract the license number
string licenseNumber = ocr.GetUTF8Text();

//Print the extracted License number
cout<<"License Plate Number:"<< licenseNumber <<endl;

ocr.End();
return 0;
}