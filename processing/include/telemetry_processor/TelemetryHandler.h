#pragma once

#include "telemetry_processor/Task.h"
#include <memory>
#include <functional>

namespace telemetry_processor {

/**
 * @brief Telemetry-specific task handler for TelemetryTaskProcessor
 * 
 * Processes tasks published by TelemetryHub Gateway via Redis.
 * Supports various telemetry processing operations:
 * - telemetry.analyze: Statistical analysis
 * - telemetry.anomaly_detect: Threshold-based anomaly detection
 * - telemetry.aggregate: Time-based aggregation
 * - telemetry.store: Persist to database
 * - telemetry.alert: Trigger alerts
 * 
 * Integration with TelemetryHub:
 * 
 *   TelemetryHub → Redis → TelemetryTaskProcessor
 *                            ↓
 *                      TelemetryHandler
 *                            ↓
 *                      [Analysis/Storage/Alerts]
 * 
 * Usage:
 *   auto handler = std::make_shared<TelemetryHandler>(db_config);
 *   worker_pool.register_handler("telemetry.analyze", handler);
 *   worker_pool.register_handler("telemetry.anomaly_detect", handler);
 */
class TelemetryHandler {
public:
    /**
     * @brief Processing result
     */
    struct ProcessResult {
        bool success = false;
        std::string message;
        double processing_time_ms = 0.0;
        std::map<std::string, double> metrics;  // Additional metrics
    };
    
    /**
     * @brief Configuration for telemetry processing
     */
    struct Config {
        // Database connection
        std::string db_host = "localhost";
        int db_port = 5432;
        std::string db_name = "telemetry";
        std::string db_user = "telemetry_user";
        std::string db_password = "";
        
        // Anomaly detection thresholds
        double temp_high_threshold = 80.0;     // °C
        double temp_low_threshold = -20.0;     // °C
        double humidity_high_threshold = 95.0; // %
        double voltage_low_threshold = 2.8;    // V
        double current_high_threshold = 2.0;   // A
        
        // Aggregation settings
        int aggregation_window_sec = 60;       // 1 minute windows
        
        // Alert settings
        std::string alert_webhook_url = "";
        bool enable_email_alerts = false;
        std::string alert_email = "";
    };
    
    /**
     * @brief Constructor
     * @param config Handler configuration
     */
    explicit TelemetryHandler(const Config& config = Config{});
    
    /**
     * @brief Destructor
     */
    ~TelemetryHandler();
    
    /**
     * @brief Process a telemetry task
     * 
     * Dispatches to appropriate handler based on task type:
     * - "telemetry.analyze" → handle_analyze()
     * - "telemetry.anomaly_detect" → handle_anomaly_detect()
     * - "telemetry.aggregate" → handle_aggregate()
     * - "telemetry.store" → handle_store()
     * - "telemetry.alert" → handle_alert()
     * 
     * @param task Task to process
     * @return ProcessResult with success status and metrics
     */
    ProcessResult process(const Task& task);
    
    /**
     * @brief Statistical analysis on telemetry data
     * 
     * Computes:
     * - Mean, median, std deviation
     * - Min/max values
     * - Rate of change
     * - Fourier transform (frequency analysis)
     * 
     * @param task Task with telemetry payload
     * @return Analysis results
     */
    ProcessResult handle_analyze(const Task& task);
    
    /**
     * @brief Anomaly detection using thresholds and statistical methods
     * 
     * Detects:
     * - Out-of-range values (threshold-based)
     * - Sudden spikes (gradient-based)
     * - Flatlines (no variance)
     * - Missing expected patterns
     * 
     * @param task Task with telemetry payload
     * @return Anomaly detection results (triggers alert if anomaly found)
     */
    ProcessResult handle_anomaly_detect(const Task& task);
    
    /**
     * @brief Time-based aggregation
     * 
     * Aggregates telemetry over time windows:
     * - 1-minute averages
     * - 5-minute rolling windows
     * - Hourly summaries
     * 
     * Reduces storage requirements while preserving trends.
     * 
     * @param task Task with telemetry payload
     * @return Aggregated data
     */
    ProcessResult handle_aggregate(const Task& task);
    
    /**
     * @brief Store telemetry data to PostgreSQL
     * 
     * Inserts into time-series table:
     *   CREATE TABLE telemetry_data (
     *     id SERIAL PRIMARY KEY,
     *     device_id VARCHAR(64) NOT NULL,
     *     timestamp TIMESTAMPTZ NOT NULL,
     *     temperature DOUBLE PRECISION,
     *     humidity DOUBLE PRECISION,
     *     pressure DOUBLE PRECISION,
     *     voltage DOUBLE PRECISION,
     *     current DOUBLE PRECISION,
     *     raw_data JSONB
     *   );
     * 
     * Uses batch inserts for performance.
     * 
     * @param task Task with telemetry payload
     * @return Storage result
     */
    ProcessResult handle_store(const Task& task);
    
    /**
     * @brief Trigger alerts based on conditions
     * 
     * Alert channels:
     * - Webhook (HTTP POST to configured endpoint)
     * - Email (SMTP)
     * - Log file
     * 
     * Alert payload includes:
     * - Device ID
     * - Timestamp
     * - Alert reason
     * - Telemetry values
     * - Severity level
     * 
     * @param task Task with alert conditions
     * @return Alert delivery result
     */
    ProcessResult handle_alert(const Task& task);
    
    /**
     * @brief Get handler statistics
     */
    struct Stats {
        size_t tasks_processed = 0;
        size_t tasks_failed = 0;
        size_t anomalies_detected = 0;
        size_t alerts_sent = 0;
        double avg_processing_time_ms = 0.0;
        std::map<std::string, size_t> task_type_counts;
    };
    
    Stats get_stats() const;
    
    /**
     * @brief Reset statistics
     */
    void reset_stats();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    
    // Helper: Parse telemetry payload from JSON
    struct TelemetryPayload {
        std::string device_id;
        std::string timestamp;
        double temperature = 0.0;
        double humidity = 0.0;
        double pressure = 0.0;
        double voltage = 0.0;
        double current = 0.0;
        nlohmann::json raw_data;
    };
    
    TelemetryPayload parse_payload(const std::string& json_payload);
    
    // Helper: Check if value exceeds thresholds
    bool is_anomaly(const TelemetryPayload& payload);
    
    // Helper: Send webhook alert
    bool send_webhook_alert(const std::string& message, const TelemetryPayload& payload);
    
    // Helper: Database connection management
    bool ensure_db_connected();
};

} // namespace telemetry_processor
