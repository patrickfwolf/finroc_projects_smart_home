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

#include "projects/smart_home/shared/tPT.h"

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
    shared::tPT1000 pt1000;
    shared::tPT100 pt100;

    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(0.0)), rrlib::si_units::tElectricResistance<double>(1000.0));
    RRLIB_UNIT_TESTS_EQUALITY(pt100.GetResistance(rrlib::si_units::tCelsius<double>(0.0)), rrlib::si_units::tElectricResistance<double>(100.0));

    RRLIB_UNIT_TESTS_EQUALITY(pt1000.GetTemperature(rrlib::si_units::tElectricResistance<double>(1000.0)), rrlib::si_units::tCelsius<double>(0.0));
    RRLIB_UNIT_TESTS_EQUALITY(pt100.GetTemperature(rrlib::si_units::tElectricResistance<double>(100.0)), rrlib::si_units::tCelsius<double>(0.0));

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(50.0))).ValueFactored(), 50.0, 0.001);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(50.0))).ValueFactored(), 50.0, 0.001);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(40.0))).ValueFactored(), 40.0, 0.001);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(40.0))).ValueFactored(), 40.0, 0.001);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(30.0))).ValueFactored(), 30.0, 0.001);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(30.0))).ValueFactored(), 30.0, 0.001);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(20.0))).ValueFactored(), 20.0, 0.001);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(20.0))).ValueFactored(), 20.0, 0.001);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(10.0))).ValueFactored(), 10.0, 0.001);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(10.0))).ValueFactored(), 10.0, 0.001);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(0.0))).ValueFactored(), 0.0, 0.001);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(0.0))).ValueFactored(), 0.0, 0.001);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(13.3))).ValueFactored(), 13.3, 0.001);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(13.3))).ValueFactored(), 13.3, 0.001);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(2.7))).ValueFactored(), 2.7, 0.001);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(2.7))).ValueFactored(), 2.7, 0.001);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(-50.0))).ValueFactored(), -50.0, 0.1);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(-50.0))).ValueFactored(), -50.0, 0.1);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(-10.0))).ValueFactored(), -10.0, 0.1);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(-10.0))).ValueFactored(), -10.0, 0.1);

    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt1000.GetTemperature(pt1000.GetResistance(rrlib::si_units::tCelsius<double>(-5.0))).ValueFactored(), -5.0, 0.1);
    RRLIB_UNIT_TESTS_EQUALITY_DOUBLE(pt100.GetTemperature(pt100.GetResistance(rrlib::si_units::tCelsius<double>(-5.0))).ValueFactored(), -5.0, 0.1);

  }

};

RRLIB_UNIT_TESTS_REGISTER_SUITE(PT1000);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
