#ifndef __FIREBASE_H__
#define __FIREBASE_H__
#include <Firebase_ESP_Client.h>
//Define FirebaseESP8266 data object
extern FirebaseData fbdo;

void Firebase_Init(const String& streamPath);
void onFirebaseStream(FirebaseStream data);
void onFirebaseStreamTimeout(bool timeout);

#endif