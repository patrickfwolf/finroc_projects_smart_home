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
/*!\file    projects/smart_home/vent/gVentControl.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-11
 *
 * \brief Contains gVentControl
 *
 * \b gVentControl
 *
 * Control group for the raspberry pi heating contol.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__vent_control__gVentControl_h__
#define __projects__smart_home__vent_control__gVentControl_h__

#include "plugins/structure/tGroup.h"

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
namespace vent_control
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * Control group for the raspberry pi heating contol.
 */
class gVentControl : public structure::tGroup
{

//----------------------------------------------------------------------
// Ports (These are the only variables that may be declared public)
//----------------------------------------------------------------------
public:

  tInput<rrlib::si_units::tCelsius<double>> in_temperature_furnace;

  tOutput<bool> out_ventilation;
  tOutput<rrlib::si_units::tCelsius<double>> out_pt100_temperature_room;
  tOutput<rrlib::si_units::tCelsius<double>> out_bmp180_temperature_room;
  tOutput<rrlib::si_units::tCelsius<double>> out_average_temperature_room;
  tOutput<rrlib::si_units::tPressure<double>> out_air_pressure_room;
  tOutput<rrlib::si_units::tAmountOfSubstance<double>> out_carbon_monoxid_room;
  tOutput<bool> out_carbon_monoxid_threshold_room;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  gVentControl(core::tFrameworkElement *parent, const std::string &name = "VentControl",
               const std::string &structure_config_file = __FILE__".xml");

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Destructor
   *
   * The destructor of groups is declared protected to avoid accidental deletion. Deleting
   * groups is already handled by the framework.
   */
  ~gVentControl();

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
