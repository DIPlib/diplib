/*
 * libics: Image Cytometry Standard file reading and writing.
 *
 * Copyright 2015-2017:
 *   Scientific Volume Imaging Holding B.V.
 *   Laapersveld 63, 1213 VB Hilversum, The Netherlands
 *   https://www.svi.nl
 *
 * Contact: libics@svi.nl
 *
 * Copyright (C) 2000-2013 Cris Luengo and others
 *
 * Large chunks of this library written by
 *    Bert Gijsbers
 *    Dr. Hans T.M. van der Voort
 * And also Damir Sudar, Geert van Kempen, Jan Jitze Krol,
 * Chiel Baarslag and Fons Laan.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * FILE : libics.h
 *
 * This is the main include file, and the only file you need to include in your
 * source code if you use the top-level functions in this library.
 */


#ifndef LIBICS_H
#define LIBICS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Library versioning is in the form major, minor, patch: */
#define ICSLIB_VERSION "1.6.2" /* also defined in configure.ac */

#if defined(__WIN32__) && !defined(WIN32)
#define WIN32
#endif


#ifdef WIN32
#ifdef BUILD_ICSLIB
#define ICSEXPORT __declspec(dllexport)
#else
#ifdef USE_ICSLIB_DLL
#define ICSEXPORT __declspec(dllimport)
#else
#define ICSEXPORT
#endif
#endif
#else
#define ICSEXPORT
#endif


/* For older visual studio versions */
#ifdef _MSC_VER
#if(_MSC_VER < 1900)
#define snprintf _snprintf
#endif
#endif


/* For the moment the largest imel is a double complex of 16 bytes: */
#define ICS_MAX_IMEL_SIZE 16


/* These determine the sizes of static arrays and strings: */
#define ICS_MAXDIM 10        /* maximum number of image dimensions.           */
#define ICS_MAX_LAMBDA 32    /* maximum number of channels.                   */
#define ICS_STRLEN_TOKEN 32  /* length of a token string.                     */
#define ICS_STRLEN_OTHER 128 /* length of other strings.                      */
#define ICS_LINE_LENGTH 1024 /* maximum length of the lines in the .ics file. */
#define ICS_MAXPATHLEN 512   /* maximum length of the file names.             */


/* These are the known data types for imels. If you use another type, you can't
   use the top-level functions: */
typedef enum {
    Ics_unknown = 0,
    Ics_uint8,     /* integer, unsigned,  8 bpp */
    Ics_sint8,     /* integer, signed,    8 bpp */
    Ics_uint16,    /* integer, unsigned, 16 bpp */
    Ics_sint16,    /* integer, signed,   16 bpp */
    Ics_uint32,    /* integer, unsigned, 32 bpp */
    Ics_sint32,    /* integer, signed,   32 bpp */
    Ics_real32,    /* real,    signed,   32 bpp */
    Ics_real64,    /* real,    signed,   64 bpp */
    Ics_complex32, /* complex, signed, 2*32 bpp */
    Ics_complex64  /* complex, signed, 2*64 bpp */
} Ics_DataType;


/* The compression methods supported by this library. */
typedef enum {
    IcsCompr_uncompressed = 0, /* No compression                              */
    IcsCompr_compress,         /* Using 'compress' (writing converts to gzip) */
    IcsCompr_gzip              /* Using zlib (ICS_ZLIB must be defined)       */
} Ics_Compression;


/* File modes. */
typedef enum {
    IcsFileMode_write, /* write mode                                  */
    IcsFileMode_read,  /* read mode                                   */
    IcsFileMode_update /*  write only meta-data, read any header item */
} Ics_FileMode;


/* Structures that define the image representation. They are only used inside
   the ICS data structure. */
typedef struct {
    size_t size;                    /* Number of imels in this dimension */
    double origin;                  /* Position of first imel            */
    double scale;                   /* Distance between imels            */
    char   order[ICS_STRLEN_TOKEN]; /* Order of this dimension           */
    char   label[ICS_STRLEN_TOKEN]; /* Label for this dimension          */
    char   unit[ICS_STRLEN_TOKEN];  /* Units for Origin and Scale        */
} Ics_DataRepresentation;


