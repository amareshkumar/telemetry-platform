# Telemetry Platform - Complete Integration Architecture
**Day 2 Documentation - Visual Mental Model**

> **Interview Talking Points Throughout This Document:**
> - Systems design: End-to-end data flow from sensors to storage
> - Microservices architecture: Decoupled services communicating via Redis
> - Message broker pattern: Redis as central communication hub
> - Async processing: Worker pool consuming tasks from queue
> - Scalability: Independent horizontal scaling of ingestion vs processing

---

## 1. System Overview - 10,000 Foot View

```mermaid
flowchart TB
    subgraph Physical["Physical Layer (Edge)"]
        S1[Temperature Sensor]
        S2[Pressure Sensor]
        S3[Humidity Sensor]
    end
    
    subgraph TH["TelemetryHub (Ingestion Service)<br/>Port 8080<br/>9.1M ops/sec"]
        TH_DEV[Device Driver<br/>UART/I2C/SPI]
        TH_QUEUE[In-Memory Queue<br/>Bounded FIFO]
        TH_API[REST API<br/>GET /health<br/>GET /metrics]
        TH_PUBLISHER[Task Publisher<br/>Redis LPUSH]
    end
    
    subgraph Redis["Redis Message Broker<br/>Port 6379"]
        R_QUEUE[Task Queue<br/>LIST: tasks:pending]
        R_DLQ[Dead Letter Queue<br/>LIST: tasks:failed]
        R_STATE[Task Metadata<br/>HASH: task:*]
    end
    
    subgraph TTP["TelemetryTaskProcessor (Processing Service)<br/>10K tasks/sec"]
        TTP_CLIENT[Redis Client<br/>BLPOP consumer]
        TTP_DISPATCHER[Task Dispatcher<br/>Route by type]
        TTP_WORKERS[Worker Pool<br/>8 threads]
        TTP_HANDLERS[Task Handlers<br/>Telemetry/Alert/Aggregate]
    end
    
    subgraph Storage["Storage Layer"]
        DB[(PostgreSQL<br/>Time-Series Data)]
        METRICS[Prometheus<br/>Metrics & Alerts]
    end
    
    S1 & S2 & S3 -->|Raw Sensor Data| TH_DEV
    TH_DEV --> TH_QUEUE
    TH_QUEUE --> TH_API
    TH_API --> TH_PUBLISHER
    TH_API -.->|Expose Metrics| METRICS
    
    TH_PUBLISHER -->|LPUSH| R_QUEUE
    R_QUEUE -->|BLPOP| TTP_CLIENT
    TTP_CLIENT --> TTP_DISPATCHER
    TTP_DISPATCHER --> TTP_WORKERS
    TTP_WORKERS --> TTP_HANDLERS
    
    TTP_HANDLERS -->|INSERT| DB
    TTP_HANDLERS -.->|Metrics| METRICS
    TTP_WORKERS -->|Failed Tasks| R_DLQ
    
    style TH fill:#e3f2fd
    style TTP fill:#f3e5f5
    style Redis fill:#fff9c4
    style Storage fill:#e8f5e9
```

**Interview Points:**
- **Decoupling:** TelemetryHub and TelemetryTaskProcessor are completely independent (can deploy/scale separately)
- **Resilience:** Redis provides buffering - if TTP goes down, tasks accumulate in queue without data loss
- **Scalability:** Add more TTP instances to increase processing throughput (horizontal scaling)

---

## 2. Detailed Data Flow - Task Lifecycle

