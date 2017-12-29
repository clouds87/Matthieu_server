///////////////////////////////////////////////////////////////////////////////
// Project      : Matthieu
// Module       : ESP12-E firmware for Matthieu
// Language     : Arduino header (C/C++)
//
// Filename     : esp12e_matthieu.hpp
//
// Created by   : Lorenzo Columbo
// Created at   : 12.03.17
// Revision     : 1.0.0
//
// ++
// Description  : ESP12-E firmware for Matthieu
//                This application allows to open a Wi-Fi Access Point to
//                provide, through a character-based interface based on a
//                simple TCP/IP socket, the control of the DC motor and servo
//                motor of the Matthieu remotely controlled boat model.
// ++
///////////////////////////////////////////////////////////////////////////////

#ifndef ESP12E_MATTHIEU_HPP_
#define ESP12E_MATTHIEU_HPP_

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

/*
 * Definition of some application constants and types
 */
const char* SSID = "Matthieu";
const char* PASSWORD = "letssail";
typedef enum {SRV_INIT, SRV_LEFT, SRV_RIGHT} servo_act_t;
typedef enum {DC_INIT, DC_FWD, DC_BWD, DC_BRK} dcmot_act_t;

void servo_ctrl(servo_act_t servo_dir, unsigned int grade);
void dcmot_ctrl(dcmot_act_t dcmot_act, unsigned int grade);

void(* resetFunc) (void) = 0;

#endif /* ESP12E_MATTHIEU_HPP_ */
