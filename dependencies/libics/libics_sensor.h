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
 * FILE : libics_sensor.h
 * Written by Bert Gijsbers, Scientific Volume Imaging BV, Hilversum, NL
 *
 * Access functions to ICS sensor parameters.
 */


#ifndef LIBICS_SENSOR_H
#define LIBICS_SENSOR_H


#include "libics.h"


#ifdef __cplusplus
extern "C" {
#endif


/* This function enables writing the sensor parameters to disk. */
ICSEXPORT Ics_Error IcsEnableWriteSensor(ICS *ics,
                                         int  enable);

/* This function enables writing the sensor parameter states to disk. */
ICSEXPORT Ics_Error IcsEnableWriteSensorStates(ICS *ics,
                                               int  enable);

/* Get the sensor type string. */
ICSEXPORT char const* IcsGetSensorType(const ICS *ics,
                                       int        channel);

/* Set the sensor type string. */
ICSEXPORT Ics_Error IcsSetSensorType(ICS        *ics,
                                     int         channel,
                                     const char *sensorType);

/* Get the sensor model string. */
ICSEXPORT const char *IcsGetSensorModel(const ICS *ics);

/* Set the sensor model string. */
ICSEXPORT Ics_Error IcsSetSensorModel(ICS        *ics,
                                      const char *sensorModel);

/* Get the number of sensor channels. */
ICSEXPORT int IcsGetSensorChannels(const ICS *ics);

/* Set the number of sensor channels. */
ICSEXPORT Ics_Error IcsSetSensorChannels(ICS *ics,
                                         int  channels);

/* Get the pinhole radius for a sensor channel. */
ICSEXPORT double IcsGetSensorPinholeRadius(const ICS *ics,
                                           int        channel);

/* Set the pinhole radius for a sensor channel. */
ICSEXPORT Ics_Error IcsSetSensorPinholeRadius(ICS    *ics,
                                              int     channel,
                                              double  radius);

/* Get the excitation wavelength for a sensor channel. */
ICSEXPORT double IcsGetSensorExcitationWavelength(const ICS *ics,
                                                  int        channel);

/* Set the excitation wavelength for a sensor channel. */
ICSEXPORT Ics_Error IcsSetSensorExcitationWavelength(ICS    *ics,
                                                     int     channel,
                                                     double  wl);

/* Get the emission wavelength for a sensor channel. */
ICSEXPORT double IcsGetSensorEmissionWavelength(const ICS *ics,
                                                int        channel);

/* Set the emission wavelength for a sensor channel. */
ICSEXPORT Ics_Error IcsSetSensorEmissionWavelength(ICS    *ics,
                                                   int     channel,
                                                   double  wl);

/* Get the excitation photon count for a sensor channel. */
ICSEXPORT int IcsGetSensorPhotonCount(const ICS *ics,
                                      int        channel);

/* Set the excitation photon count for a sensor channel. */
ICSEXPORT Ics_Error IcsSetSensorPhotonCount(ICS *ics,
                                            int  channel,
                                            int  cnt);

/* Get the sensor embedding medium refractive index. */
ICSEXPORT double IcsGetSensorMediumRI(const ICS *ics);

/* Set the sensor embedding medium refractive index. */
ICSEXPORT Ics_Error IcsSetSensorMediumRI(ICS    *ics,
                                         double  ri);

/* Get the sensor design medium refractive index. */
ICSEXPORT double IcsGetSensorLensRI(const ICS *ics);

/* Set the sensor design medium refractive index. */
ICSEXPORT Ics_Error IcsSetSensorLensRI(ICS    *ics,
                                       double  ri);

/* Get the sensor numerical apperture */
ICSEXPORT double IcsGetSensorNumAperture(const ICS *ics);

/* Set the sensor numerical apperture */
ICSEXPORT Ics_Error IcsSetSensorNumAperture(ICS    *ics,
                                            double  na);

/* Get the sensor Nipkow Disk pinhole spacing. */
ICSEXPORT double IcsGetSensorPinholeSpacing(const ICS *ics);

/* Set the sensor Nipkow Disk pinhole spacing. */
ICSEXPORT Ics_Error IcsSetSensorPinholeSpacing(ICS    *ics,
                                               double  spacing);

/* Get the STED depletion mode. */
ICSEXPORT const char *IcsGetSensorSTEDDepletionMode(const ICS *ics,
                                                    int        channel);

/* Set the STED depletion mode. */
ICSEXPORT Ics_Error IcsSetSensorSTEDDepletionMode(ICS        *ics,
                                                  int         channel,
                                                  const char *stedMode);

/* Get the STED inhibition wavelength. */
ICSEXPORT double IcsGetSensorSTEDLambda(const ICS *ics,
                                        int        channel);

/* Set the STED inhibition wavelength. */
ICSEXPORT Ics_Error IcsSetSensorSTEDLambda(ICS    *ics,
                                           int     channel,
                                           double  lambda);

/* Get the STED saturation factor. */
ICSEXPORT double IcsGetSensorSTEDSatFactor(const ICS *ics,
                                           int        channel);

/* Set the STED saturation factor. */
ICSEXPORT Ics_Error IcsSetSensorSTEDSatFactor(ICS    *ics,
                                              int     channel,
                                              double  factor);

/* Get the fraction that is not inhibited by STED. */
ICSEXPORT double IcsGetSensorSTEDImmFraction(const ICS *ics,
                                             int        channel);

/* Set the fraction that is not inhibited by STED. */
ICSEXPORT Ics_Error IcsSetSensorSTEDImmFraction(ICS    *ics,
                                                int     channel,
                                                double  fraction);

/* Get the STED  vortex to phase plate mix fraction. */
ICSEXPORT double IcsGetSensorSTEDVPPM(const ICS *ics,
                                      int        channel);

/* Set the STED vortex to phase plate mix fraction. */
ICSEXPORT Ics_Error IcsSetSensorSTEDVPPM(ICS    *ics,
                                         int     channel,
                                         double  vppm);

/* Get the Detector ppu per channel. */
ICSEXPORT double IcsGetSensorDetectorPPU(const ICS *ics,
                                         int        channel);

/* Set the Detector ppu per channel. */
ICSEXPORT Ics_Error IcsSetSensorDetectorPPU(ICS    *ics,
                                            int     channel,
                                            double  ppu);

/* Get the Detector baseline per channel. */
ICSEXPORT double IcsGetSensorDetectorBaseline(const ICS *ics,
                                              int        channel);

/* Set the Detector baseline per channel. */
ICSEXPORT Ics_Error IcsSetSensorDetectorBaseline(ICS    *ics,
                                                 int     channel,
                                                 double  baseline);

/* Get the Detector lineAvgCnt per channel. */
ICSEXPORT double IcsGetSensorDetectorLineAvgCnt(const ICS *ics,
                                                int        channel);

/* Set the Detector lineAvgCnt per channel. */
ICSEXPORT Ics_Error IcsSetSensorDetectorLineAvgCnt(ICS    *ics,
                                                   int     channel,
                                                   double  lineAvgCnt);


/* Get the value and state of a sensor parameter. */
ICSEXPORT Ics_Error IcsGetSensorParameter(const ICS           *ics,
                                          Ics_SensorParameter  parameter,
                                          int                  channel,
                                          double              *value,
                                          Ics_SensorState     *state);

Ics_Error IcsGetSensorParameterVector(const ICS            *ics,
                                      Ics_SensorParameter   parameter,
                                      int                   channel,
                                      const double        **values,
                                      Ics_SensorState      *state);


/* Get the value and state of a sensor parameter with an integer value. */
ICSEXPORT Ics_Error IcsGetSensorParameterInt(const ICS           *ics,
                                             Ics_SensorParameter  parameter,
                                             int                  channel,
                                             int              *value,
                                             Ics_SensorState     *state);

/* Get the value and state of a sensor parameter with a string value. */
ICSEXPORT Ics_Error IcsGetSensorParameterString(const ICS            *ics,
                                                Ics_SensorParameter   parameter,
                                                int                   channel,
                                                const char          **value,
                                                Ics_SensorState      *state);

/* Set the state of a sensor parameter. */
ICSEXPORT Ics_Error IcsSetSensorParameter(ICS                 *ics,
                                          Ics_SensorParameter  parameter,
                                          int                  channel,
                                          double               value,
                                          Ics_SensorState      state);

ICSEXPORT Ics_Error IcsSetSensorParameterVector(ICS                 *ics,
                                                Ics_SensorParameter  parameter,
                                                int                  channel,
                                                int                  nValues,
                                                double              *values,
                                                Ics_SensorState      state);


/* Set the state of an integer sensor parameter. */
ICSEXPORT Ics_Error IcsSetSensorParameterInt(ICS                 *ics,
                                             Ics_SensorParameter  parameter,
                                             int                  channel,
                                             int                  value,
                                             Ics_SensorState      state);


/* Set the state of a string sensor parameter. */
ICSEXPORT Ics_Error IcsSetSensorParameterString(ICS                 *ics,
                                                Ics_SensorParameter  parameter,
                                                int                  channel,
                                                const char          *value,
                                                Ics_SensorState      state);


#ifdef __cplusplus
}
#endif


#endif