```mermaid
sequenceDiagram
    participant Sensor
    participant Device as TelemetryHub::Device
    participant Queue as TelemetryHub::InMemoryQueue
    participant Gateway as TelemetryHub::Gateway
    participant Redis
    participant Worker as TTP::Worker
    participant Handler as TTP::TelemetryHandler
    participant DB as PostgreSQL
    participant Metrics as Prometheus

    Note over Sensor,Metrics: Phase 1: Data Collection (TelemetryHub)
    
    Sensor->>Device: Read temperature (I2C)
    activate Device
    Device->>Device: Create TelemetrySample<br/>{device_id, timestamp, value}
    Device->>Queue: push(sample) [9.1M ops/sec]
    deactivate Device
    
    activate Queue
    Note over Queue: Lock-free bounded queue<br/>Prevents memory overflow
    Queue->>Gateway: pop() [Background thread]
    deactivate Queue
    
    activate Gateway
    Gateway->>Gateway: Convert Sample → Task JSON<br/>{id, type, priority, payload}
    Gateway->>Redis: LPUSH tasks:pending "{...}"
    Gateway->>Metrics: Increment ingestion_count
    deactivate Gateway
    
    Note over Sensor,Metrics: Phase 2: Message Broker (Redis)
    
    activate Redis
    Note over Redis: Task stored in LIST<br/>FIFO ordering preserved
    Redis-->>Worker: BLPOP tasks:pending 5 [Blocking]
    deactivate Redis
    
    Note over Sensor,Metrics: Phase 3: Async Processing (TTP)
    
    activate Worker
    Worker->>Worker: Deserialize JSON → Task object
    Worker->>Worker: Route by task.type
    Worker->>Handler: execute(task)
    activate Handler
    
    alt Telemetry Analysis
        Handler->>Handler: Parse sensor data
        Handler->>Handler: Calculate statistics
        Handler->>DB: INSERT INTO telemetry_data
    else Anomaly Detection
        Handler->>Handler: Check thresholds
        Handler->>Handler: Statistical outlier detection
        Handler->>Metrics: Alert: temperature_anomaly
    else Aggregation
        Handler->>Handler: Time-window aggregation
        Handler->>DB: UPDATE aggregated_data
    end
    
    Handler-->>Worker: TaskResult{success=true}
    deactivate Handler
    
    Worker->>Metrics: Record metrics<br/>task_duration_ms<br/>task_success_count
    
    alt Task Success
        Worker->>Redis: DEL task:{id}
        Note over Worker: Task complete, cleanup
    else Task Failure
        Worker->>Redis: RPUSH tasks:failed "{...}"
        Worker->>Redis: INCR task:{id}:retry_count
        Note over Worker: Move to dead letter queue<br/>for later retry/inspection
    end
    deactivate Worker
```

**Interview Points:**
- **At-least-once delivery:** Task stays in Redis until explicitly deleted after success
- **Error handling:** Failed tasks move to DLQ (dead letter queue) with retry count
- **Observability:** Metrics at every stage (ingestion, processing, latency)
- **Blocking pop (BLPOP):** More efficient than polling - worker sleeps until task available

---

## 3. Component Architecture - Internal Details

