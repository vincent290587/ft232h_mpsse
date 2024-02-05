#include <windows.h>
#include <iostream>

#include <chrono>
#include <thread>

// https://github.com/sigrokproject/libserialport
// pacman -S mingw-w64-x86_64-libserialport
#include <libserialport.h>

using namespace std;

#define APP_CHECK_STATUS(X)     do { auto ret = X; if (ret) {printf("Error %d line %u \n", ret, __LINE__);} } while (0)

#define check(X)   APP_CHECK_STATUS(X)

static inline int _send_receive(struct sp_port *tx_port, struct sp_port *rx_port, const uint8_t data[], const size_t size) {

    /* We'll allow a 1 second timeout for send and receive. */
    unsigned int timeout = 200;
    uint8_t buf[128];
    memset(buf, 0xFF, sizeof(buf));

    // drain buffers
    (void)sp_nonblocking_read(rx_port, buf, sizeof(buf));

    int result = sp_nonblocking_write(tx_port, data, size);
    APP_CHECK_STATUS(result != size);

    /* Try to receive the data on the other port. */
    result = sp_blocking_read(rx_port, buf, size, timeout);
    APP_CHECK_STATUS(result != size);

    printf("Sent: \n -> ");

    for (unsigned i=0; i < size && i < sizeof(buf); i++) {

        printf("0x%02X ", data[i]);
    }
    printf("\n");

    printf("Received: \n <- ");

    for (unsigned i=0; i < size && i < sizeof(buf); i++) {

        printf("0x%02X ", buf[i]);
    }
    printf("\n");

    return result;
}

/* Helper function for error handling. */
//int check(enum sp_return result);

int main(int argc, char **argv)
{

    std::cout.setf(std::ios::unitbuf);

    if (argc < 2) {
        printf("Missing COM port name\n");
        return -1;
    }

    int num_ports = argc - 1;
    char **port_names = argv + 1;

    /* The ports we will use. */
    struct sp_port *ports[2];

    for (int i = 0; i < num_ports; i++) {

        printf("Looking for port %s.\n", port_names[i]);
        check(sp_get_port_by_name(port_names[i], &ports[i]));

        printf("Opening port.\n");
        check(sp_open(ports[i], SP_MODE_READ_WRITE));

        printf("Setting port to 1200 8N1, no flow control.\n");
        check(sp_set_baudrate(ports[i], 1200));
        check(sp_set_bits(ports[i], 8));
        check(sp_set_parity(ports[i], SP_PARITY_NONE));
        check(sp_set_stopbits(ports[i], 1));
        check(sp_set_flowcontrol(ports[i], SP_FLOWCONTROL_NONE));
    }

#if 0
    /* Now send some data on each port and receive it back. */
    for (int tx = 0; tx < num_ports; tx++) {
        /* Get the ports to send and receive on. */
        int rx = num_ports == 1 ? 0 : ((tx == 0) ? 1 : 0);
        struct sp_port *tx_port = ports[tx];
        struct sp_port *rx_port = ports[rx];

        /* The data we will send. */
        const char *data = "Hello!";
        int size = strlen(data);

        /* We'll allow a 1 second timeout for send and receive. */
        unsigned int timeout = 1000;

        /* On success, sp_blocking_write() and sp_blocking_read()
         * return the number of bytes sent/received before the
         * timeout expired. We'll store that result here. */
        int result;

        /* Send data. */
        printf("Sending '%s' (%d bytes) on port %s.\n",
               data, size, sp_get_port_name(tx_port));
        result = sp_blocking_write(tx_port, data, size, timeout);

        /* Check whether we sent all of the data. */
        if (result == size)
            printf("Sent %d bytes successfully.\n", size);
        else
            printf("Timed out, %d/%d bytes sent.\n", result, size);

        /* Allocate a buffer to receive data. */
        char *buf = (char *)malloc(size + 1);

        /* Try to receive the data on the other port. */
        printf("Receiving %d bytes on port %s.\n",
               size, sp_get_port_name(rx_port));
        result = sp_blocking_read(rx_port, buf, size, timeout);

        /* Check whether we received the number of bytes we wanted. */
        if (result == size)
            printf("Received %d bytes successfully.\n", size);
        else
            printf("Timed out, %d/%d bytes received.\n", result, size);

        /* Check if we received the same data we sent. */
        buf[result] = '\0';
        printf("Received '%s'.\n", buf);

        /* Free receive buffer. */
        free(buf);
    }
#else

    /* Get the ports to send and receive on. */
    struct sp_port *tx_port = ports[0];
    struct sp_port *rx_port = ports[0];

    {
        const uint8_t data[] = {0x11, 0x20};
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    std::this_thread::sleep_for(0.1s);

    {
        const uint8_t data[] = {0x16, 0x0B, 0x03, 0x24}; // PAS1
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    std::this_thread::sleep_for(0.1s);

    {
        const uint8_t data[] = { 0x16 , 0x1F , 0x00 , 0xBD , 0xF2};
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    {
        const uint8_t data[] = {0x16, 0x0B, 0x02, 0x23}; // PAS2
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    std::this_thread::sleep_for(0.1s);

    {
        const uint8_t data[] = { 0x16 , 0x1F , 0x00 , 0xBD , 0xF2};
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

#endif

//	cout << "Press Enter to Continue";
//	cin.ignore();

    /* Close ports and free resources. */
    for (int i = 0; i < num_ports; i++) {
        check(sp_close(ports[i]));
        sp_free_port(ports[i]);
    }

	return 0;
}
