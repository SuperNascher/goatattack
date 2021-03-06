#ifndef FONT_HPP
#define FONT_HPP

#include "Exception.hpp"
#include "Properties.hpp"
#include "Subsystem.hpp"
#include "Tile.hpp"
#include "ZipReader.hpp"

class FontException : public Exception {
public:
    FontException(const char *msg) : Exception(msg) { }
    FontException(const std::string& msg) : Exception(msg) { }
};

static const int FontMin = 32;
static const int FontMax = 128;
static const int NumOfChars = FontMax - FontMin;

typedef struct font_char {
    int32_t origin_x;
    int32_t origin_y;
    int32_t width;
    int32_t height;
} font_char_t;

class Font : public Properties {
private:
    Font(const Font&);
    Font& operator=(const Font&);

public:
    Font(Subsystem& subsystem, const std::string& filename, ZipReader *zip = 0)
        throw (KeyValueException, FontException);
    virtual ~Font();

    Tile *get_tile(int index);
    int get_fw(int index);
    int get_spacing();
    int get_font_height();
    int get_text_width(const std::string& text);
    int get_char_width(unsigned char c);

private:
    struct FontOperations {
        FontOperations() : handle(0), data(0), ptr(0), size(0), open(0), read(0), close(0) { }
        virtual ~FontOperations() {
            if (close) {
                close(*this);
            }
        }

        void *handle;
        const char *data;
        const char *ptr;
        size_t size;

        void (*open)(FontOperations& op, const std::string& filename) throw (FontException);
        void (*read)(FontOperations& op, void *data, size_t sz);
        void (*close)(FontOperations& op);
    };

    Subsystem& subsystem;

    int    offset;
    Tile *tiles[NumOfChars];
    int fw[NumOfChars];
    int fh[NumOfChars];
    int32_t font_height;
    int spacing;

    static void fo_file_open(FontOperations& op, const std::string& filename) throw (FontException);
    static void fo_file_read(FontOperations& op, void *data, size_t sz);
    static void fo_file_close(FontOperations& op);

    static void fo_zip_open(FontOperations& op, const std::string& filename) throw (FontException);
    static void fo_zip_read(FontOperations& op, void *data, size_t sz);
    static void fo_zip_close(FontOperations& op);
};

#endif // FONT_HPP
