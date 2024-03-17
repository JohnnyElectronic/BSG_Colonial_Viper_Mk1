/*!
 * @file DFPlayer.h
 * @brief DFPlayer - An Arduino MP3 Player for FN-M16P Embedded MP3 Audio Module
 * @n Header file for DFPlayer
 *
 * @version  V1.0
 * @date  2023-05-18
 */

#include "DFPlayer.h"
//#define DFP_DEBUG     /* Enables display of received data for query requests to Serial */
//#define DFP_USE_SERIAL  /* Enables the use of the physical serial port vs SoftSerial */

void dfpReset()
{
  dfpExecuteCmd(DFP_RESET,0,0);
// delay(500);
}

void dfpStop()
{
  dfpExecuteCmd(DFP_STOP,0,0);
 // delay(500);
}

void dfpPause()
{
  dfpExecuteCmd(DFP_PAUSE,0,0);
 // delay(500);
}

void dfpPlay()
{
  dfpExecuteCmd(DFP_PLAY,0,1); 
 // delay(500);
}

void dfpPlayNext()
{
  dfpExecuteCmd(DFP_PLAYNEXT,0,1);
  //delay(500);
}

void dfpPlayPrevious()
{
  dfpExecuteCmd(DFP_PLAYPREVIOUS,0,1);
  //delay(500);
}

void dfpSetVolume(int volume)
{
  dfpExecuteCmd(DFP_SETVOLUME, 0, volume); // Set the volume (0-30)
 // delay(2000);
}

void dfpUpVolume()
{
  dfpExecuteCmd(DFP_UPVOLUME, 0, 0); // Increase volume
  //delay(500);
}

void dfpDownVolume()
{
  dfpExecuteCmd(DFP_DOWNVOLUME, 0, 0); // Decrease volume
 // delay(500);
}

void dfpSetEq(int eq)
{
  dfpExecuteCmd(DFP_SETEQ, 0, eq); // Set the EQ mode (0-5, norm|pop|rock|jazz|classic|base)
 // delay(2000);
}

void dfpPlayTrack(int track)
{
  dfpExecuteCmd(DFP_PLAYTRACK, 0, track); // Play the track number (0-2999)
  //delay(2000);
}

void dfpPlayTrackMP3(int track)
{
  dfpExecuteCmd(DFP_PLAYTRACK_MP3, 0, track); // Play the track number (0-2999)
  //delay(2000);
}

// Query, Get DFP Status
// Sends a DFP_GETSTATUS command to the DFP andreturns the result
// MSB                           LSB
// 0x01 USB flash drive       | 0x00 Stopped
// 0x02 SD card               | 0x01 Playing
// 0x10 Module in sleep mode  | 0x02 Paused
// -1 command failed
int dfpGetStatus()
{
  int result = -1;
  dfpExecuteCmd(DFP_GETSTATUS, 0, 0); 
  result = dfpReadQuery(DFP_GETSTATUS);

  return result;
}

int dfpGetVolume()
{
  int result = -1;
  dfpExecuteCmd(DFP_GETVOLUME, 0, 0);
  result = dfpReadQuery(DFP_GETVOLUME) & 0xFF;

  return result;
}


void dfpSetup()
// Set up the serial stream pointer
{
    if (DFP_BUSY_PIN > 0) {
        pinMode(DFP_BUSY_PIN, INPUT);
    }

    /* Init/Reset DFP */
    dfpReset();
    delay (1000);
}


// Checks and waits while busy line is active.
// Returns 0 if timeout or 1 for busy duration completed
int dfpBusyWait()
// Read the query parameters
{
    unsigned long timer = millis();

    if (DFP_BUSY_PIN > 0) {
        /* Wait for Busy signal to activate */
        while (digitalRead(DFP_BUSY_PIN)){
          if (millis() - timer > DFP_WAIT_TIMEOUT) {
#ifdef DFP_DEBUG
  Serial.println(F("BW TIMEOUT"));
#endif
            return 0;
          }
          delay(10);
        } /* end while loop */
    
        delay (100);
    
        while (!digitalRead(DFP_BUSY_PIN)) {
          delay (50);
        }
        return 1;
    }
}
    
// Checks if serial port has data and wait if not.
// Returns 0 if timeout or 1 for data available
int dfpAvailWait()
// Read the query parameters
{
    unsigned long timer = millis();

#ifdef DFP_USE_SERIAL
    while (!Serial.available()){
      if (millis() - timer > DFP_WAIT_TIMEOUT) {
         return 0;
      }
      delay(10);
    } /* end while loop */
#else
    while (!dfpSerial.available()){
      if (millis() - timer > DFP_WAIT_TIMEOUT) {
#ifdef DFP_DEBUG
  Serial.println(F("AW TIMEOUT"));
#endif
         return 0;
      }
      delay(10);
    } /* end while loop */
#endif

    return 1;
}

