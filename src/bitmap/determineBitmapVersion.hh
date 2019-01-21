//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Usage notes
// (you can find implementer's not at the bottom of this file).
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "puffin/bitmap.hh"
#include "puffin/exceptions.hh"
#include "puffin/experimental/bitfield.hh"
#include "puffin/rgba_bitmask.hh"
#include "puffin/chunk_layout.hh"

#include <fstream>
#include <iomanip>
#include <vector>

namespace puffin { namespace impl {

std::set<BitmapVersion> determineBitmapVersion(std::istream &f) {
        // "The FileType field of the file header is where we start. If
        //  these two byte values are 424Dh ("BM"), then you have a
        //  single-image BMP file that may have been created under
        //  Windows or OS/2. If FileType is the value 4142h ("BA"), then
        //  you have an OS/2 bitmap array file. Other OS/2 BMP
        //  variations have the file extensions .ICO and .PTR.
        //
        //  If your file type is "BM", then you must now read the Size
        //  field of the bitmap header to determine the version of the
        //  file. Size will be 12 for Windows 2.x BMP and OS/2 1.x BMP,
        //  40 for Windows 3.x and Windows NT BMP, 12 to 64 for OS/2 2.x
        //  BMP, and 108 for Windows 4.x BMP. A Windows NT BMP file will
        //  always have a Compression value of 3; otherwise, it is read
        //  as a Windows 3.x BMP file.
        //
        //  Note that the only difference between Windows 2.x BMP and
        //  OS/2 1.x BMP is the data type of the Width and Height
        //  fields. For Windows 2.x, they are signed shorts and for
        //  OS/2 1.x, they are unsigned shorts. Windows 3.x, Windows NT,
        //  and OS/2 2.x BMP files only vary in the number of fields in
        //  the bitmap header and in the interpretation of the
        //  Compression field."

        const std::ostream::pos_type startPos = f.tellg();
        f.seekg(0, std::ios_base::beg);

        std::set<BitmapVersion> ret;

        const uint16_t signature = impl::read_uint16_be(f);

        switch (signature) {
                // ---------------------------------------------------------------------
        case 0x0000: {
                ret.insert(BMPv_Win_1x);
                break;
        }
                // -- 'BM' -------------------------------------------------------------
        case 0x424d: {
                f.seekg(14, std::ios_base::beg);
                const uint32_t infoHeaderSize = impl::read_uint32_le(f);

                switch (infoHeaderSize) {
                        // -- [Windows 2.x] or [OS/2 1.x]   --  --  --  --  --  --  --
                case 12: {
                        // signed width/height for Win 2.x, unsigned for OS2 1.x
                        ret.insert(BMPv_Win_2x);
                        ret.insert(BMPv_OS2_1x);
                        break;
                }
                        // -- [Windows 3.x] or [Windows NT] --  --  --  --  --  --  --
                case 40: {
                        f.seekg(12, std::ios_base::cur);
                        const uint32_t compression =
                                static_cast<BitmapCompression>(
                                        impl::read_uint32_le(f));
                        ret.insert(BMPv_Win_3x);
                        // It can only be NT if compression is BITFIELDS:
                        if (compression == BI_BITFIELDS)
                                ret.insert(BMPv_WinNT);
                        break;
                }
                        // -- [Windows 4.x] --  --  --  --  --  --  --  --  --  --  --
                case 108: {
                        //
                        ret.insert(BMPv_Win_4x);
                        break;
                }
                        // -- [OS/2 2.x] -  --  --  --  --  --  --  --  --  --  --  --
                default:
                        if (infoHeaderSize >= 12 && infoHeaderSize <= 64) {
                                ret.insert(BMPv_OS2_2x);
                        }
                        break;
                };
                break;
        }
                // -- 'BA' -------------------------------------------------------------
        case 0x4241: {
                // OS/2 Bitmap Array
                throw std::logic_error("not implemented");
                break;
        }
        };
        if (ret.size() == 0)
                ret.insert(BMPv_Unknown);
        f.seekg(startPos);
        return ret;
}

} }
