/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#include <qpidit/amqp_types_test/Receiver.hpp>

#include <iostream>
#include <json/json.h>
#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/error_condition.hpp>
#include <proton/delivery.hpp>
#include <proton/message.hpp>
#include <proton/receiver.hpp>
#include <proton/thread_safe.hpp>
#include <proton/transport.hpp>
#include <qpidit/QpidItErrors.hpp>

namespace qpidit
{
    namespace amqp_types_test
    {

        Receiver::Receiver(const std::string& brokerUrl,
                           const std::string& queueName,
                           const std::string& amqpType,
                           uint32_t expected) :
                        _brokerUrl(brokerUrl),
                        _queueName(queueName),
                        _amqpType(amqpType),
                        _expected(expected),
                        _received(0UL),
                        _receivedValueList(Json::arrayValue)
        {}

        Receiver::~Receiver() {}

        Json::Value& Receiver::getReceivedValueList() {
            return _receivedValueList;
        }

        void Receiver::on_container_start(proton::container &c) {
            std::ostringstream oss;
            oss << _brokerUrl << "/" << _queueName;
            c.open_receiver(oss.str());
        }

        void Receiver::on_message(proton::delivery &d, proton::message &m) {
            try {
                if (_received < _expected) {
                    if (_amqpType.compare("null") == 0) {
                        checkMessageType(m, proton::NULL_TYPE);
                        _receivedValueList.append("None");
                    } else if (_amqpType.compare("boolean") == 0) {
                        checkMessageType(m, proton::BOOLEAN);
                        _receivedValueList.append(proton::get<bool>(m.body()) ? "True": "False");
                    } else if (_amqpType.compare("ubyte") == 0) {
                        checkMessageType(m, proton::UBYTE);
                        _receivedValueList.append(toHexStr<uint8_t>(proton::get<uint8_t>(m.body())));
                    } else if (_amqpType.compare("ushort") == 0) {
                        checkMessageType(m, proton::USHORT);
                        _receivedValueList.append(toHexStr<uint16_t>(proton::get<uint16_t>(m.body())));
                    } else if (_amqpType.compare("uint") == 0) {
                        checkMessageType(m, proton::UINT);
                        _receivedValueList.append(toHexStr<uint32_t>(proton::get<uint32_t>(m.body())));
                    } else if (_amqpType.compare("ulong") == 0) {
                        checkMessageType(m, proton::ULONG);
                        _receivedValueList.append(toHexStr<uint64_t>(proton::get<uint64_t>(m.body())));
                    } else if (_amqpType.compare("byte") == 0) {
                        checkMessageType(m, proton::BYTE);
                        _receivedValueList.append(toHexStr<int8_t>(proton::get<int8_t>(m.body())));
                    } else if (_amqpType.compare("short") == 0) {
                        checkMessageType(m, proton::SHORT);
                        _receivedValueList.append(toHexStr<int16_t>(proton::get<int16_t>(m.body())));
                    } else if (_amqpType.compare("int") == 0) {
                        checkMessageType(m, proton::INT);
                        _receivedValueList.append(toHexStr<int32_t>(proton::get<int32_t>(m.body())));
                    } else if (_amqpType.compare("long") == 0) {
                        checkMessageType(m, proton::LONG);
                        _receivedValueList.append(toHexStr<int64_t>(proton::get<int64_t>(m.body())));
                    } else if (_amqpType.compare("float") == 0) {
                        checkMessageType(m, proton::FLOAT);
                        float f = proton::get<float>(m.body());
                        _receivedValueList.append(toHexStr<uint32_t>(*((uint32_t*)&f), true));
                    } else if (_amqpType.compare("double") == 0) {
                        checkMessageType(m, proton::DOUBLE);
                        double d = proton::get<double>(m.body());
                        _receivedValueList.append(toHexStr<uint64_t>(*((uint64_t*)&d), true));
                    } else if (_amqpType.compare("decimal32") == 0) {
                        checkMessageType(m, proton::DECIMAL32);
                        _receivedValueList.append(byteArrayToHexStr(proton::get<proton::decimal32>(m.body())));
                    } else if (_amqpType.compare("decimal64") == 0) {
                        checkMessageType(m, proton::DECIMAL64);
                        _receivedValueList.append(byteArrayToHexStr(proton::get<proton::decimal64>(m.body())));
                    } else if (_amqpType.compare("decimal128") == 0) {
                        checkMessageType(m, proton::DECIMAL128);
                        _receivedValueList.append(byteArrayToHexStr(proton::get<proton::decimal128>(m.body())));
                    } else if (_amqpType.compare("char") == 0) {
                        checkMessageType(m, proton::CHAR);
                        wchar_t c = proton::get<wchar_t>(m.body());
                        std::stringstream oss;
                        if (c < 0x7f && std::iswprint(c)) {
                            oss << (char)c;
                        } else {
                            oss << "0x" << std::hex << c;
                        }
                        _receivedValueList.append(oss.str());
                    } else if (_amqpType.compare("timestamp") == 0) {
                        checkMessageType(m, proton::TIMESTAMP);
                        std::ostringstream oss;
                        oss << "0x" << std::hex << proton::get<proton::timestamp>(m.body()).milliseconds();
                        _receivedValueList.append(oss.str());
                    } else if (_amqpType.compare("uuid") == 0) {
                        checkMessageType(m, proton::UUID);
                        std::ostringstream oss;
                        oss << proton::get<proton::uuid>(m.body());
                        _receivedValueList.append(oss.str());
                    } else if (_amqpType.compare("binary") == 0) {
                        checkMessageType(m, proton::BINARY);
                        _receivedValueList.append(std::string(proton::get<proton::binary>(m.body())));
                    } else if (_amqpType.compare("string") == 0) {
                        checkMessageType(m, proton::STRING);
                        _receivedValueList.append(proton::get<std::string>(m.body()));
                    } else if (_amqpType.compare("symbol") == 0) {
                        checkMessageType(m, proton::SYMBOL);
                        _receivedValueList.append(proton::get<proton::symbol>(m.body()));
                    } else if (_amqpType.compare("list") == 0) {
                        checkMessageType(m, proton::LIST);
                        Json::Value jsonList(Json::arrayValue);
                        _receivedValueList.append(getSequence(jsonList, m.body()));
                    } else if (_amqpType.compare("map") == 0) {
                        checkMessageType(m, proton::MAP);
                        Json::Value jsonMap(Json::objectValue);
                        _receivedValueList.append(getMap(jsonMap, m.body()));
                    } else if (_amqpType.compare("array") == 0) {
                        throw qpidit::UnsupportedAmqpTypeError(_amqpType);
                    } else {
                        throw qpidit::UnknownAmqpTypeError(_amqpType);
                    }
                }
                _received++;
                if (_received >= _expected) {
                    d.receiver().close();
                    d.connection().close();
                }
            } catch (const std::exception&) {
                d.receiver().close();
                d.connection().close();
                throw;
            }
        }

