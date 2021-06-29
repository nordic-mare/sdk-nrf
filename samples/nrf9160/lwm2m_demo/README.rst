nRF9160: LwM2M Demo
#####################

.. contents::
   :local:
   :depth: 2

The LwM2M Demo demonstrates usage of the Lightweight Machine to Machine (LwM2M) protocol to send info to an LwM2M server via LTE.
Once connected, the device can be queried to obtain GPS and sensor data, and to retrieve information about the modem.

Requirements
************
The project supports the following boards:

* nrf9160dk_nrf9160ns
* thingy91_nrf9160ns

The project also requires an LwM2M server URL address available on the internet.

Overview
********

LwM2M is an application layer protocol based on CoAP/UDP.
It is designed to expose various resources for reading, writing and executing via an LwM2M server in a very lightweight environment.
The Thingy:91 sends data such as button and switch states, accelerometer data, temperature and GPS position to the LWM2M server.
It can also receive activation commands such as buzzer activation and light control.

.. list-table::
   :align: center

   * - Button states
     - DOWN/UP
   * - Switch states
     - ON/OFF
   * - Accelerometer data
     - FLIP
   * - Temperature
     - TEMP
   * - GPS coordinates
     - GPS
   * - Buzzer
     - TRIGGER
   * - Light control
     - ON/OFF

.. _dtls_support:

DTLS Support
============

The project has DTLS security enabled by default.
You need to provide the following information to the LwM2M server before you can make a successful connection:

* Client endpoint
* Identity
* Pre-shared key

The following instructions describe how to register your device and these instructions are specific to `Leshan Demo Server`_:

1. Open the Leshan Demo Server web UI.
#. Click on "Security" in the upper-right corner.
#. Click on "Add new client security configuration".
#. Enter the following data and click "Create":

    * Client endpoint - nrf-{your Device IMEI}
    * Security mode - Pre-Shared Key
    * Identity: - nrf-{your Device IMEI}
    * Key - 000102030405060708090a0b0c0d0e0f

#. Build and run the LwM2M Demo.

Configuration
*************

Configuration options
=====================

Check and configure the following configuration options for the sample:

.. option:: CONFIG_APP_LWM2M_SERVER - LWM2M Server configuration

   This configuration specifies the LWM2M Server to be used.
   In this sample, you can set this option to ``leshan.eclipseprojects.io`` (`public Leshan Demo Server`_).

Configuration files
===================

The project provides predefined configuration files for typical use cases.

The following files are available:

* ``prj.conf`` - Standard default configuration file
* ``overlay-queue`` - Enables LwM2M Queue Mode support
* ``overlay-bootstrap.conf`` - Enables LwM2M bootstrap support

Building and Running
********************

Queue Mode support
==================

To use the LwM2M Demo with LwM2M Queue Mode support, build it with the ``-DOVERLAY_CONFIG=overlay-queue.conf`` option.

Bootstrap support
=================

To build the LwM2M Demo with LwM2M bootstrap support, build it with the ``-DOVERLAY_CONFIG=overlay-bootstrap.conf`` option.

In order to successfully run the bootstrap procedure, the device must be first registered in the LwM2M bootstrap server.

The following instructions describe how to register your device and these instructions are specific to Leshan Demo Server:

1. Open the Leshan Demo Server bootstrap web UI.
#. Click on ``Add new client bootstrap configuration``.
#. Enter the client endpoint - nrf-{your device IMEI}
#. In the ``LWM2M Bootstrap Server`` tab, enter the following data:

    * Security mode - Pre-Shared Key
    * Identity - nrf-{your device IMEI}
    * Key - 000102030405060708090a0b0c0d0e0f

#. In the ``LWM2M Server`` section, choose the desired configuration (``No security`` or ``Pre-Shared Key``).
   If you choose ``Pre-Shared Key``, add the values for ``Identity`` and ``Key`` fields (the configured Identity/Key need not match the Bootstrap Server configuration).
   The same credentials will be provided in the Leshan Demo Server Security configuration page.
   If ``No Security`` is chosen, no further configuration is needed.
   Note that in this mode, no DTLS will be used for the communication with the LwM2M server.
#. After adding values for the fields under both the ``LWM2M Bootstrap Server`` and ``LWM2M Server`` tabs, click ``Create``.
#. Build and run the LwM2M Demo.

Dependencies
************

This application uses the following NCS libraries and drivers:

* ``modem_info_readme``
* ``at_cmd_parser_readme``
* ``drivers/pwm``
* ``drivers/gpio``
* ``drivers/sensor``
* ``dk_buttons_and_leds_readme``
* ``lte_lc_readme``

It uses the following ``sdk-nrfxlib`` library:

* ``nrfxlib:nrf_modem``

In addition, it uses the following sample:

* ``secure_partition_manager``
