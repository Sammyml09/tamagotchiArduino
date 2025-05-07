String petName;
int hunger = 100;
int happiness = 100;
int health = 100;
int score;
String command;
String guess;
bool playingGame = false;
unsigned long lastUpdate;
const byte SECRET_KEY = 0xB4;  // password encryption key


// ENCRYPT FUNCTION, Parse the Name, health, hunger and happiness into encryptData() and recieve strings of encrypted data
String encryptData(String name, int hng, int hlth, int hppn) {
  String encryptedName = "";
  // Process each character in the pet name, XOR with secret key and convert to 2-digit hex.
  for (int i = 0; i < name.length(); i++) {
    int enc = name[i] ^ SECRET_KEY;
    if (enc < 16) { 
      encryptedName += "0";
    }
    encryptedName += String(enc, HEX);
  }
  
  // Process numeric values in the same way (2-digit hex)
  String encryptedHunger = "";
  int encH = hng ^ SECRET_KEY;
  if (encH < 16) encryptedHunger += "0";
  encryptedHunger += String(encH, HEX);
  
  String encryptedHealth = "";
  int encHlth = hlth ^ SECRET_KEY;
  if (encHlth < 16) encryptedHealth += "0";
  encryptedHealth += String(encHlth, HEX);
  
  String encryptedHappiness = "";
  int encHppn = hppn ^ SECRET_KEY;
  if (encHppn < 16) encryptedHappiness += "0";
  encryptedHappiness += String(encHppn, HEX);
  
  // Use semicolons to separate different parts :)
  return encryptedName + ";" + encryptedHunger + ";" + encryptedHealth + ";" + encryptedHappiness;
}

// DECRYPT FUNCTION
bool loadSaveData(String encryptedInput) {
  // Find where the ';' and note the index in the inputted string
  int firstDelim = encryptedInput.indexOf(';');
  int secondDelim = encryptedInput.indexOf(';', firstDelim + 1);
  int thirdDelim = encryptedInput.indexOf(';', secondDelim + 1);
  
  // Checking to see if it's the right format
  if (firstDelim == -1 || secondDelim == -1 || thirdDelim == -1) {
    Serial.println("Invalid save code format.");
    return false;
  }
  
  // Extract each section, by finding the character between each ';' 
  String encryptedName = encryptedInput.substring(0, firstDelim);
  String encryptedHunger = encryptedInput.substring(firstDelim + 1, secondDelim);
  String encryptedHealth = encryptedInput.substring(secondDelim + 1, thirdDelim);
  String encryptedHappiness = encryptedInput.substring(thirdDelim + 1);
  
  // Decrypt the pet name 
  String decryptedName = "";
  for (int i = 0; i < encryptedName.length(); i += 2) { // += 2 because each character is two bytes
    String hexPair = encryptedName.substring(i, i + 2); // Setting hexPair to the two bytes
    int value = (int)strtol(hexPair.c_str(), NULL, 16); // hex to decimal, converting to int
    char decodedChar = (char)(value ^ SECRET_KEY); // XOR to get original ascii back
    decryptedName += decodedChar; // Add first character 
  }
  
  // Decrypt numeric value, converting hex to decimal, finding XOR with key
  int decryptedHunger = (int)(strtol(encryptedHunger.c_str(), NULL, 16) ^ SECRET_KEY);  
  int decryptedHealth = (int)(strtol(encryptedHealth.c_str(), NULL, 16) ^ SECRET_KEY);
  int decryptedHappiness = (int)(strtol(encryptedHappiness.c_str(), NULL, 16) ^ SECRET_KEY);
  
  // Apply the decrypted values
  petName = decryptedName;
  hunger = decryptedHunger;
  health = decryptedHealth;
  happiness = decryptedHappiness;
  
  Serial.println("Save file loaded successfully!");
  Serial.print("Pet Name: "); Serial.println(petName);
  Serial.print("Hunger: "); Serial.println(hunger);
  Serial.print("Health: "); Serial.println(health);
  Serial.print("Happiness: "); Serial.println(happiness);
  
  return true; 
}

