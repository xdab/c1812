#ifndef RF_H
#define RF_H

/*
 * Convert Watts to dBm
 *
 * @param P Power [W]
 * 
 * @return Power [dBm]
 */
double watts_to_dBm(double P);

/*
 * Link budget
 *
 * @param P  Transmitter power [W]
 * @param Gt Transmitter system (antenna, cabling, connectors, ...) gain [dB]
 * @param Gr Receiver system gain [dB]
 * @param L  Transmission loss [dB]
 * 
 * @return Received signal strength [dBm]
 */
double link_budget(double P, double Gt, double Gr, double L);

#endif