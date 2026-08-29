# PriceLevel

## Purpose
`PriceLevel` represents all active resting orders at a single price, on a single side. It maintains strict FIFO ordering among those orders (price-time priority) and tracks their aggregate resting volume. 

## Responsibilites
It is responsible for :
- Holding resting orders at one price as time priority.
- O(1) insertion, removal, and front-of-queue access.
-  Maintaining an aggregate `total_volume_` in sync with its orders.

It is **not** responsible for :
- Matching logic, or knowing which side is "best".
- Owning `Order` memory (only holds non-owning Order*).
- Enforcing price-time priority across levels (that's `PriceIndex`/`BookSide` job).

## Invariants
- Every order has the level's exact `price_` and `side_`.
- `total_volume_` == sum of every order's `remainingVolume()`.
- orders_.empty() == (total_volume_==0).
All of the invariants are verified via a debug-only `verifyInvariants()`, called after every mutation.

## Design Decisions
In the current model `std::list<Order*>` is being implemented. An intrusive design avoids a per-node allocation and was the original target design, but std::list was chosen first in order to first benchmark this version, then convert to instrusive and re-benchmark. Thus, any latency difference would be backed by real measurement rather than assumed.

`total_volume_` is `Volume` and not `Quantity`. `Quantity` must always be strictly positive but `Volume` has not such restriction. This distinction was important because if `Quantity` reaches 0 it will crash the code and last order won't be able to leave the level. Thus Mercury has two seperates numeric types instead of one. 

`addOrder()` returns an iterator. Callers (`BookSide`,`OrderBook`) needs this to erase a specific order later in O(1). Without it, cancellation would require an O(n) scan.

## Complexity

| Operation | Complexity |
| --------- | ---------- |
| Add/erase/front | O(1) |
| Reduce volume | O(1) |

`Last Updated ` : 29th Aug 2026