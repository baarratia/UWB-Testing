# Qorvo Apple Nearby Interaction Beta

This project demonstrates the Nearby Interaction of Qorvo UWB DW3000 transceiver chip with Apple(R) U1 iOS-15 Reference Application.



### Supported Reference Embedded Application Target platforms

By default Qorvo supports the following target platforms. The list of supported targets could be extended.

- DWM3000EVB + nRF52832DK
- DWM3000EVB + nRF52833DK
- DWM3000EVB + nRF52840DK
- DWM3001CDK integrated module



## Installing of pre-requirements 

Embedded applications are developed for Segger Embedded Studio IDE and developed to be compatible with Apple(R) Nearby Interaction Reference Application out of box.



### Step 1: Install Segger Embedded Studio IDE

Install Segger Embedded Studio for your Host platform (Windows/Mac/Linux). 

https://www.segger.com/downloads/embedded-studio/



### Step 2: Install NRF Command Line Tools

NRF command line is required for "mergehex" post-build to merge into one binary the compiled project with the SoftDevice S113, which is required for BLE. 

NRF Command Line Tools can be installed from this link:
https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download



### Step 3: Install of Nordic nRF5 SDK

The Nordic nRF5 SDK v.16 is included to the package and does not need to be downloaded separately.

In case of difficulties, the reference Nordic SDK package nRF5_SDK_16.0.0_98a08e2 can be obtained from Nordic Semiconductors web-site:

https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5SDK160098a08e2.zip



## Running of the applications



### Step 1 : License agreement

* Read the license agreements "Qorvo Software License Agreement.pdf" in the main folder. 
  * In case of disagreements with license terms immediately delete the obtained package.
  * In case of absence of "Qorvo Software License Agreement.pdf" contact Qorvo to get a copy.
* Read the license agreements with Nordic Semiconductors in the - 'license.txt' in the SDK folder.



### Step 2 : Select the Segger Embedded Studio Target Project

Switch to the "qorvo_apple_nearby_interaction_beta" folder. 

Select the correct target project from the list below and click on it. Segger Embedded Studio will open the project for the selected target:

* Projects/nRF52832DK/ses/NRF52832DK.emProject
* Projects/nRF52833DK/ses/NRF52833DK.emProject
* Projects/nRF52840DK/ses/NRF52840DK.emProject
* Projects/DMW3001CDK/ses/DWM3001CDK.emProject



### Step 3 : Compile and run the selected project

Connect USB to the Target J-Link port. 

Simply press the **Build and Run** button in the Segger Embedded Studio, which will compile, download and Run the project.



### Step 4 : Run the iOS Reference Application

With an Apple(R) Developer account update iPhone to iOS-15. Download and compile an example application from Apple(R) Developers Nearby Interaction web site.

The Reference Application for iOS-15 can be found on the Apple(R) developers web-site:
https://developer.apple.com/documentation/nearbyinteraction/implementing_spatial_interactions_with_third-party_accessories

General documentation of the Nearby Interaction:
https://developer.apple.com/nearby-interaction/



After deployment the application will discover the target and perform a Two-Way-Ranging over UWB with the target device running above.

Please refer to the Quick Start Guide for more details. 



## Step 5 : Connecting RTT-Viewer to the Target

Segger Embedded Studio will display the RTT output console from the Accessory. 

It is also possible to see the output from the target by connecting a J-Link RTT-Viewer to the same port of the target which was used for programming of the board.  J-Link RTT Viewer can be downloaded from the Segger web-site as a part of the J-Link package. 

Please refer to the Quick Start Guide for more details. 



## Troubleshooting Segger Embedded Studio build process

Known issues on July 21 2021

1. some host platforms (MAC-OS on M1 chip) are not yet supported by NRF Command Line Tools. 

That would prevent from executing of the "mergehex" tool to merge SoftDevice and compiled target and will cause an error during building of the project. 

In this case simple remove the post-build step from the Project's Settings. 



2. Long path length does not allow to compile of the project. Keep the path length to the project as short as possible.



Signed

July 22 2021