typedef struct {
        /* Numeric representation for the pixels: */
    Ics_DataType dataType;
        /* Number of significant bits: */
    size_t       sigBits;
        /* Offset for imel values: */
    double       origin;
        /* Scaling for imel values: */
    double       scale;
        /* Units for Origin and Scale: */
    char         unit[ICS_STRLEN_TOKEN];
        /* Order is always "bits", Label is always "intensity" */
} Ics_ImelRepresentation;


/* A list of sensor parameters that are also equiped with a state. */
typedef enum {
    ICS_SENSOR_FIRST,
    ICS_SENSOR_IMAGING_DIRECTION,
    ICS_SENSOR_NUMERICAL_APERTURE,
    ICS_SENSOR_OBJECTIVE_QUALITY,
    ICS_SENSOR_MEDIUM_REFRACTIVE_INDEX,
    ICS_SENSOR_LENS_REFRACTIVE_INDEX,
    ICS_SENSOR_PINHOLE_RADIUS,
    ICS_SENSOR_ILL_PINHOLE_RADIUS,
    ICS_SENSOR_PINHOLE_SPACING,
    ICS_SENSOR_EXCITATION_BEAM_FILL,
    ICS_SENSOR_LAMBDA_EXCITATION,
    ICS_SENSOR_LAMBDA_EMISSION,    
    ICS_SENSOR_PHOTON_COUNT,
    ICS_SENSOR_INTERFACE_PRIMARY,
    ICS_SENSOR_INTERFACE_SECONDARY,
    
    ICS_SENSOR_DETECTOR_MAGN,
    ICS_SENSOR_DETECTOR_PPU,
    ICS_SENSOR_DETECTOR_BASELINE,
    ICS_SENSOR_DETECTOR_LINE_AVG_COUNT,

    ICS_SENSOR_STED_DEPLETION_MODE,
    ICS_SENSOR_STED_LAMBDA,
    ICS_SENSOR_STED_SATURATION_FACTOR,
    ICS_SENSOR_STED_IMM_FRACTION,
    ICS_SENSOR_STED_VPPM,
    
    ICS_SENSOR_SPIM_EXCITATION_TYPE,
    ICS_SENSOR_SPIM_FILL_FACTOR,
    ICS_SENSOR_SPIM_PLANE_NA,
    ICS_SENSOR_SPIM_PLANE_GAUSS_WIDTH,
    ICS_SENSOR_SPIM_PLANE_PROP_DIR,
    ICS_SENSOR_SPIM_PLANE_CENTER_OFF,
    ICS_SENSOR_SPIM_PLANE_FOCUS_OFF,
    
    ICS_SENSOR_SCATTER_MODEL,
    ICS_SENSOR_SCATTER_FREE_PATH,
    ICS_SENSOR_SCATTER_REL_CONTRIB,
    ICS_SENSOR_SCATTER_BLURRING,    
    ICS_SENSOR_LAST
} Ics_SensorParameter;


/* Supported sensor state values. */
typedef enum {
    IcsSensorState_default,
    IcsSensorState_estimated,
    IcsSensorState_reported,
    IcsSensorState_verified
} Ics_SensorState;


