# OrderBook

## Purpose
OrderBook is the core matching engine component for a single trading instrument. It maintains all resting orders on both sides of the market, executes price-time priority matching for all incoming orders, provides O(1) access to the best available price on either side. 

Every order submission, cancellation, and voluntary size reduction in Mercury flows through OrderBook. It is the component that turns individual order instructions into actual trades.

## Responsibilities
OrderBook is responsible for :
- Maintaining two `BookSide`s (bids and asks) for one instrument.
- Executing price-time priority matching when a new order is submitted. 
- Applying `fill()` to both the incoming and resting orders involved in match.
- Resting any unmatched remainder of an incoming order on its own side.
- Providing O(1) lookup from `OrderID` to an order's current location in the book. 
- Supporting cancellation and voluntary partial-quantity reduction of any resting order.
- Reporting best bid/ask and aggregate book statistics.

OrderBook is **not** responsible for :
- Managing multiple instruments (that belongs to a future `Exchange` layer).
- Owning order memory (check Ownership section)
- Risk checks, self-trade prevention, or order validation beyone what Order/Price/Quantity already enforce at construction.
- Publishing market data or maintaining an event log (maybe soon in future)

## Data Model 
``` cpp
struct Trade {
    OrderID incomingId;
    OrderID restingId;
    Price price;
    Quantity quantity;
    EventTimestamp timestamp;
};

class OrderBook {
    BookSide bids_;
    BookSide asks_;
    std::unordered_map<OrderID, Order*> order_lookup_;
    std::size_t peak_active_orders_ = 0;
};
```

Earlier versions of Mercury stored an `OrderLocation` ({`PriceLevel*`, `PriceLevel::Iterator`}) in the lookup table. This was necessary when `PriceLevel` was implemented using `std::list<Order*>`, since O(1) removal required preserving the iterator identifying an order's position within the list. Following the migration to an intrusive linked structure, an `Order*` alone became sufficient: the order itself now contains the linkage required for removal and maintains a pointer to its containing `PriceLevel`. As a result, `order_lookup_` was simplified to store only `Order*` and `OrderID` while preserving O(1) cancellation and reduction.

## Matching Algorithm
On submitOrder(Order& incoming) :
1. Determine opposing/own `BookSide` based on incoming.side()
2. While `incoming.isActive()`:
    - Get `opposing_side.best()`. If none exists or incoming order's price doesnt cross it, stop.
    - Take `opposing_side.best()->front` (oldest resting order at that price - FIFO) and store it as `resting_order`.
    - Match `min(incoming.remainingQuantity(),resting_order.remainingQuantity())`.
    - Apply `fill()` to both the orders. The corresponding PriceLevel updates its aggregate volume as part of the removal/fill bookkeeping. 
    - Record a `Trade`.
    - If the `resting_order` is now fully filled, remove it from the book and from `order_lookup_`.
3. If `incoming` still has remaining quantity and its type allows resting (not `Market`), insert it into its own side and register its `OrderLocation`.
4. Return every `Trade` generated.

Market orders never rest, so the crossing check unconditionally treats them as willing to trade at any available price, and any unmatched remainder is discarded rather than inserted.

## Cancellation and Reduction
- `cancelOrder(OrderID)` : O(1) lookup, unlinks from the resting `BookSide`, erases the `order_lookup_` entry. No-op (returns false) if the ID doesn't exist. 

- `reduceOrder(OrderID id, Quantity amount)` : voluntary size reduction (distinct from a fill since here no trade occurs). If `amount >= remainingQuantity()`, this is dispatched to `cancelOrder` instead, since reducing by the full remaining amount is equivalent to removing the order entirely. 

## Time-In-Force (as of this version)
| Type | Status |
| ---- | ------ |
| Limit | Fully implemented |
| Market | Fully implemented - never rests |
| Iceberg | Accepted as a type, behaves identically to limit. |
| GTC | Fully implemented (default behavior) |
| Day | Not implemented - behaves as GTC. Requires session-boundary/clock infrastructure not yet built. |
| IOC | Not implemented - behaves as GTC. |
| FOK | Not implemented - behaves as GTC (partial fills allowed). Requires an atomic all-or-nothing pre-check across the opposing book before any fill is applied |

These are deliberate scope cuts, not oversights, made to prioritize correctness and validation of the core matching path within the project's timeline.

## Ownership 
As per Mercury's overall design, `Exchange` (not yet built) is intended to own `Order` memory via a stable-address pool; `OrderBook` only ever holds non-owning `Order*` (via `OrderLocation`/the intrusive list). In the current LOBSTER replay pipeline, `ReplayEngine` fills this role pragmatically using a std::deque<Order>`order_storage_` for stable addresses (check ReplayEngine.md).

## Complexity
| Operation | Complexity | Notes |
| --------  | ---------- | ----- |
| Submit (no match, new level) | O(log n)find + O(n)level insert | n = occupied price levels on that side, contiguous shift and cache-friendly for realistic n |
| Submit (matches k resting orders) | O(k) | Each matched level lookup/removal is O(1) amortized |
| Cancel | O(1) amortized | Hash lookup + intrusive unlink |
| Reduce | O(1) | Direct field update, no reposition |
| Best bid/ask | O(1) | Always - sorted-vector *PriceIndex* keeps top-of-book at front()/back() with no rescan-on-empty case |

## Validation Against Real Data 
`OrderBook`, driven by `ReplayEngine`, has been replayed against historical NASDAQ TotalView-ITCH order flow
distributed through the LOBSTER sample dataset. 

**Findings** :
- Core matching, cancellation, and reduction logic is correct and produces plausible, stable book state across a full trading day (118,497 events in L1 and 301,587 events in L5). 
- A measurable fraction of message-file events (partial cancellations and deletions) reference orders that predate the sample window which includes resting liquidity present at market open with no corresponding `Submission` event in the file. These are correctly treated as no-ops by `cancelOrder`/`reduceOrder` (since the referenced `OrderID` was never seen), and are counted, not silently ignored — see `ReplayEngine.md` for exact reasoning.

## Design Decisions 

### Matching lives in OrderBook, not a seperate MatchingEngine
For a single-symbol book, a separate `MatchingEngine` class would either duplicate `OrderBook`'s knowledge of both `BookSide`s or be a trivial pass-through with no logic of its own. Thus, `OrderBook` performs its own matching directly.

### No BookSide iteration exposed
`OrderBook` does not expose raw iteration over price levels. Nothing in the current matching loop needs it since matching only ever calls `best()` repeatedly, and `removeOrder`/`erase` already keep top-of-book correct after every mutation. 
Multi-level access should be added as a purpose-built method returning copied data when actually needed, not by exposing internal iterators.

## Future Extensions 
- Full FOK/IOC/Day semantics
- Iceberg replenishment
- Multi-symbol Exchange layer, with OrderBook per symbol and a shared Order memory pool.
- Self-trade prevention (requires a trader/owner identifier which is not currently modeled on `Order`).

`Last updated` : 3rd Sept 2026 