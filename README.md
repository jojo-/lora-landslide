# lora-landslide
Landslide monitoring using lorawan

This Arduino sketch monitors a landslide using an extension meter and sends the data to The Things Network, a LoRaWAN network. The data is using the Cayenne LPP format.

## Material used:
- Arduino Leonardo
- Multi-tech MTDOT 915 LoRaWAN module
- Extension meter

## Credentials
Do not forget to update the `APP EUI` and `APP KEY` in the `join_lora()` function with your credentials.
