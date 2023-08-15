#include <fstream>
#include "WebPImage.h"
#include "webp/encode.h"
#include "tiffio.h"

/*
 * Code taken and inspired from libWebP
 */

// Un-multiply Argb data. Taken from dsp/alpha_processing
// (we don't want to force a dependency to a libdspdec library).

#define M_FIX 24    // 24bit fixed-point arithmetic
#define HALF ((1u << M_FIX) >> 1)

static uint32_t unmult(uint8_t x, uint32_t mult) {
    const uint32_t v = (x * mult + HALF) >> M_FIX;
    return (v > 255u) ? 255u : v;
}

static inline uint32_t GetScale(uint32_t a) {
    return (255u << M_FIX) / a;
}

#undef M_FIX
#undef HALF

static void MultARGBRow(uint8_t *ptr, uint32_t width) {
    int x;
    for (x = 0; x < width; ++x, ptr += 4) {
        const uint32_t alpha = ptr[3];
        if (alpha < 255) {
            if (alpha == 0) {   // alpha == 0
                ptr[0] = ptr[1] = ptr[2] = 0;
            } else {
                const uint32_t scale = GetScale(alpha);
                ptr[0] = unmult(ptr[0], scale);
                ptr[1] = unmult(ptr[1], scale);
                ptr[2] = unmult(ptr[2], scale);
            }
        }
    }
}

int ImgIoUtilCheckSizeArgumentsOverflow(uint64_t stride, size_t height) {
    const uint64_t total_size = stride * height;
    bool ok = (total_size == (size_t) total_size) && ((uint64_t) (int) stride == stride);
#if defined(WEBP_MAX_IMAGE_SIZE)
    ok = ok && (total_size <= (uint64_t)WEBP_MAX_IMAGE_SIZE);
#endif
    return ok;
}


struct WebPImage::MyWebPPicture : public WebPPicture {
};


WebPImage::WebPImage() : pic(new MyWebPPicture, [](MyWebPPicture *p) {
    WebPPictureFree(p);
    delete p;
}) {
    WebPPictureInit(pic.get());
}

WebPImage WebPImage::fromTiff(const TiffImage &tif) {
    typedef uint32_t raster;

    uint16_t samples_per_px = tif.getSamplesPerPixels();
    if (!(samples_per_px == 1 || samples_per_px == 3 || samples_per_px == 4)) {
        throw std::runtime_error("Unsupported TIFF image");
    }

    uint64_t image_width = tif.getImageWidth();
    uint32_t image_height = tif.getImageHeight();

    uint64_t stride = image_width * sizeof(raster);
    if (!ImgIoUtilCheckSizeArgumentsOverflow(stride, image_height)) {
        throw std::runtime_error("TIFF image dimension (" + std::to_string(image_width) + "x" +
                                 std::to_string(image_height) + ") is too large");
    }

    std::vector<uint16_t> extra_samples = tif.getExtraSamples();
    std::vector<uint32_t> raster_data;
    raster_data.resize(stride * image_height);

    if (TIFFReadRGBAImageOriented((TIFF *) tif.tif.get(), image_width, image_height, raster_data.data(),
                                  ORIENTATION_TOPLEFT, 1) != 1) {
        throw std::runtime_error("Unable to read TIFF image data");
    }

    WebPImage pic;
    pic.pic->use_argb = true;
    pic.pic->width = static_cast<int>(image_width);
    pic.pic->height = static_cast<int>(image_height);

    // TIFF data is ABGR
#ifdef WORDS_BIGENDIAN
    TIFFSwabArrayOfLong(raster_data.data(), image_width * image_height);
#endif
    // if we have an alpha channel, we must un-multiply from rgbA to RGBA
    if (extra_samples.size() == 1 && extra_samples[0] == EXTRASAMPLE_ASSOCALPHA) {
        uint32_t y;
        auto *tmp = (uint8_t *) raster_data.data();
        for (y = 0; y < image_height; ++y) {
            MultARGBRow(tmp, image_width);
            tmp += stride;
        }
    }

    WebPPictureImportRGBX(pic.pic.get(), (const uint8_t *) raster_data.data(), (int) stride);
    return pic;
}

void WebPImage::save(const fs::path &file) const {
    WebPConfig config;
    WebPConfigPreset(&config, WEBP_PRESET_DRAWING, 50);

    config.lossless = 1;

    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    pic->writer = WebPMemoryWrite;
    pic->custom_ptr = &writer;

    if (!WebPEncode(&config, pic.get())) {
        throw std::runtime_error("Unable to save file");
    }

    std::ofstream file_stream(file, std::ios::out | std::ios::binary);
    std::copy(writer.mem, writer.mem + writer.size, std::ostreambuf_iterator<char>(file_stream));
    file_stream.flush();
    file_stream.close();

    WebPMemoryWriterClear(&writer);
}

