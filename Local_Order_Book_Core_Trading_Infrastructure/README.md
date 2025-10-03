# Local Order Book and Core Trading Infrastructure

## Architecture

`feed_parser.h`:

Read the `sample_feed.txt` by using provided parse.

`market_snapshot.h /.cpp`:

class `MarketSnapshot` realized the market snapshot by using `std::map` of double and `std::unique_ptr`,  and do the descending sort for bids and acending sort for asks. It also includes `update_bid` and `update_ask` functions to update the market snapshot and `get_best_bid` and `get_best_ask` to get best bid and ask order from sorted map.
 
`std::unique_ptr` is an exclusive smart pointer (RAII, Resource Acquisition Is Initialization).It exclusively manages a dynamically allocated object, cannot be copied, can be moved.When the unique_ptr goes out of scope, it automatically calls delete to release the managed object, preventing memory leaks.

`order_manager.h /.cpp`:

class `OrderBook` realized the functions to proceed an order. `place_order` describes how a order will present in the MarkestSnapshot. `cancel_order` describes how to cancel an order. `handle_fill` describes how to execute an order.

`main.cpp`:

Generate the trading strategy by using a simple logic that `snapshot.get_best_ask()->price <= 100.20` or `snapshot.get_best_bid()->price >= 100.15`(to satisfy the example output). In the main fuction, it will read the `sample_feed.txt` and print corresponding actions and output.

## How to Run
```
g++ main.cpp market_snapshot.cpp order_manager.cpp -o main
./main.exe
```

## How it Works
The function is explained as above. The most important thing here is that we use `std::unique_ptr` to manage the memory, and prevent from using `new` and `delete` directly, which reduces the risk of mamory leak.