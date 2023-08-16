#include <iostream>
#include <algorithm>
#include "TiffImage.h"
#include "WebPImage.h"


int main() {
    TiffImage tif = TiffImage::openTiff("../test.tif");

    if (tif.getNumberOfDirectory() > 1) {
        std::cout << "Only working with the 1st directory" << std::endl;
    }

    WebPImage img = WebPImage::fromTiff(tif);
    img.save("../test.webp");
    return 0;
}
