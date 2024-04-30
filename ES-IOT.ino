#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define RST_PIN 0 //RFID reset pin
#define SS_PIN 2 //RFID sda pin
#define LED_PIN 5
#define LED_PIN_2 4

MFRC522 mfrc522(SS_PIN, RST_PIN);
ESP8266WebServer server(80);

byte authorizedUID_MILK[] = {0xB1, 0xCF, 0x52, 0xDF};
byte authorizedUID_LAYS[] = {0x31, 0xFD, 0x54, 0xDF};
byte authorizedUID_COKE[] = {0x51, 0x02, 0x03, 0x04};
byte specialRFID[] = {0xE1, 0xB2, 0xE7, 0xFE};

int milk_quantity = 0;
int lays_quantity = 0;
int coke_quantity = 0;

float milk_price = 28.00; // Price of milk
float lays_price = 20.00;  // Price of lays
float coke_price = 40.00;  // Price of coke

int flag=0;

bool checkUID(byte* authorizedUID) {
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_PIN_2, LOW);

  Serial.println("Ready to read RFID tags...");

  WiFi.begin("edge_40", "123456789");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

server.on("/", []() {
    // Calculate total cost
    float total_cost = (milk_quantity * milk_price) + (lays_quantity * lays_price) + (coke_quantity * coke_price);
    String buttonLabel = flag == 0 ? "Add" : "Remove"; // Dynamically determine the button label based on the flag
    String modeText = flag == 0 ? "Add Mode" : "Remove Mode"; // Text indicating the current mode

    server.send(200, "text/html",
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "  <meta charset=\"UTF-8\">\n"
        "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "  <title>Shopping Cart</title>\n"
        "  <style>\n"
        "    body {\n"
        "      font-family: Arial, sans-serif;\n"
        "      margin: 0;\n"
        "      padding: 0;\n"
        "      background-image: url('https://www.veeforu.com/wp-content/uploads/2022/10/Pink-color-gradient-aesthetic-background.-1024x576.png');\n"
        "      background-size: cover;\n"
        "      background-repeat: no-repeat;\n"
        "      color: #333;\n"
        "    }\n"
        "\n"
        "    .container {\n"
        "      max-width: 800px;\n"
        "      margin: 0 auto;\n"
        "      padding: 20px;\n"
        "    }\n"
        "\n"
        "    h1 {\n"
        "      text-align: center;\n"
        "      color: #fff;\n"
        "      margin-bottom: 30px;\n"
        "    }\n"
        "\n"
        "    .mode {\n"
        "      text-align: center;\n"
        "      font-size: 20px;\n"
        "      margin-bottom: 20px;\n"
        "    }\n"
        "\n"
        "    table {\n"
        "      width: 100%;\n"
        "      border-collapse: collapse;\n"
        "      border-spacing: 0;\n"
        "      margin-bottom: 20px;\n"
        "    }\n"
        "\n"
        "    th, td {\n"
        "      padding: 10px;\n"
        "      text-align: left;\n"
        "    }\n"
        "\n"
        "    th {\n"
        "      background-color: #f2f2f2;\n"
        "    }\n"
        "\n"
        "    button {\n"
        "      padding: 8px 20px;\n"
        "      border: none;\n"
        "      background-color: #4CAF50;\n"
        "      color: #fff;\n"
        "      cursor: pointer;\n"
        "      font-size: 16px;\n"
        "    }\n"
        "\n"
        "    button:hover {\n"
        "      background-color: #45a049;\n"
        "    }\n"
        "\n"
        "    .total-cost {\n"
        "      text-align: right;\n"
        "      font-size: 18px;\n"
        "      font-weight: bold;\n"
        "    }\n"
        "  </style>\n"
        "</head>\n"
        "<body>\n"
        "  <div class=\"container\">\n"
        "    <h1>Shopping Cart</h1>\n"
        "    <div class=\"mode\">Current Mode: <span id=\"mode\">" + modeText + "</span></div>\n" // Display current mode
        "    <table>\n"
        "      <thead>\n"
        "        <tr>\n"
        "          <th>Item Name</th>\n"
        "          <th>Quantity</th>\n"
        "          <th>Price</th>\n"
        "          <th></th>\n"
        "        </tr>\n"
        "      </thead>\n"
        "      <tbody>\n"
        "        <tr>\n"
        "          <td>Lays</td>\n"
        "          <td id=\"lays-quantity\">" + String(lays_quantity) + "</td>\n"
        "          <td id=\"lays-price\">" + String(lays_price, 2) + "</td>\n"
        "          <td><button onclick=\"updateQuantity('lays')\">" + buttonLabel + "</button></td>\n" // Dynamically insert the button label
        "        </tr>\n"
        "        <tr>\n"
        "          <td>Milk</td>\n"
        "          <td id=\"milk-quantity\">" + String(milk_quantity) + "</td>\n"
        "          <td id=\"milk-price\">" + String(milk_price, 2) + "</td>\n"
        "          <td><button onclick=\"updateQuantity('milk')\">" + buttonLabel + "</button></td>\n" // Dynamically insert the button label
        "        </tr>\n"
        "        <tr>\n"
        "          <td>Coke</td>\n"
        "          <td id=\"coke-quantity\">" + String(coke_quantity) + "</td>\n"
        "          <td id=\"coke-price\">" + String(coke_price, 2) + "</td>\n"
        "          <td><button onclick=\"updateQuantity('coke')\">" + buttonLabel + "</button></td>\n" // Dynamically insert the button label
        "        </tr>\n"
        "      </tbody>\n"
        "    </table>\n"
        "    <div class=\"total-cost\">Total Cost: ₹<span id=\"total-cost\">" + String(total_cost, 2) + "</span></div>\n"
        "  </div>\n"
        "  <script>\n"
        "    function updateQuantity(itemName) {\n"
        "      // Get the button element\n"
        "      var button = document.getElementById(itemName + '-button');\n"
        "      // Determine action based on flag\n"
        "      var action = " + (flag == 0 ? "1" : "-1") + ";\n"
        "      // Update button label\n"
        "      button.innerHTML = '" + buttonLabel + "';\n"
        "      // Logic to update quantity on webpage (e.g., increment or decrement displayed value)\n"
        "      // (Optional) Send signal to server to update internal quantity variable\n"
        "      // (Optional) Update total cost if implemented\n"
        "    }\n"
        "  </script>\n"
        "<script>"
        "setTimeout(function(){ window.location.reload(true); }, 3000);"
        "</script>"
        "</body>\n"
        "</html>");
});




  server.begin();
}


