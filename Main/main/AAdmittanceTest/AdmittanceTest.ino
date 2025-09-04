/*
 * Admittance Control Parameter Test
 * 기존 MediWalker 코드 기반으로 Admittance 계수 측정
 * SD카드 저장 기능 포함
 */

#include "src/ExoData.h"
#include "src/Motor.h"
#include "src/Controller.h"
#include "src/Joint.h"
#include "src/Logger.h"
#include "src/Time_Helper.h"
#include <SD.h>
#include <SPI.h>

// Test configuration
#define TEST_DEBUG
#define LOG_INTERVAL_MS 50      // 20Hz logging for high fidelity data
#define CONTROL_FREQUENCY 500   // 500Hz control loop
#define MAX_COMMAND_LENGTH 64   // Maximum serial command length

// Force generation modes
enum ForceMode {
    FORCE_STEP,
    FORCE_HALF_SINE,
    FORCE_MANUAL
};

// System state
struct TestSystem {
    // Core objects
    ExoData* exo_data;
    KneeJoint* test_joint;
    AdmittanceController* admittance_controller;
    
    // Timing
    unsigned long start_time;
    unsigned long last_log_time;
    unsigned long last_control_time;
    
    // Control parameters (atomic for thread safety)
    volatile float mass;
    volatile float damping; 
    volatile float stiffness;
    volatile float ref_force_magnitude;
    volatile ForceMode force_mode;
    
    // Current values
    float actual_force;
    float reference_force;
    float error_percentage;
    float current_position;
    float motor_command;
    
    // State flags
    bool running;
    bool parameters_updated;
    
    // SD logging
    File dataFile;
    String filename;
    bool sd_available;
    uint32_t log_counter;
} system;

// Test phases
enum TestPhase {
    PHASE_INIT,
    PHASE_BASELINE,      // No force applied
    PHASE_STEP_FORCE,    // Step force input
    PHASE_SINE_FORCE,    // Sine wave force
    PHASE_COMPLETE
};

TestPhase current_phase = PHASE_INIT;
unsigned long phase_start_time = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    // Initialize system structure
    memset(&system, 0, sizeof(system));
    
    Serial.println("=== Advanced Admittance Control Parameter Test ===");
    Serial.println("Professional Embedded Testing System");
    Serial.println("Based on MediWalker Framework v2.0");
    
    // Initialize subsystems
    initialize_sd_card();
    initialize_hardware_system();
    setup_default_parameters();
    create_enhanced_data_file();
    
    // Start system
    system.running = true;
    system.start_time = millis();
    
    print_command_help();
    
    Serial.println("\n🚀 System Ready - Enter commands or parameters:");
    Serial.println("Example: m0.5,c30,s50  or  force10  or  mode_sine");
}

void print_command_help() {
    Serial.println("\n📋 COMMAND INTERFACE:");
    Serial.println("• Parameter: m1.5,c25,s75  (mass, damping, stiffness)");
    Serial.println("• Force:     force15       (reference force magnitude)"); 
    Serial.println("• Mode:      mode_step     (step or half_sine)");
    Serial.println("• Control:   start/stop    (system control)");
    Serial.println("• Info:      status        (current parameters)");
    Serial.println("• Help:      help          (show this menu)");
}

void loop() {
    if (!system.running) return;
    
    unsigned long current_time = micros();
    
    // High-frequency control loop (500Hz)
    if (current_time - system.last_control_time >= (1000000 / CONTROL_FREQUENCY)) {
        execute_control_cycle();
        system.last_control_time = current_time;
    }
    
    // Medium-frequency data logging (20Hz)
    if (millis() - system.last_log_time >= LOG_INTERVAL_MS) {
        log_enhanced_data();
        system.last_log_time = millis();
    }
    
    // Process serial commands (non-blocking)
    process_serial_commands();
    
    // Update reference force generation
    update_reference_force_generation();
    
    // Safety monitoring
    monitor_system_safety();
}

void execute_control_cycle() {
    // Update admittance parameters if changed
    if (system.parameters_updated) {
        update_admittance_parameters();
        system.parameters_updated = false;
    }
    
    // Read sensors
    system.test_joint->read_data();
    system.actual_force = system.exo_data->left_side.knee.loadcell_reading;
    system.current_position = system.exo_data->left_side.knee.position;
    
    // Calculate motor command
    system.motor_command = system.admittance_controller->calc_motor_cmd();
    
    // Calculate error percentage
    calculate_error_percentage();
}

