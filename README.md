# Multithreaded Resource Scheduler

A concurrent C++17 simulation where 22 threads compete for a shared energy pool and a
set of pooled tools (5 hammers, 3 saws). Three kinds of worker acquire resources under
contention, an agent thread replenishes energy, and an auditor thread checks an
energy-conservation invariant and watches for starvation.

## Workers

| Type     | Count | Needs                  |
|----------|-------|------------------------|
| Gatherer | 10    | 1 energy               |
| Builder  | 6     | 2 energy + 1 hammer    |
| Guardian | 4     | 3 energy + 1 hammer + 1 saw |

A worker must get all its resources before working, and releases anything it already
holds if an acquisition fails. Deadlock is avoided by always taking a hammer before a
saw, using `try_lock_for` timeouts so no thread waits forever, and rolling back energy
on failure.

## Build & run

```bash
chmod +x run.sh
./run.sh --seed 42 --duration 5
```

A clean run reports `Integrity: PASS`.
