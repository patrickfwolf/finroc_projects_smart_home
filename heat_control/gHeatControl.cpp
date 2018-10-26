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
/*!\file    projects/smart_home/heat_control/gHeatControl.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-11
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/gHeatControl.h"

//----------------------------------------------------------------------
// External includes
//----------------------------------------------------------------------
#include <cassert>

#ifdef _LIB_WIRING_PI_PRESENT_
#include "libraries/gpio_raspberry_pi/mRaspberryIO.h"
#endif

#include "libraries/signal_filters/mExponentialFilter.h"

//----------------------------------------------------------------------
// Internal includes
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/mController.h"
#include "projects/smart_home/heat_control/mHardwareAbstraction.h"

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
static runtime_construction::tStandardCreateModuleAction<gHeatControl> cCREATE_ACTION_FOR_G_RASPBERRYPIHEATINGCONTROL("HeatControl");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// gHeatControl constructor
//----------------------------------------------------------------------
gHeatControl::gHeatControl(core::tFrameworkElement *parent, const std::string &name,
                           const std::string &structure_config_file) :
  tSenseControlGroup(parent, name, structure_config_file, true), // change to 'true' to make group's ports shared (so that ports in other processes can connect to its output and/or input ports)
  ci_control_mode(tControlModeType::eAUTOMATIC),
  ci_manual_pump_online_solar(false),
  ci_manual_pump_online_room(false),
  ci_manual_pump_online_ground(false),
  ci_disable_pump_room(false),
  ci_disable_pump_ground(false)
{
  auto controller = new mController(this, "Heat Controller");
  controller->par_temperature_set_point_room.Set(22.0);

  this->ci_control_mode.ConnectTo(controller->ci_control_mode);

  this->ci_manual_pump_online_ground.ConnectTo(controller->ci_manual_pump_online_ground);
  this->ci_manual_pump_online_room.ConnectTo(controller->ci_manual_pump_online_room);
  this->ci_manual_pump_online_solar.ConnectTo(controller->ci_manual_pump_online_solar);

  this->ci_disable_pump_ground.ConnectTo(controller->ci_disable_pump_ground);
  this->ci_disable_pump_room.ConnectTo(controller->ci_disable_pump_room);

  this->so_led_online_green.ConnectTo(controller->co_led_online_green);
  this->so_led_online_yellow.ConnectTo(controller->co_led_online_yellow);
  this->so_led_online_red.ConnectTo(controller->co_led_online_red);

  this->so_pump_online_ground.ConnectTo(controller->co_pump_online_ground);
  this->so_pump_online_room.ConnectTo(controller->co_pump_online_room);
  this->so_pump_online_solar.ConnectTo(controller->co_pump_online_solar);

  this->co_control_mode.ConnectTo(controller->co_control_mode);
  this->co_heating_state.ConnectTo(controller->co_heating_state);

  auto hardware_interface = new mHardwareAbstraction(this, "Hardware Abstraction");
  hardware_interface->ci_pump_online_ground.ConnectTo(controller->co_pump_online_ground);
  hardware_interface->ci_pump_online_room.ConnectTo(controller->co_pump_online_room);
  hardware_interface->ci_pump_online_solar.ConnectTo(controller->co_pump_online_solar);

  auto exponential_filter_raum = new signal_filters::mExponentialFilter<rrlib::si_units::tTemperature<double>>(this, "Filtered Temperature Room");
  exponential_filter_raum->par_number_of_ports.Set(1);
  exponential_filter_raum->Init();
  exponential_filter_raum->par_weight.Set(0.00035);
  hardware_interface->so_temperature_room.ConnectTo(exponential_filter_raum->in_input_values.at(0));
  exponential_filter_raum->out_filtered_values.at(0).ConnectTo(controller->si_temperature_room);

  auto exponential_filter_boden = new signal_filters::mExponentialFilter<rrlib::si_units::tTemperature<double>>(this, "Filtered Temperature Ground");
  exponential_filter_boden->par_number_of_ports.Set(1);
  exponential_filter_boden->Init();
  exponential_filter_boden->par_weight.Set(0.0025);
  hardware_interface->so_temperature_ground.ConnectTo(exponential_filter_boden->in_input_values.at(0));
  exponential_filter_boden->out_filtered_values.at(0).ConnectTo(controller->si_temperature_ground);

  auto exponential_filter_solar = new signal_filters::mExponentialFilter<rrlib::si_units::tTemperature<double>>(this, "Filtered Temperature Solar");
  exponential_filter_solar->par_number_of_ports.Set(1);
  exponential_filter_solar->Init();
  exponential_filter_solar->par_weight.Set(0.0025);
  hardware_interface->so_temperature_solar.ConnectTo(exponential_filter_solar->in_input_values.at(0));
  exponential_filter_solar->out_filtered_values.at(0).ConnectTo(controller->si_temperature_solar);

  auto exponential_filter_speicher = new signal_filters::mExponentialFilter<rrlib::si_units::tTemperature<double>>(this, "Filtered Temperature Boiler");
  exponential_filter_speicher->par_number_of_ports.Set(1);
  exponential_filter_speicher->Init();
  exponential_filter_speicher->par_weight.Set(0.0025);
  hardware_interface->so_temperature_boiler.ConnectTo(exponential_filter_speicher->in_input_values.at(0));
  exponential_filter_speicher->out_filtered_values.at(0).ConnectTo(controller->si_temperature_boiler);

  this->so_temperature_room.ConnectTo(exponential_filter_raum->out_filtered_values.at(0));
  this->so_temperature_ground.ConnectTo(exponential_filter_boden->out_filtered_values.at(0));
  this->so_temperature_solar.ConnectTo(exponential_filter_solar->out_filtered_values.at(0));
  this->so_temperature_boiler.ConnectTo(exponential_filter_speicher->out_filtered_values.at(0));

  hardware_interface->par_mcp3008_reference_voltage.Set(5.0);
  hardware_interface->par_mcp3008_supply_voltage.Set(5.0);
  hardware_interface->par_pt1000_pre_resistance_boden.Set(1922);
  hardware_interface->par_pt1000_pre_resistance_raum.Set(1950);
  hardware_interface->par_pt1000_pre_resistance_solar.Set(1987);
  hardware_interface->par_pt1000_pre_resistance_speicher.Set(1949);

#ifdef _LIB_WIRING_PI_PRESENT_
  auto gpio_interface = new finroc::gpio_raspberry_pi::mRaspberryIO(this, "Raspberry Pi GPIO Interface");
  gpio_interface->par_configuration_file.Set("$FINROC_PROJECT_HOME/etc/heat_control_gpio_config.xml");
  gpio_interface->Init();
  gpio_interface->GetInputs().ConnectByName(hardware_interface->GetControllerOutputs(), true);
  gpio_interface->GetOutputs().ConnectByName(hardware_interface->GetSensorInputs(), true);
#endif

}

//----------------------------------------------------------------------
// gHeatControl destructor
//----------------------------------------------------------------------
gHeatControl::~gHeatControl()
{}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
