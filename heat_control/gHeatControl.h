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
/*!\file    projects/samrt_home/heat_control/gHeatControl.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-11
 *
 * \brief Contains gHeatControl
 *
 * \b gHeatControl
 *
 * Control group for the raspberry pi heating contol.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__heat_control__gHeatControl_h__
#define __projects__smart_home__heat_control__gHeatControl_h__

#include "plugins/structure/tSenseControlGroup.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control_states/tState.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/mController.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace smart_home
{
namespace heat_control
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
class gHeatControl : public structure::tSenseControlGroup
{

//----------------------------------------------------------------------
// Ports (These are the only variables that may be declared public)
//----------------------------------------------------------------------
public:

  tControllerInput<tControlModeType> ci_control_mode;

  tControllerInput<bool> ci_manual_pump_solar;
  tControllerInput<bool> ci_manual_pump_room;
  tControllerInput<bool> ci_manual_pump_ground;

  tControllerInput<bool> ci_disable_pump_room;
  tControllerInput<bool> ci_disable_pump_ground;

  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_room_external;

  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_boiler_top;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_boiler_middle;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_boiler_bottom;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_ground;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_solar;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_furnace;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_room;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_garage;

  tSensorOutput<bool> so_led_red;
  tSensorOutput<bool> so_led_yellow;
  tSensorOutput<bool> so_led_green;

  tSensorOutput<bool> so_pump_solar;
  tSensorOutput<bool> so_pump_room;
  tSensorOutput<bool> so_pump_ground;

  tControllerOutput<heat_control_states::tCurrentState> co_heating_state;
  tControllerOutput<tControlModeType> co_control_mode;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  gHeatControl(core::tFrameworkElement *parent, const std::string &name = "HeatControl",
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
  ~gHeatControl();

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