/* Thee data structure that holds all the information in the ICS file */
typedef struct _ICS {
        /* ICS version: 1 or 2: */
    int                     version;
        /* How the ICS file was opened. Used by top-level only: */
    Ics_FileMode            fileMode;
        /* Pointer to the data to write: */
    const void             *data;
        /* Size of the data buffer: */
    size_t                  dataLength;
        /* Pixel strides (writing only): */
    const ptrdiff_t        *dataStrides;
        /* '.ics' path/filename: */
    char                    filename[ICS_MAXPATHLEN];
        /* Number of elements in each dim: */
    int                     dimensions;
        /* Image representation: */
    Ics_DataRepresentation  dim[ICS_MAXDIM];
        /* Pixel representation: */
    Ics_ImelRepresentation  imel;
        /* Coordinate system used: */
    char                    coord[ICS_STRLEN_TOKEN];
        /* Compression technique used: */
    Ics_Compression         compression;
        /* Compression level: */
    int                     compLevel;
        /* Byte storage order: */
    int                     byteOrder[ICS_MAX_IMEL_SIZE];
        /* History strings: */
    void*                   history;
        /* Status of the data file: */
    void*                   blockRead;
        /* ICS2: Source file name: */
    char                    srcFile[ICS_MAXPATHLEN];
        /* ICS2: Offset into source file: */
    size_t                  srcOffset;
        /* Set to 1 if the next params are needed: */
    int                     writeSensor;
        /* Set to 1 if the next param states are needed: */
    int                     writeSensorStates;
        /* Sensor type: */
    char                    type[ICS_MAX_LAMBDA][ICS_STRLEN_TOKEN];
        /* Model or make: */
    char                    model[ICS_STRLEN_OTHER];
        /* Number of channels: */
    int                     sensorChannels;
        /* Imaging direction: */
    char                    imagingDirection[ICS_MAX_LAMBDA][ICS_STRLEN_TOKEN];
    Ics_SensorState         imagingDirectionState[ICS_MAX_LAMBDA];
        /* Numerical Aperture: */
    double                  numAperture;
    Ics_SensorState         numApertureState;
        /* Objective quality: */
    int                     objectiveQuality[ICS_MAX_LAMBDA];
    Ics_SensorState         objectiveQualityState[ICS_MAX_LAMBDA];
        /* Refractive index of embedding medium: */
    double                  refrInxMedium;
    Ics_SensorState         refrInxMediumState;
        /* Refractive index of design medium: */
    double                  refrInxLensMedium;
    Ics_SensorState         refrInxLensMediumState;
        /* Detection pinhole in microns: */
    double                  pinholeRadius[ICS_MAX_LAMBDA];
    Ics_SensorState         pinholeRadiusState[ICS_MAX_LAMBDA];
        /* Illumination pinhole in microns: */
    double                  illPinholeRadius[ICS_MAX_LAMBDA];
    Ics_SensorState         illPinholeRadiusState[ICS_MAX_LAMBDA];
        /* Nipkow Disk pinhole spacing: */
    double                  pinholeSpacing;
    Ics_SensorState         pinholeSpacingState;
        /* Excitation beam fill factor: */
    double                  excitationBeamFill[ICS_MAX_LAMBDA];
    Ics_SensorState         excitationBeamFillState[ICS_MAX_LAMBDA];
        /* Excitation wavelength in nanometers: */
    double                  lambdaEx[ICS_MAX_LAMBDA];
    Ics_SensorState         lambdaExState[ICS_MAX_LAMBDA];
        /* Emission wavelength in nm: */
    double                  lambdaEm[ICS_MAX_LAMBDA];
    Ics_SensorState         lambdaEmState[ICS_MAX_LAMBDA];
        /* Number of excitation photons: */
    int                     exPhotonCnt[ICS_MAX_LAMBDA];
    Ics_SensorState         exPhotonCntState[ICS_MAX_LAMBDA];
        /* Emission wavelength in nm: */
    double                  interfacePrimary;
    Ics_SensorState         interfacePrimaryState;
        /* Emission wavelength in nm: */
    double                  interfaceSecondary;
    Ics_SensorState         interfaceSecondaryState;
        /* Excitation beam fill factor: */
    double                  detectorMagn[ICS_MAX_LAMBDA];
    Ics_SensorState         detectorMagnState[ICS_MAX_LAMBDA];
        /* Detector photons per unit: */
    double                  detectorPPU[ICS_MAX_LAMBDA];
    Ics_SensorState         detectorPPUState[ICS_MAX_LAMBDA];
        /* Detector Baseline: */
    double                  detectorBaseline[ICS_MAX_LAMBDA];
    Ics_SensorState         detectorBaselineState[ICS_MAX_LAMBDA];
        /* Averaging line count */
    double                  detectorLineAvgCnt[ICS_MAX_LAMBDA];
    Ics_SensorState         detectorLineAvgCntState[ICS_MAX_LAMBDA];
        /* STED depletion mode: */
    char                    stedDepletionMode[ICS_MAX_LAMBDA][ICS_STRLEN_TOKEN];
    Ics_SensorState         stedDepletionModeState[ICS_MAX_LAMBDA];
        /* STED wavelength: */
    double                  stedLambda[ICS_MAX_LAMBDA];
    Ics_SensorState         stedLambdaState[ICS_MAX_LAMBDA];
        /* STED saturation factor: */
    double                  stedSatFactor[ICS_MAX_LAMBDA];
    Ics_SensorState         stedSatFactorState[ICS_MAX_LAMBDA];
        /* STED immunity fraction: */
    double                  stedImmFraction[ICS_MAX_LAMBDA];
    Ics_SensorState         stedImmFractionState[ICS_MAX_LAMBDA];
        /* STED vortex to phase plate mix: */
    double                  stedVPPM[ICS_MAX_LAMBDA];
    Ics_SensorState         stedVPPMState[ICS_MAX_LAMBDA];
        /* SPIM excitation type: */
    char                    spimExcType[ICS_MAX_LAMBDA][ICS_STRLEN_TOKEN];
    Ics_SensorState         spimExcTypeState[ICS_MAX_LAMBDA];
        /* SPIM fill factor: */
    double                  spimFillFactor[ICS_MAX_LAMBDA];
    Ics_SensorState         spimFillFactorState[ICS_MAX_LAMBDA];
        /* SPIM plane NA: */
    double                  spimPlaneNA[ICS_MAX_LAMBDA];
    Ics_SensorState         spimPlaneNAState[ICS_MAX_LAMBDA];
        /* SPIM plane Gaussian width: */
    double                  spimPlaneGaussWidth[ICS_MAX_LAMBDA];
    Ics_SensorState         spimPlaneGaussWidthState[ICS_MAX_LAMBDA];
        /* SPIM plane propagation directory (a vector of 3 doubles): */
    double                  spimPlanePropDir[ICS_MAX_LAMBDA][3];
    Ics_SensorState         spimPlanePropDirState[ICS_MAX_LAMBDA];
        /* SPIM plane center offset : */
    double                  spimPlaneCenterOff[ICS_MAX_LAMBDA];
    Ics_SensorState         spimPlaneCenterOffState[ICS_MAX_LAMBDA];
        /* SPIM plane focus offset: */
    double                  spimPlaneFocusOff[ICS_MAX_LAMBDA];
    Ics_SensorState         spimPlaneFocusOffState[ICS_MAX_LAMBDA];
        /* Scatter model: */
    char                    scatterModel[ICS_MAX_LAMBDA][ICS_STRLEN_TOKEN];
    Ics_SensorState         scatterModelState[ICS_MAX_LAMBDA];
        /* Scatter free path: */
    double                  scatterFreePath[ICS_MAX_LAMBDA];
    Ics_SensorState         scatterFreePathState[ICS_MAX_LAMBDA];
        /* Scatter relative contribution: */
    double                  scatterRelContrib[ICS_MAX_LAMBDA];
    Ics_SensorState         scatterRelContribState[ICS_MAX_LAMBDA];
        /* Scatter blurring: */
    double                  scatterBlurring[ICS_MAX_LAMBDA];
    Ics_SensorState         scatterBlurringState[ICS_MAX_LAMBDA];

        /* SCIL_Image compatibility parameter: */
    char                    scilType[ICS_STRLEN_TOKEN];
} ICS;


