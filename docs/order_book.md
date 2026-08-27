# OrderBook

## Purpose
OrderBook is the core matching engine component for a single trading instrument. It maintains all resting orders on both sides of the market, executes price-time priority matching for all incoming orders, provides O(1) access to the best available price on either side. 

Every order submission, cancellation, and voluntary size reduction in Mercury flows through OrderBook. It is the component that turns individual order instructions into actual trades.

## Responsibilties
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
    std::unordered_map<OrderID, OrderEntry> order_lookup_;
};

`OrderEntry` stores an `OrderLocation` ({`PriceLevel*`,`PriceLevel::Iterator`}) which is enough to unlink a specific order from its price level in O(1) without needing to store a seperate `Order*` or `Side`, both of which are recoverable.

## Matching Algorithm
On submitOrder(Order& incoming) :
1. Determine opposing/own `BookSide` based on incoming.side()
2. While `incoming.isActive()`:
    - Get `opposing_side.best()`. If none exists or incoming order's price doesnt cross it, stop.
    - Take `opposing_side.best()->front` (oldest resting order at that price - FIFO) and store it as `resting_order`.
    - Match `min(incoming.remainingQuantity(),resting_order.remainingQuantity())`.
    - Apply `fill()` to both the orders, reduce the price level's aggregate volume. 
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
| Submit (matches k resting orders) | O(k) + level lookups | Each matched level lookup/removal is O(1) amortized |
| Cancel | O(1) amortized | Hash lookup + instrusive unlink |
| Reduce | O(1) | Direct field update, no reposition |
| Best bid/ask | O(1) | Always - sorted-vector PriceIndex keeps top-of-book at front()/back() with no rescan-on-empty case |


