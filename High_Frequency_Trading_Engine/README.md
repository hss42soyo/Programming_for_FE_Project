# IEOR 4741 Assignment 2 Report

**Original Console Output: (Run on Apple M1 CPU, with -O3 enabled)**

```
--- Performance Report ---
Total Market Ticks Processed: 100000
Total Orders Placed: 94094
Average Tick-to-Trade Latency (ns): 3603361
Maximum Tick-to-Trade Latency (ns): 4981500
Total Runtime (ms): 7
```

## QAs

### 1. Signals

By adding 3 signal counters, we get:

```
Signal 1 Triggers: 9929
Signal 2 Triggers: 92986
Signal 3 Triggers: 16559
```

So signal 2 triggered the most orders.

### 2. Optimization

The original code generated all the ticks at once, then start to make orders, which produced high latencies. If we could place orders while ticks are genrated, latencies will be much lower. See result:

```
--- Performance Report ---
Total Market Ticks Processed: 100000
Total Orders Placed: 94019
Average Tick-to-Trade Latency (ns): 35
Maximum Tick-to-Trade Latency (ns): 16417
Total Runtime (ms): 6
```

Latencies reduced significantly, while total runtime remained the same.

### 3. With 10x more data

Original:

```
--- Performance Report ---
Total Market Ticks Processed: 1000000
Total Orders Placed: 941087
Average Tick-to-Trade Latency (ns): 111
Maximum Tick-to-Trade Latency (ns): 187584
Signal 1 Triggers: 100093
Signal 2 Triggers: 933575
Signal 3 Triggers: 166442
Signal 4 Triggers: 14074
--------------------------
Total Runtime (ms): 152
```

The total runtime increases by roughly 10 times, but average and maximum latency also increases 10 times. It's because latency is calculated by order time minus the time data generated, but the **`process()` function was called after generating all of the data.


<!-- Also, we discovered that funtion `erase()` will cost much time, so we use a ring buffer to store tick data to prevent using `erase()` and reduced the latency.

Optimized: -->
### 4. Optional
How many orders each signal contributed is shown as above, and signal2 triggers most.

We added a volatility based signal4, which measures how far the current price is from the average price with the distance of standard deviation:

```
int TradeEngine::signal4(const MarketData &tick)
{
    double avg = getAvg(tick.instrument_id);
    double std = getStd(tick.instrument_id);
    double dir = (tick.price - avg)/std;
    if (dir > 2){
        return -1;
    }
    else if (dir < -2){
        return 1;
    }
    else{
        return 0;
    }
}
```

Last, we export order history to a CSV file and visualize it with Python, the result is in [orders.csv](./High_Frequency_Trading_Engine/orders.csv). and [visualization.ipynb](./High_Frequency_Trading_Engine/visualization.ipynb).

