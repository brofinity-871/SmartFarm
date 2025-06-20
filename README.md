# SmartFarm
---
**ESP32 Device Document: Advanced Plant Monitoring System**

---

### 1. HARDWARE OVERVIEW

#### Sensors and Requirements

| Sensor                          | Type    | Interface  | Voltage                   | Notes                                          |
| ------------------------------- | ------- | ---------- | ------------------------- | ---------------------------------------------- |
| Capacitive Soil Moisture Sensor | Analog  | ADC (GPIO) | 3.3V                      | Needs calibration per soil type                |
| BH1750 Light Sensor             | Digital | I2C        | 3.3V                      | Requires pull-up resistors                     |
| DHT22 Temp/Humidity             | Digital | GPIO       | 3.3V                      | Slower readings (\~2s delay), timing sensitive |
| Analog pH Sensor                | Analog  | ADC (GPIO) | External Module (3.3V–5V) | Requires calibration and conditioning          |

#### Power Considerations

- ESP32 operates at 3.3V logic level.
- All sensors must either match 3.3V or use level shifting.
- Use decoupling capacitors (0.1uF) on sensor lines for stability.
- Optionally power ESP32 via battery pack with charging module or wall USB.

---

### 2. SOFTWARE FUNCTIONS (HIGH-LEVEL)

#### Core Modules

- **Sensor Reading Module**
  - Read and preprocess all sensor data.
- **Task Scheduler**
  - Schedule events in a `HH:MM`/cycle format (e.g., every 5 minutes)
- **State Machine**
  - Manage states: `IDLE`, `READING`, `SENDING`, `WAITING`, `ERROR`
- **Communication Module**
  - Handle MQTT publishing/subscription
  - Optional: HTTP POST fallback
- **Error Handling Module**
  - Restart sensor reads if invalid values persist

#### Pseudocode Structure

```cpp
setup() {
  initWiFi();
  initSensors();
  initMQTT();
  setInitialState(IDLE);
}

loop() {
  switch (currentState) {
    case IDLE:
      if (timeToRead()) currentState = READING;
      break;
    case READING:
      readAllSensors();
      currentState = SENDING;
      break;
    case SENDING:
      publishToMQTT();
      currentState = WAITING;
      break;
    case WAITING:
      waitNextCycle();
      currentState = IDLE;
      break;
    case ERROR:
      logError();
      reset();
  }
}
```

---

### 3. COMMUNICATION STANDARDS

#### Primary Protocol: **MQTT**

- **Broker**: Hosted on desktop (local network)
- **Topics**:
  - `plant/sensor_data` – publishes JSON payload
  - `plant/commands/<device_id>` – subscribes for incoming tasks
- **Payload Format**:

```json
{
  "id": "esp32_1",
  "timestamp": "2025-06-19T10:00:00Z",
  "light": 502,
  "humidity": 55.3,
  "temperature": 24.7,
  "soil_moisture": 362,
  "pH": 6.2
}
```

#### Optional Fallback: **HTTP POST**

- Endpoint: `http://<desktop_ip>:5000/data`
- Send JSON payload in body

#### Message Rules

- Must include `device id` and `timestamp`
- Send only on schedule or on state change (ex. moisture too low)
- Retry with exponential backoff if delivery fails

---

### 4. CRITICAL VARIABLES

#### Hardware State

- `int soil_moisture_raw`
- `float temperature, humidity`
- `int lux_level`
- `float ph_level`

#### Timing/Control

- `unsigned long lastReadTime`
- `const int readInterval = 300000` // 5 minutes
- `enum State { IDLE, READING, SENDING, WAITING, ERROR }`
- `State currentState`

#### Communication

- `String device_id = "esp32_1"`
- `String mqtt_broker = "192.168.1.100"`
- `bool mqtt_connected`
- `String lastPayload`

#### Error & Thresholds

- `int dryThreshold = 400`
- `float minTemperature = 18.0, maxTemperature = 30.0`
- `float acceptablePHRange[] = {5.5, 6.8}`
- `int retryCount = 0`

---

### 5. HUMAN REALM: PROCESS TO RAISE HAPPY PLANTS

This realm includes all direct human responsibilities required to ensure plant well-being and respond to monitored data.

#### Core Human Actions

| Action Category           | Description                                               | Frequency      |
| ------------------------- | --------------------------------------------------------- | -------------- |
| **Feeding**               | Apply fertilizer, compost, or soil amendments             | Weekly–Monthly |
| **Watering**              | Water based on plant type, moisture reading, and weather  | Daily–Weekly   |
| **Lighting Management**   | Place, rotate, or light plants according to species needs | Setup + Weekly |
| **Environmental Control** | Adjust temperature/humidity using airflow, misting, etc.  | Daily/Ongoing  |
| **Monitoring**            | Visually inspect leaves, measure growth, check sensors    | Daily–Weekly   |
| **Pruning & Training**    | Remove dead leaves, shape plant, support growth           | As needed      |
| **Protection**            | Defend against pests, mold, and overexposure              | As needed      |
| **Documentation**         | Log sensor data, growth, and care actions                 | Weekly         |

#### Human Outcome Goals

- Respond to sensor data with appropriate action.
- Adjust care routines seasonally or by growth stage.
- Engage with plant as a living system requiring physical and emotional attention.

---

**End of Document**

