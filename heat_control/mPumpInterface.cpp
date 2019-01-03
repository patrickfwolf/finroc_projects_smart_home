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
/*!\file    projects/raspberry_pi_heating_control/control/mPumpInterface.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/mPumpInterface.h"

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
namespace heat_control
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<mPumpInterface> cCREATE_ACTION_FOR_M_PUMPINTERFACE("PumpInterface");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// mPumpInterface constructor
//----------------------------------------------------------------------
mPumpInterface::mPumpInterface(core::tFrameworkElement *parent, const std::string &name) :
  tModule(parent, name, false), // change to 'true' to make module's ports shared (so that ports in other processes can connect to its sensor outputs and controller inputs)
  in_pump_online_solar(false),
  in_pump_online_room(false),
  in_pump_online_ground(false),
  out_gpio_pump_online_solar(true),
  out_gpio_pump_online_room(true),
  out_gpio_pump_online_ground(true),
  out_gpio_pump_led_online_solar(false),
  out_gpio_pump_led_online_room(false),
  out_gpio_pump_led_online_ground(false)
{}

//----------------------------------------------------------------------
// mPumpInterface destructor
//----------------------------------------------------------------------
mPumpInterface::~mPumpInterface()
{}

//----------------------------------------------------------------------
// mPumpInterface Control
//----------------------------------------------------------------------
void mPumpInterface::Update()
{
  if (this->InputChanged())
  {
    if (in_pump_online_solar.HasChanged())
    {
      out_gpio_pump_online_solar.Publish(not in_pump_online_solar.Get());
      out_gpio_pump_led_online_solar.Publish(in_pump_online_solar.Get());
    }
    if (in_pump_online_ground.HasChanged())
    {
      out_gpio_pump_online_ground.Publish(not in_pump_online_ground.Get());
      out_gpio_pump_led_online_ground.Publish(in_pump_online_ground.Get());
    }
    if (in_pump_online_room.HasChanged())
    {
      out_gpio_pump_online_room.Publish(not in_pump_online_room.Get());
      out_gpio_pump_led_online_room.Publish(in_pump_online_room.Get());
    }
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
