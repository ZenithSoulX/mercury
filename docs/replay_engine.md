# ReplayEngine

## Purpose 
`ReplayEngine` drives an `OrderBook` from a real historical market data feed (specifically, LOBSTER's NASDAQ TotalView-ITCH message format). 
It parses one event at a time, translates each event into the corresponding `OrderBook` operation, and provides a reproducible way to exercise the matching engine on real historical order flow rather than only synthetic test cases. 

Where `OrderBook` answers "does my matching logic behave correctly",`ReplayEngine` answers "does it behave correctly on real market data". It also makes it clear that which parts of that real data fall outside the current implementation's scope.

## Responsibilities
`ReplayEngine` is responsible for :
- Owning a `LobsterParser` and reading messages from it one at a time.
- Constructing `Order` objects from parsed `LobsterMessage` data, including reconstructing a monotonic sequence number (although LOBSTER does not provide it but its rows are already in strict chronological order). The reason behind a sequence number is simple - it is to answer which order is served first if two same price orders arrive at the same time.
- Dispatching each event to the correct `OrderBook` operation (`submitOrder()`,`cancelOrder()`,`reduceOrder()`).
- Providing `Order` objects with stable memory address as long as they may rest in the book.
- Tracking event types and reporting if the current implementation deliberately does not act on (hidden executions, trading halts) rather than silently ignoring them.
- Tracking cases where a message references an `OrderID` the engine never saw submitted, and distinguishing this from a bug.
- Recording per-operation latency for later analysis.

`ReplayEngine` is **not** responsible for :
- Validating book state against LOBSTER's ground-truth orderbookfile (this belongs to `BookValidator`).
- Parsing the raw csv file (`LobsterParser`'s job).
- Any matching or book-keeping logic (that's `OrderBook`'s job).

## Event Dispatch
| LOBSTER Event type | Action |
| ------------------ | ------ |
| 1- Submission | construct an `Order`, call *OrderBook::submitOrder()* |
| 2- Partial Deletion | call *OrderBook::reduceOrder()* |
| 3- Deletion | call *OrderBook::cancelOrder()* |
| 4 - Visible Execution | No action. This is LOBSTER's own record of a trade that has already occured as a consequence of an earlier Submission event. `OrderBook`'s own matching logic should already have produced the trade when it processed that Submission. |
| 5- Hidden Execution | No action, counted only. Requires Iceberg implementation which is currently not in scope of this project |
| 7- Trading Halt | No action, counted only. |

## Sequence Number Reconstruction 
`Order`'s constructor requires a `SequenceNumber` for price-time-priority tiebreaking, but LOBSTER's message format has no such column. Since the message file is already ordered chronologically, `ReplayEngine` maintains a monotonic counter and assigns one sequence number per submission event. 

## Order Ownership During Replay
`ReplayEngine` does not implement a full memory address pool, instead it holds a std::deque<Order> `order_storage_` which never invalidates existing element's addresses on growth (unlike std::vector). 

### Why deque and not vector? (Author's Notes)
One recurring lesson while building Mercury was that the fastest data structure in isolation is not always the best fit for the system. 

`std::vector` is frequently recommended for performance because of its cache locality, but Mercury's design depends heavily on stable pointers and references. In this case, the additional safety and simplicity provided by `std::deque` outweighted the theoretical gains from contiguous storage.

Many engineering decisions in Mercury follow this pattern : *choose the data structure that best satisfies the system's requirements rather than the one with the most impressive benchmark in isolation.*

## Untracked Order Handling 
A resting order's lifecycle events can only be applied if `ReplayEngine` previously saw that order's `Submission`. In practice, a measurable number of events reference orders that predate the start of the sample file, most likely originating from orders that were already resting before the sample window began, meaning their original submission events are absent from the dataset slice being replayed.

`OrderBook::reduceOrder`/`cancelOrder` correctly no-op on an unknown `OrderID` and `ReplayEngine` counts these occurrences separately per event type rather than silently absorbing them, so the scale of this effect is visible and reportable rather than hidden. 

This is a property of validating against a bounded slice of continous real order flow (it cannot explain a state that originated before the window began) and not a defect in matching or dispatch logic (See OrderBook.md, Validation Against Real Data).

## Latency Instrumentation 
`ReplayEngine` records per-operation latency around the exact `OrderBook` call for each dispatched event, using `std::chrono::steady_clock`. These vectors feed the percentile analysis in `tools/latency_report`.

Two caveats stated for accuracy, not hedging :
- Timer calls themselves add a small (tens of ns) measurments overhead not subtracted from the reported figures. The reported latencies are a conservative upper bound.
- `reduce` has a comparatively smaller sample size (few hundered to a few thousand events, depending on file); p99 on a small sample is less statistically robust than on `submit`/`cancel`, which have tens of thousands of samples each.

## Complexity
| Operation | Complexity |
| --------- | ---------- |
| step() | Cost of one parse + one `OrderBook` dispatch call |
| runAll() | O(total events) |

## Future Extensions 
- Real `Exchange`-based memory pool, replacing the `std::deque` stand-in, once multi-symbol support is built. 
- Correlate tail latency (p99/max) with specific causes (multi-level matches vs new price-level insertion) rather than reporting the aggregate tail alone.

`Last updated` : 3rd Sept 2026 