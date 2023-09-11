#include <iostream>
#include <algorithm>
#include <fstream>
#include "TiffImage.h"
#include "WebPImage.h"


int main() {
    std::ifstream stream("../test.tif", std::ios::binary);
    std::vector<uint8_t> data;

    std::for_each(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>(),
                  [&data](const char c) { data.push_back(c); });

    TiffImage tif = TiffImage::openTiffFromMemory(data);

    if (tif.getNumberOfDirectory() > 1) {
        std::cout << "Only working with the 1st directory" << std::endl;
    }

    WebPImage img = WebPImage::fromTiff(tif);
    img.save("../test.webp");
    return 0;
}
