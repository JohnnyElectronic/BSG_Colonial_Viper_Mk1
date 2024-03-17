/*!
 * @file DFPlayer.h
 * @brief DFPlayer - An Arduino MP3 Player for FN-M16P Embedded MP3 Audio Module
 * @n Header file for DFPlayer
 *
 * @version  V1.0
 * @date  2023-05-18
 */

#include "Arduino.h"
#include <SoftwareSerial.h>

#ifndef __DFPLAYER_H__
#define __DFPLAYER_H__

extern SoftwareSerial dfpSerial;   // Used for Softserial Mode

//#define DFP_DEBUG     /* Enables display of received data for query requests to Serial */

// Not all pins on the Mega and Mega 2560 boards support change interrupts, 
// so only the following can be used for RX: 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69). 
// 
// Not all pins on the Leonardo and Micro boards support change interrupts, so only the following can be used for RX: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

#define DFP_RX_PIN         12     /* Pin Assigned for serial RX, set to -1 if not used  */
#define DFP_TX_PIN          8     /* Pin Assigned for serial TX  */
#define DFP_BUSY_PIN        0     /* Pin Assigned for busy detection, set to 0 if not used */

#define DFP_DATA_START     0
#define DFP_DATA_VERSION   1
#define DFP_DATA_LENGTH    2
#define DFP_DATA_COMMAND   3
#define DFP_DATA_FB        4
#define DFP_DATA_PARAMETER 5      /* 5 (Param 1 - MSB)/6 (Param 2 - LSB) for 1 or 2 paramters */
#define DFP_DATA_CHECKSUM  7
#define DFP_DATA_END       9
 
#define DFP_CMD_DELAY      250    /* Fixed delay after any comand sent */
#define DFP_WAIT_TIMEOUT   3000   /* Timeout delay in MS to wait for serial available data */

/* Header Values */
#define DFP_START_BYTE     0x7E
#define DFP_VERSION_BYTE   0xFF
#define DFP_COMMAND_LENGTH 0x06
#define DFP_END_BYTE       0xEF
#define DFP_ACKNOWLEDGE    0x00   /* Returns info with command 0x41 [0x01: info, 0x00: no info] */

/* Commands */
#define DFP_RESET          0x0C
#define DFP_PLAY           0x0D
#define DFP_PAUSE          0x0E

#define DFP_REPEAT_PLAY    0x11
#define DFP_PLAYTRACK_MP3  0x12
#define DFP_STOP           0x16

#define DFP_PLAYNEXT       0x01
#define DFP_PLAYPREVIOUS   0x02
#define DFP_PLAYTRACK      0x03
#define DFP_UPVOLUME       0x04
#define DFP_DOWNVOLUME     0x05
#define DFP_SETVOLUME      0x06
#define DFP_SETEQ          0x07

/* Query Commands */
#define DFP_QUERY_DEV      0x3F
#define DFP_GETSTATUS      0x42
#define DFP_GETVOLUME      0x43

/* Query Status */
#define DFP_UDISK_FIN      0x3C  /* Provides the track number completed in the parameters */
#define DFP_TFCARD_FIN     0x3D  /* Provides the track number completed in the parameters */
#define DFP_FLASH_FIN      0x3E  /* Provides the track number completed in the parameters */


#define DFP_EQ_NORMAL   0
#define DFP_EQ_POP      1
#define DFP_EQ_ROCK     2
#define DFP_EQ_JAZZ     3
#define DFP_EQ_CLASSIC  4
#define DFP_EQ_BASS     5

void dfpReset();

void dfpStop();

void dfpPause();

void dfpPlay();

void dfpPlayNext();

void dfpPlayPrevious();

void dfpSetVolume(int volume);

void dfpUpVolume();

void dfpDownVolume();

void dfpSetEq(int eq);

void dfpPlayTrack(int track);
void dfpPlayTrackMP3(int track);

void dfpSetup();

/* Query Functions */


// Query, Get DFP Status
// Sends a DFP_GETSTATUS command to the DFP andreturns the result
// MSB                           LSB
// 0x01 USB flash drive       | 0x00 Stopped
// 0x02 SD card               | 0x01 Playing
// 0x10 Module in sleep mode  | 0x02 Paused
// -1 command failed
int dfpGetStatus();

int dfpGetVolume();

int dfpBusyWait();
int dfpAvailWait();

void dfpExecuteCmd(byte CMD, byte Par1, byte Par2);

int dfpReadQuery(byte CMD);

#endif
