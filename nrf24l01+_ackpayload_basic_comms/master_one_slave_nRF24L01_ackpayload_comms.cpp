/*************************************************************************
 * Master-unit - nRF24L01+ radio communications                          *
 *      A program to operate as a master unit that transmits to and      *
 *      receives data from just one slave device. The slave              *
 *      also has an nrf24l01+ transceiver, and replies with data         *
 *      using the Acknowledgement payload facility of the Enhanced       *
 *      ShockBurst packet structure. The TMRh20 RF24 library is used     *                                  
 *      for radio operation throughout                                   *
 *                                                                       *
 *      Author: B.D. Fraser                                              *
 *                                                                       *
 *        Last modified: 26/06/2017                                      *
 *                                                                       *
 *************************************************************************/

// include external libs for nrf24l01+ radio transceiver communications
#include <RF24.h> 
#include <SPI.h> 
#include <nRF24l01.h>
#include <printf.h>

// set Chip-Enable (CE) and Chip-Select-Not (CSN) radio setup pins
#define CE_PIN 9
#define CSN_PIN 10

// set transmission cycle send rate - in milliseconds
#define SEND_RATE 1000

// create RF24 radio object using selected CE and CSN pins
RF24 radio(CE_PIN,CSN_PIN);

// setup radio pipe addresses for each sensor node
const byte nodeAddress[5] = {'N','O','D','E','1'};

// integer to store count of successful transmissions
int masterSendCount = 0;

// simple integer array for remote node data, in the form [node_id, returned_count]
int remoteNodeData[2] = {1, 1,};

// system operation timing variables - set sendRate to limit transmit rate
unsigned long currentTime;
unsigned long lastSentTime;

/* Function: setup
 *    Initialises the system wide configuration and settings prior to start
 */
void setup()
{
  // setup serial communications for basic program display
  Serial.begin(9600);
  Serial.println("[*][*][*] Beginning nRF24L01+ master-single slave program [*][*][*]");

  // ----------------------------- RADIO SETUP CONFIGURATION AND SETTINGS -------------------------// 

  // begin radio object
  radio.begin();
  
  // set power level of the radio
  radio.setPALevel(RF24_PA_LOW);

  // set RF datarate - lowest rate for longest range capability
  radio.setDataRate(RF24_250KBPS);

  // set radio channel to use - ensure all slaves match this
  radio.setChannel(0x66);

  // set time between retries and max no. of retries
  radio.setRetries(4, 10);

  // enable ack payload - each slave replies with sensor data using this feature
  radio.enableAckPayload();

  // setup a write pipe to remote node - must match the node listening pipe
  radio.openWritingPipe(nodeAddress);

  // --------------------------------------------------------------------------------------------//
}


/* Function: loop
 *    main loop program for the master device - repeats continuously during system operation
 */
void loop()
{
    // ensure we dont collect data from slave node faster than selected rate
    currentTime = millis();
    while (currentTime - lastSentTime <= SEND_RATE) {}
    
    // collect data from slave node
    receiveNodeData();

    lastSentTime = millis();
}


/* Function: receiveNodeData
 *    Make a radio call to node and retreive preloaded ack-payload
 */
void receiveNodeData() 
{
    Serial.print("[*] Master unit has successfully sent and received data ");
    Serial.print(masterSendCount);
    Serial.println(" times.");

    Serial.println("[*] Attempting to transmit data to remote node.");
    Serial.print("[*] The master unit count being sent is: ");
    Serial.println(masterSendCount);

    // boolean to indicate if radio.write() tx was successful
    bool tx_sent;
    tx_sent = radio.write( &masterSendCount, sizeof(masterSendCount) );

    // if tx success - receive and read ack reply
    if (tx_sent) {
        if (radio.isAckPayloadAvailable()) {

            // read ack payload and copy data into remoteNodeData
            radio.read(&remoteNodeData, sizeof(remoteNodeData));

            Serial.print("[+] Successfully received data from  remote node.");
            Serial.print("  ---- The received count was: ");
            Serial.println(remoteNodeData[1]); 

            // iterate command unit count
            if (masterSendCount < 500) {
                masterSendCount++;
            } else {
              masterSendCount = 1;
            }
        }
    }
    else {
      Serial.print("[-] The transmission to the selected node failed.");
    }
    Serial.println("--------------------------------------------------------");
 }