/* The  error codes. */
typedef enum {
        /* No error. */
    IcsErr_Ok                         = 0,
        /* Non fatal error: unexpected data size: */
    IcsErr_FSizeConflict,
        /* Non fatal error: the output buffer could not be completely filled
           (meaning that your buffer was too large): */
    IcsErr_OutputNotFilled,
        /* Memory allocation error: */
    IcsErr_Alloc,
        /* Image size conflicts with bits per element: */
    IcsErr_BitsVsSizeConfl,
        /* It is not possible to read COMPRESS-compressed data in blocks: */
    IcsErr_BlockNotAllowed,
        /* The buffer was too small to hold the given ROI: */
    IcsErr_BufferTooSmall,
        /* Some error occurred during compression: */
    IcsErr_CompressionProblem,
        /* The compressed input stream is currupted: */
    IcsErr_CorruptedStream,
        /* Some error occurred during decompression: */
    IcsErr_DecompressionProblem,
        /* The ICS data structure already contains incompatible stuff: */
    IcsErr_DuplicateData,
        /* Empty field (intern error): */
    IcsErr_EmptyField,
        /* All history lines have already been returned: */
    IcsErr_EndOfHistory,
        /* Unexpected end of stream: */
    IcsErr_EndOfStream,
        /* File close error on .ics file: */
    IcsErr_FCloseIcs,
        /* File close error on .ids file: */
    IcsErr_FCloseIds,
        /* Failed to copy image data from temporary file on .ics file opened for
           updating: */
    IcsErr_FCopyIds,
        /* File open error on .ics file: */
    IcsErr_FOpenIcs,
        /* File open error on .ids file: */
    IcsErr_FOpenIds,
        /* File read error on .ics file: */
    IcsErr_FReadIcs,
        /* File read error on .ids file: */
    IcsErr_FReadIds,
        /* Failed to remane .ics file opened for updating: */
    IcsErr_FTempMoveIcs,
        /* File write error on .ics file: */
    IcsErr_FWriteIcs,
        /* File write error on .ids file: */
    IcsErr_FWriteIds,
        /* Failed to write a line in .ics file: */
    IcsErr_FailWriteLine,
        /* Illegal ICS token detected: */
    IcsErr_IllIcsToken,
        /* A function parameter has a value that is not legal or does not match
           with a value previously given: */
    IcsErr_IllParameter,
        /* The given ROI extends outside the image: */
    IcsErr_IllegalROI,
        /* Line overflow in ics file: */
    IcsErr_LineOverflow,
        /* Missing "bits" element in .ics file: */
    IcsErr_MissBits,
        /* Missing main category: */
    IcsErr_MissCat,
        /* Missing layout subcategory: */
    IcsErr_MissLayoutSubCat,
        /* Missing parameter subcategory: */
    IcsErr_MissParamSubCat,
        /* Missing representation subcategory: */
    IcsErr_MissRepresSubCat,
        /* Missing sensor subcategory: */
    IcsErr_MissSensorSubCat,
        /* Missing sensor subsubcategory: */
    IcsErr_MissSensorSubSubCat,
        /* Missing sub category: */
    IcsErr_MissSubCat,
        /* There is no Data defined: */
    IcsErr_MissingData,
        /* Layout parameters missing or not defined: */
    IcsErr_NoLayout,
        /* There doesn't exist a SCIL_TYPE value for this image: */
    IcsErr_NoScilType,
        /* Not an ICS file: */
    IcsErr_NotIcsFile,
        /* The function won't work on the ICS given: */
    IcsErr_NotValidAction,
        /* Too many channels specified: */
    IcsErr_TooManyChans,
        /* Data has too many dimensions: */
    IcsErr_TooManyDims,
        /* Unknown compression type: */
    IcsErr_UnknownCompression,
         /* The datatype is not recognized: */
    IcsErr_UnknownDataType,
        /* The state is unknown. */
    IcsErr_UnknownSensorState,
        /* libics is linking to a different version of zlib than used during
           compilation: */
    IcsErr_WrongZlibVersion
} Ics_Error;


