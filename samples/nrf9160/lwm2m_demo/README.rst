.. _Coiote Device Management: https://www.avsystem.com/products/coiote-iot-device-management-platform/

.. _Coiote Device Management server: https://eu.iot.avsystem.cloud/

.. _LwM2M registry: http://openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html

.. |plusminus|  unicode:: U+000B1 .. PLUS-MINUS SIGN
   :rtrim:

.. _lwm2m_demo:

nRF9160: LwM2M Demo
#####################

.. contents::
     :local:
     :depth: 2

The LwM2M Demo demonstrates usage of the Lightweight Machine to Machine (LwM2M) 
protocol to connect a Thingy:91 or an nRF9160 DK to an LwM2M server via LTE.

.. _Requirements:

Requirements
************
The sample supports the following development kits:

.. table-from-rows:: /includes/sample_board_rows.txt
   :header: heading
   :rows: thingy91_nrf9160ns, nrf9160dk_nrf9160ns

The sample requires an activated sim card, and an LwM2M server such as 
`Leshan Demo Server`_ or `Coiote Device Management`_.

Overview
********

LwM2M is an application layer protocol based on CoAP/UDP.
It is designed to expose various resources for reading, writing and executing 
via an LwM2M server in a very lightweight environment.
The client sends data such as button and switch states, accelerometer data, 
temperature and GPS position to the LWM2M server.
It can also receive activation commands such as buzzer activation and light 
control.

The following LwM2M objects are implemented in this sample:

.. list-table:: LwM2M Objects
   :header-rows: 1

   * - Boards
     - LwM2M Server
     - Device
     - Connectivity Monitoring
     - FOTA
     - Location
     - Accelerometer
     - Colour
     - Temperature
     - Pressure
     - Humidity
     - Generic Sensor
     - Light Control
     - Push Button
     - Buzzer
     - On/Off Switch
   * - Thingy:91
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - No
   * - nRF9160 DK
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - Yes
     - No
     - Yes

.. _sensor_simulation:

Sensor simulation
=================

The sample allows choosing whether to use actual sensor measurements or 
simulated sensor data for all sensors (including the accelerometer).
If the sample is running on the nRF9160 DK, only simulated sensor data is 
available, as it does not have any of the external sensors needed for actual 
measurements.

Instructions on how to configure the sample to use the simulated sensors when 
running on the Thingy:91 can be found in the `Configuration options`_.

.. _Notifications:

Notifications 
=============

LwM2M specifies the Notify operation, which can be used to notify the server of
changes to the value of a resource field, e.g. the measured value of a 
temperature sensor. 
This allows active monitoring while using minimal bandwidth, as notifications 
can be sent with the updated values without the need for the server querying the 
client regularly.

To enable notifications, the server must initiate an observation request on the
resource(s) in question. 
See `Setup`_ for how to do this.

.. _Sensor Module:

Sensor Module
=============

The sample has a sensor module that, if enabled, reads the selected sensors
and updates the client's resource values if it detects a sufficiently large
change in one of the values. 
The threshold for what is a sufficiently large change can be configured, and can 
e.g. be a change in temperature of 1 degree Celsius.

Each sensor can be enabled separately. 
The sampling period and change threshold of that sensor can also be configured 
independently of all the other sensors.

The sensor module is intended to be used together with `Notifications`_. 
If notifications are enable for a Sensor Value resource, and the corresponding 
sensor is enabled in the sensor module, a notification will be sent only when 
that value changes significantly (as specified by the change threshold).
Thus the bandwidth usage can be significantly limited, while simultaneously
catching important changes in sensor values.

See `Setup`_ for how to enable and configure the sensor module.

.. _dtls_support:

DTLS Support
============

The sample has DTLS security enabled by default.
You need to provide the following information to the LwM2M server before you can
make a successful connection:

* Client endpoint
* Identity
* Pre-shared key

Instructions on how to register a device with a server can be found in
`Setup`_.

.. _Configuration:

Configuration
*************

|config|

.. _Setup:

Setup
=====

The following instructions describe how to register your device to 
`Leshan Demo Server`_ or `Coiote Device Management`_.