// ========== COMMAND PROCESSING ==========
void process_serial_commands() {
    if (!Serial.available()) return;
    
    static char command_buffer[MAX_COMMAND_LENGTH];
    static int buffer_index = 0;
    
    while (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (buffer_index > 0) {
                command_buffer[buffer_index] = '\0';
                execute_command(command_buffer);
                buffer_index = 0;
            }
        } else if (buffer_index < MAX_COMMAND_LENGTH - 1) {
            command_buffer[buffer_index++] = c;
        }
    }
}

void execute_command(const char* command) {
    String cmd = String(command);
    cmd.trim();
    cmd.toLowerCase();
    
    Serial.print("📝 Command: ");
    Serial.println(cmd);
    
    // Parameter updates: m1.5,c30,s50
    if (parse_admittance_parameters(cmd)) {
        Serial.println("✅ Parameters updated successfully");
        print_current_parameters();
        return;
    }
    
    // Force magnitude: force10
    if (cmd.startsWith("force")) {
        float force_value = cmd.substring(5).toFloat();
        if (force_value >= 0 && force_value <= 50) {
            system.ref_force_magnitude = force_value;
            Serial.print("✅ Reference force set to: ");
            Serial.print(force_value);
            Serial.println("N");
        } else {
            Serial.println("❌ Force must be 0-50N");
        }
        return;
    }
    
    // Force mode: mode_step or mode_sine
    if (cmd.startsWith("mode_")) {
        if (cmd.equals("mode_step")) {
            system.force_mode = FORCE_STEP;
            Serial.println("✅ Force mode: STEP");
        } else if (cmd.equals("mode_sine")) {
            system.force_mode = FORCE_HALF_SINE;
            Serial.println("✅ Force mode: HALF-SINE");
        } else {
            Serial.println("❌ Use mode_step or mode_sine");
        }
        return;
    }
    
    // System control
    if (cmd.equals("stop")) {
        system.running = false;
        Serial.println("⏹️  System stopped");
        return;
    }
    
    if (cmd.equals("start")) {
        system.running = true;
        Serial.println("▶️  System started");
        return;
    }
    
    if (cmd.equals("status")) {
        print_system_status();
        return;
    }
    
    if (cmd.equals("help")) {
        print_command_help();
        return;
    }
    
    Serial.println("❌ Unknown command. Type 'help' for commands.");
}

bool parse_admittance_parameters(const String& cmd) {
    // Parse format: m1.5,c30,s50
    int m_pos = cmd.indexOf('m');
    int c_pos = cmd.indexOf('c');
    int s_pos = cmd.indexOf('s');
    
    if (m_pos == -1 || c_pos == -1 || s_pos == -1) return false;
    
    // Extract values
    float mass = extract_parameter_value(cmd, m_pos);
    float damping = extract_parameter_value(cmd, c_pos);
    float stiffness = extract_parameter_value(cmd, s_pos);
    
    // Validate ranges
    if (mass < 0.1 || mass > 10 || 
        damping < 1 || damping > 200 || 
        stiffness < 1 || stiffness > 500) {
        Serial.println("❌ Parameters out of range:");
        Serial.println("   Mass: 0.1-10 kg, Damping: 1-200 Ns/m, Stiffness: 1-500 N/m");
        return false;
    }
    
    // Update parameters atomically
    noInterrupts();
    system.mass = mass;
    system.damping = damping;
    system.stiffness = stiffness;
    system.parameters_updated = true;
    interrupts();
    
    return true;
}

float extract_parameter_value(const String& cmd, int pos) {
    int start = pos + 1;  // Skip parameter letter
    int end = start;
    
    // Find end of number (comma or end of string)
    while (end < cmd.length() && cmd.charAt(end) != ',' && cmd.charAt(end) != ' ') {
        end++;
    }
    
    return cmd.substring(start, end).toFloat();
}

