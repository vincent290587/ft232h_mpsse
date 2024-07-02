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

static inline void _print_bytes(uint8_t buf[], const size_t size) {

    for (unsigned i=0; i < size; i++) {

        printf("0x%02X ", buf[i]);
    }
    printf("\n");
}

static inline int _send_receive(struct sp_port *tx_port, struct sp_port *rx_port, const uint8_t data[], const size_t size) {

    /* We'll allow a timeout for send and receive. */
    unsigned int timeout_ms = 125;
    uint8_t buf[128];
    memset(buf, 0xFF, sizeof(buf));

    // drain buffers
    (void)sp_nonblocking_read(rx_port, buf, sizeof(buf));

    int result = sp_nonblocking_write(tx_port, data, size);
    APP_CHECK_STATUS(result != size);

    /* Try to receive the data on the other port. */
    result = sp_blocking_read(rx_port, buf, size, timeout_ms);
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

    /* Get the ports to send and receive on. */
    struct sp_port *tx_port = ports[0];
    struct sp_port *rx_port = ports[0];

#if 1
    /* Now send some data on each port and receive it back. */
    uint8_t buf[128];
    memset(buf, 0xFF, sizeof(buf));

    const uint8_t data[] = {0x16, 0x0B, 0x03, 0x24}; // PAS3

    for (int i=0; i < 3000; i++) {

        int result = sp_nonblocking_read(rx_port, buf, sizeof(buf));
        if (result == sizeof(data)) {
            _print_bytes(buf, result);
        } else if (result) {
            printf("Insufficient data");
            _print_bytes(buf, result);
            if (i>0) {
                break;
            }
        } else {
            printf("No data");
            if (i>0) {
                break;
            }
        }

        result = sp_nonblocking_write(tx_port, data, sizeof(data));
        APP_CHECK_STATUS(result != sizeof(data));

        cout.flush();
        // recv timeout = 15
        // byte duration: 8.333 ms
        std::this_thread::sleep_for(0.1s);
    }
#else

    {
        const uint8_t data[] = {0x11, 0x20};
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    std::this_thread::sleep_for(0.1s);

    {
        const uint8_t data[] = {0x16, 0x0B, 0x0C, 0x2D}; // PAS1
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    std::this_thread::sleep_for(0.1s);

    {
        const uint8_t data[] = { 0x16 , 0x1F , 0x00 , 0xBD , 0xF2}; // RPM
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    {
        const uint8_t data[] = {0x16, 0x0B, 0x02, 0x23}; // PAS2
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    std::this_thread::sleep_for(0.1s);

    {
        const uint8_t data[] = { 0x16 , 0x1F , 0x00 , 0xBD , 0xF2}; // RPM
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    std::this_thread::sleep_for(0.1s);

    {
        const uint8_t data[] = {0x16, 0x0B, 0x03, 0x24}; // PAS3
        _send_receive(tx_port, rx_port, data, sizeof(data));
    }

    std::this_thread::sleep_for(0.1s);

    {
        const uint8_t data[] = { 0x16 , 0x1F , 0x00 , 0xBD , 0xF2}; // RPM
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
