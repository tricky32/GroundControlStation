Ground Control Station (GCS)

A minimal Qt/QML Ground Control Station for ArduCopter (MAVLink).
Discovers a drone over UDP, shows live telemetry on a map, and provides basic controls.


Features

Auto-discover vehicle over UDP (default port 5760)

Map with pan/zoom, vehicle marker with heading, home and go-to pins

Status panel: system ID, state (disarmed/armed/flying/lost), flight mode

Telemetry: lat/lon, altitude (Relative/AMSL), ground/air speed, heading, battery %

Commands: Arm, Disarm, Takeoff, Land, Change Altitude

Flight modes: Guided, PosHold, Brake, Land, RTL

Go-to on map (click location → confirm)

Optional: Center-on-vehicle button, altitude bar

Requirements

Windows + MSVC

Qt 6 (Qt Quick/QML) and CMake

ArduPilot SITL (Copter) sending to 127.0.0.1:5760

Build and Run (Qt Creator)

Open CMakeLists.txt in Qt Creator.

Select the MSVC x64 kit, configure, and build.

Run from Qt Creator (or run the built executable from your build folder).

Start the Simulator

Launch ArduPilot SITL for Copter (e.g., run_copter.cmd).

By default, SITL outputs to UDP 127.0.0.1:5760.

Start the GCS; it listens on 5760 and should discover the vehicle automatically.



Using the Application
Map

Pan: click and drag

Zoom: mouse wheel

Center on vehicle: “Center” button (if present)

Markers

Drone: position with heading (rotating icon)

Home and go-to markers when available

Status and Telemetry

System ID, state, flight mode

Latitude/Longitude, altitude (Relative/AMSL), ground/air speed, heading, battery %



Commands

Arm / Disarm

Takeoff / Land

Change Altitude (enter target and confirm)

Flight Mode: Guided, PosHold, Brake, Land, RTL

Go-to: click on the map and confirm







How It Works (High-Level)
SITL / Vehicle  -- MAVLink over UDP -->
UdpLink (C++)
  - Owns QUdpSocket
  - Receives UDP datagrams
  - Forwards bytes to MavlinkCodec
  - Can rebind to a new UDP port

MavlinkCodec (C++)
  - Parses raw bytes into MAVLink messages
  - Emits structured telemetry and events

MultiVehicleManager (C++)
  - Tracks discovered vehicles and their state
  - Exposes data via properties/signals to QML

QML UI (Map, Panels, Buttons)
  - Binds to C++ properties/signals
  - Renders markers and telemetry
  - User actions call C++ to send MAVLink commands


Data flow: UDP datagrams → UdpLink → MavlinkCodec → MultiVehicleManager → QML UI
Command flow: QML action → C++ (build and send MAVLink) → UdpLink → vehicle (with ACK/verification as implemented).




Tests

Build the tests target (QtTest).

Tests cover basic UDP/MAVLink send/receive and link behavior.

Technical Notes

Language: C++17+

Framework: Qt 6 (Qt Quick/QML)

Build system: CMake

Compiler: MSVC

External libraries: Qt and MAVLink headers only