void setup() {
  Serial.begin(9600);
  Serial.println("Welcome to the Virtual Pet Simulator!");
  Serial.println("Enter your pet's name or type 'password' to load a save:");
  
  while (Serial.available() == 0) { }
  
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  if (input.equalsIgnoreCase("password")) {
    Serial.println("Enter your encrypted save code:");
    while (Serial.available() == 0) { }
    String encryptedInput = Serial.readStringUntil('\n');
    encryptedInput.trim();
    
    // if the function loadSaveData() comes back true, the name and other vars have already been set, checking if it comes back false
    if (!loadSaveData(encryptedInput)) { 
      Serial.println("Starting a new game.");
      // If the save code is invalid, prompt for a new pet name.
      Serial.println("Enter your pet's name:");
      while (Serial.available() == 0) { }
      petName = Serial.readStringUntil('\n');
      petName.trim();
      Serial.print("Hello, ");
      Serial.println(petName);
    }
  } else {
    petName = input;
    Serial.print("Hello, ");
    Serial.println(petName);
  }
  
  displayVitals();
}

// Main loop
void loop() {
  command = "";

  // Current time
  unsigned long currentTime = millis();

  // Every 10 seconds reduce vitals, check health
  if (currentTime - lastUpdate > 10000) { 
    decreaseVitals();
    boostHealth();  // Call function to check and boost health
    lastUpdate = currentTime;
  }

  // Process commands
  if (Serial.available() > 0) {
    command = Serial.readStringUntil('\n'); 
  }

  // Checking command, then sending to routine,
  if (command.equalsIgnoreCase("Feed")) {
    feed();
    displayVitals();
  } else if (command.equalsIgnoreCase("Play")) {
    play();
    displayVitals();
  } else if (command.equalsIgnoreCase("Check Vitals")) {
    checkVitalsMessage();
    displayVitals();
  } else if (command.equalsIgnoreCase("save game")) {
    Serial.println("Your save code:");
    Serial.println(encryptData(petName, hunger, health, happiness));
    delay(3000);
    displayVitals();
  }

  // If pet has died, tell the user 
  if (health <= 0) {
    Serial.println("Your pet has died! Game Over!");
    while(true) {}
  }
}

void decreaseVitals() {
  // Always decrease hunger
  hunger -= random(10,15);
  if (hunger < 0) { // Makes sure doesn't go below 0
    hunger = 0; 
  }

  // Happiness now always decreases normally
  happiness -= random(5,8);
  if (happiness < 0) { // Make sure doesn't go below 0
    happiness = 0;
  }
  
  // Health decrements based on hunger and happiness
  if (hunger < 50 && happiness > 30) {
    health -= random(10,15);
  } else if (hunger < 50 && happiness < 30) { // Change rate of decrease for health, when both happiness and hunger are really low
    health -= random(15,20);
  }

  if (health < 0) { // Make sure doesn't go below 0
    health = 0;
  }
}

void boostHealth() {
  if (health > 75 && happiness > 75) { // Automatically increase health when hunger and happiness is high
    health += 10;
    if (health > 100) {
      health = 100; // Ensure health does not exceed 100
    }
  }
}


void displayVitals() {
  Serial.println("");
  Serial.println("Current Vitals:");
  Serial.print("Hunger: ");
  Serial.println(hunger);
  Serial.print("Happiness: ");
  Serial.println(happiness);
  Serial.print("Health: ");
  Serial.println(health);
  Serial.println("Commands: Feed, Play, Check Vitals, Save Game");
}

void feed() {
  hunger += 20;
  if (hunger > 100) hunger = 100; // Make sure doesn't go above 100
  Serial.println("");
  Serial.println("You fed your Pet!");
}

