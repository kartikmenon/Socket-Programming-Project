/* ========================================================================== */
/* File: AMStartup.ch
 *
 * Project name: Amazing Project
 * Component name: AMStartup Header file
 *
 * Author: Kartik Menon, Zach Kratochvil, Allison Wang
 * Date: May 2014
 *
 * Input: [AVATAR NUMBER] [DIFFICULTY] [HOST NAME]
 * Output: No official output
 *
 * Pseudocode:
 *
 */
/* ========================================================================== */

/* Check arguments */
void validate(int argc, char *argv[]);

/* ==========================================================================
 * Resolve the IP address from the hostname
 *
 * *** Content ***
 * Adapted from www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
 * ========================================================================== */
char *getIPAddress(char *hostName);
