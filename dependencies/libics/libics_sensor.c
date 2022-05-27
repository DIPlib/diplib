/*
 * libics: Image Cytometry Standard file reading and writing.
 *
 * Copyright 2015-2019:
 *   Scientific Volume Imaging Holding B.V.
 *   Laapersveld 63, 1213 VB Hilversum, The Netherlands
 *   https://www.svi.nl
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
 * FILE : libics_sensor.c
 * Written by Bert Gijsbers, Scientific Volume Imaging BV, Hilversum, NL
 *
 * The following library functions are contained in this file:
 *
 *   IcsEnableWriteSensor
 *   IcsSetSensorType
 *   IcsSetSensorModel
 *   IcsSetSensorChannels
 *   IcsSetSensorPinholeRadius
 *   IcsSetSensorExcitationWavelength
 *   IcsSetSensorEmissionWavelength
 *   IcsSetSensorPhotonCount
 *   IcsSetSensorMediumRI
 *   IcsSetSensorLensRI
 *   IcsSetSensorNumAperture
 *   IcsSetSensorPinholeSpacing
 *   IcsSetSTEDMode
 *   IcsSetSTEDLambda
 *   IcsSetSTEDSaturationFactor
 *   IcsSetSTEDImmunityFactor
 *   IcsSetSTEDVortexToPhasePlateMix
 *   IcsSetDetectorPPU
 *   IcsSetDetectorBaseline
 *   IcsSetDetectorLineAvgCnt
 *   IcsSetDetectorNoiseGain
 */


#include <stdlib.h>
#include <string.h>
#include "libics_sensor.h"
#include "libics_intern.h"


/* This function enables writing the sensor parameters to disk. */
Ics_Error IcsEnableWriteSensor(ICS *ics,
                               int  enable)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    ics->writeSensor = enable ? 1 : 0;
    return IcsErr_Ok;
}


/* This function enables writing the sensor parameter statess to disk. */
Ics_Error IcsEnableWriteSensorStates(ICS *ics,
                                     int  enable)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    ics->writeSensorStates = enable ? 1 : 0;
    return IcsErr_Ok;
}


/* Get the sensor type string of a sensor channel. */
char const* IcsGetSensorType(const ICS *ics,
                             int        channel)
{
    return ics->type[channel];
}


/* Set the sensor type string for a sensor channel. */
Ics_Error IcsSetSensorType(ICS        *ics,
                           int         channel,
                           const char *sensorType)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    IcsStrCpy(ics->type[channel], sensorType, sizeof(ics->type[channel]));
    return IcsErr_Ok;
}


/* Get the sensor model string. */
const char *IcsGetSensorModel(const ICS *ics)
{
    return ics->model;
}


/* Set the sensor model string. */
Ics_Error IcsSetSensorModel(ICS        *ics,
                            const char *sensorModel)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    IcsStrCpy(ics->model, sensorModel, sizeof(ics->model));
    return IcsErr_Ok;
}


/* Get the number of sensor channels. */
int IcsGetSensorChannels(const ICS *ics)
{
    return ics->sensorChannels;
}


/* Set the number of sensor channels. */
Ics_Error IcsSetSensorChannels(ICS *ics,
                               int  channels)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channels < 0 || channels > ICS_MAX_LAMBDA)
        return IcsErr_NotValidAction;
    ics->sensorChannels = channels;
    return IcsErr_Ok;
}


/* Get the number of sensor detectors. */
int IcsGetSensorDetectors(const ICS *ics)
{
    return ics->sensorDetectors;
}


/* Set the number of sensor detectors. */
Ics_Error IcsSetSensorDetectors(ICS *ics,
                               int  detectors)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (detectors < 0 || detectors > ICS_MAX_DETECT)
        return IcsErr_NotValidAction;
    ics->sensorDetectors = detectors;
    return IcsErr_Ok;
}


/* Get the pinhole radius for a sensor channel. */
double IcsGetSensorPinholeRadius(const ICS *ics,
                                 int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->pinholeRadius[channel];
}