/* Used by IcsGetHistoryString. */
typedef enum {
    IcsWhich_First, /* Get the first string */
    IcsWhich_Next  /* Get the next string */
} Ics_HistoryWhich;


typedef struct {
    int  next;                    /* index into history array, pointing to next
                                     string to read, set to -1 if there's no
                                     more to read. */
    int  previous;                /* index to previous string, useful for relace
                                     and delete. */
    char key[ICS_STRLEN_TOKEN+1]; /* optional key this iterator looks for. */
} Ics_HistoryIterator;


/* Returns a string that can be used to compare with ICSLIB_VERSION to check if
   the version of the library is the same as that of the headers. */
ICSEXPORT const char* IcsGetLibVersion(void);


/* Returns 0 if it is not an ICS file, or the version number if it is.  If
  forcename is non-zero, no extension is appended. */
ICSEXPORT int IcsVersion(const char *filename,
                         int         forceName);


/* Read a preview (2D) image out of an ICS file. The buffer is malloc'd, xsize
   and ysize are set to the image size. The data type is always uint8. You need
   to free() the data block when you're done. */
ICSEXPORT Ics_Error IcsLoadPreview(const char  *filename,
                                   size_t       planeNumber,
                                   void       **dest,
                                   size_t      *xsize,
                                   size_t      *ysize);


/* Open an ICS file for reading (mode = "r") or writing (mode = "w"). When
   writing, append a "2" to the mode string to create an ICS version 2.0
   file. Append an "f" to mode if, when reading, you want to force the file name
   to not change (no ".ics" is appended). Append a "l" to mode if, when reading,
   you don't want the locale forced to "C" (to read ICS files written with some
   other locale, set the locale properly then open the file with "rl") */
ICSEXPORT Ics_Error IcsOpen(ICS        **ics,
                            const char  *filename,
                            const char  *mode);


/* Close the ICS file. The ics 'stream' is no longer valid after this.  No files
   are actually written until this function is called. */
ICSEXPORT Ics_Error IcsClose(ICS* ics);


/* Retrieve the layout of an ICS image. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetLayout(const ICS    *ics,
                                 Ics_DataType *dt,
                                 int          *nDims,
                                 size_t       *dims);


/* Set the layout for an ICS image. Only valid if writing. */
ICSEXPORT Ics_Error IcsSetLayout(ICS*          ics,
                                 Ics_DataType  dt,
                                 int           nDims,
                                 const size_t *dims);


/* These three functions retrieve info from the ICS file.  IcsGetDataSize(ics)
   == IcsGetImelSize(ics) * IcsGetImageSize(ics) */
ICSEXPORT size_t IcsGetDataSize(const ICS *ics);
ICSEXPORT size_t IcsGetImelSize(const ICS *ics);
ICSEXPORT size_t IcsGetImageSize(const ICS *ics);


/* Read the image data from an ICS file. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetData(ICS   *ics,
                                void  *dest,
                                size_t n);


/* Read a square region of the image from an ICS file. To use the defaults in
   one of the parameters, set the pointer to NULL. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetROIData(ICS          *ics,
                                  const size_t *offset,
                                  const size_t *size,
                                  const size_t *sampling,
                                  void         *dest,
                                  size_t        n);


/* Read the image from an ICS file into a sub-block of a memory block. To use
   the defaults in one of the parameters, set the pointer to NULL. Only valid if
   reading. */
ICSEXPORT Ics_Error IcsGetDataWithStrides(ICS             *ics,
                                          void            *dest,
                                          size_t           n, // ignored
                                          const ptrdiff_t *stride,
                                          int              nDims);


/* Read a portion of the image data from an ICS file. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetDataBlock(ICS   *ics,
                                    void  *dest,
                                    size_t n);


/* Skip a portion of the image from an ICS file. Only valid if reading. */
ICSEXPORT Ics_Error IcsSkipDataBlock(ICS   *ics,
                                     size_t  n);


/* Read a plane of the image data from an ICS file, and convert it to
   uint8. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetPreviewData(ICS    *ics,
                                      void   *dest,
                                      size_t  n,
                                      size_t  planeNumber);


/* Set the image data for an ICS image. The pointer to this data must be
   accessible until IcsClose has been called. Only valid if writing. */
ICSEXPORT Ics_Error IcsSetData(ICS        *ics,
                               const void *src,
                               size_t      n);


/* Set the image data for an ICS image. The pointer to this data must be
   accessible until IcsClose has been called. Only valid if writing. */
ICSEXPORT Ics_Error IcsSetDataWithStrides(ICS             *ics,
                                          const void      *src,
                                          size_t           n,
                                          const ptrdiff_t *strides,
                                          int              nDims);

/* Set the image source parameter for an ICS version 2.0 file. Only valid if
   writing. */
