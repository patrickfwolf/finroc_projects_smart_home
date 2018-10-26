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
/*!\file    projects/smart_home/vent_control/mController.cpp
 *
 * \author  Patrick Vatter
 *
 * \date    2015-03-12
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/vent_control/mController.h"

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
namespace vent_control
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<mController> cCREATE_ACTION_FOR_M_VENTILATION("Ventilation");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// mController constructor
//----------------------------------------------------------------------
mController::mController(core::tFrameworkElement *parent, const std::string &name) :
  structure::tSenseControlModule(parent, name, false),
  par_temperature_velocity_threshold(this, "Temperature Velocity Threshold", (5.0 / 60.0)), // 5 degree per hour
  par_time_delta_threshold(this, "Time Delta Threshold", 10)
{
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
}

//----------------------------------------------------------------------
// mController Sense
//----------------------------------------------------------------------
void mController::Sense()
{

  // ventilation manually disabled
  if (ci_ventilation_mode.Get() == tVentilationMode::eOFFLINE)
  {
    co_gpio_ventilation.Publish(cVENTILATION_OFFINE);
    co_ventilation_online.Publish(false);
  }
  else if (ci_ventilation_mode.Get() == tVentilationMode::eONLINE)
  {
    co_gpio_ventilation.Publish(cVENTILATION_ONLINE);
    co_ventilation_online.Publish(true);
  }
  else if (ci_ventilation_mode.Get() == tVentilationMode::eAUTOMATIC)
  {
    auto time = rrlib::util::tTime::Now();
    auto time_delta = rrlib::signal_processing::utils::GetTimeDelta(current_time, time);

    // check time of last date to determine failure of heating control or network connection
    if (si_temperature_speicher.HasChanged())
    {
      current_temperature = si_temperature_speicher.Get();

      // advance time (only if new data arrived)
      current_time = time;

      // initialize
      if (derivation.HasTimeZero())
      {
        derivation.SetValue(current_temperature, time);
      }

      // determine temperature velocity every 15 minutes
      if (derivation.DetermineTimeDelta(time) >= rrlib::si_units::tTime<double>(15 * 60))
      {
        derivation.Derivate(current_temperature, time);
      }

      // Publish velocity
      so_temperature_velocity.Publish(derivation.GetValue());

      // check if velocity is above threshold
      if (derivation.GetValue() < par_temperature_velocity_threshold.Get())
      {
        co_gpio_ventilation.Publish(cVENTILATION_ONLINE);
        co_ventilation_online.Publish(true);
      }
      else
      {
        // check if solar panel provides heat
        // it is assumed that solar pump is running or solar panel is warmer than the main storage
        if (si_pump_online_solar.Get() or si_temperature_solar.Get() > current_temperature)
        {
          co_gpio_ventilation.Publish(cVENTILATION_ONLINE);
          co_ventilation_online.Publish(true);
        }
        else
        {
          // heat is not provided by solar panel

          co_gpio_ventilation.Publish(cVENTILATION_OFFINE);
          co_ventilation_online.Publish(false);
        }
      }
    }
    else
    {
      // determine time delta, turn ventilation off if no data arrive. This indicates network problems or a malfunction of the heating control
      if (time_delta > par_time_delta_threshold.Get())
      {
        co_gpio_ventilation.Publish(cVENTILATION_OFFINE);
        co_ventilation_online.Publish(false);
      }
    }
  }
}

//----------------------------------------------------------------------
// mController Control
//----------------------------------------------------------------------
void mController::Control()
{}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
