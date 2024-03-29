#ifndef GEO_TIFF_UTILS_WEBP_IMAGE_H
#define GEO_TIFF_UTILS_WEBP_IMAGE_H

#include "TiffImage.h"

class WebPImage {
public:

    static WebPImage fromTiff(const TiffImage &img);

    void save(const fs::path &file) const;

private:
    struct MyWebPPicture;
    std::unique_ptr<MyWebPPicture, void(*)(MyWebPPicture*)> pic;

    explicit WebPImage();
};


#endif //GEO_TIFF_UTILS_WEBP_IMAGE_H
