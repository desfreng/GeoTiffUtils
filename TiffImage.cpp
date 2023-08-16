#include "TiffImage.h"
#include "tiffiop.h"

/*
 * Code taken from the GeoTIFF library & TIFF documentation
 */
#define TIFF_TAG_GEO_PIXEL_SCALE       33550
#define TIFF_TAG_GEO_TIE_POINTS        33922
#define TIFF_TAG_GEO_KEY_DIRECTORY     34735
#define TIFF_TAG_GEO_DOUBLE_PARAMS     34736
#define TIFF_TAG_GEO_ASCII_PARAMS      34737

static char t1[] = "GeoPixelScale";
static char t2[] = "Intergraph TransformationMatrix";
static char t3[] = "GeoTransformationMatrix";
static char t4[] = "GeoTiePoints";
static char t5[] = "GeoKeyDirectory";
static char t6[] = "GeoDoubleParams";
static char t7[] = "GeoASCIIParams";

static const TIFFFieldInfo geoTiffFieldInfo[] = {
        {TIFF_TAG_GEO_PIXEL_SCALE,   TIFF_VARIABLE, TIFF_VARIABLE, TIFF_DOUBLE, FIELD_CUSTOM, TRUE, TRUE,  t1},
        {TIFF_TAG_GEO_TIE_POINTS,    TIFF_VARIABLE, TIFF_VARIABLE, TIFF_DOUBLE, FIELD_CUSTOM, TRUE, TRUE,  t4},
        {TIFF_TAG_GEO_KEY_DIRECTORY, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SHORT,  FIELD_CUSTOM, TRUE, TRUE,  t5},
        {TIFF_TAG_GEO_DOUBLE_PARAMS, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_DOUBLE, FIELD_CUSTOM, TRUE, TRUE,  t6},
        {TIFF_TAG_GEO_ASCII_PARAMS,  TIFF_VARIABLE, TIFF_VARIABLE, TIFF_ASCII,  FIELD_CUSTOM, TRUE, FALSE, t7},
};

#define N(a) (sizeof (a) / sizeof (a[0]))

static void GeoTiffDefaultDirectory(TIFF *tif) {
    TIFFMergeFieldInfo(tif, geoTiffFieldInfo, N(geoTiffFieldInfo));
}

static void initTiffTags() {
    static bool init_done = false;

    if (!init_done) {
        TIFFSetTagExtender(GeoTiffDefaultDirectory);
        init_done = true;
    }
}
// End of code from the GeoTIFF library

/*
 * Code taken from libwebp
 */

typedef struct {
    const uint8_t *data;
    toff_t size;
    toff_t pos;
} MyData;

static int MyClose(thandle_t opaque) {
    (void) opaque;
    return 0;
}

static toff_t MySize(thandle_t opaque) {
    const MyData *const my_data = (MyData *) opaque;
    return my_data->size;
}

