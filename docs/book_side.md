# BookSide 

## Purpose 
`BookSide` manages all resting orders on one side(bids or asks) of an `OrderBook`. It is the layer that gives price-time priority meaning in a market sense. 
It owns `PriceIndex` for it's side and define what "best price" actually mean for that side.

## Responsibilities 

It is responsible for :
- Owning a `PriceIndex` for one side.
- Defining "best" for its side.
- Adding an order to its price level or creating the level if needed.
- Removing an order and cleaning up an emptied level.

It is **not** responsible for :
- Matching logic or knowldege of the opposing side.
- Order memory ownership (holds/references orders via `0rder&`/`PriceLevel*`)
- Market order handling (that distinction belongs to `OrderBook`, which decides wether to call `BookSide` for a market order's remainder at all).

## Data Model
struct OrderLocation {
    PriceLevel* level;
    PriceLevel::Iterator iterator;
};
class BookSide {
    PriceIndex index_;
    Side side_;
};

`OrderLocation` is the handle every caller keeps to reference a specific resting order, thus enough to erase it in O(1) without a search and enough to recover its side via `location.level->side()` without storing it redundantly. 

## Design Decisions
1. `best()` is not a new lookup, it is just a one-line translation of `PriceIndex`'s neutral ordering. `PriceIndex` only knows ascending order, `BookSide` maps "highest=best" for bids and "lowest=best" for asks side. This keeps the side-direction concept in exactly one place rather than letting it leak into `PriceIndex` or being recomputed at every call.

2. No iteration exposed. An earlier draft had `PriceIndex`'s iterators exposed publicly. This decision was removed as nothing in the matching loop needs to walk every level, it only calls `best()` repeatedly and `removeOrder()` already keeps to top-of-the-book correct after every mutation. Exposing iteration would also leak `PriceIndex`'s internal representation (its sorted-vector storage) to every caller, making that representation harder to change later without breaking callers. 

3. No `reduceVolume()` forwarding method. `best()` and `OrderLocation::level` already return raw `PriceLevel*`. A caller holding it can call `level->reduceVolume()` directly with no loss of enscapsulation, since that boundary was already crossed the moment pointers were returned. 

4. `removeOrder()` is not a pure pass-through. Beyond erasing from the `PriceLevel`'s list, it checks whether the level is now empty and erases it from `PriceIndex` if it is so, maintaining the *"PriceIndex never holds empty level"* invariant.

## Complexity 

| Operation | Complexity |
| --------- | ---------- |
| best() | O(1) always |
| addOrder() | O(log(n)) find/create + O(1) list insert |
| removeOrder() | O(log(n)) + O(1) list erase + ( O(n) shift only if the level empties) |