// ========== FORCE GENERATION ==========
void update_reference_force_generation() {
    static unsigned long force_start_time = 0;
    static bool force_cycle_active = false;
    
    unsigned long current_time = millis();
    
    switch (system.force_mode) {
        case FORCE_STEP:
            // Simple step function
            system.reference_force = system.ref_force_magnitude;
            break;
            
        case FORCE_HALF_SINE:
            // Half-sine wave generation
            if (!force_cycle_active) {
                force_start_time = current_time;
                force_cycle_active = true;
            }
            
            float cycle_time = (current_time - force_start_time) / 1000.0; // seconds
            float half_sine_duration = 2.0; // 2 second half-sine
            
            if (cycle_time <= half_sine_duration) {
                float phase = (cycle_time / half_sine_duration) * PI;
                system.reference_force = system.ref_force_magnitude * sin(phase);
            } else {
                // Restart cycle
                force_cycle_active = false;
                system.reference_force = 0;
            }
            break;
            
        case FORCE_MANUAL:
        default:
            // Keep current value
            break;
    }
    
    // Update ExoData
    system.exo_data->set_reference_force(config_defs::joint_id::left_knee, system.reference_force);
}

void setup_test_parameters() {
    // Set initial gait state to swing for admittance testing
    exo_data->left_side.percent_gait = 50;  // Mid swing
    exo_data->left_side.assistance_active = true;
    exo_data->left_side.expected_swing_duration = 1000; // 1 second
    
    Serial.println("Test parameters configured");
}

void start_test() {
    test_start_time = millis();
    phase_start_time = millis();
    current_phase = PHASE_BASELINE;
    test_running = true;
    
    Serial.println("Test sequence started");
}

void update_system() {
    // Read sensor data
    test_joint->read_data();
    
    // Calculate admittance control
    float motor_cmd = admittance_controller->calc_motor_cmd();
    
    // Apply motor command (for testing, we'll just store it)
    exo_data->left_side.knee.controller.setpoint = motor_cmd;
}

void update_test_phase() {
    unsigned long phase_elapsed = millis() - phase_start_time;
    
    switch (current_phase) {
        case PHASE_BASELINE:
            // No external force - measure baseline response
            exo_data->set_reference_force(config_defs::joint_id::left_knee, 0.0f);
            
            if (phase_elapsed > 5000) { // 5 seconds
                current_phase = PHASE_STEP_FORCE;
                phase_start_time = millis();
                Serial.println("Phase: Step Force Test");
            }
            break;
            
        case PHASE_STEP_FORCE:
            // Apply step force input
            exo_data->set_reference_force(config_defs::joint_id::left_knee, 10.0f); // 10N step
            
            if (phase_elapsed > 10000) { // 10 seconds
                current_phase = PHASE_SINE_FORCE;
                phase_start_time = millis();
                Serial.println("Phase: Sine Wave Force Test");
            }
            break;
            
        case PHASE_SINE_FORCE:
            // Apply sinusoidal force input
            float t = phase_elapsed / 1000.0f; // Time in seconds
            float sine_force = 5.0f * sin(2.0f * PI * 0.5f * t); // 0.5 Hz, 5N amplitude
            exo_data->set_reference_force(config_defs::joint_id::left_knee, sine_force);
            
            if (phase_elapsed > 15000) { // 15 seconds
                current_phase = PHASE_COMPLETE;
                Serial.println("Phase: Test Complete");
            }
            break;
            
        default:
            break;
    }
}

void log_test_data() {
    // Get current values
    unsigned long current_time = millis() - test_start_time;
    float ref_force = exo_data->get_reference_force(config_defs::joint_id::left_knee);
    float actual_force = exo_data->left_side.knee.loadcell_reading;
    float position = exo_data->left_side.knee.position;
    float velocity = exo_data->left_side.knee.velocity;
    float motor_cmd = exo_data->left_side.knee.controller.setpoint;
    
    // Phase name
    const char* phase_name = "";
    switch (current_phase) {
        case PHASE_BASELINE: phase_name = "Baseline"; break;
        case PHASE_STEP_FORCE: phase_name = "Step"; break;
        case PHASE_SINE_FORCE: phase_name = "Sine"; break;
        default: phase_name = "Unknown"; break;
    }
    
    // Create CSV line
    String csv_line = String(current_time) + "," + 
                      String(phase_name) + "," + 
                      String(ref_force, 3) + "," + 
                      String(actual_force, 3) + "," + 
                      String(position, 3) + "," + 
                      String(velocity, 3) + "," + 
                      String(motor_cmd, 3);
    
    // Log to Serial
    Serial.println(csv_line);
    
    // Log to SD card
    if (sd_available && dataFile) {
        dataFile.println(csv_line);
        // Flush every 10 samples for data safety
        static int flush_counter = 0;
        if (++flush_counter >= 10) {
            dataFile.flush();
            flush_counter = 0;
        }
    }
}

