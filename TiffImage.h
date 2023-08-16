#ifndef GEO_TIFF_UTILS_TEST_H
#define GEO_TIFF_UTILS_TEST_H

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class WebPImage;

struct PixelScale {
    double x;
    double y;
    double z;
};

struct TiePoint {
    double imgX;
    double imgY;
    double imgZ;
    double coordX;
    double coordY;
    double coordZ;
};

class TiffImage {
public:
    static TiffImage openTiff(const fs::path &p);

    static TiffImage openTiffFromMemory(const std::vector<uint8_t> &data);

    [[nodiscard]] uint16_t getBitPerSamples() const;

    [[nodiscard]] uint16_t getSamplesPerPixels() const;

    [[nodiscard]] uint32_t getImageWidth() const;

    [[nodiscard]] uint32_t getImageHeight() const;

    [[nodiscard]] uint32_t getTileWidth() const;

    [[nodiscard]] uint32_t getTileHeight() const;

    [[nodiscard]] PixelScale getPixelScale() const;

    [[nodiscard]] std::vector<TiePoint> getTiePoints() const;

    [[nodiscard]] std::vector<unsigned short> getGeoKeys() const;

    [[nodiscard]] std::vector<std::string> getAsciiParams() const;

    [[nodiscard]] std::vector<double> getDoubleParams() const;

    [[nodiscard]] std::vector<uint16_t> getExtraSamples() const;

    [[nodiscard]] uint16_t getNumberOfDirectory() const;

    void readData(std::vector<uint32_t> &raster) const;

private:
    struct MyTIFF;
    std::unique_ptr<MyTIFF, void (*)(MyTIFF *)> tif;

    explicit TiffImage(MyTIFF *imgPtr);
};


#endif //GEO_TIFF_UTILS_TEST_H