static toff_t MySeek(thandle_t opaque, toff_t offset, int whence) {
    auto *const my_data = (MyData *) opaque;
    offset += (whence == SEEK_CUR) ? my_data->pos
                                   : (whence == SEEK_SET) ? 0
                                                          : my_data->size;
    if (offset > my_data->size) return (toff_t) -1;
    my_data->pos = offset;
    return offset;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-non-const-parameter"

static int MyMapFile(thandle_t opaque, void **base, toff_t *size) {
    (void) opaque;
    (void) base;
    (void) size;
    return 0;
}

#pragma clang diagnostic pop

static void MyUnmapFile(thandle_t opaque, void *base, toff_t size) {
    (void) opaque;
    (void) base;
    (void) size;
}

static tsize_t MyRead(thandle_t opaque, void *dst, tsize_t size) {
    auto *const my_data = (MyData *) opaque;
    if (my_data->pos + size > my_data->size) {
        size = (tsize_t) (my_data->size - my_data->pos);
    }
    if (size > 0) {
        memcpy(dst, my_data->data + my_data->pos, size);
        my_data->pos += size;
    }
    return size;
}
// end of code taken from libwebp


struct TiffImage::MyTIFF : public TIFF {
};

TiffImage::TiffImage(TiffImage::MyTIFF *imgPtr) : tif(imgPtr, (void (*)(MyTIFF *)) (&TIFFClose)) {}

TiffImage TiffImage::openTiff(const fs::path &p) {
    initTiffTags();
    auto *img = (MyTIFF *) (TIFFOpen(p.string().c_str(), "r"));
    return TiffImage(img);
}

TiffImage TiffImage::openTiffFromMemory(const std::vector<uint8_t> &data) {
    initTiffTags();

    MyData my_data = {data.data(), data.size(), 0};
    auto *tif = (MyTIFF *) TIFFClientOpen("Memory", "r", &my_data, MyRead, MyRead, MySeek, MyClose,
                                          MySize, MyMapFile, MyUnmapFile);
    return TiffImage(tif);
}

uint16_t TiffImage::getBitPerSamples() const {
    uint16_t bps;
    if (TIFFGetField(tif.get(), TIFFTAG_BITSPERSAMPLE, &bps) != 1) {
        throw std::runtime_error("Unable to read bits per samples");
    }
    return bps;
}

uint16_t TiffImage::getSamplesPerPixels() const {
    uint16_t spp;
    if (TIFFGetField(tif.get(), TIFFTAG_SAMPLESPERPIXEL, &spp) != 1) {
        throw std::runtime_error("Unable to read samples per pixel");
    }
    return spp;
}

uint32_t TiffImage::getImageWidth() const {
    uint32_t w;
    if (TIFFGetField(tif.get(), TIFFTAG_IMAGEWIDTH, &w) != 1) {
        throw std::runtime_error("Unable to read image width.");
    }
    return w;
}

uint32_t TiffImage::getImageHeight() const {
    uint32_t h;
    if (TIFFGetField(tif.get(), TIFFTAG_IMAGELENGTH, &h) != 1) {
        throw std::runtime_error("Unable to read image height.");
    }
    return h;
}

uint32_t TiffImage::getTileWidth() const {
    uint32_t w;
    if (TIFFGetField(tif.get(), TIFFTAG_TILEWIDTH, &w) != 1) {
        throw std::runtime_error("Unable to read tile width.");
    }
    return w;
}

uint32_t TiffImage::getTileHeight() const {
    uint32_t h;
    if (TIFFGetField(tif.get(), TIFFTAG_TILELENGTH, &h) != 1) {
        throw std::runtime_error("Unable to read tile height.");
    }
    return h;
}

PixelScale TiffImage::getPixelScale() const {
    uint16_t value_count;
    PixelScale *raw_data;

    if (TIFFGetField(tif.get(), TIFF_TAG_GEO_PIXEL_SCALE, &value_count, &raw_data) != 1) {
        throw std::runtime_error("Unable to read Pixel Scale.");
    }

    return *raw_data;
}

std::vector<TiePoint> TiffImage::getTiePoints() const {
    uint16_t value_count;
    double *raw_data;
    if (TIFFGetField(tif.get(), TIFF_TAG_GEO_TIE_POINTS, &value_count, &raw_data) != 1 || value_count % 6 != 0) {
        throw std::runtime_error("Unable to read TiePoints");
    }
    std::vector<TiePoint> res;

    for (int i = 0; i < value_count / 6; ++i) {
        res.push_back(*((TiePoint *) raw_data + (6 * i)));
    }
    return res;
}

std::vector<unsigned short> TiffImage::getGeoKeys() const {
    uint16_t value_count;
    unsigned short *raw_data;

    if (TIFFGetField(tif.get(), TIFF_TAG_GEO_KEY_DIRECTORY, &value_count, &raw_data) != 1) {
        throw std::runtime_error("Unable to read GeoKey Directory");
    }

    return {raw_data, raw_data + value_count};
}

std::vector<std::string> TiffImage::getAsciiParams() const {
    char *raw_data;

    if (TIFFGetField(tif.get(), TIFF_TAG_GEO_ASCII_PARAMS, &raw_data) != 1) {
        throw std::runtime_error("Unable to read ascii params");
    }
    std::vector<std::string> res;

    size_t lastIndex = 0;
    size_t index = 0;

    while (raw_data[index] != 0) {
        if (raw_data[index] == '|') {
            res.emplace_back(raw_data + lastIndex, raw_data + index);
            lastIndex = index + 1;
        }
        index++;
    }

    return res;
}

std::vector<double> TiffImage::getDoubleParams() const {
    uint16_t value_count;
    double *raw_data;

    if (TIFFGetField(tif.get(), TIFF_TAG_GEO_DOUBLE_PARAMS, &value_count, &raw_data) != 1) {
        throw std::runtime_error("Unable to read Double Params");
    }

    return {raw_data, raw_data + value_count};
}

std::vector<uint16_t> TiffImage::getExtraSamples() const {
    if (getSamplesPerPixels() > 3) {
        uint16_t extra_samples = 0;
        uint16_t *extra_samples_ptr = nullptr;

        if (TIFFGetField(tif.get(), TIFFTAG_EXTRASAMPLES, &extra_samples, &extra_samples_ptr) != 1) {
            throw std::runtime_error("Unable to retrieve TIFF ExtraSamples info");
        }

        return {extra_samples_ptr, extra_samples_ptr + extra_samples};
    } else {
        return {};
    }
}

uint16_t TiffImage::getNumberOfDirectory() const {
    return TIFFNumberOfDirectories(tif.get());
}

void TiffImage::readData(std::vector<uint32_t> &raster) const {
    uint64_t image_width = getImageWidth();
    uint32_t image_height = getImageHeight();
    raster.resize(image_width * image_height);

    if (TIFFReadRGBAImageOriented(tif.get(), image_width, image_height, raster.data(),
                                  ORIENTATION_TOPLEFT, 1) != 1) {
        throw std::runtime_error("Unable to read TIFF image data");
    }
}
