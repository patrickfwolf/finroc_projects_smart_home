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
/*!\file    projects/smart_home/heat_control/tPT1000Converter.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-05-07
 *
 * \brief   Contains tPT1000Converter.h
 *
 * \b tPT1000Converter.h
 *
 * Class which calculates the temperature measured by a pt1000 resistor, based on measured voltage.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__heat_control__tPT1000Converter_h__
#define __projects__smart_home__heat_control__tPT1000Converter_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/si_units/si_units.h"

#include <vector>
#include <cmath>
#include <limits>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/shared/pt1000_lookup.h"

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

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * Class which calculates the temperature measured by a pt1000 resistor, based on measured voltage.
 */
class tPT1000Converter
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tPT1000Converter();

  ~tPT1000Converter();


  /*!
   * Determines the temperature of the PT1000 based on a look up table
   * @param index
   * @return
   */
  inline rrlib::si_units::tCelsius<double> GetTemperatureFromLUTIndex(std::size_t index) const
  {
    if (index > cPT1000_LOOKUP.size())
    {
      RRLIB_LOG_PRINT(ERROR, "PT1000 LUT index out of bounds!");
      return rrlib::si_units::tCelsius<double>(std::numeric_limits<double>::infinity());
    }
    return rrlib::si_units::tCelsius<double>(index - 200);
  }

  /*!
   * Determines a temperature estimate based on the current resistance
   * @param resistance sensor resistance
   * @return temperature estimate
   */
  inline rrlib::si_units::tCelsius<double> GetTemperatureEstimateFromResistance(const rrlib::si_units::tElectricResistance<double> & resistance) const
  {
    rrlib::si_units::tCelsius<double> estimate ((resistance - rrlib::si_units::tElectricResistance<double>(1000.0)).Value() / 4.0);

    // is estimate lower than 0 K?
    if(estimate <= rrlib::si_units::tTemperature<double>(0.0))
    {
      return static_cast<rrlib::si_units::tCelsius<double>>(rrlib::si_units::tTemperature<double>(0.0));
    }
    else
    {
      return estimate;
    }
  }

  /*!
   * Estimates the resistance based on the temperature
   * @param temperature temperature
   * @return resistance estimate
   */
  inline rrlib::si_units::tElectricResistance<double> GetResistanceEstimateFromTemperatureResistance(const rrlib::si_units::tCelsius<double> & temperature) const
  {
    return rrlib::si_units::tElectricResistance<double>(4.0) * temperature.Value() + rrlib::si_units::tElectricResistance<double>(1000.0);
  }

  /*!
   * Checks if resistance is covered by LUT
   * @param resistance resistance
   * @return contained
   */
  inline bool IsResistanceInLUTBounds(const rrlib::si_units::tElectricResistance<double> & resistance) const
  {
    return (resistance < cPT1000_LOOKUP.at(0) or resistance > cPT1000_LOOKUP.at(cPT1000_LOOKUP.size() - 1)) ? false : true;
  }

  /*!
   * Checks if temperature is covered by LUT
   * @param temperature temperature
   * @return contained
   */
  inline bool IsTemperatureInLUTBounds(const rrlib::si_units::tCelsius<double> & temperature) const
  {
    return (temperature < rrlib::si_units::tCelsius<double>(-200.0) or temperature > rrlib::si_units::tCelsius<double>(850.0)) ? false : true;
  }

  /*!
   * Determines temperature based on resistance (uses LUT and estimate)
   * @param resistance resistance
   * @return temperature
   */
  rrlib::si_units::tCelsius<double> GetTemperatureFromResistance(const rrlib::si_units::tElectricResistance<double> & resistance) const;

  /*!
   * Determines temperature based on voltage and current (uses LUT and estimate)
   * @param voltage voltage
   * @param current current
   * @return temperature
   */
  inline rrlib::si_units::tCelsius<double> GetTemperatureFromVoltageAndCurrent(
      const rrlib::si_units::tVoltage<double> & voltage,
      const rrlib::si_units::tElectricCurrent<double> & current) const
  {
    return GetTemperatureFromResistance(voltage / current);
  }

  /*!
   * Determines the resistance based on the temperature (uses LUT and estimate)
   * @param temperature temperature
   * @return resistance
   */
  rrlib::si_units::tElectricResistance<double> GetResistanceFromTemperature(const rrlib::si_units::tCelsius<double> &temperature) const;


//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:


};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