/* Set the pinhole radius for a sensor channel. */
Ics_Error IcsSetSensorPinholeRadius(ICS    *ics,
                                    int     channel,
                                    double  radius)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->pinholeRadius[channel] = radius;
    return IcsErr_Ok;
}


/* Get the excitation wavelength for a sensor channel. */
double IcsGetSensorExcitationWavelength(const ICS *ics,
                                        int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->lambdaEx[channel];
}


/* Set the excitation wavelength for a sensor channel. */
Ics_Error IcsSetSensorExcitationWavelength(ICS    *ics,
                                           int     channel,
                                           double  wl)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->lambdaEx[channel] = wl;
    return IcsErr_Ok;
}


/* Get the emission wavelength for a sensor channel. */
double IcsGetSensorEmissionWavelength(const ICS *ics,
                                      int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->lambdaEm[channel];
}


/* Set the emission wavelength for a sensor channel. */
Ics_Error IcsSetSensorEmissionWavelength(ICS    *ics,
                                         int     channel,
                                         double  wl)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->lambdaEm[channel] = wl;
    return IcsErr_Ok;
}


/* Get the excitation photon count for a sensor channel. */
int IcsGetSensorPhotonCount(const ICS *ics,
                            int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->exPhotonCnt[channel];
}


/* Set the excitation photon count for a sensor channel. */
Ics_Error IcsSetSensorPhotonCount(ICS *ics,
                                  int  channel,
                                  int  cnt)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->exPhotonCnt[channel] = cnt;
    return IcsErr_Ok;
}


/* Get the sensor embedding medium refractive index. */
double IcsGetSensorMediumRI(const ICS *ics)
{
    return ics->refrInxMedium;
}


/* Set the sensor embedding medium refractive index. */
Ics_Error IcsSetSensorMediumRI(ICS    *ics,
                               double  ri)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    ics->refrInxMedium = ri;
    return IcsErr_Ok;
}


/* Get the sensor design medium refractive index. */
double IcsGetSensorLensRI(const ICS *ics)
{
    return ics->refrInxLensMedium;
}


/* Set the sensor design medium refractive index. */
Ics_Error IcsSetSensorLensRI(ICS    *ics,
                             double  ri)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    ics->refrInxLensMedium = ri;
    return IcsErr_Ok;
}


/* Get the sensor numerical apperture */
double IcsGetSensorNumAperture(const ICS *ics)
{
    return ics->numAperture;
}


/* Set the sensor numerical apperture */
Ics_Error IcsSetSensorNumAperture(ICS    *ics,
                                  double  na)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    ics->numAperture = na;
    return IcsErr_Ok;
}


/* Get the sensor Nipkow Disk pinhole spacing. */
double IcsGetSensorPinholeSpacing(const ICS *ics)
{
    return ics->pinholeSpacing;
}


/* Set the sensor Nipkow Disk pinhole spacing. */
Ics_Error IcsSetSensorPinholeSpacing(ICS    *ics,
                                     double  spacing)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    ics->pinholeSpacing = spacing;
    return IcsErr_Ok;
}


/* Get the STED mode per channel. */
const char * IcsGetSensorSTEDDepletionMode(const ICS *ics,
                                           int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->stedDepletionMode[channel];
}


/* Set the STED depletion mode per channel. */
Ics_Error IcsSetSensorSTEDDepletionMode(ICS        *ics,
                                        int         channel,
                                        const char *depletionMode)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    IcsStrCpy(ics->stedDepletionMode[channel], depletionMode,
              sizeof(ics->stedDepletionMode[channel]));
    return IcsErr_Ok;
}


/* Get the STED inhibition wavelength per channel. */
double IcsGetSensorSTEDLambda(const ICS *ics,
                              int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->stedLambda[channel];
}


/* Set the STED inhibition wavelength per channel. */
Ics_Error IcsSetSensorSTEDLambda(ICS    *ics,
                                 int     channel,
                                 double  lambda)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->stedLambda[channel] = lambda;
    return IcsErr_Ok;
}