        void Receiver::on_connection_error(proton::connection &c) {
            std::cerr << "AmqpReceiver::on_connection_error(): " << c.error() << std::endl;
        }

        void Receiver::on_receiver_error(proton::receiver& r) {
            std::cerr << "AmqpReceiver::on_receiver_error(): " << r.error() << std::endl;
        }

        void Receiver::on_session_error(proton::session &s) {
            std::cerr << "AmqpReceiver::on_session_error(): " << s.error() << std::endl;
        }

        void Receiver::on_transport_error(proton::transport &t) {
            std::cerr << "AmqpReceiver::on_transport_error(): " << t.error() << std::endl;
        }

        void Receiver::on_error(const proton::error_condition &ec) {
            std::cerr << "AmqpReceiver::on_error(): " << ec << std::endl;
        }

        // protected

        //static
        void Receiver::checkMessageType(const proton::message& msg, proton::type_id amqpType) {
            if (msg.body().type() != amqpType) {
                throw qpidit::IncorrectMessageBodyTypeError(amqpType, msg.body().type());
            }
        }

        //static
        Json::Value& Receiver::getMap(Json::Value& jsonMap, const proton::value& val) {
            std::map<proton::value, proton::value> msgMap;
            proton::get(val, msgMap);
            for (std::map<proton::value, proton::value>::const_iterator i = msgMap.begin(); i != msgMap.end(); ++i) {
                switch (i->second.type()) {
                case proton::LIST:
                {
                    Json::Value jsonSubList(Json::arrayValue);
                    jsonMap[proton::get<std::string>(i->first)] = getSequence(jsonSubList, i->second);
                    break;
                }
                case proton::MAP:
                {
                    Json::Value jsonSubMap(Json::objectValue);
                    jsonMap[proton::get<std::string>(i->first)] = getMap(jsonSubMap, i->second);
                    break;
                }
                case proton::ARRAY:
                    break;
                case proton::STRING:
                    jsonMap[proton::get<std::string>(i->first)] = Json::Value(proton::get<std::string>(i->second));
                    break;
                default:
                    throw qpidit::IncorrectValueTypeError(i->second);
                }
            }
            return jsonMap;
        }

        //static
        Json::Value& Receiver::getSequence(Json::Value& jsonList, const proton::value& val) {
            std::vector<proton::value> msgList;
            proton::get(val, msgList);
            for (std::vector<proton::value>::const_iterator i=msgList.begin(); i!=msgList.end(); ++i) {
                switch ((*i).type()) {
                case proton::LIST:
                {
                    Json::Value jsonSubList(Json::arrayValue);
                    jsonList.append(getSequence(jsonSubList, *i));
                    break;
                }
                case proton::MAP:
                {
                    Json::Value jsonSubMap(Json::objectValue);
                    jsonList.append(getMap(jsonSubMap, *i));
                    break;
                }
                case proton::ARRAY:
                    break;
                case proton::STRING:
                    jsonList.append(Json::Value(proton::get<std::string>(*i)));
                    break;
                default:
                    throw qpidit::IncorrectValueTypeError(*i);
                }
            }
            return jsonList;
        }

        //static
        std::string Receiver::stringToHexStr(const std::string& str) {
            std::ostringstream oss;
            oss << "0x" << std::hex;
            for (std::string::const_iterator i=str.begin(); i!=str.end(); ++i) {
                oss << std::setw(2) << std::setfill('0') << ((int)*i & 0xff);
            }
            return oss.str();
        }

    } /* namespace amqp_types_test */
} /* namespace qpidit */


/*
 * --- main ---
 * Args: 1: Broker address (ip-addr:port)
 *       2: Queue name
 *       3: AMQP type
 *       4: Expected number of test values to receive
 */

int main(int argc, char** argv) {
    // TODO: improve arg management a little...
    if (argc != 5) {
        throw qpidit::ArgumentError("Incorrect number of arguments");
    }

    try {
        qpidit::amqp_types_test::Receiver receiver(argv[1], argv[2], argv[3], std::strtoul(argv[4], NULL, 0));
        proton::container(receiver).run();

        std::cout << argv[3] << std::endl;
        Json::FastWriter fw;
        std::cout << fw.write(receiver.getReceivedValueList());
    } catch (const std::exception& e) {
        std::cerr << "AmqpReceiver error: " << e.what() << std::endl;
        exit(-1);
    }
    exit(0);
}
