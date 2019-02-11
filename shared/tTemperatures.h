//
// You received this file as part of Finroc
// A framework for intelligent robot control
//
// Copyright (C) Patrick Wolf
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    projects/smart_home/data_structures/tTemperatures.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 * \brief   Contains tTemperatures
 *
 * \b tTemperatures
 *
 * Container class which stores all temperature values (also thresholds) for the state controller.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__data_structures__tTemperatures_h__
#define __projects__smart_home__data_structures__tTemperatures_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/si_units/si_units.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace smart_home
{
namespace shared
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

// differences in °K, and thresholds in °C

static constexpr rrlib::si_units::tTemperature<double> cROOM_DIFF_SETPOINT_LOW(0.5);
static constexpr rrlib::si_units::tTemperature<double> cROOM_DIFF_SETPOINT_HIGH(0.5);
static constexpr rrlib::si_units::tTemperature<double> cROOM_DIFF_BOILER_LOW(3.0);
static constexpr rrlib::si_units::tTemperature<double> cROOM_DIFF_BOILER_HIGH(3.0);
static constexpr rrlib::si_units::tCelsius<double> cROOM_BOILER_MAX(50.0);

static constexpr rrlib::si_units::tTemperature<double> cGROUND_DIFF_BOILER_LOW(2.0);
static constexpr rrlib::si_units::tTemperature<double> cGROUND_DIFF_BOILER_HIGH(6.0);
static constexpr rrlib::si_units::tCelsius<double> cGROUND_BOILER_MIN(49.0);

static constexpr rrlib::si_units::tTemperature<double> cSOLAR_DIFF_BOILER_LOW(2.0);
static constexpr rrlib::si_units::tTemperature<double> cSOLAR_DIFF_BOILER_HIGH(6.0);

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Container class for temperature values.
/*!
 * Container class which stores all temperature values for the state controller.
 */
class tTemperatures
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Default constructor
   */
  tTemperatures():
    boiler_(0.0),
    room_(0.0),
    solar_(0.0),
    ground_(0.0),
    room_set_point_(20.0)
  {}

  /*!
   * Constructor
   * @param boiler Temperature of heat storage buffer
   * @param room Temperature of living area
   * @param solar Temperature of solar panel
   * @param ground Temperature below house within concrete heat shield
   * @param room_set_point Set point temperature of living area
   */
  tTemperatures(const rrlib::si_units::tCelsius<double> & boiler,
                const rrlib::si_units::tCelsius<double> & room,
                const rrlib::si_units::tCelsius<double> & solar,
                const rrlib::si_units::tCelsius<double> & ground,
                const rrlib::si_units::tCelsius<double> & room_set_point):
    boiler_(boiler),
    room_(room),
    solar_(solar),
    ground_(ground),
    room_set_point_(room_set_point)
  {}

  /*!
   * Destructor
   */
  ~tTemperatures()
  {}

  /*!
   * Getter for temperature of heat buffer
   * @return Temperature
   */
  inline rrlib::si_units::tCelsius<double> GetBoiler() const
  {
    return boiler_;
  }

  /*!
   * Getter for temperature of living area
   * @return Temperature
   */
  inline rrlib::si_units::tCelsius<double> GetRoom() const
  {
    return room_;
  }

  /*!
   * Getter for temperature in solar panel
   * @return Temperature
   */
  inline rrlib::si_units::tCelsius<double> GetSolar() const
  {
    return solar_;
  }

  /*!
   * Getter for temperature below house within concrete shield
   * @return Temperature
   */
  inline rrlib::si_units::tCelsius<double> GetGround() const
  {
    return ground_;
  }

  /*!
   * Getter for set point temperature of living area
   * @return Temperature
   */
  inline rrlib::si_units::tCelsius<double> GetRoomSetPoint() const
  {
    return room_set_point_;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  rrlib::si_units::tCelsius<double> boiler_;
  rrlib::si_units::tCelsius<double> room_;
  rrlib::si_units::tCelsius<double> solar_;
  rrlib::si_units::tCelsius<double> ground_;
  rrlib::si_units::tCelsius<double> room_set_point_;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif
