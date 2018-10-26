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
/*!\file    projects/smart_home/user_interface/mLED.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/user_interface/mLED.h"

//----------------------------------------------------------------------
// External includes
//----------------------------------------------------------------------
#include <cmath>

//----------------------------------------------------------------------
// Internal includes
//----------------------------------------------------------------------
#include "rrlib/signal_processing/signal_processing_utils.h"

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
namespace user_interface
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<mLED> cCREATE_ACTION_FOR_M_LED("LED");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// mLED constructor
//----------------------------------------------------------------------
mLED::mLED(core::tFrameworkElement *parent, const std::string &name) :
  tModule(parent, name, false),
  par_blink_duration(3.0)
{}

//----------------------------------------------------------------------
// mLED destructor
//----------------------------------------------------------------------
mLED::~mLED()
{}

//----------------------------------------------------------------------
// mLED Sense
//----------------------------------------------------------------------
void mLED::Update()
{
  // last update longer than 10s away
  auto now = rrlib::time::Now();
  if (time == rrlib::time::cNO_TIME or this->InputChanged())
  {
    time = now;
  }

  auto delta = rrlib::signal_processing::utils::GetTimeDelta(time, now);

  if (delta.Value() < 10.0)
  {
    out_red.Publish(in_red.Get());
    out_yellow.Publish(in_yellow.Get());
    out_green.Publish(in_green.Get());
  }
  else
  {
    double blink = par_blink_duration.Get().Value();
    double mod = std::fmod(delta.Value(), blink);
    bool red = (mod < (blink / 2.0)) ? false : true;

    out_red.Publish(red);
    out_yellow.Publish(false);
    out_green.Publish(false);
  }
}

//----------------------------------------------------------------------

}
}
}