```mermaid
flowchart TB
    subgraph TH["TelemetryHub Internal Architecture"]
        direction TB
        
        TH_HW[Hardware Layer<br/>device/uart.cpp<br/>device/i2c.cpp<br/>device/spi.cpp]
        TH_DEVICE[Device Manager<br/>src/device_manager.cpp]
        TH_SAMPLE[Sample Generator<br/>9.1M samples/sec]
        TH_QUEUE[Thread-Safe Queue<br/>include/queue.h]
        TH_GATEWAY[Gateway Core<br/>gateway/src/gateway.cpp]
        TH_REST[REST Server<br/>cpp-httplib]
        TH_REDIS[Redis Publisher<br/>NEW: redis_client.h]
        TH_CONFIG[Config Manager<br/>common/config.cpp]
        
        TH_HW --> TH_DEVICE
        TH_DEVICE --> TH_SAMPLE
        TH_SAMPLE --> TH_QUEUE
        TH_QUEUE --> TH_GATEWAY
        TH_GATEWAY --> TH_REST
        TH_GATEWAY --> TH_REDIS
        TH_CONFIG -.-> TH_DEVICE
        TH_CONFIG -.-> TH_GATEWAY
    end
    
    subgraph Common["Shared Library (telemetry_common)"]
        direction TB
        COM_REDIS[redis_client.h/cpp<br/>275 + 312 LOC<br/>Connection Pooling]
        COM_JSON[json_utils.h/cpp<br/>Serialize/Deserialize]
        COM_UUID[uuid_generator.h<br/>Task ID generation]
        COM_CONFIG[config_parser.h<br/>INI file parsing]
    end
    
    subgraph TTP["TelemetryTaskProcessor Internal Architecture"]
        direction TB
        
        TTP_MAIN[Main Entry<br/>src/main.cpp]
        TTP_CONFIG[Config Loader<br/>Load settings.ini]
        TTP_REDIS[Redis Client<br/>common/redis_client]
        TTP_QUEUE[Task Queue<br/>src/core/TaskQueue.cpp<br/>LPUSH/BLPOP wrapper]
        TTP_DISPATCHER[Task Dispatcher<br/>src/core/Dispatcher.cpp<br/>Route by type]
        TTP_POOL[Worker Pool<br/>src/core/WorkerPool.cpp<br/>8 std::thread]
        TTP_H1[TelemetryHandler<br/>handlers/telemetry.cpp]
        TTP_H2[AlertHandler<br/>handlers/alert.cpp]
        TTP_H3[AggregateHandler<br/>handlers/aggregate.cpp]
        TTP_METRICS[Metrics Exporter<br/>Prometheus format]
        
        TTP_MAIN --> TTP_CONFIG
        TTP_MAIN --> TTP_REDIS
        TTP_MAIN --> TTP_QUEUE
        TTP_MAIN --> TTP_DISPATCHER
        TTP_MAIN --> TTP_POOL
        
        TTP_QUEUE --> TTP_REDIS
        TTP_DISPATCHER --> TTP_POOL
        TTP_POOL --> TTP_H1
        TTP_POOL --> TTP_H2
        TTP_POOL --> TTP_H3
        TTP_POOL --> TTP_METRICS
    end
    
    TH_REDIS --> Common
    TTP_QUEUE --> Common
    TH_GATEWAY --> Common
    
    style TH fill:#e3f2fd
    style TTP fill:#f3e5f5
    style Common fill:#fff3e0
```

**Interview Points:**
- **Shared library benefits:** Code reuse (redis_client, json_utils, uuid_generator) eliminates duplication
- **Monorepo advantage:** Atomic commits across services - change redis_client API, update both services in one commit
- **Independent builds:** Can build TelemetryHub without TelemetryTaskProcessor (CMake options)
- **SOLID principles:** Each component has single responsibility (device, queue, gateway, worker, handler)

---

## 4. Redis Integration Details

```mermaid
flowchart LR
    subgraph TH["TelemetryHub"]
        TH_GATEWAY[Gateway]
    end
    
    subgraph Redis["Redis Data Structures"]
        direction TB
        
        R_LIST[LIST: tasks:pending<br/>LPUSH ← BLPOP<br/>FIFO task queue]
        R_DLQ[LIST: tasks:failed<br/>Dead letter queue]
        R_HASH[HASH: task:abc123<br/>Metadata storage]
        R_SET[SET: processed:today<br/>Deduplication]
        R_ZSET[ZSET: tasks:priority<br/>Priority queue<br/>Score = priority]
        R_STRING[STRING: counter:tasks<br/>Atomic counters<br/>INCR/DECR]
    end
    
    subgraph TTP["TelemetryTaskProcessor"]
        TTP_WORKER[Worker Pool]
    end
    
    TH_GATEWAY -->|LPUSH| R_LIST
    TH_GATEWAY -->|SADD| R_SET
    TH_GATEWAY -->|ZADD| R_ZSET
    TH_GATEWAY -->|INCR| R_STRING
    
    R_LIST -->|BLPOP| TTP_WORKER
    TTP_WORKER -->|RPUSH| R_DLQ
    TTP_WORKER -->|HSET| R_HASH
    TTP_WORKER -->|SISMEMBER| R_SET
    
    style Redis fill:#fff9c4
```

