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
  tSenseControlModule(parent, name, true),
  ci_control_mode(tControlModeType::eAUTOMATIC),
  par_temperature_set_point_room("Temperature Set Point Room", this, rrlib::si_units::tCelsius<double>(23.0), "temperature_set_point_room"),
  par_max_update_duration("Max Update Duration", this, std::chrono::seconds(10), "max_update_duration"),
  control_state_(nullptr),
  set_point_(23.0),
  error_(tErrorState::eNO_ERROR)
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
// mController OnParameterChange
//----------------------------------------------------------------------
void mController::OnParameterChange()
{
  if (par_temperature_set_point_room.HasChanged())
  {
    this->set_point_ = par_temperature_set_point_room.Get();
    co_set_point_temperature.Publish(set_point_, rrlib::time::Now());
  }
}

//----------------------------------------------------------------------
// mController Sense
//----------------------------------------------------------------------
void mController::Sense()
{
  bool outdated_temperature = false;
  auto current_time = rrlib::time::Now();

  bool outdated = false;
  outdated = (current_time - par_max_update_duration.Get() > si_temperature_boiler_bottom.GetTimestamp()) ? true : false;
  so_outdated_temperature_boiler_bottom.Publish(outdated, current_time);
//  outdated_temperature = outdated_temperature or outdated;

  outdated = (current_time - par_max_update_duration.Get() > si_temperature_boiler_top.GetTimestamp()) ? true : false;
  so_outdated_temperature_boiler_top.Publish(outdated, current_time);
//  outdated_temperature = outdated_temperature or outdated;

  outdated = (current_time - par_max_update_duration.Get() > si_temperature_boiler_middle.GetTimestamp()) ? true : false;
  so_outdated_temperature_boiler_middle.Publish(outdated, current_time);
  outdated_temperature = outdated_temperature or outdated;

  outdated = (current_time - par_max_update_duration.Get() > si_temperature_furnace.GetTimestamp()) ? true : false;
  so_outdated_temperature_furnace.Publish(outdated, current_time);
//  outdated_temperature = outdated_temperature or outdated;

  outdated = (current_time - par_max_update_duration.Get() > si_temperature_garage.GetTimestamp()) ? true : false;
  so_outdated_temperature_garage.Publish(outdated, current_time);
//  outdated_temperature = outdated_temperature or outdated;

  outdated = (current_time - par_max_update_duration.Get() > si_temperature_ground.GetTimestamp()) ? true : false;
  so_outdated_temperature_garage.Publish(outdated, current_time);
  outdated_temperature = outdated_temperature or outdated;

  outdated = (current_time - par_max_update_duration.Get() > si_temperature_room.GetTimestamp()) ? true : false;
  so_outdated_temperature_room.Publish(outdated, current_time);
  outdated_temperature = outdated_temperature or outdated;

  outdated = (current_time - par_max_update_duration.Get() > si_temperature_solar.GetTimestamp()) ? true : false;
  so_outdated_temperature_solar.Publish(outdated, current_time);
  outdated_temperature = outdated_temperature or outdated;

  bool implausible_temperature = false;

  if (this->SensorInputChanged())
  {
    // check plausibility of temperatures
    bool implausible = false;
    implausible = not IsTemperatureInBounds(si_temperature_room.Get(), rrlib::si_units::tCelsius<double>(50.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_room.Publish(implausible, current_time);

    implausible = not IsTemperatureInBounds(si_temperature_solar.Get(), rrlib::si_units::tCelsius<double>(150.0), rrlib::si_units::tCelsius<double>(-40.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_solar.Publish(implausible, current_time);

    implausible = not IsTemperatureInBounds(si_temperature_ground.Get(), rrlib::si_units::tCelsius<double>(50.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_ground.Publish(implausible, current_time);

    implausible = not IsTemperatureInBounds(si_temperature_garage.Get(), rrlib::si_units::tCelsius<double>(50.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_garage.Publish(implausible, current_time);

    implausible = not IsTemperatureInBounds(si_temperature_furnace.Get(), rrlib::si_units::tCelsius<double>(100.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_furnace.Publish(implausible, current_time);

    implausible = not IsTemperatureInBounds(si_temperature_boiler_bottom.Get(), rrlib::si_units::tCelsius<double>(100.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_boiler_bottom.Publish(implausible, current_time);

    implausible = not IsTemperatureInBounds(si_temperature_boiler_top.Get(), rrlib::si_units::tCelsius<double>(100.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_boiler_top.Publish(implausible, current_time);

    implausible = not IsTemperatureInBounds(si_temperature_boiler_middle.Get(), rrlib::si_units::tCelsius<double>(100.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_boiler_middle.Publish(implausible, current_time);

    implausible = not IsTemperatureInBounds(si_temperature_room_external.Get(), rrlib::si_units::tCelsius<double>(50.0), rrlib::si_units::tCelsius<double>(0.0));
    implausible_temperature = implausible_temperature or implausible;
    so_implausible_temperature_room_external.Publish(implausible, current_time);

    // integrate external room temperature if value is available
    auto temperature_room = si_temperature_room.Get();
    if (current_time - si_temperature_room_external.GetTimestamp() < par_max_update_duration.Get() and not implausible_temperature)
    {
      temperature_room += si_temperature_room_external.Get();
      temperature_room /= 2.0;
      so_temperature_room_combined.Publish(temperature_room, si_temperature_room.GetTimestamp());
    }

    temperatures_ = shared::tTemperatures
    {
      si_temperature_boiler_middle.Get(),
      temperature_room,
      si_temperature_solar.Get(),
      si_temperature_ground.Get(),
      set_point_
    };
  }

  // determine error state
  if (implausible_temperature)
  {
    if (outdated_temperature)
    {
      this->error_ = tErrorState::eIMPLAUSIBLE_OUTDATED_TEMPERATURE;
    }
    else
    {
      this->error_ = tErrorState::eIMPLAUSIBLE_TEMPERATURE;
    }
  }
  else
  {
    if (outdated_temperature)
    {
      this->error_ = tErrorState::eOUTDATED_TEMPERATURE;
    }
    else
    {
      this->error_ = tErrorState::eNO_ERROR;
    }
  }
  this->so_error_state.Publish(error_, current_time);
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

    co_control_mode.Publish(ci_control_mode.Get(), ci_control_mode.GetTimestamp());
  }

  // error handling
  if ((this->error_ != tErrorState::eNO_ERROR) and not(ci_control_mode.Get() == tControlModeType::eMANUAL))
  {
    co_pump_online_ground.Publish(false);
    co_pump_online_room.Publish(false);
    co_pump_online_solar.Publish(false);
    return;
  }

  // handle set point
  if (ci_increase_set_point_temperature.HasChanged())
  {
    set_point_ += rrlib::si_units::tCelsius<double>(0.5);
    co_set_point_temperature.Publish(set_point_, ci_increase_set_point_temperature.GetTimestamp());
  }
  if (ci_decrease_set_point_temperature.HasChanged())
  {
    set_point_ -= rrlib::si_units::tCelsius<double>(0.5);
    co_set_point_temperature.Publish(set_point_, ci_decrease_set_point_temperature.GetTimestamp());
  }
  if (ci_reset_set_point_temperature.HasChanged())
  {
    set_point_ = par_temperature_set_point_room.Get();
    co_set_point_temperature.Publish(set_point_, ci_reset_set_point_temperature.GetTimestamp());
  }

  // determine state
  bool state_changed = false;
  std::unique_ptr<heat_control_states::tState> next_state;
  control_state_->ComputeControlState(next_state, temperatures_);
  state_changed = control_state_->HasChanged();
  if (state_changed)
  {
    control_state_ = std::move(next_state);
    co_heating_state.Publish(control_state_->GetCurrentState(), rrlib::time::Now());
  }

  // control mode
  switch (ci_control_mode.Get())
  {
  case tControlModeType::eAUTOMATIC:
  {
    if (state_changed)
    {
      // get pump settings
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
  {
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
  }
  break;
  case tControlModeType::eAUTOMATIC_NO_GROUND:
  {
    if (state_changed)
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
  }
  break;
  case tControlModeType::eAUTOMATIC_SOLAR_ONLY:
  {
    if (state_changed)
    {
      auto pumps = control_state_->GetPumpSettings();
      co_pump_online_room.Publish(false);
      co_pump_online_ground.Publish(false);
      co_pump_online_solar.Publish(pumps.IsSolarOnline());
    }
  }
  break;
  default:
    co_pump_online_ground.Publish(false);
    co_pump_online_room.Publish(false);
    co_pump_online_solar.Publish(false);
  };

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

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
