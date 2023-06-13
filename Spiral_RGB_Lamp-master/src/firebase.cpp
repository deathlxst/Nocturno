#include "firebase.h"

#define FIREBASE_HOST "https://nocturno-lamp-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "BXTsEkJUEJlObEtlJZsSsEvoFgoWjF1DIA5vQ30m"

FirebaseData fbdo;
FirebaseConfig fbConfig;
FirebaseData fbdoStream;

void Firebase_Init(const String& streamPath)
{
  FirebaseAuth fbAuth;
  fbConfig.host = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);

#if defined(ESP8266)
  fbdo.setBSSLBufferSize(2*1024, 1024);
#endif

  fbdo.setResponseSize(1024);
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "small");

  while (!Firebase.ready())
  {
    Serial.println("Connecting to Firebase...");
    delay(1000);
  }

  String path = streamPath;
  if (Firebase.RTDB.beginStream(&fbdoStream, path.c_str()))
  {
    Serial.printf("Firebase stream on %s \n",  path);
    Firebase.RTDB.setStreamCallback(&fbdoStream, onFirebaseStream, onFirebaseStreamTimeout);
  }
  else
    Serial.printf("Firebase stream failed: %s", fbdoStream.errorReason());
}


void onFirebaseStreamTimeout(bool timeout)
{
  // Handle Firebase stream timeout if needed
}