/* Get the STED saturation factor per channel. */
double IcsGetSensorSTEDSatFactor(const ICS *ics,
                                 int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->stedSatFactor[channel];
}


/* Set the STED saturation factor per channel. */
Ics_Error IcsSetSensorSTEDSatFactor(ICS    *ics,
                                    int     channel,
                                    double  factor)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->stedSatFactor[channel] = factor;
    return IcsErr_Ok;
}


/* Get the STED immunity fraction per channel. */
double IcsGetSensorSTEDImmFraction(const ICS *ics,
                                   int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->stedImmFraction[channel];
}


/* Set the STED immunity fraction per channel. */
Ics_Error IcsSetSensorSTEDImmFraction(ICS    *ics,
                                      int     channel,
                                      double  fraction)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->stedImmFraction[channel] = fraction;
    return IcsErr_Ok;
}


/* Get the STED vortex to phase plate mix per channel. */
double IcsGetSensorSTEDVPPM(const ICS *ics,
                            int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->stedVPPM[channel];
}


/* Set the STED vortex to phase plate mix per channel. */
Ics_Error IcsSetSensorSTEDVPPM(ICS    *ics,
                               int     channel,
                               double  vppm)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->stedVPPM[channel] = vppm;
    return IcsErr_Ok;
}



/* Get the Detector ppu per channel. */
double IcsGetSensorDetectorPPU(const ICS *ics,
                               int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->detectorPPU[channel];
}


/* Set the Detector ppu per channel. */
Ics_Error IcsSetSensorDetectorPPU(ICS    *ics,
                                  int     channel,
                                  double  ppu)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->detectorPPU[channel] = ppu;
    return IcsErr_Ok;
}


/* Get the Detector baseline per channel. */
double IcsGetSensorDetectorBaseline(const ICS *ics,
                                    int        channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->detectorBaseline[channel];
}


/* Set the Detector baseline per channel. */
Ics_Error IcsSetSensorDetectorBaseline(ICS    *ics,
                                       int     channel,
                                       double  baseline)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->detectorBaseline [channel] = baseline;
    return IcsErr_Ok;
}


/* Get the Detector lineAvgCnt per channel. */
double IcsGetSensorDetectorLineAvgCnt(const ICS *ics,
                                      int channel)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return 0;
    else
        return ics->detectorLineAvgCnt[channel];
}


 /* Set the Detector lineAvgCnt per channel. */
Ics_Error IcsSetSensorDetectorLineAvgCnt(ICS    *ics,
                                         int     channel,
                                         double  lineAvgCnt)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
    ics->detectorLineAvgCnt[channel] = lineAvgCnt;
    return IcsErr_Ok;
}


