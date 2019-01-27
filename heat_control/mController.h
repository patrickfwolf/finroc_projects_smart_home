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
/*!\file    projects/smart_home/heat_control/mController.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 * \brief Contains mController
 *
 * \b mController
 *
 * control module for the heating control which contains the state machine and interacts with the hardware interface.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__heat_control__mController_h__
#define __projects__smart_home__heat_control__mController_h__

#include "plugins/structure/tSenseControlModule.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control_states/tReady.h"

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
enum class tControlModeType
{
  eSTOP,
  eAUTOMATIC,
  eAUTOMATIC_NO_ROOM,
  eAUTOMATIC_SOLAR_ONLY,
  eMANUAL
};

enum class tErrorState
{
  eNO_ERROR,
  eOUTDATED_TEMPERATURE,
  eIMPLAUSIBLE_TEMPERATURE,
  eIMPLAUSIBLE_OUTDATED_TEMPERATURE
};

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * control module for the heating control which contains the state machine and interacts with the hardware interface.
 */
class mController : public structure::tSenseControlModule
{

//----------------------------------------------------------------------
// Ports (These are the only variables that may be declared public)
//----------------------------------------------------------------------
public:

  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_boiler_top;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_boiler_middle;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_boiler_bottom;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_room;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_room_external;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_solar;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_ground;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_furnace;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_garage;

  tControllerInput<tControlModeType> ci_control_mode;
  tControllerInput<bool> ci_manual_pump_online_solar;
  tControllerInput<bool> ci_manual_pump_online_ground;
  tControllerInput<bool> ci_manual_pump_online_room;

  tControllerInput<bool> ci_disable_pump_room;
  tControllerInput<bool> ci_disable_pump_ground;

  tControllerInput<data_ports::tEvent> ci_increase_set_point_temperature;
  tControllerInput<data_ports::tEvent> ci_decrease_set_point_temperature;
  tControllerInput<data_ports::tEvent> ci_reset_set_point_temperature;

  tControllerOutput<bool> co_led_online_red;
  tControllerOutput<bool> co_led_online_yellow;
  tControllerOutput<bool> co_led_online_green;

  tControllerOutput<bool> co_pump_online_solar;
  tControllerOutput<bool> co_pump_online_room;
  tControllerOutput<bool> co_pump_online_ground;

  tControllerOutput<tControlModeType> co_control_mode;
  tControllerOutput<heat_control_states::tCurrentState> co_heating_state;
  tControllerOutput<tErrorState> co_error_state;
  tControllerOutput<rrlib::si_units::tCelsius<double>> co_set_point_temperature;


  tParameter<rrlib::si_units::tCelsius<double>> par_temperature_set_point_room;
  tParameter<rrlib::time::tDuration> par_max_update_duration;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  mController(core::tFrameworkElement *parent, const std::string &name = "Controller");

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Destructor
   *
   * The destructor of modules is declared protected to avoid accidental deletion. Deleting
   * modules is already handled by the framework.
   */
  ~mController();

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  virtual void Sense() override;

  virtual void Control() override;

  virtual void OnParameterChange() override;

  /*!
   * Checks if a temperature value is within bounds
   * @return within bounds
   */
  bool IsTemperatureInBounds(const rrlib::si_units::tCelsius<double> & temperature,
                             const rrlib::si_units::tCelsius<double> & upper_bound,
                             const rrlib::si_units::tCelsius<double> & lower_bound) const
  {
    return (temperature < upper_bound) and (temperature > lower_bound);
  }

  std::unique_ptr<heat_control_states::tState> control_state_;
  rrlib::si_units::tCelsius<double> set_point_;
  tErrorState error_;
  shared::tTemperatures temperatures_;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}



#endif
