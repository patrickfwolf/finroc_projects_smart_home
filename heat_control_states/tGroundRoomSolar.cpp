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

//----------------------------------------------------------------------
/*!\file    ControlModeState/ControlModeState.h/
 *
 * \author  Patrick Wolf
 *
 * \date    2014-05-14
 *
 */

#include "projects/smart_home/heat_control_states/tGroundRoomSolar.h"
//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control_states/tGroundSolar.h"
#include "projects/smart_home/heat_control_states/tRoomSolar.h"
#include "projects/smart_home/heat_control_states/tRoomGround.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace smart_home
{
namespace heat_control_states
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
void tGroundRoomSolar::ComputeControlState(std::unique_ptr<tState> & state, const shared::tTemperatures &temperatures)
{
  // Speichertemperatur unter 50°C oder Speichertemperatur niedriger als Bodentemperatur
  if ((temperatures.GetBoiler() < shared::cGROUND_BOILER_MIN) or
      (temperatures.GetBoiler() < temperatures.GetGround() + shared::cGROUND_DIFF_BOILER_LOW))
  {
    state = std::unique_ptr<tState>(new tRoomSolar());
    this->SetChanged(true);
    return;
  }

  // Raumtemperatur größer Solltemperatur +0,8°C oder Speichertemperatur größer als 50°C oder Speichertemperatur niedriger als Raumtemperatur oder Speicher zu warm
  if ((temperatures.GetRoom() >= (temperatures.GetRoomSetPoint() + shared::cROOM_DIFF_SETPOINT_HIGH)) or
      (temperatures.GetBoiler() >= shared::cROOM_BOILER_MAX) or
      (temperatures.GetBoiler() < temperatures.GetRoom() - shared::cROOM_DIFF_BOILER_LOW))
  {
    state = std::unique_ptr<tState>(new tGroundSolar());
    this->SetChanged(true);
    return;
  }

  // Solartemperatur weniger als 2°C größer als Speichertemperatur
  if (temperatures.GetSolar() - temperatures.GetBoiler() < shared::cSOLAR_DIFF_BOILER_LOW)
  {
    state = std::unique_ptr<tState>(new tRoomGround());
    this->SetChanged(true);
    return;
  }

  // no change
  this->SetChanged(false);
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
