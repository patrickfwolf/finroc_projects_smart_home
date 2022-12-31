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
/*!\file    projects/smart_home/heat_control_states/tSolar.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2014-05-14
 *
 */

#include "projects/smart_home/heat_control_states/tSolar.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control_states/tRoomSolar.h"
#include "projects/smart_home/heat_control_states/tGroundSolar.h"
#include "projects/smart_home/heat_control_states/tReady.h"

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
void tSolar::ComputeControlState(std::unique_ptr<tState> & state, const shared::tTemperatures & temperatures)
{

  // Solar less than boiler offset
  if (temperatures.GetSolar() - temperatures.GetBoiler() < shared::cSOLAR_DIFF_BOILER_LOW)
  {
    RRLIB_LOG_PRINT(DEBUG, "Solar -> Ready");
    state = std::unique_ptr<tState>(new tReady());
    this->SetChanged(true);
    return;
  }

  // Room under set-point and boiler under high threshold temperature and boiler temperature higher than room
  if ((temperatures.GetRoomSetPoint() - temperatures.GetRoom() >= shared::cROOM_DIFF_SETPOINT_HIGH) and
      (temperatures.GetBoiler() < shared::cROOM_BOILER_MAX) and
      (temperatures.GetBoiler() - temperatures.GetRoom() >= shared::cROOM_DIFF_BOILER_HIGH))
  {
    RRLIB_LOG_PRINT(DEBUG, "Solar -> Room Solar");

    state = std::unique_ptr<tState>(new tRoomSolar());
    this->SetChanged(true);
    return;
  }

  // Boiler higher than ground temperature and boiler warmer than minimum temperature
  if ((temperatures.GetBoiler() - temperatures.GetGround() >= shared::cGROUND_DIFF_BOILER_HIGH) and
      (temperatures.GetBoiler() > shared::cGROUND_BOILER_MIN))
  {
    RRLIB_LOG_PRINT(DEBUG, "Solar -> Ground Solar");

    state = std::unique_ptr<tState>(new tGroundSolar());
    this->SetChanged(true);
    return;
  }

  // No change
  this->SetChanged(false);
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