// Send a command to the DFP board.
// CMD is one of the available valid commands or querys
// Par1 (MSB) and Par2 (LSB) are the command parameters. 
// A delay of DFP_CMD_DELAY is provided before exiting.
void dfpExecuteCmd(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
    // Calculate the checksum (2 bytes)
    word checksum = -(DFP_VERSION_BYTE + DFP_COMMAND_LENGTH + CMD + DFP_ACKNOWLEDGE + Par1 + Par2);
    // Build the command line
    byte Command_line[10] = { DFP_START_BYTE, DFP_VERSION_BYTE, DFP_COMMAND_LENGTH, CMD, DFP_ACKNOWLEDGE,
    Par1, Par2, highByte(checksum), lowByte(checksum), DFP_END_BYTE};
    //Send the command line to the module
    for (byte k=0; k<10; k++)
    {
#ifdef DFP_USE_SERIAL
        Serial.write( Command_line[k]);
#else
        dfpSerial.write( Command_line[k]);
#endif
    }

    delay(DFP_CMD_DELAY);
}

// Receive Query results from the DFP board. One result is processed per call.
// CMD is optional and if > 0 will compare with the returned results and validate. If the first result 
// CMD does not match a retry will occur and retrun that result.
// 
// Returns any parameters for Par1 (MSB) and Par2 (LSB). 
// If a CMD value was provided and it does not match the query cmd or no data was detected a -1 is returned
int dfpReadQuery(byte CMD)
// Read the query parameters
{
    bool dataVal = false;
    bool dataRetry = false;
    int loopIndex = 0;
    int params;
    char inByte;

    dfpAvailWait();

#ifdef DFP_USE_SERIAL
    while ((Serial.available() > 0) && (loopIndex <= 9)) {
      inByte = Serial.read();
      switch (loopIndex) {
          case DFP_DATA_START:
              if ((inByte & 0xFF) == DFP_START_BYTE) {
                  dataVal = true;
              } else {
                  /* Bad start - no match, try again */
                  continue;
              }
              break;
          case DFP_DATA_COMMAND:
              if ((CMD > 0) && (CMD != (inByte & 0xFF))) {
                  dataVal = false;
                  dataRetry = true;  // wrong command data, try again
              }
              break;
          case DFP_DATA_PARAMETER:
              params = (inByte & 0xFF) << 8;
              break;
          case DFP_DATA_PARAMETER+1:
              params = params | (inByte & 0xFF);
              break;
          case DFP_DATA_END:
              if ((inByte & 0xFF) != DFP_END_BYTE) {
                  dataVal = false;
              }

              if (dataRetry) {
                  loopIndex = -1;
              }
              break;
          default:
              break;
      } /* end switch */

      loopIndex++;
    } /* End while loop */

    if (dataVal) {
        return params;
    }
#else
#ifdef DFP_DEBUG
  Serial.print(F("Received: "));
#endif

    while ((dfpSerial.available() > 0) && (loopIndex <= 9)) {
      inByte = dfpSerial.read();

#ifdef DFP_DEBUG
  Serial.print((inByte & 0xFF), HEX);
#endif

      switch (loopIndex) {
          case DFP_DATA_START:
              if ((inByte & 0xFF) == DFP_START_BYTE) {
                  dataVal = true;
#ifdef DFP_DEBUG
  Serial.print(F("(S)"));
#endif
              } else {
                  /* Bad start - no match, try again */
#ifdef DFP_DEBUG
  Serial.println();
  Serial.print(F("Received: "));
#endif
                  continue;
              }
              break;
          case DFP_DATA_COMMAND:
              if ((CMD > 0) && (CMD != (inByte & 0xFF))) {
                  dataVal = false;
                  dataRetry = true;  // wrong command data, try again
              }
#ifdef DFP_DEBUG
  Serial.print(F("(C)"));
#endif
              break;
          case DFP_DATA_PARAMETER:
              params = (inByte & 0xFF) << 8;
#ifdef DFP_DEBUG
  Serial.print(F("(P)"));
#endif
              break;
          case DFP_DATA_PARAMETER+1:
              params = params | (inByte & 0xFF);
#ifdef DFP_DEBUG
  Serial.print(F("(P)"));
#endif
              break;
          case DFP_DATA_END:
              if ((inByte & 0xFF) != DFP_END_BYTE) {
                  dataVal = false;
              }

              if (dataRetry) {
                  loopIndex = -1;
              }
#ifdef DFP_DEBUG
  Serial.print(F("(E)"));
#endif
              break;
          default:
              break;
      } /* end switch */

#ifdef DFP_DEBUG
  Serial.print(F(" "));
#endif
      loopIndex++;
    } /* End while loop */

    if (dataVal) {
#ifdef DFP_DEBUG
  Serial.println();
#endif
        return params;
    }

#ifdef DFP_DEBUG
    if (loopIndex) {
        Serial.println(F("BAD_DATA"));
    } else {
        Serial.println(F("NO_DATA"));
    }
#endif

#endif

    return -1;
}

