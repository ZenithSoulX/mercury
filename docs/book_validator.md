# BookValidator

## Purpose 

`BookValidator` is Mercury's validation tool for comparing reconstructed book state against LOBSTER reference snapshots.

It drives a `ReplayEngine` and a `LobsterOrderbookParser` in lockstep and checks whether Mercury's top-of-the-book state matches the corresponding LOBSTER snapshot after each processed event.

Where unit tests verify matching logic against controlled scenarios, BookValidator evaluates the engine against real historical market data.

## Responsibilities

It is responsible for :
- Advancing `ReplayEngine` and `LobsterOrderbookParser` one row at a time, together.
- Comparing `OrderBook`'s top-of-the-book (price + volume, both sides) against the corresponding orderbook-file row.
- Reporting the first mismatch with enough context (row index, both sets of values) to debug directly, or reporting a full pass.

It is **not** responsible for :
- Driving the parsing or dispatch logic itself (that's `ReplayEngine`).
- Deciding which LOBSTER event types are in scope (that's a `ReplayEngine`/documentation-level decision - see below).

## How It Works 
``` cpp
while(true){
    auto lobster_row = orderbook_parser_.next();
    bool advanced = engine_.step();
    if(!lobster_row || !advanced) break;
}
```
One `engine_.step()`(one dispatched LOBSTER event) corresponds to exactly one row of the orderbook file. This row alignment is what LOBSTER's format guarantees, and it's the entire basis for the comparison being meaningful at all.

## Current Validation Status

Mercury successfully replays full LOBSTER trading-day samples and maintains a stable, internally-consistent book state throughout the run.

However, exact top-of-the-book agreement with LOBSTER is not currently achieved.
The primary source of disagreement is that LOBSTER sample files begin with an already-populated order book. Orders resting before the sample window are present in the reference snapshots but their original submission events are absent from the message file being replayed.

As a result, later deletion and reduction events may reference orders that Mercury never observed being submitted. These events are tracked and reported explicitly by `ReplayEngine` rather than being silently ignored.

See `ReplayEngine.md` for discussion of untracked orders and event coverage.

## Design Decisions
1. **Validates unconditionally, per row as it does not know or skip by event type.** BookValidator has no visibility into which LOBSTER event type produced a given row; it only sees "book state after event N." This was a deliberate simplification : rather than threading event-type awareness through the validator, mismatches are interpreted after the fact by cross-referencing the row index against the message file. This is noted as a real, named limitation (see Future Extensions) rather than something silently worked around.

2. **Reports the first mismatch with full context, not just pass/fail.** A bare "row 4231 failed" is far less useful than the actual engine-vs-ground-truth values side by side. This is what turns a mismatch into something debuggable in minutes rather than requiring a separate investigation just to see what disagreed.

## Validation Scope 

The current validator compares:
- Best bid price
- Best bid volume
- Best ask price
- Best ask volume

It does **not** compare:
- Full depth-of-book structure
- Individual order identities
- Trade sequences
- Hidden liquidity

This keeps validation focused on externally observable market state while avoiding assumptions about liquidity that originated before the
sample window.

## Complexity 
| Operation | Complexity |
| --------- | ---------- |
| run() | O(total events), one comparison per row |

## Future Extensions 
- Have `ReplayEngine::step()` return the event type it just processed, so `BookValidator` can skip comparison on out-of-scope rows (hidden executions, halts) explicitly, rather than mismatches needing after-the-fact cross-referencing.
- Extend validation beyond top-of-book to compare deeper book levels when sufficient initialization data is available.

`Last Updated` : 3rd Sept 2026
