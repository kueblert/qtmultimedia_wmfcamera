#ifndef ESCAPI_MINIMAL_H
#define ESCAPI_MINIMAL_H

// Options accepted by above:
// Return raw data instead of converted rgb. Using this option assumes you know what you're doing.
#define CAPTURE_OPTION_RAWDATA 1
// Mask to check for valid options - all options OR:ed together.
#define CAPTURE_OPTIONS_MASK (CAPTURE_OPTION_RAWDATA)
#include <mfapi.h>

struct SimpleCapParams
{
    /* Target buffer.
     * Must be at least mWidth * mHeight * sizeof(int) of size.
     * Possibly also the magical 4 instead of sizeof(int).
     */
    unsigned int * mTargetBuf;
    long mBufferBytes;

    /* Image encoding */
    GUID mFormat;
    /* Buffer width */
    int mWidth;
    /* Buffer height */
    int mHeight;

    bool doCapture;
};

enum CAPTURE_PROPETIES
{
    CAPTURE_BRIGHTNESS,
    CAPTURE_CONTRAST,
    CAPTURE_HUE,
    CAPTURE_SATURATION,
    CAPTURE_SHARPNESS,
    CAPTURE_GAMMA,
    CAPTURE_COLORENABLE,
    CAPTURE_WHITEBALANCE,
    CAPTURE_BACKLIGHTCOMPENSATION,
    CAPTURE_GAIN,
    CAPTURE_PAN,
    CAPTURE_TILT,
    CAPTURE_ROLL,
    CAPTURE_ZOOM,
    CAPTURE_EXPOSURE,
    CAPTURE_IRIS,
    CAPTURE_FOCUS,
    CAPTURE_PROP_MAX
};

#include <QMap>
static QMap<int,int> gOptions;

#endif // ESCAPI_MINIMAL_H