ICSEXPORT Ics_Error IcsSetSource(ICS        *ics,
                                 const char *fname,
                                 size_t      offset);


/* Set the compression method and compression parameter. Only valid if
   writing. */
ICSEXPORT Ics_Error IcsSetCompression(ICS             *ics,
                                      Ics_Compression  compression,
                                      int              level);


/* Get the position of the image in the real world: the origin of the first
   pixel, the distances between pixels and the units in which to measure.  If
   you are not interested in one of the parameters, set the pointer to NULL.
   Dimensions start at 0. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetPosition(const ICS *ics,
                                   int        dimension,
                                   double    *origin,
                                   double    *scale,
                                   char*      units);

/* Idem, but without copying the strings. Output pointer `units` set to internal
   buffer, which will be valid until IcsClose is called. */
ICSEXPORT Ics_Error IcsGetPositionF(const ICS   *ics,
                                    int          dimension,
                                    double      *origin,
                                    double      *scale,
                                    const char **units);


/* Set the position of the image in the real world: the origin of the first
   pixel, the distances between pixels and the units in which to measure.  If
   units is NULL or empty, it is set to the default value of "undefined".
   Dimensions start at 0. Only valid if writing. */
ICSEXPORT Ics_Error IcsSetPosition(ICS*        ics,
                                   int         dimension,
                                   double      origin,
                                   double      scale,
                                   const char *units);


/* Get the ordering of the dimensions in the image. The ordering is defined by
   names and labels for each dimension. The defaults are x, y, z, t (time) and p
   (probe). Dimensions start at 0. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetOrder(const ICS *ics,
                                int        dimension,
                                char      *order,
                                char      *label);

/* Idem, but without copying the strings. Output pointers `order` and `label`
   set to internal buffer, which will be valid until IcsClose is called. */
ICSEXPORT Ics_Error IcsGetOrderF(const ICS   *ics,
                                 int          dimension,
                                 const char **order,
                                 const char **label);


/* Set the ordering of the dimensions in the image. The ordering is defined by
   providing names and labels for each dimension. The defaults are x, y, z, t
   (time) and p (probe). Dimensions start at 0. Only valid if writing. */
ICSEXPORT Ics_Error IcsSetOrder(ICS        *ics,
                                int         dimension,
                                const char *order,
                                const char *label);


/* Get the coordinate system used in the positioning of the pixels.  Related to
   IcsGetPosition(). The default is "video". Only valid if reading. */
ICSEXPORT Ics_Error IcsGetCoordinateSystem(const ICS *ics,
                                           char      *coord);


/* Set the coordinate system used in the positioning of the pixels.  Related to
   IcsSetPosition(). The default is "video". Only valid if writing. */
ICSEXPORT Ics_Error IcsSetCoordinateSystem(ICS        *ics,
                                           const char *coord);


/* Get the number of significant bits. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetSignificantBits(const ICS *ics,
                                          size_t    *nBits);



/* Set the number of significant bits. Only valid if writing. */
ICSEXPORT Ics_Error IcsSetSignificantBits(ICS   *ics,
                                          size_t nBits);


/* Set the position of the pixel values: the offset and scaling, and the units
   in which to measure. If you are not interested in one of the parameters, set
   the pointer to NULL. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetImelUnits(const ICS *ics,
                                    double    *origin,
                                    double    *scale,
                                    char      *units);

/* Idem, but without copying the strings. Output pointer `units` set to internal
   buffer, which will be valid until IcsClose is called. */
ICSEXPORT Ics_Error IcsGetImelUnitsF(const ICS   *ics,
                                     double      *origin,
                                     double      *scale,
                                     const char **units);


/* Set the position of the pixel values: the offset and scaling, and the units
   in which to measure. If units is NULL or empty, it is set to the default
   value of "relative". Only valid if writing. */
ICSEXPORT Ics_Error IcsSetImelUnits(ICS        *ics,
                                    double      origin,
                                    double      scale,
                                    const char *units);


/* Get the string for the SCIL_TYPE parameter. This string is used only by
   SCIL_Image. Only valid if reading. */
