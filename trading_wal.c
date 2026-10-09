#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// Fixed-size binary struct for speed and predictable offsets
typedef struct {
    int order_id;
    char symbol[8];
    int quantity;
    double price;
    char side; // 'B' for Buy, 'S' for Sell
} Order;

int main() {
    // Open log with O_APPEND.
    // Using O_DSYNC forces write() to block until data is physically on disk.
    int fd = open("trading_wal.log", O_CREAT | O_WRONLY | O_APPEND | O_DSYNC, 0644);
    if (fd < 0) {
        perror("Failed to open WAL");
        return 1;
    }

    // Simulate an incoming order
    Order new_order = {101, "AAPL", 50, 150.25, 'B'};

    // 1. Write the order to the log
    ssize_t bytes_written = write(fd, &new_order, sizeof(Order));
    if (bytes_written != sizeof(Order)) {
        perror("Failed to write to WAL");
        // Do not process order! State is unsafe.
        close(fd);
        return 1;
    }

    // 2. Data is now safe on disk (thanks to O_DSYNC).
    // If the system crashes right now, the recovery routine will read this file
    // and push it to the order book on reboot.

    // 3. Forward to in-memory Matching Engine
    // process_order_in_memory(&new_order);

    printf("Order %d committed and sent to engine.\\n", new_order.order_id);

    close(fd);
    return 0;
}
