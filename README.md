# finroc_projects_smart_home
FINROC project (https://www.finroc.org) for home automation incl. a heat control, vent control, and user interface. Each part runs on a Raspberry Pi and is connected via Ethernet. Additionally, the heat control state can be accessed via a Fingui through any device with an installation of Finroc (e.g. Laptop, workstation, Rasperry Pi with screen).

The Finroc version requires a special version of the rrlib_si_units (not the one from finroc.org). You can get it here:
hg clone https://agrosy.cs.uni-kl.de/hg/rrlib_si_units
Use this version to replace the default repository of the rrlib which comes with your Finroc installation from finroc.org. The checkout should done with the user 'anonymous' and your email as password.

Additionally, the following finroc libraries are required, place them in the respective libraries or rrlib folders
hg clone https://agrosy.cs.uni-kl.de/hg/finroc_libraries_gpio_raspberry_pi
hg clone https://agrosy.cs.uni-kl.de/hg/rrlib_gpio

The Raspberry Pi libraries depend on the wiring_pi library. Install it and run 
make libdb makefile
from your finroc home directory to check they are installed and detected by Finroc properly.
