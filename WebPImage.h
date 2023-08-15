#ifndef GEO_TIFF_UTILS_WEBP_IMAGE_H
#define GEO_TIFF_UTILS_WEBP_IMAGE_H

#include "TiffImage.h"
#include "webp/encode.h"

class WebPImage {
public:

    static WebPImage fromTiff(const TiffImage &img);

    void save(const fs::path &file) const;

private:
    struct MyWebPPicture;
    using webp_ptr = std::unique_ptr<MyWebPPicture, void(*)(MyWebPPicture*)>;
    webp_ptr pic;

    explicit WebPImage();
};


#endif //GEO_TIFF_UTILS_WEBP_IMAGE_H