/* Get the state of a sensor parameter. */
Ics_Error IcsGetSensorParameter(const ICS           *ics,
                                Ics_SensorParameter  parameter,
                                int                  channel,
                                double              *value,
                                Ics_SensorState     *state)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;

    switch (parameter) {
        case ICS_SENSOR_NUMERICAL_APERTURE:
            *value = ics->numAperture;
            *state = ics->numApertureState;
            break;
        case ICS_SENSOR_MEDIUM_REFRACTIVE_INDEX:
            *value = ics->refrInxMedium;
            *state = ics->refrInxMediumState;
            break;
        case ICS_SENSOR_LENS_REFRACTIVE_INDEX:
            *value = ics->refrInxLensMedium;
            *state = ics->refrInxLensMediumState;
            break;
        case ICS_SENSOR_PINHOLE_RADIUS:
            *value = ics->pinholeRadius[channel];
            *state = ics->pinholeRadiusState[channel];
            break;
        case ICS_SENSOR_ILL_PINHOLE_RADIUS:
            *value = ics->illPinholeRadius[channel];
            *state = ics->illPinholeRadiusState[channel];
            break;
        case ICS_SENSOR_PINHOLE_SPACING:
            *value = ics->pinholeSpacing;
            *state = ics->pinholeSpacingState;
            break;
        case ICS_SENSOR_EXCITATION_BEAM_FILL:
            *value = ics->excitationBeamFill[channel];
            *state = ics->excitationBeamFill[channel];
            break;
        case ICS_SENSOR_LAMBDA_EXCITATION:
            *value = ics->lambdaEx[channel];
            *state = ics->lambdaExState[channel];
            break;
        case ICS_SENSOR_LAMBDA_EMISSION:
            *value = ics->lambdaEm[channel];
            *state = ics->lambdaEmState[channel];
            break;
        case ICS_SENSOR_PHOTON_COUNT:
            *value = ics->exPhotonCnt[channel];
            *state = ics->exPhotonCntState[channel];
            break;
        case ICS_SENSOR_INTERFACE_PRIMARY:
            *value = ics->interfacePrimary;
            *state = ics->interfacePrimaryState;
            break;
        case ICS_SENSOR_INTERFACE_SECONDARY:
            *value = ics->interfaceSecondary;
            *state = ics->interfaceSecondaryState;
            break;
         case ICS_SENSOR_DETECTOR_MAGN:
            *value = ics->detectorMagn[channel];
            *state = ics->detectorMagnState[channel];
            break;
         case ICS_SENSOR_DETECTOR_PPU:
            *value = ics->detectorPPU[channel];
            *state = ics->detectorPPUState[channel];
            break;
        case ICS_SENSOR_DETECTOR_BASELINE:
            *value = ics->detectorBaseline[channel];
            *state = ics->detectorBaselineState[channel];
            break;
        case ICS_SENSOR_DETECTOR_LINE_AVG_COUNT:
            *value = ics->detectorLineAvgCnt[channel];
            *state = ics->detectorLineAvgCntState[channel];
            break;
        case ICS_SENSOR_DETECTOR_NOISE_GAIN:
            *value = ics->detectorNoiseGain[channel];
            *state = ics->detectorNoiseGainState[channel];
            break;
        case ICS_SENSOR_DETECTOR_SCALE:
            *value = ics->detectorScale[channel];
            *state = ics->detectorScaleState[channel];
            break;
        case ICS_SENSOR_DETECTOR_STRETCH:
            *value = ics->detectorStretch[channel];
            *state = ics->detectorStretchState[channel];
            break;
        case ICS_SENSOR_DETECTOR_ROT:
            *value = ics->detectorRot[channel];
            *state = ics->detectorRotState[channel];
            break;
        case ICS_SENSOR_STED_LAMBDA:
            *value = ics->stedLambda[channel];
            *state = ics->stedLambdaState[channel];
            break;
        case ICS_SENSOR_STED_SATURATION_FACTOR:
            *value = ics->stedSatFactor[channel];
            *state = ics->stedSatFactorState[channel];
            break;
        case ICS_SENSOR_STED_IMM_FRACTION:
            *value = ics->stedImmFraction[channel];
            *state = ics->stedImmFractionState[channel];
            break;
        case ICS_SENSOR_STED_VPPM:
            *value = ics->stedVPPM[channel];
            *state = ics->stedVPPMState[channel];
            break;
        case ICS_SENSOR_SPIM_FILL_FACTOR:
            *value = ics->spimFillFactor[channel];
            *state = ics->spimFillFactorState[channel];
            break;
        case ICS_SENSOR_SPIM_PLANE_NA:
            *value = ics->spimPlaneNA[channel];
            *state = ics->spimPlaneNAState[channel];
            break;
        case ICS_SENSOR_SPIM_PLANE_GAUSS_WIDTH:
            *value = ics->spimPlaneGaussWidth[channel];
            *state = ics->spimPlaneGaussWidthState[channel];
            break;
        case ICS_SENSOR_SPIM_PLANE_CENTER_OFF:
            *value = ics->spimPlaneCenterOff[channel];
            *state = ics->spimPlaneCenterOffState[channel];
            break;
        case ICS_SENSOR_SPIM_PLANE_FOCUS_OFF:
            *value = ics->spimPlaneFocusOff[channel];
            *state = ics->spimPlaneFocusOffState[channel];
            break;
        case ICS_SENSOR_SCATTER_FREE_PATH:
            *value = ics->scatterFreePath[channel];
            *state = ics->scatterFreePathState[channel];
            break;
        case ICS_SENSOR_SCATTER_REL_CONTRIB:
            *value = ics->scatterRelContrib[channel];
            *state = ics->scatterRelContribState[channel];
            break;
        case ICS_SENSOR_SCATTER_BLURRING:
            *value = ics->scatterBlurring[channel];
            *state = ics->scatterBlurringState[channel];
            break;
        default:
            *value = 0;
            *state = IcsSensorState_default;
            return IcsErr_NotValidAction;
    }

    return IcsErr_Ok;
}


