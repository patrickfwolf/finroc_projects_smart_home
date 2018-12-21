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
/*!\file    projects/smart_home/heat_control/mHardwareAbstraction.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 * \brief Contains mHardwareAbstraction
 *
 * \b mHardwareAbstraction
 *
 * module that provides a hardware interface, a/d conversion and is capable of setting pumps.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__heat_control__mHardwareAbstraction_h__
#define __projects__smart_home__heat_control__mHardwareAbstraction_h__

#include "plugins/structure/tSenseControlModule.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/shared/tPT.h"
#include "projects/smart_home/shared/tMCP3008Converter.h"

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
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * module that provides a hardware interface, a/d conversion and is capable of setting pumps.
 */
class mHardwareAbstraction : public structure::tSenseControlModule
{

//----------------------------------------------------------------------
// Ports (These are the only variables that may be declared public)
//----------------------------------------------------------------------
public:

  tSensorInput<unsigned short> si_mcp3008_voltage_boiler_raw;
  tSensorInput<unsigned short> si_mcp3008_voltage_room_raw;
  tSensorInput<unsigned short> si_mcp3008_voltage_solar_raw;
  tSensorInput<unsigned short> si_mcp3008_voltage_ground_raw;

  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_boiler;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_room;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_solar;
  tSensorOutput<rrlib::si_units::tCelsius<double>> so_temperature_ground;

  tSensorOutput<rrlib::si_units::tElectricResistance<double>> so_resistance_boiler;
  tSensorOutput<rrlib::si_units::tElectricResistance<double>> so_resistance_room;
  tSensorOutput<rrlib::si_units::tElectricResistance<double>> so_resistance_solar;
  tSensorOutput<rrlib::si_units::tElectricResistance<double>> so_resistance_ground;

  tSensorOutput<rrlib::si_units::tVoltage<double>> so_mcp3008_voltage_boiler;
  tSensorOutput<rrlib::si_units::tVoltage<double>> so_mcp3008_voltage_room;
  tSensorOutput<rrlib::si_units::tVoltage<double>> so_mcp3008_voltage_solar;
  tSensorOutput<rrlib::si_units::tVoltage<double>> so_mcp3008_voltage_ground;

  tControllerInput<bool> ci_pump_online_solar;
  tControllerInput<bool> ci_pump_online_room;
  tControllerInput<bool> ci_pump_online_ground;

  tControllerOutput<bool> co_gpio_pump_online_solar;
  tControllerOutput<bool> co_gpio_pump_online_room;
  tControllerOutput<bool> co_gpio_pump_online_ground;

  tControllerOutput<bool> co_gpio_pump_led_online_solar;
  tControllerOutput<bool> co_gpio_pump_led_online_room;
  tControllerOutput<bool> co_gpio_pump_led_online_ground;

  tParameter<rrlib::si_units::tVoltage<double>> par_mcp3008_reference_voltage;
  tParameter<rrlib::si_units::tVoltage<double>> par_mcp3008_supply_voltage;

  tParameter<rrlib::si_units::tElectricResistance<double>> par_pt1000_pre_resistance_solar;
  tParameter<rrlib::si_units::tElectricResistance<double>> par_pt1000_pre_resistance_boden;
  tParameter<rrlib::si_units::tElectricResistance<double>> par_pt1000_pre_resistance_raum;
  tParameter<rrlib::si_units::tElectricResistance<double>> par_pt1000_pre_resistance_speicher;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  mHardwareAbstraction(core::tFrameworkElement *parent, const std::string &name = "HardwareInterface");

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Destructor
   *
   * The destructor of modules is declared protected to avoid accidental deletion. Deleting
   * modules is already handled by the framework.
   */
  ~mHardwareAbstraction();

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  virtual void OnParameterChange() override;
  virtual void Sense() override;
  virtual void Control() override;

  virtual rrlib::si_units::tCelsius<double> PT1000MCP3008VoltageToTemperature(
      const rrlib::si_units::tVoltage<double> & ad_output_voltage,
      const rrlib::si_units::tVoltage<double> & ad_reference_voltage,
      const rrlib::si_units::tVoltage<double> & ad_supply_voltage,
      const rrlib::si_units::tElectricResistance<double> & sensor_pre_resistance) const;

  shared::tPT1000 pt1000_converter_;
  shared::tMCP3008Converter mcp3008_converter_;

  bool pump_boden_online_;
  bool pump_raum_online_;
  bool pump_solar_online_;


};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}



#endif
