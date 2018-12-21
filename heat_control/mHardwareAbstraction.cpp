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
/*!\file    projects/raspberry_pi_heating_control/control/mHardwareAbstraction.cpp
 *
 * \author  Patrick Vatter
 *
 * \date    2015-03-12
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/mHardwareAbstraction.h"

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
static runtime_construction::tStandardCreateModuleAction<mHardwareAbstraction> cCREATE_ACTION_FOR_M_HARDWAREINTERFACE("HardwareInterface");


//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// mHardwareAbstraction constructor
//----------------------------------------------------------------------
mHardwareAbstraction::mHardwareAbstraction(core::tFrameworkElement *parent, const std::string &name) :
  tSenseControlModule(parent, name, false), // change to 'true' to make module's ports shared (so that ports in other processes can connect to its sensor outputs and controller inputs)
  si_mcp3008_voltage_boiler_raw(512),
  si_mcp3008_voltage_room_raw(512),
  si_mcp3008_voltage_solar_raw(512),
  si_mcp3008_voltage_ground_raw(512),
  ci_pump_online_solar(false),
  ci_pump_online_room(false),
  ci_pump_online_ground(false),
  co_gpio_pump_online_solar(true),
  co_gpio_pump_online_room(true),
  co_gpio_pump_online_ground(true),
  co_gpio_pump_led_online_solar(false),
  co_gpio_pump_led_online_room(false),
  co_gpio_pump_led_online_ground(false),
  par_mcp3008_reference_voltage(3.3),
  par_mcp3008_supply_voltage(5.0),
  par_pt1000_pre_resistance_solar(1000),
  par_pt1000_pre_resistance_boden(1000),
  par_pt1000_pre_resistance_raum(1000),
  par_pt1000_pre_resistance_speicher(1000),
  mcp3008_converter_(3.3)
{}

//----------------------------------------------------------------------
// mHardwareAbstraction destructor
//----------------------------------------------------------------------
mHardwareAbstraction::~mHardwareAbstraction()
{}


//----------------------------------------------------------------------
// mHardwareAbstraction OnParameterChange
//----------------------------------------------------------------------
void mHardwareAbstraction::OnParameterChange()
{
  if(par_mcp3008_reference_voltage.HasChanged())
  {
    mcp3008_converter_.SetReferenceVoltage(par_mcp3008_reference_voltage.Get());
  }
}

//----------------------------------------------------------------------
// mHardwareAbstraction Sense
//----------------------------------------------------------------------
void mHardwareAbstraction::Sense()
{
  if (this->SensorInputChanged())
  {
    rrlib::si_units::tVoltage<double> reference_voltage = par_mcp3008_reference_voltage.Get();
    rrlib::si_units::tVoltage<double> supply_voltage = par_mcp3008_supply_voltage.Get();
    rrlib::si_units::tVoltage<double> voltage_boiler = mcp3008_converter_.ConvertADValueToVoltage(si_mcp3008_voltage_boiler_raw.Get());
    rrlib::si_units::tVoltage<double> voltage_ground = mcp3008_converter_.ConvertADValueToVoltage(si_mcp3008_voltage_ground_raw.Get());
    rrlib::si_units::tVoltage<double> voltage_room = mcp3008_converter_.ConvertADValueToVoltage(si_mcp3008_voltage_room_raw.Get());
    rrlib::si_units::tVoltage<double> voltage_solar = mcp3008_converter_.ConvertADValueToVoltage(si_mcp3008_voltage_solar_raw.Get());

    rrlib::si_units::tCelsius<double> temperature_boiler = PT1000MCP3008VoltageToTemperature(
        voltage_boiler, reference_voltage, supply_voltage, par_pt1000_pre_resistance_speicher.Get());
    rrlib::si_units::tCelsius<double> temperature_ground = PT1000MCP3008VoltageToTemperature(
        voltage_ground, reference_voltage, supply_voltage, par_pt1000_pre_resistance_boden.Get());
    rrlib::si_units::tCelsius<double> temperature_room = PT1000MCP3008VoltageToTemperature(
        voltage_room, reference_voltage, supply_voltage, par_pt1000_pre_resistance_raum.Get());
    rrlib::si_units::tCelsius<double> temperature_solar = PT1000MCP3008VoltageToTemperature(
        voltage_solar, reference_voltage, supply_voltage, par_pt1000_pre_resistance_solar.Get());

    so_temperature_boiler.Publish(temperature_boiler);
    so_temperature_ground.Publish(temperature_ground);
    so_temperature_room.Publish(temperature_room);
    so_temperature_solar.Publish(temperature_solar);

    so_resistance_boiler.Publish(pt1000_converter_.GetResistance(temperature_boiler));
    so_resistance_ground.Publish(pt1000_converter_.GetResistance(temperature_ground));
    so_resistance_room.Publish(pt1000_converter_.GetResistance(temperature_room));
    so_resistance_solar.Publish(pt1000_converter_.GetResistance(temperature_solar));

  }
}

//----------------------------------------------------------------------
// mHardwareAbstraction Control
//----------------------------------------------------------------------
void mHardwareAbstraction::Control()
{
  if (this->ControllerInputChanged())
  {
    if (ci_pump_online_solar.HasChanged())
    {
      co_gpio_pump_online_solar.Publish(not ci_pump_online_solar.Get());
      co_gpio_pump_led_online_solar.Publish(ci_pump_online_solar.Get());
    }
    if (ci_pump_online_ground.HasChanged())
    {
      co_gpio_pump_online_ground.Publish(not ci_pump_online_ground.Get());
      co_gpio_pump_led_online_ground.Publish(ci_pump_online_ground.Get());
    }
    if (ci_pump_online_room.HasChanged())
    {
      co_gpio_pump_online_room.Publish(not ci_pump_online_room.Get());
      co_gpio_pump_led_online_room.Publish(ci_pump_online_room.Get());
    }
  }
}


//----------------------------------------------------------------------
// mHardwareAbstraction ConvertVoltageToTemperature
//----------------------------------------------------------------------
rrlib::si_units::tCelsius<double> mHardwareAbstraction::PT1000MCP3008VoltageToTemperature(
    const rrlib::si_units::tVoltage<double> & ad_output_voltage,
    const rrlib::si_units::tVoltage<double> & ad_reference_voltage,
    const rrlib::si_units::tVoltage<double> & ad_supply_voltage,
    const rrlib::si_units::tElectricResistance<double> & sensor_pre_resistance) const
{
  /*
   * _____ V_supply
   *   |
   *  [ ] PT1000
   *   |____ ADC V_ref
   *   |
   *  [ ] R
   * __|__ GND
   *
   * V_supply / V_ref = R_pre + R_pt / R_pre
   *
   */

  rrlib::si_units::tElectricResistance<double> resistance = sensor_pre_resistance * ((ad_supply_voltage / ad_output_voltage).Value() - 1.0);
  return pt1000_converter_.GetTemperature(resistance);
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
