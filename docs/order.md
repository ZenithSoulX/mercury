
# Order

## Purpose

An `Order` represents a single trading instruction submitted to the exchange.

The Order is the fundamental domain entity of Mercury. It encapsulates the intent of a participant to buy or sell a specified quantity of an asset under a given set of constraints (price, order type, and time-in-force).

## Responsibilities

An `Order` is responsible for:

- Representing a single trading instruction.
- Maintaining its immutable identity.
- Tracking execution progress.
- Maintaining its own lifecycle state.
- Enforcing its internal invariants.

An `Order` is **not** responsible for:

- Matching against other orders.
- Knowing about the OrderBook.
- Managing market state.
- Publishing events.
- Performing risk validation.
- Assigning sequence numbers.

Those responsibilities belong to higher-level components such as the Matching Engine, OrderBook and Exchange.

## Data Model

Every order contains:

- Unique Order ID
- Side (Buy / Sell)
- Order Type
- Time In Force
- Price
- Original Quantity
- Remaining Quantity
- Sequence Number
- Timestamp
- Current Status

Identity and trading parameters are immutable after construction.

Only the following members are allowed to change during an order's lifetime:

- Remaining Quantity
- Order Status

## Invariants

The following conditions must always hold true throughout the lifetime of an Order.

### Identity

- Every order has a unique immutable Order ID.
- Every order has exactly one timestamp.
- Every order has exactly one sequence number.

### Quantities

- Original Quantity > 0
- Remaining Quantity ≥ 0
- Remaining Quantity ≤ Original Quantity

### Pricing

For priced order types (Limit, Iceberg):
- Price > 0

Market orders are exempt from this constraint.

### Lifecycle
Every newly constructed Order begins in the **Active** state.

The object itself is responsible for maintaining valid status transitions. Invalid transitions are rejected through assertions during development builds.

## Lifecycle of Order

```
           +---------+
           | Active  |
           +---------+
             |     |
     Fill    |     | Cancel
             |     |
             v     v
+----------------+  +-----------+
| PartiallyFilled|  | Cancelled |
+----------------+  +-----------+
         |
         | Fill Remaining
         v
    +----------+
    |  Filled  |
    +----------+
```

Once an Order reaches either **Filled** or **Cancelled**, it becomes terminal and cannot transition back into an active state.

## Design Decisions

### Immutable Identity

Identity and trading parameters never change after construction.

If a participant wishes to modify an order, the correct workflow is:

1. Cancel the existing order.
2. Submit a new order.

This mirrors the behaviour of real electronic exchanges.

### Deleted Copy and Move Operations

Orders are intentionally non-copyable and non-movable.

The order book stores stable references to active orders. Allowing an order to be moved would invalidate those references and break constant-time cancellation and modification operations.

Preventing copy construction also preserves the one-object-per-order model used throughout the engine.

### Private Mutation

Operations that mutate execution state (`fill()` and `cancel()`) are intentionally private.

Only trusted exchange components should be allowed to modify an Order's lifecycle.

This prevents arbitrary parts of the codebase from placing an Order into an inconsistent state.

### Strong Types

Mercury uses strongly typed wrappers instead of primitive integers.

For example:

- OrderID
- Price
- Quantity
- SequenceNumber

This prevents accidental mixing of unrelated values while making the code more expressive.

### Defense-in-Depth
Although primitive domain types perform their own validation, the Order constructor performs additional assertions.

This follows a defense-in-depth philosophy where every layer verifies assumptions made about its collaborators.

### Quantity Reduction
Some market data feeds represent partial cancellations as reductions in resting quantity rather than explicit order replacements.

Mercury models this behavious through quantity reduction operations.

A reduction :
- decreases the remaining quantity of an active order
- preserves order identity
- preserves price-time priority
- does not constitute an execution

## Ownership

Orders are owned by the component responsible for their storage.
In the current implementation, orders are stored by the `ReplayEngine` during historical replay and referenced by the OrderBook through stable pointers.
Future versions may replace this storage with a dedicated exchange-level memory pool while preserving stable order identity.

Other components reference Orders but do not own them.

```
ReplayEngine
    │
    ├── owns Orders
    │
    ├── OrderBook -------- references
    ├── Matching Engine -- references
    ├── Market Data ------ references
    └── Replay Engine ---- references
```

### Relationship with Price Levels

Orders do not know which PriceLevel or OrderBook they belong to.

Instead, PriceLevel maintains the FIFO ordering required for price-time priority, while Order maintains only its own execution state and identity.

This separation keeps the Order abstraction small and prevents business logic from leaking into the domain object.

This ownership model guarantees stable identity throughout the lifetime of an Order.

## Complexity

| Operation | Complexity |
|------------|-----------:|
| Construction | O(1) |
| Read Access | O(1) |
| Fill | O(1)* |
| Cancel | O(1) |
*O(1) for the Order itself; check Order_book.md for full cost of a fill event including PriceLevel/PriceIndex bookkeeping.

## Future Extensions

Explicitly scoped out of the current version, with reasoning:

- **FOK (Fill-Or-Kill)**: requires an atomic pre-check across the
  opposing book before committing any fills — cannot be implemented
  as a small extension of the current matching loop without a
  dedicated all-or-nothing verification pass.
- **IOC (Immediate-Or-Cancel)**: currently behaves identically to
  GTC (unmatched remainder rests). Correct IOC behavior (discard
  remainder instead of resting) is a small, well-understood change,
  deferred for time rather than difficulty.
- **Day (time-in-force)**: requires session-boundary/clock
  infrastructure not currently modeled; also behaves as GTC today.
- **Iceberg replenishment**: requires tracking hidden reserve
  quantity separately from visible resting quantity, plus
  replenishment-triggers-new-priority logic. Not implemented;
  LOBSTER's hidden-execution events (type 5) are explicitly excluded
  from validation as a result (see ReplayEngine.md).
- Stop orders, pegged orders, self-trade prevention: out of scope,
  not yet designed.

## Summary

The `Order` class is intentionally small.

It is a domain object, not a matching engine, not an order book, and not a trading strategy.

Its purpose is simply to represent a valid trading instruction while maintaining its own invariants throughout its lifetime.

Every other subsystem in Mercury is built upon this foundation.

`Last updated` : 3rd Sept 2026 