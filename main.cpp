#pragma once

#include <modbus/modbus.h>

#include <mutex>
#include <atomic>
#include <iostream>
#include <vector>
#include <string>

namespace modbus {

class LibModbusTcpTransport{

public:
    LibModbusTcpTransport(std::string host, int port, int slaveId);
    ~LibModbusTcpTransport();
    bool connect();
    void disconnect();
    bool isConnected() const;
    bool setSlave(int slaveId);

    std::string getHost() const;

    std::vector<uint16_t> readHoldingRegisters(uint16_t start, uint16_t count);
    bool writeHoldingRegister(uint16_t address, uint16_t value);

    std::vector<uint8_t> readCoils(uint16_t start, uint16_t count);
    bool writeCoil(uint16_t address, uint16_t value);

    std::vector<uint16_t> readInputRegisters(uint16_t start, uint16_t count);
    std::vector<uint8_t> readInputCoils(uint16_t start, uint16_t count);

    std::string lastError() const;

private:
    void cleanup();

private:
    std::string host_;
    int port_;
    int slaveId_;
    // Note: LibModbus context is not thread safe, so there should always be 1 context per device connected.
    modbus_t* ctx_ = nullptr;
    std::atomic<bool> connected_ {false};
    mutable std::mutex mutex_;
    std::string lastError_;
};

}

namespace modbus {

LibModbusTcpTransport::LibModbusTcpTransport(std::string host, int port, int slaveId): host_(std::move(host)), port_(port), slaveId_(slaveId) {

}

LibModbusTcpTransport::~LibModbusTcpTransport() {
    cleanup();
}

bool LibModbusTcpTransport::connect() {
    std::lock_guard lock(mutex_);
    if(connected_) {
        return true;
    }

    ctx_ = modbus_new_tcp(host_.c_str(), port_);

    if(!ctx_) {
        lastError_ = "Failed to create modbus context";
        return false;
    }

    struct timeval timeout;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    modbus_set_response_timeout(ctx_, timeout.tv_sec, timeout.tv_usec);

    if(modbus_connect(ctx_) == -1) {
        lastError_ = modbus_strerror(errno);
        cleanup();
        return false;
    }

    if(modbus_set_slave(ctx_, slaveId_) == -1) {
        lastError_ = modbus_strerror(errno);
        cleanup();
        return false;
    }

    connected_ = true;
    return true;
}

void LibModbusTcpTransport::disconnect() {
    std::lock_guard lock(mutex_);
    cleanup();
}

bool LibModbusTcpTransport::isConnected() const {
    return connected_;
}

bool LibModbusTcpTransport::setSlave(int slaveId) {
    std::lock_guard lock(mutex_);
    slaveId_ = slaveId;
    if(!ctx_) {
        return true;
    }

    if(modbus_set_slave(ctx_, slaveId) == -1) {
        lastError_ = modbus_strerror(errno);
        return false;
    }

    return true;
}

std::string LibModbusTcpTransport::getHost() const {
    return host_;
}

std::vector<uint16_t> LibModbusTcpTransport::readHoldingRegisters(uint16_t start, uint16_t count) {
    std::lock_guard lock(mutex_);
    std::vector<uint16_t> result(count);
    if(!connected_) {
        return {};
    }

    int rc = modbus_read_registers(ctx_, start, count, result.data());

    if(rc == -1) {
        lastError_ = modbus_strerror(errno);
        connected_ = false;
        cleanup();
        return {};
    }

    return result;
}

bool LibModbusTcpTransport::writeHoldingRegister(uint16_t address, uint16_t value) {
    std::lock_guard lock(mutex_);
    if(!connected_) {
        return false;
    }

    int rc = modbus_write_register(ctx_, address, value);

    if(rc == -1) {
        lastError_ = modbus_strerror(errno);
        connected_ = false;
        cleanup();
        return false;
    }

    return true;
}

std::vector<uint8_t> LibModbusTcpTransport::readCoils(uint16_t start, uint16_t count){
    std::lock_guard lock(mutex_);
    std::vector<uint8_t> result(count);
    if(!connected_) {
        return {};
    }

    int rc = modbus_read_bits(ctx_, start, count, result.data());

    if(rc == -1) {
        lastError_ = modbus_strerror(errno);
        connected_ = false;
        cleanup();
        return {};
    }

    return result;
}

bool LibModbusTcpTransport::writeCoil(uint16_t address, uint16_t value){
    std::lock_guard lock(mutex_);
    if(!connected_) {
        return false;
    }

    int rc = modbus_write_bit(ctx_, address, value);

    if(rc == -1) {
        lastError_ = modbus_strerror(errno);
        connected_ = false;
        cleanup();
        return false;
    }

    return true;
}

std::vector<uint16_t> LibModbusTcpTransport::readInputRegisters(uint16_t start, uint16_t count){
    std::lock_guard lock(mutex_);
    std::vector<uint16_t> result(count);
    if(!connected_) {
        return {};
    }

    int rc = modbus_read_input_registers(ctx_, start, count, result.data());

    if(rc == -1) {
        lastError_ = modbus_strerror(errno);
        connected_ = false;
        cleanup();
        return {};
    }

    return result;
}

std::vector<uint8_t> LibModbusTcpTransport::readInputCoils(uint16_t start, uint16_t count){
    std::lock_guard lock(mutex_);
    std::vector<uint8_t> result(count);
    if(!connected_) {
        return {};
    }

    int rc = modbus_read_input_bits(ctx_, start, count, result.data());

    if(rc == -1) {
        lastError_ = modbus_strerror(errno);
        connected_ = false;
        cleanup();
        return {};
    }

    return result;
}

std::string LibModbusTcpTransport::lastError() const {
    return lastError_;
}

void LibModbusTcpTransport::cleanup() {
    connected_ = false;
    if(ctx_) {
        modbus_close(ctx_);
        modbus_free(ctx_);
        ctx_ = nullptr;
    }
}

}

int main() {
    // auto transport = std::make_shared<LibModbusTcpTransport>();
    auto transport = std::make_shared<modbus::LibModbusTcpTransport>("127.0.0.1", 502, 1);

    transport->connect();

    if(transport->isConnected()){

        uint16_t startAddr{8123};
        uint16_t count{10};

        auto coils = transport->readCoils(startAddr, count);

        for(auto coil: coils){
            std::cout << "coil addr: " << startAddr << ", count: " << coil << std::endl;
        }
    }
    else{
        std::cout << "device not connected at: " << transport->getHost() << std::endl;
    }


    return 0;
}


