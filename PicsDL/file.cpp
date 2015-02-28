/**
 * Copyright 2014 Clément Nardi
 *
 * This file is part of PicsDL.
 *
 * PicsDL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PicsDL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PicsDL.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "file.h"
#include <QDateTime>
#include <QStringList>
#include <QMap>
#include <QDebug>
#include "libexif/exif-loader.h"
#include <QElapsedTimer>
#include <QImageReader>
#include <QBuffer>
#include <QImage>
#include <QRgb>
#include <QTimeZone>
#include <utime.h>
#include <QDir>
#include "transfermanager.h"
#include "geotagger.h"

#ifdef _WIN32
#include "WPDInterface.h"
#include "wpdiodevice.h"
#endif

static QStringList pictureExtensions = QString("jpg,jpeg,dng,raf,bmp,cr2,crw,dcr,dib,erf,fpx,gif,jfif,mos,mrf,mrw,nef,orf,pcx,pef,png,psd,srf,tif,tiff,wdp,x3f,xps").split(",");
static QStringList JPEGExtensions = QString("jpg,jpeg").split(",");
static QStringList videoExtensions = QString("mov,avi,mts,m2ts,3g2,3gp,asf,m1v,m2t,m2v,m4p,mod,mp2,mp2v,mp4,mpe,mpeg,mpg,mpv,mpv2,qt,ts,tts,vob,wm,wmv").split(",");


/* copied from exif-entry.c */

#define ESL_NNNN { EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED }
#define ESL_OOOO { EXIF_SUPPORT_LEVEL_OPTIONAL, EXIF_SUPPORT_LEVEL_OPTIONAL, EXIF_SUPPORT_LEVEL_OPTIONAL, EXIF_SUPPORT_LEVEL_OPTIONAL }
#define ESL_MMMN { EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_NOT_RECORDED }
#define ESL_MMMM { EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_MANDATORY }
#define ESL_OMON { EXIF_SUPPORT_LEVEL_OPTIONAL, EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_OPTIONAL, EXIF_SUPPORT_LEVEL_NOT_RECORDED }
#define ESL_NNOO { EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_OPTIONAL, EXIF_SUPPORT_LEVEL_OPTIONAL }
#define ESL_NNMN { EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_NOT_RECORDED }
#define ESL_NNMM { EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_MANDATORY, EXIF_SUPPORT_LEVEL_MANDATORY }
#define ESL_NNNM { EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_MANDATORY }
#define ESL_NNNO { EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_NOT_RECORDED, EXIF_SUPPORT_LEVEL_OPTIONAL }
#define ESL_GPS { ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN }

