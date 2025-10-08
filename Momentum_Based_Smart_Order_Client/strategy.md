### 1. Price History Tracking

Our decision algorithm maintains the three most recent prices in a sliding window. Each new price is appended to the history, and the oldest one is removed if the window exceeds three entries. This ensures that the client always evaluates trends using the latest data.

### 2. Momentum Detection

Once three prices are available, denoted as a, b, and c in chronological order:

- **Upward momentum** is detected if a < b < c, i.e., prices are strictly increasing.
- **Downward momentum** is detected if a > b > c, i.e., prices are strictly decreasing.
- If neither condition holds, no momentum is assumed.

### 3. Order Execution

When upward momentum is detected, the client sends a **Buy order** corresponding to the current price ID.

When downward momentum is detected, the client sends a **Sell order**.

Before sending the order, a small random delay (10–59 ms) is introduced to simulate realistic network or execution latency.

If no momentum is observed, the client ignores the price update and takes no action.