.. tabs::
   
   .. tab:: Leshan Demo Server
   
      Since Leshan does not have a No Security option, DTLS must be
      enabled if Leshan is to be used. 
      
      1. Open the Leshan Demo Server web UI.
      #. Click on "Security" in the upper-right corner.
      #. Click on "Add new client security configuration".
      #. Enter the following data and click "Create":

         * Client endpoint - nrf-{your Device IMEI}
         * Identity: - nrf-{your Device IMEI}
         * Security mode - Pre-Shared Key
         * Key - {your PSK}

   .. tab:: Coiote Device Management
      
      TODO: Coiote setup instructions.

Then, the server address and PSK must be set in the client:

1. Open :file:`src/prj.conf`. 
#. Set :option:`CONFIG_APP_LWM2M_SERVER` to the correct server URL.

   * For `Leshan Demo Server`_: ``leshan.eclipseprojects.io``
     (`public Leshan Demo Server`_).
   * For `Coiote Device Management`_: ``eu.iot.avsystem.cloud``
     (`Coiote Device Management server`_).
#. Set :option:`CONFIG_APP_PSK` to the hexadecimal representation of the
   PSK used when registering the device with the server.

The sample can now be built and run, and the client should connect to the
server. 
See `Building and running`_ for instructions.

Enabling notifications for a resource varies slightly from server to server. 
The client must be connected to the server to enable notifications. 
Below are instructions for Leshan and Coiote:

.. tabs::
   
   .. tab:: Leshan Demo Server
   
      TODO: Enable notifications on leshan

   .. tab:: Coiote Device Management
      
      1. Find your device in the Device inventory tab in the menu to the left
         and click on the blue ID in the Identity column.
      #. Click on the Objects tab in the new menu to the left, just below
         Dashboard.
      #. Find the object(s) that you want to receive notifications from, and
         expand it by clicking on it.
      #. Find the resource(s) you want to track. 

         * This can be any and all the resources of an object, but only
           resources which can be expected to change make sense to track.
         * If you want to use the `Sensor Module`_, at least the Sensor
           Value resource should be tracked for all sensors enabled in 
           the Sensor Module.
      #. Click on the Value Tracking button of the selected resource.
      #. Select Observe or Monitoring from the dropdown menu.

         * Observe will only update the Value field of the resource when it
           receives a notification.
         * Monitoring will additionally create a graph of the logged datapoints.
      #. Click on ``Limit data usage`` to configure how often notifications
         are sent.

.. _Configuration options:

Configuration options
=====================

Check and configure the following configuration options for the sample:

Server options:

.. option:: CONFIG_APP_LWM2M_SERVER - LWM2M Server configuration

   The URL of the LwM2M server to be used.

.. option:: CONFIG_APP_PSK - Pre Shared Key

   The hexadecimal representation of the PSK used when registering the device 
   with the server.

LwM2M objects options:

.. option:: CONFIG_APP_<OBJECT TYPE> - Enable an LwM2M object

   Enable an LwM2M object. 
   Objects not enabled (all compatible are by default) will not show up in the 
   server.

.. option:: CONFIG_<SENSOR TYPE>_USE_SIM - Simulate sensor data

   Sensor returns simulated data and not actual measurements. 
   Available for all sensors, including the accelerometer.

.. option:: CONFIG_LWM2M_IPSO_APP_<OBJECT TYPE>_VERSION_1_X - Select IPSO object version for app defined IPSO objects

   Select which version of the OMA IPSO object specification is to be used by
   the user defined IPSO objects.
   See the `LwM2M registry`_ for a list of objects and their available versions.
   User defined IPSO objects in this sample are:
   
   * Buzzer - ID 3338
   * Colour - ID 3335


Sensor module options:

.. option:: CONFIG_SENSOR_MODULE - Periodic sensor reading

   Enable periodic reading of sensors, and updating the resource values when
   the change is sufficiently large. 
   Notify server if the resource(s) is observed.

.. option:: CONFIG_SENSOR_MODULE_<SENSOR TYPE> - Enable sensors

   Enable this sensor in the Sensor Module.

.. option:: CONFIG_SENSOR_MODULE_<SENSOR TYPE>_PERIOD - Sensor read period

   Time in seconds between sensor readings from this sensor.

.. option:: CONFIG_SENSOR_MODULE_<SENSOR TYPE>_DELTA - Required change

   Required change in sensor value before the corresponding resource value is
   updated.

