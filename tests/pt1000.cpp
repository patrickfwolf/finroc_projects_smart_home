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

#include "projects/smart_home/shared/tPT1000Converter.h"

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
class PT1000 : public rrlib::util::tUnitTestSuite
{
  RRLIB_UNIT_TESTS_BEGIN_SUITE(PT1000);
  RRLIB_UNIT_TESTS_ADD_TEST(Coversions);
  RRLIB_UNIT_TESTS_END_SUITE;

private:

  void Coversions()
  {
    shared::tPT1000Converter pt1000;

    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(0.0)), rrlib::si_units::tElectricResistance<double>(1000.0));
    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(10.0)), shared::cPT1000_LOOKUP.at(210));

    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(rrlib::si_units::tElectricResistance<double>(1000.0)), rrlib::si_units::tCelsius<double>(0.0));


    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(50.0))),
        rrlib::si_units::tCelsius<double>(50.0));
    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(40.0))),
        rrlib::si_units::tCelsius<double>(40.0));
    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(30.0))),
        rrlib::si_units::tCelsius<double>(30.0));
    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(20.0))),
        rrlib::si_units::tCelsius<double>(20.0));
    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(10.0))),
        rrlib::si_units::tCelsius<double>(10.0));
//    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(2.3))),
//        rrlib::si_units::tCelsius<double>(2.3));
    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(0.0))),
        rrlib::si_units::tCelsius<double>(0.0));
//    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperatureFromResistance(pt1000.GetResistanceFromTemperature(rrlib::si_units::tCelsius<double>(-10.0))),
//        rrlib::si_units::tCelsius<double>(-10.0));

  }

};

RRLIB_UNIT_TESTS_REGISTER_SUITE(PT1000);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