**Interview Points:**
- **LIST for FIFO queue:** O(1) push/pop, supports blocking operations (BLPOP)
- **ZSET for priority:** Score-based ordering, useful for task prioritization
- **SET for deduplication:** O(1) membership check, prevents duplicate processing
- **HASH for metadata:** Store task state (retries, timestamps, errors)
- **Atomic operations:** INCR/DECR for counters (thread-safe without locks)

---

## 5. Deployment Architecture - Production

```mermaid
flowchart TB
    subgraph Edge["Factory Floor / Edge Network"]
        S1[Sensor 1]
        S2[Sensor 2]
        S3[Sensor N]
    end
    
    subgraph K8s["Kubernetes Cluster (Production)"]
        direction TB
        
        subgraph NS_Ingestion["Namespace: telemetry-ingestion"]
            direction LR
            TH1[TelemetryHub<br/>Replica 1<br/>:8080]
            TH2[TelemetryHub<br/>Replica 2<br/>:8080]
            TH3[TelemetryHub<br/>Replica N<br/>:8080]
            LB_TH[LoadBalancer<br/>Service]
            
            LB_TH --> TH1 & TH2 & TH3
        end
        
        subgraph NS_Broker["Namespace: message-broker"]
            direction TB
            R_MASTER[Redis Master<br/>:6379<br/>Persistent Volume]
            R_REPLICA1[Redis Replica 1<br/>:6379<br/>Read-only]
            R_REPLICA2[Redis Replica 2<br/>:6379<br/>Read-only]
            R_SENTINEL[Redis Sentinel<br/>Auto-failover]
            
            R_MASTER -->|Replication| R_REPLICA1
            R_MASTER -->|Replication| R_REPLICA2
            R_SENTINEL -.->|Monitor| R_MASTER
        end
        
        subgraph NS_Processing["Namespace: task-processing"]
            direction LR
            TTP1[TelemetryTaskProcessor<br/>Replica 1<br/>8 workers]
            TTP2[TelemetryTaskProcessor<br/>Replica 2<br/>8 workers]
            TTP3[TelemetryTaskProcessor<br/>Replica N<br/>8 workers]
            
            HPA[HorizontalPodAutoscaler<br/>Scale 1-10 based on<br/>CPU > 70%]
            HPA -.->|Auto-scale| TTP1 & TTP2 & TTP3
        end
        
        subgraph NS_Observability["Namespace: observability"]
            PROM[Prometheus<br/>:9090<br/>Metrics collection]
            GRAFANA[Grafana<br/>:3000<br/>Dashboards]
            ALERT[AlertManager<br/>:9093<br/>Notifications]
            
            GRAFANA --> PROM
            PROM --> ALERT
        end
    end
    
    subgraph External["External Services"]
        DB[(PostgreSQL<br/>RDS Instance<br/>Time-series data)]
        S3[S3 Bucket<br/>Long-term storage]
    end
    
    S1 & S2 & S3 -->|HTTP POST| LB_TH
    TH1 & TH2 & TH3 -->|LPUSH| R_MASTER
    R_MASTER -->|BLPOP| TTP1 & TTP2 & TTP3
    
    TTP1 & TTP2 & TTP3 -->|INSERT| DB
    TTP1 & TTP2 & TTP3 -.->|Metrics| PROM
    TH1 & TH2 & TH3 -.->|Metrics| PROM
    R_MASTER -.->|Metrics| PROM
    
    DB -->|Backup| S3
    
    style NS_Ingestion fill:#e3f2fd
    style NS_Processing fill:#f3e5f5
    style NS_Broker fill:#fff9c4
    style NS_Observability fill:#e8f5e9
```

**Interview Points:**
- **Horizontal scaling:** TelemetryHub and TTP scale independently based on load
- **Redis HA:** Master-replica setup with Sentinel for automatic failover
- **Kubernetes benefits:** Auto-scaling (HPA), health checks, rolling updates
- **Observability:** Prometheus scrapes metrics from all components
- **Persistent storage:** Redis uses PersistentVolume for task durability

