# LOBSTER Parser

## Purpose 
Mercury validates its matching logic against real NASDAQ TotalView-ITCH data via LOBSTER's sample dataset containing message and orderbook csv format. This document covers two parsers responsible for reading that : `LobsterParser`(message file -> `LobsterMessage`) and `LobsterOrderBookParser` (orderbook snapshot file -> LobsterOrderbookRow). 

They are documented together because they are small, mirror-image components with the same job, turn one csv line into one typed struct, and they share the same design reasoning.

## Responsibilities

They are responsibile for :
- Reading one line at a time from their respective file.
- Producing a faithful, typed representation of that line's raw fields. 
- Reporting end-of-the-file via `std::optional` rather than a sentinel value or exception.

They are **not** responsible for :
- Interpreting what an event means.
- Validating field values against domain rules/invariants.
- Anything about `OrderBook`. 

## Data Model
``` cpp
enum class LobsterEventType {
    Submission=1,PartialCancellation = 2, Deletion = 3, VisibleExecution = 4, HiddenExecution = 5, TradingHalt = 7
};
struct LobsterMessage {
    std::uint64_t timestamp;
    LobsterEventType event_type;
    std::uint64_t order_id;
    std::uint64_t size;
    std::uint64_t price;
    int direction;
};
struct LobsterLevel {
    std::int64_t ask_price; std::uint64_t ask_size;
    std::int64_t bid_price; std::int64_t bid_size;
};
struct LobsterOrderbookRow {
    std::vector<LobsterLevel> levels;
};
```

Both `LobsterMessage` and `LobsterOrderBookRow` are deliberately raw-value DTOs, no `Price`/`OrderID`/`Side` domain types. The translation of raw values into validated domain types happens downstream, in `ReplayEngine`, at the point where a value is actually about to become an `Order`. This keeps the parser itself free of domain validation concerns.

## Design Decisions 

1. **Timestamp parsed as a manually-split integer, not `std::stod`.** LOBSTER's timestamp column has upto ~14 significant decimal digits at nanosecond resolution, which is right at the edge of `double`'s reliable precision, and base-10 decimals often can't be represented exactly in binary floating point. The parser splits on `.`, parses the integer and fractional parts separately as integers, and combines them into a single `std::uint64_t` nanosecond count. This avoids the risk of floating-point rounding entirely.

2. **`std::optional<T>` for end-of-the-file, not a sentinel or exception.** A missing next row is a normal, expected condition for any caller reading to completion.  `std::optional` makes it explicit in the return type and forces callers to check, rather than relying on convention (a null/negative sentinel) or paying exception overhead for something that isn't exceptional.

3. **No validation of `event_type`'s numeric value beyond an assert.** An unrecognized event type would indicate either an unsupported LOBSTER export format or a bug in the parser itself, so it's treated as an internal-invariant check (assert), consistent with the exceptions-at-boundaries / asserts-for-invariants distinction used throughout Mercury, rather than a throw.

4. **`LobsterOrderBookParser` takes `num_levels` as a constructor argument.** The orderbook file's column count varies by requested depth (L1, L5, L10, ...); rather than hardcoding a level count, the caller states it explicitly, so the same parser and the same downstream validation tooling works unchanged across every depth Mercury has been tested against.

## Complexity 
| Operation | Complexity |
| --------- | ---------- |
| next() | O(1) amortized per call |

## Known Limitations 
- No recovery from a malformed line (missing/extra fields). This throws from the underlying `stoull`/`stoi` calls rather than failing gracefully. 
- The parser assumes the file format matches the LOBSTER schema used during development. Additional columns or alternate export formats would require parser updates.

`Last updated` : 3rd Sept 2026