/* Get the state of a vector sensor parameter. */
Ics_Error IcsGetSensorParameterVector(const ICS            *ics,
                                      Ics_SensorParameter   parameter,
                                      int                   channel,
                                      const double        **values,
                                      Ics_SensorState      *state)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;
 
    switch (parameter) {
        case ICS_SENSOR_SPIM_PLANE_PROP_DIR:
            *values = ics->spimPlanePropDir[channel];
            *state = ics->spimPlanePropDirState[channel];
            break;
        case ICS_SENSOR_DETECTOR_SENSITIVITY:
            *values = ics->detectorSensitivity[channel];
            *state = ics->detectorSensitivityState[channel];
            break;
        case ICS_SENSOR_DETECTOR_RADIUS:
            *values = ics->detectorRadius[channel];
            *state = ics->detectorRadiusState[channel];
            break;
        default:
            *values = NULL;
            *state = IcsSensorState_default;
            return IcsErr_NotValidAction;
    }

    return IcsErr_Ok;
}


/* Get the state of a matrix sensor parameter. Sets **values to a pointer to
   the matrix, which effectively becomes a flattened array. */
Ics_Error IcsGetSensorParameterMatrix(const ICS             *ics,
                                      Ics_SensorParameter    parameter,
                                      int                    channel,
                                      const double         **values,
                                      Ics_SensorState       *state)
{
    if (channel < 0 || channel >= ics->sensorChannels) {
        return IcsErr_NotValidAction;
    }
    
    switch (parameter) {
        case ICS_SENSOR_DETECTOR_OFFSET:
            *values = (const double*)ics->detectorOffset[channel];
            *state  = ics->detectorOffsetState[channel];
            break;
        default:
            *values = NULL;
            *state = IcsSensorState_default;
            return IcsErr_NotValidAction;
    }

    return IcsErr_Ok;
}


/* Get the state of a sensor parameter. */
Ics_Error IcsGetSensorParameterInt(const ICS           *ics,
                                   Ics_SensorParameter  parameter,
                                   int                  channel,
                                   int                 *value,
                                   Ics_SensorState     *state)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;

    switch (parameter) {
        case ICS_SENSOR_OBJECTIVE_QUALITY:
            *value = ics->objectiveQuality[channel];
            *state = ics->objectiveQualityState[channel];
            break;
        case ICS_SENSOR_PHOTON_COUNT:
            *value = ics->exPhotonCnt[channel];
            *state = ics->exPhotonCntState[channel];
            break;
        default:
            *value = 0;
            *state = IcsSensorState_default;
            return IcsErr_NotValidAction;
    }

    return IcsErr_Ok;
}


