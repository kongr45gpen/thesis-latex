#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <string>
#include "cobs.h"

using namespace std::literals;
using boost::asio::ip::udp;

const auto port = "/dev/ttyACM0"s;
const int baudRate = 115200;

const auto host = "127.0.0.1"s;
const int udpOutPort = 10015;
const int udpInPort = 10025;

int main() {
    try {
        // Initialise serial connection
        boost::asio::io_service io;
        boost::asio::serial_port serial(io, port);
        serial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));

        boost::asio::streambuf buf;
        std::istream is(&buf);
        std::istringstream iss;

        // Initialise UDP connection
        udp::socket txSocket(io);
        txSocket.open(udp::v4());
        udp::endpoint endpoint(boost::asio::ip::make_address_v4(host), udpOutPort);

        udp::endpoint rxEndpoint(boost::asio::ip::make_address_v4(host), udpInPort);
        udp::socket rxSocket(io, rxEndpoint);
        boost::array<unsigned char, 1024> rxbuf;

        std::function<void(const boost::system::error_code&,std::size_t)> udpHandler =
            [&udpHandler, &rxSocket, &rxEndpoint, &rxbuf, &serial]
            (const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (error.failed()) {
                    std::cerr << error.category().name() << ": " << error.message() << std::endl;
                    std::this_thread::sleep_for(500ms);
                } else {
                    std::array<unsigned char, 1024> cobs_buffer;

                    if (bytes_transferred > 4) {
                        std::cout << "Received TC[" 
                            << int{rxbuf[1]} 
                            << "," << int{rxbuf[2]}
                            << "] (" << bytes_transferred
                            << " bytes)" << std::endl;
                    }

                    // COBS encoding
                    auto result = cobs_encode(
                        cobs_buffer.data(), cobs_buffer.size() - 1, rxbuf.data(), bytes_transferred);

                    if (result.status != COBS_ENCODE_OK) {
                        std::cerr << "COBS encode status returned "
                            << static_cast<int>(result.status) << std::endl;
                    } else {
                        // Send packet via serial
                        cobs_buffer[result.out_len] = '\0';
                        boost::asio::write(serial, boost::asio::buffer(cobs_buffer.data(), result.out_len + 1));
                    }
                }

                rxSocket.async_receive_from(boost::asio::buffer(rxbuf), rxEndpoint, udpHandler);
        };

        rxSocket.async_receive_from(boost::asio::buffer(rxbuf), rxEndpoint, udpHandler);

        std::function<void(const boost::system::error_code&,std::size_t)> serialHandler =
            [&serial, &serialHandler, &buf, &txSocket, &endpoint]
            (const boost::system::error_code& error, std::size_t bytes_transferred) -> void {

                if (error.failed()) {
                    std::cerr << error.category().name() << ": " << error.message() << std::endl;
                    std::this_thread::sleep_for(500ms);
                } else {
                    std::string receivedAll(reinterpret_cast<const char *>(buf.data().data()), buf.size());

                    // Find the first occurence of a zero
                    size_t zeroLocation = receivedAll.find('\0');
                    std::string receivedRaw(reinterpret_cast<const char *>(buf.data().data()), zeroLocation);
                    buf.consume(zeroLocation + 1);

                    // COBS decoding
                    std::vector<char> buffer(receivedRaw.size());
                    auto result = cobs_decode(buffer.data(), buffer.size(), receivedRaw.c_str(),
                                                    receivedRaw.size()); // strip the last byte

                    if (result.status != COBS_DECODE_OK) {
                        std::cerr << "COBS status returned " << static_cast<int>(result.status) << std::endl;
                    } else {
                        // Send correct packet via UDP

                        if (buffer[1] == -1 && buffer[2] == -1) {
                            std::cout << std::string(buffer.data(), result.out_len).substr(4);
                        } else {
                            txSocket.send_to(boost::asio::buffer(buffer.data(), result.out_len), endpoint);
                            std::cout << "\u001b[38;5;243m"
                                << "Received TM (" << receivedRaw.size()<< ")" << "\u001b[0m" << std::endl;
                        }
                    }
                }

            // Read next data packet
            boost::asio::async_read_until(serial, buf, '\0', serialHandler);
        };

        boost::asio::async_read_until(serial, buf, '\0', serialHandler);

        io.run();



    } catch (boost::system::system_error &e) {
        std::cerr << e.what() << std::endl;
    }

    std::cout << "endl" << std::endl;
}
