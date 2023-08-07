/*
   @title     StarMod
   @file      SysModNetwork.cpp
   @date      20230807
   @repo      https://github.com/ewowi/StarMod
   @Authors   https://github.com/ewowi/StarMod/commits/main
   @Copyright (c) 2023 Github StarMod Commit Authors
   @license   GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 */

#include "SysModNetwork.h"
#include "Module.h"
#include "SysModModules.h"

#include "SysModPrint.h"
#include "SysModWeb.h"
#include "SysModUI.h"
#include "SysModModel.h"

#include <WiFi.h>

//init static variables (https://www.tutorialspoint.com/cplusplus/cpp_static_members.htm)
bool SysModNetwork::forceReconnect = false;

SysModNetwork::SysModNetwork() :Module("Network") {};

//setup wifi an async webserver
void SysModNetwork::setup() {
  Module::setup();
  print->print("%s %s\n", __PRETTY_FUNCTION__, name);

  parentVar = ui->initModule(parentVar, name);
  ui->initText(parentVar, "ssid", "", false);
  ui->initPassword(parentVar, "pw", "", false, [](JsonObject var) { //uiFun
    web->addResponse(var["id"], "label", "Password");
  });
  ui->initButton(parentVar, "connect", nullptr, false, [](JsonObject var) {
    web->addResponse(var["id"], "comment", "Force reconnect (you loose current connection)");
  }, [](JsonObject var) {
    forceReconnect = true;
  });
  ui->initText(parentVar, "nwstatus", nullptr, true, [](JsonObject var) { //uiFun
    web->addResponse(var["id"], "label", "Status");
  });

  print->print("%s %s %s\n", __PRETTY_FUNCTION__, name, success?"success":"failed");
}

void SysModNetwork::loop() {
  // Module::loop();
  handleConnection();
}

void SysModNetwork::handleConnection() {
  if (lastReconnectAttempt == 0) { // do this only once
    print->print("lastReconnectAttempt == 0\n");
    initConnection();
    return;
  }

  if (forceReconnect) {
    print->print("Forcing reconnect.");
    initConnection();
    interfacesInited = false;
    forceReconnect = false;
    // wasConnected = false;
    return;
  }
  if (!(WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED)) { //!Network.interfacesInited()
    if (interfacesInited) {
      print->print("Disconnected!\n");
      interfacesInited = false;
      initConnection();
    }

    if (!apActive && millis() - lastReconnectAttempt > 12000 ) { //&& (!wasConnected || apBehavior == AP_BEHAVIOR_NO_CONN)
      print->print("Not connected AP.\n");
      initAP();
    }
  } else if (!interfacesInited) { //newly connected
    mdl->setValueP("nwstatus", "Connected %s", WiFi.localIP().toString().c_str());

    interfacesInited = true;

    Modules::newConnection = true; // send all modules connect notification

    // shut down AP
    if (apActive) { //apBehavior != AP_BEHAVIOR_ALWAYS
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      apActive = false;
      print->println(F("Access point disabled (handle)."));
    }
  }
}

void SysModNetwork::initConnection() {

  // ws.onEvent(wsEvent);

  WiFi.disconnect(true);        // close old connections

  lastReconnectAttempt = millis();

  if (!apActive) {
    print->print("Access point disabled (init).\n");
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
  }

  WiFi.setSleep(!noWifiSleep);
  WiFi.setHostname("StarMod");

  const char * ssid = mdl->getValue("ssid");
  const char * password = mdl->getValue("pw");
  if (ssid && strlen(ssid)>0) {
    char passXXX [20] = "";
    for (int i = 0; i < strlen(password); i++) strcat(passXXX, "*");
    print->print("Connecting to WiFi %s / %s\n", ssid, passXXX);
    WiFi.begin(ssid, password);
  }
  else
    print->print("No SSID");
}

void SysModNetwork::initAP() {
  print->print("Opening access point %s\n", apSSID);
  WiFi.softAPConfig(IPAddress(4, 3, 2, 1), IPAddress(4, 3, 2, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID, apPass, apChannel, apHide);
  if (!apActive) // start captive portal if AP active
  {
    mdl->setValueP("nwstatus", "AP %s / %s @ %s", apSSID, apPass, WiFi.softAPIP().toString().c_str());

    Modules::newConnection = true; // send all modules connect notification

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }

  apActive = true;
}