---

## 6. Task Processing Flow - Worker Pool Details

```mermaid
stateDiagram-v2
    [*] --> Pending: Gateway publishes task
    
    Pending --> InFlight: Worker calls BLPOP
    
    state InFlight {
        [*] --> Deserialize
        Deserialize --> Validate
        Validate --> Route
        
        Route --> Handler1: type="telemetry.analyze"
        Route --> Handler2: type="telemetry.alert"
        Route --> Handler3: type="telemetry.aggregate"
        
        Handler1 --> Execute
        Handler2 --> Execute
        Handler3 --> Execute
        
        Execute --> Success: No errors
        Execute --> Retry: Transient error
        Execute --> Failed: Fatal error
    }
    
    Success --> Completed: DEL task from Redis
    Completed --> [*]
    
    Retry --> Pending: retry_count < 5<br/>LPUSH back to queue
    Retry --> DeadLetter: retry_count >= 5
    
    Failed --> DeadLetter: RPUSH to DLQ
    DeadLetter --> [*]
    
    note right of Pending
        LIST: tasks:pending
        Waiting for worker
    end note
    
    note right of InFlight
        Processing by worker
        Duration: 10-100ms
    end note
    
    note right of Completed
        Task successful
        Metrics recorded
    end note
    
    note right of DeadLetter
        LIST: tasks:failed
        Manual inspection needed
    end note
```

**Interview Points:**
- **State transitions:** Clear task lifecycle from pending → in-flight → completed/failed
- **Retry logic:** Exponential backoff for transient errors (network timeouts, DB unavailable)
- **Dead letter queue:** Failed tasks preserved for debugging/analysis
- **Idempotency:** Handlers designed to be idempotent (safe to retry)

---

## 7. Scaling Strategy - Performance Optimization

```mermaid
flowchart TB
    subgraph Current["Current Setup<br/>10K tasks/sec"]
        TH_C[TelemetryHub × 1<br/>9.1M ops/sec ingestion]
        R_C[Redis × 1<br/>100K ops/sec capacity]
        TTP_C[TelemetryTaskProcessor × 1<br/>8 workers]
    end
    
    subgraph Scale1["Scale Option 1: Vertical<br/>30K tasks/sec"]
        TH_V[TelemetryHub × 1<br/>Same capacity]
        R_V[Redis × 1<br/>Enable pipelining<br/>Connection pool: 20]
        TTP_V[TelemetryTaskProcessor × 1<br/>16 workers<br/>Larger instance]
    end
    
    subgraph Scale2["Scale Option 2: Horizontal<br/>100K tasks/sec"]
        TH_H[TelemetryHub × 3<br/>Load balanced]
        R_H[Redis × 1<br/>With Sentinel HA]
        TTP_H[TelemetryTaskProcessor × 10<br/>8 workers each<br/>Kubernetes HPA]
    end
    
    subgraph Scale3["Scale Option 3: Distributed<br/>500K tasks/sec"]
        TH_D[TelemetryHub × 10<br/>Multi-region]
        R_D[Redis Cluster<br/>6 nodes (3 master, 3 replica)<br/>Sharded by hash slot]
        TTP_D[TelemetryTaskProcessor × 50<br/>Event-driven async I/O<br/>libuv / ASIO]
    end
    
    Current -.->|Add CPU/RAM| Scale1
    Scale1 -.->|Add replicas| Scale2
    Scale2 -.->|Architecture change| Scale3
    
    style Current fill:#e8f5e9
    style Scale1 fill:#fff9c4
    style Scale2 fill:#ffe0b2
    style Scale3 fill:#ffccbc
```

**Interview Points:**
- **Vertical first:** Easiest to implement, 3-5x improvement
- **Horizontal next:** More complex, 10x improvement, requires load balancing
- **Distributed last:** Architecture change, 50x improvement, needs Redis Cluster
- **Bottleneck analysis:** Profile to find constraint (Redis CPU? Worker threads? Network?)

