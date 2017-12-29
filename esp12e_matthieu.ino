///////////////////////////////////////////////////////////////////////////////
// Project      : Matthieu
// Module       : ESP12-E firmware for Matthieu
// Language     : Arduino (C/C++)
//
// Filename     : esp12e_matthieu.ino
//
// Created by   : Lorenzo Columbo
// Created at   : 12.03.17
// Revision     : 1.0.0
//
// ++
// Description  : ESP12-E firmware for Matthieu
//                See associated header file for more information.
// ++
///////////////////////////////////////////////////////////////////////////////

#include "esp12e_matthieu.hpp"

/*
 * Definition of some global variables
 */
WiFiServer server(80);
WiFiClient client;
bool alreadyConnected = false;
typedef enum {BEG_CHAR, DCMOT_CMD, DCMOT_NUM, SERVO_CMD, SERVO_NUM, END_CHAR, EXEC_CTRL} rxStateType;
rxStateType rxState = BEG_CHAR;

/*
 * Control the rotation of the servo motor, by specifying an action between
 * init, left and right and an integer grade in the range [0, 3]; for the special
 * action "init", grade value is irrelevant; arguments (SRV_LEFT, 0) and (SRV_RIGHT, 0)
 * have the same effect.
 */
void servo_ctrl(servo_act_t servo_dir, unsigned int grade) {
    if(grade > 3)
    	grade = 3;
    switch(servo_dir) {
	case SRV_INIT:
	    pinMode(D0, OUTPUT);
	    analogWriteFreq(333);
	    analogWrite(D0, 1024*(50+0*(25/(7/2)))/100);
    	break;
	case SRV_LEFT:
	    analogWrite(D0, 1024*(50-grade*(25/(7/2)))/100);
    	break;
	case SRV_RIGHT:
	    analogWrite(D0, 1024*(50+grade*(25/(7/2)))/100);
    	break;
    }
}

/*
 * Control the movement of the DC motor, by specifying an action between
 * forward, backward and brake and an integer grade in the range [0, 9]; for the
 * special action init, grade value is irrelevant; arguments (DC_FWD, 0), (DC_BWD, 0)
 * and (DC_BRK, 0) have the same effect.
 */
void dcmot_ctrl(dcmot_act_t dcmot_act, unsigned int grade) {
	static unsigned int grade_old;
	if(grade > 9)
    	grade = 9;
    switch(dcmot_act) {
	case DC_INIT:
        // TODO: control D3 as a PWM output, in order to change DC motor speed
	    pinMode(D1, OUTPUT);
	    pinMode(D2, OUTPUT);
	    pinMode(D3, OUTPUT);
        digitalWrite(D1, LOW);
        digitalWrite(D2, LOW);
        analogWrite(D3, 0);
        break;
	case DC_FWD:
        digitalWrite(D1, LOW);
        digitalWrite(D2, HIGH);
        if((grade_old == 0) && (grade > 0)) {
            digitalWrite(D3, HIGH);
            delay(100);
        }
        analogWrite(D3, 1024*grade/9*3/4);
        break;
	case DC_BWD:
        digitalWrite(D1, HIGH);
        digitalWrite(D2, LOW);
        digitalWrite(D3, HIGH);
        if((grade_old == 0) && (grade > 0)) {
            digitalWrite(D3, HIGH);
            delay(100);
        }
        analogWrite(D3, 1024*grade/9*3/4);
        break;
	case DC_BRK:
        digitalWrite(D1, HIGH);
        digitalWrite(D2, HIGH);
        digitalWrite(D3, HIGH);
        if((grade_old == 0) && (grade > 0)) {
            digitalWrite(D3, HIGH);
            delay(100);
        }
        analogWrite(D3, 1024*grade/9*3/4);
        break;
    }
    grade_old = grade;
}

/*
 * At the bring-up, perform the following operations:
 *  1) open the serial port for printing debugging messages;
 *  2) configure the Wi-Fi Access Point;
 *  3) initialize all the I/O ports needed to control the boat equipment.
 */
void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Configure Wi-Fi Access Point with parameters:");
    Serial.print("SSID: ");
    Serial.printf("%s\n\r", SSID);
    Serial.print("PASSWORD: ");
    Serial.printf("%s\n\r", PASSWORD);
    WiFi.softAP(SSID, PASSWORD);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("IP address: ");
    Serial.println(myIP);
    server.begin();

    servo_ctrl(SRV_INIT, 0);
    dcmot_ctrl(DC_INIT, 0);

    Serial.println("Setup completed.");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

/*
 * After the bring-up, perform continuously the following operations.
 * If no client has connected yet, stay ready to receive an incoming connection;
 * when this happens, send a welcome message to it describing the available commands
 * (this is useful only when Android app is not used as the control interface).
 * If a client has already connected, poll the read character buffer: if a character
 * command is recognized, perform the associated operation on the boat equipment.
 */
void loop() {
	static String updateString;
	static char dcmotCmd;
	static char dcmotNum;
	static char servoCmd;
	static char servoNum;

	if (!alreadyConnected) {
		client = server.available();
		if (client) {
			client.flush();
			Serial.print("New client connected, with IP address:");
			Serial.println(client.remoteIP());
			client.println("--- Matthieu boat model remote control ---");
			alreadyConnected = true;
		}
	} else {
		if (!client.connected()) {
			Serial.println("");
			Serial.println("Client disconnected.");
			alreadyConnected = false;
		}
		else {
			updateString = client.readStringUntil('/');

			dcmotCmd = updateString[1];
			dcmotNum = updateString[2];
			servoCmd = updateString[3];
			servoNum = updateString[4];

			Serial.printf("%c%c%c%c\n\r", dcmotCmd, dcmotNum, servoCmd, servoNum);
			if (dcmotCmd == 'A')
				dcmot_ctrl(DC_FWD, (unsigned int) ((dcmotNum-0x30)*3));
			if (dcmotCmd == 'I')
				dcmot_ctrl(DC_BWD, (unsigned int) ((dcmotNum-0x30)*3));
			if (servoCmd == 'S')
				servo_ctrl(SRV_LEFT, ((unsigned int) (servoNum-0x30)));
			if (servoCmd == 'D')
				servo_ctrl(SRV_RIGHT, ((unsigned int) (servoNum-0x30)));
		}
	}
	delay(10);
}