void loop() {
  server.handleClient();

  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (checkUID(authorizedUID_MILK)) {
      if (flag == 0) {
        milk_quantity++;
        Serial.println("MILK ADDED");
      } else {
        if(milk_quantity > 0) {
          milk_quantity--;
          Serial.println("MILK REMOVED");
        } else {
          Serial.println("No MILK to remove");
        }
      }
      digitalWrite(LED_PIN, HIGH);
      delay(1000); // 1 second delay
      digitalWrite(LED_PIN, LOW);
    } else if (checkUID(authorizedUID_LAYS)) {
      if (flag == 0) {
        lays_quantity++;
        Serial.println("LAYS ADDED");
      } else {
        if(lays_quantity > 0) {
          lays_quantity--;
          Serial.println("LAYS REMOVED");
        } else {
          Serial.println("No LAYS to remove");
        }
      }
      digitalWrite(LED_PIN, HIGH);
      delay(1000); // 1 second delay
      digitalWrite(LED_PIN, LOW);
    } else if (checkUID(authorizedUID_COKE)) {
      if (flag == 0) {
        coke_quantity++;
        Serial.println("COKE ADDED");
      } else {
        if(coke_quantity > 0) {
          coke_quantity--;
          Serial.println("COKE REMOVED");
        } else {
          Serial.println("No COKE to remove");
        }
      }
      digitalWrite(LED_PIN, HIGH);
      delay(1000); // 1 second delay
      digitalWrite(LED_PIN, LOW);
    } else {
      // Unauthorized card
      Serial.println("Unauthorized card");
    }

    if (checkUID(specialRFID)) {
      flag = 1 - flag; // Toggle between 0 and 1
      Serial.print("Flag changed to: ");
      Serial.println(flag);
    }
    if(flag==1){
      digitalWrite(LED_PIN_2, HIGH);
    } else {
      digitalWrite(LED_PIN_2, LOW);
    }

    // Halt the PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
  }
}
