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
/*!\file    projects/smart_home/heat_control/tPT1000VoltageToTemperatureConverter.hpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-05-07
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/shared/tPT1000Converter.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
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

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// tPT1000VoltageToTemperatureConverter constructors
//----------------------------------------------------------------------
tPT1000Converter::tPT1000Converter()
{
}

//----------------------------------------------------------------------
// tPT1000VoltageToTemperatureConverter destructor
//----------------------------------------------------------------------
tPT1000Converter::~tPT1000Converter()
{}

//----------------------------------------------------------------------
// tPT1000VoltageToTemperatureConverter GetTemperatureFromResistance
//----------------------------------------------------------------------
rrlib::si_units::tCelsius<double> tPT1000Converter::GetTemperatureFromResistance(const rrlib::si_units::tElectricResistance<double> & resistance) const
{
  if (not IsResistanceInLUTBounds(resistance))
  {
    RRLIB_LOG_PRINT(WARNING, "Sensor resistance not within LUT bounds (-200°C to 850°C). Estimating temperature.");
    return GetTemperatureEstimateFromResistance(resistance);
  }

  // perform binary search on LUT
  std::size_t pivot = std::floor((cPT1000_LOOKUP.size() - 1) / 2);
  std::size_t step_size = std::floor(pivot / 2);
  while (step_size != 0)
  {
    if (cPT1000_LOOKUP.at(pivot) ==  resistance)
    {
      step_size = 0;
      continue;
    }
    if (cPT1000_LOOKUP.at(pivot) > resistance)
    {
      pivot -= step_size;
    }
    else
    {
      pivot += step_size;
    }
    step_size = std::floor(step_size / 2);
  }

  rrlib::si_units::tCelsius<double> temperature_low  = GetTemperatureFromLUTIndex(pivot);
  rrlib::si_units::tCelsius<double> temperature_high = GetTemperatureFromLUTIndex(pivot + 1);
  rrlib::si_units::tCelsius<double> temperature_diff = rrlib::si_units::tCelsius<double> (std::fabs(temperature_high.ValueFactored() - temperature_low.ValueFactored()));

  // do a linear interpolation of value
  rrlib::si_units::tElectricResistance<double> resistance_diff = resistance - cPT1000_LOOKUP.at(pivot);
  if (resistance_diff == rrlib::si_units::tElectricResistance<double>(0.0))
  {
    return temperature_low;
  }
  else
  {
    double factor = (resistance_diff / (cPT1000_LOOKUP.at(pivot + 1) - cPT1000_LOOKUP.at(pivot))).Value();
    rrlib::si_units::tCelsius<double> linear_interpolation = temperature_diff * factor;
    return temperature_low + linear_interpolation;
  }
}

//----------------------------------------------------------------------
// tPT1000VoltageToTemperatureConverter GetResistanceFromTemperature
//----------------------------------------------------------------------
rrlib::si_units::tElectricResistance<double> tPT1000Converter::GetResistanceFromTemperature(const rrlib::si_units::tCelsius<double> &temperature) const
{
  if (not IsTemperatureInLUTBounds(temperature))
  {
    RRLIB_LOG_PRINT(WARNING, "Sensor temperature not within LUT bounds (-200°C to 850°C). Estimating resistance.");
    return GetResistanceEstimateFromTemperatureResistance(temperature);
  }

  rrlib::si_units::tElectricResistance<double> lower_lut_resistance = cPT1000_LOOKUP.at(std::floor(temperature.ValueFactored()) + cLUT_OFFSET);
  rrlib::si_units::tElectricResistance<double> upper_lut_resistance = cPT1000_LOOKUP.at(std::floor(temperature.ValueFactored()) + cLUT_OFFSET + 1);
  rrlib::si_units::tElectricResistance<double> resistance_diff = upper_lut_resistance - lower_lut_resistance;
  if (std::floor(temperature.ValueFactored()) == temperature.ValueFactored())
  {
    return lower_lut_resistance;
  }
  else
  {
    return (std::fabs(temperature.ValueFactored() - std::floor(temperature.ValueFactored()))) * resistance_diff + lower_lut_resistance;
  }
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
