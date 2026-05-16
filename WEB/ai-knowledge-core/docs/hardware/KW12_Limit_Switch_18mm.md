# Technical Specification: KW12 Limit Switch (18mm Lever)

The KW12 limit switch is used in the Transmitter (Pedal) unit to detect the physical trigger activation. This specific version features an extended 18mm lever for improved mechanical leverage.

## Technical Drawing Details
- **Lever Length**: 18.0mm (pivot to tip).
- **Body Width**: 19.8mm (approx. based on total dimension 16.2mm between mounting points).
- **Body Height**: 9.6mm (excluding terminals).
- **Body Depth**: 6.7mm.
- **Total Height (with lever rest)**: 14.0mm.
- **Lever Width**: 4.0mm.
- **Mounting Holes**: 
  - Diameters: Φ2.2mm and Φ2.7mm.
  - Spacing between centers: 9.5mm.
  - Distance from edge to first hole: 7.3mm.
- **Terminal Layout**: 3 Pins (COM, NO, NC).
- **Electrical Ratings**: 
  - 5A 125/250V AC.
  - Contact Resistance: ≤ 50mΩ.
  - Insulation Resistance: ≥ 100MΩ.

## Application in Welder Pedal
It is connected to a digital input of the ATmega32u4 with a internal pull-up resistor. The pedal mechanism activates the switch at the very beginning of the compression stroke to:
1. Wake the system from standby/sleep.
2. Trigger the "switchClosed" boolean in the LoRa data packet.
3. Activate the safety relays in the receiver unit.