/* Get the state of a sensor parameter. */
Ics_Error IcsGetSensorParameterString(const ICS            *ics,
                                      Ics_SensorParameter   parameter,
                                      int                   channel,
                                      const char          **value,
                                      Ics_SensorState      *state)
{
    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;


    switch (parameter) {
        case ICS_SENSOR_DETECTOR_MIRROR:
            *value = ics->detectorMirror[channel];
            *state = ics->detectorMirrorState[channel];
            break;
        case ICS_SENSOR_DETECTOR_MODEL:
            *value = ics->detectorModel[channel];
            *state = ics->detectorModelState[channel];
            break;
        case ICS_SENSOR_DETECTOR_REDUCEHIST:
            *value = ics->detectorRedHist[channel];
            *state = ics->detectorRedHistState[channel];
            break;
        case ICS_SENSOR_IMAGING_DIRECTION:
            *value = ics->imagingDirection[channel];
            *state = ics->imagingDirectionState[channel];
            break;
        case ICS_SENSOR_STED_DEPLETION_MODE:
            *value = ics->stedDepletionMode[channel];
            *state = ics->stedDepletionModeState[channel];
            break;
        case ICS_SENSOR_SPIM_EXCITATION_TYPE:
            *value = ics->spimExcType[channel];
            *state = ics->spimExcTypeState[channel];
            break;
        case ICS_SENSOR_SCATTER_MODEL:
            *value = ics->scatterModel[channel];
            *state = ics->scatterModelState[channel];
            break;
        case ICS_SENSOR_DESCRIPTION:
            *value = ics->description[channel];
            *state = ics->descriptionState[channel];
            break;
        default:
            *value = "";
            *state = IcsSensorState_default;
            return IcsErr_NotValidAction;
    }

    return IcsErr_Ok;
}


/* Set the state of a sensor parameter. */
Ics_Error IcsSetSensorParameter(ICS                 *ics,
                                Ics_SensorParameter  parameter,
                                int                  channel,
                                double               value,
                                Ics_SensorState      state)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;

    switch (parameter) {
        case ICS_SENSOR_NUMERICAL_APERTURE:
            ics->numAperture = value;
            ics->numApertureState = state;
            break;
        case ICS_SENSOR_MEDIUM_REFRACTIVE_INDEX:
            ics->refrInxMedium = value;
            ics->refrInxMediumState = state;
            break;
        case ICS_SENSOR_LENS_REFRACTIVE_INDEX:
            ics->refrInxLensMedium = value;
            ics->refrInxLensMediumState = state;
            break;
        case ICS_SENSOR_PINHOLE_RADIUS:
            ics->pinholeRadius[channel] = value;
            ics->pinholeRadiusState[channel] = state;
            break;
        case ICS_SENSOR_ILL_PINHOLE_RADIUS:
            ics->illPinholeRadius[channel] = value;
            ics->illPinholeRadiusState[channel] = state;
            break;
        case ICS_SENSOR_PINHOLE_SPACING:
            ics->pinholeSpacing = value;
            ics->pinholeSpacingState = state;
            break;
        case ICS_SENSOR_EXCITATION_BEAM_FILL:
            ics->excitationBeamFill[channel] = value;
            ics->excitationBeamFillState[channel] = state;
            break;
        case ICS_SENSOR_LAMBDA_EXCITATION:
            ics->lambdaEx[channel] = value;
            ics->lambdaExState[channel] = state;
            break;
        case ICS_SENSOR_LAMBDA_EMISSION:
            ics->lambdaEm[channel] = value;
            ics->lambdaEmState[channel] = state;
            break;
        case ICS_SENSOR_PHOTON_COUNT:
            ics->exPhotonCnt[channel] = (int)value;
            ics->exPhotonCntState[channel] = state;
            break;
        case ICS_SENSOR_INTERFACE_PRIMARY:
            ics->interfacePrimary = value;
            ics->interfacePrimaryState = state;
            break;
        case ICS_SENSOR_INTERFACE_SECONDARY:
            ics->interfaceSecondary = value;
            ics->interfaceSecondaryState = state;
            break;
        case ICS_SENSOR_DETECTOR_MAGN:
            ics->detectorMagn[channel] = value;
            ics->detectorMagnState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_PPU:
            ics->detectorPPU[channel] = value;
            ics->detectorPPUState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_BASELINE:
            ics->detectorBaseline[channel] = value;
            ics->detectorBaselineState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_LINE_AVG_COUNT:
            ics->detectorLineAvgCnt[channel] = value;
            ics->detectorLineAvgCntState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_NOISE_GAIN:
            ics->detectorNoiseGain[channel] = value;
            ics->detectorNoiseGainState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_SCALE:
            ics->detectorScale[channel] = value;
            ics->detectorScaleState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_STRETCH:
            ics->detectorStretch[channel] = value;
            ics->detectorStretchState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_ROT:
            ics->detectorRot[channel] = value;
            ics->detectorRotState[channel] = state;
            break;
        case ICS_SENSOR_STED_LAMBDA:
            ics->stedLambda[channel] = value;
            ics->stedLambdaState[channel] = state;
            break;
        case ICS_SENSOR_STED_SATURATION_FACTOR:
            ics->stedSatFactor[channel] = value;
            ics->stedSatFactorState[channel] = state;
            break;
        case ICS_SENSOR_STED_IMM_FRACTION:
            ics->stedImmFraction[channel] = value;
            ics->stedImmFractionState[channel] = state;
            break;
        case ICS_SENSOR_STED_VPPM:
            ics->stedVPPM[channel] = value;
            ics->stedVPPMState[channel] = state;
            break;
        case ICS_SENSOR_SPIM_FILL_FACTOR:
            ics->spimFillFactor[channel] = value;
            ics->spimFillFactorState[channel] = state;
            break;
        case ICS_SENSOR_SPIM_PLANE_NA:
            ics->spimPlaneNA[channel] = value;
            ics->spimPlaneNAState[channel] = state;
            break;
        case ICS_SENSOR_SPIM_PLANE_GAUSS_WIDTH:
            ics->spimPlaneGaussWidth[channel] = value;
            ics->spimPlaneGaussWidthState[channel] = state;
            break;
        case ICS_SENSOR_SPIM_PLANE_CENTER_OFF:
            ics->spimPlaneCenterOff[channel] = value;
            ics->spimPlaneCenterOffState[channel] = state;
            break;
        case ICS_SENSOR_SPIM_PLANE_FOCUS_OFF:
            ics->spimPlaneFocusOff[channel] = value;
            ics->spimPlaneFocusOffState[channel] = state;
            break;
        case ICS_SENSOR_SCATTER_FREE_PATH:
            ics->scatterFreePath[channel] = value;
            ics->scatterFreePathState[channel] = state;
            break;
        case ICS_SENSOR_SCATTER_REL_CONTRIB:
            ics->scatterRelContrib[channel] = value;
            ics->scatterRelContribState[channel] = state;
            break;
        case ICS_SENSOR_SCATTER_BLURRING:
            ics->scatterBlurring[channel] = value;
            ics->scatterBlurringState[channel] = state;
            break;
        default:
            return IcsErr_NotValidAction;
    }

    return IcsErr_Ok;
}


