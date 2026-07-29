## Design Rationale

### Why std::list<Order*>?

Alternatives considered:

- std::vector<Order*>
- std::deque<Order*>
- Intrusive list

Decision:
std::list<Order*> was chosen because:
- O(1) FIFO removal
- O(1) cancellation with cached iterators
- Stable iterators
- Container hidden behind the PriceLevel interface, allowing future replacement without API changes

Future optimization:
Replace std::list with an intrusive linked list if profiling shows allocator or cache overhead is significant.

## What PriceLevel Owns?
A PriceLevel is responsible for exactly four things:
1. Maintaining FIFO order
2. Maintaining aggregate quantity
3. Ensuring every order belongs to this price level
4. Providing efficient iteration

Nothing else.

## PriceLevel Invariants
Every order :
- has the same price
- has the same side
- is active
total_volume is the sum of remainingVolume(order)