.. _Additional configuration:

Additional configuration
========================

LwM2M options:

:option:`CONFIG_LWM2M_ENGINE_MAX_OBSERVER` - Max number of resources that can be 
tracked. 
Must be increased if you want to observe more than 10 resources.

:option:`CONFIG_LWM2M_ENGINE_MAX_MESSAGES` - Max number of LwM2M message
objects. 
Needs to be increased if a large amount of notifications will be sent at once.

:option:`CONFIG_LWM2M_ENGINE_MAX_PENDING` - Max number of pending LwM2M message
objects. 
Needs to be increased if a large amount of notifications will be sent at once.

:option:`CONFIG_LWM2M_ENGINE_MAX_REPLIES` - Max number of LwM2M reply
objects. 
Needs to be increased if a large amount of notifications will be sent at once.

:option:`CONFIG_LWM2M_COAP_BLOCK_SIZE` - Increase if you want to add many new
LwM2M objects to the sample, as the registration procedure contains info about
all the LwM2M objects in one block.

:option:`CONFIG_LWM2M_ENGINE_DEFAULT_LIFETIME` - Set this to configure how often
the client sends *I'm alive* messages to the server. 

.. option:: CONFIG_LWM2M_IPSO_<OBJECT TYPE>_VERSION_1_X - Select IPSO object version

   Select which version of the OMA IPSO object specification is to be used.
   See the `LwM2M registry`_ for a list of objects and their available versions.

ADXL362 accelerometer sensor range. 
Choose one. 
Default is |plusminus| 2g. 
Resolution depends on range: |plusminus| 2g has higher resolution than 
|plusminus| 4g, which again has higher resolution than |plusminus| 8g. 
Only affects the Thingy:91.

:option:`CONFIG_ADXL362_ACCEL_RANGE_2G` - Sensor range of |plusminus| 2g.

:option:`CONFIG_ADXL362_ACCEL_RANGE_4G` - Sensor range of |plusminus| 4g.

:option:`CONFIG_ADXL362_ACCEL_RANGE_8G` - Sensor range of |plusminus| 8g.

.. _Configuration files:

Configuration files
===================

The sample provides predefined configuration files for typical use cases.

The following files are available:

* ``prj.conf`` - Standard default configuration file
* ``overlay-queue`` - Enables LwM2M Queue Mode support
* ``overlay-bootstrap.conf`` - Enables LwM2M bootstrap support
* ``overlay-thingy91.conf`` - Thingy:91 specific configuration
* ``overlay-nrf9160dk.conf`` - nRF9160 DK specific configuration

The sample can either be configured by editing the :file:`prj.conf` file and the 
relevant overlay files, or through menuconfig or guiconfig.

To open menuconfig or guiconfig:

Run ``west build -d <build directory> -b <build target> -t <menu or gui>config 
-- -DOVERLAY_CONFIG=<overlay files>``

Example:

``west build -d build_thingy91_nrf9160ns -b thingy91_nrf9160ns -t menuconfig 
-- -DOVERLAY_CONFIG=overlay-thingy91.conf overlay-bootstrap.conf``
opens menuconfig and configures the application to run on the Thingy:91 with 
LwM2M bootstrap support.

Note: the device specific config file corresponding to your build target should 
always be used, as it contains configuration options that is essential for the 
sample to run correctly on that device.

.. _Building and running:

Building and running
********************

Building the sample can either be done from a commandline, or through the
nRF Connect VS Code extension if you are using VS Code.

