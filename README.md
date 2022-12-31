# finroc_projects_smart_home
FINROC project (https://www.finroc.org) for home automation including a heat control, vent control, and user interface. Each part runs on a Raspberry Pi and is connected via Ethernet. Additionally, the heat control state can be accessed via a Fingui through any device with an installation of Finroc (e.g. Laptop, workstation, Rasperry Pi with screen).

The software uses a version of the rrlib_si_units hosted here:

 hg clone https://agrosy.cs.uni-kl.de/hg/rrlib_si_units $FINROC_HOME/sources/cpp/rrlib/si_units
  
Use this version to replace the default repository of the rrlib which comes with your Finroc installation from finroc.org. The checkout should done with the user 'anonymous' and your email as password.

Additionally, the following finroc libraries are required, place them in the respective libraries or rrlib folders

 hg clone https://agrosy.cs.uni-kl.de/hg/finroc_libraries_gpio_raspberry_pi $FINROC_HOME/sources/cpp/libraries/gpio_raspberry_pi

 hg clone https://agrosy.cs.uni-kl.de/hg/rrlib_gpio $FINROC_HOME/sources/cpp/rrlib/gpio

The Raspberry Pi GPIO interface depends on wiring_pi. Download and install the lib, then run 

  make libdb makefile
  
to create the makefile. After sucessfully building it, run

 make smart_home
 
 to build it.
