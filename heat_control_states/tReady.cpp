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

#include "projects/smart_home/heat_control_states/tReady.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control_states/tGround.h"
#include "projects/smart_home/heat_control_states/tRoom.h"
#include "projects/smart_home/heat_control_states/tSolar.h"

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
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
void tReady::ComputeControlState(std::unique_ptr<tState> & state, const shared::tTemperatures &temperatures)
{
  // Solartemperatur mehr als 6°C größer als Speichertemperatur
  if (temperatures.GetSolar() - temperatures.GetBoiler() >= shared::cSOLAR_DIFF_BOILER_HIGH)
  {
    state = std::unique_ptr<tState>(new tSolar());
    this->SetChanged(true);
    return;
  }

  // Raumtemperatur unter Sollwert und Speichertemperatur unter 50°C und Speichertemperatur höher als Raumtemperatur
  if ((temperatures.GetRoom() < (temperatures.GetRoomSetPoint() - shared::cROOM_DIFF_SETPOINT_LOW)) and
      (temperatures.GetBoiler() < shared::cROOM_BOILER_MAX) and
      (temperatures.GetBoiler() > temperatures.GetRoom() + shared::cROOM_DIFF_BOILER_HIGH))
  {
    state = std::unique_ptr<tState>(new tRoom());
    this->SetChanged(true);
    return;
  }

  // Speichertemperatur höher als Bodenplattentemperatur und Speichertemperatur höher als Mindestspeichertemperatur
  if ((temperatures.GetBoiler() > shared::cGROUND_BOILER_MIN) and
      (temperatures.GetBoiler() > temperatures.GetGround() + shared::cGROUND_DIFF_BOILER_HIGH))
  {
    state = std::unique_ptr<tState>(new tGround());
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
