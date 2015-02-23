/***************************************************************************
 *   Copyright (C) 2008 by Ralf Kaestner                                   *
 *   ralf.kaestner@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef EPOS_PROFILE_H
#define EPOS_PROFILE_H

#include "epos.h"

/** \file profile.h
  * \brief EPOS profile generic functions
  */

/** \name Object Indexes
  * \brief Predefined EPOS profile control object indexes
  */
//@{
#define EPOS_PROFILE_INDEX_ACCELERATION              0x6083
#define EPOS_PROFILE_INDEX_DECELERATION              0x6084
#define EPOS_PROFILE_INDEX_TYPE                      0x6086
#define EPOS_PROFILE_INDEX_MAX_VELOCITY              0x607F
#define EPOS_PROFILE_INDEX_MAX_ACCELERATION          0x60C5
#define EPOS_PROFILE_INDEX_QUICKSTOP_DECELERATION    0x6085
//@}

/** \name Status Words
  * \brief Predefined EPOS profile control status words
  */
//@{
#define EPOS_PROFILE_STATUS_REACHED           0x0400
//@}

/** \brief EPOS motion profile types
  */
typedef enum {
  epos_profile_linear,            //!< Linear profile.
  epos_profile_sinusoidal,        //!< Sinusoidal profile.
} epos_profile_type_t;

/** \brief Structure defining an EPOS motion profile value
  */
typedef struct epos_profile_value_t {
  float position;                 //!< The profile position in [rad].
  float velocity;                 //!< The profile velocity in [rad/s].
  float acceleration;             //!< The profile acceleration in [rad/s^2].
} epos_profile_value_t;

/** \brief Wait for completion of an EPOS motion profile
  * \param[in] node The EPOS node to complete the motion profile.
  * \param[in] timeout The timeout of the wait operation in [s].
  * \return The resulting device error code.
  */
int epos_profile_wait(
  epos_node_t* node,
  double timeout);

/** \brief Set the profile acceleration of an EPOS device
  * \param[in] dev The EPOS device to set the profile acceleration for.
  * \param[in] acceleration The profile acceleration for the specified
  *   EPOS device in [au].
  * \return The resulting device error code.
  */
int epos_profile_set_acceleration(
  epos_device_t* dev,
  unsigned int acceleration);

/** \brief Set the profile deceleration of an EPOS device
  * \param[in] dev The EPOS device to set the profile deceleration for.
  * \param[in] deceleration The profile deceleration for the specified
  *   EPOS device in [au].
  * \return The resulting device error code.
  */
int epos_profile_set_deceleration(
  epos_device_t* dev,
  unsigned int deceleration);

/** \brief Set the maximum profile velocity of an EPOS device
  * \param[in] dev The EPOS device to set the maximum profile velocity for.
  * \param[in] max_velocity The maximum profile velocity for the specified
  *   EPOS device in [vu].
  * \return The resulting device error code.
  */
int epos_profile_set_max_velocity(
  epos_device_t* dev,
  unsigned int max_velocity);

/** \brief Set the maximum acceleration of an EPOS device
  * \param[in] dev The EPOS device to set the maximum acceleration for.
  * \param[in] max_acc The maximum acceleration for the specified EPOS device
  *   in [au].
  * \return The resulting device error code.
  */
int epos_profile_set_max_acceleration(
  epos_device_t* dev,
  unsigned int max_acc);

/** \brief Set the quickstop deceleration of an EPOS device
  * \param[in] dev The EPOS device to set the quickstop deceleration for.
  * \param[in] quickstop_dec The quickstop deceleration for the specified EPOS device
  *   in [au].
  * \return The resulting device error code.
  */
int epos_profile_set_quickstop_deceleration(
  epos_device_t* dev,
  unsigned int quickstop_dec);

/** \brief Set the motion profile type of an EPOS device
  * \param[in] dev The EPOS device to set the motion profile type for.
  * \param[in] type The motion profile type for the specified EPOS device.
  * \return The resulting device error code.
  */
int epos_profile_set_type(
  epos_device_t* dev,
  epos_profile_type_t type);

#endif
