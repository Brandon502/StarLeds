/*
   @title     StarMod
   @file      UserModMDNS.h
   @date      20240114
   @repo      https://github.com/ewowi/StarMod
   @Authors   https://github.com/ewowi/StarMod/commits/main
   @Copyright © 2024 Github StarMod Commit Authors
   @license   GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
   @license   For non GPL-v3 usage, commercial licenses must be purchased. Contact moonmodules@icloud.com
*/

#include <ESPmDNS.h>

class UserModMDNS:public SysModule {

public:
  String escapedMac;
  char cmDNS[64] = ""; //not 33?

  UserModMDNS() :SysModule("MDNS") {
  };

  //setup filesystem
  void setup() {
    SysModule::setup();

    escapedMac = WiFi.macAddress();
    escapedMac.replace(":", "");
    escapedMac.toLowerCase();

  }

  void loop() {
    // SysModule::loop();
  }

  void onOffChanged() {
    if (mdls->isConnected && isEnabled) {

      resetMDNS();

    } else {
      MDNS.end();
    }
  }

  void resetMDNS() {

    if (!mdls->isConnected) return;
    
    //reset cmDNS
    const char * instanceName = mdl->getValue("instanceName");
    if (strcmp(instanceName, "StarMod") == 0 )
      sprintf(cmDNS, "star-%*s", 6, escapedMac.c_str() + 6);
    else
      strcpy(cmDNS, instanceName);

    MDNS.end();
    MDNS.begin(cmDNS);

    USER_PRINTF("mDNS started %s -> %s\n", WiFi.macAddress().c_str(), cmDNS);
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("star", "tcp", 80);
    MDNS.addServiceTxt("star", "tcp", "mac", escapedMac.c_str());
  }

};

extern UserModMDNS *mdns;