/* Set the state of a vector sensor parameter. */
Ics_Error IcsSetSensorParameterVector(ICS                 *ics,
                                      Ics_SensorParameter  parameter,
                                      int                  channel,
                                      int                  nValues,
                                      double              *values,
                                      Ics_SensorState      state)
{
    int j;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;

    switch (parameter) {
        case ICS_SENSOR_SPIM_PLANE_PROP_DIR:
            for (j = 0; j < nValues; j++) {
                ics->spimPlanePropDir[channel][j] = values[j];
            }
            ics->spimPlanePropDirState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_SENSITIVITY:
            if (nValues < 0 || nValues > ics->sensorDetectors)
                return IcsErr_NotValidAction;
            for (j = 0; j < nValues; j++) {
                ics->detectorSensitivity[channel][j] = values[j];
            }
            ics->detectorSensitivityState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_RADIUS:
            if (nValues < 0 || nValues > ics->sensorDetectors)
                return IcsErr_NotValidAction;
            for (j = 0; j < nValues; j++) {
                ics->detectorRadius[channel][j] = values[j];
            }
            ics->detectorRadiusState[channel] = state;
            break; 
        default:
            return IcsErr_NotValidAction; }

    return IcsErr_Ok;
}


/* Set the state of a matrix sensor parameter. *values must be a flattened
   [n][m] array, not an array of arrays. */
