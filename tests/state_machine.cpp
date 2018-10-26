//
// You received this file as part of RRLib
// Robotics Research Library
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
/*!\file    projects/smart_home/test/state_machine.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-07-09
 *
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/util/tUnitTestSuite.h"
#include <memory>
#include <iostream>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include <cassert>

#include "projects/smart_home/heat_control_states/tReady.h"
#include "projects/smart_home/heat_control_states/tGround.h"
#include "projects/smart_home/heat_control_states/tRoom.h"
#include "projects/smart_home/heat_control_states/tSolar.h"
#include "projects/smart_home/heat_control_states/tRoomGround.h"
#include "projects/smart_home/heat_control_states/tGroundSolar.h"
#include "projects/smart_home/heat_control_states/tRoomSolar.h"
#include "projects/smart_home/heat_control_states/tGroundRoomSolar.h"

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
namespace tests
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------
class StateMachine : public rrlib::util::tUnitTestSuite
{
  RRLIB_UNIT_TESTS_BEGIN_SUITE(StateMachine);
  RRLIB_UNIT_TESTS_ADD_TEST(Functions);
  RRLIB_UNIT_TESTS_ADD_TEST(PumpValues);
  RRLIB_UNIT_TESTS_ADD_TEST(StateChange);
  RRLIB_UNIT_TESTS_END_SUITE;

private:

  void Functions()
  {
    heat_control_states::tReady ready;
    RRLIB_UNIT_TESTS_ASSERT(not ready.HasChanged());

  }

  void PumpValues()
  {
    heat_control_states::tReady ready;
    RRLIB_UNIT_TESTS_EQUALITY(false, ready.GetPumpSettings().IsGroundOnline());
    RRLIB_UNIT_TESTS_EQUALITY(false, ready.GetPumpSettings().IsRoomOnline());
    RRLIB_UNIT_TESTS_EQUALITY(false, ready.GetPumpSettings().IsSolarOnline());
    RRLIB_UNIT_TESTS_ASSERT(heat_control_states::tCurrentState::eREADY == ready.GetCurrentState());

    heat_control_states::tGround ground;
    RRLIB_UNIT_TESTS_EQUALITY(true, ground.GetPumpSettings().IsGroundOnline());
    RRLIB_UNIT_TESTS_EQUALITY(false, ground.GetPumpSettings().IsRoomOnline());
    RRLIB_UNIT_TESTS_EQUALITY(false, ground.GetPumpSettings().IsSolarOnline());
    RRLIB_UNIT_TESTS_ASSERT(heat_control_states::tCurrentState::eGROUND == ground.GetCurrentState());

    heat_control_states::tRoom room;
    RRLIB_UNIT_TESTS_EQUALITY(false, room.GetPumpSettings().IsGroundOnline());
    RRLIB_UNIT_TESTS_EQUALITY(true, room.GetPumpSettings().IsRoomOnline());
    RRLIB_UNIT_TESTS_EQUALITY(false, room.GetPumpSettings().IsSolarOnline());
    RRLIB_UNIT_TESTS_ASSERT(heat_control_states::tCurrentState::eROOM == room.GetCurrentState());

    heat_control_states::tRoomSolar room_solar;
    RRLIB_UNIT_TESTS_EQUALITY(false, room_solar.GetPumpSettings().IsGroundOnline());
    RRLIB_UNIT_TESTS_EQUALITY(true, room_solar.GetPumpSettings().IsRoomOnline());
    RRLIB_UNIT_TESTS_EQUALITY(true, room_solar.GetPumpSettings().IsSolarOnline());
    RRLIB_UNIT_TESTS_ASSERT(heat_control_states::tCurrentState::eSOLAR_ROOM == room_solar.GetCurrentState());

    heat_control_states::tRoomGround room_ground;
    RRLIB_UNIT_TESTS_EQUALITY(true, room_ground.GetPumpSettings().IsGroundOnline());
    RRLIB_UNIT_TESTS_EQUALITY(true, room_ground.GetPumpSettings().IsRoomOnline());
    RRLIB_UNIT_TESTS_EQUALITY(false, room_ground.GetPumpSettings().IsSolarOnline());
    RRLIB_UNIT_TESTS_ASSERT(heat_control_states::tCurrentState::eROOM_GROUND == room_ground.GetCurrentState());

    heat_control_states::tGroundSolar ground_solar;
    RRLIB_UNIT_TESTS_EQUALITY(true, ground_solar.GetPumpSettings().IsGroundOnline());
    RRLIB_UNIT_TESTS_EQUALITY(false, ground_solar.GetPumpSettings().IsRoomOnline());
    RRLIB_UNIT_TESTS_EQUALITY(true, ground_solar.GetPumpSettings().IsSolarOnline());
    RRLIB_UNIT_TESTS_ASSERT(heat_control_states::tCurrentState::eSOLAR_GROUND == ground_solar.GetCurrentState());

    heat_control_states::tGroundRoomSolar ground_room_solar;
    RRLIB_UNIT_TESTS_EQUALITY(true, ground_room_solar.GetPumpSettings().IsGroundOnline());
    RRLIB_UNIT_TESTS_EQUALITY(true, ground_room_solar.GetPumpSettings().IsRoomOnline());
    RRLIB_UNIT_TESTS_EQUALITY(true, ground_room_solar.GetPumpSettings().IsSolarOnline());
    RRLIB_UNIT_TESTS_ASSERT(heat_control_states::tCurrentState::eSOLAR_ROOM_GROUND == ground_room_solar.GetCurrentState());
  }

  void StateChange()
  {
    std::unique_ptr<heat_control_states::tState> state;
    std::unique_ptr<heat_control_states::tState> new_state;
    state = std::unique_ptr<heat_control_states::tState>(new heat_control_states::tReady());

    RRLIB_UNIT_TESTS_ASSERT(heat_control_states::tCurrentState::eREADY == state->GetCurrentState());
    RRLIB_UNIT_TESTS_ASSERT(not state->HasChanged());

    rrlib::si_units::tCelsius<double> boiler = 50.0;
    rrlib::si_units::tCelsius<double> room = 20.0;
    rrlib::si_units::tCelsius<double> solar = 50.0;
    rrlib::si_units::tCelsius<double> ground = 30.0;
    rrlib::si_units::tCelsius<double> room_set_point = 20.0;

    /*
    eREADY = 0,
    eSOLAR = 1,
    eROOM = 2,
    eGROUND = 3,
    eSOLAR_ROOM = 4,
    eSOLAR_GROUND = 5,
    eROOM_GROUND = 6,
    eSOLAR_ROOM_GROUND = 7
    */

    state->ComputeControlState(new_state, shared::tTemperatures(boiler, room, solar, ground, room_set_point));
    if (state->HasChanged())
    {
      state = std::move(new_state);
    }
    RRLIB_UNIT_TESTS_ASSERT(not state->HasChanged());
