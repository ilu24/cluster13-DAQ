void setup() {
  Serial.begin(9600);
}

void loop() {
  while(Serial.available() > 0) {
    String in = Serial.readStringUntil('\n');
    Serial.println(in);
  }
}