ICSEXPORT Ics_Error IcsGetScilType(const ICS *ics,
                                   char      *scilType);


/* Set the string for the SCIL_TYPE parameter. This string is used only by
   SCIL_Image. It is required if you want to read the image using
   SCIL_Image. Only valid if writing. */
ICSEXPORT Ics_Error IcsSetScilType(ICS        *ics,
                                   const char *scilType);


/* As IcsSetScilType, but creates a string according to the DataType in the ICS
   structure. It can create a string for g2d, g3d, f2d, f3d, c2d and c3d. Only
   valid if writing. */
ICSEXPORT Ics_Error IcsGuessScilType(ICS *ics);


/* Returns a textual representation of an error. */
ICSEXPORT const char *IcsGetErrorText(Ics_Error error);


/* Add history lines to the ICS file. key can be NULL */
ICSEXPORT Ics_Error IcsAddHistoryString(ICS        *ics,
                                        const char *key,
                                        const char *value);
#define IcsAddHistory IcsAddHistoryString


/* Delete all history lines with key from ICS file. key can be NULL, deletes
   all. */
ICSEXPORT Ics_Error IcsDeleteHistory(ICS        *ics,
                                     const char *key);


/* Get the number of HISTORY lines from the ICS file. */
ICSEXPORT Ics_Error IcsGetNumHistoryStrings(ICS *ics,
                                            int *num);


/* Get history line from the ICS file. string must have at least ICS_LINE_LENGTH
   characters allocated. */
ICSEXPORT Ics_Error IcsGetHistoryString(ICS             *ics,
                                        char            *string,
                                        Ics_HistoryWhich which);


/* Get history line from the ICS file as key/value pair. key must have
   ICS_STRLEN_TOKEN characters allocated, and value ICS_LINE_LENGTH.  key can be
   null, token will be discarded. */
ICSEXPORT Ics_Error IcsGetHistoryKeyValue(ICS*             ics,
                                          char*            key,
                                          char*            value,
                                          Ics_HistoryWhich which);


/* Initializes history iterator. key can be NULL. */
ICSEXPORT Ics_Error IcsNewHistoryIterator(ICS                 *ics,
                                          Ics_HistoryIterator *it,
                                          const char          *key);


/* Get history line from the ICS file using iterator. string must have at least
   ICS_LINE_LENGTH characters allocated. */
ICSEXPORT Ics_Error IcsGetHistoryStringI(ICS                 *ics,
                                         Ics_HistoryIterator *it,
                                         char                *string);

/* Idem, but without copying the string. Output pointer `string` set to internal
   buffer, which will be valid until IcsClose or IcsFreeHistory is called. */
ICSEXPORT Ics_Error IcsGetHistoryStringIF(ICS                 *ics,
                                          Ics_HistoryIterator *it,
                                          const char         **string);


/* Get history line from the ICS file as key/value pair using iterator.  key
   must have ICS_STRLEN_TOKEN characters allocated, and value
   ICS_LINE_LENGTH. key can be null, token will be discarded. */
ICSEXPORT Ics_Error IcsGetHistoryKeyValueI(ICS                 *ics,
                                           Ics_HistoryIterator *it,
                                           char                *key,
                                           char                *value);

/* Idem, but without copying the string. Output pointer `value` set to internal
   buffer, which will be valid until IcsClose or IcsFreeHistory is called. */
ICSEXPORT Ics_Error IcsGetHistoryKeyValueIF(ICS                 *ics,
                                            Ics_HistoryIterator *it,
                                            char                *key,
                                            const char         **value);


/* Delete last retrieved history line (iterator still points to the same
   string). */
ICSEXPORT Ics_Error IcsDeleteHistoryStringI(ICS                 *ics,
                                            Ics_HistoryIterator *it);


/* Delete last retrieved history line (iterator still points to the same
   string). */
ICSEXPORT Ics_Error IcsReplaceHistoryStringI(ICS                 *ics,
                                             Ics_HistoryIterator *it,
                                             const char          *key,
                                             const char          *value);


#ifdef __cplusplus
}
#endif

#endif