Ics_Error IcsSetSensorParameterMatrix(ICS                  *ics,
                                      Ics_SensorParameter   parameter,
                                      int                   channel,
                                      int                   nValues,
                                      int                   mValues,
                                      double               *values,
                                      Ics_SensorState       state)
{
    int i, j;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;

    switch (parameter) {
        case ICS_SENSOR_DETECTOR_OFFSET:
            if (nValues < 0 || nValues > ics->sensorDetectors)
                return IcsErr_NotValidAction;
            if (mValues < 0 || mValues > 3)
                return IcsErr_NotValidAction;
            for (i= 0; i < nValues; i++) {
                for (j= 0; j < mValues; j++) {
                    ics->detectorOffset[channel][i][j] =
                        values[i * mValues + j];
                }
            }
            ics->detectorOffsetState[channel] = state;
            break;
        default:
            return IcsErr_NotValidAction; }

    return IcsErr_Ok;
}


/* Set the state of an integer sensor parameter. */
Ics_Error IcsSetSensorParameterInt(ICS                 *ics,
                                   Ics_SensorParameter  parameter,
                                   int                  channel,
                                   int                  value,
                                   Ics_SensorState      state)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;

    switch (parameter) {
        case ICS_SENSOR_OBJECTIVE_QUALITY:
            ics->objectiveQuality[channel] = value;
            ics->objectiveQualityState[channel] = state;
            break;
        case ICS_SENSOR_PHOTON_COUNT:
            ics->exPhotonCnt[channel] = value;
            ics->exPhotonCntState[channel] = state;
            break;
        default:
            return IcsErr_NotValidAction;
    }

    return IcsErr_Ok;
}


/* Set the state of a string sensor parameter. */
Ics_Error IcsSetSensorParameterString(ICS                 *ics,
                                      Ics_SensorParameter  parameter,
                                      int                  channel,
                                      const char          *value,
                                      Ics_SensorState      state)
{
    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (channel < 0 || channel >= ics->sensorChannels)
        return IcsErr_NotValidAction;

    switch (parameter) {
        case ICS_SENSOR_DETECTOR_MIRROR:
            IcsStrCpy(ics->detectorMirror[channel], value, ICS_STRLEN_TOKEN);
            ics->detectorMirrorState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_MODEL:
            IcsStrCpy(ics->detectorModel[channel], value, ICS_STRLEN_TOKEN);
            ics->detectorModelState[channel] = state;
            break;
        case ICS_SENSOR_DETECTOR_REDUCEHIST:
            IcsStrCpy(ics->detectorRedHist[channel], value, ICS_STRLEN_TOKEN);
            ics->detectorRedHistState[channel] = state;
            break;
        case ICS_SENSOR_IMAGING_DIRECTION:
            IcsStrCpy(ics->imagingDirection[channel], value, ICS_STRLEN_TOKEN);
            ics->imagingDirectionState[channel] = state;
            break;
        case ICS_SENSOR_STED_DEPLETION_MODE:
            IcsStrCpy(ics->stedDepletionMode[channel], value, ICS_STRLEN_TOKEN);
            ics->stedDepletionModeState[channel] = state;
            break;
        case ICS_SENSOR_SPIM_EXCITATION_TYPE:
            IcsStrCpy(ics->spimExcType[channel], value, ICS_STRLEN_TOKEN);
            ics->spimExcTypeState[channel] = state;
            break;
        case ICS_SENSOR_SCATTER_MODEL:
            IcsStrCpy(ics->scatterModel[channel], value, ICS_STRLEN_TOKEN);
            ics->scatterModelState[channel] = state;
            break;
        case ICS_SENSOR_DESCRIPTION:
            IcsStrCpy(ics->description[channel], value, ICS_STRLEN_TOKEN);
            ics->descriptionState[channel] = state;
            break;
        default:
            return IcsErr_NotValidAction;
    }

    return IcsErr_Ok;
}