.. tabs::
  
   .. tab:: Commandline

      1. Run ``west build -d <build directory> -b <build target>  
         -- -DOVERLAY_CONFIG=<overlay files>`` to build the sample.

         * This step can be skipped if you have configured the application
           through menuconfig or guiconfig as shown in `Configuration files`_.
      #. Run ``west flash -d <build directory>`` to finish building (if 
         menuconfig or guiconfig was used) and to flash the device.
      #. Find your device in the server.

         * Devices are listed under Device inventory in Coiote.
         * Devices are listed under Clients in Leshan.

   .. tab:: nRF Connect for VS Code

      1. Click on the nRF Connect extension icon in the menu to the left.
      #. Click the ``Add Application`` button, or click on the 
         ``Open Welcome Page`` and click on ``Add an existing application``, 
         and select the :file:`lwm2m_demo/` folder.
      #. Add a build configuration by clicking on ``No build configurations``
         under the ``APPLICATIONS`` tab in the extension menu. 

         * Additional build configurations can be added by clicking on the icon
           next to the application name.
      #. Select the correct board from the drop-down menu. See the table in
         `Requirements`_ for build targets.
      #. Select the overlay files you want to use in the drop-down menu under
         ``Kconfig fragments``. You can select multiple overlay files by opening
         the drop-down again.
      #. Hit ``Generate Config``.
      #. To build the sample, click on the ``Build`` action under the 
         ``ACTIONS`` tab in the extension menu.
      #. Finally, click the ``Flash`` action to flash the device.
      #. Find your device in the server.

         * Coiote: devices are listed under Device inventory.
         * Leshan: devices are listed under Clients.

Queue Mode support
==================

To use the LwM2M Demo with LwM2M Queue Mode support, build it with the 
``-DOVERLAY_CONFIG=overlay-queue.conf`` option.

Bootstrap support
=================

To build the LwM2M Demo with LwM2M bootstrap support, build it with the 
``-DOVERLAY_CONFIG=overlay-bootstrap.conf`` option.

In order to successfully run the bootstrap procedure, the device must be first 
registered in the LwM2M bootstrap server.

The following instructions describe how to register your device with a bootstrap
server:

.. tabs::

   .. tab:: Leshan Demo Server

      1. Open the Leshan Demo Server bootstrap web UI.
      #. Click on ``Add new client bootstrap configuration``.
      #. Enter the client endpoint - nrf-{your device IMEI}
      #. In the ``LWM2M Bootstrap Server`` tab, enter the following data:

            * Security mode - Pre-Shared Key
            * Identity - nrf-{your device IMEI}
            * Key - 000102030405060708090a0b0c0d0e0f

      #. In the ``LWM2M Server`` section, choose the desired configuration 
         (``No security`` or ``Pre-Shared Key``). 
         If you choose ``Pre-Shared Key``, add the values for ``Identity`` and 
         ``Key`` fields (the configured Identity/Key need not match the 
         Bootstrap Server configuration).
         The same credentials will be provided in the Leshan Demo Server 
         Security configuration page.
         If ``No Security`` is chosen, no further configuration is needed.
         Note that in this mode, no DTLS will be used for the communication with 
         the LwM2M server.
      #. After adding values for the fields under both the ``LWM2M Bootstrap 
         Server`` and ``LWM2M Server`` tabs, click ``Create``.
      #. Build and run the sample.

   .. tab:: Coiote Device Management

      TODO: Register with Coiote.

Firmware Over The Air
=====================

TODO: Instructions for FOTA

Testing
=======

TODO: Write test procedure

Known issues and limitations
****************************

TODO: write issues and limitations.

Changing lifetime does not work correctly.

* Lifetime is now set in rd_client_event() in main, but anything other
  than 60 seconds causes problems. 
  > 60 seconds -> resending message and timeout. 
  < 60 seconds trouble when starting up.

Socket error when tracking many resources and sensor module enabled for many 
sensors.

* To reproduce: 

  1. Enable all sensors (including accelerometer) except light/colour in sensor 
     module.
  #. Enable value tracking for sensor value, and max/min measured for the same 
     sensors and the accelerometer.
  #. When the work_cbs are called (sensor module), the error occurs.
	  Restarts RD Client and all is fine (error MAY occur even after this, but 
     rarely).
* Errors:

  * "net_lwm2m_engine: Poll reported a socket error, 08"
  * Error: "net_lwm2m_rd_client: RD Client socket error: 5"

Dependencies
************

This application uses the following NCS libraries and drivers:

* :ref:`modem_info_readme`
* :ref:`at_cmd_parser_readme`
* :ref:`dk_buttons_and_leds_readme`
* :ref:`lte_lc_readme`
* :ref:`lib_date_time`

It uses the following ``sdk-nrfxlib`` library:

* :ref:`nrfxlib:nrf_modem`

It uses the following Zephyr libraries:

* :ref:`gpio_api`
* :ref:`pwm_api`
* :ref:`sensor_api`

In addition, it uses the following sample:

* :ref:`secure_partition_manager`