static const struct TagEntry {
    /*! Tag ID. There may be duplicate tags when the same number is used for
     * different meanings in different IFDs. */
    int tag;
    const char *name;
    const char *title;
    const char *description;
    /*! indexed by the types [ExifIfd][ExifDataType] */
    ExifSupportLevel esl[EXIF_IFD_COUNT][EXIF_DATA_TYPE_COUNT];
} ExifTagTable[] = {
    {EXIF_TAG_GPS_VERSION_ID, "GPSVersionID", ("GPS Tag Version"),
     ("Indicates the version of <GPSInfoIFD>. The version is given "
        "as 2.0.0.0. This tag is mandatory when <GPSInfo> tag is "
        "present. (Note: The <GPSVersionID> tag is given in bytes, "
        "unlike the <ExifVersion> tag. When the version is "
        "2.0.0.0, the tag value is 02000000.H)."), ESL_GPS},
    {EXIF_TAG_INTEROPERABILITY_INDEX, "InteroperabilityIndex",
     ("Interoperability Index"),
     ("Indicates the identification of the Interoperability rule. "
        "Use \"R98\" for stating ExifR98 Rules. Four bytes used "
        "including the termination code (NULL). see the separate "
        "volume of Recommended Exif Interoperability Rules (ExifR98) "
        "for other tags used for ExifR98."),
     { ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_OOOO } },
    {EXIF_TAG_GPS_LATITUDE_REF, "GPSLatitudeRef", ("North or South Latitude"),
     ("Indicates whether the latitude is north or south latitude. The "
        "ASCII value 'N' indicates north latitude, and 'S' is south "
        "latitude."), ESL_GPS},
    {EXIF_TAG_INTEROPERABILITY_VERSION, "InteroperabilityVersion",
     ("Interoperability Version"), "",
     { ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_OOOO } },
    {EXIF_TAG_GPS_LATITUDE, "GPSLatitude", ("Latitude"),
     ("Indicates the latitude. The latitude is expressed as three "
        "RATIONAL values giving the degrees, minutes, and seconds, "
        "respectively. When degrees, minutes and seconds are expressed, "
        "the format is dd/1,mm/1,ss/1. When degrees and minutes are used "
        "and, for example, fractions of minutes are given up to two "
        "decimal places, the format is dd/1,mmmm/100,0/1."),
     ESL_GPS},
    {EXIF_TAG_GPS_LONGITUDE_REF, "GPSLongitudeRef", ("East or West Longitude"),
     ("Indicates whether the longitude is east or west longitude. "
        "ASCII 'E' indicates east longitude, and 'W' is west "
        "longitude."), ESL_GPS},
    {EXIF_TAG_GPS_LONGITUDE, "GPSLongitude", ("Longitude"),
     ("Indicates the longitude. The longitude is expressed as three "
        "RATIONAL values giving the degrees, minutes, and seconds, "
        "respectively. When degrees, minutes and seconds are expressed, "
        "the format is ddd/1,mm/1,ss/1. When degrees and minutes are "
        "used and, for example, fractions of minutes are given up to "
        "two decimal places, the format is ddd/1,mmmm/100,0/1."),
     ESL_GPS},
    {EXIF_TAG_GPS_ALTITUDE_REF, "GPSAltitudeRef", ("Altitude Reference"),
     ("Indicates the altitude used as the reference altitude. If the "
        "reference is sea level and the altitude is above sea level, 0 "
        "is given. If the altitude is below sea level, a value of 1 is given "
        "and the altitude is indicated as an absolute value in the "
        "GSPAltitude tag. The reference unit is meters. Note that this tag "
        "is BYTE type, unlike other reference tags."), ESL_GPS},
    {EXIF_TAG_GPS_ALTITUDE, "GPSAltitude", ("Altitude"),
     ("Indicates the altitude based on the reference in GPSAltitudeRef. "
        "Altitude is expressed as one RATIONAL value. The reference unit "
        "is meters."), ESL_GPS},
    {EXIF_TAG_GPS_TIME_STAMP, "GPSTimeStamp", ("GPS Time (Atomic Clock)"),
         ("Indicates the time as UTC (Coordinated Universal Time). "
        "TimeStamp is expressed as three RATIONAL values giving "
            "the hour, minute, and second."), ESL_GPS},
    {EXIF_TAG_GPS_SATELLITES, "GPSSatellites", ("GPS Satellites"),
         ("Indicates the GPS satellites used for measurements. This "
            "tag can be used to describe the number of satellites, their ID "
            "number, angle of elevation, azimuth, SNR and other information "
            "in ASCII notation. The format is not specified. If the GPS "
            "receiver is incapable of taking measurements, value of the tag "
            "shall be set to NULL."), ESL_GPS},
    {EXIF_TAG_GPS_STATUS, "GPSStatus", ("GPS Receiver Status"),
         ("Indicates the status of the GPS receiver when the image is "
            "recorded. 'A' means measurement is in progress, and 'V' means "
            "the measurement is Interoperability."), ESL_GPS},
    {EXIF_TAG_GPS_MEASURE_MODE, "GPSMeasureMode", ("GPS Measurement Mode"),
         ("Indicates the GPS measurement mode. '2' means "
            "two-dimensional measurement and '3' means three-dimensional "
            "measurement is in progress."), ESL_GPS},
    {EXIF_TAG_GPS_DOP, "GPSDOP", ("Measurement Precision"),
         ("Indicates the GPS DOP (data degree of precision). An HDOP "
            "value is written during two-dimensional measurement, and PDOP "
            "during three-dimensional measurement."), ESL_GPS},
    {EXIF_TAG_GPS_SPEED_REF, "GPSSpeedRef", ("Speed Unit"),
         ("Indicates the unit used to express the GPS receiver speed "
            "of movement. 'K', 'M' and 'N' represent kilometers per hour, "
            "miles per hour, and knots."), ESL_GPS},
    {EXIF_TAG_GPS_SPEED, "GPSSpeed", ("Speed of GPS Receiver"),
     ("Indicates the speed of GPS receiver movement."), ESL_GPS},
    {EXIF_TAG_GPS_TRACK_REF, "GPSTrackRef", ("Reference for direction of movement"),
         ("Indicates the reference for giving the direction of GPS "
            "receiver movement. 'T' denotes true direction and 'M' is "
            "magnetic direction."), ESL_GPS},
    {EXIF_TAG_GPS_TRACK, "GPSTrack", ("Direction of Movement"),
         ("Indicates the direction of GPS receiver movement. The range "
            "of values is from 0.00 to 359.99."), ESL_GPS},
    {EXIF_TAG_GPS_IMG_DIRECTION_REF, "GPSImgDirectionRef", ("GPS Image Direction Reference"),
     ("Indicates the reference for giving the direction of the image when it is captured. "
        "'T' denotes true direction and 'M' is magnetic direction."), ESL_GPS},
    {EXIF_TAG_GPS_IMG_DIRECTION, "GPSImgDirection", ("GPS Image Direction"),
     ("Indicates the direction of the image when it was captured. The range of values is "
        "from 0.00 to 359.99."), ESL_GPS},
    {EXIF_TAG_GPS_MAP_DATUM, "GPSMapDatum", ("Geodetic Survey Data Used"),
         ("Indicates the geodetic survey data used by the GPS "
            "receiver. If the survey data is restricted to Japan, the value "
            "of this tag is 'TOKYO' or 'WGS-84'. If a GPS Info tag is "
            "recorded, it is strongly recommended that this tag be recorded."), ESL_GPS},
    {EXIF_TAG_GPS_DEST_LATITUDE_REF, "GPSDestLatitudeRef", ("Reference For Latitude of Destination"),
         ("Indicates whether the latitude of the destination point is "
            "north or south latitude. The ASCII value 'N' indicates north "
            "latitude, and 'S' is south latitude."), ESL_GPS},
    {EXIF_TAG_GPS_DEST_LATITUDE, "GPSDestLatitude", ("Latitude of Destination"),
         ("Indicates the latitude of the destination point. The "
            "latitude is expressed as three RATIONAL values giving the "
            "degrees, minutes, and seconds, respectively. If latitude is "
            "expressed as degrees, minutes and seconds, a typical format "
            "would be dd/1,mm/1,ss/1. When degrees and minutes are used and, "
            "for example, fractions of minutes are given up to two decimal "
            "places, the format would be dd/1,mmmm/100,0/1."), ESL_GPS},
    {EXIF_TAG_GPS_DEST_LONGITUDE_REF, "GPSDestLongitudeRef", ("Reference for Longitude of Destination"),
         ("Indicates whether the longitude of the destination point is "
            "east or west longitude. ASCII 'E' indicates east longitude, and "
            "'W' is west longitude."), ESL_GPS},
    {EXIF_TAG_GPS_DEST_LONGITUDE, "GPSDestLongitude", ("Longitude of Destination"),
         ("Indicates the longitude of the destination point. The "
            "longitude is expressed as three RATIONAL values giving the "
            "degrees, minutes, and seconds, respectively. If longitude is "
            "expressed as degrees, minutes and seconds, a typical format "
            "would be ddd/1,mm/1,ss/1. When degrees and minutes are used "
            "and, for example, fractions of minutes are given up to two "
            "decimal places, the format would be ddd/1,mmmm/100,0/1."),
         ESL_GPS},
    {EXIF_TAG_GPS_DEST_BEARING_REF, "GPSDestBearingRef", ("Reference for Bearing of Destination"),
         ("Indicates the reference used for giving the bearing to "
            "the destination point. 'T' denotes true direction and 'M' is "
            "magnetic direction."), ESL_GPS},
    {EXIF_TAG_GPS_DEST_BEARING, "GPSDestBearing", ("Bearing of Destination"),
         ("Indicates the bearing to the destination point. The range "
            "of values is from 0.00 to 359.99."), ESL_GPS},
    {EXIF_TAG_GPS_DEST_DISTANCE_REF, "GPSDestDistanceRef", ("Reference for Distance to Destination"),
         ("Indicates the unit used to express the distance to the "
            "destination point. 'K', 'M' and 'N' represent kilometers, miles "
            "and nautical miles."), ESL_GPS},
    {EXIF_TAG_GPS_DEST_DISTANCE, "GPSDestDistance", ("Distance to Destination"),
     ("Indicates the distance to the destination point."), ESL_GPS},
    {EXIF_TAG_GPS_PROCESSING_METHOD, "GPSProcessingMethod", ("Name of GPS Processing Method"),
         ("A character string recording the name of the method used "
            "for location finding. The first byte indicates the character "
            "code used, and this is followed by the name "
            "of the method. Since the Type is not ASCII, NULL termination is "
            "not necessary."), ESL_GPS},
    {EXIF_TAG_GPS_AREA_INFORMATION, "GPSAreaInformation", ("Name of GPS Area"),
         ("A character string recording the name of the GPS area. The "
            "first byte indicates the character code used, "
            "and this is followed by the name of the GPS area. Since "
            "the Type is not ASCII, NULL termination is not necessary."), ESL_GPS},
    {EXIF_TAG_GPS_DATE_STAMP, "GPSDateStamp", ("GPS Date"),
         ("A character string recording date and time information "
            "relative to UTC (Coordinated Universal Time). The format is "
            "\"YYYY:MM:DD\". The length of the string is 11 bytes including "
            "NULL."), ESL_GPS},
    {EXIF_TAG_GPS_DIFFERENTIAL, "GPSDifferential", ("GPS Differential Correction"),
         ("Indicates whether differential correction is applied to the "
            "GPS receiver."), ESL_GPS},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_NEW_SUBFILE_TYPE, "NewSubfileType",
     ("New Subfile Type"), ("A general indication of the kind of data "
        "contained in this subfile.")},
    {EXIF_TAG_IMAGE_WIDTH, "ImageWidth", ("Image Width"),
     ("The number of columns of image data, equal to the number of "
        "pixels per row. In JPEG compressed data a JPEG marker is "
        "used instead of this tag."),
     { ESL_MMMN, ESL_MMMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_IMAGE_LENGTH, "ImageLength", ("Image Length"),
     ("The number of rows of image data. In JPEG compressed data a "
        "JPEG marker is used instead of this tag."),
     { ESL_MMMN, ESL_MMMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_BITS_PER_SAMPLE, "BitsPerSample", ("Bits per Sample"),
     ("The number of bits per image component. In this standard each "
        "component of the image is 8 bits, so the value for this "
        "tag is 8. See also <SamplesPerPixel>. In JPEG compressed data "
        "a JPEG marker is used instead of this tag."),
     { ESL_MMMN, ESL_MMMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_COMPRESSION, "Compression", ("Compression"),
     ("The compression scheme used for the image data. When a "
        "primary image is JPEG compressed, this designation is "
        "not necessary and is omitted. When thumbnails use JPEG "
        "compression, this tag value is set to 6."),
     { ESL_MMMN, ESL_MMMM, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_PHOTOMETRIC_INTERPRETATION, "PhotometricInterpretation",
     ("Photometric Interpretation"),
     ("The pixel composition. In JPEG compressed data a JPEG "
        "marker is used instead of this tag."),
     { ESL_MMMN, ESL_MMMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 */
    {EXIF_TAG_FILL_ORDER, "FillOrder", ("Fill Order"), ""},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_DOCUMENT_NAME, "DocumentName", ("Document Name"), ""},
    {EXIF_TAG_IMAGE_DESCRIPTION, "ImageDescription",
     ("Image Description"),
     ("A character string giving the title of the image. It may be "
        "a comment such as \"1988 company picnic\" or "
        "the like. Two-bytes character codes cannot be used. "
        "When a 2-bytes code is necessary, the Exif Private tag "
        "<UserComment> is to be used."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_MAKE, "Make", ("Manufacturer"),
     ("The manufacturer of the recording "
        "equipment. This is the manufacturer of the DSC, scanner, "
        "video digitizer or other equipment that generated the "
        "image. When the field is left blank, it is treated as "
        "unknown."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_MODEL, "Model", ("Model"),
     ("The model name or model number of the equipment. This is the "
        "model name or number of the DSC, scanner, video digitizer "
        "or other equipment that generated the image. When the field "
        "is left blank, it is treated as unknown."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_STRIP_OFFSETS, "StripOffsets", ("Strip Offsets"),
     ("For each strip, the byte offset of that strip. It is "
        "recommended that this be selected so the number of strip "
        "bytes does not exceed 64 Kbytes. With JPEG compressed "
        "data this designation is not needed and is omitted. See also "
        "<RowsPerStrip> and <StripByteCounts>."),
     { ESL_MMMN, ESL_MMMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_ORIENTATION, "Orientation", ("Orientation"),
     ("The image orientation viewed in terms of rows and columns."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SAMPLES_PER_PIXEL, "SamplesPerPixel",
     ("Samples per Pixel"),
     ("The number of components per pixel. Since this standard applies "
        "to RGB and YCbCr images, the value set for this tag is 3. "
        "In JPEG compressed data a JPEG marker is used instead of this "
        "tag."),
     { ESL_MMMN, ESL_MMMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_ROWS_PER_STRIP, "RowsPerStrip", ("Rows per Strip"),
     ("The number of rows per strip. This is the number of rows "
        "in the image of one strip when an image is divided into "
        "strips. With JPEG compressed data this designation is not "
        "needed and is omitted. See also <StripOffsets> and "
        "<StripByteCounts>."),
     { ESL_MMMN, ESL_MMMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_STRIP_BYTE_COUNTS, "StripByteCounts", ("Strip Byte Count"),
     ("The total number of bytes in each strip. With JPEG compressed "
        "data this designation is not needed and is omitted."),
     { ESL_MMMN, ESL_MMMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_X_RESOLUTION, "XResolution", ("X-Resolution"),
     ("The number of pixels per <ResolutionUnit> in the <ImageWidth> "
        "direction. When the image resolution is unknown, 72 [dpi] "
        "is designated."),
     { ESL_MMMM, ESL_MMMM, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_Y_RESOLUTION, "YResolution", ("Y-Resolution"),
     ("The number of pixels per <ResolutionUnit> in the <ImageLength> "
        "direction. The same value as <XResolution> is designated."),
     { ESL_MMMM, ESL_MMMM, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_PLANAR_CONFIGURATION, "PlanarConfiguration",
     ("Planar Configuration"),
     ("Indicates whether pixel components are recorded in a chunky "
        "or planar format. In JPEG compressed files a JPEG marker "
        "is used instead of this tag. If this field does not exist, "
        "the TIFF default of 1 (chunky) is assumed."),
     { ESL_OMON, ESL_OMON, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_RESOLUTION_UNIT, "ResolutionUnit", ("Resolution Unit"),
     ("The unit for measuring <XResolution> and <YResolution>. The same "
        "unit is used for both <XResolution> and <YResolution>. If "
        "the image resolution is unknown, 2 (inches) is designated."),
     { ESL_MMMM, ESL_MMMM, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_TRANSFER_FUNCTION, "TransferFunction",
     ("Transfer Function"),
     ("A transfer function for the image, described in tabular style. "
        "Normally this tag is not necessary, since color space is "
        "specified in the color space information tag (<ColorSpace>)."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SOFTWARE, "Software", ("Software"),
     ("This tag records the name and version of the software or "
        "firmware of the camera or image input device used to "
        "generate the image. The detailed format is not specified, but "
        "it is recommended that the example shown below be "
        "followed. When the field is left blank, it is treated as "
        "unknown."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_DATE_TIME, "DateTime", ("Date and Time"),
     ("The date and time of image creation. In this standard "
        "(EXIF-2.1) it is the date and time the file was changed."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_ARTIST, "Artist", ("Artist"),
     ("This tag records the name of the camera owner, photographer or "
        "image creator. The detailed format is not specified, but it is "
        "recommended that the information be written as in the example "
        "below for ease of Interoperability. When the field is "
        "left blank, it is treated as unknown."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_WHITE_POINT, "WhitePoint", ("White Point"),
     ("The chromaticity of the white point of the image. Normally "
        "this tag is not necessary, since color space is specified "
        "in the color space information tag (<ColorSpace>)."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_PRIMARY_CHROMATICITIES, "PrimaryChromaticities",
     ("Primary Chromaticities"),
     ("The chromaticity of the three primary colors of the image. "
        "Normally this tag is not necessary, since color space is "
        "specified in the color space information tag (<ColorSpace>)."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 */
    {EXIF_TAG_SUB_IFDS, "SubIFDs", "SubIFD Offsets", ("Defined by Adobe Corporation "
        "to enable TIFF Trees within a TIFF file.")},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_TRANSFER_RANGE, "TransferRange", ("Transfer Range"), ""},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_JPEG_PROC, "JPEGProc", "JPEGProc", ""},
    {EXIF_TAG_JPEG_INTERCHANGE_FORMAT, "JPEGInterchangeFormat",
     ("JPEG Interchange Format"),
     ("The offset to the start byte (SOI) of JPEG compressed "
        "thumbnail data. This is not used for primary image "
        "JPEG data."),
     { ESL_NNNN, ESL_NNNM, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH,
     "JPEGInterchangeFormatLength", ("JPEG Interchange Format Length"),
     ("The number of bytes of JPEG compressed thumbnail data. This "
        "is not used for primary image JPEG data. JPEG thumbnails "
        "are not divided but are recorded as a continuous JPEG "
        "bitstream from SOI to EOI. Appn and COM markers should "
        "not be recorded. Compressed thumbnails must be recorded in no "
        "more than 64 Kbytes, including all other data to be "
        "recorded in APP1."),
     { ESL_NNNN, ESL_NNNM, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_YCBCR_COEFFICIENTS, "YCbCrCoefficients",
     ("YCbCr Coefficients"),
     ("The matrix coefficients for transformation from RGB to YCbCr "
        "image data. No default is given in TIFF; but here the "
        "value given in \"Color Space Guidelines\", is used "
        "as the default. The color space is declared in a "
        "color space information tag, with the default being the value "
        "that gives the optimal image characteristics "
        "Interoperability this condition."),
     { ESL_NNOO, ESL_NNOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_YCBCR_SUB_SAMPLING, "YCbCrSubSampling",
     ("YCbCr Sub-Sampling"),
     ("The sampling ratio of chrominance components in relation to the "
        "luminance component. In JPEG compressed data a JPEG marker "
        "is used instead of this tag."),
     { ESL_NNMN, ESL_NNMN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_YCBCR_POSITIONING, "YCbCrPositioning",
     ("YCbCr Positioning"),
     ("The position of chrominance components in relation to the "
        "luminance component. This field is designated only for "
        "JPEG compressed data or uncompressed YCbCr data. The TIFF "
        "default is 1 (centered); but when Y:Cb:Cr = 4:2:2 it is "
        "recommended in this standard that 2 (co-sited) be used to "
        "record data, in order to improve the image quality when viewed "
        "on TV systems. When this field does not exist, the reader shall "
        "assume the TIFF default. In the case of Y:Cb:Cr = 4:2:0, the "
        "TIFF default (centered) is recommended. If the reader "
        "does not have the capability of supporting both kinds of "
        "<YCbCrPositioning>, it shall follow the TIFF default regardless "
        "of the value in this field. It is preferable that readers "
        "be able to support both centered and co-sited positioning."),
     { ESL_NNMM, ESL_NNOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_REFERENCE_BLACK_WHITE, "ReferenceBlackWhite",
     ("Reference Black/White"),
     ("The reference black point value and reference white point "
        "value. No defaults are given in TIFF, but the values "
        "below are given as defaults here. The color space is declared "
        "in a color space information tag, with the default "
        "being the value that gives the optimal image characteristics "
        "Interoperability these conditions."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 */
    {EXIF_TAG_XML_PACKET, "XMLPacket", ("XML Packet"), ("XMP Metadata")},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_RELATED_IMAGE_FILE_FORMAT, "RelatedImageFileFormat",
     "RelatedImageFileFormat", ""},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_RELATED_IMAGE_WIDTH, "RelatedImageWidth",
     "RelatedImageWidth", ""},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_RELATED_IMAGE_LENGTH, "RelatedImageLength",
     "RelatedImageLength", ""},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_CFA_REPEAT_PATTERN_DIM, "CFARepeatPatternDim",
     "CFARepeatPatternDim", ""},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_CFA_PATTERN, "CFAPattern",
     ("CFA Pattern"),
     ("Indicates the color filter array (CFA) geometric pattern of the "
        "image sensor when a one-chip color area sensor is used. "
        "It does not apply to all sensing methods.")},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_BATTERY_LEVEL, "BatteryLevel", ("Battery Level"), ""},
    {EXIF_TAG_COPYRIGHT, "Copyright", ("Copyright"),
     ("Copyright information. In this standard the tag is used to "
        "indicate both the photographer and editor copyrights. It is "
        "the copyright notice of the person or organization claiming "
        "rights to the image. The Interoperability copyright "
        "statement including date and rights should be written in this "
        "field; e.g., \"Copyright, John Smith, 19xx. All rights "
        "reserved.\". In this standard the field records both the "
        "photographer and editor copyrights, with each recorded in a "
        "separate part of the statement. When there is a clear "
        "distinction between the photographer and editor copyrights, "
        "these are to be written in the order of photographer followed "
        "by editor copyright, separated by NULL (in this case, "
        "since the statement also ends with a NULL, there are two NULL "
        "codes) (see example 1). When only the photographer is given, "
        "it is terminated by one NULL code (see example 2). When only "
        "the editor copyright is given, "
        "the photographer copyright part consists of one space followed "
        "by a terminating NULL code, then the editor copyright is given "
        "(see example 3). When the field is left blank, it is treated "
        "as unknown."),
     { ESL_OOOO, ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_EXPOSURE_TIME, "ExposureTime", ("Exposure Time"),
     ("Exposure time, given in seconds (sec)."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FNUMBER, "FNumber", ("F-Number"),
     ("The F number."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 */
    {EXIF_TAG_IPTC_NAA, "IPTC/NAA", "IPTC/NAA", ""},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_IMAGE_RESOURCES, "ImageResources", ("Image Resources Block"), ""},
    {EXIF_TAG_EXIF_IFD_POINTER, "ExifIfdPointer", "ExifIFDPointer",
     ("A pointer to the Exif IFD. Interoperability, Exif IFD has the "
        "same structure as that of the IFD specified in TIFF. "
        "ordinarily, however, it does not contain image data as in "
        "the case of TIFF."),
     { ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 */
    {EXIF_TAG_INTER_COLOR_PROFILE, "InterColorProfile",
     "InterColorProfile", ""},
    {EXIF_TAG_EXPOSURE_PROGRAM, "ExposureProgram", ("Exposure Program"),
     ("The class of the program used by the camera to set exposure "
        "when the picture is taken."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SPECTRAL_SENSITIVITY, "SpectralSensitivity",
     ("Spectral Sensitivity"),
     ("Indicates the spectral sensitivity of each channel of the "
        "camera used. The tag value is an ASCII string compatible "
        "with the standard developed by the ASTM Technical Committee."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_GPS_INFO_IFD_POINTER, "GPSInfoIFDPointer",
     ("GPS Info IFD Pointer"),
     ("A pointer to the GPS Info IFD. The "
        "Interoperability structure of the GPS Info IFD, like that of "
        "Exif IFD, has no image data."),
     { ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },

    {EXIF_TAG_ISO_SPEED_RATINGS, "ISOSpeedRatings",
     ("ISO Speed Ratings"),
     ("Indicates the ISO Speed and ISO Latitude of the camera or "
        "input device as specified in ISO 12232."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_OECF, "OECF", ("Opto-Electronic Conversion Function"),
     ("Indicates the Opto-Electronic Conversion Function (OECF) "
        "specified in ISO 14524. <OECF> is the relationship between "
        "the camera optical input and the image values."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 */
    {EXIF_TAG_TIME_ZONE_OFFSET, "TimeZoneOffset", ("Time Zone Offset"),
     ("Encodes time zone of camera clock relative to GMT.")},
    {EXIF_TAG_EXIF_VERSION, "ExifVersion", ("Exif Version"),
     ("The version of this standard supported. Nonexistence of this "
        "field is taken to mean nonconformance to the standard."),
     { ESL_NNNN, ESL_NNNN, ESL_MMMM, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_DATE_TIME_ORIGINAL, "DateTimeOriginal",
     ("Date and Time (Original)"),
     ("The date and time when the original image data was generated. "
        "For a digital still camera "
        "the date and time the picture was taken are recorded."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_DATE_TIME_DIGITIZED, "DateTimeDigitized",
     ("Date and Time (Digitized)"),
     ("The date and time when the image was stored as digital data."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_COMPONENTS_CONFIGURATION, "ComponentsConfiguration",
     ("Components Configuration"),
     ("Information specific to compressed data. The channels of "
        "each component are arranged in order from the 1st "
        "component to the 4th. For uncompressed data the data "
        "arrangement is given in the <PhotometricInterpretation> tag. "
        "However, since <PhotometricInterpretation> can only "
        "express the order of Y, Cb and Cr, this tag is provided "
        "for cases when compressed data uses components other than "
        "Y, Cb, and Cr and to enable support of other sequences."),
     { ESL_NNNN, ESL_NNNN, ESL_NNNM, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_COMPRESSED_BITS_PER_PIXEL, "CompressedBitsPerPixel",
     ("Compressed Bits per Pixel"),
     ("Information specific to compressed data. The compression mode "
        "used for a compressed image is indicated in unit bits "
        "per pixel."),
     { ESL_NNNN, ESL_NNNN, ESL_NNNO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SHUTTER_SPEED_VALUE, "ShutterSpeedValue", ("Shutter Speed"),
     ("Shutter speed. The unit is the APEX (Additive System of "
        "Photographic Exposure) setting."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_APERTURE_VALUE, "ApertureValue", ("Aperture"),
     ("The lens aperture. The unit is the APEX value."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_BRIGHTNESS_VALUE, "BrightnessValue", ("Brightness"),
     ("The value of brightness. The unit is the APEX value. "
        "Ordinarily it is given in the range of -99.99 to 99.99."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_EXPOSURE_BIAS_VALUE, "ExposureBiasValue",
     ("Exposure Bias"),
     ("The exposure bias. The units is the APEX value. Ordinarily "
        "it is given in the range of -99.99 to 99.99."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_MAX_APERTURE_VALUE, "MaxApertureValue", ("Maximum Aperture Value"),
     ("The smallest F number of the lens. The unit is the APEX value. "
        "Ordinarily it is given in the range of 00.00 to 99.99, "
        "but it is not limited to this range."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SUBJECT_DISTANCE, "SubjectDistance",
     ("Subject Distance"),
     ("The distance to the subject, given in meters."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_METERING_MODE, "MeteringMode", ("Metering Mode"),
     ("The metering mode."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_LIGHT_SOURCE, "LightSource", ("Light Source"),
     ("The kind of light source."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FLASH, "Flash", ("Flash"),
     ("This tag is recorded when an image is taken using a strobe "
        "light (flash)."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FOCAL_LENGTH, "FocalLength", ("Focal Length"),
     ("The actual focal length of the lens, in mm. Conversion is not "
        "made to the focal length of a 35 mm film camera."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SUBJECT_AREA, "SubjectArea", ("Subject Area"),
     ("This tag indicates the location and area of the main subject "
        "in the overall scene."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 */
    {EXIF_TAG_TIFF_EP_STANDARD_ID, "TIFF/EPStandardID", ("TIFF/EP Standard ID"), ""},
    {EXIF_TAG_MAKER_NOTE, "MakerNote", ("Maker Note"),
     ("A tag for manufacturers of Exif writers to record any desired "
        "information. The contents are up to the manufacturer."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_USER_COMMENT, "UserComment", ("User Comment"),
     ("A tag for Exif users to write keywords or comments on the image "
        "besides those in <ImageDescription>, and without the "
        "character code limitations of the <ImageDescription> tag. The "
        "character code used in the <UserComment> tag is identified "
        "based on an ID code in a fixed 8-byte area at the start of "
        "the tag data area. The unused portion of the area is padded "
        "with NULL (\"00.h\"). ID codes are assigned by means of "
        "registration. The designation method and references for each "
        "character code are defined in the specification. The value of "
        "CountN is determined based on the 8 bytes in the character code "
        "area and the number of bytes in the user comment part. Since "
        "the TYPE is not ASCII, NULL termination is not necessary. "
        "The ID code for the <UserComment> area may be a Defined code "
        "such as JIS or ASCII, or may be Undefined. The Undefined name "
        "is UndefinedText, and the ID code is filled with 8 bytes of all "
        "\"NULL\" (\"00.H\"). An Exif reader that reads the "
        "<UserComment> tag must have a function for determining the "
        "ID code. This function is not required in Exif readers that "
        "do not use the <UserComment> tag. "
        "When a <UserComment> area is set aside, it is recommended that "
        "the ID code be ASCII and that the following user comment "
        "part be filled with blank characters [20.H]."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SUB_SEC_TIME, "SubsecTime", ("Sub-second Time"),
     ("A tag used to record fractions of seconds for the "
        "<DateTime> tag."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SUB_SEC_TIME_ORIGINAL, "SubSecTimeOriginal",
     ("Sub-second Time (Original)"),
     ("A tag used to record fractions of seconds for the "
        "<DateTimeOriginal> tag."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SUB_SEC_TIME_DIGITIZED, "SubSecTimeDigitized",
     ("Sub-second Time (Digitized)"),
     ("A tag used to record fractions of seconds for the "
        "<DateTimeDigitized> tag."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 (Microsoft extension) */
    {EXIF_TAG_XP_TITLE, "XPTitle", ("XP Title"),
     ("A character string giving the title of the image, encoded in "
        "UTF-16LE."),
     { ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 (Microsoft extension) */
    {EXIF_TAG_XP_COMMENT, "XPComment", ("XP Comment"),
     ("A character string containing a comment about the image, encoded "
        "in UTF-16LE."),
     { ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 (Microsoft extension) */
    {EXIF_TAG_XP_AUTHOR, "XPAuthor", ("XP Author"),
     ("A character string containing the name of the image creator, "
        "encoded in UTF-16LE."),
     { ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 (Microsoft extension) */
    {EXIF_TAG_XP_KEYWORDS, "XPKeywords", ("XP Keywords"),
     ("A character string containing key words describing the image, "
        "encoded in UTF-16LE."),
     { ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 (Microsoft extension) */
    {EXIF_TAG_XP_SUBJECT, "XPSubject", ("XP Subject"),
     ("A character string giving the image subject, encoded in "
        "UTF-16LE."),
     { ESL_OOOO, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FLASH_PIX_VERSION, "FlashPixVersion", "FlashPixVersion",
     ("The FlashPix format version supported by a FPXR file."),
     { ESL_NNNN, ESL_NNNN, ESL_MMMM, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_COLOR_SPACE, "ColorSpace", ("Color Space"),
     ("The color space information tag is always "
        "recorded as the color space specifier. Normally sRGB (=1) "
        "is used to define the color space based on the PC monitor "
        "conditions and environment. If a color space other than "
        "sRGB is used, Uncalibrated (=FFFF.H) is set. Image data "
        "recorded as Uncalibrated can be treated as sRGB when it is "
        "converted to FlashPix."),
     { ESL_NNNN, ESL_NNNN, ESL_MMMM, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_PIXEL_X_DIMENSION, "PixelXDimension", ("Pixel X Dimension"),
     ("Information specific to compressed data. When a "
        "compressed file is recorded, the valid width of the "
        "meaningful image must be recorded in this tag, whether or "
        "not there is padding data or a restart marker. This tag "
        "should not exist in an uncompressed file."),
     { ESL_NNNN, ESL_NNNN, ESL_NNNM, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_PIXEL_Y_DIMENSION, "PixelYDimension", ("Pixel Y Dimension"),
     ("Information specific to compressed data. When a compressed "
        "file is recorded, the valid height of the meaningful image "
        "must be recorded in this tag, whether or not there is padding "
        "data or a restart marker. This tag should not exist in an "
        "uncompressed file. "
        "Since data padding is unnecessary in the vertical direction, "
        "the number of lines recorded in this valid image height tag "
        "will in fact be the same as that recorded in the SOF."),
     { ESL_NNNN, ESL_NNNN, ESL_NNNM, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_RELATED_SOUND_FILE, "RelatedSoundFile",
     ("Related Sound File"),
     ("This tag is used to record the name of an audio file related "
        "to the image data. The only relational information "
        "recorded here is the Exif audio file name and extension (an "
        "ASCII string consisting of 8 characters + '.' + 3 "
        "characters). The path is not recorded. Stipulations on audio "
        "and file naming conventions are defined in the specification. "
        "When using this tag, audio files must be recorded in "
        "conformance to the Exif audio format. Writers are also allowed "
        "to store the data such as Audio within APP2 as FlashPix "
        "extension stream data. "
        "The mapping of Exif image files and audio files is done "
        "in any of three ways, [1], [2] and [3]. If multiple files "
        "are mapped to one file as in [2] or [3], the above "
        "format is used to record just one audio file name. If "
        "there are multiple audio files, the first recorded file is "
        "given. In the case of [3], for example, for the "
        "Exif image file \"DSC00001.JPG\" only  \"SND00001.WAV\" is "
        "given as the related Exif audio file. When there are three "
        "Exif audio files \"SND00001.WAV\", \"SND00002.WAV\" and "
        "\"SND00003.WAV\", the Exif image file name for each of them, "
        "\"DSC00001.JPG\", is indicated. By combining multiple "
        "relational information, a variety of playback possibilities "
        "can be supported. The method of using relational information "
        "is left to the implementation on the playback side. Since this "
        "information is an ASCII character string, it is terminated by "
        "NULL. When this tag is used to map audio files, the relation "
        "of the audio file to image data must also be indicated on the "
        "audio file end."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_INTEROPERABILITY_IFD_POINTER, "InteroperabilityIFDPointer",
     ("Interoperability IFD Pointer"),
     ("Interoperability IFD is composed of tags which stores the "
        "information to ensure the Interoperability and pointed "
        "by the following tag located in Exif IFD. "
        "The Interoperability structure of Interoperability IFD is "
        "the same as TIFF defined IFD structure "
        "but does not contain the "
        "image data characteristically compared with normal TIFF "
        "IFD."),
     { ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FLASH_ENERGY, "FlashEnergy", ("Flash Energy"),
     ("Indicates the strobe energy at the time the image is "
        "captured, as measured in Beam Candle Power Seconds (BCPS)."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SPATIAL_FREQUENCY_RESPONSE, "SpatialFrequencyResponse",
     ("Spatial Frequency Response"),
     ("This tag records the camera or input device spatial frequency "
        "table and SFR values in the direction of image width, "
        "image height, and diagonal direction, as specified in ISO "
        "12233."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FOCAL_PLANE_X_RESOLUTION, "FocalPlaneXResolution",
     ("Focal Plane X-Resolution"),
     ("Indicates the number of pixels in the image width (X) direction "
        "per <FocalPlaneResolutionUnit> on the camera focal plane."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FOCAL_PLANE_Y_RESOLUTION, "FocalPlaneYResolution",
     ("Focal Plane Y-Resolution"),
     ("Indicates the number of pixels in the image height (V) direction "
        "per <FocalPlaneResolutionUnit> on the camera focal plane."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FOCAL_PLANE_RESOLUTION_UNIT, "FocalPlaneResolutionUnit",
     ("Focal Plane Resolution Unit"),
     ("Indicates the unit for measuring <FocalPlaneXResolution> and "
        "<FocalPlaneYResolution>. This value is the same as the "
        "<ResolutionUnit>."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SUBJECT_LOCATION, "SubjectLocation",
     ("Subject Location"),
     ("Indicates the location of the main subject in the scene. The "
        "value of this tag represents the pixel at the center of the "
        "main subject relative to the left edge, prior to rotation "
        "processing as per the <Rotation> tag. The first value "
        "indicates the X column number and the second indicates "
        "the Y row number."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_EXPOSURE_INDEX, "ExposureIndex", ("Exposure Index"),
     ("Indicates the exposure index selected on the camera or "
        "input device at the time the image is captured."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SENSING_METHOD, "SensingMethod", ("Sensing Method"),
     ("Indicates the image sensor type on the camera or input "
        "device."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FILE_SOURCE, "FileSource", ("File Source"),
     ("Indicates the image source. If a DSC recorded the image, "
        "the tag value of this tag always be set to 3, indicating "
        "that the image was recorded on a DSC."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SCENE_TYPE, "SceneType", ("Scene Type"),
     ("Indicates the type of scene. If a DSC recorded the image, "
        "this tag value must always be set to 1, indicating that the "
        "image was directly photographed."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_NEW_CFA_PATTERN, "CFAPattern",
     ("CFA Pattern"),
     ("Indicates the color filter array (CFA) geometric pattern of the "
        "image sensor when a one-chip color area sensor is used. "
        "It does not apply to all sensing methods."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_CUSTOM_RENDERED, "CustomRendered", ("Custom Rendered"),
     ("This tag indicates the use of special processing on image "
        "data, such as rendering geared to output. When special "
        "processing is performed, the reader is expected to disable "
        "or minimize any further processing."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_EXPOSURE_MODE, "ExposureMode", ("Exposure Mode"),
     ("This tag indicates the exposure mode set when the image was "
        "shot. In auto-bracketing mode, the camera shoots a series of "
        "frames of the same scene at different exposure settings."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_WHITE_BALANCE, "WhiteBalance", ("White Balance"),
     ("This tag indicates the white balance mode set when the image "
        "was shot."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_DIGITAL_ZOOM_RATIO, "DigitalZoomRatio",
     ("Digital Zoom Ratio"),
     ("This tag indicates the digital zoom ratio when the image was "
        "shot. If the numerator of the recorded value is 0, this "
        "indicates that digital zoom was not used."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_FOCAL_LENGTH_IN_35MM_FILM, "FocalLengthIn35mmFilm",
     ("Focal Length in 35mm Film"),
     ("This tag indicates the equivalent focal length assuming a "
        "35mm film camera, in mm. A value of 0 means the focal "
        "length is unknown. Note that this tag differs from the "
        "FocalLength tag."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SCENE_CAPTURE_TYPE, "SceneCaptureType",
     ("Scene Capture Type"),
     ("This tag indicates the type of scene that was shot. It can "
        "also be used to record the mode in which the image was "
        "shot. Note that this differs from the scene type "
        "<SceneType> tag."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_GAIN_CONTROL, "GainControl", ("Gain Control"),
     ("This tag indicates the degree of overall image gain "
        "adjustment."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_CONTRAST, "Contrast", ("Contrast"),
     ("This tag indicates the direction of contrast processing "
        "applied by the camera when the image was shot."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SATURATION, "Saturation", ("Saturation"),
     ("This tag indicates the direction of saturation processing "
        "applied by the camera when the image was shot."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SHARPNESS, "Sharpness", ("Sharpness"),
     ("This tag indicates the direction of sharpness processing "
        "applied by the camera when the image was shot."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_DEVICE_SETTING_DESCRIPTION, "DeviceSettingDescription",
     ("Device Setting Description"),
     ("This tag indicates information on the picture-taking "
        "conditions of a particular camera model. The tag is used "
        "only to indicate the picture-taking conditions in the "
        "reader."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_SUBJECT_DISTANCE_RANGE, "SubjectDistanceRange",
     ("Subject Distance Range"),
     ("This tag indicates the distance to the subject."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {EXIF_TAG_IMAGE_UNIQUE_ID, "ImageUniqueID", ("Image Unique ID"),
     ("This tag indicates an identifier assigned uniquely to "
        "each image. It is recorded as an ASCII string equivalent "
        "to hexadecimal notation and 128-bit fixed length."),
     { ESL_NNNN, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    /* Not in EXIF 2.2 */
    {EXIF_TAG_GAMMA, "Gamma", ("Gamma"),
     ("Indicates the value of coefficient gamma.")},
    /* Not in EXIF 2.2 */
    {EXIF_TAG_PRINT_IMAGE_MATCHING, "PrintImageMatching", ("PRINT Image Matching"),
     ("Related to Epson's PRINT Image Matching technology")},
    /* Not in EXIF 2.2 (from the Microsoft HD Photo specification) */
    {EXIF_TAG_PADDING, "Padding", ("Padding"),
     ("This tag reserves space that can be reclaimed later when "
            "additional metadata are added. New metadata can be written "
            "in place by replacing this tag with a smaller data element "
            "and using the reclaimed space to store the new or expanded "
            "metadata tags."),
     { ESL_OOOO, ESL_NNNN, ESL_OOOO, ESL_NNNN, ESL_NNNN } },
    {0, NULL, NULL, NULL}
};

class TagInfo
{
public:
    TagInfo() {
        tag = -1;
        title = "";
        description = "";
        importance = 0;
    }
    TagInfo(int tag_, QString title_, QString description_) {
        tag = tag_;
        title = title_;
        description = description_;
        importance = 0;
    }

    int tag;
    QString title;
    QString description;
    int importance;
} ;

static QMap<QString,TagInfo*> *tagInfoMap = NULL;

static void setImportance(QString key, int value) {
    TagInfo * ti = tagInfoMap->value(key,NULL);
    //qDebug() << "setting importance of" << key << "to" << value;

    if (ti != NULL) {
        ti->importance = value;
    } else {
        qDebug() << "Unknown EXIF tag:" << key;
    }
}

static void initTagInfoMap() {
    if (tagInfoMap == NULL) {
        int v = 500;
        tagInfoMap = new QMap<QString,TagInfo*>();
        int tableSize = sizeof(ExifTagTable) / sizeof(TagEntry);
        qDebug() << "Known EXIF tags: " << tableSize;
        for (int i = 0; i < tableSize; i++) {
            TagEntry te = ExifTagTable[i];
            if (QString(te.name).size()>0) {
                //qDebug() << "inserting" << QString(te.name) ;
                tagInfoMap->insert(QString(te.name),new TagInfo(te.tag,QString(te.title),QString(te.description)));
            }
        }
        setImportance("Make", v--);
        setImportance("Model", v--);
        setImportance("DateTimeOriginal", v--);
        setImportance("DateTime", v--);
        setImportance("ExposureTime", v--);
        setImportance("ISOSpeedRatings", v--);
        setImportance("FNumber", v--);
        setImportance("FocalLength", v--);
        setImportance("DigitalZoomRatio", v--);
        setImportance("PixelXDimension", v--);
        setImportance("PixelYDimension", v--);
        setImportance("Flash", v--);
        setImportance("ExposureMode", v--);
        setImportance("ExposureProgram", v--);
        setImportance("MeteringMode", v--);
        setImportance("SceneCaptureType", v--);
        setImportance("Contrast", v--);
        setImportance("Saturation", v--);
        setImportance("Sharpness", v--);
        setImportance("WhiteBalance", v--);
        setImportance("BrightnessValue", v--);
        setImportance("ExposureBiasValue", v--);
        setImportance("MaxApertureValue", v--);
    }
}


File::File(const File &fi) {
    shallowCopy(&fi);
}

File &File::operator=(const File &fi) {
    shallowCopy(&fi);
}

void File::shallowCopy(const File *fi) {
    lastModified = fi->lastModified;
    absoluteFilePath = fi->absoluteFilePath;
    size = fi->size;
    isDir = fi->isDir;
    IDPath = fi->IDPath;
    constructCommonFields();
}

File::File(QString path) {
    init(QFileInfo(path));
}

File::File()
{
    absoluteFilePath = "";
    IDPath = "";
    isDir = true;
    constructCommonFields();
}

File::File(QFileInfo qfi)
{
    init(qfi);
}

void File::init(QFileInfo qfi) {
    lastModified = qfi.lastModified().toTime_t();
    absoluteFilePath = qfi.absoluteFilePath();
    size = qfi.size();
    isDir = qfi.isDir();
    IDPath = "";
    constructCommonFields();
}

File::File(uint lastModified_, QString absoluteFilePath_, quint64 size_, bool isDir_, QString IDPath_){
    lastModified = lastModified_;
    absoluteFilePath = absoluteFilePath_;
    size = size_;
    isDir = isDir_;
    IDPath = IDPath_;
    constructCommonFields();
}

void File::constructCommonFields() {
    exifData = NULL;
    exifLoadAttempted = false;
    buffer = NULL;
    geotaggedBuffer = NULL;
    transferTo = "";
    pipedTo = "";
    transferOnGoing = false;
}

File::~File(){
    if (exifData != NULL) {
        exif_data_free(exifData);
        exifData = NULL;
    }
}

QString File::fileName() const{
    return absoluteFilePath.split("/").last();
}

QString File::extension() const {
    QStringList split = fileName().split(".");
    if (split.size()>1) {
        return split.last().toLower();
    } else {
        return "";
    }
}

int File::operator<(File other) {
    return absoluteFilePath < other.absoluteFilePath;
}

int File::operator==(const File &other) const {
    if (size == other.size && fileName() == other.fileName()) {
        if (lastModified == other.lastModified) {
            return true;
        }
        /** https://github.com/clement-nardi/PicsDL/issues/9
         *  https://bugreports.qt-project.org/browse/QTBUG-43185#comment-267721
         */
        if (abs(lastModified - other.lastModified) < (26 *60 *60) && // Time zones go from -12 to +14
            abs(lastModified - other.lastModified)%(30*60) == 0    ) { // Some time zones have 30 minutes shift
            //qDebug() << "Found the same file with a " << QString("%1").arg((((double)lastModified - (double)(other.lastModified)))/(60*60),0,'f',1) << " hour shift.";
            return true;
        }
    }
    return false;
}

uint qHash(File fi) {
    return fi.size;
}

bool File::isAttachmentOf(File other) {
    return        absoluteFilePath.left(      absoluteFilePath.lastIndexOf(".")) ==
            other.absoluteFilePath.left(other.absoluteFilePath.lastIndexOf("."));
}

bool File::isPicture() const {
    return pictureExtensions.contains(extension());
}

bool File::isVideo() const{
    return videoExtensions.contains(extension());
}

bool File::isJPEG() const {
    return JPEGExtensions.contains(extension());
}

void File::loadExifData() {
    if (exifData == NULL && !exifLoadAttempted) {
        exifLoadAttempted = true;
        if (IDPath.startsWith("WPD:/")) {

#ifdef _WIN32
            static QMap<QString,DWORD> lastSuccessfulTransferSize;
            qDebug() << "loadExifData for" << absoluteFilePath;
            QElapsedTimer timer;
            qint64 initTime;
            qint64 transferTime = 0;
            qint64 closeTime;
            QString deviceID = IDPath.split("/")[1];
            QString objectID = IDPath.split("/").last();
            WCHAR * deviceID_i = (WCHAR*) malloc(sizeof(WCHAR)*(deviceID.size()+1));
            WCHAR * objectID_i = (WCHAR*) malloc(sizeof(WCHAR)*(objectID.size()+1));
            deviceID.toWCharArray(deviceID_i);
            deviceID_i[deviceID.size()] = L'\0';
            objectID.toWCharArray(objectID_i);
            objectID_i[objectID.size()] = L'\0';
            DWORD optimalTransferSize;
            DWORD read;
            DWORD totalRead = 0;
            timer.start();
            if (!WPDI_InitTransfer(deviceID_i,objectID_i,&optimalTransferSize)) return ;
            initTime = timer.nsecsElapsed();
            DWORD transferSize = lastSuccessfulTransferSize.value(extension(),256);
            int count = 0;
            unsigned char * data = (unsigned char *)malloc(sizeof(unsigned char) * transferSize);
            //exifData = exif_data_new_from_data	(data,read)	;
            ExifLoader* exifLoader = exif_loader_new();
            while (true) {
                timer.restart();
                WPDI_readNextData(data,transferSize,&read);
                transferTime += timer.nsecsElapsed();

                if (read <= 0) {
                    break;
                }

                count++;
                totalRead += read;
                if (exif_loader_write(exifLoader, data, read)) {
                    //qDebug() << "exif_loader_write needs more data after" << totalRead << "bytes";
                    if (transferSize<1024) {
                        transferSize = 1024;
                        free(data);
                        data = (unsigned char *)malloc(sizeof(unsigned char) * transferSize);
                    } else {
                        transferSize = 1024;
                    }
                } else {
                    //qDebug() << "exif_loader_write finished after" << totalRead << "bytes";
                    lastSuccessfulTransferSize.insert(extension(),totalRead);
                    break;
                }
            }
            exifData = exif_loader_get_data(exifLoader);
            if (exifData != NULL) {
                //qDebug() << "exifData is not NULL";
            } else {
                //qDebug() << "exifData is NULL";
            }

            free(data);
            free(deviceID_i);
            free(objectID_i);
            free(exifLoader);
            /** when file is not completely downloaded, WPDI_CloseTransfer takes 0.4 seconds to complete
             * Since transfer speed is around 30MB/s, complete the download if the size of the file is under 10MB
             */
            if (size < 12000000) {
                free(data);
                data = (unsigned char *)malloc(sizeof(unsigned char) * optimalTransferSize);
                timer.restart();
                while (WPDI_readNextData(data,optimalTransferSize,&read)) {
                    totalRead += read;
                }
                transferTime += timer.nsecsElapsed();
            }

            timer.restart();
            WPDI_CloseTransfer();
            closeTime = timer.nsecsElapsed();
            qint64 totalTime = initTime + transferTime + closeTime;
            qDebug() << QString("Init %1ms (%2\%)), Transfer %3ms (%4\%) %5@%6/s, Close %7ms (%8\%)")
                        .arg((double)initTime/1000000,0,'f',3)
                        .arg((double)initTime*100/totalTime,0,'f',2)
                        .arg((double)transferTime/1000000,0,'f',3)
                        .arg((double)transferTime*100/totalTime,0,'f',2)
                        .arg(File::size2Str(totalRead))
                        .arg(File::size2Str((qint64)((double)totalRead*1000000000/(double)transferTime)))
                        .arg((double)closeTime/1000000,0,'f',3)
                        .arg((double)closeTime*100/totalTime,0,'f',2) ;
#endif
        } else {
            /* Doesn't work with paths containing non-ascii characters */
            //exifData = exif_data_new_from_file (absoluteFilePath.toStdString().c_str());

            QFile file(absoluteFilePath);
            if (!file.open(QIODevice::ReadOnly)) {
                return ;
            }
            ExifLoader* exifLoader = exif_loader_new();
            char data[4096];
            while (true) {
                qint64 read = file.read(data,4096);
                if (read <= 0) {
                    break;
                }
                if (!exif_loader_write(exifLoader, (uchar *) data, read)) {
                    break;
                }
            }
            file.close();
            exifData = exif_loader_get_data(exifLoader);
            free(exifLoader);
        }
    }
}

QString File::getEXIFValue(QString key) const {
    //qDebug() << "get EXIF Key" << key << "for" << fileName();
    if (exifData != NULL && exifData->data != NULL) {
        //qDebug() << "calling initTagInfoMap";
        initTagInfoMap();
        //qDebug() << "calling tagInfoMap->value";
        TagInfo *ti = tagInfoMap->value(key,NULL);
        if (ti != NULL) {
            //qDebug() << "get Key Info: tag=" << ti->tag << "title=" << ti->title;
            if (ti->tag != -1) {
                ExifEntry *entry = exif_data_get_entry(exifData,(ExifTag)(ti->tag));
                if (entry != NULL) {
                    char value[1024];
                    //qDebug() << "calling exif_entry_get_value";
                    exif_entry_get_value(entry, value, 1024);
                    value[1023] = '\0'; //security
                    //qDebug() << "value=" << value;
                    return QString(value);
                    //return getStringFromUnsignedChar(entry->data);
                } else {
                    //qDebug() << "no such entry: " << key;
                }
            } else {
                //qDebug() << "Not enough information on this key: " << key;
            }
        } else {
            //qDebug() << "unknown EXIF key: " << key;
        }
    } else {
         //qDebug() << "exifData is NULL !!!";
    }
    return "";
}


QPixmap File::getThumbnail() {
    if (thumbnail.isNull()) {
        if (exifData != NULL) {
            if (exifData->data && exifData->size) {
                QBuffer buf;
                QImageReader ir;
                QImage image;
                buf.open(QIODevice::ReadWrite);
                buf.write((char*)(exifData->data),exifData->size);
                buf.seek(0);
                ir.setDevice(&buf);
                ir.autoDetectImageFormat();
                image = ir.read();
                if (image.isNull()) {
                    qDebug() << ir.errorString();
                } else {
                    QImage croppedImage;
                    int firstLine = 0;
                    int lastLine = image.height()-1;
                    int const blackThreshold = 15;
                    while (firstLine<image.height()) {
                        bool allIsBlack = true;
                        for (int col = 0; col<image.width(); col++) {
                            if (qGray(image.pixel(col,firstLine)) > blackThreshold) {
                                /* found a non-black pixel */
                                allIsBlack = false;
                                break;
                            }
                        }
                        if (allIsBlack) {
                            firstLine++;
                        } else {
                            break;
                        }
                    }
                    while (lastLine>=0) {
                        bool allIsBlack = true;
                        for (int col = 0; col<image.width(); col++) {
                            if (qGray(image.pixel(col,lastLine)) > blackThreshold) {
                                /* found a non-black pixel */
                                allIsBlack = false;
                                break;
                            }
                        }
                        if (allIsBlack) {
                            lastLine--;
                        } else {
                            break;
                        }
                    }
                    croppedImage = image.copy(0,firstLine,image.width(),lastLine-firstLine+1);

                    thumbnail = QPixmap::fromImage(croppedImage.scaled(QSize(160,160),Qt::KeepAspectRatio));
                }
            }
        }
    }
    return thumbnail;
}

uint File::dateTaken() const{
    QString exifDate = dateTakenRaw();
    if (exifDate.size()>0) {
        QStringList exifDateSplit = exifDate.replace(" ",":").split(":");
        if (exifDateSplit.size()>=6) {
            QDateTime date = QDateTime(QDate(exifDateSplit.at(0).toInt(),
                                             exifDateSplit.at(1).toInt(),
                                             exifDateSplit.at(2).toInt()),
                                       QTime(exifDateSplit.at(3).toInt(),
                                             exifDateSplit.at(4).toInt(),
                                             exifDateSplit.at(5).toInt()));
            return date.toTime_t();
        } else {
            qDebug() << "not enough information: " << exifDate;
        }
    } else {
        //qDebug() << "empty date? :" << exifDate;
    }
    return lastModified;
}

QString File::dateTakenRaw() const{
    QString exifDate = getEXIFValue("DateTimeDigitized");
    if (exifDate.size() == 0) {
        exifDate = getEXIFValue("DateTimeOriginal");
    }
    if (exifDate.size() == 0) {
        exifDate = getEXIFValue("DateTime");
    }
    return exifDate;
}

static bool tagLessThan(QString a, QString b) {
    int ia = tagInfoMap->value(a)->importance;
    int ib = tagInfoMap->value(b)->importance;
    if (ia!=ib) {
        return ia>ib;
    } else {
        return a < b;
    }
}

QStringList File::getEXIFTags(){
    QStringList results;
    if (exifData != NULL) {
        initTagInfoMap();
        qDebug() << "start getEXIFTags";
        for (int i = 0; i < tagInfoMap->keys().size(); i++) {
            if (getEXIFValue(tagInfoMap->keys().at(i)).size() > 0) {
                results.append(tagInfoMap->keys().at(i));
            }
        }
        qSort(results.begin(), results.end(), tagLessThan);
        exif_data_dump(exifData);
    }
    return results;
}

QString File::size2Str(qint64 nbBytes) {
    if (nbBytes < 0) {
        return "-" + size2Str(-nbBytes);
    }
    QString baseUnit = "B"; // Byte
    QStringList prefixes;
    prefixes << "" << "K" << "M" << "G" << "T" << "P" << "E" << "Z" << "Y";
    int prefixIdx = 0;
    qint64 number = nbBytes;
    while (number >= 1024*1000) {
        number /= 1024;
        prefixIdx++;
    }
    double value = (double) number;
    if (value >= 1000){
        value /= 1024;
        prefixIdx++;
    }
    return QString("%1 %2%3").arg(value,0,'g',3).arg(prefixes[prefixIdx]).arg(baseUnit);
}

QList<File> File::ls(bool *theresMore) {
    QList<File> res;
    *theresMore = false;
    if (IDPath.startsWith("WPD:/")) {

#ifdef _WIN32
        if (absoluteFilePath.count("/") > 5) return res;
        QString deviceID = IDPath.split("/")[1];
        WCHAR * deviceID_i = (WCHAR*) malloc(sizeof(WCHAR)*(deviceID.size()+1));
        QString objectID;
        if (IDPath.count("/") == 1) {
            /* this is the root */
            objectID = "DEVICE";
        } else {
            objectID = IDPath.split("/").last();
        }
        WCHAR * objectID_i = (WCHAR*) malloc(sizeof(WCHAR)*(objectID.size()+1));
        deviceID.toWCharArray(deviceID_i);
        deviceID_i[deviceID.size()] = L'\0';
        objectID.toWCharArray(objectID_i);
        objectID_i[objectID.size()] = L'\0';
        int count;
        WPDFileInfo WPDList[MAX_REQUESTED_ELEMENTS];
        /*qDebug() << "Calling WPDI_LS(" << QString::fromWCharArray(deviceID_i)
                 << "," << QString::fromWCharArray(objectID_i);*/
        qDebug() << "listing content of" << absoluteFilePath;
        //qDebug() << "             path=" << IDPath;
        WPDI_LS(deviceID_i,objectID_i,WPDList,&count);
        qDebug() << "returned" << count << "elements";
        for (int i = 0; i<count; i++) {
            //qDebug() << "appending element " << i << QString::fromWCharArray(WPDList[i].id) << QString::fromWCharArray(WPDList[i].name);
            //qDebug() << QString::fromWCharArray(WPDList[i].date);
            res.append(File(QDateTime::fromString(QString::fromWCharArray(WPDList[i].date),
                                                      "yyyy/MM/dd:HH:mm:ss.zzz").toTime_t(),
                                absoluteFilePath + "/" + QString::fromWCharArray(WPDList[i].name),
                                WPDList[i].size,
                                WPDList[i].isDir,
                                IDPath + "/" + QString::fromWCharArray(WPDList[i].id)));
            WPDI_Free(WPDList[i].id);
            WPDI_Free(WPDList[i].name);
            WPDI_Free(WPDList[i].date);
        }
        if (count >= MAX_REQUESTED_ELEMENTS) {
            *theresMore = true;
        } else {
            WPDI_Reset_LS();
        }
        free(deviceID_i);
        free(objectID_i);
#endif
    } else {
        QDir dir = QDir(absoluteFilePath);
        dir.setFilter(dir.filter()|QDir::NoDotAndDotDot|QDir::System);
        for (int i = 0; i<dir.entryInfoList().size(); i++) {
            res.append(File(dir.entryInfoList().at(i)));
        }
    }
    return res;
}


void File::launchWrite(QString dest, bool geotag){
    if (geotag && geotags.size()>0) {
        ExifData * newexif = exif_data_new_from_data(exifData->data,exifData->size);

    }
}


void File::writeHeader(QString dest){
    if (dest == "") {
        dest = absoluteFilePath + ".header";
    }
    QFile file(dest);
    file.open(QIODevice::WriteOnly);
    char *data;
    uint size;

    exif_data_set_option(exifData, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION  );
    exif_data_save_data(exifData,(uchar **)&data,&size);
    file.write((char *)data,size);
    file.close();

}

void File::writeContent(QString dest){
    if (dest == "") {
        dest = absoluteFilePath + ".content";
    }
    QFile source(absoluteFilePath);
    source.open(QIODevice::ReadOnly);
    source.seek(exifData->size);
    QFile file(dest);
    file.open(QIODevice::WriteOnly);
    file.write(source.readAll());
    file.close();
    source.close();

}


bool File::copyWithDirs(QString to) {
    QDir().mkpath(QFileInfo(to).absolutePath());
    QFile *toFile = new QFile(to);
    bool res;
    res = FillIODeviceWithContent(toFile);
    if (res) {
        File::setDates(to,lastModified);
    } else {
        //TODO: Remove the destination file?
    }

    return res;
}


void File::setDates(QString fileName,uint date) {
    struct utimbuf times;
    times.actime = date;
    times.modtime = date;
    utime(fileName.toStdString().c_str(),&times);
}

#define CHUNK_SIZE 4*1024
#define NB_CHUNKS 5

IOReader::IOReader(QIODevice *device_, QSemaphore *s_, TransferManager *tm_) {
    device = device_;
    s = s_;
    tm = tm_;
    tm->totalToCache += NB_CHUNKS*CHUNK_SIZE;
}
void IOReader::run() {
    bool deviceIsFile = ((dynamic_cast<QFile*>       (device)) != NULL) ||
                        ((dynamic_cast<WPDIODevice*> (device)) != NULL)   ;
    if (!device->open(QIODevice::ReadOnly)) {
        qWarning() << QString("WARNING: unable to open %1 for reading").arg(deviceIsFile?"File":"Buffer");
    }
    if (deviceIsFile) {
        emit readStarted();
    }

    bool theEnd = false;
    while (!theEnd && !tm->wasStopped) {
        s->acquire();
        QByteArray ba = device->read(CHUNK_SIZE);
        tm->totalCached+=ba.size();
        theEnd = device->atEnd();
        emit dataChunk(ba,!theEnd); /* send 4KB data chuncks */
    }
    device->close();
    if (!deviceIsFile) {
        /* read from a buffer, it will be deleted after write is finished */
        tm->totalCached-=device->size();
        tm->totalToCache-=device->size();
    }
}

IOWriter::IOWriter(QIODevice *device_, QSemaphore *s_, TransferManager *tm_){
    device = device_;
    s = s_;
    initDone = false;
    tm = tm_;
}

void IOWriter::init(){
    QFile * out = dynamic_cast<QFile*> (device);
    deviceIsFile = (out != NULL);
    if (!device->open(QIODevice::WriteOnly)) {
        qWarning() << QString("WARNING: unable to open %1 for writing").arg(deviceIsFile?"File":"Buffer");
    }
}

void IOWriter::dataChunk(QByteArray ba, bool theresMore){
    if (!initDone) {
        initDone = true;
        init();
    }
    device->write(ba);
    s->release();
    if (deviceIsFile) {
        tm->totalTransfered+=ba.size();
        tm->totalCached-=ba.size();
    } else {
        //tm->totalCached-=ba.size();
        //tm->totalCached+=ba.size();
    }
    if (!theresMore || tm->wasStopped) {
        tm->totalToCache -= NB_CHUNKS*CHUNK_SIZE;
        device->close();
        emit writeFinished();
    }
}

void File::readStarted() {
    transferOnGoing = true;
    emit readStarted(this);
}

void File::writeFinished() {
    qDebug() << QString("%1 - writeFinished to %2").arg(fileName()).arg(pipedTo);
    if (pipedTo == "buffer") {
        tm->geotagger->geotag(this);
    } else {
        setDates(pipedTo,lastModified);
        if (buffer != NULL || geotaggedBuffer != NULL) {
            tm->geotagSemaphore.release();
        } else {
            tm->directSemaphore.release();
        }
        if (tm->wasStopped) {
            QFile(pipedTo).remove();
        }
        transferOnGoing = false;
        emit writeFinished(this);
    }
}

void File::launchTransferTo(QString to, TransferManager *tm_, bool geotag) {
    qDebug() << QString("%1 - launchTransferTo %2").arg(fileName()).arg(to);
    tm = tm_;
    transferTo = to;
    if (geotag) {
        pipeToBuffer();
    } else {
        pipe(to);
    }
}


void File::setGeotaggedBuffer(QBuffer *geotaggedBuffer_) {
    qDebug() << QString("%1 - setGeotaggedBuffer").arg(fileName());
    geotaggedBuffer = geotaggedBuffer_;
    pipe(transferTo);
}


QIODevice *File::getReadDevice() {

    if (!IDPath.startsWith("WPD:/")) {
        return new QFile(absoluteFilePath);
#ifdef _WIN32
    } else if (IDPath.startsWith("WPD:/")) {
        return new WPDIODevice(IDPath);
#endif
    } else {
        return NULL;
    }
}

void File::pipeToBuffer(){
    pipedTo = "buffer";
    if (buffer != NULL) {
        buffer->deleteLater();
        buffer = NULL;
    }
    QIODevice * in = getReadDevice();
    buffer = new QBuffer();
    pipe(in, buffer);
}

void File::pipe(QString to){
    qDebug() << QString("%1 - pipe to %2").arg(fileName()).arg(to);
    QIODevice * in = NULL;
    QIODevice * out = new QFile(to);
    if (geotaggedBuffer != NULL && geotaggedBuffer->size() > 0) {
        in = geotaggedBuffer;
        qint64 diff = geotaggedBuffer->size() - buffer->size();
        tm->totalCached += diff;
        tm->totalToCache += diff;

        if (buffer != NULL) {
            buffer->deleteLater();
            buffer = NULL;
        }
    } else if (buffer != NULL && buffer->size() > 0) {
        qWarning() << QString("%1 - WARNING - problem with geotaggedBuffer");
        in = buffer;
        if (geotaggedBuffer != NULL) {
            geotaggedBuffer->deleteLater();
            geotaggedBuffer = NULL;
        }
    } else {
        in = getReadDevice();
    }
    connect(this,SIGNAL(writeFinished(File*)),out,SLOT(deleteLater()));
    QDir().mkpath(QFileInfo(to).absolutePath());
    pipedTo = to;
    pipe(in, out);
}

void File::pipe(QIODevice *in, QIODevice *out){
    QThread *writeThread = new QThread();
    readSemaphore.release(NB_CHUNKS-readSemaphore.available()); /* 20KB read cache */
    IOWriter *writer = new IOWriter(out,&readSemaphore,tm);
    IOReader *reader = new IOReader(in,&readSemaphore,tm);
    writer->moveToThread(writeThread);
    connect(writeThread,SIGNAL(finished()),writeThread,SLOT(deleteLater()));
    connect(reader,SIGNAL(dataChunk(QByteArray,bool)),writer,SLOT(dataChunk(QByteArray,bool)),Qt::QueuedConnection);
    connect(reader,SIGNAL(readStarted()),this,SLOT(readStarted()),Qt::QueuedConnection);
    connect(reader,SIGNAL(finished()),reader,SLOT(deleteLater()));
    connect(reader,SIGNAL(finished()),in,SLOT(deleteLater()));
    connect(writer,SIGNAL(writeFinished()),this,SLOT(writeFinished()));
    connect(writer,SIGNAL(writeFinished()),writeThread,SLOT(quit()));
    connect(writer,SIGNAL(writeFinished()),writer,SLOT(deleteLater()));
    writeThread->start();
    reader->start();

    if ((dynamic_cast<QFile*> (out)) == NULL) {
        tm->totalToCache += in->size();
    }
}


bool File::FillIODeviceWithContent(QIODevice *out) {
    if (!IDPath.startsWith("WPD:/")) {
        QFile in(absoluteFilePath);

        if (!in.open(QIODevice::ReadOnly)) {
            return false;
        }
        if (!out->open(QIODevice::WriteOnly)) {
            in.close();
            return false;
        }
        qint64 written = out->write(in.readAll());
        out->close();
        in.close();
        return written > 0;
#ifdef _WIN32
    } else if (IDPath.startsWith("WPD:/")) {
        QElapsedTimer timer;
        qint64 initTime;
        qint64 transferTime = 0;
        qint64 closeTime;
        QString deviceID = IDPath.split("/")[1];
        QString objectID = IDPath.split("/").last();
        WCHAR * deviceID_i = (WCHAR*) malloc(sizeof(WCHAR)*(deviceID.size()+1));
        WCHAR * objectID_i = (WCHAR*) malloc(sizeof(WCHAR)*(objectID.size()+1));
        deviceID.toWCharArray(deviceID_i);
        deviceID_i[deviceID.size()] = L'\0';
        objectID.toWCharArray(objectID_i);
        objectID_i[objectID.size()] = L'\0';
        DWORD optimalTransferSize;
        DWORD read;
        qint64 written;
        qint64 totalWritten = 0;
        timer.start();
        if (!WPDI_InitTransfer(deviceID_i,objectID_i,&optimalTransferSize)) return false;
        initTime = timer.nsecsElapsed();
        /* Create the destination file only if InitTransfer worked */
        if (!out->open(QIODevice::WriteOnly)) return false;
        //qDebug() << "optimalTransferSize=" << optimalTransferSize;
        unsigned char * data = (unsigned char *)malloc(sizeof(unsigned char) * optimalTransferSize);
        timer.restart();
        while (WPDI_readNextData(data,optimalTransferSize,&read)) {
            transferTime += timer.nsecsElapsed();
            written = out->write((char *)data,read);
            if ( written == -1) {
                qWarning() << "Problem during toFile.write(data,"<<read<<")";
                break;
            }
            if ( written != read ) {
                qWarning() << "Problem during toFile.write(data,"<<read<<"), written=" << written;
            }
            totalWritten += written;
            //qDebug() << "Transfered " << totalWritten << "so far!";
            timer.restart();
        }
        free(data);
        free(deviceID_i);
        free(objectID_i);
        out->close();
        timer.restart();
        WPDI_CloseTransfer();
        closeTime = timer.nsecsElapsed();
        qint64 totalTime = initTime + transferTime + closeTime;
        qDebug() << QString("Init %1ms (%2\%)), Transfer %3ms (%4\%) %5@%6/s, Close %7ms (%8\%)")
                    .arg((double)initTime/1000000,0,'f',3)
                    .arg((double)initTime*100/totalTime,0,'f',2)
                    .arg((double)transferTime/1000000,0,'f',3)
                    .arg((double)transferTime*100/totalTime,0,'f',2)
                    .arg(File::size2Str(totalWritten))
                    .arg(File::size2Str((qint64)((double)totalWritten*1000000000/(double)transferTime)))
                    .arg((double)closeTime/1000000,0,'f',3)
                    .arg((double)closeTime*100/totalTime,0,'f',2) ;
        if ( written == -1) {
            return false;
        } else {
            return true;
        }
#endif
    }
}

bool File::moveWithDirs(QString to) {
    QDir().mkpath(QFileInfo(to).absolutePath());
    return QFile(absoluteFilePath).rename(to);
}

#ifdef _WIN32
#include "windows.h"
#endif

bool File::setHidden() {
#ifdef _WIN32
    WCHAR * wPath = (WCHAR*) malloc(sizeof(WCHAR)*(absoluteFilePath.size()+1));
    QString windowsStylePath = absoluteFilePath;
    windowsStylePath.replace("/","\\").toWCharArray(wPath);
    DWORD dwAttrs = GetFileAttributes(wPath);
    SetFileAttributes(wPath, dwAttrs | FILE_ATTRIBUTE_HIDDEN);
    free(wPath);
#endif
}



