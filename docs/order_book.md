### Do we need bids() and asks()
we will need this soon. Right now nothing calls these functions as the OrderBook itself does the matching.

Will be itroduced back if we add a MatchingEngine or LOBSTER needs multi-level depth.

### What sumbitOrder() does?
submitOrder() only inserts the incoming order into order_lookup_ if it has remaining quantity after matching. Fully executed incoming orders never become resting orders and therefore never appear in the lookup table.