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
/*!\file    projects/smart_home/heat_control/mController.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/mController.h"

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
namespace heat_control
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<mController> cCREATE_ACTION_FOR_M_CONTROLLER("Controller");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// mController constructor
//----------------------------------------------------------------------
mController::mController(core::tFrameworkElement *parent, const std::string &name) :
  tSenseControlModule(parent, name, false), // change to 'true' to make module's ports shared (so that ports in other processes can connect to its sensor outputs and controller inputs)
  ci_control_mode(tControlModeType::eAUTOMATIC),
  control_state_(nullptr)
{
  control_state_ = std::unique_ptr<heat_control_states::tState>(new heat_control_states::tReady());
}

//----------------------------------------------------------------------
// mController destructor
//----------------------------------------------------------------------
mController::~mController()
{
}

//----------------------------------------------------------------------
// mController Sense
//----------------------------------------------------------------------
void mController::Sense()
{
  if (this->SensorInputChanged())
  {

    // pump control
    std::unique_ptr<heat_control_states::tState> next_state;

    shared::tTemperatures temperatures
    {
      si_temperature_boiler_middle.Get(),
      si_temperature_room.Get(),
      si_temperature_solar.Get(),
      si_temperature_ground.Get(),
      par_temperature_set_point_room.Get()
    };
    control_state_->ComputeControlState(next_state, temperatures);
    if (control_state_->HasChanged())
    {
      control_state_ = std::move(next_state);
    }

    co_heating_state.Publish(control_state_->GetCurrentState());

    // led control
    auto boiler = si_temperature_boiler_middle.Get();
    if (boiler <= rrlib::si_units::tCelsius<double>(25.0))
    {
      co_led_online_green.Publish(false);
      co_led_online_yellow.Publish(false);
      co_led_online_red.Publish(true);
    }
    else if (boiler > rrlib::si_units::tCelsius<double>(25.0) && boiler <= rrlib::si_units::tCelsius<double>(30.0))
    {
      co_led_online_green.Publish(false);
      co_led_online_yellow.Publish(true);
      co_led_online_red.Publish(true);
    }
    else if (boiler > rrlib::si_units::tCelsius<double>(30.0) && boiler <= rrlib::si_units::tCelsius<double>(40.0))
    {
      co_led_online_green.Publish(false);
      co_led_online_yellow.Publish(true);
      co_led_online_red.Publish(false);
    }
    else if (boiler > rrlib::si_units::tCelsius<double>(40.0) && boiler <= rrlib::si_units::tCelsius<double>(45.0))
    {
      co_led_online_green.Publish(true);
      co_led_online_yellow.Publish(true);
      co_led_online_red.Publish(false);
    }
    else if (boiler > rrlib::si_units::tCelsius<double>(45.0))
    {
      co_led_online_green.Publish(true);
      co_led_online_yellow.Publish(false);
      co_led_online_red.Publish(false);
    }
  }
}

//----------------------------------------------------------------------
// mController Control
//----------------------------------------------------------------------
void mController::Control()
{

  // reset if control mode changes
  if (ci_control_mode.HasChanged())
  {
    co_pump_online_ground.Publish(false);
    co_pump_online_room.Publish(false);
    co_pump_online_solar.Publish(false);
    if (control_state_ != nullptr)
    {
      control_state_ = std::unique_ptr<heat_control_states::tState>(new heat_control_states::tReady());
    }

    co_control_mode.Publish(ci_control_mode.Get());
  }


  switch (ci_control_mode.Get())
  {
  case tControlModeType::eAUTOMATIC:
  {
    auto pumps = control_state_->GetPumpSettings();

    // check if pump values are manually disabled
    if (ci_disable_pump_ground.Get())
    {
      co_pump_online_ground.Publish(false);
    }
    else
    {
      co_pump_online_ground.Publish(pumps.IsGroundOnline());
    }
    if (ci_disable_pump_room.Get())
    {
      co_pump_online_room.Publish(false);
    }
    else
    {
      co_pump_online_room.Publish(pumps.IsRoomOnline());
    }
    // solar pump is never disabled
    co_pump_online_solar.Publish(pumps.IsSolarOnline());
  }
  break;
  case tControlModeType::eSTOP:
    if (ci_control_mode.HasChanged())
    {
      co_pump_online_ground.Publish(false);
      co_pump_online_room.Publish(false);
      co_pump_online_solar.Publish(false);
    }
    break;
  case tControlModeType::eMANUAL:
    if (ci_manual_pump_online_ground.HasChanged())
    {
      co_pump_online_ground.Publish(ci_manual_pump_online_ground.Get());
    }
    if (ci_manual_pump_online_room.HasChanged())
    {
      co_pump_online_room.Publish(ci_manual_pump_online_room.Get());
    }
    if (ci_manual_pump_online_solar.HasChanged())
    {
      co_pump_online_solar.Publish(ci_manual_pump_online_solar.Get());
    }
    break;
  case tControlModeType::eAUTOMATIC_NO_ROOM:
  {
    auto pumps = control_state_->GetPumpSettings();
    if (ci_disable_pump_room.Get())
    {
      co_pump_online_room.Publish(false);
    }
    else
    {
      co_pump_online_room.Publish(pumps.IsRoomOnline());
    }
    co_pump_online_ground.Publish(false);
    co_pump_online_solar.Publish(pumps.IsSolarOnline());
  }
  break;
  case tControlModeType::eAUTOMATIC_SOLAR_ONLY:
  {
    auto pumps = control_state_->GetPumpSettings();
    co_pump_online_room.Publish(false);
    co_pump_online_ground.Publish(false);
    co_pump_online_solar.Publish(pumps.IsSolarOnline());
  }
  break;
  default:
    co_pump_online_ground.Publish(false);
    co_pump_online_room.Publish(false);
    co_pump_online_solar.Publish(false);
  };

}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
