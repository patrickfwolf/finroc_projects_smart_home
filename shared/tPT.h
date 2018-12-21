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
/*!\file    projects/smart_home/heat_control/tPT.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-05-07
 *
 * \brief   Contains tPT.h
 *
 * \b tPT.h
 *
 * Class which calculates the temperature measured by a pt1000 resistor, based on measured voltage.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__heat_control__tPT_h__
#define __projects__smart_home__heat_control__tPT_h__

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

// factors according to DIN EN 60751
static constexpr double cPT_A(3.90802 * 1E-3);
static constexpr double cPT_B(-5.801 * 1E-7);
static constexpr double cPT_C(-4.735 * 1E-12);

static constexpr rrlib::si_units::tCelsius<double> cPT_TEMP_THRESHOLD_LOW(-200.0);
static constexpr rrlib::si_units::tCelsius<double> cPT_TEMP_THRESHOLD_HIGH(850.0);
static constexpr rrlib::si_units::tCelsius<double> cPT_TEMP_ZERO(0.0);

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * Class which calculates the temperature measured by a pt1000 resistor, based on measured voltage.
 */
template<int TResistance>
class tPT
{
public:
	int cRESISTANCE = TResistance;
	rrlib::si_units::tElectricResistance<double> cR_0 = static_cast<rrlib::si_units::tElectricResistance<double>>(TResistance);

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tPT(){};

  ~tPT(){};

  /*!
   * Determines a temperature estimate based on the current resistance
   * @param resistance sensor resistance
   * @return temperature estimate
   */
  inline rrlib::si_units::tCelsius<double> GetTemperature(const rrlib::si_units::tElectricResistance<double> & resistance) const
  {
	  if(resistance.Value() > 0.0)
	  {
		  return rrlib::si_units::tCelsius<double>((-cPT_A * cR_0.Value() + std::sqrt(std::pow(cPT_A * cR_0.Value(), 2) - 4.0 * cPT_B * cR_0.Value() * (cR_0 - resistance).Value())) / (2 * cPT_B * cR_0.Value()));
	  }
	  else
	  {
		  return rrlib::si_units::tCelsius<double>(0);
	  }
  }

  /*!
   * Determines resistance based on the temperature
   * @param temperature temperature
   * @return resistance
   */
  inline rrlib::si_units::tElectricResistance<double> GetResistance(const rrlib::si_units::tCelsius<double> & temperature) const
  {
      double t = temperature.ValueFactored();

	  if(temperature < cPT_TEMP_THRESHOLD_LOW)
	  {
		  return rrlib::si_units::tElectricResistance<double>(0.0);
	  }
	  else if (temperature < cPT_TEMP_ZERO)
	  {
		  return cR_0 * (1.0 + cPT_A * t + cPT_B * std::pow(t, 2) + cPT_C * (t - 100.0) * std::pow(t, 3));
	  }
	  else if(temperature < cPT_TEMP_THRESHOLD_HIGH)
	  {
		  return cR_0 * (1.0 + cPT_A * t + cPT_B * std::pow(t, 2));
	  }
	  else
	  {
		  return rrlib::si_units::tElectricResistance<double>(0.0);
	  }
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:


};

using tPT1000 = tPT<1000>;
using tPT500 = tPT<500>;
using tPT100 = tPT<100>;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