void complete_test() {
    test_running = false;
    
    // Close SD card file
    if (sd_available && dataFile) {
        dataFile.close();
        Serial.print("Data saved to SD card: ");
        Serial.println(filename);
    }
    
    Serial.println("\n=== Test Completed ===");
    Serial.println("Analysis Instructions:");
    Serial.println("1. Data saved to SD card (if available) and Serial output");
    Serial.println("2. Plot Reference Force vs Position response");
    Serial.println("3. Analyze step response for Mass, Damping estimation");
    Serial.println("4. Analyze frequency response for Stiffness estimation");
    Serial.println("");
    Serial.println("Recommended Parameter Tuning:");
    Serial.println("- If overshoot > 20%: Increase Damping");
    Serial.println("- If settling time > 2s: Decrease Mass or increase Damping");
    Serial.println("- If steady-state error > 5%: Adjust Stiffness");
    Serial.println("- For faster response: Decrease Mass");
    Serial.println("- For stability: Increase Damping");
    
    // Print current parameters
    Serial.println("\nCurrent Admittance Parameters:");
    Serial.print("Mass: "); Serial.print(0.5f); Serial.println(" kg");
    Serial.print("Damping: "); Serial.print(30.0f); Serial.println(" Ns/m");
    Serial.print("Stiffness: "); Serial.print(50.0f); Serial.println(" N/m");
    
    if (sd_available) {
        Serial.print("SD Card File: ");
        Serial.println(filename);
    }
    
    Serial.println("\nTest stopped. Reset to run again.");
}

// SD Card Functions
void initialize_sd_card() {
    Serial.print("Initializing SD card...");
    
    if (!SD.begin()) {
        Serial.println("SD card initialization failed!");
        Serial.println("Test will continue with Serial output only.");
        sd_available = false;
        return;
    }
    
    Serial.println("SD card initialized successfully.");
    sd_available = true;
}

void create_data_file() {
    if (!sd_available) return;
    
    // Create unique filename with timestamp
    static int file_counter = 0;
    do {
        filename = "admittance_test_" + String(file_counter++) + ".csv";
    } while (SD.exists(filename.c_str()));
    
    dataFile = SD.open(filename.c_str(), FILE_WRITE);
    
    if (dataFile) {
        // Write CSV header
        dataFile.println("Time_ms,Phase,RefForce_N,ActualForce_N,Position_deg,Velocity_degps,MotorCmd_deg");
        dataFile.flush();
        Serial.print("Data file created: ");
        Serial.println(filename);
    } else {
        Serial.println("Error creating data file!");
        sd_available = false;
    }
}

// Utility functions for testing without full system
void simulate_loadcell_reading() {
    // Simulate loadcell response based on motor position
    float position = exo_data->left_side.knee.position;
    float simulated_force = position * 0.1f + random(-100, 100) / 1000.0f; // Add noise
    exo_data->left_side.knee.loadcell_reading = simulated_force;
}

void simulate_motor_response() {
    // Simple motor response simulation
    static float current_pos = 0.0f;
    static float prev_time = 0.0f;
    
    float target_pos = exo_data->left_side.knee.controller.setpoint;
    float current_time = millis() / 1000.0f;
    float dt = current_time - prev_time;
    
    if (dt > 0.001f) {
        // Simple first-order response
        float tau = 0.1f; // Time constant
        current_pos += (target_pos - current_pos) * dt / tau;
        
        exo_data->left_side.knee.position = current_pos;
        exo_data->left_side.knee.velocity = (target_pos - current_pos) / tau;
        
        prev_time = current_time;
    }
    
    // Simulate loadcell reading based on position
    simulate_loadcell_reading();
}