#include <iostream>
#include <algorithm>
#include "TiffImage.h"
#include "WebPImage.h"


int main() {
    TiffImage tif = TiffImage::openTiff("../test.tif", "r");

    if (tif.getNumberOfDirectory() > 1) {
        std::cout << "Only working with the 1st directory" << std::endl;
    }

    PixelScale scale = tif.getPixelScale();
    std::cout << scale.x << " " << scale.y << " " << scale.z << std::endl;

    for (TiePoint p: tif.getTiePoints()) {
        std::cout << std::setprecision(8);
        std::cout << p.imgX << "," << p.imgY << "," << p.imgZ
                  << " -> "
                  << p.coordX << "," << p.coordY << "," << p.coordZ << std::endl;
    }
    for (const auto &s: tif.getAsciiParams()) {
        std::cout << '"' << s << "\" ";
    }

    std::cout << std::endl;

    for (unsigned short e: tif.getGeoKeys()) {
        std::cout << e << " ";
    }
    std::cout << std::endl;

    auto de = tif.getDoubleParams();
    std::for_each(de.begin(), de.end(), [](double a) { std::cout << a << " "; });
    std::cout << "\n\n\n\n" << std::flush << std::endl;

    std::cout << tif.getImageWidth() << "x" << tif.getImageHeight() << std::endl;
    std::cout << "bps : " << tif.getBitPerSamples() << std::endl;
    std::cout << "spp : " << tif.getSamplesPerPixels() << std::endl;

    size_t nb_bit = tif.getImageWidth() * tif.getImageHeight() * tif.getSamplesPerPixels() * tif.getBitPerSamples();
    std::cout << "Total bit count : " << nb_bit << std::endl;
    std::cout << "Total byte count : " << nb_bit / 8 << std::endl;

    WebPImage img = WebPImage::fromTiff(tif);
    img.save("../test.webp");
    return 0;
}
