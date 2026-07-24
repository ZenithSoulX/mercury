# Order

## Purpose

An `Order` represents a single trading instruction submitted to the exchange.

It encapsulates the intent of a participant to buy or sell a specified quantity of an asset under a given set of constraints (price, order type, and time-in-force).

The `Order` is the fundamental domain entity of Mercury. Every operation performed by the exchange ultimately acts upon one or more `Order` objects.

---

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

---

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

---

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

The object itself is responsible for maintaining valid status transitions.

---

## Lifecycle

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

---

## Design Decisions

### Immutable Identity

Identity and trading parameters never change after construction.

If a participant wishes to modify an order, the correct workflow is:

1. Cancel the existing order.
2. Submit a new order.

This mirrors the behaviour of real electronic exchanges.

---

### Deleted Copy and Move Operations

Orders represent real trading instructions.

Creating another object with the same identity would violate the exchange model and could introduce subtle bugs.

Orders are therefore intended to be:

- constructed once,
- owned by the Exchange,
- accessed through stable pointers or references.

---

### Private Mutation

Operations that mutate execution state (`fill()` and `cancel()`) are intentionally private.

Only trusted exchange components should be allowed to modify an Order's lifecycle.

This prevents arbitrary parts of the codebase from placing an Order into an inconsistent state.

---

### Strong Types

Mercury uses strongly typed wrappers instead of primitive integers.

For example:

- OrderID
- Price
- Quantity
- SequenceNumber

This prevents accidental mixing of unrelated values while making the code more expressive.

---

### Defense-in-Depth

Although primitive domain types perform their own validation, the Order constructor performs additional assertions.

This follows a defense-in-depth philosophy where every layer verifies assumptions made about its collaborators.

---

## Ownership

Orders are owned by the Exchange (or its future memory pool).

Other components reference Orders but do not own them.

```
Exchange
    │
    ├── owns Orders
    │
    ├── OrderBook -------- references
    ├── Matching Engine -- references
    ├── Market Data ------ references
    └── Replay Engine ---- references
```

This ownership model guarantees stable identity throughout the lifetime of an Order.

---

## Complexity

| Operation | Complexity |
|------------|-----------:|
| Construction | O(1) |
| Read Access | O(1) |
| Fill | O(1) |
| Cancel | O(1) |

---

## Future Extensions

The current implementation intentionally models only the core concepts required by the exchange.

Future versions may extend Order with support for:

- Iceberg order execution
- Stop Orders
- Pegged Orders
- Hidden Liquidity
- Self Trade Prevention
- Exchange-specific order attributes

These additions should preserve the existing public interface whenever possible.

---

## Summary

The `Order` class is intentionally small.

It is a domain object—not a matching engine, not an order book, and not a trading strategy.

Its purpose is simply to represent a valid trading instruction while maintaining its own invariants throughout its lifetime.

Every other subsystem in Mercury is built upon this foundation.