// Change message depending on state of hunger & happiness
void checkVitalsMessage() {
  if (hunger < 50) {
    Serial.println("");
    Serial.println("Your Pet is Hungry!");
  } else {
    Serial.println("");
    Serial.println("Your Pet is Full!");
  }

  if (happiness < 50) {
    Serial.println("");
    Serial.println("Your Pet is Unhappy!");
  } else {
    Serial.println("");
    Serial.println("Your Pet is Very Happy!");
  }

  if (hunger < 20 || happiness < 20) {
    health -= 10;
    Serial.println("");
    Serial.println("Your Pet is in danger!");
  }
}

// Print 20 lines
void clear() {
  for(int i = 0; i < 20; i++) {
    Serial.println("");
  } 
}

void play() {
  clear();
  playingGame = true;
  score = 0; // Reset score at the beginning of each play session
  
  Serial.println("Guess the direction!");
  Serial.println("Your Pet will ask for you to guess ");
  Serial.println("whether it'll go left or right. You");
  Serial.println("will type your answer, and if you're");
  Serial.println("right, you'll get points. The more ");
  Serial.println("points, the larger the happiness boost!");

  printHappyPet();

  while (playingGame) {

    if (Serial.available() > 0) {
      String userGuess = Serial.readStringUntil('\n');
      String direction = random(0, 2) == 0 ? "l" : "r"; // If random == 0, then choice is l, otherwise r

      if (userGuess.equalsIgnoreCase(direction)) {
        Serial.println("CORRECT!!");
        score++;
        printHappyPet();
      
      } else if (userGuess.equalsIgnoreCase("exit") && score > 0) {
        playingGame = false;
        
        // Add happiness boost
        int happinessBoost = score * 5;
        happiness += happinessBoost;
        if (happiness > 100) { 
          happiness = 100;
        }
        
        Serial.print("Your score was: ");
        Serial.println(score);
        Serial.print("You also gained ");
        Serial.print(happinessBoost);
        Serial.println(" happiness points!");
        
        return;
      } else if (userGuess.equalsIgnoreCase("exit")) {
        playingGame = false;
        return;
      } else {
        Serial.println("WRONG!!");
        printSadPet();
      }
    }
  }
}

// Happy Face :)
void printHappyPet() {
  Serial.println("");
  Serial.println("Guess which way? (L) or (R)? or type 'exit' ");
  Serial.println("   ^^^^^^^^^^^^^^^^^^^^^^^   ");
  Serial.println("  ^                       ^  ");
  Serial.println(" ^   <_▉_>        <_▉_>  ^ ");
  Serial.println(" ^                         ^ ");
  Serial.println(" ^^      _         _      ^^ ");
  Serial.println("  ^^      \\_______/      ^^  ");
  Serial.println("    ^^^^^^^^^^^^^^^^^^^^^    ");
  Serial.println("    ^^^               ^^^    ");
  Serial.println("    ^^^               ^^^    ");
  Serial.println("    ^^^               ^^^    ");
  Serial.println("");
}

// Sad Face :(
void printSadPet() {
  Serial.println("");
  Serial.println("Guess which way? (L) or (R)? or type 'exit' ");
  Serial.println("   ^^^^^^^^^^^^^^^^^^^^^^^   ");
  Serial.println("  ^                       ^  ");
  Serial.println(" ^   <👁>           <👁>    ^ ");
  Serial.println(" ^                         ^ ");
  Serial.println(" ^^     >___________<     ^^ ");
  Serial.println("  ^^    /           \\    ^^  ");
  Serial.println("    ^^^^^^^^^^^^^^^^^^^^^    ");
  Serial.println("    ^^^               ^^^    ");
  Serial.println("    ^^^               ^^^    ");
  Serial.println("    ^^^               ^^^    ");
  Serial.println("");
  Serial.println("   Doɴ'ᴛ ʙᴇ ᴡʀᴏɴɢ ᴀɢᴀɪɴ...   ");
  Serial.println("");
}