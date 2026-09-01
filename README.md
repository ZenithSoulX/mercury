# Mercury

**A price-time-priority limit order book matching engine, validated against real NASDAQ data.**

Mercury is a price-time-priority limit order book and matching engine built in C++20 and validated against real NASDAQ TotalView-ITCH order flow (via LOBSTER sample dataset). 

Rather than evaluating correctness only through synthetic test cases, Mercury replays historical market events and compares its reconstructed book state against LOBSTER's independently-generated ground truth. The project emphasizes low-latency data structure, explicit memory ownership, strong type safety, and measurable performance. 

[Architecture]

## Performance Snapshot (Apple Silicon)

### AAPL 2013 L1 dataset

| Operation | p50 | p90 | p99 |
|-----------|-----|-----|-----|
| Submit | 500 ns | 1.58 µs | 7.33 µs |
| Cancel | 42 ns | 84 ns | 125 ns |
| Reduce | 125 ns | 167 ns | 167 ns |

### AAPL 2012 L5 dataset

| Operation | p50 | p90 | p99 |
|-----------|-----|-----|-----|
| Submit | 500 ns | 1.58 µs | 7.08 µs |
| Cancel | 42 ns | 84 ns | 125 ns |
| Reduce | 125 ns | 125 ns | 167 ns |

## Why this project

Most "limit order book" projects stop at correctness against self-written test cases. Mercury goes further in three specific ways:

1. **Replay on real market data :** Mercury processes historical NASDAQ TotalView-ITCH events from the LOBSTER dataset, allowing correctness checks and performance measurements on real order flow rather than synthetic scenarios (Read `data/README.md` for more information regarding validation).
2. **Measured, not assumed :** Every performance claim in this README is backed by a real, reproducible measurement (`tools/latency_report`), not a theoretical complexity argument.
3. **Documented with reasoning, not just description.** Every non-trivial design decision like why a sorted vector instead of `std::map`, why two distinct numeric types (`Quantity` vs `Volume`), why copy/move are deleted on `Order`, etc are written down with the trade-off considered and why they were resolved the way they were. See `docs/`.

## Architecture 

Mercury is organsied as a strict layered hierarchy - each layer knows only what it needs to, nothing more :

```text
LOBSTER Data Files
(message + orderbook)

        |
        v

LobsterParser / LobsterOrderbookParser
(raw CSV -> typed events)

        |
        v

ReplayEngine
- assigns sequence numbers
- owns order memory
- tracks unsupported event types
- records latency metrics

        |
        v

OrderBook
- price-time-priority matching

├── BookSide (bids)
│   └── PriceIndex
│
└── BookSide (asks)
    └── PriceIndex

PriceIndex
- sorted Price -> PriceLevel mapping
- binary-search lookup

PriceLevel
- FIFO queue of resting orders

Order
- immutable identity
- mutable execution state
```

## Key Features

- Price-time-priority matching
- O(1) order cancellation and reduction via intrusive linked orders
- Strong-type domain model (OrderID, Price, Quantity, Volume, Timestamp, etc.)
- Replay engine for historical NASDAQ order flow
- Benchmarking tools for latency measurement
- Unit-tested matching logic and replay infrastructure