//    RRLIB_UNIT_TESTS_EQUALITY(static_cast<int>(heat_control_states::tCurrentState::eREADY), static_cast<int>(state->GetCurrentState()));
//
//    // ----------  Ready transitions  ------------------
//
//    room_set_point = 20.0;
//    state->ComputeControlState(new_state, shared::tTemperatures(boiler, room, solar, ground, room_set_point));
//    if (state->HasChanged())
//    {
//      state = std::move(new_state);
//    }
//    RRLIB_UNIT_TESTS_ASSERT(not state->HasChanged());
//    RRLIB_UNIT_TESTS_EQUALITY(static_cast<int>(heat_control_states::tCurrentState::eREADY),  static_cast<int>(state->GetCurrentState()));
//
//
//    solar = 20.0;
//    state->ComputeControlState(new_state, shared::tTemperatures(boiler, room, solar, ground, room_set_point));
//    RRLIB_UNIT_TESTS_ASSERT(state->HasChanged());
//    if (state->HasChanged())
//    {
//      state = std::move(new_state);
//    }
//    RRLIB_UNIT_TESTS_EQUALITY(1, static_cast<int>(state->GetCurrentState()));
//
//    boiler = 17.9;
//    room = 20.0;
//    state->ComputeControlState(new_state, shared::tTemperatures(boiler, room, solar, ground, room_set_point));
//    RRLIB_UNIT_TESTS_ASSERT(not state->HasChanged());
//    if (state->HasChanged())
//    {
//      state = std::move(new_state);
//    }
//    RRLIB_UNIT_TESTS_EQUALITY(1, static_cast<int>(state->GetCurrentState()));
//
//
//    solar = 18.0;
//    state->ComputeControlState(new_state, shared::tTemperatures(boiler, room, solar, ground, room_set_point));
//    RRLIB_UNIT_TESTS_ASSERT(state->HasChanged());
//    if (state->HasChanged())
//    {
//      state = std::move(new_state);
//    }
//    RRLIB_UNIT_TESTS_EQUALITY(0, static_cast<int>(state->GetCurrentState()));

  }
};

RRLIB_UNIT_TESTS_REGISTER_SUITE(StateMachine);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
