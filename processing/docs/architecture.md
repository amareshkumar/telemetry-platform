# TelemetryTaskProcessor Architecture

## Overview

DistQueue is a distributed task queue system built in C++17 with Redis as the persistence layer. It follows a producer-consumer pattern with exactly-once delivery semantics.

## Core Components

### 1. Task

The fundamental unit of work in DistQueue.

**Properties:**
- `id`: Unique identifier (UUID v4)
- `type`: Task type (e.g., "compute", "io", "notify")
- `payload`: JSON-serialized task data
- `priority`: HIGH, NORMAL, or LOW
- `status`: PENDING, RUNNING, COMPLETED, FAILED, CANCELLED
- `retry_count`: Current retry attempt
- `max_retries`: Maximum allowed retries
- `created_at`: Creation timestamp
- `updated_at`: Last modification timestamp
- `worker_id`: ID of worker processing the task

**Operations:**
- `to_json()`: Serialize to JSON
- `from_json()`: Deserialize from JSON
- `create()`: Factory method for new tasks

### 2. Producer

Submits tasks to the queue.

**Responsibilities:**
- Create tasks with appropriate metadata
- Serialize tasks to JSON
- Push tasks to Redis queue (RPUSH)
- Track submission metrics

**Queue Structure:**
```
distqueue:tasks:pending    - Pending tasks (priority sorted)
distqueue:tasks:running    - Currently executing tasks
distqueue:tasks:completed  - Successfully completed tasks
distqueue:tasks:failed     - Failed tasks (after all retries)
```

### 3. Worker

Processes tasks from the queue.

**Responsibilities:**
- Poll queue for tasks (BLPOP)
- Execute task logic
- Update task status
- Handle failures and retries
- Report metrics

**Workflow:**
1. BLPOP from `distqueue:tasks:pending`
2. Move task to `distqueue:tasks:running`
3. Execute task logic
4. On success: Move to `distqueue:tasks:completed`
5. On failure: Retry or move to `distqueue:tasks:failed`

### 4. RedisClient

Wrapper around Redis operations.

**Day 1:** In-memory mock implementation
**Day 2+:** Real Redis integration with redis-plus-plus

**Operations:**
- `rpush(key, value)`: Push to list
- `blpop(key, timeout)`: Blocking pop from list
- `set(key, value)`: Set key-value
- `get(key)`: Get value
- `del(key)`: Delete key
- `llen(key)`: List length

## Data Flow

```
┌──────────┐
│ Producer │
│  (C++)   │
└────┬─────┘
     │ create task
     │ serialize JSON
     │ RPUSH to queue
     ▼
┌─────────────────┐
│ Redis Queue     │
│ (pending list)  │
└────┬────────────┘
     │ BLPOP
     ▼
┌──────────┐
│  Worker  │
│  (C++)   │
└────┬─────┘
     │ execute
     │ update status
     ▼
┌─────────────────┐
│ Result Storage  │
│ (Redis)         │
└─────────────────┘
```

## Exactly-Once Semantics

To ensure tasks are executed exactly once:

1. **Atomic Move**: Use Redis transactions (MULTI/EXEC) or Lua scripts
   ```lua
   -- Atomic pop from pending, push to running
   local task = redis.call('LPOP', KEYS[1])
   if task then
       redis.call('RPUSH', KEYS[2], task)
   end
   return task
   ```

2. **Worker Heartbeat**: Workers periodically update heartbeat timestamp
   - If heartbeat expires, task is returned to queue

3. **Task Deduplication**: Use task ID for idempotency
   - Check if task already completed before execution

## Error Handling

### Task Failures

1. **Temporary Failure**: Increment retry_count, requeue if < max_retries
2. **Permanent Failure**: Move to failed queue after max retries
3. **Worker Crash**: Heartbeat timeout → return task to pending

### Redis Failures

1. **Connection Lost**: Retry with exponential backoff
2. **Command Failed**: Log error, alert monitoring
3. **Persistence Lost**: Tasks in memory may be lost (future: use Redis persistence)

## Metrics (Day 8+)

**Key Metrics:**
- Tasks submitted (counter)
- Tasks completed (counter)
- Tasks failed (counter)
- Queue depth (gauge)
- Task latency p50/p95/p99 (histogram)
- Worker utilization (gauge)

**Export to Prometheus:**
```
TELEMETRY_PROCESSOR_tasks_submitted_total{type="compute"}
TELEMETRY_PROCESSOR_tasks_completed_total{type="compute"}
TELEMETRY_PROCESSOR_tasks_failed_total{type="compute"}
TELEMETRY_PROCESSOR_queue_depth{priority="high"}
TELEMETRY_PROCESSOR_task_latency_seconds{quantile="0.99"}
```

## Scalability

### Horizontal Scaling

- **Multiple Workers**: Each worker polls the same queue
- **Load Balancing**: Redis BLPOP provides natural load distribution
- **No Worker Coordination**: Workers are stateless

### Vertical Scaling

- **Thread Pool**: Each worker runs multiple threads
- **Batch Processing**: Process multiple tasks per transaction

### Future Enhancements

- Priority queues (separate queues by priority)
- Sharding (distribute tasks across multiple Redis instances)
- Rate limiting (control task submission rate)
- Scheduled tasks (delayed execution)

## Day-by-Day Evolution

**Day 1**: Task struct, Mock Redis, JSON serialization  
**Day 2**: Producer API, real Redis integration  
**Day 3**: Worker implementation  
**Day 4**: Exactly-once semantics  
**Day 5**: Error handling, retries  
**Day 6**: Python client library  
**Day 7**: Integration testing  
**Day 8**: Prometheus metrics  
**Day 9**: Multi-worker coordination  
**Day 10**: Docker Compose deployment  
**Day 11**: Load testing  
**Day 12**: Documentation polish  

---

This architecture balances simplicity (Redis as single dependency) with production concerns (exactly-once, metrics, error handling). It demonstrates distributed systems thinking without over-engineering.