---

## 8. Interview Preparation - Architecture Q&A

### Q1: "How do you handle backpressure if TTP is slower than TelemetryHub?"

**Answer:** 
Three-layer approach:
1. **Redis queue buffering:** Tasks accumulate in Redis LIST (memory permitting)
2. **TelemetryHub bounded queue:** Prevents unlimited memory growth (drops oldest if full)
3. **Monitoring & alerting:** Prometheus alert when `redis_list_length > 10000` → trigger auto-scaling

**Follow-up:** "What if Redis runs out of memory?"
- Redis maxmemory policy: `volatile-lru` (evict least-recently-used keys with TTL)
- Critical tasks use `ZADD` with high score (priority queue) instead of `LPUSH`
- Circuit breaker: TelemetryHub stops publishing if Redis unreachable

---

### Q2: "Why Redis over Kafka for message broker?"

**Answer:**
| Feature | Redis | Kafka | Decision |
|---------|-------|-------|----------|
| Latency | Sub-ms | 10-50ms | Redis wins for real-time |
| Throughput | 100K ops/sec | 1M+ msgs/sec | Redis sufficient for 10K tasks/sec |
| Persistence | AOF + RDB | Replicated log | Both adequate |
| Complexity | Single node | ZooKeeper + brokers | Redis simpler |
| Use case | Simple queue | Event streaming | Queue fits better |

**Conclusion:** Redis is right-sized for this project. Kafka adds unnecessary complexity unless requirements grow to 100K+ tasks/sec across multiple datacenters.

---

### Q3: "How do you ensure task durability (no data loss)?"

**Answer:**
1. **Redis AOF (Append-Only File):** Every write logged to disk (fsync every 1 sec)
2. **BLPOP pattern:** Task removed from queue only after successful processing
3. **At-least-once delivery:** Task stays in Redis until worker explicitly deletes it
4. **Dead letter queue:** Failed tasks preserved for retry/inspection

**Trade-off:** AOF adds ~30% write latency but guarantees durability. For this use case (telemetry analysis), slight latency acceptable vs data loss.

---

### Q4: "Walk me through the code: Gateway publishes task to Redis"

**Answer (pseudocode):**
```cpp
// Gateway receives telemetry sample
void Gateway::handle_sample(const TelemetrySample& sample) {
    // 1. Create task JSON
    nlohmann::json task = {
        {"id", generate_uuid()},
        {"type", "telemetry.analyze"},
        {"priority", 50},
        {"payload", sample.to_json()}
    };
    
    // 2. Publish to Redis (shared library)
    redis_client_->lpush("tasks:pending", task.dump());
    
    // 3. Track for deduplication
    redis_client_->sadd("published:today", task["id"]);
    
    // 4. Metrics
    metrics_.increment("tasks_published");
}
```

**Interview points:**
- JSON serialization using nlohmann/json library
- UUID generation for task tracking
- Redis LPUSH for FIFO queue (O(1) operation)
- SET for deduplication (prevents duplicate processing)

---

## Summary - Day 2 Complete! 🎉

**Deliverables:**
1. ✅ Mock Redis client implementation (275 LOC)
2. ✅ Comprehensive unit tests (20+ test cases)
3. ✅ 8 detailed architecture diagrams (system, data flow, component, deployment)
4. ✅ Interview Q&A preparation

**Next Steps (Day 3):**
- Implement TaskQueue.cpp (wrapper around redis_client for task operations)
- Task serialization (Task struct ↔ JSON)
- Priority queue support using Redis ZSET
- Unit tests for TaskQueue

**Interview Confidence:**
You can now explain:
- End-to-end architecture (sensors → ingestion → Redis → processing → storage)
- Why Redis over Kafka/RabbitMQ
- How to scale from 10K to 100K tasks/sec
- Task durability and at-least-once delivery
- Mock testing strategy and dependency injection
