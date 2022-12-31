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


#include "projects/smart_home/heat_control_states/tRoom.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control_states/tReady.h"
#include "projects/smart_home/heat_control_states/tRoomGround.h"
#include "projects/smart_home/heat_control_states/tRoomSolar.h"

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

void tRoom::ComputeControlState(std::unique_ptr<tState> & state, const shared::tTemperatures &temperatures)
{
  // room warm enough or boiler too hot or boiler not warm enough
  if ((temperatures.GetRoomSetPoint() - temperatures.GetRoom() < shared::cROOM_DIFF_SETPOINT_LOW) or
      (temperatures.GetBoiler() >= shared::cROOM_BOILER_MAX) or
      (temperatures.GetBoiler() - temperatures.GetRoom() < shared::cROOM_DIFF_BOILER_LOW))
  {
    RRLIB_LOG_PRINT(DEBUG, "Room -> Ready");
    state = std::unique_ptr<tState>(new tReady());
    this->SetChanged(true);
    return;
  }

  // solar warmer than boiler
  if (temperatures.GetSolar() - temperatures.GetBoiler() >= shared::cSOLAR_DIFF_BOILER_HIGH)
  {
    RRLIB_LOG_PRINT(DEBUG, "Room -> Room Solar");
    state = std::unique_ptr<tState>(new tRoomSolar());
    this->SetChanged(true);
    return;
  }

  // Boiler warmer than ground and hotter than minimum temperature
  if ((temperatures.GetBoiler() - temperatures.GetGround() >= shared::cGROUND_DIFF_BOILER_HIGH) and
      (temperatures.GetBoiler() > shared::cGROUND_BOILER_MIN))
  {
    RRLIB_LOG_PRINT(DEBUG, "Room -> Room Ground");
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
